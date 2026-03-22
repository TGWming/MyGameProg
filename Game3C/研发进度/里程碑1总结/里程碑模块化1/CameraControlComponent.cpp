// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraControlComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UCameraControlComponent::UCameraControlComponent()
{
	// Set this component to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// ==================== 初始化默认配置 ====================
	CameraSettings = FCameraSettings(); // 使用LockOnConfig中的默认值
	AdvancedCameraSettings = FAdvancedCameraSettings(); // 使用LockOnConfig中的默认值
	SocketProjectionSettings = FSocketProjectionSettings(); // 使用LockOnConfig中的默认值

	// ==================== 初始化状态变量 ====================
	CurrentCameraState = ECameraState::Normal;
	CurrentLockOnTarget = nullptr;
	PreviousLockOnTarget = nullptr;
	
	// 相机跟随控制初始化
	bShouldCameraFollowTarget = true;
	bShouldCharacterRotateToTarget = true;
	bPlayerIsMoving = false;

	// 平滑切换状态初始化
	bIsSmoothSwitching = false;
	SmoothSwitchStartTime = 0.0f;
	SmoothSwitchStartRotation = FRotator::ZeroRotator;
	SmoothSwitchTargetRotation = FRotator::ZeroRotator;
	bShouldSmoothSwitchCamera = false;
	bShouldSmoothSwitchCharacter = false;

	// 自动修正状态初始化
	bIsCameraAutoCorrection = false;
	CameraCorrectionStartTime = 0.0f;
	CameraCorrectionStartRotation = FRotator::ZeroRotator;
	CameraCorrectionTargetRotation = FRotator::ZeroRotator;
	DelayedCorrectionTarget = nullptr;

	// 相机重置状态初始化
	bIsSmoothCameraReset = false;
	SmoothResetStartTime = 0.0f;
	SmoothResetStartRotation = FRotator::ZeroRotator;
	SmoothResetTargetRotation = FRotator::ZeroRotator;

	// 高级距离自适应初始化
	bIsAdvancedCameraAdjustment = false;
	LastAdvancedAdjustmentTime = 0.0f;
	CurrentTargetSizeCategory = EEnemySizeCategory::Unknown;
	CurrentTargetDistance = 0.0f;

	// 调试控制初始化
	bEnableCameraDebugLogs = false;
	bEnableAdvancedAdjustmentDebugLogs = false;
}

// Called when the game starts
void UCameraControlComponent::BeginPlay()
{
	Super::BeginPlay();

	// 验证拥有者是否为角色
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("CameraControlComponent: Owner is not a Character! Component requires Character owner."));
		return;
	}

	// 验证控制器
	APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraControlComponent: No PlayerController found on owner. Some features may not work."));
	}

	// 验证相机组件
	USpringArmComponent* SpringArm = GetSpringArmComponent();
	UCameraComponent* Camera = GetCameraComponent();
	
	if (!SpringArm)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraControlComponent: No SpringArmComponent found on owner. Camera control may not work properly."));
	}
	
	if (!Camera)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraControlComponent: No CameraComponent found on owner. Camera control may not work properly."));
	}

	// 设置初始相机状态
	UpdateCameraState(ECameraState::Normal);

	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraControlComponent: Successfully initialized for %s"), 
			*OwnerCharacter->GetName());
	}
}

// Called every frame
void UCameraControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 优先级处理：平滑相机重置（最高优先级，因为是非锁定状态）
	if (bIsSmoothCameraReset)
	{
		UpdateSmoothCameraReset();
		return; // 重置期间不执行其他相机逻辑
	}

	// 优先级处理：相机自动修正
	if (bIsCameraAutoCorrection)
	{
		UpdateCameraAutoCorrection();
	}

	// 锁定状态的相机更新
	if (CurrentLockOnTarget && IsValid(CurrentLockOnTarget))
	{
		// 更新平滑切换状态（优先级高于普通相机更新）
		if (bIsSmoothSwitching)
		{
			UpdateSmoothTargetSwitch();
		}
		else if (!bIsCameraAutoCorrection) // 只在非自动修正状态下才执行普通相机更新
		{
			UpdateLockOnCamera();
		}

		// 高级距离自适应相机调整
		if (AdvancedCameraSettings.bEnableDistanceAdaptiveCamera)
		{
			UpdateAdvancedCameraAdjustment();
		}
	}

	// 相机状态调试信息 - 高频日志优化
	if (bEnableCameraDebugLogs && CurrentLockOnTarget)
	{
		static float LastDebugTime = 0.0f;
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastDebugTime > 2.0f) // 每2秒记录一次，避免日志泛滥
		{
			// 高频日志优化 - 原始代码保留但注释
			// UE_LOG(LogTemp, VeryVerbose, TEXT("CameraControl: State=%s, Target=%s, Follow=%s, Rotate=%s"), 
			//	*UEnum::GetValueAsString(CurrentCameraState),
			//	*CurrentLockOnTarget->GetName(),
			//	bShouldCameraFollowTarget ? TEXT("YES") : TEXT("NO"),
			//	bShouldCharacterRotateToTarget ? TEXT("YES") : TEXT("NO"));
			LastDebugTime = CurrentTime;
		}
	}
}

// ==================== 配置接口实现 ====================

void UCameraControlComponent::SetCameraSettings(const FCameraSettings& Settings)
{
	CameraSettings = Settings;
	
	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraControlComponent: Camera settings updated"));
		UE_LOG(LogTemp, Log, TEXT("- InterpSpeed: %.1f"), CameraSettings.CameraInterpSpeed);
		UE_LOG(LogTemp, Log, TEXT("- SmoothTracking: %s"), CameraSettings.bEnableSmoothCameraTracking ? TEXT("ON") : TEXT("OFF"));
		UE_LOG(LogTemp, Log, TEXT("- TrackingMode: %d"), CameraSettings.CameraTrackingMode);
	}
}

void UCameraControlComponent::SetAdvancedCameraSettings(const FAdvancedCameraSettings& Settings)
{
	AdvancedCameraSettings = Settings;
	
	if (bEnableAdvancedAdjustmentDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraControlComponent: Advanced camera settings updated"));
		UE_LOG(LogTemp, Log, TEXT("- DistanceAdaptive: %s"), AdvancedCameraSettings.bEnableDistanceAdaptiveCamera ? TEXT("ON") : TEXT("OFF"));
		UE_LOG(LogTemp, Log, TEXT("- TerrainCompensation: %s"), AdvancedCameraSettings.bEnableTerrainHeightCompensation ? TEXT("ON") : TEXT("OFF"));
		UE_LOG(LogTemp, Log, TEXT("- EnemySizeAdaptation: %s"), AdvancedCameraSettings.bEnableEnemySizeAdaptation ? TEXT("ON") : TEXT("OFF"));
	}
}

// ==================== 输入处理接口实现 ====================

void UCameraControlComponent::HandlePlayerInput(float TurnInput, float LookUpInput)
{
	// 检查是否应该中断自动控制
	if (ShouldInterruptAutoControl(TurnInput, LookUpInput))
	{
		// 停止自动相机控制
		if (bIsCameraAutoCorrection)
		{
			bIsCameraAutoCorrection = false;
			DelayedCorrectionTarget = nullptr;
			UpdateCameraState(ECameraState::LockedOn);
			
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player input detected - stopping camera auto correction"));
			}
		}
		
		if (bIsSmoothCameraReset)
		{
			bIsSmoothCameraReset = false;
			UpdateCameraState(ECameraState::Normal);
			
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player input detected - stopping smooth camera reset"));
			}
		}
	}
}

void UCameraControlComponent::HandlePlayerMovement(bool bIsMoving)
{
	bPlayerIsMoving = bIsMoving;

	// 如果玩家开始移动且处于锁定状态，启用相机跟随和身体转向
	if (bPlayerIsMoving && CurrentLockOnTarget)
	{
		// 如果正在进行平滑切换，立即停止并恢复正常跟随
		if (bIsSmoothSwitching)
		{
			bIsSmoothSwitching = false;
			bShouldSmoothSwitchCamera = false;
			bShouldSmoothSwitchCharacter = false;
			UpdateCameraState(ECameraState::LockedOn);
			
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player movement interrupted smooth target switch"));
			}
		}
		
		// 如果正在进行相机自动修正，立即停止
		if (bIsCameraAutoCorrection)
		{
			bIsCameraAutoCorrection = false;
			DelayedCorrectionTarget = nullptr;
			UpdateCameraState(ECameraState::LockedOn);
			
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player movement interrupted camera auto correction"));
			}
		}
		
		if (!bShouldCameraFollowTarget)
		{
			bShouldCameraFollowTarget = true;
			bShouldCharacterRotateToTarget = true;
			
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player started moving - enabling camera follow and character rotation"));
			}
		}
	}
}

// ==================== 核心相机函数实现 ====================

void UCameraControlComponent::UpdateLockOnCamera()
{
	if (!CurrentLockOnTarget || !IsValid(CurrentLockOnTarget))
	{
		// 高频日志优化 - 添加降频控制
		if (bEnableCameraDebugLogs)
		{
			static int32 FrameCounter = 0;
			if (++FrameCounter % 300 == 0) // 每5秒只记录一次
			{
				UE_LOG(LogTemp, Warning, TEXT("UpdateLockOnCamera: No valid target"));
			}
		}
		return;
	}

	APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
	{
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableCameraDebugLogs)
		// {
		//	UE_LOG(LogTemp, Error, TEXT("UpdateLockOnCamera: No PlayerController"));
		// }
		return;
	}

	// 只有在应该跟随目标时才更新相机
	if (!bShouldCameraFollowTarget)
	{
		if (bShouldCharacterRotateToTarget)
		{
			UpdateCharacterRotationToTarget();
		}
		return;
	}

	// ==================== 与蓝图一致的锁定相机逻辑 ====================
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return;

	// 获取玩家位置
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	
	// 获取目标的Socket或位置（优先使用Socket）
	FVector TargetLocation;
	if (SocketProjectionSettings.bUseSocketProjection && HasValidSocket(CurrentLockOnTarget))
	{
		// 使用Socket位置
		TargetLocation = GetTargetSocketWorldLocation(CurrentLockOnTarget);
		// 高频日志优化 - 原始代码保留但注释
		// if (bEnableCameraDebugLogs)
		// {
		//	UE_LOG(LogTemp, VeryVerbose, TEXT("LockOn Camera: Using socket '%s' at location (%.1f, %.1f, %.1f)"), 
		//		*SocketProjectionSettings.TargetSocketName.ToString(), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
		// }
	}
	else
	{
		// 使用Actor位置作为备选
		TargetLocation = CurrentLockOnTarget->GetActorLocation();
	}

	// 应用目标位置偏移（对应蓝图中的Vector减法）
	TargetLocation += CameraSettings.TargetLocationOffset;

	// 高级相机调整：根据距离和敌人尺寸调整目标位置
	if (AdvancedCameraSettings.bEnableDistanceAdaptiveCamera || AdvancedCameraSettings.bEnableEnemySizeAdaptation)
	{
		float Distance = CalculateDistanceToTarget(CurrentLockOnTarget);
		EEnemySizeCategory SizeCategory = GetTargetSizeCategory(CurrentLockOnTarget);
		TargetLocation = CalculateAdvancedTargetLocation(CurrentLockOnTarget, SizeCategory, Distance);
	}

	// 计算玩家朝向目标的旋转
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);

	// 获取当前相机/控制器的旋转
	FRotator CurrentRotation = PlayerController->GetControlRotation();

	// 使用 FMath::RInterpTo 进行平滑插值
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FRotator NewRotation;
	
	if (CameraSettings.bEnableSmoothCameraTracking)
	{
		// 根据距离调整插值速度
		float Distance = CalculateDistanceToTarget(CurrentLockOnTarget);
		float SpeedMultiplier = GetCameraSpeedMultiplierForDistance(Distance);
		float AdjustedInterpSpeed = CameraSettings.CameraInterpSpeed * SpeedMultiplier;

		// 根据相机跟踪模式进行不同类型的插值
		switch (CameraSettings.CameraTrackingMode)
		{
		case 0: // 完全跟踪（Pitch + Yaw）
			NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, AdjustedInterpSpeed);
			break;
		case 1: // 仅水平跟踪（只有Yaw）
			{
				FRotator HorizontalLookAt = FRotator(CurrentRotation.Pitch, LookAtRotation.Yaw, CurrentRotation.Roll);
				NewRotation = FMath::RInterpTo(CurrentRotation, HorizontalLookAt, DeltaTime, AdjustedInterpSpeed);
			}
			break;
		case 2: // 自定义模式（可扩展）
		default:
			NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, AdjustedInterpSpeed);
			break;
		}
	}
	else
	{
		// 直接设置旋转，不进行插值
		NewRotation = LookAtRotation;
	}
	
	// 应用相机旋转
	PlayerController->SetControlRotation(NewRotation);

	// 调试信息（可控制开关） - 高频日志优化
	if (bEnableCameraDebugLogs)
	{
		static float LastDetailedLogTime = 0.0f;
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastDetailedLogTime > 3.0f) // 每3秒记录一次详细信息
		{
			float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - LookAtRotation.Yaw));
			// 高频日志优化 - 原始代码保留但注释
			// UE_LOG(LogTemp, VeryVerbose, TEXT("LockOn Camera: Target=%s, InterpSpeed=%.1f, AngleDiff=%.1f degrees"), 
			//	*CurrentLockOnTarget->GetName(), CameraSettings.CameraInterpSpeed, AngleDiff);
			LastDetailedLogTime = CurrentTime;
		}
	}

	// 根据设置决定是否让角色也朝向目标
	if (bShouldCharacterRotateToTarget)
	{
		FRotator CharacterRotation = FMath::RInterpTo(OwnerCharacter->GetActorRotation(), 
			FRotator(0, LookAtRotation.Yaw, 0), DeltaTime, CHARACTER_ROTATION_SPEED);
		OwnerCharacter->SetActorRotation(CharacterRotation);
	}
}

void UCameraControlComponent::UpdateCharacterRotationToTarget()
{
	if (!CurrentLockOnTarget || !IsValid(CurrentLockOnTarget))
		return;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return;

	// 计算朝向目标的旋转（仅Y轴旋转）
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = CurrentLockOnTarget->GetActorLocation();
	
	// 应用目标位置偏移（与相机逻辑保持一致）
	TargetLocation += CameraSettings.TargetLocationOffset;
	
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
	
	// 只更新角色的Y轴旋转，保持水平朝向
	FRotator CharacterRotation = FRotator(0, LookAtRotation.Yaw, 0);
	FRotator NewRotation = FMath::RInterpTo(OwnerCharacter->GetActorRotation(), CharacterRotation, 
		GetWorld()->GetDeltaSeconds(), CHARACTER_ROTATION_SPEED);
	
	OwnerCharacter->SetActorRotation(NewRotation);
}

void UCameraControlComponent::StartSmoothTargetSwitch(AActor* NewTarget)
{
	if (!NewTarget || !ValidateTarget(NewTarget))
	{
		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("StartSmoothTargetSwitch: Invalid target"));
		}
		return;
	}

	APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
		return;

	// 计算角度差异
	float AngleDifference = CalculateAngleToTarget(NewTarget);
	float Distance = CalculateDistanceToTarget(NewTarget);

	// 检查是否应该使用直接切换
	if (ShouldUseDirectSwitching(NewTarget, AngleDifference, Distance))
	{
		PerformDirectTargetSwitch(NewTarget);
		return;
	}

	if (AngleDifference <= TARGET_SWITCH_ANGLE_THRESHOLD)
	{
		// 角度差异小于阈值，只更新UI，不移动相机和角色
		bShouldCameraFollowTarget = false;
		bShouldCharacterRotateToTarget = false;
		bIsSmoothSwitching = false;
		UpdateCameraState(ECameraState::LockedOn);
	}
	else
	{
		// 角度差异超过阈值，启动平滑切换
		bIsSmoothSwitching = true;
		bShouldSmoothSwitchCamera = true;
		bShouldSmoothSwitchCharacter = true;
		UpdateCameraState(ECameraState::SmoothSwitching);
		
		// 记录切换开始状态
		SmoothSwitchStartTime = GetWorld()->GetTimeSeconds();
		SmoothSwitchStartRotation = PlayerController->GetControlRotation();
		
		// 计算目标旋转
		FVector PlayerLocation = GetOwnerCharacter()->GetActorLocation();
		FVector TargetLocation = NewTarget->GetActorLocation();
		SmoothSwitchTargetRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
		
		// 临时禁用相机跟随
		bShouldCameraFollowTarget = false;
		bShouldCharacterRotateToTarget = false;

		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Started smooth target switch: Angle=%.1f degrees, Distance=%.1f"), 
				AngleDifference, Distance);
		}
	}

	// 更新目标引用
	PreviousLockOnTarget = CurrentLockOnTarget;
	CurrentLockOnTarget = NewTarget;
}

void UCameraControlComponent::UpdateSmoothTargetSwitch()
{
	if (!bIsSmoothSwitching || !CurrentLockOnTarget || !IsValid(CurrentLockOnTarget))
		return;

	APlayerController* PlayerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!PlayerController || !OwnerCharacter)
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 重新计算目标旋转（因为目标可能在移动）	
	FRotator CurrentTargetRotation = UKismetMathLibrary::FindLookAtRotation(
		OwnerCharacter->GetActorLocation(), CurrentLockOnTarget->GetActorLocation());

	// 平滑插值到目标旋转
	if (bShouldSmoothSwitchCamera)
	{
		FRotator CurrentRotation = PlayerController->GetControlRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, CurrentTargetRotation, 
			DeltaTime, TARGET_SWITCH_SMOOTH_SPEED);
		PlayerController->SetControlRotation(NewRotation);

		// 检查是否接近目标旋转
		if (IsInterpolationComplete(NewRotation, CurrentTargetRotation, LOCK_COMPLETION_THRESHOLD))
		{
			bShouldSmoothSwitchCamera = false;
		}
	}

	// 平滑旋转角色
	if (bShouldSmoothSwitchCharacter)
	{
		FRotator CharacterTargetRotation = FRotator(0, CurrentTargetRotation.Yaw, 0);
		FRotator CurrentCharacterRotation = OwnerCharacter->GetActorRotation();
		FRotator NewCharacterRotation = FMath::RInterpTo(CurrentCharacterRotation, CharacterTargetRotation, 
			DeltaTime, TARGET_SWITCH_SMOOTH_SPEED);
		OwnerCharacter->SetActorRotation(NewCharacterRotation);

		// 检查是否接近目标旋转
		if (IsInterpolationComplete(NewCharacterRotation, CharacterTargetRotation, LOCK_COMPLETION_THRESHOLD))
		{
			bShouldSmoothSwitchCharacter = false;
		}
	}

	// 检查是否完成所有平滑切换
	if (!bShouldSmoothSwitchCamera && !bShouldSmoothSwitchCharacter)
	{
		// 平滑切换完成，恢复正常的相机跟随状态
		bIsSmoothSwitching = false;
		bShouldCameraFollowTarget = true;
		bShouldCharacterRotateToTarget = true;
		UpdateCameraState(ECameraState::LockedOn);

		// 触发切换完成事件
		OnCameraSwitchCompleted.Broadcast(CurrentLockOnTarget);

		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Smooth target switch completed"));
		}
	}
}

void UCameraControlComponent::StartSmoothCameraReset()
{
	APlayerController* PlayerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!PlayerController || !OwnerCharacter)
		return;
		
	// 计算目标旋转（角色正前方，稍微向上）
	FRotator TargetRotation = OwnerCharacter->GetActorRotation() + FRotator(-10.0f, 0.0f, 0.0f);
	
	// 启动平滑重置
	bIsSmoothCameraReset = true;
	SmoothResetStartTime = GetWorld()->GetTimeSeconds();
	SmoothResetStartRotation = PlayerController->GetControlRotation();
	SmoothResetTargetRotation = TargetRotation;
	UpdateCameraState(ECameraState::SmoothReset);
	
	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Started smooth camera reset from %s to %s"), 
			*SmoothResetStartRotation.ToString(), *SmoothResetTargetRotation.ToString());
	}
}

void UCameraControlComponent::UpdateSmoothCameraReset()
{
	if (!bIsSmoothCameraReset)
		return;

	APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	// 平滑插值到目标旋转
	FRotator CurrentRotation = PlayerController->GetControlRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, SmoothResetTargetRotation, 
		DeltaTime, CAMERA_RESET_SPEED);
	
	PlayerController->SetControlRotation(NewRotation);

	// 检查是否完成重置
	if (IsInterpolationComplete(NewRotation, SmoothResetTargetRotation, CAMERA_RESET_ANGLE_THRESHOLD))
	{
		// 重置完成
		bIsSmoothCameraReset = false;
		PlayerController->SetControlRotation(SmoothResetTargetRotation); // 确保精确到达目标旋转
		UpdateCameraState(ECameraState::Normal);

		// 触发重置完成事件
		OnCameraResetCompleted.Broadcast();

		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Smooth camera reset completed"));
		}
	}
}

void UCameraControlComponent::StartCameraReset(const FRotator& TargetRotation)
{
	APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
		return;
		
	// 直接设置相机旋转
	PlayerController->SetControlRotation(TargetRotation);
	
	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Camera reset to target rotation: %s"), *TargetRotation.ToString());
	}
}

void UCameraControlComponent::PerformSimpleCameraReset()
{
	StartSmoothCameraReset();
}

void UCameraControlComponent::StartCameraAutoCorrection(AActor* Target)
{
	StartCameraCorrectionForTarget(Target);
}

void UCameraControlComponent::UpdateCameraAutoCorrection()
{
	if (!bIsCameraAutoCorrection)
		return;

	APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float CorrectionDuration = CurrentTime - CameraCorrectionStartTime;
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 平滑插值到修正目标旋转
	FRotator CurrentRotation = PlayerController->GetControlRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, CameraCorrectionTargetRotation, 
		DeltaTime, CAMERA_AUTO_CORRECTION_SPEED);
	
	PlayerController->SetControlRotation(NewRotation);

	// 检查是否完成修正
	if (IsInterpolationComplete(NewRotation, CameraCorrectionTargetRotation, 2.0f) || CorrectionDuration > 1.5f)
	{
		bIsCameraAutoCorrection = false;
		UpdateCameraState(CurrentLockOnTarget ? ECameraState::LockedOn : ECameraState::Normal);
		
		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Camera auto correction completed (Duration: %.1fs)"), CorrectionDuration);
		}
		
		// 修正完成后，清理状态
		DelayedCorrectionTarget = nullptr;
	}
}

void UCameraControlComponent::StartCameraCorrectionForTarget(AActor* Target)
{
	if (!Target || !ValidateTarget(Target))
	{
		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("StartCameraCorrectionForTarget: Invalid target"));
		}
		return;
	}

	APlayerController* PlayerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!PlayerController || !OwnerCharacter)
		return;

	// 计算朝向目标的旋转
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
	
	// 启动相机自动修正
	bIsCameraAutoCorrection = true;
	CameraCorrectionStartTime = GetWorld()->GetTimeSeconds();
	CameraCorrectionStartRotation = PlayerController->GetControlRotation();
	UpdateCameraState(ECameraState::AutoCorrection);
	
	// 计算修正后的目标旋转
	FRotator CurrentRotation = PlayerController->GetControlRotation();
	float YawDifference = FRotator::NormalizeAxis(TargetRotation.Yaw - CurrentRotation.Yaw);
	
	// 计算修正方向和幅度
	float CorrectionAmount;
	if (FMath::Abs(YawDifference) <= CAMERA_CORRECTION_OFFSET)
	{
		// 如果角度差异小于修正偏移量，直接对准目标
		CorrectionAmount = YawDifference;
	}
	else
	{
		// 如果角度差异较大，只修正一部分
		CorrectionAmount = FMath::Sign(YawDifference) * CAMERA_CORRECTION_OFFSET;
	}
	
	CameraCorrectionTargetRotation = FRotator(
		CurrentRotation.Pitch,
		CurrentRotation.Yaw + CorrectionAmount,
		CurrentRotation.Roll
	);

	// 触发修正开始事件
	OnCameraCorrectionStarted.Broadcast(Target);

	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Started camera correction for target: %s (Correction: %.1f degrees)"), 
			*Target->GetName(), CorrectionAmount);
	}
}

void UCameraControlComponent::DelayedCameraCorrection()
{
	if (!DelayedCorrectionTarget || !ValidateTarget(DelayedCorrectionTarget))
	{
		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("DelayedCameraCorrection: Invalid delayed target"));
		}
		return;
	}

	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Executing delayed camera correction for target: %s"), 
			*DelayedCorrectionTarget->GetName());
	}

	StartCameraCorrectionForTarget(DelayedCorrectionTarget);
}

// ==================== 高级距离自适应功能实现 ====================

void UCameraControlComponent::UpdateAdvancedCameraAdjustment()
{
	if (!CurrentLockOnTarget || !IsValid(CurrentLockOnTarget))
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAdvancedAdjustmentTime < ADVANCED_ADJUSTMENT_INTERVAL)
		return; // 限制更新频率

	LastAdvancedAdjustmentTime = CurrentTime;

	// 计算当前状态
	float Distance = CalculateDistanceToTarget(CurrentLockOnTarget);
	EEnemySizeCategory SizeCategory = GetTargetSizeCategory(CurrentLockOnTarget);

	// 检查是否需要更新
	bool bNeedUpdate = (SizeCategory != CurrentTargetSizeCategory) || 
					   (FMath::Abs(Distance - CurrentTargetDistance) > 50.0f); // 距离变化超过50单位

	if (bNeedUpdate)
	{
		CurrentTargetSizeCategory = SizeCategory;
		CurrentTargetDistance = Distance;

		// 应用高级调整
		if (AdvancedCameraSettings.bEnableDistanceAdaptiveCamera || 
			AdvancedCameraSettings.bEnableEnemySizeAdaptation)
		{
			FVector AdjustedLocation = CalculateAdvancedTargetLocation(CurrentLockOnTarget, SizeCategory, Distance);
			
			// 触发高级调整事件
			OnAdvancedCameraAdjusted.Broadcast(SizeCategory, Distance, AdjustedLocation);

			// 高频日志优化 - 添加降频控制
			if (bEnableAdvancedAdjustmentDebugLogs)
			{
				static int32 AdjustmentFrameCounter = 0;
				if (++AdjustmentFrameCounter % 180 == 0) // 每3秒只记录一次
				{
					UE_LOG(LogTemp, Log, TEXT("Advanced camera adjustment: Size=%s, Distance=%.1f, Location=(%.1f,%.1f,%.1f)"), 
						*UEnum::GetValueAsString(SizeCategory), Distance, 
						AdjustedLocation.X, AdjustedLocation.Y, AdjustedLocation.Z);
				}
			}
		}
	}
}

FVector UCameraControlComponent::CalculateAdvancedTargetLocation(AActor* Target, EEnemySizeCategory SizeCategory, float Distance)
{
	if (!Target || !IsValid(Target))
		return FVector::ZeroVector;

	// 获取基础目标位置
	FVector BaseLocation = Target->GetActorLocation();

	// 应用Socket偏移（如果使用Socket）
	if (SocketProjectionSettings.bUseSocketProjection && HasValidSocket(Target))
	{
		BaseLocation = GetTargetSocketWorldLocation(Target);
	}

	// 应用基础位置偏移
	BaseLocation += CameraSettings.TargetLocationOffset;

	// 应用敌人尺寸相关的高度偏移
	if (AdvancedCameraSettings.bEnableEnemySizeAdaptation)
	{
		FVector SizeOffset = GetHeightOffsetForEnemySize(SizeCategory);
		BaseLocation += SizeOffset;
	}

	// 应用地形高度补偿
	if (AdvancedCameraSettings.bEnableTerrainHeightCompensation)
	{
		BaseLocation = ApplyTerrainHeightCompensation(BaseLocation, Target);
	}

	return BaseLocation;
}

bool UCameraControlComponent::ShouldUseDirectSwitching(AActor* Target, float AngleDifference, float Distance)
{
	if (!AdvancedCameraSettings.bEnableDistanceAdaptiveCamera)
		return false;

	// 检查角度和距离条件
	bool bAngleCondition = AngleDifference <= AdvancedCameraSettings.DirectSwitchAngleThreshold;
	bool bDistanceCondition = Distance <= AdvancedCameraSettings.DirectSwitchDistanceThreshold;

	return bAngleCondition && bDistanceCondition;
}

void UCameraControlComponent::PerformDirectTargetSwitch(AActor* NewTarget)
{
	if (!NewTarget || !ValidateTarget(NewTarget))
		return;

	// 直接切换目标，不进行平滑动画
	PreviousLockOnTarget = CurrentLockOnTarget;
	CurrentLockOnTarget = NewTarget;
	
	// 保持正常的相机跟随状态
	bShouldCameraFollowTarget = true;
	bShouldCharacterRotateToTarget = true;
	bIsSmoothSwitching = false;

	// 立即触发切换完成事件
	OnCameraSwitchCompleted.Broadcast(NewTarget);

	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Performed direct target switch to: %s"), *NewTarget->GetName());
	}
}

// ==================== 状态管理函数实现 ====================

void UCameraControlComponent::SetLockOnTarget(AActor* Target)
{
	if (Target && !ValidateTarget(Target))
	{
		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetLockOnTarget: Invalid target provided"));
		}
		return;
	}

	PreviousLockOnTarget = CurrentLockOnTarget;
	CurrentLockOnTarget = Target;

	if (Target)
	{
		UpdateCameraState(ECameraState::LockedOn);
		bShouldCameraFollowTarget = true;
		bShouldCharacterRotateToTarget = true;
	}
	else
	{
		UpdateCameraState(ECameraState::Normal);
		bShouldCameraFollowTarget = true;
		bShouldCharacterRotateToTarget = true;
	}

	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Lock-on target set to: %s"), Target ? *Target->GetName() : TEXT("None"));
	}
}

void UCameraControlComponent::ClearLockOnTarget()
{
	SetLockOnTarget(nullptr);
}

void UCameraControlComponent::SetCameraFollowState(bool bShouldFollow, bool bShouldRotateCharacter)
{
	bShouldCameraFollowTarget = bShouldFollow;
	bShouldCharacterRotateToTarget = bShouldRotateCharacter;

	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Camera follow state updated: Follow=%s, Rotate=%s"), 
			bShouldFollow ? TEXT("ON") : TEXT("OFF"),
			bShouldRotateCharacter ? TEXT("ON") : TEXT("OFF"));
	}
}

void UCameraControlComponent::RestoreCameraFollowState()
{
	bShouldCameraFollowTarget = true;
	bShouldCharacterRotateToTarget = true;
	
	// 停止自动修正
	bIsCameraAutoCorrection = false;
	
	// 清空延迟修正目标
	DelayedCorrectionTarget = nullptr;

	// 更新相机状态
	UpdateCameraState(CurrentLockOnTarget ? ECameraState::LockedOn : ECameraState::Normal);

	if (bEnableCameraDebugLogs)
	{
		UE_LOG(LogTemp, Log, TEXT("Camera follow state restored"));
	}
}

// ==================== 内部辅助函数实现 ====================

ACharacter* UCameraControlComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

APlayerController* UCameraControlComponent::GetOwnerController() const
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return nullptr;
		
	return Cast<APlayerController>(OwnerCharacter->GetController());
}

USpringArmComponent* UCameraControlComponent::GetSpringArmComponent() const
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return nullptr;
		
	return OwnerCharacter->FindComponentByClass<USpringArmComponent>();
}

UCameraComponent* UCameraControlComponent::GetCameraComponent() const
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return nullptr;
		
	return OwnerCharacter->FindComponentByClass<UCameraComponent>();
}

float UCameraControlComponent::CalculateAngleToTarget(AActor* Target) const
{
	if (!Target || !IsValid(Target))
		return 180.0f;

	APlayerController* PlayerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!PlayerController || !OwnerCharacter)
		return 180.0f;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector CurrentCameraForward = PlayerController->GetControlRotation().Vector();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算角度差异
	float DotProduct = FVector::DotProduct(CurrentCameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}

float UCameraControlComponent::CalculateDirectionAngle(AActor* Target) const
{
	if (!Target || !IsValid(Target))
		return 0.0f;

	APlayerController* PlayerController = GetOwnerController();
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!PlayerController || !OwnerCharacter)
		return 0.0f;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FRotator ControlRotation = PlayerController->GetControlRotation();
	FVector CurrentCameraForward = ControlRotation.Vector();
	FVector CurrentCameraRight = ControlRotation.RotateVector(FVector::RightVector);

	// 计算到目标的向量
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算相对于相机前方向的角度
	float ForwardDot = FVector::DotProduct(CurrentCameraForward, ToTarget);
	float RightDot = FVector::DotProduct(CurrentCameraRight, ToTarget);

	// 使用atan2计算角度，范围是-180到180
	float AngleRadians = FMath::Atan2(RightDot, ForwardDot);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}

FVector UCameraControlComponent::GetTargetSocketWorldLocation(AActor* Target) const
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	// 尝试获取目标的骨骼网格组件
	USkeletalMeshComponent* SkeletalMesh = Target->FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh && SkeletalMesh->DoesSocketExist(SocketProjectionSettings.TargetSocketName))
	{
		// 如果Socket存在，返回Socket的世界位置加上偏移
		FVector SocketLocation = SkeletalMesh->GetSocketLocation(SocketProjectionSettings.TargetSocketName);
		return SocketLocation + SocketProjectionSettings.SocketOffset;
	}

	// 如果没有Socket，返回Actor的位置加上偏移
	return Target->GetActorLocation() + SocketProjectionSettings.SocketOffset;
}

bool UCameraControlComponent::HasValidSocket(AActor* Target) const
{
	if (!Target)
		return false;

	// 检查目标是否有指定的Socket
	USkeletalMeshComponent* SkeletalMesh = Target->FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh)
	{
		return SkeletalMesh->DoesSocketExist(SocketProjectionSettings.TargetSocketName);
	}

	return false;
}

EEnemySizeCategory UCameraControlComponent::GetTargetSizeCategory(AActor* Target) const
{
	if (!Target || !IsValid(Target))
		return EEnemySizeCategory::Unknown;

	// 计算目标的边界盒尺寸
	FVector Origin, BoxExtent;
	Target->GetActorBounds(false, Origin, BoxExtent);
	
	// 使用最大尺寸作为判断标准
	float MaxSize = FMath::Max3(BoxExtent.X, BoxExtent.Y, BoxExtent.Z) * 2.0f; // BoxExtent是半尺寸
	
	// 根据配置的阈值分类
	if (MaxSize <= AdvancedCameraSettings.SmallEnemySizeThreshold)
	{
		return EEnemySizeCategory::Small;
	}
	else if (MaxSize <= AdvancedCameraSettings.LargeEnemySizeThreshold)
	{
		return EEnemySizeCategory::Medium;
	}
	else
	{
		return EEnemySizeCategory::Large;
	}
}

float UCameraControlComponent::CalculateDistanceToTarget(AActor* Target) const
{
	if (!Target || !IsValid(Target))
		return 0.0f;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return 0.0f;

	return FVector::Dist(OwnerCharacter->GetActorLocation(), Target->GetActorLocation());
}

float UCameraControlComponent::GetCameraSpeedMultiplierForDistance(float Distance) const
{
	if (!AdvancedCameraSettings.bEnableDistanceAdaptiveCamera)
		return 1.0f;

	// 根据距离范围返回相应的速度倍数
	if (Distance <= AdvancedCameraSettings.CloseRangeThreshold)
	{
		return AdvancedCameraSettings.CloseRangeCameraSpeedMultiplier;
	}
	else if (Distance <= AdvancedCameraSettings.MediumRangeThreshold)
	{
		return AdvancedCameraSettings.MediumRangeCameraSpeedMultiplier;
	}
	else if (Distance <= AdvancedCameraSettings.FarRangeThreshold)
	{
		return AdvancedCameraSettings.FarRangeCameraSpeedMultiplier;
	}
	else
	{
		// 超远距离，使用远距离倍数
		return AdvancedCameraSettings.FarRangeCameraSpeedMultiplier;
	}
}

FVector UCameraControlComponent::GetHeightOffsetForEnemySize(EEnemySizeCategory SizeCategory) const
{
	switch (SizeCategory)
	{
	case EEnemySizeCategory::Small:
		return AdvancedCameraSettings.SmallEnemyHeightOffset;
	case EEnemySizeCategory::Medium:
		return AdvancedCameraSettings.MediumEnemyHeightOffset;
	case EEnemySizeCategory::Large:
		return AdvancedCameraSettings.LargeEnemyHeightOffset;
	default:
		return FVector::ZeroVector;
	}
}

void UCameraControlComponent::UpdateCameraState(ECameraState NewState)
{
	if (CurrentCameraState != NewState)
	{
		ECameraState OldState = CurrentCameraState;
		CurrentCameraState = NewState;

		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Camera state changed: %s -> %s"), 
				*UEnum::GetValueAsString(OldState),
				*UEnum::GetValueAsString(NewState));
		}
	}
}

bool UCameraControlComponent::ShouldInterruptAutoControl(float TurnInput, float LookUpInput) const
{
	// 检测玩家是否有明显的相机操作意图
	return (FMath::Abs(TurnInput) > 0.05f || FMath::Abs(LookUpInput) > 0.05f);
}

// ==================== 私有辅助函数实现 ====================

void UCameraControlComponent::PerformCameraInterpolation(const FRotator& TargetRotation, float InterpSpeed)
{
	APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
		return;

	FRotator CurrentRotation = PlayerController->GetControlRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, 
		GetWorld()->GetDeltaSeconds(), InterpSpeed);
	
	PlayerController->SetControlRotation(NewRotation);
}

void UCameraControlComponent::PerformCharacterRotationInterpolation(const FRotator& TargetRotation, float InterpSpeed)
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return;

	FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, 
		GetWorld()->GetDeltaSeconds(), InterpSpeed);
	
	OwnerCharacter->SetActorRotation(NewRotation);
}

bool UCameraControlComponent::IsInterpolationComplete(const FRotator& Current, const FRotator& Target, float Threshold) const
{
	float YawDiff = FMath::Abs(FRotator::NormalizeAxis(Current.Yaw - Target.Yaw));
	float PitchDiff = FMath::Abs(FRotator::NormalizeAxis(Current.Pitch - Target.Pitch));
	
	return (YawDiff < Threshold && PitchDiff < Threshold);
}

float UCameraControlComponent::NormalizeAngleDifference(float AngleDiff) const
{
	return FRotator::NormalizeAxis(AngleDiff);
}

float UCameraControlComponent::CalculateDistanceWeight(float Distance, float MaxDistance) const
{
	if (MaxDistance <= 0.0f)
		return 1.0f;
		
	return FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
}

FVector UCameraControlComponent::ApplyTerrainHeightCompensation(const FVector& BaseLocation, AActor* Target) const
{
	if (!Target || !IsValid(Target))
		return BaseLocation;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
		return BaseLocation;

	// 计算高度差异
	float PlayerHeight = OwnerCharacter->GetActorLocation().Z;
	float TargetHeight = Target->GetActorLocation().Z;
	float HeightDifference = TargetHeight - PlayerHeight;

	// 如果高度差异超过阈值，应用补偿
	if (FMath::Abs(HeightDifference) > AdvancedCameraSettings.MaxTerrainHeightDifference)
	{
		float CompensationAmount = HeightDifference * AdvancedCameraSettings.TerrainCompensationFactor;
		return BaseLocation + FVector(0.0f, 0.0f, -CompensationAmount);
	}

	return BaseLocation;
}

bool UCameraControlComponent::ValidateTarget(AActor* Target) const
{
	if (!Target)
		return false;
		
	if (!IsValid(Target))
		return false;
		
	// 检查目标是否不是玩家自己
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (OwnerCharacter && Target == OwnerCharacter)
		return false;
		
	return true;
}