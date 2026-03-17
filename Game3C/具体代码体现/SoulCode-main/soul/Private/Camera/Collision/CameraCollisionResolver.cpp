// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Collision/CameraCollisionResolver.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

/**
 * CameraCollisionResolver.cpp
 * 
 * Implementation of the collision resolver that manages all 20 collision strategies.
 * 
 * This is the complete version with all strategies registered:
 * - Step 5.2: Detection Strategies (D01-D04)
 * - Step 5.3: Response Strategies (RS01-RS05)
 * - Step 5.4: Occlusion Strategies (OC01-OC04)
 * - Step 5.5: Recovery + Special Strategies (RC01-RC03, SP01-SP04)
 * 
 * Execution Flow:
 * 1. Detection Phase - Detect collisions using Det strategies
 * 2. Response Phase - Calculate adjustments using Res strategies
 * 3. Occlusion Phase - Handle view blocking using Occ strategies
 * 4. Recovery Phase - Return to normal using Rec strategies
 * 5. Special Phase - Handle special scenarios using Spc strategies
 */

// Detection Strategies (Step 5.2)
#include "Camera/Collision/CameraCollision_Detection.h"

// Response Strategies (Step 5.3)
#include "Camera/Collision/CameraCollision_Response.h"

// Occlusion Strategies (Step 5.4)
#include "Camera/Collision/CameraCollision_Occlusion.h"

// Recovery Strategies (Step 5.5)
#include "Camera/Collision/CameraCollision_Recovery.h"

// Special Strategies (Step 5.5)
#include "Camera/Collision/CameraCollision_Special.h"


//========================================
// Constructor
//========================================

UCameraCollisionResolver::UCameraCollisionResolver()
	: PreCollisionDistance(0.0f)
	, PreCollisionPosition(FVector::ZeroVector)
	, TimeSinceOcclusionCheck(0.0f)
	, bIsInitialized(false)
{
}


//========================================
// Initialization
//========================================

void UCameraCollisionResolver::Initialize(USoulsCameraManager* InCameraManager)
{
	if (bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraCollisionResolver: Already initialized"));
		return;
	}

	CameraManager = InCameraManager;

	// Reset state
	CollisionState.Reset();
	CurrentDetectionResult.Reset();
	PreviousDetectionResult.Reset();

	// Create all collision strategies
	CreateAllStrategies(this);

	bIsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("CameraCollisionResolver: Initialized with %d strategies"), 
		RegisteredStrategies.Num());
}

void UCameraCollisionResolver::Shutdown()
{
	// Clear all strategies
	RegisteredStrategies.Empty();
	
	// Reset state
	CollisionState.Reset();
	CurrentDetectionResult.Reset();
	
	bIsInitialized = false;
	
	UE_LOG(LogTemp, Log, TEXT("CameraCollisionResolver: Shutdown complete"));
}


//========================================
// Strategy Registration
//========================================

bool UCameraCollisionResolver::RegisterStrategy(UCameraCollisionBase* Strategy)
{
	if (!Strategy)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraCollisionResolver: Cannot register null strategy"));
		return false;
	}

	ECollisionStrategyID StrategyType = Strategy->GetStrategyType();

	if (StrategyType == ECollisionStrategyID::CollisionNone)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraCollisionResolver: Cannot register strategy with None type"));
		return false;
	}

	if (RegisteredStrategies.Contains(StrategyType))
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraCollisionResolver: Strategy type %s already registered"),
			*UEnum::GetValueAsString(StrategyType));
		return false;
	}

	RegisteredStrategies.Add(StrategyType, Strategy);
	
	UE_LOG(LogTemp, Verbose, TEXT("CameraCollisionResolver: Registered strategy %s [%s]"),
		*Strategy->GetStrategyName().ToString(),
		*UEnum::GetValueAsString(Strategy->GetStrategyCategory()));

	return true;
}

bool UCameraCollisionResolver::UnregisterStrategy(ECollisionStrategyID StrategyType)
{
	if (StrategyType == ECollisionStrategyID::CollisionNone)
	{
		return false;
	}

	if (RegisteredStrategies.Contains(StrategyType))
	{
		RegisteredStrategies.Remove(StrategyType);
		UE_LOG(LogTemp, Verbose, TEXT("CameraCollisionResolver: Unregistered strategy %s"),
			*UEnum::GetValueAsString(StrategyType));
		return true;
	}
	
	return false;
}

UCameraCollisionBase* UCameraCollisionResolver::GetStrategy(ECollisionStrategyID StrategyType) const
{
	UCameraCollisionBase* const* FoundStrategy = RegisteredStrategies.Find(StrategyType);
	return FoundStrategy ? *FoundStrategy : nullptr;
}

TArray<UCameraCollisionBase*> UCameraCollisionResolver::GetAllStrategies() const
{
	TArray<UCameraCollisionBase*> Result;
	RegisteredStrategies.GenerateValueArray(Result);
	return Result;
}

TArray<UCameraCollisionBase*> UCameraCollisionResolver::GetStrategiesByCategory(ECollisionCategory Category) const
{
	TArray<UCameraCollisionBase*> Result;

	for (const auto& Pair : RegisteredStrategies)
	{
		if (Pair.Value && Pair.Value->GetStrategyCategory() == Category)
		{
			Result.Add(Pair.Value);
		}
	}

	// Sort by priority
	Result.Sort([](const UCameraCollisionBase& A, const UCameraCollisionBase& B)
	{
		return A.GetPriority() > B.GetPriority();
	});

	return Result;
}


//========================================
// Collision Resolution (Main Entry Point)
//========================================

void UCameraCollisionResolver::ResolveCollisions(FStageExecutionContext& Context)
{
	// ★★★ 诊断日志开始 ★★★
	bool bShouldLog = false;
	
#if WITH_EDITOR
	if (CameraManager.IsValid())
	{
		bShouldLog = CameraManager->IsCollisionDebugEnabled();
	}
#endif

	// 如果启用了调试，限制前 100 帧输出（避免无限刷屏）
	static int32 ResolverDiagCount = 0;
	if (bShouldLog && ResolverDiagCount < 100)
	{
		ResolverDiagCount++;
	}
	else
	{
		bShouldLog = false;
	}
	
	float DistanceAtStart = Context.Output.Distance;
	
	if (bShouldLog)
	{
		UE_LOG(LogTemp, Error, TEXT(""));
		UE_LOG(LogTemp, Error, TEXT("  ┌─ CollisionResolver::ResolveCollisions 诊断 #%d ─┐"), ResolverDiagCount);
		UE_LOG(LogTemp, Error, TEXT("  │ 输入 Distance: %.1f"), DistanceAtStart);
		UE_LOG(LogTemp, Error, TEXT("  │ bIsInitialized: %s"), bIsInitialized ? TEXT("YES") : TEXT("NO"));
	}
	// ★★★ 诊断日志结束 ★★★

	if (!bIsInitialized)
	{
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ ⚠️ 未初始化，直接返回"));
			UE_LOG(LogTemp, Error, TEXT("  └────────────────────────────────────────────┘"));
		}
		return;
	}

	// Store previous result for comparison
	PreviousDetectionResult = CurrentDetectionResult;

	// Phase 1: Detection
	if (bShouldLog)
	{
		UE_LOG(LogTemp, Error, TEXT("  │ Phase 1: PerformDetection"));
	}
	CurrentDetectionResult = PerformDetection(Context);
	if (bShouldLog)
	{
		UE_LOG(LogTemp, Error, TEXT("  │   bCollisionDetected: %s"), CurrentDetectionResult.bCollisionDetected ? TEXT("YES") : TEXT("NO"));
		UE_LOG(LogTemp, Error, TEXT("  │   SafeDistance: %.1f"), CurrentDetectionResult.SafeDistance);
		UE_LOG(LogTemp, Error, TEXT("  │   DesiredDistance: %.1f"), CurrentDetectionResult.DesiredDistance);
	}

	// Update collision state based on detection
	UpdateCollisionState(Context.DeltaTime, CurrentDetectionResult);

	// Collect all responses
	TArray<FCollisionResponseOutput> AllResponses;

	// Phase 2: Response (only if collision detected)
	if (CurrentDetectionResult.bCollisionDetected)
	{
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ Phase 2: ExecuteResponseStrategies (有碰撞)"));
			UE_LOG(LogTemp, Error, TEXT("  │   调用前 Distance: %.1f"), Context.Output.Distance);
		}
		ExecuteResponseStrategies(Context, CurrentDetectionResult, AllResponses);
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │   收集到 %d 个响应"), AllResponses.Num());
		}
	}
	else
	{
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ Phase 2: 跳过 (无碰撞)"));
		}
	}

	// ★★★ 持续碰撞状态诊断 ★★★
#if WITH_EDITOR
	bool bCollisionStateLog = false;
	if (CameraManager.IsValid())
	{
		bCollisionStateLog = CameraManager->IsCollisionDebugEnabled();
	}
	
	if (bCollisionStateLog && CurrentDetectionResult.bCollisionDetected)
	{
		UE_LOG(LogTemp, Warning, TEXT("[碰撞状态] bCollision: YES, SafeDist: %.1f, DesiredDist: %.1f, Responses: %d"),
			CurrentDetectionResult.SafeDistance,
			CurrentDetectionResult.DesiredDistance,
			AllResponses.Num());
			
		for (int32 i = 0; i < AllResponses.Num(); ++i)
		{
			UE_LOG(LogTemp, Warning, TEXT("  Response[%d]: DistAdj=%.1f, Weight=%.2f"),
				i, AllResponses[i].DistanceAdjustment, AllResponses[i].Weight);
		}
	}
#endif

	// Phase 3: Occlusion
	if (CurrentDetectionResult.HasOcclusion())
	{
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ Phase 3: ExecuteOcclusionStrategies"));
		}
		ExecuteOcclusionStrategies(Context, CurrentDetectionResult);
	}

	// Phase 4: Recovery (only if recovering)
	if (CollisionState.bIsRecovering)
	{
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ Phase 4: ExecuteRecoveryStrategies (正在恢复)"));
			UE_LOG(LogTemp, Error, TEXT("  │   调用前 Distance: %.1f"), Context.Output.Distance);
		}
		ExecuteRecoveryStrategies(Context, Context.DeltaTime);
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │   调用后 Distance: %.1f"), Context.Output.Distance);
		}
	}

	// Phase 5: Special (check for special environments)
	if (CurrentDetectionResult.IsInSpecialEnvironment())
	{
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ Phase 5: ExecuteSpecialStrategies"));
		}
		ExecuteSpecialStrategies(Context, CurrentDetectionResult, AllResponses);
	}

	// Blend and apply all responses
	if (AllResponses.Num() > 0)
	{
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ 混合并应用 %d 个响应"), AllResponses.Num());
			UE_LOG(LogTemp, Error, TEXT("  │   应用前 Distance: %.1f"), Context.Output.Distance);
		}
		
		FCollisionResponseOutput BlendedResponse;
		BlendResponses(AllResponses, BlendedResponse);
		
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("  │   BlendedResponse.DistanceAdjustment: %.1f"), BlendedResponse.DistanceAdjustment);
		}
		
		// ★ Hybrid Mode C：只有混合后有实际调整才应用 ★
		if (BlendedResponse.HasAdjustments())
		{
			ApplyResponse(Context, BlendedResponse);
			
			if (bShouldLog)
			{
				UE_LOG(LogTemp, Error, TEXT("  │   应用后 Distance: %.1f"), Context.Output.Distance);
			}
		}
		else
		{
			if (bShouldLog)
			{
				UE_LOG(LogTemp, Error, TEXT("  │   ✓ 跳过应用（混合后无调整，Hybrid Mode C）"));
			}
		}
	}

	// Debug visualization
	if (Config.bDrawDebug)
	{
		UWorld* World = CameraManager.IsValid() ? CameraManager->GetWorld() : nullptr;
		DrawDebug(World);
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("CollisionResolver: Colliding=%d, Recovering=%d, CharOccluded=%d, TargetOccluded=%d"),
		CollisionState.bIsColliding,
		CollisionState.bIsRecovering,
		CollisionState.bCharacterOccluded,
		CollisionState.bTargetOccluded);

	// ★★★ 最终诊断日志 ★★★
	if (bShouldLog)
	{
		float DistanceAtEnd = Context.Output.Distance;
		UE_LOG(LogTemp, Error, TEXT("  │ 输出 Distance: %.1f"), DistanceAtEnd);
		UE_LOG(LogTemp, Error, TEXT("  │ 总变化: %.1f -> %.1f (差异: %.1f)"), 
			DistanceAtStart, DistanceAtEnd, DistanceAtEnd - DistanceAtStart);
		if (FMath::Abs(DistanceAtStart - DistanceAtEnd) > 1.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("  │ ⚠️ CollisionResolver 修改了距离！"));
		}
		UE_LOG(LogTemp, Error, TEXT("  └────────────────────────────────────────────┘"));
	}
}

void UCameraCollisionResolver::Update(float DeltaTime, const FStageExecutionContext& Context)
{
	if (!bIsInitialized)
	{
		return;
	}

	// Update all strategies
	for (auto& Pair : RegisteredStrategies)
	{
		if (Pair.Value)
		{
			Pair.Value->Update(DeltaTime, Context);
		}
	}

	// Update occlusion check timer
	TimeSinceOcclusionCheck += DeltaTime;

	// Update recovery if active
	if (CollisionState.bIsRecovering)
	{
		UpdateRecovery(DeltaTime);
	}
}


//========================================
// Detection
//========================================

FCollisionDetectionResult UCameraCollisionResolver::PerformDetection(const FStageExecutionContext& Context)
{
	FCollisionDetectionResult Result;
	Result.Reset();

	// Store desired values
	Result.DesiredDistance = Context.Output.Distance;
	Result.DesiredLocation = Context.Output.FocusPoint - 
		Context.Output.Rotation.Vector() * Context.Output.Distance;

	// Execute detection strategies
	ExecuteDetectionStrategies(Context, Result);

	// Check occlusion (at configured interval)
	if (TimeSinceOcclusionCheck >= Config.OcclusionCheckInterval)
	{
		if (Config.bCheckCharacterOcclusion)
		{
			Result.bCharacterOccluded = IsCharacterOccluded(Context);
		}
		
		if (Config.bCheckTargetOcclusion && Context.InputContext.bHasTarget)
		{
			Result.bTargetOccluded = IsTargetOccluded(Context);
		}

		TimeSinceOcclusionCheck = 0.0f;
	}
	else
	{
		// Use previous occlusion state
		Result.bCharacterOccluded = CollisionState.bCharacterOccluded;
		Result.bTargetOccluded = CollisionState.bTargetOccluded;
	}

	return Result;
}

bool UCameraCollisionResolver::IsCharacterOccluded(const FStageExecutionContext& Context) const
{
	UWorld* World = CameraManager.IsValid() ? CameraManager->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	// Calculate camera location
	FVector CameraLocation = Context.Output.FocusPoint - 
		Context.Output.Rotation.Vector() * Context.Output.Distance;
	
	// Character location (look at torso, not feet)
	FVector CharacterLocation = Context.InputContext.CharacterLocation;
	CharacterLocation.Z += 100.0f;

	// Setup trace params
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	
	// Ignore the character
	if (AActor* Owner = CameraManager.IsValid() ? CameraManager->GetOwner() : nullptr)
	{
		Params.AddIgnoredActor(Owner);
	}

	// Perform trace
	FHitResult Hit;
	bool bHit = World->LineTraceSingleByChannel(
		Hit,
		CameraLocation,
		CharacterLocation,
		ECC_Visibility,
		Params
	);

	return bHit;
}

bool UCameraCollisionResolver::IsTargetOccluded(const FStageExecutionContext& Context) const
{
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}

	UWorld* World = CameraManager.IsValid() ? CameraManager->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	// Calculate camera location
	FVector CameraLocation = Context.Output.FocusPoint - 
		Context.Output.Rotation.Vector() * Context.Output.Distance;
	
	// Target location (center of target)
	FVector TargetLocation = Context.InputContext.TargetLocation;
	TargetLocation.Z += 100.0f;  // Approximate center

	// Setup trace params
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	
	// Ignore both character and target
	if (AActor* Owner = CameraManager.IsValid() ? CameraManager->GetOwner() : nullptr)
	{
		Params.AddIgnoredActor(Owner);
	}
	if (Context.InputContext.TargetActor.IsValid())
	{
		Params.AddIgnoredActor(Context.InputContext.TargetActor.Get());
	}

	// Perform trace
	FHitResult Hit;
	bool bHit = World->LineTraceSingleByChannel(
		Hit,
		CameraLocation,
		TargetLocation,
		ECC_Visibility,
		Params
	);

	return bHit;
}


//========================================
// Phase Execution
//========================================

void UCameraCollisionResolver::ExecuteDetectionStrategies(
	const FStageExecutionContext& Context,
	FCollisionDetectionResult& OutResult)
{
	TArray<UCameraCollisionBase*> DetectionStrategies = GetStrategiesByCategory(ECollisionCategory::Detection);

	for (UCameraCollisionBase* Strategy : DetectionStrategies)
	{
		if (!Strategy || !Strategy->IsEnabled())
		{
			continue;
		}

		if (Strategy->ShouldExecute(Context))
		{
			FCollisionResponseOutput Response;
			if (Strategy->Execute(Context, OutResult, Response))
			{
				Strategy->SetActive(true);
				
				// First detection that finds collision wins
				if (OutResult.bCollisionDetected)
				{
					UE_LOG(LogTemp, Verbose, TEXT("CollisionResolver: Detection by %s - Distance=%.1f"),
						*Strategy->GetStrategyName().ToString(),
						OutResult.SafeDistance);
					break;
				}
			}
		}
		else
		{
			Strategy->SetActive(false);
		}
	}
}

void UCameraCollisionResolver::ExecuteResponseStrategies(
	const FStageExecutionContext& Context,
	const FCollisionDetectionResult& DetectionResult,
	TArray<FCollisionResponseOutput>& OutResponses)
{
	TArray<UCameraCollisionBase*> ResponseStrategies = GetStrategiesByCategory(ECollisionCategory::Response);

	for (UCameraCollisionBase* Strategy : ResponseStrategies)
	{
		if (!Strategy || !Strategy->IsEnabled())
		{
			continue;
		}

		if (Strategy->ShouldExecute(Context))
		{
			FCollisionResponseOutput Response;
			if (Strategy->Execute(Context, DetectionResult, Response))
			{
				Strategy->SetActive(true);
				OutResponses.Add(Response);
				
				UE_LOG(LogTemp, VeryVerbose, TEXT("CollisionResolver: Response from %s - DistAdj=%.1f"),
					*Strategy->GetStrategyName().ToString(),
					Response.DistanceAdjustment);
			}
		}
		else
		{
			Strategy->SetActive(false);
		}
	}
}

void UCameraCollisionResolver::ExecuteOcclusionStrategies(
	const FStageExecutionContext& Context,
	FCollisionDetectionResult& DetectionResult)
{
	TArray<UCameraCollisionBase*> OcclusionStrategies = GetStrategiesByCategory(ECollisionCategory::Occlusion);

	for (UCameraCollisionBase* Strategy : OcclusionStrategies)
	{
		if (!Strategy || !Strategy->IsEnabled())
		{
			continue;
		}

		if (Strategy->ShouldExecute(Context))
		{
			FCollisionResponseOutput Response;
			if (Strategy->Execute(Context, DetectionResult, Response))
			{
				Strategy->SetActive(true);
				
				// Apply occlusion response immediately
				// (occlusion handlers typically set actor visibility/materials)
				
				UE_LOG(LogTemp, VeryVerbose, TEXT("CollisionResolver: Occlusion handled by %s"),
					*Strategy->GetStrategyName().ToString());
			}
		}
		else
		{
			Strategy->SetActive(false);
		}
	}
}

void UCameraCollisionResolver::ExecuteRecoveryStrategies(
	FStageExecutionContext& Context,
	float DeltaTime)
{
	TArray<UCameraCollisionBase*> RecoveryStrategies = GetStrategiesByCategory(ECollisionCategory::Recovery);

	for (UCameraCollisionBase* Strategy : RecoveryStrategies)
	{
		if (!Strategy || !Strategy->IsEnabled())
		{
			continue;
		}

		if (Strategy->ShouldExecute(Context))
		{
			// Pass empty detection result for recovery
			FCollisionDetectionResult EmptyResult;
			EmptyResult.DesiredDistance = PreCollisionDistance;
			EmptyResult.DesiredLocation = PreCollisionPosition;

			FCollisionResponseOutput Response;
			if (Strategy->Execute(Context, EmptyResult, Response))
			{
				Strategy->SetActive(true);
				
				// Apply recovery response
				ApplyResponse(Context, Response);
				
				UE_LOG(LogTemp, VeryVerbose, TEXT("CollisionResolver: Recovery by %s - Progress=%.2f"),
					*Strategy->GetStrategyName().ToString(),
					CollisionState.RecoveryProgress);
			}
		}
		else
		{
			Strategy->SetActive(false);
		}
	}
}

void UCameraCollisionResolver::ExecuteSpecialStrategies(
	const FStageExecutionContext& Context,
	const FCollisionDetectionResult& DetectionResult,
	TArray<FCollisionResponseOutput>& OutResponses)
{
	TArray<UCameraCollisionBase*> SpecialStrategies = GetStrategiesByCategory(ECollisionCategory::Special);

	for (UCameraCollisionBase* Strategy : SpecialStrategies)
	{
		if (!Strategy || !Strategy->IsEnabled())
		{
			continue;
		}

		if (Strategy->ShouldExecute(Context))
		{
			FCollisionResponseOutput Response;
			if (Strategy->Execute(Context, DetectionResult, Response))
			{
				Strategy->SetActive(true);
				OutResponses.Add(Response);
				
				UE_LOG(LogTemp, VeryVerbose, TEXT("CollisionResolver: Special handling by %s"),
					*Strategy->GetStrategyName().ToString());
			}
		}
		else
		{
			Strategy->SetActive(false);
		}
	}
}


//========================================
// Response Processing
//========================================

void UCameraCollisionResolver::BlendResponses(
	const TArray<FCollisionResponseOutput>& Responses,
	FCollisionResponseOutput& OutBlended)
{
	if (Responses.Num() == 0)
	{
		return;
	}

	// Sort by priority (higher first)
	TArray<FCollisionResponseOutput> SortedResponses = Responses;
	SortedResponses.Sort([](const FCollisionResponseOutput& A, const FCollisionResponseOutput& B)
	{
		return A.Priority > B.Priority;
	});

	// Check for override responses
	for (const FCollisionResponseOutput& Response : SortedResponses)
	{
		if (Response.bOverride)
		{
			OutBlended = Response;
			return;
		}
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	for (const FCollisionResponseOutput& Response : SortedResponses)
	{
		TotalWeight += Response.Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		OutBlended = SortedResponses[0];
		return;
	}

	// Weighted blend
	OutBlended = FCollisionResponseOutput();
	
	for (const FCollisionResponseOutput& Response : SortedResponses)
	{
		float NormalizedWeight = Response.Weight / TotalWeight;
		
		OutBlended.PositionAdjustment += Response.PositionAdjustment * NormalizedWeight;
		OutBlended.DistanceAdjustment += Response.DistanceAdjustment * NormalizedWeight;
		OutBlended.RotationAdjustment.Pitch += Response.RotationAdjustment.Pitch * NormalizedWeight;
		OutBlended.RotationAdjustment.Yaw += Response.RotationAdjustment.Yaw * NormalizedWeight;
		OutBlended.RotationAdjustment.Roll += Response.RotationAdjustment.Roll * NormalizedWeight;
		OutBlended.FOVAdjustment += Response.FOVAdjustment * NormalizedWeight;
	}

	// Use highest priority for blend time
	OutBlended.BlendTime = SortedResponses[0].BlendTime;
	OutBlended.Priority = SortedResponses[0].Priority;
}

void UCameraCollisionResolver::ApplyResponse(
	FStageExecutionContext& Context,
	const FCollisionResponseOutput& Response)
{
	// ★★★ 诊断日志 ★★★
	bool bShouldLog = false;
	
#if WITH_EDITOR
	if (CameraManager.IsValid())
	{
		bShouldLog = CameraManager->IsCollisionDebugEnabled();
	}
	
	// 限制输出次数
	static int32 ApplyResponseDiagCount = 0;
	if (bShouldLog && ApplyResponseDiagCount < 100)
	{
		ApplyResponseDiagCount++;
	}
	else
	{
		bShouldLog = false;
	}
#endif
	
	float DistanceBefore = Context.Output.Distance;
	
	if (!Response.HasAdjustments())
	{
#if WITH_EDITOR
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Error, TEXT("      ApplyResponse: 无调整，直接返回"));
		}
#endif
		return;
	}

#if WITH_EDITOR
	if (bShouldLog)
	{
		UE_LOG(LogTemp, Error, TEXT("      ApplyResponse 诊断 #%d:"), ApplyResponseDiagCount);
		UE_LOG(LogTemp, Error, TEXT("        输入 Distance: %.1f"), DistanceBefore);
		UE_LOG(LogTemp, Error, TEXT("        Response.DistanceAdjustment: %.1f"), Response.DistanceAdjustment);
		UE_LOG(LogTemp, Error, TEXT("        Config.MinDistanceFromTarget: %.1f"), Config.MinDistanceFromTarget);
	}
#endif

	// Apply position adjustment
	Context.Output.FocusPoint += Response.PositionAdjustment;

	// Apply distance adjustment (negative = closer)
	float NewDistance = Context.Output.Distance + Response.DistanceAdjustment;
	Context.Output.Distance = FMath::Max(
		Config.MinDistanceFromTarget,
		NewDistance
	);

#if WITH_EDITOR
	if (bShouldLog)
	{
		UE_LOG(LogTemp, Error, TEXT("        计算: %.1f + %.1f = %.1f"), DistanceBefore, Response.DistanceAdjustment, NewDistance);
		UE_LOG(LogTemp, Error, TEXT("        Clamp后: %.1f"), Context.Output.Distance);
		UE_LOG(LogTemp, Error, TEXT("        输出 Distance: %.1f (变化: %.1f)"), 
			Context.Output.Distance, Context.Output.Distance - DistanceBefore);
	}
#endif

	// Apply rotation adjustment
	Context.Output.Rotation.Pitch += Response.RotationAdjustment.Pitch;
	Context.Output.Rotation.Yaw += Response.RotationAdjustment.Yaw;
	Context.Output.Rotation.Roll += Response.RotationAdjustment.Roll;

	// Apply FOV adjustment
	Context.Output.FOV += Response.FOVAdjustment;

	// ★ Hybrid Mode C 修复：只有实际产生调整时才标记碰撞状态 ★
	// 检查是否有任何实际的调整发生
	bool bHasActualAdjustment = 
		!Response.PositionAdjustment.IsNearlyZero() ||
		FMath::Abs(Response.DistanceAdjustment) > 0.1f ||
		FMath::Abs(Response.RotationAdjustment.Pitch) > 0.1f ||
		FMath::Abs(Response.RotationAdjustment.Yaw) > 0.1f ||
		FMath::Abs(Response.RotationAdjustment.Roll) > 0.1f ||
		FMath::Abs(Response.FOVAdjustment) > 0.1f;
	
	// 只有实际產生调整时才标记
	if (bHasActualAdjustment)
	{
		Context.Output.bCollisionAdjusted = true;
		
#if WITH_EDITOR
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("        ★ 标记 bCollisionAdjusted = true（有实际调整）"));
		}
#endif
	}
	else
	{
#if WITH_EDITOR
		if (bShouldLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("        ✓ 跳过标记（无实际调整，Hybrid Mode C）"));
		}
#endif
	}
}


void UCameraCollisionResolver::DrawDebug(const UWorld* World) const
{
#if ENABLE_DRAW_DEBUG
	if (!World || !Config.bDrawDebug)
	{
		return;
	}

	float Duration = Config.DebugDrawDuration;

	// Draw collision state indicator
	if (CollisionState.bIsColliding)
	{
		// Red sphere at safe location when colliding
		DrawDebugSphere(World, CurrentDetectionResult.SafeLocation, 15.0f, 8, 
			FColor::Red, false, Duration, 0, 2.0f);
		
		// Line from desired to safe location
		if (!CurrentDetectionResult.DesiredLocation.IsZero() && 
			!CurrentDetectionResult.SafeLocation.IsZero())
		{
			DrawDebugLine(World,
				CurrentDetectionResult.DesiredLocation,
				CurrentDetectionResult.SafeLocation,
				FColor::Orange, false, Duration, 0, 1.5f);
		}
		
		// Draw collision point
		if (CurrentDetectionResult.HitResult.bBlockingHit)
		{
			DrawDebugSphere(World, CurrentDetectionResult.HitResult.ImpactPoint, 
				8.0f, 6, FColor::Yellow, false, Duration, 0, 1.0f);
		}
	}
	else if (CollisionState.bIsRecovering)
	{
		// Yellow sphere when recovering
		DrawDebugSphere(World, PreCollisionPosition, 10.0f, 8, 
			FColor::Yellow, false, Duration, 0, 1.0f);
		
		// Draw recovery progress
		FVector RecoveryIndicator = PreCollisionPosition + FVector::UpVector * 50.0f;
		DrawDebugString(World, RecoveryIndicator, 
			FString::Printf(TEXT("Recovery: %.0f%%"), CollisionState.RecoveryProgress * 100.0f),
			nullptr, FColor::Yellow, Duration, false, 1.0f);
	}

	// Draw collision normal if hit detected
	if (CurrentDetectionResult.bCollisionDetected && 
		!CurrentDetectionResult.CollisionNormal.IsNearlyZero())
	{
		FVector ImpactPoint = CurrentDetectionResult.HitResult.ImpactPoint;
		DrawDebugDirectionalArrow(World,
			ImpactPoint,
			ImpactPoint + CurrentDetectionResult.CollisionNormal * 50.0f,
			20.0f, FColor::Blue, false, Duration, 0, 1.0f);
	}

	// Draw distance info
	if (CurrentDetectionResult.bCollisionDetected)
	{
		FVector InfoLocation = CurrentDetectionResult.SafeLocation + FVector::UpVector * 30.0f;
		FString DistanceInfo = FString::Printf(TEXT("Safe: %.1f / Desired: %.1f"), 
			CurrentDetectionResult.SafeDistance, 
			CurrentDetectionResult.DesiredDistance);
		DrawDebugString(World, InfoLocation, DistanceInfo, nullptr, FColor::White, Duration, false, 1.0f);
	}

	// Draw active strategy indicators
	FVector StrategyInfoBase = FVector::ZeroVector;
	if (CameraManager.IsValid() && CameraManager->GetOwner())
	{
		StrategyInfoBase = CameraManager->GetOwner()->GetActorLocation() + FVector(0, 0, 200);
	}
	
	int32 ActiveCount = 0;
	for (const auto& Pair : RegisteredStrategies)
	{
		if (Pair.Value && Pair.Value->IsActive())
		{
			ActiveCount++;
		}
	}
	
	if (ActiveCount > 0 && !StrategyInfoBase.IsZero())
	{
		FString ActiveInfo = FString::Printf(TEXT("Active Strategies: %d"), ActiveCount);
		DrawDebugString(World, StrategyInfoBase, ActiveInfo, nullptr, FColor::Green, Duration, false, 1.0f);
	}
#endif
}

//========================================
// State Management
//========================================

void UCameraCollisionResolver::UpdateCollisionState(float DeltaTime, const FCollisionDetectionResult& DetectionResult)
{
	bool bWasColliding = CollisionState.bIsColliding;

	// Update collision state
	CollisionState.bIsColliding = DetectionResult.bCollisionDetected;
	CollisionState.bCharacterOccluded = DetectionResult.bCharacterOccluded;
	CollisionState.bTargetOccluded = DetectionResult.bTargetOccluded;

	if (CollisionState.bIsColliding)
	{
		// Colliding
		CollisionState.CollisionTime += DeltaTime;
		CollisionState.TimeSinceCollision = 0.0f;
		CollisionState.bIsRecovering = false;
		CollisionState.RecoveryProgress = 0.0f;
		CollisionState.CurrentDistance = DetectionResult.SafeDistance;
		CollisionState.DesiredDistance = DetectionResult.DesiredDistance;

		// Store pre-collision state on first collision frame
		if (!bWasColliding)
		{
			PreCollisionDistance = DetectionResult.DesiredDistance;
			PreCollisionPosition = DetectionResult.DesiredLocation;
			
			UE_LOG(LogTemp, Verbose, TEXT("CollisionResolver: Collision started, PreDistance=%.1f"),
				PreCollisionDistance);
		}
	}
	else
	{
		// Not colliding
		CollisionState.CollisionTime = 0.0f;
		CollisionState.TimeSinceCollision += DeltaTime;

		// Start recovery if was colliding
		if (bWasColliding && !CollisionState.bIsRecovering)
		{
			BeginRecovery();
		}
	}
}

void UCameraCollisionResolver::BeginRecovery()
{
	CollisionState.bIsRecovering = true;
	CollisionState.RecoveryProgress = 0.0f;
	
	UE_LOG(LogTemp, Verbose, TEXT("CollisionResolver: Recovery started"));
}

void UCameraCollisionResolver::UpdateRecovery(float DeltaTime)
{
	if (!CollisionState.bIsRecovering)
	{
		return;
	}

	// Apply recovery delay
	if (CollisionState.TimeSinceCollision < Config.RecoveryDelay)
	{
		return;
	}

	// Update recovery progress
	float RecoveryDelta = DeltaTime * Config.RecoverySpeed;
	CollisionState.RecoveryProgress = FMath::Min(1.0f, CollisionState.RecoveryProgress + RecoveryDelta);

	// Check if recovery complete
	if (CollisionState.RecoveryProgress >= 1.0f)
	{
		CollisionState.bIsRecovering = false;
		CollisionState.RecoveryProgress = 1.0f;
		
		UE_LOG(LogTemp, Verbose, TEXT("CollisionResolver: Recovery complete"));
	}
}


//========================================
// Strategy Creation
//========================================

void UCameraCollisionResolver::CreateAllStrategies(UObject* Outer)
{
	UE_LOG(LogTemp, Log, TEXT("CameraCollisionResolver: Creating all 20 collision strategies..."));

	// ★★★ 辅助 Lambda：注册并初始化策略 ★★★
	auto InitAndRegisterStrategy = [this](UCameraCollisionBase* Strategy) -> bool
	{
		if (Strategy)
		{
			// 设置 OwnerCameraManager 引用
			Strategy->SetOwnerCameraManager(CameraManager.Get());
			return RegisterStrategy(Strategy);
		}
		return false;
	};

	//========================================
	// Detection Strategies (D01-D04) - 4 strategies
	//========================================
	// 
	// ★ 当前状态：默认禁用 ★
	// 
	// 原因：Hybrid Mode C 下使用 SpringArm 原生碰撞，
	//       SpringArm 会自己处理碰撞检测和拉近。
	//       自定义 Detection 策略的检测结果与 SpringArm 不同步，
	//       会导致碰撞状态混乱和相机跳动。
	// 
	// 将来可启用的场景：
	// - 完全禁用 SpringArm 原生碰撞时（bDoCollisionTest = false）
	// - 需要自定义碰撞检测逻辑时（如多射线、球形检测）
	// - 需要碰撞信息用于其他高级功能时
	// 
	// 启用方法：
	//   SoulsCameraManager->SetCollisionStrategyEnabled(
	//       ECollisionStrategyID::Collision_D01_SingleRay, true);
	// 
	//========================================
	
	// 创建 Detection 策略但默认禁用
	{
		UCameraCollision_D01_SingleRay* D01 = NewObject<UCameraCollision_D01_SingleRay>(Outer);
		if (D01)
		{
			D01->SetEnabled(false);  // 默认禁用
			D01->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(D01);
		}
		
		UCameraCollision_D02_SphereSweep* D02 = NewObject<UCameraCollision_D02_SphereSweep>(Outer);
		if (D02)
		{
			D02->SetEnabled(false);  // 默认禁用
			D02->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(D02);
		}
		
		UCameraCollision_D03_MultiRay* D03 = NewObject<UCameraCollision_D03_MultiRay>(Outer);
		if (D03)
		{
			D03->SetEnabled(false);  // 默认禁用
			D03->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(D03);
		}
		
		UCameraCollision_D04_MultiSphereSweep* D04 = NewObject<UCameraCollision_D04_MultiSphereSweep>(Outer);
		if (D04)
		{
			D04->SetEnabled(false);  // 默认禁用
			D04->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(D04);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("  Detection strategies registered: 4 (DISABLED by default for Hybrid Mode C)"));

	//========================================
	// Response Strategies (RS01-RS05) - 5 strategies
	//========================================
	// 
	// ★ 当前状态：默认禁用 ★
	// 
	// 原因：Hybrid Mode C 下使用 SpringArm 原生碰撞处理拉近。
	//       Detection 已禁用，Response 也无需执行。
	// 
	// 将来可启用的场景：
	// - 需要滑动绕行（RS02/RS03）代替直接拉近
	// - 需要 FOV 补偿（RS04）来保持视野
	// - 需要瞬时切换（RS05）用于特殊场景
	// 
	//========================================
	
	// 创建 Response 策略但默认禁用
	{
		UCameraCollision_RS01_PullIn* RS01 = NewObject<UCameraCollision_RS01_PullIn>(Outer);
		if (RS01)
		{
			RS01->SetEnabled(false);  // 默认禁用
			RS01->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RS01);
		}
		
		UCameraCollision_RS02_Slide* RS02 = NewObject<UCameraCollision_RS02_Slide>(Outer);
		if (RS02)
		{
			RS02->SetEnabled(false);  // 默认禁用
			RS02->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RS02);
		}
		
		UCameraCollision_RS03_Orbit* RS03 = NewObject<UCameraCollision_RS03_Orbit>(Outer);
		if (RS03)
		{
			RS03->SetEnabled(false);  // 默认禁用
			RS03->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RS03);
		}
		
		UCameraCollision_RS04_FOVCompensate* RS04 = NewObject<UCameraCollision_RS04_FOVCompensate>(Outer);
		if (RS04)
		{
			RS04->SetEnabled(false);  // 默认禁用
			RS04->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RS04);
		}
		
		UCameraCollision_RS05_InstantSnap* RS05 = NewObject<UCameraCollision_RS05_InstantSnap>(Outer);
		if (RS05)
		{
			RS05->SetEnabled(false);  // 默认禁用
			RS05->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RS05);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("  Response strategies registered: 5 (DISABLED by default for Hybrid Mode C)"));

	//========================================
	// Occlusion Strategies (OC01-OC04) - 4 strategies
	//========================================
	// 
	// ★ 当前状态：默认禁用 ★
	// 
	// 原因：Hybrid Mode C 下使用 SpringArm 原生碰撞，
	//       SpringArm 会自动处理碰撞后的距离恢复。
	//       自定义 Recovery 策略会与之冲突，导致相机跳动。
	// 
	// 将来可启用的场景：
	// - 完全禁用 SpringArm 原生碰撞时（bDoCollisionTest = false）
	// - 需要自定义恢复曲线/速度时
	// - 特殊状态下需要不同的恢复行为时
	// 
	// 启用方法：
	//   SoulsCameraManager->SetCollisionStrategyEnabled(
	//       ECollisionStrategyID::Collision_RC01_DelayedRecovery, true);
	// 
	//========================================
	
	// 创建 Recovery 策略但默认禁用
	{
		UCameraCollision_RC01_DelayedRecovery* RC01 = NewObject<UCameraCollision_RC01_DelayedRecovery>(Outer);
		if (RC01)
		{
			RC01->SetEnabled(false);  // 默认禁用
			RC01->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RC01);
		}
		
		UCameraCollision_RC02_SmoothRecovery* RC02 = NewObject<UCameraCollision_RC02_SmoothRecovery>(Outer);
		if (RC02)
		{
			RC02->SetEnabled(false);  // 默认禁用
			RC02->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RC02);
		}
		
		UCameraCollision_RC03_StepRecovery* RC03 = NewObject<UCameraCollision_RC03_StepRecovery>(Outer);
		if (RC03)
		{
			RC03->SetEnabled(false);  // 默认禁用
			RC03->SetOwnerCameraManager(CameraManager.Get());
			RegisterStrategy(RC03);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("  Recovery strategies registered: 3 (DISABLED by default for Hybrid Mode C)"));

	//========================================
	// Special Strategies (SP01-SP04) - 4 strategies
	//========================================
	
	InitAndRegisterStrategy(NewObject<UCameraCollision_SP01_TightSpace>(Outer));
	InitAndRegisterStrategy(NewObject<UCameraCollision_SP02_LowCeiling>(Outer));
	InitAndRegisterStrategy(NewObject<UCameraCollision_SP03_CliffEdge>(Outer));
	InitAndRegisterStrategy(NewObject<UCameraCollision_SP04_CornerCase>(Outer));
	
	UE_LOG(LogTemp, Log, TEXT("  Special strategies registered: 4"));

	//========================================
	// Summary
	//========================================
	
	UE_LOG(LogTemp, Log, TEXT("CameraCollisionResolver: Registered %d strategies"),
		RegisteredStrategies.Num());
		
	// Final progress
	UE_LOG(LogTemp, Log, TEXT("  Detection:  4/4 complete"));
	UE_LOG(LogTemp, Log, TEXT("  Response:   5/5 complete"));
	UE_LOG(LogTemp, Log, TEXT("  Occlusion:  4/4 complete"));
	UE_LOG(LogTemp, Log, TEXT("  Recovery:   3/3 complete"));
	UE_LOG(LogTemp, Log, TEXT("  Special:    4/4 complete"));
	UE_LOG(LogTemp, Log, TEXT("  ================================"));
	UE_LOG(LogTemp, Log, TEXT("  TOTAL:      20/20 strategies"));
}
