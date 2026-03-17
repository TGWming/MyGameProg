// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LockOnConfig.h"
#include "SubTargetManager.h"
#include "Mycharacter.generated.h"

class USubTargetManager;
class USoulsCameraManager;
class USoulsCameraDebugComponent;

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Dodging UMETA(DisplayName = "Dodging"),
	Attacking UMETA(DisplayName = "Attacking"),
	Blocking UMETA(DisplayName = "Blocking"),
	Staggered UMETA(DisplayName = "Staggered"),
	Dead UMETA(DisplayName = "Dead")
};

UCLASS()
class SOUL_API AMycharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMycharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========== Components =========
	
	// ========== Camera Component References - Will be found from Blueprint in BeginPlay ==========
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoomRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCameraRef;

	// ========== Camera Reset Settings ==========
	
	/** 相机回正的插值速度（越大越快，5.0 约 0.3 秒完成） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Reset")
	float CameraResetSpeed = 5.0f;
	
	/** 是否在回正时也重置 Pitch 到默认值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Reset")
	bool bResetPitchOnCameraReset = false;
	
	/** 回正时的默认 Pitch 角度（仅当 bResetPitchOnCameraReset = true 时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Reset", meta = (EditCondition = "bResetPitchOnCameraReset"))
	float DefaultCameraResetPitch = -15.0f;

	// ========== Auto Camera Realign Settings (P4-B) ==========
	
	/** 是否启用跑动时自动回正（匹诺曹风格） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|AutoRealign")
	bool bEnableAutoRealign = true;
	
	/** 自动回正的最大速度（度/秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|AutoRealign", meta = (ClampMin = "10.0", ClampMax = "120.0"))
	float AutoRealignMaxSpeed = 45.0f;
	
	/** 触发自动回正的屏幕边缘阈值（0.33 = 三分法外侧 1/3） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|AutoRealign", meta = (ClampMin = "0.1", ClampMax = "0.5"))
	float ScreenEdgeThreshold = 0.33f;
	
	/** 判定为"有实际位移"的最小速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|AutoRealign", meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float MinMovementSpeedForRealign = 50.0f;
	
	/** 自动回正完成的角度阈值（达到此角度差时停止回正） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|AutoRealign", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float AutoRealignCompletionThreshold = 3.0f;

	// ========== Camera LockOn State Settings ==========
	
	/** 锁定时相机应该切换到的状态名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|LockOn")
	FName LockOnCameraState = FName("LockOn_Hard");
	
	/** 解除锁定时相机应该切换到的状态名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|LockOn")
	FName ExploreCameraState = FName("Explore_Default");
	
	/** Boss 锁定时相机应该切换到的状态名称（可选） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|LockOn")
	FName BossLockOnCameraState = FName("LockOn_Boss");
	
	/** 是否启用相机锁定状态切换 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|LockOn")
	bool bEnableCameraLockOnStateChange = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock-On")
	class UTargetDetectionComponent* TargetDetectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock-On")
	class UUIManagerComponent* UIManagerComponent;

	/** 子锁点管理组件 - 管理多锁点的选择和切换 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USubTargetManager* SubTargetManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USoulsCameraManager* SoulsCameraManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Debug")
	USoulsCameraDebugComponent* SoulsCameraDebugComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock-On")
	class USphereComponent* LockOnDetectionSphere;

	// ========== Movement Properties =========
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LockedOnSpeed = 400.0f;

	// ========== Stamina System =========
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float StaminaRegenRate = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DodgeCost = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float RunStaminaCostPerSecond = 15.0f;

	// ========== Health System =========
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	// ========== Dodge System =========
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeInvincibilityDuration = 0.4f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge")
	bool bIsDodging = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge")
	bool bIsInvincible = false;

	// ========== Combat System =========
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	ECharacterState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsBlocking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 AttackComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ComboResetTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BlockDamageReduction = 0.8f;

	// ========== Lock-On System =========
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	bool bIsLockedOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	AActor* CurrentLockOnTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float LockOnRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float TargetSwitchCooldown = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	TArray<AActor*> LockOnCandidates;

	// ========== Debug Visualization =========
	
	/** Whether to show lock-on detection sphere (green/red wireframe) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Debug")
	bool bShowLockOnDebugSphere = true;

	/** Debug sphere line thickness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Debug", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float DebugSphereLineThickness = 3.0f;

	// ★ Debug LOG 开关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Debug")
	bool bEnableLockOnSphereDebugLog = false;

	// ========== Input Functions ==========

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookUpRate(float Rate);
	void Turn(float Value);
	void TurnRate(float Rate);
	
	void StartRunning();
	void StopRunning();

	void DodgeRoll();
	void PerformDodge();

	void LightAttack();
	void HeavyAttack();
	void PerformAttack(bool bIsHeavy);

	void StartBlocking();
	void StopBlocking();

	// New modular lock-on functions
	void ToggleLockOn();
	void StartLockOn(AActor* Target);
	void CancelLockOn();
	void UpdateLockOnTarget();
	void SwitchLockOnTargetLeft();
	void SwitchLockOnTargetRight();
	void SwitchToTarget(AActor* NewTarget);
	void HandleRightStickX(float Value);

	// ========== Camera Reset ==========
	
	/** 
	 * 触发相机平滑回正到角色背后
	 * 绑定到 CameraReset Action (Tab / Left Thumbstick Button)
	 */
	void TriggerCameraReset();
	
	/**
	 * 每帧更新相机回正插值（在 Tick 中调用）
	 */
	void UpdateCameraReset(float DeltaTime);

	/**
	 * 每帧更新自动回正（匹诺曹风格）
	 * 在 Tick 中调用
	 */
	void UpdateAutoRealign(float DeltaTime);
	
	/**
	 * 检查是否应该进行自动回正
	 */
	bool ShouldAutoRealign() const;
	
	/**
	 * 更新角色在屏幕上的位置
	 */
	void UpdateCharacterScreenPosition();

	// Legacy functions kept for compatibility
	void FindLockOnTarget();
	void UpdateLockOn();
	void ClearLockOn();

	/**
	 * Get optimal lock position for target (three-layer system)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	FVector GetOptimalLockPosition(AActor* Target);

	/**
	 * Check if currently locked on
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	bool IsLockedOn() const { return bIsLockedOn; }

	/**
	 * Check if camera input should be blocked (for use in Blueprint)
	 * Returns true if input should be BLOCKED
	 */
	UFUNCTION(BlueprintPure, Category = "Lock-On")
	bool ShouldBlockCameraInput() const { return bIsLockedOn; }
	
	/**
	 * Get current lock-on target
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	AActor* GetLockOnTarget() const { return CurrentLockOnTarget; }

	/**
	 * 通知相机系统锁定状态变化
	 * @param bIsLocking true = 开始锁定, false = 解除锁定
	 * @param Target 锁定目标（解锁时可为 nullptr）
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|LockOn")
	void NotifyCameraLockOnStateChanged(bool bIsLocking, AActor* Target);
	
	/**
	 * 检查目标是否是 Boss
	 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsTargetBoss(AActor* Target) const;

	/**
	 * Fallback lock-on method - doesn't use sphere overlap, searches world for Pawns directly
	 * Used for debugging when sphere collision isn't working
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On|Debug")
	void ToggleLockOnFallback();

	/**
	 * Debug: List all Pawns in the scene
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On|Debug")
	void DebugListAllPawns();

	// ========== State Management ==========

	bool CanPerformAction() const;
	bool CanMove() const;
	void SetCharacterState(ECharacterState NewState);

	// ========== Stamina Management ==========

	void RegenerateStamina(float DeltaTime);
	bool ConsumeStamina(float Amount);

	// ========== Combat Functions ==========

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamage(float DamageAmount, bool bCanBlock);

	void ResetCombo();

	// ========== Timer Handles ==========

	FTimerHandle DodgeTimerHandle;
	FTimerHandle InvincibilityTimerHandle;
	FTimerHandle ComboResetTimerHandle;

private:
	// Movement input values
	float ForwardInput = 0.0f;
	float RightInput = 0.0f;
	bool bIsRunning = false;

	// Lock-on state
	float LastTargetSwitchTime = 0.0f;
	float LastRightStickX = 0.0f;
	const float THUMBSTICK_THRESHOLD = 0.9f;
	
	// Cached lock method and offset
	ELockMethod CachedLockMethod = ELockMethod::None;
	FVector CachedLockOffset = FVector::ZeroVector;
	
	// Legacy - kept for backward compatibility
	AActor* LockedOnTarget = nullptr;
	float LockOnBreakDistance = 2500.0f;
	
	// ========== Camera Reset State ==========
	
	/** 是否正在执行相机回正 */
	bool bIsCameraResetting = false;
	
	/** 相机回正的目标 Yaw */
	float CameraResetTargetYaw = 0.0f;
	
	/** 相机回正插值速度（越大越快） */
	float CameraResetInterpSpeed = 5.0f;
	
	/** 相机回正完成的角度阈值 */
	float CameraResetCompletionThreshold = 1.0f;

	// ========== Auto Realign State (P4-B) ==========
	
	/** 当前是否正在自动回正 */
	bool bIsAutoRealigning = false;
	
	/** 角色在屏幕上的位置（0-1，0.5 为中心） */
	FVector2D CharacterScreenPosition = FVector2D(0.5f, 0.5f);
	
	/** 计算角色到屏幕边缘的距离（用于回正强度） */
	float GetScreenEdgeDistance() const;
	
	/** 计算自动回正的方向（-1 = 左转，1 = 右转） */
	float GetAutoRealignDirection() const;
};
