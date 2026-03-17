// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Utility/SoulsCameraDebugComponent.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Modules/CameraModuleRegistry.h"
#include "Camera/Modules/CameraModuleBase.h"
#include "Camera/Core/CameraModifierManager.h"
#include "Camera/Modifiers/CameraModifierBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "TargetDetectionComponent.h"
#include "SubTargetManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"

//========================================
// Constructor
//========================================

USoulsCameraDebugComponent::USoulsCameraDebugComponent()
{
	// Enable tick
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// 所有 UPROPERTY 成员的默认值现在在头文件中通过 C++11 默认成员初始化器设置
	// 这样可以避免 Hot Reload 时构造函数覆盖蓝图中编辑器设置的值

	// 非 UPROPERTY 运行时变量仍在此初始化
	LastDeltaTime = 0.0f;
	AverageFrameTime = 0.0f;
	PeakFrameTime = 0.0f;
	FrameCount = 0;
	TimeSincePerformanceReset = 0.0f;
	TimeSinceLastHistorySample = 0.0f;
}

//========================================
// UActorComponent Interface
//========================================

void USoulsCameraDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find camera manager
	FindCameraManager();
}

void USoulsCameraDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bDebugEnabled || !CameraManager)
	{
		return;
	}

	// Update performance metrics
	UpdatePerformanceMetrics(DeltaTime);

	// Update focus point history
	UpdateHistory(DeltaTime);

	// Draw debug visualization
	DrawDebugVisualization(DeltaTime);
}

//========================================
// Debug Control
//========================================

void USoulsCameraDebugComponent::SetDebugEnabled(bool bEnabled)
{
	bDebugEnabled = bEnabled;

	// Reset performance tracking when enabled
	if (bEnabled)
	{
		ResetPerformanceData();
		FocusPointHistory.Empty();
	}
}

bool USoulsCameraDebugComponent::IsDebugEnabled() const
{
	return bDebugEnabled;
}

void USoulsCameraDebugComponent::SetDebugFlags(const FCameraDebugFlags& Flags)
{
	DebugFlags = Flags;
}

FCameraDebugFlags USoulsCameraDebugComponent::GetDebugFlags() const
{
	return DebugFlags;
}

//========================================
// Debug String Generation
//========================================

FString USoulsCameraDebugComponent::GetDebugString() const
{
	FString Result;

	if (DebugFlags.bShowStateInfo)
	{
		Result += GetStateDebugString();
		Result += TEXT("\n");
	}

	if (DebugFlags.bShowModuleInfo)
	{
		Result += GetModulesDebugString();
		Result += TEXT("\n");
	}

	if (DebugFlags.bShowModifierInfo)
	{
		Result += GetModifiersDebugString();
		Result += TEXT("\n");
	}

	if (DebugFlags.bShowCollisionProbes)
	{
		Result += GetCollisionDebugString();
		Result += TEXT("\n");
	}

	if (DebugFlags.bShowPerformanceStats)
	{
		Result += GetPerformanceString();
	}

	return Result;
}

FString USoulsCameraDebugComponent::GetStateDebugString() const
{
	if (!CameraManager)
	{
		return TEXT("[Camera State] - No Camera Manager");
	}

	// Get camera output data
	FSoulsCameraOutput Output = CameraManager->GetCurrentOutput();

	FString Result = TEXT("=== Camera State ===\n");
	Result += FString::Printf(TEXT("State: %s\n"), *CameraManager->GetCurrentStateName().ToString());
	Result += FString::Printf(TEXT("Category: %s\n"), *UEnum::GetValueAsString(CameraManager->GetCurrentCategory()));
	Result += FString::Printf(TEXT("In Transition: %s\n"), CameraManager->IsInTransition() ? TEXT("Yes") : TEXT("No"));
	Result += TEXT("\n");

	// Camera output info
	Result += TEXT("--- Output ---\n");
	Result += FString::Printf(TEXT("Focus Point: (%.1f, %.1f, %.1f)\n"), 
		Output.FocusPoint.X, Output.FocusPoint.Y, Output.FocusPoint.Z);
	Result += FString::Printf(TEXT("Rotation: (P=%.1f, Y=%.1f, R=%.1f)\n"), 
		Output.Rotation.Pitch, Output.Rotation.Yaw, Output.Rotation.Roll);
	Result += FString::Printf(TEXT("Distance: %.2f\n"), Output.Distance);
	Result += FString::Printf(TEXT("FOV: %.2f\n"), Output.FOV);

	// Target info
	if (CameraManager->HasLockOnTarget())
	{
		AActor* Target = CameraManager->GetLockOnTarget();
		Result += FString::Printf(TEXT("Lock Target: %s\n"), Target ? *Target->GetName() : TEXT("None"));
	}

	return Result;
}

FString USoulsCameraDebugComponent::GetModulesDebugString() const
{
	if (!CameraManager)
	{
		return TEXT("[Modules] - No Camera Manager");
	}

	FString Result = TEXT("=== Modules ===\n");

	UCameraModuleRegistry* Registry = CameraManager->GetModuleRegistry();
	if (Registry)
	{
		int32 ModuleCount = Registry->GetModuleCount();
		int32 ActiveCount = Registry->GetActiveModuleCount();

		Result += FString::Printf(TEXT("Total Modules: %d\n"), ModuleCount);
		Result += FString::Printf(TEXT("Active Modules: %d\n"), ActiveCount);

		// Get all modules and list active ones
		TArray<UCameraModuleBase*> AllModules = Registry->GetAllModules();
		for (UCameraModuleBase* Module : AllModules)
		{
			if (Module && Module->IsActive())
			{
				Result += FString::Printf(TEXT("  - %s\n"), *Module->GetModuleName().ToString());
			}
		}
	}
	else
	{
		Result += TEXT("Module Registry not available\n");
	}

	return Result;
}

FString USoulsCameraDebugComponent::GetModifiersDebugString() const
{
	if (!CameraManager)
	{
		return TEXT("[Modifiers] - No Camera Manager");
	}

	FString Result = TEXT("=== Active Modifiers ===\n");

	UCameraModifierManager* ModifierManager = CameraManager->GetModifierManager();
	if (ModifierManager)
	{
		TArray<UCameraModifierBase*> ActiveModifiers;
		ModifierManager->GetActiveModifiers(ActiveModifiers);

		if (ActiveModifiers.Num() == 0)
		{
			Result += TEXT("No active modifiers\n");
		}
		else
		{
			for (UCameraModifierBase* Modifier : ActiveModifiers)
			{
				if (Modifier)
				{
					FString StateStr;
					switch (Modifier->GetState())
					{
					case EModifierState::BlendingIn:
						StateStr = TEXT("BlendIn");
						break;
					case EModifierState::Active:
						StateStr = TEXT("Active");
						break;
					case EModifierState::BlendingOut:
						StateStr = TEXT("BlendOut");
						break;
					case EModifierState::Paused:
						StateStr = TEXT("Paused");
						break;
					default:
						StateStr = TEXT("Unknown");
						break;
					}

					Result += FString::Printf(TEXT("  - %s [%s] Weight: %.2f\n"),
						*Modifier->GetModifierName().ToString(),
						*StateStr,
						Modifier->GetCurrentWeight());
				}
			}
		}

		Result += FString::Printf(TEXT("Total Active: %d\n"), ActiveModifiers.Num());
	}
	else
	{
		Result += TEXT("Modifier Manager not available\n");
	}

	return Result;
}

FString USoulsCameraDebugComponent::GetCollisionDebugString() const
{
	if (!CameraManager)
	{
		return TEXT("[Collision] - No Camera Manager");
	}

	FString Result = TEXT("=== Collision ===\n");

	Result += FString::Printf(TEXT("Collision Active: %s\n"), 
		CameraManager->IsCollisionActive() ? TEXT("Yes") : TEXT("No"));
	Result += FString::Printf(TEXT("In Recovery: %s\n"), 
		CameraManager->IsInCollisionRecovery() ? TEXT("Yes") : TEXT("No"));
	Result += FString::Printf(TEXT("Character Occluded: %s\n"), 
		CameraManager->IsCharacterOccluded() ? TEXT("Yes") : TEXT("No"));
	Result += FString::Printf(TEXT("Target Occluded: %s\n"), 
		CameraManager->IsTargetOccluded() ? TEXT("Yes") : TEXT("No"));
	Result += FString::Printf(TEXT("Adjusted Distance: %.2f\n"), 
		CameraManager->GetCollisionAdjustedDistance());

	return Result;
}

//========================================
// Debug Visualization
//========================================

void USoulsCameraDebugComponent::DrawDebugVisualization(float DeltaTime)
{
	if (!bDebugEnabled || !DebugFlags.HasAnyEnabled())
	{
		return;
	}

	// Draw world visualizations
	if (DebugFlags.HasWorldVisualization())
	{
		if (DebugFlags.bShowFocusPoint)
		{
			DrawFocusPoint();
		}

		if (DebugFlags.bShowCameraPath)
		{
			DrawCameraPath();
		}

		if (DebugFlags.bShowCollisionProbes)
		{
			DrawCollisionProbes();
		}

		if (DebugFlags.bShowTargetInfo)
		{
			DrawTargetInfo();
		}

		if (DebugFlags.bShowFramingDebug)
		{
			DrawFramingDebug();
		}

		if (DebugFlags.bShowLockOnDetection)
		{
			DrawLockOnDetection();
		}
	}

	// Draw on-screen text
	if (DebugFlags.bShowOnScreen)
	{
		DrawOnScreenDebug();
	}
}

void USoulsCameraDebugComponent::DrawFocusPoint()
{
	UWorld* World = GetWorld();
	if (!World || !CameraManager)
	{
		return;
	}

	// Get camera data through GetCurrentOutput
	FSoulsCameraOutput Output = CameraManager->GetCurrentOutput();
	FVector FocusPoint = Output.FocusPoint;

	// Draw focus point sphere
	DrawDebugSphere(World, FocusPoint, FocusPointSize, 8, FocusPointColor, false, -1.0f, 0, LineThickness * 0.5f);

	// Draw coordinate axes at focus point
	const float AxisLength = 30.0f;
	DrawDebugLine(World, FocusPoint, FocusPoint + FVector::ForwardVector * AxisLength, FColor::Red, false, -1.0f, 0, LineThickness * 0.5f);
	DrawDebugLine(World, FocusPoint, FocusPoint + FVector::RightVector * AxisLength, FColor::Green, false, -1.0f, 0, LineThickness * 0.5f);
	DrawDebugLine(World, FocusPoint, FocusPoint + FVector::UpVector * AxisLength, FColor::Blue, false, -1.0f, 0, LineThickness * 0.5f);

	// Draw text label
	DrawDebugString(World, FocusPoint + FVector(0, 0, 20), TEXT("Focus"), nullptr, FocusPointColor, 0.0f, true);
}

void USoulsCameraDebugComponent::DrawCameraPath()
{
	UWorld* World = GetWorld();
	if (!World || !CameraManager)
	{
		return;
	}

	FSoulsCameraOutput Output = CameraManager->GetCurrentOutput();
	FVector FocusPoint = Output.FocusPoint;
	FVector CameraLocation = Output.GetCameraLocation();
	FRotator CameraRotation = Output.Rotation;

	// Draw line from focus to camera
	DrawDebugLine(World, FocusPoint, CameraLocation, CameraPathColor, false, -1.0f, 0, LineThickness);

	// Draw camera position sphere
	DrawDebugSphere(World, CameraLocation, 8.0f, 8, CameraPathColor, false, -1.0f, 0, LineThickness * 0.5f);

	// Draw camera forward direction
	FVector CameraForward = CameraRotation.Vector();
	DrawDebugLine(World, CameraLocation, CameraLocation + CameraForward * 50.0f, FColor::Magenta, false, -1.0f, 0, LineThickness);

	// Draw camera up and right vectors
	FVector CameraRight = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
	FVector CameraUp = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Z);
	DrawDebugLine(World, CameraLocation, CameraLocation + CameraRight * 30.0f, FColor::Green, false, -1.0f, 0, LineThickness * 0.5f);
	DrawDebugLine(World, CameraLocation, CameraLocation + CameraUp * 30.0f, FColor::Blue, false, -1.0f, 0, LineThickness * 0.5f);

	// Draw distance text
	float Distance = FVector::Dist(FocusPoint, CameraLocation);
	FVector MidPoint = (FocusPoint + CameraLocation) * 0.5f;
	DrawDebugString(World, MidPoint, FString::Printf(TEXT("%.1f"), Distance), nullptr, CameraPathColor, 0.0f, true);

	// Draw focus point history if available
	if (FocusPointHistory.Num() > 1)
	{
		for (int32 i = 1; i < FocusPointHistory.Num(); ++i)
		{
			float Alpha = static_cast<float>(i) / static_cast<float>(FocusPointHistory.Num());
			FColor HistoryColor = FColor::MakeRedToGreenColorFromScalar(Alpha);
			DrawDebugLine(World, FocusPointHistory[i - 1], FocusPointHistory[i], HistoryColor, false, -1.0f, 0, LineThickness * 0.3f);
		}
	}
}

void USoulsCameraDebugComponent::DrawCollisionProbes()
{
	UWorld* World = GetWorld();
	if (!World || !CameraManager)
	{
		return;
	}

	FSoulsCameraOutput Output = CameraManager->GetCurrentOutput();
	FVector FocusPoint = Output.FocusPoint;
	FVector CameraLocation = Output.GetCameraLocation();

	// Determine collision state color
	FColor StateColor = SafeColor;
	if (CameraManager->IsCollisionActive())
	{
		StateColor = CollisionColor;
	}
	else if (CameraManager->IsInCollisionRecovery())
	{
		StateColor = WarningColor;
	}

	// Draw collision probe line
	FVector Direction = (CameraLocation - FocusPoint).GetSafeNormal();
	float AdjustedDistance = CameraManager->GetCollisionAdjustedDistance();
	FVector AdjustedLocation = FocusPoint + Direction * AdjustedDistance;

	// Draw the desired vs actual camera position
	if (CameraManager->IsCollisionActive())
	{
		// Draw the blocked (desired) position
		DrawDebugSphere(World, CameraLocation, 5.0f, 6, FColor::Red, false, -1.0f, 0, 1.0f);
		DrawDebugString(World, CameraLocation + FVector(0, 0, 10), TEXT("Desired"), nullptr, FColor::Red, 0.0f, true);

		// Draw the adjusted (safe) position
		DrawDebugSphere(World, AdjustedLocation, 8.0f, 8, SafeColor, false, -1.0f, 0, 1.5f);
		DrawDebugString(World, AdjustedLocation + FVector(0, 0, 10), TEXT("Safe"), nullptr, SafeColor, 0.0f, true);

		// Draw line between desired and safe
		DrawDebugLine(World, AdjustedLocation, CameraLocation, WarningColor, false, -1.0f, 0, 1.0f);
	}

	// Draw collision state indicator
	FVector IndicatorPos = FocusPoint + FVector(0, 0, 100);
	FString CollisionStateStr;
	if (CameraManager->IsCollisionActive())
	{
		CollisionStateStr = TEXT("COLLISION ACTIVE");
	}
	else if (CameraManager->IsInCollisionRecovery())
	{
		CollisionStateStr = TEXT("Recovering...");
	}
	else
	{
		CollisionStateStr = TEXT("Clear");
	}
	DrawDebugString(World, IndicatorPos, CollisionStateStr, nullptr, StateColor, 0.0f, true);

	// Draw occlusion indicators
	if (CameraManager->IsCharacterOccluded())
	{
		DrawDebugString(World, FocusPoint + FVector(0, 0, 50), TEXT("OCCLUDED"), nullptr, ErrorColor, 0.0f, true);
	}
}

void USoulsCameraDebugComponent::DrawTargetInfo()
{
	UWorld* World = GetWorld();
	if (!World || !CameraManager)
	{
		return;
	}

	if (!CameraManager->HasLockOnTarget())
	{
		return;
	}

	AActor* Target = CameraManager->GetLockOnTarget();
	if (!Target)
	{
		return;
	}

	FSoulsCameraOutput Output = CameraManager->GetCurrentOutput();
	FVector FocusPoint = Output.FocusPoint;
	FVector TargetLocation = Target->GetActorLocation();

	// Draw target indicator
	DrawDebugSphere(World, TargetLocation, TargetIndicatorSize, 12, TargetColor, false, -1.0f, 0, LineThickness);

	// Draw connection line from focus to target
	DrawDebugLine(World, FocusPoint, TargetLocation, TargetColor, false, -1.0f, 0, LineThickness * 0.5f);

	// Draw target name
	DrawDebugString(World, TargetLocation + FVector(0, 0, TargetIndicatorSize + 20), 
		FString::Printf(TEXT("Target: %s"), *Target->GetName()), 
		nullptr, TargetColor, 0.0f, true);

	// Draw distance to target
	float DistanceToTarget = FVector::Dist(FocusPoint, TargetLocation);
	FVector MidPoint = (FocusPoint + TargetLocation) * 0.5f;
	DrawDebugString(World, MidPoint + FVector(0, 0, 10), 
		FString::Printf(TEXT("Dist: %.1f"), DistanceToTarget), 
		nullptr, TargetColor, 0.0f, true);

	// Draw target occlusion indicator
	if (CameraManager->IsTargetOccluded())
	{
		DrawDebugString(World, TargetLocation + FVector(0, 0, TargetIndicatorSize + 40), 
			TEXT("TARGET OCCLUDED"), nullptr, ErrorColor, 0.0f, true);
	}
}

void USoulsCameraDebugComponent::DrawFramingDebug()
{
	UWorld* World = GetWorld();
	if (!World || !CameraManager)
	{
		return;
	}

	FSoulsCameraOutput Output = CameraManager->GetCurrentOutput();

	// === 获取基础数据 ===
	FVector PlayerLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	bool bHasTarget = false;

	// 从 CameraManager 获取输入上下文数据
	if (ACharacter* OwnerChar = Cast<ACharacter>(CameraManager->GetOwner()))
	{
		PlayerLocation = OwnerChar->GetActorLocation();
	}

	AActor* LockOnTarget = CameraManager->GetLockOnTarget();
	if (LockOnTarget)
	{
		TargetLocation = LockOnTarget->GetActorLocation();
		bHasTarget = true;
	}

	// === 如果没有目标，不绘制 Framing Debug ===
	if (!bHasTarget)
	{
		return;
	}

	// === 1. 绘制加权中心（蓝色球体）===
	// 使用 0.5/0.5 默认权重计算加权中心
	FVector WeightedCenter = (PlayerLocation + TargetLocation) * 0.5f;
	DrawDebugSphere(World, WeightedCenter, 15.0f, 8, FramingCenterColor, false, -1.0f, 0, LineThickness * 0.5f);
	DrawDebugString(World, WeightedCenter + FVector(0, 0, 25), TEXT("WeightedCenter"), nullptr, FramingCenterColor, 0.0f, true);

	// === 2. 绘制 Player → WeightedCenter → Target 连线 ===
	DrawDebugLine(World, PlayerLocation, WeightedCenter, FramingCenterColor, false, -1.0f, 0, LineThickness * 0.3f);
	DrawDebugLine(World, WeightedCenter, TargetLocation, FramingCenterColor, false, -1.0f, 0, LineThickness * 0.3f);

	// === 3. 绘制 WeightedCenter → FocusPoint 偏移线（绿色）===
	// 这条线展示 Anchor 约束产生的偏移
	FVector FocusPoint = Output.FocusPoint;
	DrawDebugLine(World, WeightedCenter, FocusPoint, FramingAnchorColor, false, -1.0f, 0, LineThickness);
	DrawDebugString(World, FocusPoint + FVector(0, 0, 35), TEXT("Framing Focus"), nullptr, FramingAnchorColor, 0.0f, true);

	// === 4. 绘制 Framing 状态文字 ===
	FVector TextLocation = FocusPoint + FVector(0, 0, 55);

	// 判断 Framing 是否在实际运行
	// 如果 FocusPoint 和 WeightedCenter 差距很大，说明 Anchor 偏移在生效
	float AnchorOffset = FVector::Dist(WeightedCenter, FocusPoint);
	FString FramingStatus = AnchorOffset > 10.0f ? TEXT("FRAMING ACTIVE") : TEXT("FRAMING IDLE");

	DrawDebugString(World, TextLocation, 
		FString::Printf(TEXT("[%s]\nP-T Dist: %.0f\nAnchor Offset: %.1f"), 
			*FramingStatus,
			FVector::Dist(PlayerLocation, TargetLocation),
			AnchorOffset),
		nullptr, FramingAnchorColor, 0.0f, true);
}

void USoulsCameraDebugComponent::DrawLockOnDetection()
{
	UWorld* World = GetWorld();
	if (!World || !CameraManager)
	{
		return;
	}

	// ==================== 获取玩家角色和检测组件 ====================
	AActor* OwnerActor = CameraManager->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	APawn* PlayerPawn = Cast<APawn>(OwnerActor);
	if (!PlayerPawn)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		PlayerPawn = PC ? PC->GetPawn() : nullptr;
	}
	if (!PlayerPawn)
	{
		return;
	}

	UTargetDetectionComponent* DetectionComp = PlayerPawn->FindComponentByClass<UTargetDetectionComponent>();
	if (!DetectionComp)
	{
		return;
	}

	// ==================== 基础参数 ====================
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	float Range = DetectionComp->LockOnRange;
	float FullAngle = DetectionComp->LockOnAngle;       // 例如 120° 总角度
	float SectorAngle = DetectionComp->SectorLockAngle;  // 例如 60° 优先区

	// 获取相机前方向（水平投影）
	APlayerController* PC = World->GetFirstPlayerController();
	FVector CameraForward = FVector::ForwardVector;
	if (PC && PC->PlayerCameraManager)
	{
		FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();
		CameraForward = FRotationMatrix(FRotator(0.f, CamRot.Yaw, 0.f)).GetUnitAxis(EAxis::X);
	}

	// 绘制高度 = 玩家位置 + 小幅抬高避免地面遮挡
	FVector DrawOrigin = PlayerLocation + FVector(0, 0, 10.0f);

	// ==================== 1. 检测范围圆（红色） ====================
	DrawDebugCircle(World, DrawOrigin, Range, 64,
		DetectionRangeColor, false, -1.0f, 0, LineThickness * 0.5f,
		FVector(0, 1, 0), FVector(1, 0, 0), false);

	// ==================== 2. 外圈检测锥（黄色）====================
	{
		float HalfAngleRad = FMath::DegreesToRadians(FullAngle / 2.0f);
		FVector LeftDir = CameraForward.RotateAngleAxis(-(FullAngle / 2.0f), FVector::UpVector);
		FVector RightDir = CameraForward.RotateAngleAxis(FullAngle / 2.0f, FVector::UpVector);

		FVector LeftEnd = DrawOrigin + LeftDir * Range;
		FVector RightEnd = DrawOrigin + RightDir * Range;

		// 两条边线
		DrawDebugLine(World, DrawOrigin, LeftEnd, DetectionConeColor, false, -1.0f, 0, LineThickness);
		DrawDebugLine(World, DrawOrigin, RightEnd, DetectionConeColor, false, -1.0f, 0, LineThickness);

		// 弧线（用多段线近似）
		const int32 ArcSegments = 24;
		float StartAngle = -(FullAngle / 2.0f);
		float AngleStep = FullAngle / (float)ArcSegments;
		for (int32 i = 0; i < ArcSegments; ++i)
		{
			float A1 = StartAngle + AngleStep * i;
			float A2 = StartAngle + AngleStep * (i + 1);
			FVector P1 = DrawOrigin + CameraForward.RotateAngleAxis(A1, FVector::UpVector) * Range;
			FVector P2 = DrawOrigin + CameraForward.RotateAngleAxis(A2, FVector::UpVector) * Range;
			DrawDebugLine(World, P1, P2, DetectionConeColor, false, -1.0f, 0, LineThickness * 0.8f);
		}

		// 标签
		FVector LabelPos = DrawOrigin + CameraForward * (Range + 50.0f);
		DrawDebugString(World, LabelPos, FString::Printf(TEXT("Detection: %.0f°"), FullAngle),
			nullptr, DetectionConeColor, 0.0f, true);
	}

	// ==================== 3. 内圈优先锥（绿色）====================
	{
		float SectorRange = Range * 0.85f; // 内圈稍短，视觉上区分
		FVector LeftDir = CameraForward.RotateAngleAxis(-(SectorAngle / 2.0f), FVector::UpVector);
		FVector RightDir = CameraForward.RotateAngleAxis(SectorAngle / 2.0f, FVector::UpVector);

		FVector LeftEnd = DrawOrigin + LeftDir * SectorRange;
		FVector RightEnd = DrawOrigin + RightDir * SectorRange;

		// 两条边线
		DrawDebugLine(World, DrawOrigin, LeftEnd, SectorConeColor, false, -1.0f, 0, LineThickness * 1.5f);
		DrawDebugLine(World, DrawOrigin, RightEnd, SectorConeColor, false, -1.0f, 0, LineThickness * 1.5f);

		// 弧线
		const int32 ArcSegments = 16;
		float StartAngle = -(SectorAngle / 2.0f);
		float AngleStep = SectorAngle / (float)ArcSegments;
		for (int32 i = 0; i < ArcSegments; ++i)
		{
			float A1 = StartAngle + AngleStep * i;
			float A2 = StartAngle + AngleStep * (i + 1);
			FVector P1 = DrawOrigin + CameraForward.RotateAngleAxis(A1, FVector::UpVector) * SectorRange;
			FVector P2 = DrawOrigin + CameraForward.RotateAngleAxis(A2, FVector::UpVector) * SectorRange;
			DrawDebugLine(World, P1, P2, SectorConeColor, false, -1.0f, 0, LineThickness * 1.2f);
		}

		// 标签
		FVector LabelPos = DrawOrigin + CameraForward * (SectorRange * 0.5f) + FVector(0, 0, 30.0f);
		DrawDebugString(World, LabelPos, FString::Printf(TEXT("Sector: %.0f°"), SectorAngle),
			nullptr, SectorConeColor, 0.0f, true);
	}

	// ==================== 4. 相机前方向线 ====================
	DrawDebugLine(World, DrawOrigin, DrawOrigin + CameraForward * Range,
		FColor::White, false, -1.0f, 0, LineThickness * 0.3f);

	// ==================== 5. 候选目标标记 ====================
	TArray<AActor*> Candidates = DetectionComp->LockOnCandidates;
	AActor* CurrentLockedTarget = CameraManager->HasLockOnTarget() ? CameraManager->GetLockOnTarget() : nullptr;

	// 获取 SubTargetManager（如果有）
	USubTargetManager* SubTargetMgr = PlayerPawn->FindComponentByClass<USubTargetManager>();

	for (AActor* Candidate : Candidates)
	{
		if (!Candidate || !IsValid(Candidate))
		{
			continue;
		}

		FVector CandidateLocation = Candidate->GetActorLocation();
		float DistToCandidate = FVector::Dist(PlayerLocation, CandidateLocation);

		// 计算角度判断在哪个区域
		FVector ToCandidate = CandidateLocation - PlayerLocation;
		ToCandidate.Z = 0.0f;
		ToCandidate.Normalize();
		float DotProduct = FVector::DotProduct(CameraForward, ToCandidate);
		float AngleToCandidate = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

		// 确定颜色
		FColor CandidateColor;
		float SphereSize;
		FString StatusStr;

		if (Candidate == CurrentLockedTarget)
		{
			// 当前锁定目标 = 蓝色大球
			CandidateColor = FColor::Blue;
			SphereSize = TargetIndicatorSize * 0.8f;
			StatusStr = TEXT("LOCKED");
		}
		else if (AngleToCandidate <= SectorAngle / 2.0f)
		{
			// 在优先区内 = 绿色
			CandidateColor = FColor::Green;
			SphereSize = TargetIndicatorSize * 0.5f;
			StatusStr = TEXT("Sector");
		}
		else if (AngleToCandidate <= FullAngle / 2.0f)
		{
			// 在检测区内但不在优先区 = 黄色
			CandidateColor = FColor::Yellow;
			SphereSize = TargetIndicatorSize * 0.4f;
			StatusStr = TEXT("Edge");
		}
		else
		{
			// 超出检测锥（不太可能出现在候选列表中）= 红色
			CandidateColor = FColor::Red;
			SphereSize = TargetIndicatorSize * 0.3f;
			StatusStr = TEXT("Out");
		}

		// 画球
		DrawDebugSphere(World, CandidateLocation, SphereSize, 8, CandidateColor, false, -1.0f, 0, LineThickness);

		// 画连线（从玩家到候选目标）
		DrawDebugLine(World, DrawOrigin, CandidateLocation, CandidateColor, false, -1.0f, 0, LineThickness * 0.5f);

		// 画文字标签
		FString Label = FString::Printf(TEXT("%s\n[%s] %.0f\u00B0 %.0fm"),
			*Candidate->GetName(),
			*StatusStr,
			AngleToCandidate,
			DistToCandidate / 100.0f); // 转换为米

		DrawDebugString(World, CandidateLocation + FVector(0, 0, SphereSize + 15.0f),
			Label, nullptr, CandidateColor, 0.0f, true);

		// ==================== 6. 子锁点标记（如果是当前锁定的多锁点 Entity）====================
		if (Candidate == CurrentLockedTarget && SubTargetMgr && SubTargetMgr->IsMultiTargetEntity())
		{
			// 获取当前子锁点位置
			if (SubTargetMgr->HasValidSubTarget())
			{
				FVector SubTargetPos = SubTargetMgr->GetCurrentLockPosition();
				// 当前子锁点 = 品红色菱形
				DrawDebugPoint(World, SubTargetPos, 20.0f, FColor::Magenta, false, -1.0f, 0);
				DrawDebugLine(World, CandidateLocation, SubTargetPos, FColor::Magenta, false, -1.0f, 0, LineThickness * 0.8f);
				DrawDebugString(World, SubTargetPos + FVector(0, 0, 25.0f), TEXT("SubTarget"),
					nullptr, FColor::Magenta, 0.0f, true);
			}
		}
	}

	// ==================== 7. 范围/角度信息文本 ====================
	FVector InfoPos = DrawOrigin + FVector(0, 0, 200.0f);
	FString InfoText = FString::Printf(TEXT("=== Lock-On Detection ===\nRange: %.0f (%.0fm)\nDetection: %.0f\u00B0\nSector: %.0f\u00B0\nCandidates: %d\nLocked: %s"),
		Range, Range / 100.0f,
		FullAngle,
		SectorAngle,
		Candidates.Num(),
		CurrentLockedTarget ? *CurrentLockedTarget->GetName() : TEXT("None"));

	DrawDebugString(World, InfoPos, InfoText, nullptr, FColor::White, 0.0f, true);
}

void USoulsCameraDebugComponent::DrawOnScreenDebug()
{
	if (!GEngine || !bDebugEnabled)
	{
		return;
	}

	// Build debug string
	FString DebugText = GetDebugString();

	// Display on screen
	if (!DebugText.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(
			-1,  // Key (-1 = unique each frame)
			0.0f,  // Time to display (0 = single frame)
			FColor::White,
			DebugText,
			true,  // Newer on top
			FVector2D(ScreenTextScale, ScreenTextScale)
		);
	}
}

//========================================
// Internal Methods
//========================================

void USoulsCameraDebugComponent::FindCameraManager()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Find camera manager on owner
	CameraManager = Owner->FindComponentByClass<USoulsCameraManager>();

	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraDebugComponent: Could not find USoulsCameraManager on owner %s"), *Owner->GetName());
	}
}

void USoulsCameraDebugComponent::UpdatePerformanceMetrics(float DeltaTime)
{
	LastDeltaTime = DeltaTime;
	FrameCount++;
	TimeSincePerformanceReset += DeltaTime;

	// Update peak
	if (DeltaTime > PeakFrameTime)
	{
		PeakFrameTime = DeltaTime;
	}

	// Update rolling average
	const float AverageAlpha = 0.1f;
	AverageFrameTime = FMath::Lerp(AverageFrameTime, DeltaTime, AverageAlpha);

	// Reset periodically
	if (TimeSincePerformanceReset >= PerformanceSampleInterval)
	{
		ResetPerformanceData();
	}
}

void USoulsCameraDebugComponent::UpdateHistory(float DeltaTime)
{
	if (!CameraManager)
	{
		return;
	}

	TimeSinceLastHistorySample += DeltaTime;

	if (TimeSinceLastHistorySample >= HistorySampleInterval)
	{
		TimeSinceLastHistorySample = 0.0f;

		// Get current focus point
		FSoulsCameraOutput Output = CameraManager->GetCurrentOutput();
		FocusPointHistory.Add(Output.FocusPoint);

		// Trim history if too large
		while (FocusPointHistory.Num() > MaxHistorySize)
		{
			FocusPointHistory.RemoveAt(0);
		}
	}
}

void USoulsCameraDebugComponent::ResetPerformanceData()
{
	FrameCount = 0;
	PeakFrameTime = 0.0f;
	TimeSincePerformanceReset = 0.0f;
}

FString USoulsCameraDebugComponent::GetPerformanceString() const
{
	FString Result = TEXT("=== Performance ===\n");

	float CurrentFPS = (LastDeltaTime > 0.0f) ? (1.0f / LastDeltaTime) : 0.0f;
	float AverageFPS = (AverageFrameTime > 0.0f) ? (1.0f / AverageFrameTime) : 0.0f;
	float MinFPS = (PeakFrameTime > 0.0f) ? (1.0f / PeakFrameTime) : 0.0f;

	Result += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentFPS);
	Result += FString::Printf(TEXT("Average FPS: %.1f\n"), AverageFPS);
	Result += FString::Printf(TEXT("Min FPS: %.1f\n"), MinFPS);
	Result += FString::Printf(TEXT("Frame Time: %.2f ms\n"), LastDeltaTime * 1000.0f);
	Result += FString::Printf(TEXT("Avg Frame Time: %.2f ms\n"), AverageFrameTime * 1000.0f);
	Result += FString::Printf(TEXT("Peak Frame Time: %.2f ms\n"), PeakFrameTime * 1000.0f);
	Result += FString::Printf(TEXT("Sample Frames: %d\n"), FrameCount);

	return Result;
}
