// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModule_Rotation.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModule_Rotation.cpp
 * 
 * This file contains the implementations of 9 Rotation modules (R01-R09).
 * Rotation modules are responsible for calculating the camera rotation (FRotator),
 * which determines the direction the camera is looking.
 * 
 * Output is passed through FModuleOutput.RotationOutput field.
 * 
 * Module List:
 * - R01: Player Input Free - Free camera rotation from player input (implemented)
 * - R02: Player Input Lagged - Smooth interpolated rotation (implemented)
 * - R03: LookAt Target - Hard lock-on rotation to target (implemented)
 * - R04: LookAt Target Soft - Soft lock with player input (implemented)
 * - R05: LookAt Boss - Boss-specific look-at behavior (implemented)
 * - R06: AutoOrient Movement - Auto-align to movement direction (implemented)
 * - R07: AutoOrient Delayed - Auto-center after input timeout (implemented)
 * - R08: LookAt Point - Look at fixed world point (implemented)
 * - R09: Spline Rotation - Rotation along spline path (placeholder)
 */


//========================================
// R01: Player Input Free - Free camera rotation from player input
//========================================

UCameraModule_R01_PlayerInput_Free::UCameraModule_R01_PlayerInput_Free()
	: CurrentRotation(FRotator::ZeroRotator)
	, BaseSensitivity(1.0f)
{
}

void UCameraModule_R01_PlayerInput_Free::OnActivate(const FStageExecutionContext& Context)
{
	Super::OnActivate(Context);
	CurrentRotation = Context.InputContext.PreviousCameraRotation;
}

bool UCameraModule_R01_PlayerInput_Free::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	if (!bIsEnabled)
	{
		return false;
	}
	
	if (Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	if (StateConfig.StateBase.Flags.bIgnoreInput)
	{
		return false;
	}
	
	return true;
}

bool UCameraModule_R01_PlayerInput_Free::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	OutOutput = CreateBaseOutput();
	
	FVector2D CameraInput = Context.InputContext.CameraInput;
	
	float InputSensitivity = StateConfig.Module.Rotation.InputSensitivity;
	if (InputSensitivity <= 0.0f)
	{
		InputSensitivity = 1.0f;
	}
	
	float TotalSensitivity = InputSensitivity * BaseSensitivity;
	
	float DeltaYaw = CameraInput.X * TotalSensitivity;
	float DeltaPitch = CameraInput.Y * TotalSensitivity * -1.0f;
	
	CurrentRotation.Yaw += DeltaYaw;
	CurrentRotation.Pitch += DeltaPitch;
	
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	if (MinPitch >= MaxPitch)
	{
		MinPitch = -80.0f;
		MaxPitch = 80.0f;
	}
	
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, MinPitch, MaxPitch);
	
	CurrentRotation.Yaw = FMath::Fmod(CurrentRotation.Yaw, 360.0f);
	if (CurrentRotation.Yaw < 0.0f)
	{
		CurrentRotation.Yaw += 360.0f;
	}
	
	CurrentRotation.Roll = 0.0f;
	
	OutOutput.RotationOutput = CurrentRotation;
	OutOutput.bHasRotationOutput = true;
	
	return true;
}

//========================================
// R02: Player Input Lagged - Smoothed camera rotation
//========================================

UCameraModule_R02_PlayerInput_Lagged::UCameraModule_R02_PlayerInput_Lagged()
	: TargetRotation(FRotator::ZeroRotator)
	, SmoothedRotation(FRotator::ZeroRotator)
	, bInitialized(false)
{
}

void UCameraModule_R02_PlayerInput_Lagged::OnActivate(const FStageExecutionContext& Context)
{
	Super::OnActivate(Context);
	
	TargetRotation = Context.InputContext.PreviousCameraRotation;
	SmoothedRotation = Context.InputContext.PreviousCameraRotation;
	bInitialized = true;
}

bool UCameraModule_R02_PlayerInput_Lagged::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	if (!bIsEnabled)
	{
		return false;
	}
	
	if (Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	if (StateConfig.StateBase.Flags.bIgnoreInput)
	{
		return false;
	}
	
	if (!StateConfig.Module.Rotation.bEnableRotationLag)
	{
		return false;
	}
	
	return true;
}

bool UCameraModule_R02_PlayerInput_Lagged::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	OutOutput = CreateBaseOutput();
	
	if (!bInitialized)
	{
		TargetRotation = Context.InputContext.PreviousCameraRotation;
		SmoothedRotation = Context.InputContext.PreviousCameraRotation;
		bInitialized = true;
	}
	
	FVector2D CameraInput = Context.InputContext.CameraInput;
	
	float InputSensitivity = StateConfig.Module.Rotation.InputSensitivity;
	if (InputSensitivity <= 0.0f)
	{
		InputSensitivity = 1.0f;
	}
	
	float DeltaYaw = CameraInput.X * InputSensitivity;
	float DeltaPitch = CameraInput.Y * InputSensitivity * -1.0f;
	
	TargetRotation.Yaw += DeltaYaw;
	TargetRotation.Pitch += DeltaPitch;
	
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	if (MinPitch >= MaxPitch)
	{
		MinPitch = -80.0f;
		MaxPitch = 80.0f;
	}
	
	TargetRotation.Pitch = FMath::Clamp(TargetRotation.Pitch, MinPitch, MaxPitch);
	
	TargetRotation.Yaw = FMath::Fmod(TargetRotation.Yaw, 360.0f);
	if (TargetRotation.Yaw < 0.0f)
	{
		TargetRotation.Yaw += 360.0f;
	}
	
	TargetRotation.Roll = 0.0f;
	
	float LagSpeed = StateConfig.StateBase.Lag.RotationLagSpeed;
	if (LagSpeed <= 0.0f)
	{
		LagSpeed = 5.0f;
	}
	
	SmoothedRotation = FMath::RInterpTo(
		SmoothedRotation,
		TargetRotation,
		Context.DeltaTime,
		LagSpeed
	);
	
	OutOutput.RotationOutput = SmoothedRotation;
	OutOutput.bHasRotationOutput = true;
	
	return true;
}

//========================================
// R03: LookAt Target - Hard lock-on rotation
//========================================

bool UCameraModule_R03_LookAt_Target::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// 检查模块是否启用
	if (!bIsEnabled)
	{
		return false;
	}
	
	// ★★★ 关键检查：必须有锁定目标 ★★★
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	// Boss 目标使用 R05，不使用 R03
	if (Context.InputContext.bTargetIsBoss)
	{
		return false;
	}
	
	// ★★★ 通过状态名称检测锁定状态（最可靠的方法）★★★
	FName CurrentStateName = Context.Output.CurrentStateName;
	if (!CurrentStateName.IsNone())
	{
		FString StateNameStr = CurrentStateName.ToString();
		
		// 检查状态名是否包含 "LockOn"
		if (StateNameStr.Contains(TEXT("LockOn"), ESearchCase::IgnoreCase))
		{
			// ★ 从 Manager 读取 Debug 开关
			if (Context.Manager && Context.Manager->IsR03DebugEnabled())
			{
				UE_LOG(LogTemp, Log, TEXT("R03: Activating for state '%s'"), *StateNameStr);
			}
			return true;
		}
	}
	
	// 备用方法：检查配置标志 bRequiresTarget
	if (StateConfig.StateBase.Flags.bRequiresTarget)
	{
		return true;
	}
	
	return false;
}

bool UCameraModule_R03_LookAt_Target::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	OutOutput = CreateBaseOutput();
	
	// ★★★ 强制设置 R03 的模块信息 ★★★
	OutOutput.ModuleID = ECameraModuleID::Module_R03_Rotation_LookAtTarget;
	OutOutput.ModuleCategory = ECameraModuleCategory::CategoryRotation;
	// ★★★ 提高 R03 优先级，确保在锁定状态下覆盖其他 Rotation 模块 ★★★
	OutOutput.Priority = 250;  // 高于约束模块 (210) 和其他模块
	OutOutput.BlendPolicy = EBlendPolicy::Override;
	OutOutput.Weight = 1.0f;
	
	// ★ 从 Manager 读取 Debug 开关
	bool bShouldLogR03 = false;
	if (Context.Manager)
	{
		bShouldLogR03 = Context.Manager->IsR03DebugEnabled();
	}
	
	// ★★★ 调试日志 ★★★
	if (bShouldLogR03)
	{
		UE_LOG(LogTemp, Warning, TEXT(""));
		UE_LOG(LogTemp, Warning, TEXT("★★★ R03 LookAt_Target COMPUTE ★★★"));
		UE_LOG(LogTemp, Warning, TEXT("  bHasTarget: %s"), Context.InputContext.bHasTarget ? TEXT("YES") : TEXT("NO"));
		if (Context.InputContext.TargetActor.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("  TargetActor: %s"), *Context.InputContext.TargetActor->GetName());
		}
		UE_LOG(LogTemp, Warning, TEXT("  TargetLocation: (%.1f, %.1f, %.1f)"),
			Context.InputContext.TargetLocation.X,
			Context.InputContext.TargetLocation.Y,
			Context.InputContext.TargetLocation.Z);
		UE_LOG(LogTemp, Warning, TEXT("  CharacterLocation: (%.1f, %.1f, %.1f)"),
			Context.InputContext.CharacterLocation.X,
			Context.InputContext.CharacterLocation.Y,
			Context.InputContext.CharacterLocation.Z);
	}
	
	if (!Context.InputContext.bHasTarget)
	{
		OutOutput.bHasRotationOutput = false;
		return false;
	}
	
	// ★★★ 修复：使用角色位置作为相机观察起点 ★★★
	FVector CameraOrigin = Context.InputContext.CharacterLocation;
	CameraOrigin.Z += 100.0f; // 相机在角色头部高度
	
	// 目标位置
	FVector TargetLocation = Context.InputContext.TargetLocation;
	
	// === R10 Framing FocusPoint Override ===
	// 如果有 Framing 模块输出了 FocusPoint，使用它替代原始 TargetLocation
	// 这样相机朝向会跟随构图意图
	for (const FModuleOutput& PrevOutput : Context.ModuleOutputs)
	{
		if (PrevOutput.bHasFocusPoint && PrevOutput.Weight > 0.0f)
		{
			TargetLocation = PrevOutput.FocusPoint;
			break;
		}
	}
	
	// 目标高度偏移（看向敌人的中心/头部）
	float TargetSize = Context.InputContext.TargetSize;
	if (TargetSize <= 0.0f)
	{
		TargetSize = 100.0f;
	}
	float TargetHeightOffset = TargetSize * 0.5f;
	TargetHeightOffset = FMath::Clamp(TargetHeightOffset, 30.0f, 150.0f);
	TargetLocation.Z += TargetHeightOffset;
	
	// ★★★ 计算看向目标的旋转 ★★★
	FVector LookDirection = TargetLocation - CameraOrigin;
	LookDirection = LookDirection.GetSafeNormal();
	
	if (LookDirection.IsNearlyZero())
	{
		LookDirection = FVector::ForwardVector;
	}
	
	FRotator LookAtRotation = LookDirection.Rotation();
	
	// Pitch 限制
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	if (MinPitch >= MaxPitch)
	{
		MinPitch = -60.0f;
		MaxPitch = 60.0f;
	}
	
	LookAtRotation.Pitch = FMath::Clamp(LookAtRotation.Pitch, MinPitch, MaxPitch);
	LookAtRotation.Roll = 0.0f;
	
	// ★★★ 调试输出 ★★★
	if (bShouldLogR03)
	{
		UE_LOG(LogTemp, Warning, TEXT("  Computed Rotation: Pitch=%.1f, Yaw=%.1f"),
			LookAtRotation.Pitch, LookAtRotation.Yaw);
	}
	
	OutOutput.RotationOutput = LookAtRotation;
	OutOutput.bHasRotationOutput = true;
	
	// ★★★ 调试：确认输出结构正确填充 ★★★
	if (bShouldLogR03)
	{
		UE_LOG(LogTemp, Error, TEXT("  ★★★ R03 Output Structure ★★★"));
		UE_LOG(LogTemp, Error, TEXT("    bHasRotationOutput: %s"), OutOutput.bHasRotationOutput ? TEXT("YES") : TEXT("NO"));
		UE_LOG(LogTemp, Error, TEXT("    RotationOutput: Pitch=%.1f, Yaw=%.1f"), 
			OutOutput.RotationOutput.Pitch, OutOutput.RotationOutput.Yaw);
		UE_LOG(LogTemp, Error, TEXT("    ModuleID: %d"), static_cast<int32>(OutOutput.ModuleID));
		UE_LOG(LogTemp, Error, TEXT("    ModuleCategory: %d"), static_cast<int32>(OutOutput.ModuleCategory));
		UE_LOG(LogTemp, Error, TEXT("    Priority: %d"), OutOutput.Priority);
		UE_LOG(LogTemp, Error, TEXT("    Weight: %.2f"), OutOutput.Weight);
		UE_LOG(LogTemp, Error, TEXT("    BlendPolicy: %d"), static_cast<int32>(OutOutput.BlendPolicy));
	}
	
	return true;
}

//========================================
// R04: LookAt Target Soft - Soft lock with player input
//========================================

UCameraModule_R04_LookAt_Target_Soft::UCameraModule_R04_LookAt_Target_Soft()
	: InputOffset(FRotator::ZeroRotator)
	, CurrentRotation(FRotator::ZeroRotator)
{
}

void UCameraModule_R04_LookAt_Target_Soft::OnActivate(const FStageExecutionContext& Context)
{
	Super::OnActivate(Context);
	CurrentRotation = Context.InputContext.PreviousCameraRotation;
	InputOffset = FRotator::ZeroRotator;
}

bool UCameraModule_R04_LookAt_Target_Soft::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	if (!bIsEnabled)
	{
		return false;
	}
	
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	if (Context.InputContext.bTargetIsBoss)
	{
		return false;
	}
	
	// ★★★ 新增：通过状态名称检测软锁定状态 ★★★
	FName CurrentStateName = Context.Output.CurrentStateName;
	FString StateNameStr = CurrentStateName.ToString();
	
	// 只在软锁定状态下激活 R04
	// 硬锁定 (LockOn_Hard) 使用 R03
	if (StateNameStr.Contains(TEXT("LockOn_Soft"), ESearchCase::IgnoreCase))
	{
		return true;
	}
	
	// 如果状态明确是硬锁定，不激活 R04（让 R03 处理）
	if (StateNameStr.Contains(TEXT("LockOn_Hard"), ESearchCase::IgnoreCase))
	{
		return false;
	}
	
	// 其他 LockOn 状态，检查是否需要软锁定
	if (StateNameStr.Contains(TEXT("LockOn"), ESearchCase::IgnoreCase))
	{
		// 默认使用软锁定（允许玩家微调）
		// 如果你想默认硬锁定，可以返回 false
		return true;
	}
	
	return false;
}

bool UCameraModule_R04_LookAt_Target_Soft::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	OutOutput = CreateBaseOutput();
	
	if (!Context.InputContext.bHasTarget)
	{
		OutOutput.bHasRotationOutput = false;
		return false;
	}
	
	FVector CameraFocusPoint = Context.Output.FocusPoint;
	if (CameraFocusPoint.IsNearlyZero())
	{
		CameraFocusPoint = Context.InputContext.CharacterLocation;
		CameraFocusPoint.Z += 100.0f;
	}
	
	FVector TargetLocation = Context.InputContext.TargetLocation;
	
	float TargetSize = Context.InputContext.TargetSize;
	if (TargetSize <= 0.0f)
	{
		TargetSize = 100.0f;
	}
	
	float TargetHeightOffset = TargetSize * 0.3f;
	TargetHeightOffset = FMath::Clamp(TargetHeightOffset, 20.0f, 100.0f);
	TargetLocation.Z += TargetHeightOffset;
	
	FVector LookDirection = (TargetLocation - CameraFocusPoint).GetSafeNormal();
	if (LookDirection.IsNearlyZero())
	{
		LookDirection = FVector::ForwardVector;
	}
	
	FRotator BaseLookAtRotation = LookDirection.Rotation();
	
	FVector2D CameraInput = Context.InputContext.CameraInput;
	
	float InputSensitivity = StateConfig.Module.Rotation.InputSensitivity;
	if (InputSensitivity <= 0.0f)
	{
		InputSensitivity = 1.0f;
	}
	
	float SoftLockSensitivity = InputSensitivity * 0.5f;
	
	float DeltaYaw = CameraInput.X * SoftLockSensitivity;
	float DeltaPitch = CameraInput.Y * SoftLockSensitivity * -1.0f;
	
	InputOffset.Yaw += DeltaYaw;
	InputOffset.Pitch += DeltaPitch;
	
	const float MaxOffsetYaw = 30.0f;
	const float MaxOffsetPitch = 15.0f;
	
	InputOffset.Yaw = FMath::Clamp(InputOffset.Yaw, -MaxOffsetYaw, MaxOffsetYaw);
	InputOffset.Pitch = FMath::Clamp(InputOffset.Pitch, -MaxOffsetPitch, MaxOffsetPitch);
	
	float InputMagnitude = CameraInput.Size();
	if (InputMagnitude < 0.1f)
	{
		const float DecayRate = 2.0f;
		InputOffset.Yaw = FMath::FInterpTo(InputOffset.Yaw, 0.0f, Context.DeltaTime, DecayRate);
		InputOffset.Pitch = FMath::FInterpTo(InputOffset.Pitch, 0.0f, Context.DeltaTime, DecayRate);
	}
	
	FRotator TargetRotation = BaseLookAtRotation + InputOffset;
	
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	if (MinPitch >= MaxPitch)
	{
		MinPitch = -80.0f;
		MaxPitch = 80.0f;
	}
	
	TargetRotation.Pitch = FMath::Clamp(TargetRotation.Pitch, MinPitch, MaxPitch);
	TargetRotation.Roll = 0.0f;
	
	float InterpSpeed = StateConfig.Module.Rotation.LookAtInterpSpeed;
	if (InterpSpeed <= 0.0f)
	{
		InterpSpeed = 5.0f;
	}
	
	CurrentRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, Context.DeltaTime, InterpSpeed);
	
	float SoftLockStrength = StateConfig.Module.Rotation.SoftLockStrength;
	if (SoftLockStrength <= 0.0f || SoftLockStrength > 1.0f)
	{
		SoftLockStrength = 0.7f;
	}
	
	OutOutput.RotationOutput = CurrentRotation;
	OutOutput.bHasRotationOutput = true;
	OutOutput.Weight = SoftLockStrength;
	
	return true;
}


//========================================
// R05: LookAt Boss - Boss-specific look-at behavior
//========================================

bool UCameraModule_R05_LookAt_Boss::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	if (!bIsEnabled)
	{
		return false;
	}
	
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	if (!Context.InputContext.bTargetIsBoss)
	{
		return false;
	}
	
	return true;
}

bool UCameraModule_R05_LookAt_Boss::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	OutOutput = CreateBaseOutput();
	
	if (!Context.InputContext.bHasTarget)
	{
		OutOutput.bHasRotationOutput = false;
		return false;
	}
	
	FVector CameraFocusPoint = Context.Output.FocusPoint;
	if (CameraFocusPoint.IsNearlyZero())
	{
		CameraFocusPoint = Context.InputContext.CharacterLocation;
		CameraFocusPoint.Z += 100.0f;
	}
	
	FVector TargetLocation = Context.InputContext.TargetLocation;
	float TargetSize = Context.InputContext.TargetSize;
	
	if (TargetSize <= 0.0f)
	{
		TargetSize = 200.0f;
	}
	
	float BossHeightOffset = TargetSize * 0.4f;
	BossHeightOffset = FMath::Clamp(BossHeightOffset, 50.0f, 300.0f);
	
	FVector BossLookPoint = TargetLocation;
	BossLookPoint.Z += BossHeightOffset;
	
	FVector LookDirection = (BossLookPoint - CameraFocusPoint).GetSafeNormal();
	if (LookDirection.IsNearlyZero())
	{
		LookDirection = FVector::ForwardVector;
	}
	
	FRotator LookAtRotation = LookDirection.Rotation();
	
	float DistanceToTarget = Context.InputContext.TargetDistance;
	if (DistanceToTarget <= 0.0f)
	{
		DistanceToTarget = (TargetLocation - Context.InputContext.CharacterLocation).Size();
	}
	
	float SizeToDistanceRatio = TargetSize / FMath::Max(DistanceToTarget, 100.0f);
	
	if (SizeToDistanceRatio > 0.5f)
	{
		float PitchAdjust = (SizeToDistanceRatio - 0.5f) * 20.0f;
		PitchAdjust = FMath::Clamp(PitchAdjust, 0.0f, 15.0f);
		LookAtRotation.Pitch -= PitchAdjust;
	}
	
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	if (MinPitch >= MaxPitch)
	{
		MinPitch = -80.0f;
		MaxPitch = 80.0f;
	}
	
	LookAtRotation.Pitch = FMath::Clamp(LookAtRotation.Pitch, MinPitch, MaxPitch);
	LookAtRotation.Roll = 0.0f;
	
	OutOutput.RotationOutput = LookAtRotation;
	OutOutput.bHasRotationOutput = true;
	
	return true;
}


//========================================
// R06: AutoOrient Movement - Auto-align to movement direction
//========================================

bool UCameraModule_R06_AutoOrient_Movement::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	if (!bIsEnabled)
	{
		return false;
	}
	
	if (Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	if (!Context.InputContext.bIsMoving)
	{
		return false;
	}
	
	float MinSpeed = 100.0f;
	if (Context.InputContext.CharacterSpeed < MinSpeed)
	{
		return false;
	}
	
	FVector2D CameraInput = Context.InputContext.CameraInput;
	float InputMagnitude = CameraInput.Size();
	if (InputMagnitude >= 0.1f)
	{
		return false;
	}
	
	return true;
}

bool UCameraModule_R06_AutoOrient_Movement::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	OutOutput = CreateBaseOutput();
	
	FVector Velocity = Context.InputContext.CharacterVelocity;
	Velocity.Z = 0.0f;
	
	if (Velocity.IsNearlyZero())
	{
		OutOutput.bHasRotationOutput = false;
		return false;
	}
	
	FRotator MovementRotation = Velocity.Rotation();
	
	FRotator DesiredRotation;
	DesiredRotation.Yaw = MovementRotation.Yaw;
	
	FRotator CurrentRotation = Context.Output.Rotation;
	if (CurrentRotation.IsNearlyZero())
	{
		CurrentRotation = Context.InputContext.PreviousCameraRotation;
	}
	DesiredRotation.Pitch = CurrentRotation.Pitch;
	DesiredRotation.Roll = 0.0f;
	
	float CharacterSpeed = Context.InputContext.CharacterSpeed;
	const float MaxRunSpeed = 600.0f;
	float SpeedRatio = FMath::Clamp(CharacterSpeed / MaxRunSpeed, 0.0f, 1.0f);
	
	const float MaxInfluence = 0.5f;
	float Weight = SpeedRatio * MaxInfluence;
	
	OutOutput.RotationOutput = DesiredRotation;
	OutOutput.bHasRotationOutput = true;
	OutOutput.Weight = Weight;
	
	return true;
}


//========================================
// R07: AutoOrient Delayed - Auto-center after input timeout
//========================================

UCameraModule_R07_AutoOrient_Delayed::UCameraModule_R07_AutoOrient_Delayed()
	: TimeSinceInput(0.0f)
	, bIsAutoCentering(false)
{
}

bool UCameraModule_R07_AutoOrient_Delayed::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	if (!bIsEnabled)
	{
		return false;
	}
	
	if (Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	if (!StateConfig.StateBase.AutoCorrect.bEnableAutoCenter)
	{
		return false;
	}
	
	return true;
}

bool UCameraModule_R07_AutoOrient_Delayed::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	OutOutput = CreateBaseOutput();
	
	FVector2D CameraInput = Context.InputContext.CameraInput;
	float InputMagnitude = CameraInput.Size();
	
	if (InputMagnitude > 0.1f)
	{
		TimeSinceInput = 0.0f;
		bIsAutoCentering = false;
		OutOutput.bHasRotationOutput = false;
		return false;
	}
	
	TimeSinceInput += Context.DeltaTime;
	
	float AutoCenterDelay = StateConfig.StateBase.AutoCorrect.AutoCenterDelay;
	if (AutoCenterDelay <= 0.0f)
	{
		AutoCenterDelay = 2.0f;
	}
	
	if (TimeSinceInput < AutoCenterDelay)
	{
		OutOutput.bHasRotationOutput = false;
		return false;
	}
	
	bIsAutoCentering = true;
	
	FRotator CharacterRotation = Context.InputContext.CharacterRotation;
	
	FRotator TargetRotation;
	TargetRotation.Yaw = CharacterRotation.Yaw;
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;
	
	float TimePastDelay = TimeSinceInput - AutoCenterDelay;
	const float BlendDuration = 2.0f;
	float Weight = FMath::Clamp(TimePastDelay / BlendDuration, 0.0f, 0.5f);
	
	OutOutput.RotationOutput = TargetRotation;
	OutOutput.bHasRotationOutput = true;
	OutOutput.Weight = Weight;
	OutOutput.BlendPolicy = EBlendPolicy::Blend;
	
	return true;
}

//========================================
// R08: LookAt Point - Look at fixed world point
//========================================

/**
 * ShouldActivate - Determine if fixed point look-at should be active
 * 
 * R08 activates when:
 * - Module is enabled
 * - Current state is cinematic (bIsCinematic flag)
 * - A valid look-at point is defined (non-zero FixedPointLocation)
 * 
 * This module is designed for:
 * - Cinematic cutscenes with specific camera focus
 * - Looking at important objects/landmarks
 * - Scripted camera moments
 * - Points of interest in the world
 * 
 * Priority 180 (high) ensures it takes precedence over input modules
 * during cinematic moments. Uses Override policy for authoritative control.
 * 
 * Differences from R03 (LookAt Target):
 * - R03: Looks at dynamic target Actor (moves with target)
 * - R08: Looks at fixed world point (static coordinate)
 */
bool UCameraModule_R08_LookAt_Point::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if current state is cinematic
	// R08 is typically used for cutscenes and scripted moments
	if (!StateConfig.StateBase.Flags.bIsCinematic)
	{
		return false;
	}
	
	// Check if a valid look-at point is defined
	// Use FixedPointLocation from Position params (shared with P06)
	// This allows both P06 (position) and R08 (rotation) to use same point
	FVector LookAtPoint = StateConfig.Module.Position.FixedPointLocation;
	
	if (LookAtPoint.IsNearlyZero())
	{
		// No valid look-at point defined - don't activate
		return false;
	}
	
	// All conditions met - activate fixed point look-at
	return true;
}

/**
 * Compute - Calculate rotation to face fixed world point
 * 
 * This module calculates the rotation needed to make the camera face
 * a specific world coordinate. This is simpler than R03 (target tracking)
 * because the point is static.
 * 
 * Steps:
 * 1. Get look-at point from StateConfig (Position.FixedPointLocation)
 * 2. Get camera focus point (from Position modules)
 * 3. Calculate look direction vector
 * 4. Convert to rotation
 * 5. Clamp pitch to limits
 * 6. Output the calculated rotation
 * 
 * Key Features:
 * - Static world point (doesn't move)
 * - Uses Override policy (authoritative for cinematics)
 * - Similar to R03 but without dynamic target
 * - Useful for pointing camera at landmarks/objects
 * 
 * Use Cases:
 * - Cutscene: camera looks at distant castle
 * - Intro sequence: camera pans to important object
 * - Scripted moment: draw attention to specific location
 */
bool UCameraModule_R08_LookAt_Point::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get look-at point from StateConfig
	// Use Position.FixedPointLocation (shared with P06 for cinematic setups)
	FVector LookAtPoint = StateConfig.Module.Position.FixedPointLocation;
	
	// Validate look-at point
	if (LookAtPoint.IsNearlyZero())
	{
		// No valid look-at point - cannot compute
		// This shouldn't happen if ShouldActivate is correct
		UE_LOG(LogTemp, Warning, TEXT("R08 LookAt Point: Activated but no valid point defined!"));
		OutOutput.bHasRotationOutput = false;
		return false;
	}
	
	// Step 2: Get camera focus point
	// This is calculated by Position modules in Stage 3
	FVector CameraFocusPoint = Context.Output.FocusPoint;
	
	// Fallback if FocusPoint is invalid
	if (CameraFocusPoint.IsNearlyZero())
	{
		// Use character location as fallback
		CameraFocusPoint = Context.InputContext.CharacterLocation;
		
		// Add standard height offset for typical camera position
		CameraFocusPoint.Z += 100.0f;  // 1 meter above character
	}
	
	// Step 3: Calculate look direction vector
	// Direction FROM camera position TO look-at point
	FVector LookDirection = LookAtPoint - CameraFocusPoint;
	
	// Normalize to unit vector (GetSafeNormal handles zero-length safely)
	LookDirection = LookDirection.GetSafeNormal();
	
	// Handle edge case: zero direction (camera at look-at point)
	if (LookDirection.IsNearlyZero())
	{
		// Camera is at the look-at point, cannot determine direction
		// Fallback: look forward
		LookDirection = FVector::ForwardVector;
	}
	
	// Step 4: Convert direction vector to rotation
	FRotator LookAtRotation = LookDirection.Rotation();
	
	// Step 5: Clamp pitch to configured limits
	// Same as other look-at modules, prevent extreme angles
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	// Validate pitch limits are reasonable
	if (MinPitch >= MaxPitch)
	{
		// Invalid config, use default limits
		MinPitch = -80.0f;
		MaxPitch = 80.0f;
	}
	
	LookAtRotation.Pitch = FMath::Clamp(LookAtRotation.Pitch, MinPitch, MaxPitch);
	
	// Step 6: Force roll to zero
	// Third-person cameras should never roll
	LookAtRotation.Roll = 0.0f;
	
	// Step 7: Fill output structure
	OutOutput.RotationOutput = LookAtRotation;
	OutOutput.bHasRotationOutput = true;
	
	return true;
}


//========================================
// R09: Spline Rotation - Rotation along spline path
//========================================

/**
 * ShouldActivate - Determine if spline rotation should be active
 * 
 * R09 activates when:
 * - Module is enabled
 * - Current state is cinematic (splines are for scripted sequences)
 * - (Future) A valid spline component reference exists
 * 
 * This module is designed to work with P07 (Spline Follow) for
 * scripted camera paths along spline curves.
 * 
 * Current Implementation (Placeholder):
 * Since full spline system is not yet integrated, we use bIsCinematic
 * as the activation condition. When spline system is added, this will
 * also check for a valid SplineComponent reference.
 * 
 * Priority 200 (highest) ensures spline-based cameras take precedence
 * over all other rotation modules during scripted sequences.
 */
bool UCameraModule_R09_Spline_Rotation::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if current state is cinematic
	// Spline rotation is typically used for cutscenes and scripted sequences
	if (!StateConfig.StateBase.Flags.bIsCinematic)
	{
		return false;
	}
	
	// TODO: When spline system is implemented, add check:
	// if (!StateConfig.Module.Rotation.SplineComponent.IsValid())
	// {
	//     return false;
	// }
	
	// For now, activate for all cinematic states
	// This allows testing the module even without a spline system
	return true;
}

/**
 * Compute - Calculate spline-based rotation (placeholder implementation)
 * 
 * CURRENT IMPLEMENTATION (Placeholder):
 * Since the full spline system is not yet integrated, this module provides
 * a placeholder implementation that simulates spline-like rotation behavior
 * by looking at the character with a sinusoidal yaw offset.
 * 
 * This allows:
 * - Testing the module activation/deactivation
 * - Verifying the pipeline integration
 * - Demonstrating override behavior
 * 
 * FUTURE IMPLEMENTATION (When Spline System Available):
 * 1. Get SplineComponent reference from StateConfig
 * 2. Get current position along spline (from P07 or external system)
 * 3. Calculate rotation options:
 *    a) Use spline tangent direction (camera looks along path)
 *    b) Look at specific target while moving along spline
 *    c) Use custom rotation curve defined on spline
 * 4. Apply rotation with optional smoothing
 * 5. Output the calculated rotation
 * 
 * Placeholder Behavior:
 * - Looks at character (like other look-at modules)
 * - Adds sinusoidal yaw offset to simulate movement
 * - Provides predictable, testable behavior
 * - Demonstrates Override policy
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_R09_Spline_Rotation::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// ========================================
	// PLACEHOLDER IMPLEMENTATION
	// ========================================
	// This section simulates spline rotation behavior until the real
	// spline system is integrated. It creates a look-at rotation with
	// animated yaw offset.
	
	// Get camera focus point (from Position modules)
	FVector CameraFocusPoint = Context.Output.FocusPoint;
	
	// Fallback if invalid
	if (CameraFocusPoint.IsNearlyZero())
	{
		CameraFocusPoint = Context.InputContext.CharacterLocation;
		CameraFocusPoint.Z += 100.0f;
	}
	
	// Get character location as look-at target
	FVector CharacterLocation = Context.InputContext.CharacterLocation;
	
	// Calculate look direction towards character
	FVector LookDirection = (CharacterLocation - CameraFocusPoint).GetSafeNormal();
	
	// Handle edge case: if camera is at character position
	if (LookDirection.IsNearlyZero())
	{
		// Use character's forward direction as fallback
		LookDirection = Context.InputContext.CharacterRotation.Vector();
	}
	
	// Convert to rotation
	FRotator BaseLookAtRotation = LookDirection.Rotation();
	
	// Simulate spline rotation with sinusoidal yaw offset
	// In real implementation, this would come from spline tangent or custom curve
	// 
	// Using GameTime to create animated effect (simulates camera moving along path)
	float GameTime = Context.InputContext.GameTime;
	
	// Normalize time to 0-1 range (10 second cycle)
	float SimulatedSplineT = FMath::Fmod(GameTime * 0.1f, 1.0f);
	
	// Calculate yaw offset using sine wave (-15 to +15 degrees)
	// This simulates camera panning left-right as it moves along spline
	const float MaxYawOffset = 15.0f;
	float YawOffset = FMath::Sin(SimulatedSplineT * PI * 2.0f) * MaxYawOffset;
	
	// Apply offset to base rotation
	FRotator SimulatedSplineRotation = BaseLookAtRotation;
	SimulatedSplineRotation.Yaw += YawOffset;
	
	// Clamp pitch to configured limits
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	if (MinPitch >= MaxPitch)
	{
		MinPitch = -80.0f;
		MaxPitch = 80.0f;
	}
	
	SimulatedSplineRotation.Pitch = FMath::Clamp(SimulatedSplineRotation.Pitch, MinPitch, MaxPitch);
	
	// Force roll to zero
	SimulatedSplineRotation.Roll = 0.0f;
	
	// Store in member variable (for debugging/reference)
	SplineRotation = SimulatedSplineRotation;
	
	// ========================================
	// END PLACEHOLDER IMPLEMENTATION
	// ========================================
	
	// Fill output structure
	OutOutput.RotationOutput = SplineRotation;
	OutOutput.bHasRotationOutput = true;
	
	// Optional debug logging (use VeryVerbose to avoid spam)
	UE_LOG(LogTemp, VeryVerbose, TEXT("R09 Spline Rotation: Using placeholder (Yaw offset: %.1f degrees)"), YawOffset);
	
	return true;
}
