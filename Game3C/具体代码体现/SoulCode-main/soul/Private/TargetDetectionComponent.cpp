// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetDetectionComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UTargetDetectionComponent::UTargetDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UTargetDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UTargetDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTargetDetectionComponent::SetDetectionSphere(USphereComponent* Sphere)
{
	LockOnDetectionSphere = Sphere;
}

void UTargetDetectionComponent::FindLockOnCandidates()
{
	UE_LOG(LogTemp, Error, TEXT("======== FindLockOnCandidates ========"));
	
	if (!LockOnDetectionSphere)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: LockOnDetectionSphere is NULL!"));
		UE_LOG(LogTemp, Error, TEXT("====================================="));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Sphere Valid: YES"));
	UE_LOG(LogTemp, Warning, TEXT("Sphere Radius: %.1f"), LockOnDetectionSphere->GetScaledSphereRadius());
	UE_LOG(LogTemp, Warning, TEXT("Owner Location: %s"), *GetOwner()->GetActorLocation().ToString());

	// Throttle searches for performance
	// Temporarily disable throttling to ensure search is executed every time
	// float CurrentTime = GetWorld()->GetTimeSeconds();
	// if (CurrentTime - LastTargetSearchTime < TargetSearchInterval)
	// {
	// 	return;
	// }
	// LastTargetSearchTime = CurrentTime;

	// Clear previous candidates
	LockOnCandidates.Empty();

	// Get all overlapping actors
	TArray<AActor*> AllOverlapping;
	LockOnDetectionSphere->GetOverlappingActors(AllOverlapping);
	UE_LOG(LogTemp, Warning, TEXT("All Overlapping Actors: %d"), AllOverlapping.Num());

	// Get only Pawn types
	TArray<AActor*> OverlappingPawns;
	LockOnDetectionSphere->GetOverlappingActors(OverlappingPawns, APawn::StaticClass());
	UE_LOG(LogTemp, Warning, TEXT("Overlapping Pawns: %d"), OverlappingPawns.Num());

	// Check each Pawn
	for (AActor* Actor : OverlappingPawns)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- Checking: %s ---"), *Actor->GetName());
		
		if (Actor == GetOwner())
		{
			UE_LOG(LogTemp, Warning, TEXT("  SKIP: Is Owner"));
			continue;
		}

		float Distance;
		if (bUseBoundsCenterForLargeEnemies && Actor->ActorHasTag(LargeEnemyTag))
		{
			Distance = GetDistanceToTargetBoundsEdge(Actor);
			UE_LOG(LogTemp, Warning, TEXT("  [LargeEnemy] BoundsEdge Distance: %.1f (Max: %.1f)"), Distance, LockOnRange);
		}
		else
		{
			Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("  Distance: %.1f (Max: %.1f)"), Distance, LockOnRange);
		}

		bool bValid = IsValidLockOnTarget(Actor);
		if (bValid)
		{
			UE_LOG(LogTemp, Warning, TEXT("  RESULT: VALID - Adding to candidates"));
			LockOnCandidates.Add(Actor);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  RESULT: INVALID - See IsValidLockOnTarget logs above"));
		}
	}

	UE_LOG(LogTemp, Error, TEXT("FINAL CANDIDATES COUNT: %d"), LockOnCandidates.Num());
	UE_LOG(LogTemp, Error, TEXT("====================================="));

	// ★ 补充搜索：大型敌人可能 Overlap 检测不到
	{
		TArray<AActor*> PreLargeSearch = LockOnCandidates;
		FindLargeEnemyCandidates(LockOnCandidates);

		// 对新增的大型敌人也做有效性检查
		for (int32 i = LockOnCandidates.Num() - 1; i >= 0; --i)
		{
			AActor* Candidate = LockOnCandidates[i];
			if (!PreLargeSearch.Contains(Candidate))
			{
				// 这是补充搜索新增的，需要验证有效性
				if (!IsValidLockOnTarget(Candidate))
				{
					LockOnCandidates.RemoveAt(i);
				}
			}
		}
	}

	// Sort by direction
	SortCandidatesByDirection(LockOnCandidates);

	// Broadcast update
	OnTargetsUpdated.Broadcast(LockOnCandidates);
}

bool UTargetDetectionComponent::IsValidLockOnTarget(AActor* Target) const
{
	UE_LOG(LogTemp, Log, TEXT("    [IsValidLockOnTarget] Checking: %s"), Target ? *Target->GetName() : TEXT("NULL"));

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: Target is NULL"));
		return false;
	}
	
	if (!IsValid(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: Target is not valid (IsValid failed)"));
		return false;
	}

	if (Target == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: Target is Owner"));
		return false;
	}

	if (Target->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: Target is pending kill"));
		return false;
	}

	// Check if friendly (you can add tag checking here)
	if (Target->ActorHasTag(FName("Friendly")))
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: Target has Friendly tag"));
		return false;
	}

	// Check distance
	float Distance;
	FVector TargetLoc;

	if (bUseBoundsCenterForLargeEnemies && Target->ActorHasTag(LargeEnemyTag))
	{
		// 大型敌人：用 Bounds 边缘距离判断是否在范围内
		Distance = GetDistanceToTargetBoundsEdge(Target);
		// 方向计算仍然用 Bounds 中心
		TargetLoc = GetTargetDetectionLocation(Target);
		UE_LOG(LogTemp, Warning, TEXT("    [LargeEnemy] Edge distance: %.1f, Range: %.1f"), Distance, LockOnRange);
	}
	else
	{
		TargetLoc = Target->GetActorLocation();
		Distance = FVector::Dist(GetOwner()->GetActorLocation(), TargetLoc);
	}

	if (Distance > LockOnRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: Distance %.1f > Range %.1f"), Distance, LockOnRange);
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("    PASS: Distance %.1f <= Range %.1f"), Distance, LockOnRange);

	// Check angle (horizontal projection only)
	FVector CameraForward = GetCameraForwardVector();
	FVector ToTarget = GetTargetDetectionLocation(Target) - GetOwner()->GetActorLocation();
	ToTarget.Z = 0.0f;
	ToTarget.Normalize();

	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	float AngleToTarget = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

	if (AngleToTarget > LockOnAngle / 2.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: Angle %.1f > MaxAngle %.1f"), AngleToTarget, LockOnAngle / 2.0f);
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("    PASS: Angle %.1f <= MaxAngle %.1f"), AngleToTarget, LockOnAngle / 2.0f);

	// ★ 垂直角度检测扩展 — 不影响现有水平检测
	if (bEnableVerticalDetection)
	{
		FVector ToTarget3D = Target->GetActorLocation() - GetOwner()->GetActorLocation();
		// 计算垂直角度：ToTarget3D 与水平面的夹角
		float HorizontalDist = FVector(ToTarget3D.X, ToTarget3D.Y, 0.0f).Size();
		float VerticalAngle = FMath::RadiansToDegrees(FMath::Atan2(FMath::Abs(ToTarget3D.Z), HorizontalDist));

		if (VerticalAngle > VerticalDetectionAngle)
		{
			UE_LOG(LogTemp, Log, TEXT("    REJECT: Vertical angle %.1f > Max %.1f"), VerticalAngle, VerticalDetectionAngle);
			return false;
		}
		UE_LOG(LogTemp, Log, TEXT("    PASS: Vertical angle %.1f <= Max %.1f"), VerticalAngle, VerticalDetectionAngle);
	}

	// Line of sight check
	bool bHasLineOfSight = PerformLineOfSightCheck(Target);
	if (!bHasLineOfSight)
	{
		UE_LOG(LogTemp, Warning, TEXT("    REJECT: No line of sight"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("    PASS: Has line of sight"));

	UE_LOG(LogTemp, Warning, TEXT("    ACCEPTED: All checks passed!"));
	return true;
}

bool UTargetDetectionComponent::IsTargetStillLockable(AActor* Target) const
{
	if (!Target || !IsValid(Target))
		return false;

	if (Target->IsPendingKill())
		return false;

	// Use extended range for maintaining lock
	float Distance;
	if (bUseBoundsCenterForLargeEnemies && Target->ActorHasTag(LargeEnemyTag))
	{
		Distance = GetDistanceToTargetBoundsEdge(Target);
	}
	else
	{
		Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	}

	if (Distance > LockOnRange * ExtendedLockRangeMultiplier)
		return false;

	// Skip angle validation - allow maintaining lock on targets outside view

	// Line of sight check
	if (!PerformLineOfSightCheck(Target))
		return false;

	return true;
}

AActor* UTargetDetectionComponent::TryGetSectorLockTarget()
{
	UE_LOG(LogTemp, Warning, TEXT("--- TryGetSectorLockTarget ---"));
	UE_LOG(LogTemp, Warning, TEXT("Candidates count: %d"), LockOnCandidates.Num());

	if (LockOnCandidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No candidates available!"));
		return nullptr;
	}

	// Filter candidates to those within sector lock angle (±30 degrees from camera)
	TArray<AActor*> SectorTargets;
	FVector CameraForward = GetCameraForwardVector();

	for (AActor* Candidate : LockOnCandidates)
	{
		FVector ToTarget = Candidate->GetActorLocation() - GetOwner()->GetActorLocation();
		ToTarget.Z = 0.0f;
		ToTarget.Normalize();

		float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
		float AngleToTarget = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

		UE_LOG(LogTemp, Log, TEXT("  %s: Angle=%.1f, SectorAngle=%.1f"), 
			*Candidate->GetName(), AngleToTarget, SectorLockAngle / 2.0f);

		if (AngleToTarget <= SectorLockAngle / 2.0f)
		{
			SectorTargets.Add(Candidate);
			UE_LOG(LogTemp, Log, TEXT("    -> In sector!"));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Sector targets: %d"), SectorTargets.Num());

	AActor* BestTarget = GetBestTargetFromList(SectorTargets);
	UE_LOG(LogTemp, Warning, TEXT("Best target: %s"), BestTarget ? *BestTarget->GetName() : TEXT("NONE"));
	
	return BestTarget;
}

AActor* UTargetDetectionComponent::GetBestTargetFromList(const TArray<AActor*>& Targets)
{
	if (Targets.Num() == 0)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector CameraForward = GetCameraForwardVector();

	for (AActor* Target : Targets)
	{
		// Calculate angle factor (0-1, higher is better)
		FVector ToTarget = Target->GetActorLocation() - OwnerLocation;
		ToTarget.Z = 0.0f;
		ToTarget.Normalize();

		float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
		float AngleFactor = (DotProduct + 1.0f) / 2.0f; // Map from [-1,1] to [0,1]

		// Calculate distance factor (0-1, higher is closer/better)
		float Distance = FVector::Dist(OwnerLocation, Target->GetActorLocation());
		float DistanceFactor = 1.0f - FMath::Sqrt(Distance / LockOnRange);
		DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

		// Combined score
		float Score = (AngleFactor * 0.7f) + (DistanceFactor * 0.3f);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

void UTargetDetectionComponent::SortCandidatesByDirection(TArray<AActor*>& Targets)
{
	Targets.Sort([this](const AActor& A, const AActor& B)
	{
		float AngleA = CalculateDirectionAngle(const_cast<AActor*>(&A));
		float AngleB = CalculateDirectionAngle(const_cast<AActor*>(&B));
		return AngleA < AngleB;
	});
}

float UTargetDetectionComponent::CalculateAngleToTarget(AActor* Target) const
{
	if (!Target)
		return 0.0f;

	FVector CameraForward = GetCameraForwardVector();
	FVector ToTarget = Target->GetActorLocation() - GetOwner()->GetActorLocation();
	ToTarget.Z = 0.0f; // Horizontal projection only
	ToTarget.Normalize();

	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	return FMath::RadiansToDegrees(FMath::Acos(DotProduct));
}

float UTargetDetectionComponent::CalculateDirectionAngle(AActor* Target) const
{
	if (!Target)
		return 0.0f;

	FVector CameraForward = GetCameraForwardVector();
	FVector CameraRight = FVector::CrossProduct(FVector::UpVector, CameraForward);
	CameraRight.Normalize();

	FVector ToTarget = Target->GetActorLocation() - GetOwner()->GetActorLocation();
	ToTarget.Z = 0.0f; // Horizontal projection only
	ToTarget.Normalize();

	float ForwardDot = FVector::DotProduct(CameraForward, ToTarget);
	float RightDot = FVector::DotProduct(CameraRight, ToTarget);

	// Atan2 gives us -180 to +180, negative=left, positive=right
	return FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
}

EEnemySizeCategory UTargetDetectionComponent::GetTargetSizeCategory(AActor* Target)
{
	if (!Target)
		return EEnemySizeCategory::Medium;

	// Check cache first
	if (EnemySizeCache.Contains(Target))
	{
		return EnemySizeCache[Target];
	}

	// Calculate size
	FVector Origin, BoxExtent;
	Target->GetActorBounds(false, Origin, BoxExtent);
	float MaxDimension = FMath::Max3(BoxExtent.X, BoxExtent.Y, BoxExtent.Z) * 2.0f;

	EEnemySizeCategory Category;
	if (MaxDimension < 150.0f)
	{
		Category = EEnemySizeCategory::Small;
	}
	else if (MaxDimension < 400.0f)
	{
		Category = EEnemySizeCategory::Medium;
	}
	else
	{
		Category = EEnemySizeCategory::Large;
	}

	// Cache result
	EnemySizeCache.Add(Target, Category);

	return Category;
}

bool UTargetDetectionComponent::PerformLineOfSightCheck(AActor* Target) const
{
	if (!Target)
		return false;

	FHitResult HitResult;
	FVector Start = GetOwner()->GetActorLocation() + FVector(0.0f, 0.0f, RaycastHeightOffset);

	FVector End;
	if (bUseTargetCenterForLineOfSight)
	{
		// 使用目标实际中心（胶囊体中心），适配高大敌人
		ACharacter* TargetChar = Cast<ACharacter>(Target);
		if (TargetChar && TargetChar->GetCapsuleComponent())
		{
			End = TargetChar->GetCapsuleComponent()->GetComponentLocation();
		}
		else
		{
			// 非 Character 的 fallback：使用检测位置（大型敌人用 Bounds 中心）
			End = GetTargetDetectionLocation(Target);
		}
	}
	else
	{
		// 保留原始逻辑
		End = Target->GetActorLocation() + FVector(0.0f, 0.0f, RaycastHeightOffset);
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	// If nothing hit or the hit actor is our target, we have line of sight
	return !bHit || HitResult.GetActor() == Target;
}

FVector UTargetDetectionComponent::GetCameraForwardVector() const
{
	UCameraComponent* Camera = GetPlayerCamera();
	FVector Forward;

	if (Camera)
	{
		Forward = Camera->GetForwardVector();
	}
	else
	{
		Forward = GetOwner()->GetActorForwardVector();
	}

	Forward.Z = 0.0f; // Horizontal projection only
	Forward.Normalize();
	return Forward;
}

FVector UTargetDetectionComponent::GetCameraForwardVector(bool bIncludeVertical) const
{
	UCameraComponent* Camera = GetPlayerCamera();
	FVector Forward;

	if (Camera)
	{
		Forward = Camera->GetForwardVector();
	}
	else
	{
		Forward = GetOwner()->GetActorForwardVector();
	}

	if (!bIncludeVertical)
	{
		Forward.Z = 0.0f;
	}

	Forward.Normalize();
	return Forward;
}

UCameraComponent* UTargetDetectionComponent::GetPlayerCamera() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
		if (PC && PC->PlayerCameraManager)
		{
			// Try to get camera component from view target
			AActor* ViewTarget = PC->GetViewTarget();
			if (ViewTarget)
			{
				return ViewTarget->FindComponentByClass<UCameraComponent>();
			}
		}
	}

	return nullptr;
}

FVector UTargetDetectionComponent::GetTargetDetectionLocation(AActor* Target) const
{
	if (!Target) return FVector::ZeroVector;
	
	if (bUseBoundsCenterForLargeEnemies && Target->ActorHasTag(LargeEnemyTag))
	{
		FVector Origin, BoxExtent;
		Target->GetActorBounds(false, Origin, BoxExtent);
		UE_LOG(LogTemp, Log, TEXT("[LargeEnemy] %s using Bounds center: %s (Extent: %s)"),
			*Target->GetName(), *Origin.ToString(), *BoxExtent.ToString());
		return Origin;
	}
	
	return Target->GetActorLocation();
}

float UTargetDetectionComponent::GetDistanceToTargetBoundsEdge(AActor* Target) const
{
	if (!Target) return MAX_FLT;
	
	FVector Origin, BoxExtent;
	Target->GetActorBounds(false, Origin, BoxExtent);
	
	FVector OwnerLoc = GetOwner()->GetActorLocation();
	
	// 将玩家位置 Clamp 到 Bounds Box 内部，得到最近表面点
	FVector ClosestPoint;
	ClosestPoint.X = FMath::Clamp(OwnerLoc.X, Origin.X - BoxExtent.X, Origin.X + BoxExtent.X);
	ClosestPoint.Y = FMath::Clamp(OwnerLoc.Y, Origin.Y - BoxExtent.Y, Origin.Y + BoxExtent.Y);
	ClosestPoint.Z = FMath::Clamp(OwnerLoc.Z, Origin.Z - BoxExtent.Z, Origin.Z + BoxExtent.Z);
	
	float EdgeDist = FVector::Dist(OwnerLoc, ClosestPoint);
	
	UE_LOG(LogTemp, Log, TEXT("[LargeEnemy] %s BoundsEdge dist: %.1f (Center: %s, Extent: %s, Closest: %s)"),
		*Target->GetName(), EdgeDist, *Origin.ToString(), *BoxExtent.ToString(), *ClosestPoint.ToString());
	
	return EdgeDist;
}

void UTargetDetectionComponent::FindLargeEnemyCandidates(TArray<AActor*>& OutCandidates)
{
	if (!bUseBoundsCenterForLargeEnemies) return;

	// 使用 UGameplayStatics 获取所有 Pawn
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

	FVector OwnerLoc = GetOwner()->GetActorLocation();

	for (AActor* Pawn : AllPawns)
	{
		if (Pawn == GetOwner()) continue;
		if (!Pawn->ActorHasTag(LargeEnemyTag)) continue;

		// 已经在 OutCandidates 中的跳过（Overlap 已经检测到了）
		if (OutCandidates.Contains(Pawn)) continue;

		// 用 Bounds 边缘距离计算
		float DistToBounds = GetDistanceToTargetBoundsEdge(Pawn);

		UE_LOG(LogTemp, Warning, TEXT("[LargeEnemy Search] %s | BoundsEdgeDist: %.1f | Range: %.1f"),
			*Pawn->GetName(), DistToBounds, LockOnRange);

		if (DistToBounds <= LockOnRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LargeEnemy Search] %s ADDED to candidates via Bounds check"), *Pawn->GetName());
			OutCandidates.Add(Pawn);
		}
	}
}
