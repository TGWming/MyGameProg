// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Engine.h"
#include "UObject/StructOnScope.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ==================== 初始化变量 ====================
	bIsLockedOn = false;
	CurrentLockOnTarget = nullptr;
	LockOnRange = 2000.0f;  // 增加到20米，更接近黑魂3的体验
	LockOnAngle = 120.0f;   // 增加视野角度到±60度，更容易锁定
	NormalWalkSpeed = 600.0f;
	LockedWalkSpeed = 600.0f;
	ForwardInputValue = 0.0f;
	RightInputValue = 0.0f;
	LastFindTargetsTime = 0.0f;
	
	// ==================== 新增：锁定相机控制参数初始化 ====================
	CameraInterpSpeed = 5.0f;           // 默认插值速度
	bEnableSmoothCameraTracking = true; // 默认启用平滑跟踪
	CameraTrackingMode = 0;             // 默认完全跟踪模式
	
	// 目标位置偏移初始化（对应蓝图中的Vector减法）
	TargetLocationOffset = FVector(0.0f, 0.0f, -250.0f);

	// ==================== 调试控制初始化 ====================
	bEnableCameraDebugLogs = false;     // 默认关闭相机调试
	bEnableLockOnDebugLogs = false;     // 默认关闭锁定调试
	
	// 输入状态初始化
	bRightStickLeftPressed = false;
	bRightStickRightPressed = false;
	LastRightStickX = 0.0f;
	
	// 目标切换状态初始化
	bJustSwitchedTarget = false;
	TargetSwitchCooldown = 0.5f;
	LastTargetSwitchTime = 0.0f;

	// 锁定切换平滑处理初始化
	bIsSmoothSwitching = false;
	SmoothSwitchStartTime = 0.0f;
	SmoothSwitchStartRotation = FRotator::ZeroRotator;
	SmoothSwitchTargetRotation = FRotator::ZeroRotator;
	bShouldSmoothSwitchCamera = false;
	bShouldSmoothSwitchCharacter = false;

	// 相机跟随控制初始化
	bShouldCameraFollowTarget = true;
	bShouldCharacterRotateToTarget = true; // 角色身体转向控制初始化
	bPlayerIsMoving = false;

	// 扇形锁定系统初始化
	bIsCameraAutoCorrection = false;
	CameraCorrectionStartTime = 0.0f;
	CameraCorrectionStartRotation = FRotator::ZeroRotator;
	CameraCorrectionTargetRotation = FRotator::ZeroRotator;
	DelayedCorrectionTarget = nullptr;

	// UMG相关初始化
	LockOnWidgetClass = nullptr;
	LockOnWidgetInstance = nullptr;
	PreviousLockOnTarget = nullptr;

	// 相机重置初始化
	bIsSmoothCameraReset = false;
	SmoothResetStartTime = 0.0f;
	SmoothResetStartRotation = FRotator::ZeroRotator;
	SmoothResetTargetRotation = FRotator::ZeroRotator;

	// ==================== Socket投射系统初始化 ====================
	TargetSocketName = TEXT("Spine2Socket");
	ProjectionScale = 1.0f;
	SocketOffset = FVector(0.0f, 0.0f, 50.0f);
	bUseSocketProjection = true; // 默认启用Socket投射

	// ==================== 创建相机组件 ====================
	// 创建弹簧臂组件
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // 相机距离
	CameraBoom->bUsePawnControlRotation = true; // 跟随控制器旋转

	// 创建相机组件
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 相机不独立旋转

	// ==================== 创建锁定检测球体 ====================
	LockOnDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LockOnDetectionSphere"));
	LockOnDetectionSphere->SetupAttachment(RootComponent);
	LockOnDetectionSphere->SetSphereRadius(LockOnRange);
	LockOnDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LockOnDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	LockOnDetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// ==================== 设置角色移动参数 ====================
	// 设置角色不随控制器旋转
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 配置角色移动组件
	GetCharacterMovement()->bOrientRotationToMovement = true; // 角色面向移动方向
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // 旋转速度
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 设置初始移动速度
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	
	// 更新检测球体半径
	if (LockOnDetectionSphere)
	{
		LockOnDetectionSphere->SetSphereRadius(LockOnRange);
	}
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新平滑相机重置（优先级最高，因为是非锁定状态）
	if (bIsSmoothCameraReset)
	{
		UpdateSmoothCameraReset();
		return; // 重置期间不执行其他相机逻辑
	}

	// 更新相机自动修正（优先级最高）
	if (bIsCameraAutoCorrection)
	{
		UpdateCameraAutoCorrection();
	}

	// 更新锁定状态
	if (bIsLockedOn)
	{
		UpdateLockOnTarget();
		
		// 更新平滑切换状态（优先级高于普通相机更新）
		if (bIsSmoothSwitching)
		{
			UpdateSmoothTargetSwitch();
		}
		else if (!bIsCameraAutoCorrection) // 只在非自动修正状态下才执行普通相机更新
		{
			// 核心锁定相机更新逻辑
			UpdateLockOnCamera();
		}
		
		// 更新UMG锁定UI（包括Socket投射）
		UpdateLockOnWidget();

		// 相机调试信息（可控制）
		if (bEnableCameraDebugLogs && CurrentLockOnTarget)
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("LockOn Active: Target=%s, CameraFollow=%s, CharacterRotate=%s"), 
				*CurrentLockOnTarget->GetName(),
				bShouldCameraFollowTarget ? TEXT("YES") : TEXT("NO"),
				bShouldCharacterRotateToTarget ? TEXT("YES") : TEXT("NO"));
		}
	}

	// 定期查找可锁定目标（降低频率以优化性能）
	// 在平滑切换期间暂停目标搜索，防止冲突
	if (!bIsSmoothSwitching)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastFindTargetsTime > TARGET_SEARCH_INTERVAL)
		{
			FindLockOnCandidates();
			LastFindTargetsTime = CurrentTime;

			// 锁定调试信息（可控制）
			if (bEnableLockOnDebugLogs)
			{
				UE_LOG(LogTemp, Verbose, TEXT("Target search completed: Found %d candidates"), LockOnCandidates.Num());
			}
		}
	}
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerInputComponent is null!"));
		return;
	}

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 绑定移动输入
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);

	// 绑定相机输入
	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

	// 绑定跳跃输入
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMyCharacter::StopJump);

	// ==================== 锁定输入绑定 ====================
	// 锁定输入 - 支持右摇杆按下和鼠标中键
	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AMyCharacter::HandleLockOnButton);
	
	// 右摇杆水平输入（用于切换目标）
	PlayerInputComponent->BindAxis("RightStickX", this, &AMyCharacter::HandleRightStickX);
	
	// 键盘左右箭头切换目标
	PlayerInputComponent->BindAction("SwitchTargetLeft", IE_Pressed, this, &AMyCharacter::SwitchLockOnTargetLeft);
	PlayerInputComponent->BindAction("SwitchTargetRight", IE_Pressed, this, &AMyCharacter::SwitchLockOnTargetRight);

	// ==================== 调试输入绑定 ====================
	// 添加调试按key (F键) 来测试输入
	PlayerInputComponent->BindAction("DebugInput", IE_Pressed, this, &AMyCharacter::DebugInputTest);
	
	// 打印当前绑定状态
	UE_LOG(LogTemp, Warning, TEXT("Input bindings set up successfully"));
}

// ==================== 移动函数实现 ====================
void AMyCharacter::MoveForward(float Value)
{
	ForwardInputValue = Value;

	// 检测玩家是否在移动（用于相机跟随逻辑）
	bPlayerIsMoving = (FMath::Abs(Value) > 0.1f || FMath::Abs(RightInputValue) > 0.1f);

	// 如果玩家开始移动且处于锁定状态，启用相机跟随和身体转向
	if (bPlayerIsMoving && bIsLockedOn)
	{
		// 如果正在进行平滑切换，立即停止并恢复正常跟随
		if (bIsSmoothSwitching)
		{
			bIsSmoothSwitching = false;
			bShouldSmoothSwitchCamera = false;
			bShouldSmoothSwitchCharacter = false;
			UE_LOG(LogTemp, Warning, TEXT("Player movement interrupted smooth target switch"));
		}
		
		// 如果正在进行相机自动修正，立即停止
		if (bIsCameraAutoCorrection)
		{
			bIsCameraAutoCorrection = false;
			DelayedCorrectionTarget = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Player movement interrupted camera auto correction"));
		}
		
		if (!bShouldCameraFollowTarget)
		{
			bShouldCameraFollowTarget = true;
			bShouldCharacterRotateToTarget = true; // 同时启用身体转向
			UE_LOG(LogTemp, Warning, TEXT("Player started moving - enabling camera follow and character rotation"));
		}
	}

	if (Value != 0.0f && Controller)
	{
		if (bIsLockedOn)
		{
			// 锁定状态下：相对于角色朝向移动
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, Value);
		}
		else
		{
			// 自由状态下：相对于相机朝向移动
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}
}

void AMyCharacter::MoveRight(float Value)
{
	RightInputValue = Value;

	// 检测玩家是否在移动（用于相机跟随逻辑）
	bPlayerIsMoving = (FMath::Abs(Value) > 0.1f || FMath::Abs(ForwardInputValue) > 0.1f);

	// 如果玩家开始移动且处于锁定状态，启用相机跟随和身体转向
	if (bPlayerIsMoving && bIsLockedOn)
	{
		// 如果正在进行平滑切换，立即停止并恢复正常跟随
		if (bIsSmoothSwitching)
		{
			bIsSmoothSwitching = false;
			bShouldSmoothSwitchCamera = false;
			bShouldSmoothSwitchCharacter = false;
			UE_LOG(LogTemp, Warning, TEXT("Player movement interrupted smooth target switch"));
		}
		
		// 如果正在进行相机自动修正，立即停止
		if (bIsCameraAutoCorrection)
		{
			bIsCameraAutoCorrection = false;
			DelayedCorrectionTarget = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Player movement interrupted camera auto correction"));
		}
		
		if (!bShouldCameraFollowTarget)
		{
			bShouldCameraFollowTarget = true;
			bShouldCharacterRotateToTarget = true; // 同时启用身体转向
			UE_LOG(LogTemp, Warning, TEXT("Player started moving - enabling camera follow and character rotation"));
		}
	}

	if (Value != 0.0f && Controller)
	{
		if (bIsLockedOn)
		{
			// 锁定状态下：相对于角色朝向移动
			const FVector Direction = GetActorRightVector();
			AddMovementInput(Direction, Value);
		}
		else
		{
			// 自由状态下：相对于相机朝向移动
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, Value);
		}
	}
}

void AMyCharacter::StartJump()
{
	Jump();
}

void AMyCharacter::StopJump()
{
	StopJumping();
}

// ==================== 相机控制 ====================
void AMyCharacter::Turn(float Rate)
{
	// 锁定状态下完全禁止玩家的水平相机输入
	if (bIsLockedOn)
	{
		// 在锁定状态下，仍然检测输入以停止自动相机控制
		if (FMath::Abs(Rate) > 0.1f)
		{
			// 玩家有相机操作意图，停止自动相机控制，但不添加输入
			if (bIsCameraAutoCorrection)
			{
				bIsCameraAutoCorrection = false;
				DelayedCorrectionTarget = nullptr;
				if (bEnableCameraDebugLogs)
				{
					UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping auto correction"));
				}
			}
			
			if (bIsSmoothCameraReset)
			{
				bIsSmoothCameraReset = false;
				if (bEnableCameraDebugLogs)
				{
					UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping smooth camera reset"));
				}
			}
			
			// 锁定状态下不添加相机输入，保持完全锁定
			// AddControllerYawInput(Rate); // 注释掉这行
		}
		return; // 锁定状态下直接返回
	}

	// 非锁定状态下的正常相机控制
	if (FMath::Abs(Rate) > 0.1f)
	{
		// 玩家有相机操作意图，停止自动相机控制
		if (bIsCameraAutoCorrection)
		{
			bIsCameraAutoCorrection = false;
			DelayedCorrectionTarget = nullptr;
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping auto correction"));
			}
		}
		
		if (bIsSmoothCameraReset)
		{
			bIsSmoothCameraReset = false;
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping smooth camera reset"));
			}
		}
	}
	
	// 非锁定状态下允许玩家控制相机
	AddControllerYawInput(Rate);
}

void AMyCharacter::LookUp(float Rate)
{
	// 锁定状态下完全禁止玩家的垂直相机输入
	if (bIsLockedOn)
	{
		// 在锁定状态下，仍然检测输入以停止自动相机控制
		if (FMath::Abs(Rate) > 0.1f)
		{
			// 玩家有相机操作意图，停止自动相机控制，但不添加输入
			if (bIsCameraAutoCorrection)
			{
				bIsCameraAutoCorrection = false;
				DelayedCorrectionTarget = nullptr;
				if (bEnableCameraDebugLogs)
				{
					UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping auto correction"));
				}
			}
			
			if (bIsSmoothCameraReset)
			{
				bIsSmoothCameraReset = false;
				if (bEnableCameraDebugLogs)
				{
					UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping smooth camera reset"));
				}
			}
			
			// 锁定状态下不添加相机输入，保持完全锁定
			// AddControllerPitchInput(Rate); // 注释掉这行
		}
		return; // 锁定状态下直接返回
	}

	// 非锁定状态下的正常相机控制
	if (FMath::Abs(Rate) > 0.1f)
	{
		// 玩家有相机操作意图，停止自动相机控制
		if (bIsCameraAutoCorrection)
		{
			bIsCameraAutoCorrection = false;
			DelayedCorrectionTarget = nullptr;
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping auto correction"));
			}
		}
		
		if (bIsSmoothCameraReset)
		{
			bIsSmoothCameraReset = false;
			if (bEnableCameraDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Player camera input detected - stopping smooth camera reset"));
			}
		}
	}
	
	// 非锁定状态下允许玩家控制相机
	AddControllerPitchInput(Rate);
}

// ==================== 锁定系统实现 ====================
void AMyCharacter::ToggleLockOn()
{
	if (bEnableLockOnDebugLogs)
	{
		UE_LOG(LogTemp, Warning, TEXT("ToggleLockOn called - Current state: %s"), 
			bIsLockedOn ? TEXT("LOCKED") : TEXT("UNLOCKED"));
	}
	
	if (bIsLockedOn)
	{
		// 额外的UI清理保险 - 在调用CancelLockOn之前强制清理所有UI
		if (bEnableLockOnDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Cancelling lock-on..."));
		}
		
		HideAllLockOnWidgets();
		if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport()) 
		{
			LockOnWidgetInstance->RemoveFromViewport();
			LockOnWidgetInstance = nullptr;
		}
		
		// 取消锁定的逻辑保持不变
		CancelLockOn();
	}
	else
	{
		if (bEnableLockOnDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Attempting to start lock-on..."));
		}
		
		FindLockOnCandidates();
		
		if (bEnableLockOnDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("Found %d lock-on candidates"), LockOnCandidates.Num());
		}
		
		// 只检查扇形区域内的目标
		AActor* SectorTarget = TryGetSectorLockTarget();
		
		if (SectorTarget)
		{
			if (bEnableLockOnDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("Found sector target: %s"), *SectorTarget->GetName());
			}
			// 情况1：敌人在扇形锁定范围内 → 进入锁定
			StartLockOn(SectorTarget);
		}
		else
		{
			if (bEnableLockOnDebugLogs)
			{
				UE_LOG(LogTemp, Log, TEXT("No valid sector target found, performing camera reset"));
			}
			// 情况2：无论是否有敌人，都执行镜头重置
			PerformSimpleCameraReset();
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== ToggleLockOn complete ==="));
}

// 新增：执行简单的相机重置（修改为平滑版本）
void AMyCharacter::PerformSimpleCameraReset()
{
	StartSmoothCameraReset();
}

// 新增：开始平滑相机重置
void AMyCharacter::StartSmoothCameraReset()
{
	if (!Controller)
		return;
		
	// 计算目标旋转（角色正前方，稍微向上）
	FRotator TargetRotation = GetActorRotation() + FRotator(-10.0f, 0.0f, 0.0f);
	
	// 启动平滑重置
	bIsSmoothCameraReset = true;
	SmoothResetStartTime = GetWorld()->GetTimeSeconds();
	SmoothResetStartRotation = Controller->GetControlRotation();
	SmoothResetTargetRotation = TargetRotation;
	
	UE_LOG(LogTemp, Warning, TEXT("Started smooth camera reset from %s to %s"), 
		*SmoothResetStartRotation.ToString(), *SmoothResetTargetRotation.ToString());
}

// 新增：更新平滑相机重置
void AMyCharacter::UpdateSmoothCameraReset()
{
	if (!bIsSmoothCameraReset || !Controller)
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	// 平滑插值到目标旋转
	FRotator CurrentRotation = Controller->GetControlRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, SmoothResetTargetRotation, 
		DeltaTime, CAMERA_RESET_SPEED);
	
	Controller->SetControlRotation(NewRotation);

	// 检查是否完成重置 - 计算角度差异
	float YawDiff = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - SmoothResetTargetRotation.Yaw));
	float PitchDiff = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Pitch - SmoothResetTargetRotation.Pitch));
	float TotalAngleDiff = YawDiff + PitchDiff;
	
	if (TotalAngleDiff < CAMERA_RESET_ANGLE_THRESHOLD)
	{
		// 重置完成
		bIsSmoothCameraReset = false;
		Controller->SetControlRotation(SmoothResetTargetRotation); // 确保精确到达目标旋转
		UE_LOG(LogTemp, Warning, TEXT("Smooth camera reset completed"));
	}
}

// 新增：开始锁定目标
void AMyCharacter::StartLockOn(AActor* Target)
{
	if (!Target)
		return;
		
	bIsLockedOn = true;
	CurrentLockOnTarget = Target;
	
	// 设置锁定状态下的移动速度
	GetCharacterMovement()->MaxWalkSpeed = LockedWalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	// 显示锁定UI
	ShowLockOnWidget();
	
	UE_LOG(LogTemp, Warning, TEXT("Started lock-on with target: %s"), *Target->GetName());
}

// 新增：取消锁定
void AMyCharacter::CancelLockOn()
{
	if (!bIsLockedOn)
		return;
	
	// 强制隐藏当前目标和所有候选目标的UI（优先级最高）
	HideAllLockOnWidgets();
	
	// 额外保险：如果有屏幕空间UI也隐藏
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->RemoveFromViewport();
		LockOnWidgetInstance = nullptr;
	}
	
	// 重置锁定状态
	bIsLockedOn = false;
	PreviousLockOnTarget = CurrentLockOnTarget;
	CurrentLockOnTarget = nullptr;
	
	// 重置相机跟随状态
	bShouldCameraFollowTarget = true;
	bShouldCharacterRotateToTarget = true;
	
	// 停止任何正在进行的平滑切换或相机修正
	bIsSmoothSwitching = false;
	bShouldSmoothSwitchCamera = false;
	bShouldSmoothSwitchCharacter = false;
	bIsCameraAutoCorrection = false;
	DelayedCorrectionTarget = nullptr;
	
	// 恢复正常移动设置
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	
	UE_LOG(LogTemp, Warning, TEXT("Lock-on cancelled"));
}

// 新增：重置相机
void AMyCharacter::ResetCamera()
{
	// 这个函数现在只是调用简单重置，保持兼容性
	PerformSimpleCameraReset();
}

// 移除 StartCameraReset() 的调用，因为这个函数不存在
// 如果需要，可以添加这个函数的实现：
void AMyCharacter::StartCameraReset(const FRotator& TargetRotation)
{
	if (!Controller)
		return;
		
	// 直接设置相机旋转，或者可以添加平滑插值
	Controller->SetControlRotation(TargetRotation);
	
	UE_LOG(LogTemp, Warning, TEXT("Camera reset to target rotation: %s"), *TargetRotation.ToString());
}

// ==================== 更新函数 ====================
void AMyCharacter::FindLockOnCandidates()
{
	LockOnCandidates.Empty();

	if (!LockOnDetectionSphere)
	{
		UE_LOG(LogTemp, Error, TEXT("LockOnDetectionSphere is null!"));
		return;
	}

	// 获取球体范围内的所有重叠Actor
	TArray<AActor*> OverlappingActors;
	LockOnDetectionSphere->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

	// 临时存储有效目标
	TArray<AActor*> ValidTargets;
	
	for (AActor* Actor : OverlappingActors)
	{
		if (IsValidLockOnTarget(Actor))
		{
			ValidTargets.Add(Actor);
		}
	}

	// 按角度排序有效目标 - 从左到右
	if (ValidTargets.Num() > 1)
	{
		SortCandidatesByDirection(ValidTargets);
	}

	// 将排序后的目标添加到候选列表
	LockOnCandidates = ValidTargets;

	UE_LOG(LogTemp, Verbose, TEXT("Lock-on candidates updated: %d targets available"), LockOnCandidates.Num());
}

bool AMyCharacter::IsValidLockOnTarget(AActor* Target)
{
	if (!Target || Target == this)
	{
		return false;
	}

	// 添加敌友识别检查
	if (Target->ActorHasTag(FName("Friendly")) || Target->ActorHasTag(FName("Player")))
	{
		return false;
	}

	// 检查目标是否为敌人（可选，如果你想只锁定特定敌人）
	// if (!Target->ActorHasTag(FName("Enemy")))
	// {
	//     return false;
	// }

	// 检查目标是否还活着（如果目标有生命值组件）	
	if (APawn* TargetPawn = Cast<APawn>(Target))
	{
		// 检查Pawn是否被销毁或无效
		if (!IsValid(TargetPawn) || TargetPawn->IsPendingKill())
		{
			return false;
		}

		// 如果有生命值组件，检查是否死亡
		// UHealthComponent* HealthComp = TargetPawn->FindComponentByClass<UHealthComponent>();
		// if (HealthComp && HealthComp->IsDead())
		// {
		//     return false;
		// }
	}

	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	// 检查距离
	float Distance = FVector::Dist(PlayerLocation, TargetLocation);
	if (Distance > LockOnRange)
	{
		return false;
	}

	// 检查视线遮挡
	FHitResult HitResult;
	FVector StartLocation = PlayerLocation + FVector(0, 0, RAYCAST_HEIGHT_OFFSET);
	FVector EndLocation = TargetLocation + FVector(0, 0, RAYCAST_HEIGHT_OFFSET);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// 如果有遮挡且击中的不是目标本身，则无效
	if (bHit && HitResult.GetActor() != Target)
	{
		return false;
	}

	return true;
}

bool AMyCharacter::IsTargetInSectorLockZone(AActor* Target) const
{
	if (!Target || !Controller)
		return false;

	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector CameraForward = Controller->GetControlRotation().Vector();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算角度
	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// 检查是否在扇形锁定区域内
	bool bInSectorZone = AngleDegrees <= (SECTOR_LOCK_ANGLE * 0.5f);

	return bInSectorZone;
}

bool AMyCharacter::IsTargetInEdgeDetectionZone(AActor* Target) const
{
	if (!Target || !Controller)
		return false;

	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector CameraForward = Controller->GetControlRotation().Vector();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算角度
	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// 检查是否在边缘检测区域内
	bool bInEdgeZone = (AngleDegrees > (SECTOR_LOCK_ANGLE * 0.5f)) && 
					   (AngleDegrees <= (EDGE_DETECTION_ANGLE * 0.5f));

	return bInEdgeZone;
}

AActor* AMyCharacter::GetBestTargetFromList(const TArray<AActor*>& TargetList)
{
	if (TargetList.Num() == 0)
		return nullptr;

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	FVector PlayerLocation = GetActorLocation();
	FVector CameraForward = Controller ? Controller->GetControlRotation().Vector() : GetActorForwardVector();

	for (AActor* Candidate : TargetList)
	{
		if (!IsValid(Candidate))
			continue;
			
		FVector ToTarget = (Candidate->GetActorLocation() - PlayerLocation).GetSafeNormal();
		float Distance = FVector::Dist(PlayerLocation, Candidate->GetActorLocation());
		
		// 防止除零错误
		if (LockOnRange <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("LockOnRange is zero or negative!"));
			continue;
		}
		
		// 使用相机前方向量计算点积
		float DotProduct = FVector::DotProduct(CameraForward, ToTarget);

		// 评分算法：角度因素占70%，距离因素占30%
		float NormalizedDistance = FMath::Sqrt(Distance / LockOnRange);
		float AngleFactor = DotProduct;
		float DistanceFactor = 1.0f - NormalizedDistance;
		
		float Score = (AngleFactor * 0.7f) + (DistanceFactor * 0.3f);

		// 排除几乎在相同位置的目标
		if (Distance < 50.0f)
		{
			Score -= 0.5f; // 减少评分，避免锁定到几乎重叠的目标
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

AActor* AMyCharacter::GetBestLockOnTarget()
{
	return GetBestSectorLockTarget();
}

AActor* AMyCharacter::GetBestSectorLockTarget()
{
	FindLockOnCandidates();

	if (LockOnCandidates.Num() == 0)
		return nullptr;

	// 首先尝试在扇形锁定区域内找目标
	TArray<AActor*> SectorTargets;
	TArray<AActor*> EdgeTargets;

	for (AActor* Candidate : LockOnCandidates)
	{
		if (!Candidate)
			continue;
			
		if (IsTargetInSectorLockZone(Candidate))
		{
			SectorTargets.Add(Candidate);
		}
		else if (IsTargetInEdgeDetectionZone(Candidate))
		{
			EdgeTargets.Add(Candidate);
		}
	}

	// 优先从扇形区域内选择目标
	if (SectorTargets.Num() > 0)
	{
		return GetBestTargetFromList(SectorTargets);
	}

	// 如果扇形区域内没有目标，检查边缘区域
	if (EdgeTargets.Num() > 0)
	{
		return GetBestTargetFromList(EdgeTargets);
	}

	return nullptr;
}

void AMyCharacter::UpdateLockOnTarget()
{
	if (!CurrentLockOnTarget)
	{
		bIsLockedOn = false;
		// 强制清理UI
		HideAllLockOnWidgets();
		if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
		{
			LockOnWidgetInstance->RemoveFromViewport();
			LockOnWidgetInstance = nullptr;
		}
		return;
	}

	// 检查当前锁定目标是否仍然有效
	if (!IsTargetStillLockable(CurrentLockOnTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Current lock-on target is no longer lockable - cancelling lock-on"));
		
		// 强制隐藏所有UI（使用更强的清理）
		HideAllLockOnWidgets();
		if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
		{
			LockOnWidgetInstance->RemoveFromViewport();
			LockOnWidgetInstance = nullptr;
		}
		
		// 然后取消锁定状态
		bIsLockedOn = false;
		PreviousLockOnTarget = CurrentLockOnTarget;
		CurrentLockOnTarget = nullptr;
		bShouldCameraFollowTarget = true;
		bShouldCharacterRotateToTarget = true;
		
		// 停止任何正在进行的平滑切换或相机修正
		bIsSmoothSwitching = false;
		bShouldSmoothSwitchCamera = false;
		bShouldSmoothSwitchCharacter = false;
		bIsCameraAutoCorrection = false;
		DelayedCorrectionTarget = nullptr;
		
		// 恢复正常移动设置
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	}
}

bool AMyCharacter::IsTargetStillLockable(AActor* Target)
{
	if (!Target || Target == this)
		return false;

	if (!IsValid(Target))
		return false;

	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	// 使用更大的距离范围来保持锁定
	float Distance = FVector::Dist(PlayerLocation, TargetLocation);
	float ExtendedLockOnRange = LockOnRange * EXTENDED_LOCK_RANGE_MULTIPLIER;
	
	if (Distance > ExtendedLockOnRange)
	{
		return false;
	}

	return true;
}

void AMyCharacter::UpdateLockOnCamera()
{
	if (!CurrentLockOnTarget || !Controller)
		return;

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
	// 1. 获取玩家控制器（已有）
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
		return;

	// 2. 获取玩家位置
	FVector PlayerLocation = GetActorLocation();
	
	// 3. 获取目标的Socket或位置（优先使用Socket）
	FVector TargetLocation;
	if (bUseSocketProjection && HasValidSocket(CurrentLockOnTarget))
	{
		// 使用Socket位置
		TargetLocation = GetTargetSocketWorldLocation(CurrentLockOnTarget);
		if (bEnableCameraDebugLogs)
		{
			UE_LOG(LogTemp, Log, TEXT("LockOn Camera: Using socket '%s' at location (%.1f, %.1f, %.1f)"), 
				*TargetSocketName.ToString(), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
		}
	}
	else
	{
		// 使用Actor位置作为备选
		TargetLocation = CurrentLockOnTarget->GetActorLocation();
	}

	// 应用目标位置偏移（对应蓝图中的Vector减法：敌人位置 - (0, 0, 250)）
	TargetLocation += TargetLocationOffset;

	// 4. 计算玩家朝向目标的旋转（等价于蓝图里的 FindLookAtRotation）
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);

	// 5. 获取当前相机/控制器的旋转
	FRotator CurrentRotation = Controller->GetControlRotation();

	// 6. 使用 FMath::RInterpTo 进行平滑插值（与蓝图一致的插值逻辑）
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FRotator NewRotation;
	
	if (bEnableSmoothCameraTracking)
	{
		// 根据相机跟踪模式进行不同类型的插值
		switch (CameraTrackingMode)
		{
		case 0: // 完全跟踪（Pitch + Yaw）
			NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, CameraInterpSpeed);
			break;
		case 1: // 仅水平跟踪（只有Yaw）
			{
				FRotator HorizontalLookAt = FRotator(CurrentRotation.Pitch, LookAtRotation.Yaw, CurrentRotation.Roll);
				NewRotation = FMath::RInterpTo(CurrentRotation, HorizontalLookAt, DeltaTime, CameraInterpSpeed);
			}
			break;
		case 2: // 自定义模式（可扩展）
		default:
			NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, CameraInterpSpeed);
			break;
		}
	}
	else
	{
		// 直接设置旋转，不进行插值
		NewRotation = LookAtRotation;
	}
	
	// 7. 调用 Controller->SetControlRotation 应用相机旋转
	Controller->SetControlRotation(NewRotation);

	// 调试信息（可控制开关）
	if (bEnableCameraDebugLogs)
	{
		float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - LookAtRotation.Yaw));
		UE_LOG(LogTemp, VeryVerbose, TEXT("LockOn Camera: Target=%s, InterpSpeed=%.1f, AngleDiff=%.1f degrees"), 
			*CurrentLockOnTarget->GetName(), CameraInterpSpeed, AngleDiff);
	}

	// 根据设置决定是否让角色也朝向目标
	if (bShouldCharacterRotateToTarget)
	{
		FRotator CharacterRotation = FMath::RInterpTo(GetActorRotation(), FRotator(0, LookAtRotation.Yaw, 0), 
			DeltaTime, CHARACTER_ROTATION_SPEED);
		SetActorRotation(CharacterRotation);
	}
}

void AMyCharacter::UpdateCharacterRotationToTarget()
{
	if (!CurrentLockOnTarget)
		return;

	// 计算朝向目标的旋转（仅Y轴旋转）
	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = CurrentLockOnTarget->GetActorLocation();
	
	// 应用目标位置偏移（与相机逻辑保持一致）
	TargetLocation += TargetLocationOffset;
	
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
	
	// 只更新角色的Y轴旋转，保持水平朝向
	FRotator CharacterRotation = FRotator(0, LookAtRotation.Yaw, 0);
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), CharacterRotation, 
		GetWorld()->GetDeltaSeconds(), CHARACTER_ROTATION_SPEED);
	
	SetActorRotation(NewRotation);
}

float AMyCharacter::CalculateAngleToTarget(AActor* Target) const
{
	if (!IsValid(Target) || !Controller)
		return 180.0f;

	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector CurrentCameraForward = Controller->GetControlRotation().Vector();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// 计算角度差异
	float DotProduct = FVector::DotProduct(CurrentCameraForward, ToTarget);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}

float AMyCharacter::CalculateDirectionAngle(AActor* Target) const
{
	if (!IsValid(Target) || !Controller)
		return 0.0f;

	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FRotator ControlRotation = Controller->GetControlRotation();
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

void AMyCharacter::SortCandidatesByDirection(TArray<AActor*>& Targets)
{
	if (Targets.Num() <= 1)
		return;

	// 移除无效指针
	Targets.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });

	if (Targets.Num() <= 1)
		return;

	// 使用Lambda表达式进行排序，从左到右（角度从小到大）
	Targets.Sort([this](const AActor& A, const AActor& B) {
		if (!IsValid(&A) || !IsValid(&B)) 
			return false;
		
		float AngleA = CalculateDirectionAngle(const_cast<AActor*>(&A));
		float AngleB = CalculateDirectionAngle(const_cast<AActor*>(&B));
		return AngleA < AngleB;
	});
}

void AMyCharacter::StartSmoothTargetSwitch(AActor* NewTarget)
{
	if (!NewTarget || !Controller)
		return;

	// 计算角度差异
	float AngleDifference = CalculateAngleToTarget(NewTarget);

	if (AngleDifference <= TARGET_SWITCH_ANGLE_THRESHOLD)
	{
		// 角度差异小于阈值，只更新UI，不移动相机和角色
		bShouldCameraFollowTarget = false;
		bShouldCharacterRotateToTarget = false;
		bIsSmoothSwitching = false;
	}
	else
	{
		// 角度差异超过阈值，启动平滑切换
		bIsSmoothSwitching = true;
		bShouldSmoothSwitchCamera = true;
		bShouldSmoothSwitchCharacter = true;
		
		// 记录切换开始状态
		SmoothSwitchStartTime = GetWorld()->GetTimeSeconds();
		SmoothSwitchStartRotation = Controller->GetControlRotation();
		
		// 计算目标旋转
		FVector PlayerLocation = GetActorLocation();
		FVector TargetLocation = NewTarget->GetActorLocation();
		SmoothSwitchTargetRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
		
		// 临时禁用相机跟随
		bShouldCameraFollowTarget = false;
		bShouldCharacterRotateToTarget = false;
	}
}

void AMyCharacter::UpdateSmoothTargetSwitch()
{
	if (!bIsSmoothSwitching || !Controller || !CurrentLockOnTarget)
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 重新计算目标旋转（因为目标可能在移动）	
	FRotator CurrentTargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentLockOnTarget->GetActorLocation());

	// 平滑插值到目标旋转
	if (bShouldSmoothSwitchCamera)
	{
		FRotator CurrentRotation = Controller->GetControlRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, CurrentTargetRotation, 
			DeltaTime, TARGET_SWITCH_SMOOTH_SPEED);
		Controller->SetControlRotation(NewRotation);

		// 检查是否接近目标旋转
		float AngleDifference = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - CurrentTargetRotation.Yaw));
		if (AngleDifference < LOCK_COMPLETION_THRESHOLD)
		{
			bShouldSmoothSwitchCamera = false;
		}
	}

	// 平滑旋转角色
	if (bShouldSmoothSwitchCharacter)
	{
		FRotator CharacterTargetRotation = FRotator(0, CurrentTargetRotation.Yaw, 0);
		FRotator CurrentCharacterRotation = GetActorRotation();
		FRotator NewCharacterRotation = FMath::RInterpTo(CurrentCharacterRotation, CharacterTargetRotation, 
			DeltaTime, TARGET_SWITCH_SMOOTH_SPEED);
		SetActorRotation(NewCharacterRotation);

		// 检查是否接近目标旋转
		float AngleDifference = FMath::Abs(FRotator::NormalizeAxis(NewCharacterRotation.Yaw - CharacterTargetRotation.Yaw));
		if (AngleDifference < LOCK_COMPLETION_THRESHOLD)
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
	}
}

// ==================== 输入处理函数 ====================
void AMyCharacter::HandleRightStickX(float Value)
{
	// 只在锁定状态下处理右摇杆切换目标
	if (!bIsLockedOn)
		return;

	// 检测摇杆从中性位置移动到左右
	bool bCurrentLeftPressed = (Value < -THUMBSTICK_THRESHOLD);
	bool bCurrentRightPressed = (Value > THUMBSTICK_THRESHOLD);

	// 检测按下事件（从未按下到按下）
	if (bCurrentLeftPressed && !bRightStickLeftPressed)
	{
		SwitchLockOnTargetLeft();
	}
	else if (bCurrentRightPressed && !bRightStickRightPressed)
	{
		SwitchLockOnTargetRight();
	}

	// 更新状态
	bRightStickLeftPressed = bCurrentLeftPressed;
	bRightStickRightPressed = bCurrentRightPressed;
	LastRightStickX = Value;
}

void AMyCharacter::HandleLockOnButton()
{
	ToggleLockOn();
}

void AMyCharacter::SwitchLockOnTargetLeft()
{
	if (!bIsLockedOn || LockOnCandidates.Num() <= 1)
		return;

	// 切换前先强制隐藏所有UI，确保没有遗留
	HideAllLockOnWidgets();

	// 只在必要时刷新候选目标列表
	if (LockOnCandidates.Num() == 0)
	{
		FindLockOnCandidates();
	}

	int32 CurrentIndex = LockOnCandidates.Find(CurrentLockOnTarget);
	if (CurrentIndex != INDEX_NONE && CurrentIndex > 0)
	{
		int32 NewIndex = CurrentIndex - 1;
		AActor* NewTarget = LockOnCandidates[NewIndex];
		
		if (IsValidLockOnTarget(NewTarget))
		{
			PreviousLockOnTarget = CurrentLockOnTarget;
			CurrentLockOnTarget = NewTarget;
			StartSmoothTargetSwitch(NewTarget);
			ShowLockOnWidget();
		}
	}
}

void AMyCharacter::SwitchLockOnTargetRight()
{
	if (!bIsLockedOn || LockOnCandidates.Num() <= 1)
		return;

	// 切换前先强制隐藏所有UI，确保没有遗留
	HideAllLockOnWidgets();

	// 只在必要时刷新候选目标列表
	if (LockOnCandidates.Num() == 0)
	{
		FindLockOnCandidates();
	}

	int32 CurrentIndex = LockOnCandidates.Find(CurrentLockOnTarget);
	if (CurrentIndex != INDEX_NONE && CurrentIndex < LockOnCandidates.Num() - 1)
	{
		int32 NewIndex = CurrentIndex + 1;
		AActor* NewTarget = LockOnCandidates[NewIndex];
		
		if (IsValidLockOnTarget(NewTarget))
		{
			PreviousLockOnTarget = CurrentLockOnTarget;
			CurrentLockOnTarget = NewTarget;
			StartSmoothTargetSwitch(NewTarget);
			ShowLockOnWidget();
		}
	}
}

void AMyCharacter::DebugInputTest()
{
	UE_LOG(LogTemp, Warning, TEXT("=== DEBUG INPUT TEST ==="));
	UE_LOG(LogTemp, Warning, TEXT("IsLockedOn: %s"), bIsLockedOn ? TEXT("True") : TEXT("False"));
	UE_LOG(LogTemp, Warning, TEXT("Current Target: %s"), CurrentLockOnTarget ? *CurrentLockOnTarget->GetName() : TEXT("None"));
	UE_LOG(LogTemp, Warning, TEXT("Available Targets: %d"), LockOnCandidates.Num());
	UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void AMyCharacter::DebugWidgetSetup()
{
	UE_LOG(LogTemp, Warning, TEXT("=== WIDGET SETUP DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass is set: %s"), LockOnWidgetClass ? TEXT("YES") : TEXT("NO"));
	
	if (LockOnWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass name: %s"), *LockOnWidgetClass->GetName());
		UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass path: %s"), *LockOnWidgetClass->GetPathName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LockOnWidgetClass is NULL! Attempting to find available Widgets..."));
		
		// 尝试查找可用的Widget类
		TArray<FString> PossibleWidgetPaths = {
			TEXT("/Game/Levels/Widget_LockOnIcon.Widget_LockOnIcon_C"),
			TEXT("/Game/LockOnTS/Widgets/UI_LockOnWidget.UI_LockOnWidget_C"),
			TEXT("Widget_LockOnIcon_C"),
			TEXT("UI_LockOnWidget_C")
		};
		
		UE_LOG(LogTemp, Warning, TEXT("Searching for Widget classes:"));
		for (const FString& WidgetPath : PossibleWidgetPaths)
		{
			UClass* FoundWidgetClass = LoadObject<UClass>(nullptr, *WidgetPath);
			if (!FoundWidgetClass)
			{
				FoundWidgetClass = FindObject<UClass>(ANY_PACKAGE, *WidgetPath);
			}
			
			if (FoundWidgetClass && FoundWidgetClass->IsChildOf(UUserWidget::StaticClass()))
			{
				UE_LOG(LogTemp, Warning, TEXT("? FOUND: %s"), *WidgetPath);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("? NOT FOUND: %s"), *WidgetPath);
			}
		}
		
		UE_LOG(LogTemp, Error, TEXT("Steps to fix:"));
		UE_LOG(LogTemp, Error, TEXT("1. Open your character Blueprint"));
		UE_LOG(LogTemp, Error, TEXT("2. Find 'Lock On Widget Class' in the UI category"));
		UE_LOG(LogTemp, Error, TEXT("3. Set it to Widget_LockOnIcon or UI_LockOnWidget"));
		UE_LOG(LogTemp, Error, TEXT("4. Ensure the Widget has 'UpdateLockOnPostition' Custom Event"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Socket projection enabled: %s"), bUseSocketProjection ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Target Socket Name: %s"), *TargetSocketName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Socket Offset: (%.1f, %.1f, %.1f)"), SocketOffset.X, SocketOffset.Y, SocketOffset.Z);
	UE_LOG(LogTemp, Warning, TEXT("Current widget instance: %s"), LockOnWidgetInstance ? TEXT("EXISTS") : TEXT("NULL"));
	
	// 检查PlayerController
	APlayerController* PC = Cast<APlayerController>(GetController());
	UE_LOG(LogTemp, Warning, TEXT("PlayerController available: %s"), PC ? TEXT("YES") : TEXT("NO"));
	
	// 检查当前锁定目标
	if (CurrentLockOnTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current lock-on target: %s"), *CurrentLockOnTarget->GetName());
		
		// 检查目标是否有指定的Socket
		bool bHasSocket = HasValidSocket(CurrentLockOnTarget);
		UE_LOG(LogTemp, Warning, TEXT("Target has Socket '%s': %s"), *TargetSocketName.ToString(), bHasSocket ? TEXT("YES") : TEXT("NO"));
		
		if (bHasSocket)
		{
			FVector SocketLocation = GetTargetSocketWorldLocation(CurrentLockOnTarget);
			UE_LOG(LogTemp, Warning, TEXT("Socket world location: (%.1f, %.1f, %.1f)"), SocketLocation.X, SocketLocation.Y, SocketLocation.Z);
			
			FVector2D ScreenPos = ProjectSocketToScreen(SocketLocation);
			UE_LOG(LogTemp, Warning, TEXT("Socket screen position: (%.1f, %.1f)"), ScreenPos.X, ScreenPos.Y);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No current lock-on target"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("========================"));
}

// ==================== UI相关函数 ====================
void AMyCharacter::ShowLockOnWidget()
{
	if (!CurrentLockOnTarget)
		return;

	// 隐藏所有其他目标的UI
	HideAllLockOnWidgets();

	// 如果启用Socket投射，使用新的Socket投射UI系统
	if (bUseSocketProjection)
	{
		ShowSocketProjectionWidget();
		return;
	}

	// 传统的3D空间UI显示（保持兼容性）
	UActorComponent* WidgetComp = CurrentLockOnTarget->GetComponentByClass(UWidgetComponent::StaticClass());
	if (WidgetComp)
	{
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComp);
		if (WidgetComponent)
		{
			WidgetComponent->SetVisibility(true);
			PreviousLockOnTarget = CurrentLockOnTarget;
			return;
		}
	}

	// 备用方案：使用屏幕空间UI
	if (!LockOnWidgetClass)
		return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->RemoveFromViewport();
	}

	LockOnWidgetInstance = CreateWidget<UUserWidget>(PC, LockOnWidgetClass);
	if (LockOnWidgetInstance)
	{
		LockOnWidgetInstance->AddToViewport();
		PreviousLockOnTarget = CurrentLockOnTarget;
	}
}

void AMyCharacter::HideLockOnWidget()
{
	HideAllLockOnWidgets();

	// 隐藏Socket投射UI
	if (bUseSocketProjection)
	{
		HideSocketProjectionWidget();
	}

	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->RemoveFromViewport();
	}

	LockOnWidgetInstance = nullptr;
	PreviousLockOnTarget = nullptr;
}

void AMyCharacter::UpdateLockOnWidget()
{
	// 如果不在锁定状态，确保所有UI都隐藏
	if (!bIsLockedOn)
	{
		HideAllLockOnWidgets();
		if (bUseSocketProjection)
		{
			HideSocketProjectionWidget();
		}
		if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
		{
			LockOnWidgetInstance->RemoveFromViewport();
			LockOnWidgetInstance = nullptr;
		}
		return;
	}

	// 检测目标是否发生变化
	bool bTargetChanged = (CurrentLockOnTarget != PreviousLockOnTarget);
	
	if (bTargetChanged && IsValid(PreviousLockOnTarget))
	{
		// 隐藏上一个目标的UI
		if (bUseSocketProjection)
		{
			HideSocketProjectionWidget();
		}
		else
		{
			UActorComponent* PrevWidgetComp = PreviousLockOnTarget->GetComponentByClass(UWidgetComponent::StaticClass());
			if (PrevWidgetComp)
			{
				UWidgetComponent* PrevWidgetComponent = Cast<UWidgetComponent>(PrevWidgetComp);
				if (PrevWidgetComponent && PrevWidgetComponent->IsVisible())
				{
					PrevWidgetComponent->SetVisibility(false);
				}
			}
		}
		
		PreviousLockOnTarget = CurrentLockOnTarget;
	}

	// 如果当前有锁定目标且处于锁定状态，更新UI
	if (bIsLockedOn && IsValid(CurrentLockOnTarget))
	{
		if (bUseSocketProjection)
		{
			// 使用Socket投射更新UI位置
			UpdateSocketProjectionWidget();
		}
		else
		{
			// 传统3D空间UI更新
			UActorComponent* WidgetComp = CurrentLockOnTarget->GetComponentByClass(UWidgetComponent::StaticClass());
			if (WidgetComp)
			{
				UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComp);
				if (WidgetComponent && (!WidgetComponent->IsVisible() || bTargetChanged))
				{
					WidgetComponent->SetVisibility(true);
				}
			}
		}
	}
}

// ==================== Socket投射系统实现 ====================
FVector AMyCharacter::GetTargetSocketWorldLocation(AActor* Target) const
{
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetTargetSocketWorldLocation: Target is null"));
		return FVector::ZeroVector;
	}

	// 尝试获取目标的骨骼网格组件
	USkeletalMeshComponent* SkeletalMesh = Target->FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh && SkeletalMesh->DoesSocketExist(TargetSocketName))
	{
		// 如果Socket存在，返回Socket的世界位置加上偏移
		FVector SocketLocation = SkeletalMesh->GetSocketLocation(TargetSocketName);
		FVector FinalLocation = SocketLocation + SocketOffset;
		
		UE_LOG(LogTemp, Verbose, TEXT("Socket '%s' found on target '%s' at location (%.1f, %.1f, %.1f)"), 
			*TargetSocketName.ToString(), *Target->GetName(), FinalLocation.X, FinalLocation.Y, FinalLocation.Z);
		
		return FinalLocation;
	}
	else
	{
		// 如果没有Socket，返回Actor的位置加上偏移
		FVector FallbackLocation = Target->GetActorLocation() + SocketOffset;
		
		UE_LOG(LogTemp, Verbose, TEXT("Socket '%s' not found on target '%s', using fallback location (%.1f, %.1f, %.1f)"), 
			*TargetSocketName.ToString(), *Target->GetName(), FallbackLocation.X, FallbackLocation.Y, FallbackLocation.Z);
		
		return FallbackLocation;
	}
}

bool AMyCharacter::HasValidSocket(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// 检查目标是否有指定的Socket
	USkeletalMeshComponent* SkeletalMesh = Target->FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh)
	{
		bool bSocketExists = SkeletalMesh->DoesSocketExist(TargetSocketName);
		UE_LOG(LogTemp, Verbose, TEXT("Socket check for '%s': Socket '%s' exists = %s"), 
			*Target->GetName(), *TargetSocketName.ToString(), bSocketExists ? TEXT("YES") : TEXT("NO"));
		return bSocketExists;
	}

	UE_LOG(LogTemp, Verbose, TEXT("Target '%s' has no SkeletalMeshComponent"), *Target->GetName());
	return false;
}

FVector2D AMyCharacter::ProjectSocketToScreen(const FVector& SocketWorldLocation) const
{
	// 获取玩家控制器
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectSocketToScreen: No PlayerController"));
		return FVector2D::ZeroVector;
	}

	// 将世界位置投射到屏幕坐标
	FVector2D ScreenLocation;
	bool bProjected = PC->ProjectWorldLocationToScreen(SocketWorldLocation, ScreenLocation);
	
	if (bProjected)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Projection successful: World(%.1f, %.1f, %.1f) -> Screen(%.1f, %.1f)"), 
			SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z,
			ScreenLocation.X, ScreenLocation.Y);
		return ScreenLocation;
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("Projection failed for world location (%.1f, %.1f, %.1f)"), 
			SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z);
		return FVector2D::ZeroVector;
	}
}

void AMyCharacter::ShowSocketProjectionWidget()
{
	if (!CurrentLockOnTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowSocketProjectionWidget: No current lock-on target"));
		return;
	}

	// 如果LockOnWidgetClass没有设置，尝试在运行时查找
	if (!LockOnWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowSocketProjectionWidget: LockOnWidgetClass is not set! Attempting runtime search..."));
		
		// 尝试多种可能的Widget类名称和路径
		TArray<FString> PossibleWidgetPaths = {
			TEXT("/Game/Levels/Widget_LockOnIcon.Widget_LockOnIcon_C"),
			TEXT("/Game/LockOnTS/Widgets/UI_LockOnWidget.UI_LockOnWidget_C"),
			TEXT("Widget_LockOnIcon_C"),
			TEXT("UI_LockOnWidget_C"),
			TEXT("/Game/Levels/Widget_LockOnIcon"),
			TEXT("/Game/LockOnTS/Widgets/UI_LockOnWidget")
		};
		
		for (const FString& WidgetPath : PossibleWidgetPaths)
		{
			UClass* FoundWidgetClass = LoadObject<UClass>(nullptr, *WidgetPath);
			
			if (!FoundWidgetClass)
			{
				// 尝试通过名称查找
				FoundWidgetClass = FindObject<UClass>(ANY_PACKAGE, *WidgetPath);
			}
			
			if (FoundWidgetClass && FoundWidgetClass->IsChildOf(UUserWidget::StaticClass()))
			{
				LockOnWidgetClass = FoundWidgetClass;
				UE_LOG(LogTemp, Warning, TEXT("Found Widget class at runtime: %s (Path: %s)"), 
					*FoundWidgetClass->GetName(), *WidgetPath);
				break;
			}
			else
			{
				UE_LOG(LogTemp, Verbose, TEXT("Widget not found at path: %s"), *WidgetPath);
			}
		}
		
		if (!LockOnWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find any Widget class at runtime!"));
			UE_LOG(LogTemp, Error, TEXT("Available Widget files found in project:"));
			UE_LOG(LogTemp, Error, TEXT("- F:\\soul\\Content\\Levels\\Widget_LockOnIcon.uasset"));
			UE_LOG(LogTemp, Error, TEXT("- F:\\soul\\Content\\LockOnTS\\Widgets\\UI_LockOnWidget.uasset"));
			UE_LOG(LogTemp, Error, TEXT("Please ensure:"));
			UE_LOG(LogTemp, Error, TEXT("1. One of these Widgets is compiled and valid"));
			UE_LOG(LogTemp, Error, TEXT("2. LockOnWidgetClass is set in Blueprint to one of these Widgets"));
			UE_LOG(LogTemp, Error, TEXT("3. Widget contains UpdateLockOnPostition event"));
			UE_LOG(LogTemp, Error, TEXT("4. Call DebugWidgetSetup() function for more info"));
			return;
		}
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowSocketProjectionWidget: No valid PlayerController"));
		return;
	}

	// 如果已有实例且在视口中，先移除
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("Removing existing UMG widget instance"));
		LockOnWidgetInstance->RemoveFromViewport();
	}

	// 创建新的Widget实例
	LockOnWidgetInstance = CreateWidget<UUserWidget>(PC, LockOnWidgetClass);
	if (LockOnWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Successfully created UMG widget instance: %s"), *LockOnWidgetInstance->GetClass()->GetName());
		
		LockOnWidgetInstance->AddToViewport();
		UE_LOG(LogTemp, Warning, TEXT("UMG widget added to viewport"));
		
		// 立即更新位置
		UpdateSocketProjectionWidget();
		
		PreviousLockOnTarget = CurrentLockOnTarget;
		
		UE_LOG(LogTemp, Warning, TEXT("Socket projection widget created and shown for target: %s"), 
			*CurrentLockOnTarget->GetName());
		UE_LOG(LogTemp, Warning, TEXT("Using Socket: %s"), *TargetSocketName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UMG widget instance! LockOnWidgetClass: %s"), 
			LockOnWidgetClass ? *LockOnWidgetClass->GetName() : TEXT("NULL"));
	}
}

void AMyCharacter::UpdateSocketProjectionWidget()
{
	if (!CurrentLockOnTarget || !LockOnWidgetInstance || !LockOnWidgetInstance->IsInViewport())
		return;

	// Get target Socket world location
	FVector SocketWorldLocation = GetTargetSocketWorldLocation(CurrentLockOnTarget);
	
	// Project to screen coordinates
	FVector2D ScreenPosition = ProjectSocketToScreen(SocketWorldLocation);
	
	// Check if projection is successful (screen coordinates are valid)
	if (ScreenPosition != FVector2D::ZeroVector)
	{
		// Try to call your UMG event function UpdateLockOnPostition (keeping original spelling)
		UFunction* UpdateFunction = LockOnWidgetInstance->GetClass()->FindFunctionByName(FName(TEXT("UpdateLockOnPostition")));
		
		if (UpdateFunction)
		{
			// Check function parameter count
			if (UpdateFunction->NumParms >= 1)
			{
				// Create parameter structure
				uint8* Params = static_cast<uint8*>(FMemory_Alloca(UpdateFunction->ParmsSize));
				FMemory::Memzero(Params, UpdateFunction->ParmsSize);
				
				// Find FVector2D parameter
				bool bFoundParam = false;
				for (TFieldIterator<FProperty> It(UpdateFunction); It; ++It)
				{
					FProperty* Property = *It;
					if (Property->HasAnyPropertyFlags(CPF_Parm) && !Property->HasAnyPropertyFlags(CPF_ReturnParm))
					{
						// Check if it's FVector2D type
						if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
						{
							if (StructProp->Struct && StructProp->Struct->GetFName() == FName("Vector2D"))
							{
								// Set parameter value
								FVector2D* ParamPtr = reinterpret_cast<FVector2D*>(Params + Property->GetOffset_ForInternal());
								*ParamPtr = ScreenPosition;
								bFoundParam = true;
								UE_LOG(LogTemp, Warning, TEXT("Setting UMG position parameter: (%.1f, %.1f) using function UpdateLockOnPostition"), 
									ScreenPosition.X, ScreenPosition.Y);
								break;
							}
						}
					}
				}
				
				if (bFoundParam)
				{
					// Call function
					LockOnWidgetInstance->ProcessEvent(UpdateFunction, Params);
					UE_LOG(LogTemp, Warning, TEXT("Successfully called UMG update function UpdateLockOnPostition with position: (%.1f, %.1f)"), 
						ScreenPosition.X, ScreenPosition.Y);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Could not find Vector2D parameter in UMG function UpdateLockOnPostition"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("UMG function UpdateLockOnPostition has no parameters"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find UMG update function UpdateLockOnPostition in widget class"));
		}
		
		// Ensure Widget is visible
		if (!LockOnWidgetInstance->IsVisible())
		{
			LockOnWidgetInstance->SetVisibility(ESlateVisibility::Visible);
			UE_LOG(LogTemp, Warning, TEXT("Set UMG widget visibility to Visible"));
		}
		
		// Debug log - including Socket information
		UE_LOG(LogTemp, Warning, TEXT("Socket projection updated: Socket(%s) World(%.1f, %.1f, %.1f) -> Screen(%.1f, %.1f)"), 
			*TargetSocketName.ToString(),
			SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z,
			ScreenPosition.X, ScreenPosition.Y);
	}
	else
	{
		// Target is off-screen, hide Widget
		if (LockOnWidgetInstance->IsVisible())
		{
			LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
			UE_LOG(LogTemp, Warning, TEXT("Target off-screen, hiding UMG widget"));
		}
	}
}

void AMyCharacter::HideSocketProjectionWidget()
{
	if (LockOnWidgetInstance && LockOnWidgetInstance->IsInViewport())
	{
		LockOnWidgetInstance->RemoveFromViewport();
		UE_LOG(LogTemp, Warning, TEXT("Socket projection widget hidden"));
	}
	
	LockOnWidgetInstance = nullptr;
}

// ==================== 缺失的函数实现 ====================
void AMyCharacter::HideAllLockOnWidgets()
{
	// 遍历所有候选目标，隐藏它们的UI
	for (AActor* Candidate : LockOnCandidates)
	{
		if (IsValid(Candidate))
		{
			UActorComponent* WidgetComp = Candidate->GetComponentByClass(UWidgetComponent::StaticClass());
			if (WidgetComp)
			{
				UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComp);
				if (WidgetComponent && WidgetComponent->IsVisible())
				{
					WidgetComponent->SetVisibility(false);
				}
			}
		}
	}
	
	// 额外安全措施：如果有当前锁定目标，也确保它的UI被隐藏
	if (IsValid(CurrentLockOnTarget))
	{
		UActorComponent* WidgetComp = CurrentLockOnTarget->GetComponentByClass(UWidgetComponent::StaticClass());
		if (WidgetComp)
		{
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComp);
			if (WidgetComponent && WidgetComponent->IsVisible())
			{
				WidgetComponent->SetVisibility(false);
			}
		}
	}
	
	// 额外安全措施：如果有之前的锁定目标，也确保它的UI被隐藏
	if (IsValid(PreviousLockOnTarget))
	{
		UActorComponent* WidgetComp = PreviousLockOnTarget->GetComponentByClass(UWidgetComponent::StaticClass());
		if (WidgetComp)
		{
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComp);
			if (WidgetComponent && WidgetComponent->IsVisible())
			{
				WidgetComponent->SetVisibility(false);
			}
		}
	}
}

bool AMyCharacter::HasCandidatesInSphere()
{
	FindLockOnCandidates();
	return LockOnCandidates.Num() > 0;
}

AActor* AMyCharacter::TryGetSectorLockTarget()
{
	if (LockOnCandidates.Num() == 0)
		return nullptr;

	// 筛选扇形区域内的目标
	TArray<AActor*> SectorTargets;
	
	for (AActor* Candidate : LockOnCandidates)
	{
		if (Candidate && IsTargetInSectorLockZone(Candidate))
		{
			SectorTargets.Add(Candidate);
		}
	}

	// 如果扇形区域内有目标，返回最佳目标
	if (SectorTargets.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found %d targets in sector lock zone"), SectorTargets.Num());
		return GetBestTargetFromList(SectorTargets);
	}

	return nullptr;
}

AActor* AMyCharacter::TryGetCameraCorrectionTarget()
{
	if (LockOnCandidates.Num() == 0)
		return nullptr;

	// 寻找最近的目标（不限制角度）
	AActor* ClosestTarget = nullptr;
	float ClosestDistance = FLT_MAX;
	
	FVector PlayerLocation = GetActorLocation();
	
	for (AActor* Candidate : LockOnCandidates)
	{
		if (!Candidate)
			continue;
			
		float Distance = FVector::Dist(PlayerLocation, Candidate->GetActorLocation());
		
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestTarget = Candidate;
		}
	}

	// 检查最近的目标是否在合理的修正范围内
	if (ClosestTarget)
	{
		float AngleToTarget = CalculateAngleToTarget(ClosestTarget);
		
		// 只对角度在合理范围内的目标进行相机修正（避免修正到背后的敌人）
		if (AngleToTarget <= 160.0f) // 最大160度，即左右160度范围内
		{
			UE_LOG(LogTemp, Warning, TEXT("Found camera correction target: %s (Distance: %.1f, Angle: %.1f degrees)"), 
				*ClosestTarget->GetName(), ClosestDistance, AngleToTarget);
			return ClosestTarget;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Closest target %s is too far behind (%.1f degrees), no correction"), 
				*ClosestTarget->GetName(), AngleToTarget);
		}
	}

	return nullptr;
}

void AMyCharacter::StartCameraCorrectionForTarget(AActor* Target)
{
	if (!Target || !Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartCameraCorrectionForTarget: Invalid target or controller"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting camera correction for target: %s"), *Target->GetName());

	// 计算朝向目标的旋转
	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
	
	// 启动相机自动修正
	bIsCameraAutoCorrection = true;
	CameraCorrectionStartTime = GetWorld()->GetTimeSeconds();
	CameraCorrectionStartRotation = Controller->GetControlRotation();
	
	// 计算修正后的目标旋转
	FRotator CurrentRotation = Controller->GetControlRotation();
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
		// 如果角度差异较大，只修正一部分（让目标进入扇形区域边缘）	
		CorrectionAmount = FMath::Sign(YawDifference) * CAMERA_CORRECTION_OFFSET;
	}
	
	CameraCorrectionTargetRotation = FRotator(
		CurrentRotation.Pitch,
		CurrentRotation.Yaw + CorrectionAmount,
		CurrentRotation.Roll
	);

	UE_LOG(LogTemp, Warning, TEXT("Camera correction: Current=%.1f degrees, Target=%.1f degrees, Correction=%.1f degrees"), 
		CurrentRotation.Yaw, TargetRotation.Yaw, CorrectionAmount);
}

void AMyCharacter::StartCameraAutoCorrection(AActor* Target)
{
	// 这个函数保留为了兼容性，实际使用 StartCameraCorrectionForTarget
	StartCameraCorrectionForTarget(Target);
}

void AMyCharacter::UpdateCameraAutoCorrection()
{
	if (!bIsCameraAutoCorrection || !Controller)
		return;

	// 检查玩家是否有输入 - 提供额外的安全检查
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		// 检查玩家的相机输入
		float PlayerYawInput = PC->GetInputAxisValue("Turn");
		float PlayerPitchInput = PC->GetInputAxisValue("LookUp");
		
		if (FMath::Abs(PlayerYawInput) > 0.05f || FMath::Abs(PlayerPitchInput) > 0.05f)
		{
			// 玩家有输入，立即停止自动修正
			bIsCameraAutoCorrection = false;
			DelayedCorrectionTarget = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Player input detected during auto correction - stopping immediately"));
			return;
		}
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float CorrectionDuration = CurrentTime - CameraCorrectionStartTime;
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 平滑插值到修正目标旋转
	FRotator CurrentRotation = Controller->GetControlRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, CameraCorrectionTargetRotation, 
		DeltaTime, CAMERA_AUTO_CORRECTION_SPEED);
	
	Controller->SetControlRotation(NewRotation);

	// 检查是否完成修正
	float AngleDifference = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - CameraCorrectionTargetRotation.Yaw));
	
	if (AngleDifference < 2.0f || CorrectionDuration > 1.5f) // 接近目标旋转或超时
	{
		bIsCameraAutoCorrection = false;
		UE_LOG(LogTemp, Warning, TEXT("Camera correction completed (Angle diff: %.1f degrees, Duration: %.1fs)"), 
			AngleDifference, CorrectionDuration);
		
		// 修正完成后，清理状态
		DelayedCorrectionTarget = nullptr;
	}
}

void AMyCharacter::DelayedCameraCorrection()
{
	if (!DelayedCorrectionTarget || !Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("DelayedCameraCorrection: Invalid target or controller"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Executing delayed camera correction for target: %s"), *DelayedCorrectionTarget->GetName());

	// 计算朝向目标的旋转
	FVector PlayerLocation = GetActorLocation();
	FVector TargetLocation = DelayedCorrectionTarget->GetActorLocation();
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, TargetLocation);
	
	// 启动相机自动修正
	bIsCameraAutoCorrection = true;
	CameraCorrectionStartTime = GetWorld()->GetTimeSeconds();
	CameraCorrectionStartRotation = Controller->GetControlRotation();
	
	// 计算修正后的目标旋转（部分修正，不是完全对准）
	FRotator CurrentRotation = Controller->GetControlRotation();
	float YawDifference = FRotator::NormalizeAxis(TargetRotation.Yaw - CurrentRotation.Yaw);
	
	// 限制修正幅度，只修正一部分角度
	float CorrectionAmount = FMath::Sign(YawDifference) * FMath::Min(FMath::Abs(YawDifference), CAMERA_CORRECTION_OFFSET);
	CameraCorrectionTargetRotation = FRotator(
		CurrentRotation.Pitch,
		CurrentRotation.Yaw + CorrectionAmount,
		CurrentRotation.Roll
	);

	UE_LOG(LogTemp, Log, TEXT("Camera correction: Current=%.1f degrees, Target=%.1f degrees, Correction=%.1f degrees"), 
		CurrentRotation.Yaw, TargetRotation.Yaw, CorrectionAmount);
}

void AMyCharacter::RestoreCameraFollowState()
{
	UE_LOG(LogTemp, Log, TEXT("Restoring camera follow state"));

	// 恢复相机跟随状态
	bShouldCameraFollowTarget = true;
	bShouldCharacterRotateToTarget = true;
	
	// 停止自动修正
	bIsCameraAutoCorrection = false;
	
	// 清空延迟修正目标
	DelayedCorrectionTarget = nullptr;

	UE_LOG(LogTemp, Log, TEXT("Camera follow state restored"));
}

void AMyCharacter::DrawLockOnCursor()
{
	// 这个函数留空，因为我们使用UMG UI而不是Debug绘制
}