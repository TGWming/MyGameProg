// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnConfig.h"
#include "CameraControlComponent.generated.h"

// 前置声明
class ACharacter;
class APlayerController;
class USpringArmComponent;
class UCameraComponent;
class USphereComponent;

// 相机状态枚举
UENUM(BlueprintType)
enum class ECameraState : uint8
{
	Normal				UMETA(DisplayName = "Normal"),
	LockedOn			UMETA(DisplayName = "Locked On"),
	SmoothSwitching		UMETA(DisplayName = "Smooth Switching"),
	AutoCorrection		UMETA(DisplayName = "Auto Correction"),
	SmoothReset			UMETA(DisplayName = "Smooth Reset"),
	AdvancedAdjustment	UMETA(DisplayName = "Advanced Adjustment")
};

// 相机切换完成事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraSwitchCompleted, AActor*, NewTarget);

// 相机重置完成事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraResetCompleted);

// 相机修正开始事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraCorrectionStarted, AActor*, Target);

// 高级相机调整事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAdvancedCameraAdjusted, EEnemySizeCategory, TargetSize, float, Distance, FVector, AdjustedLocation);

/**
 * 独立的相机控制组件
 * 负责处理所有相机跟踪、切换、修正和高级调整功能
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UCameraControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraControlComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 事件委托 ====================
	/** 相机切换完成时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Camera Control Events")
	FOnCameraSwitchCompleted OnCameraSwitchCompleted;

	/** 相机重置完成时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Camera Control Events")
	FOnCameraResetCompleted OnCameraResetCompleted;

	/** 相机修正开始时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Camera Control Events")
	FOnCameraCorrectionStarted OnCameraCorrectionStarted;

	/** 高级相机调整时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Camera Control Events")
	FOnAdvancedCameraAdjusted OnAdvancedCameraAdjusted;

	// ==================== 配置参数 ====================
	/** 基础相机设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FCameraSettings CameraSettings;

	/** 高级相机设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Camera Settings")
	FAdvancedCameraSettings AdvancedCameraSettings;

	/** Socket投射设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Projection Settings")
	FSocketProjectionSettings SocketProjectionSettings;

	// ==================== 调试控制 ====================
	/** 是否启用相机调试日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableCameraDebugLogs;

	/** 是否启用高级调整调试日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableAdvancedAdjustmentDebugLogs;

protected:
	// ==================== 内部状态变量 ====================
	/** 当前相机状态 */
	UPROPERTY()
	ECameraState CurrentCameraState;

	/** 当前锁定目标 */
	UPROPERTY()
	AActor* CurrentLockOnTarget;

	/** 上一个锁定目标（用于切换逻辑） */
	UPROPERTY()
	AActor* PreviousLockOnTarget;

	/** 是否应该相机跟随目标 */
	UPROPERTY()
	bool bShouldCameraFollowTarget;

	/** 是否应该角色转向目标 */
	UPROPERTY()
	bool bShouldCharacterRotateToTarget;

	/** 玩家是否在移动 */
	UPROPERTY()
	bool bPlayerIsMoving;

	// ==================== 平滑切换相关变量 ====================
	/** 是否正在进行平滑切换 */
	UPROPERTY()
	bool bIsSmoothSwitching;

	/** 平滑切换开始时间 */
	float SmoothSwitchStartTime;

	/** 平滑切换起始旋转 */
	FRotator SmoothSwitchStartRotation;

	/** 平滑切换目标旋转 */
	FRotator SmoothSwitchTargetRotation;

	/** 相机是否需要平滑切换 */
	bool bShouldSmoothSwitchCamera;

	/** 角色是否需要平滑切换 */
	bool bShouldSmoothSwitchCharacter;

	// ==================== 自动修正相关变量 ====================
	/** 是否正在进行相机自动修正 */
	UPROPERTY()
	bool bIsCameraAutoCorrection;

	/** 修正开始时间 */
	float CameraCorrectionStartTime;

	/** 修正起始相机旋转 */
	FRotator CameraCorrectionStartRotation;

	/** 修正目标相机旋转 */
	FRotator CameraCorrectionTargetRotation;

	/** 延迟修正的目标引用 */
	UPROPERTY()
	AActor* DelayedCorrectionTarget;

	// ==================== 相机重置相关变量 ====================
	/** 是否正在进行平滑相机重置 */
	bool bIsSmoothCameraReset;

	/** 平滑重置开始时间 */
	float SmoothResetStartTime;

	/** 平滑重置起始相机旋转 */
	FRotator SmoothResetStartRotation;

	/** 平滑重置目标相机旋转 */
	FRotator SmoothResetTargetRotation;

	// ==================== 高级距离自适应相关变量 ====================
	/** 是否正在进行高级相机调整 */
	bool bIsAdvancedCameraAdjustment;

	/** 上次高级调整时间 */
	float LastAdvancedAdjustmentTime;

	/** 当前距离分类 */
	EEnemySizeCategory CurrentTargetSizeCategory;

	/** 当前目标距离 */
	float CurrentTargetDistance;

	/** 高级调整更新间隔 */
	static constexpr float ADVANCED_ADJUSTMENT_INTERVAL = 0.1f;

	// ==================== 性能优化常量 ====================
	/** 目标切换角度阈值 */
	static constexpr float TARGET_SWITCH_ANGLE_THRESHOLD = 20.0f;

	/** 平滑切换速度 */
	static constexpr float TARGET_SWITCH_SMOOTH_SPEED = 3.0f;

	/** 锁定完成阈值 */
	static constexpr float LOCK_COMPLETION_THRESHOLD = 5.0f;

	/** 相机自动修正速度 */
	static constexpr float CAMERA_AUTO_CORRECTION_SPEED = 7.0f;

	/** 相机修正偏移角度 */
	static constexpr float CAMERA_CORRECTION_OFFSET = 150.0f;

	/** 相机重置速度 */
	static constexpr float CAMERA_RESET_SPEED = 7.0f;

	/** 相机重置角度阈值 */
	static constexpr float CAMERA_RESET_ANGLE_THRESHOLD = 1.0f;

	/** 角色旋转速度 */
	static constexpr float CHARACTER_ROTATION_SPEED = 10.0f;

public:
	// ==================== 主要接口函数 ====================
	
	/** 设置相机设置 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetCameraSettings(const FCameraSettings& Settings);

	/** 设置高级相机设置 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetAdvancedCameraSettings(const FAdvancedCameraSettings& Settings);

	/** 获取当前相机状态 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	ECameraState GetCurrentCameraState() const { return CurrentCameraState; }

	/** 获取当前锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	AActor* GetCurrentLockOnTarget() const { return CurrentLockOnTarget; }

	// ==================== 输入处理接口 ====================
	
	/** 处理玩家相机输入 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void HandlePlayerInput(float TurnInput, float LookUpInput);

	/** 处理玩家移动状态 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void HandlePlayerMovement(bool bIsMoving);

	// ==================== 从MyCharacter迁移的核心相机函数 ====================
	
	/** 更新锁定相机 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void UpdateLockOnCamera();

	/** 更新角色朝向目标的旋转 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void UpdateCharacterRotationToTarget();

	/** 开始平滑目标切换 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void StartSmoothTargetSwitch(AActor* NewTarget);

	/** 更新平滑目标切换 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void UpdateSmoothTargetSwitch();

	/** 开始平滑相机重置 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void StartSmoothCameraReset();

	/** 更新平滑相机重置 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void UpdateSmoothCameraReset();

	/** 开始相机重置 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void StartCameraReset(const FRotator& TargetRotation);

	/** 执行简单的相机重置 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void PerformSimpleCameraReset();

	/** 开始相机自动修正 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void StartCameraAutoCorrection(AActor* Target);

	/** 更新相机自动修正 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void UpdateCameraAutoCorrection();

	/** 开始针对特定目标的相机修正 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void StartCameraCorrectionForTarget(AActor* Target);

	/** 延迟相机修正 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void DelayedCameraCorrection();

	// ==================== 新增的高级距离自适应功能 ====================
	
	/** 更新高级相机调整 */
	UFUNCTION(BlueprintCallable, Category = "Advanced Camera")
	void UpdateAdvancedCameraAdjustment();

	/** 计算高级目标位置 */
	UFUNCTION(BlueprintCallable, Category = "Advanced Camera")
	FVector CalculateAdvancedTargetLocation(AActor* Target, EEnemySizeCategory SizeCategory, float Distance);

	/** 检查是否应该使用直接切换 */
	UFUNCTION(BlueprintCallable, Category = "Advanced Camera")
	bool ShouldUseDirectSwitching(AActor* Target, float AngleDifference, float Distance);

	/** 执行直接目标切换 */
	UFUNCTION(BlueprintCallable, Category = "Advanced Camera")
	void PerformDirectTargetSwitch(AActor* NewTarget);

	// ==================== 状态管理函数 ====================
	
	/** 设置锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetLockOnTarget(AActor* Target);

	/** 清除锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ClearLockOnTarget();

	/** 设置相机跟随状态 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetCameraFollowState(bool bShouldFollow, bool bShouldRotateCharacter);

	/** 恢复相机跟随状态 */
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void RestoreCameraFollowState();

protected:
	// ==================== 内部辅助函数 ====================
	
	/** 获取拥有者角色 */
	ACharacter* GetOwnerCharacter() const;

	/** 获取拥有者的控制器 */
	APlayerController* GetOwnerController() const;

	/** 获取弹簧臂组件 */
	USpringArmComponent* GetSpringArmComponent() const;

	/** 获取相机组件 */
	UCameraComponent* GetCameraComponent() const;

	/** 计算到目标的角度差异 */
	float CalculateAngleToTarget(AActor* Target) const;

	/** 计算目标相对于玩家的方向角度 */
	float CalculateDirectionAngle(AActor* Target) const;

	/** 获取目标的Socket世界位置 */
	FVector GetTargetSocketWorldLocation(AActor* Target) const;

	/** 检查目标是否有有效的Socket */
	bool HasValidSocket(AActor* Target) const;

	/** 获取目标的尺寸分类 */
	EEnemySizeCategory GetTargetSizeCategory(AActor* Target) const;

	/** 计算到目标的距离 */
	float CalculateDistanceToTarget(AActor* Target) const;

	/** 根据距离获取相机速度倍数 */
	float GetCameraSpeedMultiplierForDistance(float Distance) const;

	/** 根据敌人尺寸获取高度偏移 */
	FVector GetHeightOffsetForEnemySize(EEnemySizeCategory SizeCategory) const;

	/** 更新相机状态 */
	void UpdateCameraState(ECameraState NewState);

	/** 检查玩家输入是否中断自动控制 */
	bool ShouldInterruptAutoControl(float TurnInput, float LookUpInput) const;

private:
	// ==================== 私有辅助函数 ====================
	
	/** 内部函数：执行相机插值 */
	void PerformCameraInterpolation(const FRotator& TargetRotation, float InterpSpeed);

	/** 内部函数：执行角色旋转插值 */
	void PerformCharacterRotationInterpolation(const FRotator& TargetRotation, float InterpSpeed);

	/** 内部函数：检查插值是否完成 */
	bool IsInterpolationComplete(const FRotator& Current, const FRotator& Target, float Threshold = 2.0f) const;

	/** 内部函数：标准化角度差异 */
	float NormalizeAngleDifference(float AngleDiff) const;

	/** 内部函数：计算距离权重 */
	float CalculateDistanceWeight(float Distance, float MaxDistance) const;

	/** 内部函数：应用地形高度补偿 */
	FVector ApplyTerrainHeightCompensation(const FVector& BaseLocation, AActor* Target) const;

	/** 内部函数：验证目标有效性 */
	bool ValidateTarget(AActor* Target) const;
};