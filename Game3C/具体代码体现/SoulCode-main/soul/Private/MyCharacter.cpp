// Fill out your copyright notice in the Description page of Project Settings.

#include "Mycharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TargetDetectionComponent.h"
#include "UIManagerComponent.h"
#include "LockOnConfigComponent.h"
#include "SubTargetManager.h"
#include "DrawDebugHelpers.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Utility/SoulsCameraDebugComponent.h"

// Sets default values
AMycharacter::AMycharacter()
{
 	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	// ========== Camera references initialized to null, will be found in BeginPlay ==========
	CameraBoomRef = nullptr;
	FollowCameraRef = nullptr;

	// Create Lock-On Detection Sphere
	LockOnDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LockOnDetectionSphere"));
	LockOnDetectionSphere->SetupAttachment(RootComponent);
	LockOnDetectionSphere->InitSphereRadius(LockOnRange);
	LockOnDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LockOnDetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	LockOnDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	LockOnDetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LockOnDetectionSphere->SetHiddenInGame(true);
	LockOnDetectionSphere->SetGenerateOverlapEvents(true);

	// Create Lock-On Components
	TargetDetectionComponent = CreateDefaultSubobject<UTargetDetectionComponent>(TEXT("TargetDetectionComponent"));
	UIManagerComponent = CreateDefaultSubobject<UUIManagerComponent>(TEXT("UIManagerComponent"));
	SubTargetManager = CreateDefaultSubobject<USubTargetManager>(TEXT("SubTargetManager"));

	// Create Camera System Components
	SoulsCameraManager = CreateDefaultSubobject<USoulsCameraManager>(TEXT("SoulsCameraManager"));
	SoulsCameraDebugComponent = CreateDefaultSubobject<USoulsCameraDebugComponent>(TEXT("SoulsCameraDebugComponent"));

	// Initialize stats
	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	CurrentState = ECharacterState::Idle;
}

// Called when the game starts or when spawned
void AMycharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;

	// ========== Find camera components from Blueprint ==========
	UE_LOG(LogTemp, Warning, TEXT("========== FINDING CAMERA COMPONENTS =========="));
	
	// Find CameraBoom (SpringArmComponent)
	if (!CameraBoomRef)
	{
		CameraBoomRef = FindComponentByClass<USpringArmComponent>();
		if (CameraBoomRef)
		{
			UE_LOG(LogTemp, Warning, TEXT("CameraBoom found: %s"), *CameraBoomRef->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CameraBoom NOT FOUND! Please add a SpringArmComponent in Blueprint!"));
		}
	}

	// Find FollowCamera (CameraComponent)
	if (!FollowCameraRef)
	{
		FollowCameraRef = FindComponentByClass<UCameraComponent>();
		if (FollowCameraRef)
		{
			UE_LOG(LogTemp, Warning, TEXT("FollowCamera found: %s"), *FollowCameraRef->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FollowCamera NOT FOUND! Please add a CameraComponent in Blueprint!"));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("================================================"));

	// Initialize lock-on components
	if (TargetDetectionComponent && LockOnDetectionSphere)
	{
		TargetDetectionComponent->SetDetectionSphere(LockOnDetectionSphere);
		TargetDetectionComponent->LockOnRange = LockOnRange;
		UE_LOG(LogTemp, Warning, TEXT("TargetDetectionComponent initialized"));
	}

	// ===== Debug: Sphere initialization verification =====
	UE_LOG(LogTemp, Error, TEXT("========== SPHERE DEBUG =========="));
	if (LockOnDetectionSphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("LockOnDetectionSphere: VALID"));
		UE_LOG(LogTemp, Warning, TEXT("  Radius: %.1f"), LockOnDetectionSphere->GetScaledSphereRadius());
		UE_LOG(LogTemp, Warning, TEXT("  Location: %s"), *LockOnDetectionSphere->GetComponentLocation().ToString());
		UE_LOG(LogTemp, Warning, TEXT("  Collision Enabled: %d"), (int32)LockOnDetectionSphere->GetCollisionEnabled());
		UE_LOG(LogTemp, Warning, TEXT("  Object Type: %d"), (int32)LockOnDetectionSphere->GetCollisionObjectType());
		
		ECollisionResponse PawnResponse = LockOnDetectionSphere->GetCollisionResponseToChannel(ECC_Pawn);
		UE_LOG(LogTemp, Warning, TEXT("  Response to Pawn: %d (0=Ignore, 1=Overlap, 2=Block)"), (int32)PawnResponse);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LockOnDetectionSphere: NULL - THIS IS A PROBLEM!"));
	}
	UE_LOG(LogTemp, Error, TEXT("==================================="));

	// ========== COMPONENT DIAGNOSTIC ==========
	UE_LOG(LogTemp, Error, TEXT("========== ALL COMPONENTS ON THIS ACTOR =========="));
	TArray<UActorComponent*> AllComps;
	GetComponents(AllComps);
	for (UActorComponent* Comp : AllComps)
	{
		UE_LOG(LogTemp, Warning, TEXT("  Component: %s [Class: %s]"), 
			*Comp->GetName(), *Comp->GetClass()->GetName());
	}
	UE_LOG(LogTemp, Error, TEXT("TargetDetectionComponent ptr: %p"), TargetDetectionComponent);
	UE_LOG(LogTemp, Error, TEXT("UIManagerComponent ptr: %p"), UIManagerComponent);
	UE_LOG(LogTemp, Error, TEXT("SubTargetManager ptr: %p"), SubTargetManager);
	UE_LOG(LogTemp, Error, TEXT("SoulsCameraManager ptr: %p"), SoulsCameraManager);
	UE_LOG(LogTemp, Error, TEXT("================================================="));
}

// Called every frame
void AMycharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ===== Debug: Draw detection sphere (visibility controlled in editor) =====
	if (LockOnDetectionSphere && bShowLockOnDebugSphere)
	{
		DrawDebugSphere(
			GetWorld(),
			LockOnDetectionSphere->GetComponentLocation(),
			LockOnDetectionSphere->GetScaledSphereRadius(),
			24,                    // Segments
			bIsLockedOn ? FColor::Red : FColor::Green,  // Red when locked, green otherwise
			false,                 // Not persistent
			-1.0f,                // Duration: one frame
			0,                    // Depth priority
			DebugSphereLineThickness  // Use configurable line thickness
		);
	}

	// ===== Debug: Check actors inside sphere every second =====
	static float SphereDebugTimer = 0.0f;
	SphereDebugTimer += DeltaTime;
	if (bEnableLockOnSphereDebugLog && SphereDebugTimer >= 1.0f)
	{
		SphereDebugTimer = 0.0f;
		
		if (LockOnDetectionSphere)
		{
			TArray<AActor*> AllOverlapping;
			LockOnDetectionSphere->GetOverlappingActors(AllOverlapping);
			
			TArray<AActor*> OverlappingPawns;
			LockOnDetectionSphere->GetOverlappingActors(OverlappingPawns, APawn::StaticClass());
			
			UE_LOG(LogTemp, Warning, TEXT("----- Sphere Status -----"));
			UE_LOG(LogTemp, Warning, TEXT("All Overlapping: %d"), AllOverlapping.Num());
			UE_LOG(LogTemp, Warning, TEXT("Overlapping Pawns: %d"), OverlappingPawns.Num());
			
			for (AActor* Actor : OverlappingPawns)
			{
				if (Actor && Actor != this)
				{
					float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
					UE_LOG(LogTemp, Warning, TEXT("  - %s (Dist: %.1f)"), *Actor->GetName(), Dist);
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("-------------------------"));
		}
	}

	// Regenerate stamina
	RegenerateStamina(DeltaTime);

	// Consume stamina while running
	if (bIsRunning && (ForwardInput != 0.0f || RightInput != 0.0f))
	{
		if (!ConsumeStamina(RunStaminaCostPerSecond * DeltaTime))
		{
			StopRunning();
		}
	}

	// Update lock-on
	if (bIsLockedOn)
	{
		UpdateLockOnTarget();
		
		// ★ 新增：锁定时让角色朝向敌人
		if (CurrentLockOnTarget && Controller)
		{
			FVector DirectionToTarget = CurrentLockOnTarget->GetActorLocation() - GetActorLocation();
			DirectionToTarget.Z = 0;  // 忽略高度差
			
			if (!DirectionToTarget.IsNearlyZero())
			{
				FRotator TargetRotation = DirectionToTarget.Rotation();
				FRotator CurrentRotation = Controller->GetControlRotation();
				
				// 只更新 Yaw，保持 Pitch 不变（由相机系统控制）
				FRotator NewRotation = FMath::RInterpTo(
					CurrentRotation,
					FRotator(CurrentRotation.Pitch, TargetRotation.Yaw, 0.0f),
					DeltaTime,
					10.0f  // 旋转速度，可调整
				);
				
				Controller->SetControlRotation(NewRotation);
			}
		}
	}

	// ========== Camera Reset Update ==========
	if (bIsCameraResetting)
	{
		UpdateCameraReset(DeltaTime);
	}

	// ========== Auto Realign Update (P4-B) ==========
	if (bEnableAutoRealign && !bIsCameraResetting)
	{
		UpdateCharacterScreenPosition();
		UpdateAutoRealign(DeltaTime);
	}

	// Update character state based on movement
	if (CurrentState == ECharacterState::Idle || CurrentState == ECharacterState::Walking || CurrentState == ECharacterState::Running)
	{
		if (GetVelocity().SizeSquared() > 0.0f)
		{
			CurrentState = bIsRunning ? ECharacterState::Running : ECharacterState::Walking;
		}
		else
		{
			CurrentState = ECharacterState::Idle;
		}
	}
}

// Called to bind functionality to input
void AMycharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// ===== Debug log: Confirm input binding is called =====
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("SetupPlayerInputComponent CALLED"));
	UE_LOG(LogTemp, Error, TEXT("PlayerInputComponent Valid: %s"), PlayerInputComponent ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));

	// Movement
	PlayerInputComponent->BindAxis("MoveForward", this, &AMycharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMycharacter::MoveRight);

	// Camera - Bind all view control inputs
	PlayerInputComponent->BindAxis("Turn", this, &AMycharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMycharacter::TurnRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AMycharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMycharacter::LookUpRate);

	// Right stick for target switching
	PlayerInputComponent->BindAxis("RightStickX", this, &AMycharacter::HandleRightStickX);

	// Actions
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AMycharacter::StartRunning);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AMycharacter::StopRunning);

	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AMycharacter::DodgeRoll);

	PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &AMycharacter::LightAttack);
	PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &AMycharacter::HeavyAttack);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AMycharacter::StartBlocking);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AMycharacter::StopBlocking);

	// Lock-on input - Support both action names
	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AMycharacter::ToggleLockOn);
	PlayerInputComponent->BindAction("SetLockon", IE_Pressed, this, &AMycharacter::ToggleLockOn);
	
	// Log to confirm binding
	UE_LOG(LogTemp, Warning, TEXT("LockOn and SetLockon actions bound to ToggleLockOn"));
	
	// Optional dedicated buttons for target switching
	PlayerInputComponent->BindAction("SwitchTargetLeft", IE_Pressed, this, &AMycharacter::SwitchLockOnTargetLeft);
	PlayerInputComponent->BindAction("SwitchTargetRight", IE_Pressed, this, &AMycharacter::SwitchLockOnTargetRight);

	// ========== Camera Reset ==========
	PlayerInputComponent->BindAction("CameraReset", IE_Pressed, this, &AMycharacter::TriggerCameraReset);
	UE_LOG(LogTemp, Log, TEXT("CameraReset action bound to TriggerCameraReset"));

	// ===== Debug key bindings =====
	// Bind F key for fallback lock-on testing
	PlayerInputComponent->BindAction("DebugLockOn", IE_Pressed, this, &AMycharacter::ToggleLockOnFallback);
	
	// Bind G key to list all pawns
	PlayerInputComponent->BindAction("DebugListPawns", IE_Pressed, this, &AMycharacter::DebugListAllPawns);
	
	UE_LOG(LogTemp, Warning, TEXT("Debug keys bound: F=FallbackLockOn, G=ListAllPawns"));
	UE_LOG(LogTemp, Warning, TEXT("(Note: Add these to Project Settings > Input if not already defined)"));
}

// ========== Movement Functions ==========

void AMycharacter::MoveForward(float Value)
{
	ForwardInput = Value;

	if (!CanMove() || Value == 0.0f)
		return;

	if (bIsLockedOn && CurrentLockOnTarget)
	{
		// When locked on, move relative to camera
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
	else
	{
		// Normal movement
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMycharacter::MoveRight(float Value)
{
	RightInput = Value;

	if (!CanMove() || Value == 0.0f)
		return;

	if (bIsLockedOn && CurrentLockOnTarget)
	{
		// When locked on, strafe
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
	else
	{
		// Normal movement
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMycharacter::LookUp(float Value)
{
	// Disable pitch control completely when locked on (mouse Y-axis and gamepad right stick Y-axis)
	if (bIsLockedOn)
	{
		// Don't process any LookUp input when locked on
		return;
	}
	
	// Not locked on: process view control normally
	AddControllerPitchInput(Value);
}

void AMycharacter::LookUpRate(float Rate)
{
	// Disable pitch control completely when locked on (gamepad right stick Y-axis)
	if (bIsLockedOn)
	{
		// Don't process any LookUp input when locked on
		return;
	}
	
	// Not locked on: use frame-rate independent rate control
	AddControllerPitchInput(Rate * GetWorld()->GetDeltaSeconds());
}

void AMycharacter::Turn(float Value)
{
	// Disable yaw control completely when locked on (mouse X-axis and gamepad right stick X-axis)
	if (bIsLockedOn)
	{
		// Don't process any Turn input when locked on
		return;
	}
	
	// Not locked on: process view control normally
	AddControllerYawInput(Value);
}

void AMycharacter::TurnRate(float Rate)
{
	// Disable yaw control completely when locked on (gamepad right stick X-axis)
	if (bIsLockedOn)
	{
		// Don't process any Turn input when locked on
		return;
	}
	
	// Not locked on: use frame-rate independent rate control
	AddControllerYawInput(Rate * GetWorld()->GetDeltaSeconds());
}

// ========== Running ==========

void AMycharacter::StartRunning()
{
	if (CanPerformAction() && CurrentStamina > 0.0f)
	{
		bIsRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

void AMycharacter::StopRunning()
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = bIsLockedOn ? LockedOnSpeed : WalkSpeed;
}

// ========== Dodge System ==========

void AMycharacter::DodgeRoll()
{
	if (!CanPerformAction() || bIsDodging || !ConsumeStamina(DodgeCost))
		return;

	PerformDodge();
}

void AMycharacter::PerformDodge()
{
	bIsDodging = true;
	bIsInvincible = true;
	SetCharacterState(ECharacterState::Dodging);

	// Calculate dodge direction
	FVector DodgeDirection = GetActorForwardVector();
	
	if (ForwardInput != 0.0f || RightInput != 0.0f)
	{
		// Dodge in movement direction
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		DodgeDirection = (FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) * ForwardInput + 
		                  FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) * RightInput).GetSafeNormal();
	}
	else
	{
		// Dodge backward if not moving
		DodgeDirection = -GetActorForwardVector();
	}

	// Launch character
	FVector LaunchVelocity = DodgeDirection * (DodgeDistance / DodgeDuration);
	LaunchCharacter(LaunchVelocity, true, true);

	// Set timers
	GetWorldTimerManager().SetTimer(DodgeTimerHandle, [this]()
	{
		bIsDodging = false;
		SetCharacterState(ECharacterState::Idle);
	}, DodgeDuration, false);

	GetWorldTimerManager().SetTimer(InvincibilityTimerHandle, [this]()
	{
		bIsInvincible = false;
	}, DodgeInvincibilityDuration, false);
}

// ========== Combat System ==========

void AMycharacter::LightAttack()
{
	if (!CanPerformAction() || !ConsumeStamina(AttackCost))
		return;

	PerformAttack(false);
}

void AMycharacter::HeavyAttack()
{
	if (!CanPerformAction() || !ConsumeStamina(AttackCost * 1.5f))
		return;

	PerformAttack(true);
}

void AMycharacter::PerformAttack(bool bIsHeavy)
{
	SetCharacterState(ECharacterState::Attacking);
	
	AttackComboCount++;
	
	// Reset combo timer
	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);
	GetWorldTimerManager().SetTimer(ComboResetTimerHandle, this, &AMycharacter::ResetCombo, ComboResetTime, false);

	// Here you would play attack animation montage
	// PlayAnimMontage(bIsHeavy ? HeavyAttackMontage : LightAttackMontage);

	// Simulate attack duration (should be driven by animation in real implementation)
	GetWorldTimerManager().SetTimer(DodgeTimerHandle, [this]()
	{
		if (CurrentState == ECharacterState::Attacking)
		{
			SetCharacterState(ECharacterState::Idle);
		}
	}, bIsHeavy ? 1.0f : 0.6f, false);

	UE_LOG(LogTemp, Warning, TEXT("Performing %s Attack - Combo: %d"), 
		bIsHeavy ? TEXT("Heavy") : TEXT("Light"), AttackComboCount);
}

void AMycharacter::ResetCombo()
{
	AttackComboCount = 0;
}

void AMycharacter::StartBlocking()
{
	if (!CanPerformAction())
		return;

	bIsBlocking = true;
	SetCharacterState(ECharacterState::Blocking);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.5f;
}

void AMycharacter::StopBlocking()
{
	bIsBlocking = false;
	if (CurrentState == ECharacterState::Blocking)
	{
		SetCharacterState(ECharacterState::Idle);
		GetCharacterMovement()->MaxWalkSpeed = bIsLockedOn ? LockedOnSpeed : WalkSpeed;
	}
}

void AMycharacter::ApplyDamage(float DamageAmount, bool bCanBlock)
{
	if (bIsInvincible)
		return;

	float ActualDamage = DamageAmount;

	if (bIsBlocking && bCanBlock)
	{
		ActualDamage *= (1.0f - BlockDamageReduction);
		ConsumeStamina(DamageAmount * 0.5f);
		UE_LOG(LogTemp, Warning, TEXT("Blocked! Damage reduced to: %f"), ActualDamage);
	}

	CurrentHealth -= ActualDamage;
	CurrentHealth = FMath::Max(0.0f, CurrentHealth);

	UE_LOG(LogTemp, Warning, TEXT("Taking Damage: %f, Health: %f/%f"), ActualDamage, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		SetCharacterState(ECharacterState::Dead);
		// Handle death
	}
}

// ========== Modular Lock-On System ==========

void AMycharacter::ToggleLockOn()
{
	// Debug logs to confirm input received
	UE_LOG(LogTemp, Error, TEXT("==========================================="));
	UE_LOG(LogTemp, Error, TEXT("TOGGLE LOCK-ON PRESSED"));
	UE_LOG(LogTemp, Error, TEXT("==========================================="));
	
	// Display debug message on screen (easier to see)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("LOCK-ON BUTTON PRESSED!"));
	}

	UE_LOG(LogTemp, Warning, TEXT("===== ToggleLockOn Called ====="));
	UE_LOG(LogTemp, Warning, TEXT("Current bIsLockedOn: %s"), bIsLockedOn ? TEXT("TRUE") : TEXT("FALSE"));
	
	if (bIsLockedOn)
	{
		CancelLockOn();
	}
	else
	{
		if (TargetDetectionComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetDetectionComponent is valid, finding candidates..."));
			TargetDetectionComponent->FindLockOnCandidates();
			LockOnCandidates = TargetDetectionComponent->LockOnCandidates;
			
			UE_LOG(LogTemp, Warning, TEXT("Found %d lock-on candidates"), LockOnCandidates.Num());
			
			AActor* Target = TargetDetectionComponent->TryGetSectorLockTarget();
			if (Target)
			{
				UE_LOG(LogTemp, Warning, TEXT("Best target found: %s"), *Target->GetName());
				StartLockOn(Target);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No valid lock-on target found"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("TargetDetectionComponent is NULL!"));
		}
	}
}

void AMycharacter::StartLockOn(AActor* Target)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("StartLockOn: Target is NULL!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("===== StartLockOn ====="));
	UE_LOG(LogTemp, Warning, TEXT("Target: %s"), *Target->GetName());

	// ===== Debug: List all sockets on target =====
	if (UIManagerComponent)
	{
		UIManagerComponent->DebugListTargetSockets(Target);
	}

	// Clear cached lock method if target changed
	if (CurrentLockOnTarget != Target)
	{
		CachedLockMethod = ELockMethod::None;
	}

	bIsLockedOn = true;
	CurrentLockOnTarget = Target;
	LockedOnTarget = Target; // Keep legacy variable in sync

	// Disable character auto-rotation to movement direction
	GetCharacterMovement()->bOrientRotationToMovement = false;
	// ★ 新增：锁定时启用控制器旋转控制角色朝向
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->MaxWalkSpeed = LockedOnSpeed;

	// Show UI
	if (UIManagerComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerComponent is valid, showing widget..."));
		UE_LOG(LogTemp, Warning, TEXT("LockOnWidgetClass is %s"), UIManagerComponent->LockOnWidgetClass ? TEXT("SET") : TEXT("NULL"));
		UIManagerComponent->ShowLockOnWidget(Target);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UIManagerComponent is NULL!"));
	}

	// ★ 通知相机系统锁定状态变化
	// ==================== 通知 SubTargetManager ====================
	if (SubTargetManager)
	{
		SubTargetManager->SetLockedEntity(Target);
	}
	NotifyCameraLockOnStateChanged(true, Target);

	// ===== Input control state change log =====
	UE_LOG(LogTemp, Warning, TEXT("[INPUT LOCK] Camera control DISABLED:"));
	UE_LOG(LogTemp, Warning, TEXT("  - Mouse X/Y: BLOCKED"));
	UE_LOG(LogTemp, Warning, TEXT("  - Gamepad Right Stick Y: BLOCKED"));
	UE_LOG(LogTemp, Warning, TEXT("  - Gamepad Right Stick X: TARGET SWITCHING ENABLED"));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
			TEXT("Locked On - Camera Control Disabled | Right Stick: Switch Targets"));
	}

	UE_LOG(LogTemp, Log, TEXT("Locked on to: %s"), *Target->GetName());
}

void AMycharacter::CancelLockOn()
{
	if (!bIsLockedOn)
		return;

	// Clear UI first
	if (UIManagerComponent)
	{
		UIManagerComponent->HideLockOnWidget();
	}

	// Reset state
	bIsLockedOn = false;
	AActor* PreviousTarget = CurrentLockOnTarget;
	CurrentLockOnTarget = nullptr;
	LockedOnTarget = nullptr; // Keep legacy variable in sync

	// ★ 新增：解锁时禁用控制器旋转，恢复朝向移动
	bUseControllerRotationYaw = false;
	// Restore character rotation behavior
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;

	// ★ 通知相机系统解除锁定
	// ==================== 清除 SubTargetManager ====================
	if (SubTargetManager)
	{
		SubTargetManager->ClearLockedEntity();
	}
	NotifyCameraLockOnStateChanged(false, nullptr);

	// ===== Input control state restore log =====
	UE_LOG(LogTemp, Warning, TEXT("[INPUT UNLOCK] Camera control RESTORED:"));
	UE_LOG(LogTemp, Warning, TEXT("  - Mouse X/Y: ENABLED"));
	UE_LOG(LogTemp, Warning, TEXT("  - Gamepad Right Stick X/Y: ENABLED"));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			TEXT("Lock Released - Camera Control Restored"));
	}

	UE_LOG(LogTemp, Log, TEXT("Lock-on cancelled"));
}

void AMycharacter::UpdateLockOnTarget()
{
	if (!bIsLockedOn || !CurrentLockOnTarget)
		return;

	if (TargetDetectionComponent)
	{
		if (!TargetDetectionComponent->IsTargetStillLockable(CurrentLockOnTarget))
		{
			UE_LOG(LogTemp, Warning, TEXT("Target no longer lockable, cancelling lock-on"));
			CancelLockOn();
		}
	}
}

void AMycharacter::SwitchLockOnTargetLeft()
{
	if (!bIsLockedOn || LockOnCandidates.Num() == 0)
		return;

	// ==================== 分层决策：先尝试 Entity 内部切换 ====================
	if (SubTargetManager && SubTargetManager->IsMultiTargetEntity())
	{
		bool bSwitched = SubTargetManager->TrySwitchSubTarget(-1.0f); // 负值 = 向左
		if (bSwitched)
		{
			UE_LOG(LogTemp, Log, TEXT("[SubTarget] Internal switch LEFT within entity %s"),
				CurrentLockOnTarget ? *CurrentLockOnTarget->GetName() : TEXT("NULL"));
			return; // 输入已被消耗，不做 Entity 级切换
		}
		// 到达 Entity 内部最左边，继续往下走 Entity 级切换（带黏性判定）
	}

	// ==================== Entity 级切换（原有逻辑 + 黏性评分） ====================
	// Update candidates first
	if (TargetDetectionComponent)
	{
		TargetDetectionComponent->FindLockOnCandidates();
		LockOnCandidates = TargetDetectionComponent->LockOnCandidates;
	}

	if (LockOnCandidates.Num() == 0)
		return;

	// Sort candidates left to right
	TArray<AActor*> Sorted = LockOnCandidates;
	if (TargetDetectionComponent)
	{
		TargetDetectionComponent->SortCandidatesByDirection(Sorted);
	}

	// Find current index
	int32 CurrentIndex = Sorted.Find(CurrentLockOnTarget);
	if (CurrentIndex == INDEX_NONE)
		CurrentIndex = 0;

	// ===== 黏性检查：当前 Entity 是否抵抗切出 =====
	if (CurrentIndex > 0 && SubTargetManager && SubTargetManager->IsMultiTargetEntity())
	{
		// 检查下一个目标是否是不同类型（LargeEnemy vs 普通敌人）
		int32 NextIndex = CurrentIndex - 1;
		bool bSwitchingToDifferentType = false;
		if (Sorted.IsValidIndex(NextIndex) && CurrentLockOnTarget)
		{
			bool bCurrentIsLarge = CurrentLockOnTarget->ActorHasTag(FName("LargeEnemy"));
			bool bNextIsLarge = Sorted[NextIndex]->ActorHasTag(FName("LargeEnemy"));
			bSwitchingToDifferentType = (bCurrentIsLarge != bNextIsLarge);
		}

		if (!bSwitchingToDifferentType)
		{
			float StickinessBonus = SubTargetManager->CalculateStickinessBonus(CurrentLockOnTarget, LockOnCandidates);
		
			// ===== 临时诊断日志 =====
			bool bCurrentIsMultiTarget = SubTargetManager->IsMultiTargetEntity();
			UE_LOG(LogTemp, Warning, TEXT("[DIAG-LEFT] Target=%s | IsMultiTarget=%s | Stickiness=%.4f | Threshold=0.3 | Candidates=%d | CurrentIndex=%d"),
				CurrentLockOnTarget ? *CurrentLockOnTarget->GetName() : TEXT("NULL"),
				bCurrentIsMultiTarget ? TEXT("YES") : TEXT("NO"),
				StickinessBonus,
				Sorted.Num(),
				CurrentIndex);
			// ===== 临时诊断日志结束 =====
		
			if (StickinessBonus > 0.3f) // 黏性阈值：高于此值则拒绝切出
			{
				UE_LOG(LogTemp, Warning, TEXT("[DIAG-LEFT] >>> BLOCKED by stickiness (same type)"));
				return; // 黏性阻止切出
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[Switch] Cross-type switch LEFT: bypassing stickiness check"));
		}
	}

	// ===== NO WRAP AROUND: Stop at boundary =====
	if (CurrentIndex > 0)
	{
		int32 NewIndex = CurrentIndex - 1;
		if (Sorted.IsValidIndex(NewIndex))
		{
			SwitchToTarget(Sorted[NewIndex]);
			UE_LOG(LogTemp, Log, TEXT("Switched LEFT: Index %d -> %d (%s)"),
				CurrentIndex, NewIndex, *Sorted[NewIndex]->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Cannot switch LEFT: Already at leftmost target"));
	}
}

void AMycharacter::SwitchLockOnTargetRight()
{
	if (!bIsLockedOn || LockOnCandidates.Num() == 0)
		return;

	// ==================== 分层决策：先尝试 Entity 内部切换 ====================
	if (SubTargetManager && SubTargetManager->IsMultiTargetEntity())
	{
		bool bSwitched = SubTargetManager->TrySwitchSubTarget(1.0f); // 正值 = 向右
		if (bSwitched)
		{
			UE_LOG(LogTemp, Log, TEXT("[SubTarget] Internal switch RIGHT within entity %s"),
				CurrentLockOnTarget ? *CurrentLockOnTarget->GetName() : TEXT("NULL"));
			return; // 输入已被消耗，不做 Entity 级切换
		}
		// 到达 Entity 内部最右边，继续往下走 Entity 级切换（带黏性判定）
	}

	// ==================== Entity 级切换（原有逻辑 + 黏性评分） ====================
	// Update candidates first
	if (TargetDetectionComponent)
	{
		TargetDetectionComponent->FindLockOnCandidates();
		LockOnCandidates = TargetDetectionComponent->LockOnCandidates;
	}

	if (LockOnCandidates.Num() == 0)
		return;

	// Sort candidates left to right
	TArray<AActor*> Sorted = LockOnCandidates;
	if (TargetDetectionComponent)
	{
		TargetDetectionComponent->SortCandidatesByDirection(Sorted);
	}

	// Find current index
	int32 CurrentIndex = Sorted.Find(CurrentLockOnTarget);
	if (CurrentIndex == INDEX_NONE)
		CurrentIndex = 0;

	// ===== 黏性检查：当前 Entity 是否抵抗切出 =====
	if (CurrentIndex < Sorted.Num() - 1 && SubTargetManager && SubTargetManager->IsMultiTargetEntity())
	{
		// 检查下一个目标是否是不同类型（LargeEnemy vs 普通敌人）
		int32 NextIndex = CurrentIndex + 1;
		bool bSwitchingToDifferentType = false;
		if (Sorted.IsValidIndex(NextIndex) && CurrentLockOnTarget)
		{
			bool bCurrentIsLarge = CurrentLockOnTarget->ActorHasTag(FName("LargeEnemy"));
			bool bNextIsLarge = Sorted[NextIndex]->ActorHasTag(FName("LargeEnemy"));
			bSwitchingToDifferentType = (bCurrentIsLarge != bNextIsLarge);
		}

		if (!bSwitchingToDifferentType)
		{
			float StickinessBonus = SubTargetManager->CalculateStickinessBonus(CurrentLockOnTarget, LockOnCandidates);
		
			// ===== 临时诊断日志 =====
			bool bCurrentIsMultiTarget = SubTargetManager->IsMultiTargetEntity();
			UE_LOG(LogTemp, Warning, TEXT("[DIAG-RIGHT] Target=%s | IsMultiTarget=%s | Stickiness=%.4f | Threshold=0.3 | Candidates=%d | CurrentIndex=%d"),
				CurrentLockOnTarget ? *CurrentLockOnTarget->GetName() : TEXT("NULL"),
				bCurrentIsMultiTarget ? TEXT("YES") : TEXT("NO"),
				StickinessBonus,
				Sorted.Num(),
				CurrentIndex);
			// ===== 临时诊断日志结束 =====
		
			if (StickinessBonus > 0.3f) // 黏性阈值：高于此值则拒绝切出
			{
				UE_LOG(LogTemp, Warning, TEXT("[DIAG-RIGHT] >>> BLOCKED by stickiness (same type)"));
				return; // 黏性阻止切出
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[Switch] Cross-type switch RIGHT: bypassing stickiness check"));
		}
	}

	// ===== NO WRAP AROUND: Stop at boundary =====
	if (CurrentIndex < Sorted.Num() - 1)
	{
		int32 NewIndex = CurrentIndex + 1;
		if (Sorted.IsValidIndex(NewIndex))
		{
			SwitchToTarget(Sorted[NewIndex]);
			UE_LOG(LogTemp, Log, TEXT("Switched RIGHT: Index %d -> %d (%s)"),
				CurrentIndex, NewIndex, *Sorted[NewIndex]->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Cannot switch RIGHT: Already at rightmost target"));
	}
}

void AMycharacter::SwitchToTarget(AActor* NewTarget)
{
	if (!NewTarget || NewTarget == CurrentLockOnTarget)
		return;

	AActor* OldTarget = CurrentLockOnTarget;
	
	// Update lock-on
	CurrentLockOnTarget = NewTarget;
	LockedOnTarget = NewTarget; // Keep legacy variable in sync
	CachedLockMethod = ELockMethod::None; // Reset cache for new target

	// Update UI
	if (UIManagerComponent)
	{
		UIManagerComponent->UpdateLockOnWidget(NewTarget, OldTarget);
	}

	// ==================== 通知 SubTargetManager 切换 Entity ====================
	if (SubTargetManager)
	{
		SubTargetManager->SetLockedEntity(NewTarget);
	}

	// ★ 通知相机系统新的目标
	NotifyCameraLockOnStateChanged(true, NewTarget);

	UE_LOG(LogTemp, Log, TEXT("Switched lock-on to: %s"), *NewTarget->GetName());
}

void AMycharacter::HandleRightStickX(float Value)
{
	// === Not locked on ===
	// When not locked on, right stick X-axis controls view via Turn axis binding
	// Here we only need to update LastRightStickX for edge detection
	if (!bIsLockedOn)
	{
		LastRightStickX = Value;
		return;
	}

	// === Locked on: Handle target switching ===
	
	// Cooldown check: Prevent frequent switching
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastTargetSwitchTime < TargetSwitchCooldown)
	{
		LastRightStickX = Value;
		return;
	}

	// Edge detection: Only trigger switch when crossing threshold (avoid continuous triggering)
	bool bWasLeft = LastRightStickX < -THUMBSTICK_THRESHOLD;
	bool bIsLeft = Value < -THUMBSTICK_THRESHOLD;
	bool bWasRight = LastRightStickX > THUMBSTICK_THRESHOLD;
	bool bIsRight = Value > THUMBSTICK_THRESHOLD;

	// Push right stick left: Switch to left target
	if (bIsLeft && !bWasLeft)
	{
		SwitchLockOnTargetLeft();
		LastTargetSwitchTime = CurrentTime;
		
		UE_LOG(LogTemp, Log, TEXT("[Input] Right Stick LEFT - Switching to left target"));
	}
	// Push right stick right: Switch to right target
	else if (bIsRight && !bWasRight)
	{
		SwitchLockOnTargetRight();
		LastTargetSwitchTime = CurrentTime;
		
		UE_LOG(LogTemp, Log, TEXT("[Input] Right Stick RIGHT - Switching to right target"));
	}

	LastRightStickX = Value;
}

FVector AMycharacter::GetOptimalLockPosition(AActor* Target)
{
	if (!Target)
		return FVector::ZeroVector;

	// 1. Check for per-actor config override
	ULockOnConfigComponent* Config = Target->FindComponentByClass<ULockOnConfigComponent>();
	if (Config && Config->bOverrideOffset && Config->IsConfigValid())
	{
		if (Config->bPreferSocket)
		{
			// Try to get socket location
			FName SocketName = Config->GetEffectiveSocketName();
			if (!SocketName.IsNone())
			{
				ACharacter* Character = Cast<ACharacter>(Target);
				if (Character && Character->GetMesh() && Character->GetMesh()->DoesSocketExist(SocketName))
				{
					CachedLockMethod = ELockMethod::Socket;
					CachedLockOffset = FVector::ZeroVector;
					return Character->GetMesh()->GetSocketLocation(SocketName);
				}
			}
		}
		
		// Use custom offset
		CachedLockMethod = ELockMethod::Root;
		CachedLockOffset = Config->GetEffectiveOffset();
		return Target->GetActorLocation() + CachedLockOffset;
	}

	// 2. Check cache for same target
	if (Target == CurrentLockOnTarget && CachedLockMethod != ELockMethod::None)
	{
		switch (CachedLockMethod)
		{
		case ELockMethod::Socket:
			{
				ACharacter* Character = Cast<ACharacter>(Target);
				if (Character && Character->GetMesh())
				{
					// Try default socket names
					TArray<FName> SocketNames = {FName("LockOnSocket"), FName("HeadSocket"), FName("head")};
					for (FName SocketName : SocketNames)
					{
						if (Character->GetMesh()->DoesSocketExist(SocketName))
						{
							return Character->GetMesh()->GetSocketLocation(SocketName);
						}
					}
				}
			}
			break;
		case ELockMethod::Capsule:
			{
				ACharacter* Character = Cast<ACharacter>(Target);
				if (Character && Character->GetCapsuleComponent())
				{
					return Character->GetCapsuleComponent()->GetComponentLocation() + CachedLockOffset;
				}
			}
			break;
		case ELockMethod::Root:
			return Target->GetActorLocation() + CachedLockOffset;
		}
	}

	// 3. Priority: Socket > Capsule > Root
	// Try default socket names
	ACharacter* Character = Cast<ACharacter>(Target);
	if (Character && Character->GetMesh())
	{
		TArray<FName> SocketNames = {FName("LockOnSocket"), FName("HeadSocket"), FName("head")};
		for (FName SocketName : SocketNames)
		{
			if (Character->GetMesh()->DoesSocketExist(SocketName))
			{
				CachedLockMethod = ELockMethod::Socket;
				CachedLockOffset = FVector::ZeroVector;
				return Character->GetMesh()->GetSocketLocation(SocketName);
			}
		}
	}

	// Try capsule center with height ratio
	if (Character && Character->GetCapsuleComponent())
	{
		float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		FVector Offset = FVector(0.0f, 0.0f, CapsuleHalfHeight * 0.5f);
		CachedLockMethod = ELockMethod::Capsule;
		CachedLockOffset = Offset;
		return Character->GetCapsuleComponent()->GetComponentLocation() + Offset;
	}

	// Fall back to actor location with default offset
	CachedLockMethod = ELockMethod::Root;
	CachedLockOffset = FVector(0.0f, 0.0f, 50.0f);
	return Target->GetActorLocation() + CachedLockOffset;
}

// ========== Legacy Lock-On Functions (for backward compatibility) ==========

void AMycharacter::FindLockOnTarget()
{
	// Redirect to new system
	ToggleLockOn();
}

void AMycharacter::UpdateLockOn()
{
	// Legacy implementation kept for compatibility
	if (!LockedOnTarget)
	{
		ClearLockOn();
		return;
	}

	// Check if target is too far
	float Distance = FVector::Dist(GetActorLocation(), LockedOnTarget->GetActorLocation());
	if (Distance > LockOnBreakDistance)
	{
		ClearLockOn();
		return;
	}

	// Rotate to face target
	FVector DirectionToTarget = LockedOnTarget->GetActorLocation() - GetActorLocation();
	DirectionToTarget.Z = 0;
	FRotator TargetRotation = DirectionToTarget.Rotation();
	
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 10.0f));

	// Update camera to look at target
	if (Controller && FollowCameraRef)
	{
		FVector CameraDirection = LockedOnTarget->GetActorLocation() - FollowCameraRef->GetComponentLocation();
		FRotator CameraRotation = CameraDirection.Rotation();
		Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), CameraRotation, GetWorld()->GetDeltaSeconds(), 5.0f));
	}
}

void AMycharacter::ClearLockOn()
{
	// Redirect to new system
	CancelLockOn();
}

// ========== Debug Fallback Lock-On Methods ==========

void AMycharacter::DebugListAllPawns()
{
	UE_LOG(LogTemp, Error, TEXT("======== DEBUG: ALL PAWNS IN WORLD ========"));
	
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);
	
	UE_LOG(LogTemp, Warning, TEXT("Total Pawns found: %d"), AllPawns.Num());
	
	FVector MyLocation = GetActorLocation();
	
	for (AActor* Pawn : AllPawns)
	{
		if (!Pawn) continue;
		
		float Distance = FVector::Dist(MyLocation, Pawn->GetActorLocation());
		bool bIsPlayer = (Pawn == this);
		
		UE_LOG(LogTemp, Warning, TEXT("  %s%s - Distance: %.1f - Class: %s"),
			bIsPlayer ? TEXT("[PLAYER] ") : TEXT(""),
			*Pawn->GetName(),
			Distance,
			*Pawn->GetClass()->GetName());
			
		// Draw lines pointing to each Pawn in the world
		if (!bIsPlayer)
		{
			FColor LineColor = Distance <= LockOnRange ? FColor::Yellow : FColor::Silver;
			DrawDebugLine(
				GetWorld(),
				MyLocation + FVector(0, 0, 50),
				Pawn->GetActorLocation() + FVector(0, 0, 50),
				LineColor,
				false,
				2.0f,
				0,
				2.0f
			);
			
			// Draw sphere at Pawn location
			FColor SphereColor = Distance <= LockOnRange ? FColor::Yellow : FColor::Silver;
			DrawDebugSphere(
				GetWorld(),
				Pawn->GetActorLocation(),
				50.0f,
				12,
				SphereColor,
				false,
				2.0f
			);
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("============================================"));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
			FString::Printf(TEXT("Found %d Pawns - Check Output Log"), AllPawns.Num()));
	}
}

void AMycharacter::ToggleLockOnFallback()
{
	UE_LOG(LogTemp, Error, TEXT("======== FALLBACK LOCK-ON ========"));
	
	if (bIsLockedOn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Currently locked - Cancelling"));
		CancelLockOn();
		return;
	}
	
	// Search all Pawns in the world directly
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);
	
	UE_LOG(LogTemp, Warning, TEXT("Found %d Pawns in world"), AllPawns.Num());
	
	AActor* ClosestEnemy = nullptr;
	float ClosestDistance = LockOnRange;
	FVector MyLocation = GetActorLocation();
	
	for (AActor* Pawn : AllPawns)
	{
		// Skip self
		if (Pawn == this)
		{
			UE_LOG(LogTemp, Log, TEXT("  Skipping self"));
			continue;
		}
		
		float Distance = FVector::Dist(MyLocation, Pawn->GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("  %s: Distance=%.1f"), *Pawn->GetName(), Distance);
		
		// Check if within range and is closest
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestEnemy = Pawn;
			UE_LOG(LogTemp, Warning, TEXT("    -> New closest target!"));
		}
	}
	
	if (ClosestEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("LOCKING ON TO: %s (Distance: %.1f)"), 
			*ClosestEnemy->GetName(), ClosestDistance);
		StartLockOn(ClosestEnemy);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
				FString::Printf(TEXT("Locked onto: %s"), *ClosestEnemy->GetName()));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NO VALID TARGET FOUND within range %.1f"), LockOnRange);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No target in range!"));
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("=================================="));
}

// ========== State Management ==========

bool AMycharacter::CanPerformAction() const
{
	return CurrentState != ECharacterState::Dodging && 
	       CurrentState != ECharacterState::Attacking &&
	       CurrentState != ECharacterState::Staggered &&
	       CurrentState != ECharacterState::Dead;
}

bool AMycharacter::CanMove() const
{
	return CurrentState != ECharacterState::Dodging &&
	       CurrentState != ECharacterState::Attacking &&
	       CurrentState != ECharacterState::Staggered &&
	       CurrentState != ECharacterState::Dead;
}

void AMycharacter::SetCharacterState(ECharacterState NewState)
{
	CurrentState = NewState;
}

// ========== Stamina Management ==========

void AMycharacter::RegenerateStamina(float DeltaTime)
{
	if (CurrentStamina < MaxStamina && !bIsRunning && !bIsBlocking && CurrentState != ECharacterState::Attacking)
	{
		CurrentStamina += StaminaRegenRate * DeltaTime;
		CurrentStamina = FMath::Min(CurrentStamina, MaxStamina);
	}
}

bool AMycharacter::ConsumeStamina(float Amount)
{
	if (CurrentStamina >= Amount)
	{
		CurrentStamina -= Amount;
		return true;
	}
	return false;
}

// ========== Camera Reset Implementation ==========

void AMycharacter::TriggerCameraReset()
{
	// 锁定状态下不允许手动回正（相机由锁定系统控制）
	if (bIsLockedOn)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraReset: Ignored - Currently locked on"));
		return;
	}
	
	// 获取 Controller
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraReset: No PlayerController found"));
		return;
	}
	
	// 设置目标 Yaw 为角色当前朝向
	CameraResetTargetYaw = GetActorRotation().Yaw;
	
	// 开始回正
	bIsCameraResetting = true;
	CameraResetInterpSpeed = CameraResetSpeed;
	
	UE_LOG(LogTemp, Log, TEXT("CameraReset: Started - Target Yaw: %.1f"), CameraResetTargetYaw);
	
	// 屏幕提示（可选，调试用）
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Camera Reset"));
	}
}

void AMycharacter::UpdateCameraReset(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		bIsCameraResetting = false;
		return;
	}
	
	// 如果锁定了，停止回正
	if (bIsLockedOn)
	{
		bIsCameraResetting = false;
		UE_LOG(LogTemp, Log, TEXT("CameraReset: Cancelled - Lock-on activated"));
		return;
	}
	
	// 获取当前 Control Rotation
	FRotator CurrentRotation = PC->GetControlRotation();
	
	// 计算目标 Rotation
	FRotator TargetRotation;
	TargetRotation.Yaw = CameraResetTargetYaw;
	TargetRotation.Roll = 0.0f;
	
	// Pitch 处理
	if (bResetPitchOnCameraReset)
	{
		TargetRotation.Pitch = DefaultCameraResetPitch;
	}
	else
	{
		TargetRotation.Pitch = CurrentRotation.Pitch;  // 保持当前 Pitch
	}
	
	// 平滑插值
	FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		CameraResetInterpSpeed
	);
	
	// 应用新的 Rotation
	PC->SetControlRotation(NewRotation);
	
	// 检查是否完成
	float YawDifference = FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - CameraResetTargetYaw));
	bool bYawComplete = YawDifference < CameraResetCompletionThreshold;
	
	bool bPitchComplete = true;
	if (bResetPitchOnCameraReset)
	{
		float PitchDifference = FMath::Abs(NewRotation.Pitch - DefaultCameraResetPitch);
		bPitchComplete = PitchDifference < CameraResetCompletionThreshold;
	}
	
	if (bYawComplete && bPitchComplete)
	{
		bIsCameraResetting = false;
		UE_LOG(LogTemp, Log, TEXT("CameraReset: Completed"));
	}
	
	// 如果玩家有相机输入，中断回正（让玩家控制优先）
	float YawInput = GetInputAxisValue(TEXT("Turn"));
	float PitchInput = GetInputAxisValue(TEXT("LookUp"));
	float TurnRateInput = GetInputAxisValue(TEXT("TurnRate"));
	float LookUpRateInput = GetInputAxisValue(TEXT("LookUpRate"));
	
	float TotalInput = FMath::Abs(YawInput) + FMath::Abs(PitchInput) + 
	                   FMath::Abs(TurnRateInput) + FMath::Abs(LookUpRateInput);
	
	if (TotalInput > 0.1f)
	{
		bIsCameraResetting = false;
		UE_LOG(LogTemp, Log, TEXT("CameraReset: Interrupted by player input"));
	}
}

// ========== Auto Realign Implementation (P4-B) ==========

void AMycharacter::UpdateCharacterScreenPosition()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		CharacterScreenPosition = FVector2D(0.5f, 0.5f);
		return;
	}
	
	// 将角色世界坐标投影到屏幕坐标
	FVector2D ScreenPos;
	bool bProjected = PC->ProjectWorldLocationToScreen(GetActorLocation(), ScreenPos);
	
	if (!bProjected)
	{
		// 如果角色在屏幕外，根据相对位置估算
		FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
		FVector ToCharacter = GetActorLocation() - CameraLocation;
		FVector CameraRight = PC->PlayerCameraManager->GetCameraRotation().RotateVector(FVector::RightVector);
		
		float DotRight = FVector::DotProduct(ToCharacter.GetSafeNormal(), CameraRight);
		
		// 估算屏幕位置
		CharacterScreenPosition.X = 0.5f + DotRight * 0.5f;
		CharacterScreenPosition.Y = 0.8f; // 假设在屏幕下方
		return;
	}
	
	// 获取视口尺寸
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		CharacterScreenPosition = FVector2D(0.5f, 0.5f);
		return;
	}
	
	// 转换为 0-1 范围（0.5 = 屏幕中心）
	CharacterScreenPosition.X = ScreenPos.X / (float)ViewportSizeX;
	CharacterScreenPosition.Y = ScreenPos.Y / (float)ViewportSizeY;
	
	// Clamp 到 0-1 范围
	CharacterScreenPosition.X = FMath::Clamp(CharacterScreenPosition.X, 0.0f, 1.0f);
	CharacterScreenPosition.Y = FMath::Clamp(CharacterScreenPosition.Y, 0.0f, 1.0f);
}

bool AMycharacter::ShouldAutoRealign() const
{
	// 条件 1：非锁定状态
	if (bIsLockedOn)
	{
		return false;
	}
	
	// 条件 2：正在移动（有移动输入）
	if (FMath::Abs(ForwardInput) < 0.1f && FMath::Abs(RightInput) < 0.1f)
	{
		return false;
	}
	
	// 条件 3：有实际位移（排除撞墙）
	float CurrentSpeed = GetVelocity().Size2D();
	if (CurrentSpeed < MinMovementSpeedForRealign)
	{
		return false;
	}
	
	// 条件 4：角色在屏幕边缘区域
	float DistanceFromCenterX = FMath::Abs(CharacterScreenPosition.X - 0.5f);
	float DistanceFromCenterY = CharacterScreenPosition.Y - 0.5f; // 只关心下方
	
	// 水平距离：超出中心区域
	bool bAtHorizontalEdge = DistanceFromCenterX > (0.5f - ScreenEdgeThreshold);
	
	// 垂直距离：在屏幕下半部分的边缘
	bool bAtBottomEdge = DistanceFromCenterY > (0.5f - ScreenEdgeThreshold);
	
	if (!bAtHorizontalEdge && !bAtBottomEdge)
	{
		return false;
	}
	
	// 条件 5：相机与角色朝向有角度差
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return false;
	}
	
	float CameraYaw = PC->GetControlRotation().Yaw;
	float ActorYaw = GetActorRotation().Yaw;
	float YawDifference = FMath::Abs(FRotator::NormalizeAxis(CameraYaw - ActorYaw));
	
	// 如果已经对齐，不需要回正
	if (YawDifference < AutoRealignCompletionThreshold)
	{
		return false;
	}
	
	return true;
}

float AMycharacter::GetScreenEdgeDistance() const
{
	// 计算角色离屏幕中心的距离（0 = 中心，1 = 边缘）
	float DistanceFromCenterX = FMath::Abs(CharacterScreenPosition.X - 0.5f) * 2.0f; // 0-1
	float DistanceFromCenterY = FMath::Max(0.0f, (CharacterScreenPosition.Y - 0.5f) * 2.0f); // 只关心下方，0-1
	
	// 取最大值
	float MaxDistance = FMath::Max(DistanceFromCenterX, DistanceFromCenterY);
	
	// 映射到阈值范围：低于阈值 = 0，边缘 = 1
	float ThresholdStart = 1.0f - (ScreenEdgeThreshold * 2.0f); // 开始触发的距离
	float NormalizedDistance = FMath::GetMappedRangeValueClamped(
		FVector2D(ThresholdStart, 1.0f),
		FVector2D(0.0f, 1.0f),
		MaxDistance
	);
	
	return NormalizedDistance;
}

float AMycharacter::GetAutoRealignDirection() const
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return 0.0f;
	}
	
	float CameraYaw = PC->GetControlRotation().Yaw;
	float ActorYaw = GetActorRotation().Yaw;
	
	// 计算从相机 Yaw 到角色 Yaw 的最短旋转方向
	float DeltaYaw = FRotator::NormalizeAxis(ActorYaw - CameraYaw);
	
	// 正值 = 需要向右转（顺时针），负值 = 需要向左转（逆时针）
	return FMath::Sign(DeltaYaw);
}

void AMycharacter::UpdateAutoRealign(float DeltaTime)
{
	// 检查是否应该回正
	if (!ShouldAutoRealign())
	{
		bIsAutoRealigning = false;
		return;
	}
	
	bIsAutoRealigning = true;
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}
	
	// 获取当前状态
	FRotator CurrentRotation = PC->GetControlRotation();
	float ActorYaw = GetActorRotation().Yaw;
	
	// 计算边缘距离（用于强度）
	float EdgeDistance = GetScreenEdgeDistance();
	
	// 计算回正方向
	float RealignDirection = GetAutoRealignDirection();
	
	// 计算回正速度（线性增加，越靠边缘越快）
	float RealignSpeed = AutoRealignMaxSpeed * EdgeDistance;
	
	// 计算这一帧的回正量
	float RealignDelta = RealignDirection * RealignSpeed * DeltaTime;
	
	// 应用回正（叠加到当前旋转）
	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw += RealignDelta;
	
	// 检查是否会过冲（超过目标）
	float NewDeltaYaw = FRotator::NormalizeAxis(ActorYaw - NewRotation.Yaw);
	float OldDeltaYaw = FRotator::NormalizeAxis(ActorYaw - CurrentRotation.Yaw);
	
	// 如果方向改变了，说明过冲了，直接设置到目标
	if (FMath::Sign(NewDeltaYaw) != FMath::Sign(OldDeltaYaw) && FMath::Abs(OldDeltaYaw) < 10.0f)
	{
		NewRotation.Yaw = ActorYaw;
	}
	
	// 应用新的旋转
	PC->SetControlRotation(NewRotation);
	
	// Debug 日志（可选）
#if WITH_EDITOR
	static float DebugTimer = 0.0f;
	DebugTimer += DeltaTime;
	if (DebugTimer >= 1.0f)
	{
		DebugTimer = 0.0f;
		UE_LOG(LogTemp, Verbose, TEXT("AutoRealign: ScreenPos(%.2f, %.2f) EdgeDist=%.2f Dir=%.0f Speed=%.1f"),
			CharacterScreenPosition.X, CharacterScreenPosition.Y,
			EdgeDistance, RealignDirection, RealignSpeed);
	}
#endif
}

// ========== Camera LockOn State Integration ==========

void AMycharacter::NotifyCameraLockOnStateChanged(bool bIsLocking, AActor* Target)
{
	// 检查是否启用
	if (!bEnableCameraLockOnStateChange)
	{
		return;
	}
	
	// 获取 SoulsCameraManager 组件
	USoulsCameraManager* CameraManager = SoulsCameraManager;
	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyCameraLockOnStateChanged: SoulsCameraManager not found!"));
		return;
	}
	
	if (bIsLocking && Target)
	{
		// 开始锁定
		CameraManager->SetLockOnTarget(Target);
		
		// 根据目标类型选择状态
		FName TargetState = LockOnCameraState;
		if (IsTargetBoss(Target))
		{
			TargetState = BossLockOnCameraState;
		}
		
		bool bSuccess = CameraManager->RequestStateChange(TargetState, false);
		
		UE_LOG(LogTemp, Log, TEXT("Camera LockOn: Target=%s, State=%s, Success=%s"),
			*Target->GetName(),
			*TargetState.ToString(),
			bSuccess ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		// 解除锁定
		CameraManager->ClearLockOnTarget();
		
		bool bSuccess = CameraManager->RequestStateChange(ExploreCameraState, false);
		
		UE_LOG(LogTemp, Log, TEXT("Camera LockOn Cleared: State=%s, Success=%s"),
			*ExploreCameraState.ToString(),
			bSuccess ? TEXT("YES") : TEXT("NO"));
	}
}

bool AMycharacter::IsTargetBoss(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}
	
	// 方法 1: 通过 Tag 检查
	if (Target->Tags.Contains(FName("Boss")))
	{
		return true;
	}
	
	// 方法 2: 你可以在这里添加其他检查逻辑
	// 例如检查 Target 是否是某个 Boss 类的实例
	
	return false;
}

