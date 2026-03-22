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

	// ==================== 创建目标检测组件 ====================
	TargetDetectionComponent = CreateDefaultSubobject<UTargetDetectionComponent>(TEXT("TargetDetectionComponent"));

	// ==================== 创建相机控制组件 ====================
	CameraControlComponent = CreateDefaultSubobject<UCameraControlComponent>(TEXT("CameraControlComponent"));

	// ==================== 创建UI管理组件 ====================
	UIManagerComponent = CreateDefaultSubobject<UUIManagerComponent>(TEXT("UIManagerComponent"));

	// ==================== 创建Soul游戏系统组件 ====================
	PoiseComponent = CreateDefaultSubobject<UPoiseComponent>(TEXT("PoiseComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	DodgeComponent = CreateDefaultSubobject<UDodgeComponent>(TEXT("DodgeComponent"));
	ExecutionComponent = CreateDefaultSubobject<UExecutionComponent>(TEXT("ExecutionComponent"));

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
	CameraBoom->TargetArmLength = 450.0f; // 相机距离
	CameraBoom->bUsePawnControlRotation = true; // 跟随控制器旋转

	// 设置相机臂的初始角度（俯视角度）
	CameraBoom->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

	// 创建相机组件
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
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

	// ==================== 配置目标检测组件 ====================
	if (TargetDetectionComponent)
	{
		// 设置检测球体引用
		TargetDetectionComponent->SetLockOnDetectionSphere(LockOnDetectionSphere);
		
		// 配置锁定参数
		TargetDetectionComponent->LockOnSettings.LockOnRange = LockOnRange;
		TargetDetectionComponent->LockOnSettings.LockOnAngle = LockOnAngle;
		
		// 配置相机参数
		TargetDetectionComponent->CameraSettings.CameraInterpSpeed = CameraInterpSpeed;
		TargetDetectionComponent->CameraSettings.bEnableSmoothCameraTracking = bEnableSmoothCameraTracking;
		TargetDetectionComponent->CameraSettings.CameraTrackingMode = CameraTrackingMode;
		TargetDetectionComponent->CameraSettings.TargetLocationOffset = TargetLocationOffset;
		
		// 配置Socket投射参数
		TargetDetectionComponent->SocketProjectionSettings.bUseSocketProjection = bUseSocketProjection;
		TargetDetectionComponent->SocketProjectionSettings.TargetSocketName = TargetSocketName;
		TargetDetectionComponent->SocketProjectionSettings.SocketOffset = SocketOffset;
		TargetDetectionComponent->SocketProjectionSettings.ProjectionScale = ProjectionScale;
		
		// 配置调试设置
		TargetDetectionComponent->bEnableTargetDetectionDebugLogs = bEnableLockOnDebugLogs;
		TargetDetectionComponent->bEnableSizeAnalysisDebugLogs = bEnableLockOnDebugLogs;
		
		UE_LOG(LogTemp, Warning, TEXT("MyCharacter: TargetDetectionComponent configured successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter: TargetDetectionComponent is null!"));
	}

	// ==================== 初始化相机控制组件设置 ====================
	if (CameraControlComponent)
	{
		// 配置相机设置
		FCameraSettings CameraSetup;
		CameraSetup.CameraInterpSpeed = CameraInterpSpeed;
		CameraSetup.bEnableSmoothCameraTracking = bEnableSmoothCameraTracking;
		CameraSetup.CameraTrackingMode = CameraTrackingMode;
		CameraSetup.TargetLocationOffset = TargetLocationOffset;
		CameraControlComponent->SetCameraSettings(CameraSetup);
		
		// 配置高级相机设置
		FAdvancedCameraSettings AdvancedSetup;
		// 使用CameraControlComponent中定义的常量
		CameraControlComponent->SetAdvancedCameraSettings(AdvancedSetup);
		
		UE_LOG(LogTemp, Warning, TEXT("MyCharacter: CameraControlComponent configured successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter: CameraControlComponent is null!"));
	}

	// ==================== 初始化UI管理组件设置 ====================
	if (UIManagerComponent)
	{
		UIManagerComponent->LockOnWidgetClass = LockOnWidgetClass;
		
		// 配置Socket投射设置
		FSocketProjectionSettings SocketSettings;
		SocketSettings.TargetSocketName = TargetSocketName;
		SocketSettings.SocketOffset = SocketOffset;
		SocketSettings.ProjectionScale = ProjectionScale;
		SocketSettings.bUseSocketProjection = bUseSocketProjection;
		UIManagerComponent->SetSocketProjectionSettings(SocketSettings);
		
		UE_LOG(LogTemp, Warning, TEXT("MyCharacter: UIManagerComponent configured successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter: UIManagerComponent is null!"));
	}

	// ==================== 魂类组件基础验证（不实现具体功能集成） ====================
	if (PoiseComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("PoiseComponent initialized successfully"));
	}

	if (StaminaComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("StaminaComponent initialized successfully"));
	}

	if (DodgeComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("DodgeComponent initialized successfully"));
	}

	if (ExecutionComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("ExecutionComponent initialized successfully"));
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

		// 相机调试信息（可控制）- 添加降频控制
		if (bEnableCameraDebugLogs && CurrentLockOnTarget)
		{
			static float LastCameraDebugLogTime = 0.0f;
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if (CurrentTime - LastCameraDebugLogTime > 0.5f) // 每0.5秒记录一次
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("LockOn Active: Target=%s, CameraFollow=%s, CharacterRotate=%s"), 
					*CurrentLockOnTarget->GetName(),
					bShouldCameraFollowTarget ? TEXT("YES") : TEXT("NO"),
					bShouldCharacterRotateToTarget ? TEXT("YES") : TEXT("NO"));
				LastCameraDebugLogTime = CurrentTime;
			}
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

			// 锁定调试信息（可控制）- 添加降频控制
			if (bEnableLockOnDebugLogs)
			{
				static float LastTargetSearchLogTime = 0.0f;
				if (CurrentTime - LastTargetSearchLogTime > 1.0f) // 每1秒记录一次
				{
					UE_LOG(LogTemp, Verbose, TEXT("Target search completed: Found %d candidates"), LockOnCandidates.Num());
					LastTargetSearchLogTime = CurrentTime;
				}
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
	
	// 添加目标尺寸分析调试按键 (G键)
	PlayerInputComponent->BindAction("DebugTargetSizes", IE_Pressed, this, &AMyCharacter::DebugDisplayTargetSizes);
	
	// 打印当前绑定状态
	UE_LOG(LogTemp, Warning, TEXT("Input bindings set up successfully"));
	UE_LOG(LogTemp, Warning, TEXT("Available debug commands:"));
	UE_LOG(LogTemp, Warning, TEXT("- DebugInput (F): General debug info"));
	UE_LOG(LogTemp, Warning, TEXT("- DebugTargetSizes (G): Enemy size analysis"));
}

// ==================== 移动函数实现 ====================
void AMyCharacter::MoveForward(float Value)
{
	ForwardInputValue = Value;

	// 先处理相机控制组件移动输入
	if (CameraControlComponent)
	{
		bool bIsMoving = (FMath::Abs(Value) > 0.1f);
		CameraControlComponent->HandlePlayerMovement(bIsMoving);
	}

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

	// 先处理相机控制组件移动输入
	if (CameraControlComponent)
	{
		bool bIsMoving = (FMath::Abs(Value) > 0.1f);
		CameraControlComponent->HandlePlayerMovement(bIsMoving);
	}

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
	// 先处理相机控制组件输入
	if (CameraControlComponent)
	{
		CameraControlComponent->HandlePlayerInput(Rate, 0.0f);
	}

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
		// 玩家有相机操作意图，停止自动修正
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
	// 先处理相机控制组件输入
	if (CameraControlComponent)
	{
		CameraControlComponent->HandlePlayerInput(0.0f, Rate);
	}

	// 锁定状态下完全禁止玩家的垂直相机输入
	if (bIsLockedOn)
	{
		// 在锁定状态下，仍然检测输入以停止自动相机控制
		if (FMath::Abs(Rate) > 0.1f)
		{
			// 玩家有相机操作意图，停止自动修正
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
		// 玩家有相机操作意图，停止自动修正
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
	if (CameraControlComponent)
	{
		CameraControlComponent->PerformSimpleCameraReset();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::PerformSimpleCameraReset: CameraControlComponent is null!"));
	}
}

// 新增：开始平滑相机重置
void AMyCharacter::StartSmoothCameraReset()
{
	if (CameraControlComponent)
	{
		CameraControlComponent->StartSmoothCameraReset();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::StartSmoothCameraReset: CameraControlComponent is null!"));
	}
}

// 新增：更新平滑相机重置
void AMyCharacter::UpdateSmoothCameraReset()
{
	if (CameraControlComponent)
	{
		CameraControlComponent->UpdateSmoothCameraReset();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::UpdateSmoothCameraReset: CameraControlComponent is null!"));
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

// 改为组件调用
void AMyCharacter::StartCameraReset(const FRotator& TargetRotation)
{
	if (CameraControlComponent)
	{
		CameraControlComponent->StartCameraReset(TargetRotation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::StartCameraReset: CameraControlComponent is null!"));
	}
}

// ==================== 更新函数 ====================
void AMyCharacter::FindLockOnCandidates()
{
	if (TargetDetectionComponent)
	{
		TargetDetectionComponent->FindLockOnCandidates();
		LockOnCandidates = TargetDetectionComponent->GetLockOnCandidates();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::FindLockOnCandidates: TargetDetectionComponent is null!"));
		LockOnCandidates.Empty();
	}
}

bool AMyCharacter::IsValidLockOnTarget(AActor* Target)
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->IsValidLockOnTarget(Target);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::IsValidLockOnTarget: TargetDetectionComponent is null!"));
		return false;
	}
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
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->GetBestTargetFromList(TargetList);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::GetBestTargetFromList: TargetDetectionComponent is null!"));
		return nullptr;
	}
}

AActor* AMyCharacter::GetBestLockOnTarget()
{
	return GetBestSectorLockTarget();
}

AActor* AMyCharacter::GetBestSectorLockTarget()
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->GetBestSectorLockTarget();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::GetBestSectorLockTarget: TargetDetectionComponent is null!"));
		return nullptr;
	}
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
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->IsTargetStillLockable(Target);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::IsTargetStillLockable: TargetDetectionComponent is null!"));
		return false;
	}
}

void AMyCharacter::UpdateLockOnCamera()
{
	if (CameraControlComponent)
	{
		CameraControlComponent->SetLockOnTarget(CurrentLockOnTarget);
		CameraControlComponent->UpdateLockOnCamera();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::UpdateLockOnCamera: CameraControlComponent is null!"));
	}
}

void AMyCharacter::UpdateCharacterRotationToTarget()
{
	if (CameraControlComponent && CurrentLockOnTarget)
	{
		CameraControlComponent->SetLockOnTarget(CurrentLockOnTarget);
		CameraControlComponent->UpdateCharacterRotationToTarget();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::UpdateCharacterRotationToTarget: CameraControlComponent or CurrentLockOnTarget is null!"));
	}
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
	if (TargetDetectionComponent)
	{
		TargetDetectionComponent->SortCandidatesByDirection(Targets);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::SortCandidatesByDirection: TargetDetectionComponent is null!"));
	}
}

void AMyCharacter::StartSmoothTargetSwitch(AActor* NewTarget)
{
	if (CameraControlComponent && NewTarget)
	{
		CameraControlComponent->StartSmoothTargetSwitch(NewTarget);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::StartSmoothTargetSwitch: CameraControlComponent or NewTarget is null!"));
	}
}

void AMyCharacter::UpdateSmoothTargetSwitch()
{
	if (CameraControlComponent)
	{
		CameraControlComponent->UpdateSmoothTargetSwitch();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::UpdateSmoothTargetSwitch: CameraControlComponent is null!"));
	}
}

// ==================== 输入处理函数 ====================
void AMyCharacter::HandleRightStickX(float Value)
{
	// 只在锁定状态下处理右摇杆切换目标
	if (!bIsLockedOn)
		return;

	// 检测摇杆从中性位置移动到左右
	bRightStickLeftPressed = (Value < -THUMBSTICK_THRESHOLD);
	bRightStickRightPressed = (Value > THUMBSTICK_THRESHOLD);

	// 检测按下事件（从未按下到按下）
	if (bRightStickLeftPressed)
	{
		SwitchLockOnTargetLeft();
	}
	else if (bRightStickRightPressed)
	{
		SwitchLockOnTargetRight();
	}

	// 更新状态
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
		
		// 性能优化：注释高频日志
		// UE_LOG(LogTemp, Verbose, TEXT("Socket '%s' found on target '%s' at location (%.1f, %.1f, %.1f)"), 
		//		*TargetSocketName.ToString(), *Target->GetName(), FinalLocation.X, FinalLocation.Y, FinalLocation.Z);
		
		return FinalLocation;
	}
	else
	{
		// 如果没有Socket，返回Actor的位置加上偏移
		FVector FallbackLocation = Target->GetActorLocation() + SocketOffset;
		
		// 性能优化：注释高频日志
		// UE_LOG(LogTemp, Verbose, TEXT("Socket '%s' not found on target '%s', using fallback location (%.1f, %.1f, %.1f)"), 
		//		*TargetSocketName.ToString(), *Target->GetName(), FallbackLocation.X, FallbackLocation.Y, FallbackLocation.Z);
		
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
		// 性能优化：注释高频日志
		// UE_LOG(LogTemp, Verbose, TEXT("Socket check for '%s': Socket '%s' exists = %s"), 
		//		*Target->GetName(), *TargetSocketName.ToString(), bSocketExists ? TEXT("YES") : TEXT("NO"));
		return bSocketExists;
	}

	// 性能优化：注释高频日志
	// UE_LOG(LogTemp, Verbose, TEXT("Target '%s' has no SkeletalMeshComponent"), *Target->GetName());
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
		// 性能优化：注释高频日志
		// UE_LOG(LogTemp, Verbose, TEXT("Projection successful: World(%.1f, %.1f, %.1f) -> Screen(%.1f, %.1f)"), 
		//	SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z,
		//	ScreenLocation.X, ScreenLocation.Y);
		return ScreenLocation;
	}
	else
	{
		// 性能优化：注释高频日志
		// UE_LOG(LogTemp, Verbose, TEXT("Projection failed for world location (%.1f, %.1f, %.1f)"), 
		//	SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z);
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
								// 性能优化：注释高频日志
								// UE_LOG(LogTemp, Warning, TEXT("Setting UMG position parameter: (%.1f, %.1f) using function UpdateLockOnPostition"), 
								//	ScreenPosition.X, ScreenPosition.Y);
								break;
							}
						}
					}
				}
				
				if (bFoundParam)
				{
					// Call function
					LockOnWidgetInstance->ProcessEvent(UpdateFunction, Params);
					// 性能优化：注释高频日志
					// UE_LOG(LogTemp, Warning, TEXT("Successfully called UMG update function UpdateLockOnPostition with position: (%.1f, %.1f)"), 
					//	ScreenPosition.X, ScreenPosition.Y);
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
		
		// Debug log - including Socket information - 添加降频控制
		if (bEnableCameraDebugLogs)
		{
			static float LastSocketProjectionLogTime = 0.0f;
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if (CurrentTime - LastSocketProjectionLogTime > 1.0f) // 每1秒记录一次
			{
				UE_LOG(LogTemp, Warning, TEXT("Socket projection updated: Socket(%s) World(%.1f, %.1f, %.1f) -> Screen(%.1f, %.1f)"), 
					*TargetSocketName.ToString(),
					SocketWorldLocation.X, SocketWorldLocation.Y, SocketWorldLocation.Z,
					ScreenPosition.X, ScreenPosition.Y);
				LastSocketProjectionLogTime = CurrentTime;
			}
		}
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
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->HasCandidatesInSphere();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::HasCandidatesInSphere: TargetDetectionComponent is null!"));
		return false;
	}
}

AActor* AMyCharacter::TryGetSectorLockTarget()
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->TryGetSectorLockTarget();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::TryGetSectorLockTarget: TargetDetectionComponent is null!"));
		return nullptr;
	}
}

AActor* AMyCharacter::TryGetCameraCorrectionTarget()
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->TryGetCameraCorrectionTarget();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::TryGetCameraCorrectionTarget: TargetDetectionComponent is null!"));
		return nullptr;
	}
}

void AMyCharacter::StartCameraCorrectionForTarget(AActor* Target)
{
	if (CameraControlComponent && Target)
	{
		CameraControlComponent->StartCameraCorrectionForTarget(Target);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::StartCameraCorrectionForTarget: CameraControlComponent or Target is null!"));
	}
}

void AMyCharacter::StartCameraAutoCorrection(AActor* Target)
{
	if (CameraControlComponent && Target)
	{
		CameraControlComponent->StartCameraAutoCorrection(Target);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::StartCameraAutoCorrection: CameraControlComponent or Target is null!"));
	}
}

void AMyCharacter::UpdateCameraAutoCorrection()
{
	if (CameraControlComponent)
	{
		CameraControlComponent->UpdateCameraAutoCorrection();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::UpdateCameraAutoCorrection: CameraControlComponent is null!"));
	}
}

void AMyCharacter::DelayedCameraCorrection()
{
	if (CameraControlComponent)
	{
		CameraControlComponent->DelayedCameraCorrection();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::DelayedCameraCorrection: CameraControlComponent is null!"));
	}
}

void AMyCharacter::RestoreCameraFollowState()
{
	// 这个函数用于恢复相机跟随状态
	// 由于我们已经将功能转移到CameraControlComponent，这里保持为空或做基本的状态恢复
	bShouldCameraFollowTarget = true;
	bShouldCharacterRotateToTarget = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Camera follow state restored"));
}

// ==================== 新增：敌人尺寸分析接口实现 ====================

EEnemySizeCategory AMyCharacter::GetTargetSizeCategory(AActor* Target)
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->GetTargetSizeCategory(Target);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::GetTargetSizeCategory: TargetDetectionComponent is null!"));
		return EEnemySizeCategory::Unknown;
	}
}

TArray<AActor*> AMyCharacter::GetTargetsBySize(EEnemySizeCategory SizeCategory)
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->GetTargetsBySize(SizeCategory);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::GetTargetsBySize: TargetDetectionComponent is null!"));
		return TArray<AActor*>();
	}
}

AActor* AMyCharacter::GetNearestTargetBySize(EEnemySizeCategory SizeCategory)
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->GetNearestTargetBySize(SizeCategory);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::GetNearestTargetBySize: TargetDetectionComponent is null!"));
		return nullptr;
	}
}

TMap<EEnemySizeCategory, int32> AMyCharacter::GetSizeCategoryStatistics()
{
	if (TargetDetectionComponent)
	{
		return TargetDetectionComponent->GetSizeCategoryStatistics();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyCharacter::GetSizeCategoryStatistics: TargetDetectionComponent is null!"));
		return TMap<EEnemySizeCategory, int32>();
	}
}

void AMyCharacter::DebugDisplayTargetSizes()
{
	if (!TargetDetectionComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("DebugDisplayTargetSizes: TargetDetectionComponent is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== TARGET SIZE ANALYSIS DEBUG ==="));
	
	// 获取统计信息
	TMap<EEnemySizeCategory, int32> Statistics = GetSizeCategoryStatistics();
	
	UE_LOG(LogTemp, Warning, TEXT("Enemy Count by Size Category:"));
	for (auto& Pair : Statistics)
	{
		FString CategoryName = UEnum::GetValueAsString(Pair.Key);
		UE_LOG(LogTemp, Warning, TEXT("- %s: %d enemies"), *CategoryName, Pair.Value);
	}
	
	// 显示每个分类的最近目标
	UE_LOG(LogTemp, Warning, TEXT("Nearest Targets by Category:"));
	
	TArray<EEnemySizeCategory> Categories = {
		EEnemySizeCategory::Small,
		EEnemySizeCategory::Medium,
		EEnemySizeCategory::Large
	};
	
	for (EEnemySizeCategory Category : Categories)
	{
		AActor* NearestTarget = GetNearestTargetBySize(Category);
		FString CategoryName = UEnum::GetValueAsString(Category);
		
		if (NearestTarget)
		{
			float Distance = FVector::Dist(GetActorLocation(), NearestTarget->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("- %s: %s (Distance: %.1f)"), 
				*CategoryName, *NearestTarget->GetName(), Distance);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("- %s: No targets found"), *CategoryName);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("==================================="));
}