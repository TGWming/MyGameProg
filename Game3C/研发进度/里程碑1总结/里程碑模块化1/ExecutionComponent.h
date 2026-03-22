// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "ExecutionComponent.generated.h"

// 前置声明
class AActor;
class UAnimMontage;

/** 处决类型枚举 */
UENUM(BlueprintType)
enum class EExecutionType : uint8
{
	None		UMETA(DisplayName = "None"),
	Backstab	UMETA(DisplayName = "Backstab"),		// 背刺
	Riposte		UMETA(DisplayName = "Riposte"),			// 弹反
	Plunge		UMETA(DisplayName = "Plunge"),			// 坠落攻击
	Critical	UMETA(DisplayName = "Critical")			// 暴击处决
};

/** 处决设置结构体 */
USTRUCT(BlueprintType)
struct FExecutionSettings
{
	GENERATED_BODY()

	/** 背刺检测范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab", meta = (ClampMin = "50.0", ClampMax = "300.0"))
	float BackstabRange = 150.0f;

	/** 背刺角度阈值（度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab", meta = (ClampMin = "15.0", ClampMax = "90.0"))
	float BackstabAngle = 45.0f;

	/** 弹反时间窗口（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float RiposteWindow = 0.5f;

	/** 坠落攻击最小高度要求 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plunge", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float PlungeHeightRequirement = 200.0f;

	/** 暴击伤害倍数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical", meta = (ClampMin = "1.5", ClampMax = "10.0"))
	float CriticalDamageMultiplier = 3.0f;

	/** 背刺动画蒙太奇 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* BackstabMontage = nullptr;

	/** 弹反动画蒙太奇 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* RiposteMontage = nullptr;

	/** 坠落攻击动画蒙太奇 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* PlungeMontage = nullptr;

	FExecutionSettings()
	{
		BackstabRange = 150.0f;
		BackstabAngle = 45.0f;
		RiposteWindow = 0.5f;
		PlungeHeightRequirement = 200.0f;
		CriticalDamageMultiplier = 3.0f;
		BackstabMontage = nullptr;
		RiposteMontage = nullptr;
		PlungeMontage = nullptr;
	}
};

/** 处决开始事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecutionStarted, AActor*, Target, EExecutionType, ExecutionType);

/** 处决完成事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecutionCompleted, AActor*, Target, EExecutionType, ExecutionType);

/** 背刺机会事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackstabOpportunity, AActor*, Target);

/**
 * 背刺处决组件
 * 负责处理背刺、弹反、坠落攻击等处决系统功能
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UExecutionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// ==================== 常量定义 ====================
	/** 背刺定位插值速度 */
	static constexpr float BACKSTAB_POSITION_INTERP_SPEED = 8.0f;
	
	/** 处决位置调整精度阈值 */
	static constexpr float EXECUTION_POSITION_THRESHOLD = 5.0f;
	
	/** 弹反检测频率（秒） */
	static constexpr float RIPOSTE_CHECK_INTERVAL = 0.05f;
	
	/** 坠落检测高度采样频率（秒） */
	static constexpr float PLUNGE_HEIGHT_CHECK_INTERVAL = 0.1f;
	
	/** 处决状态更新频率（秒） */
	static constexpr float EXECUTION_UPDATE_INTERVAL = 0.02f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 处决设置 ====================
	/** 处决系统设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution Settings")
	FExecutionSettings ExecutionSettings;

	// ==================== 事件委托 ====================
	/** 处决开始事件 */
	UPROPERTY(BlueprintAssignable, Category = "Execution Events")
	FOnExecutionStarted OnExecutionStarted;

	/** 处决完成事件 */
	UPROPERTY(BlueprintAssignable, Category = "Execution Events")
	FOnExecutionCompleted OnExecutionCompleted;

	/** 背刺机会事件 */
	UPROPERTY(BlueprintAssignable, Category = "Execution Events")
	FOnBackstabOpportunity OnBackstabOpportunity;

	// ==================== 核心接口函数 ====================
	/** 检查是否可以执行指定类型的处决 */
	UFUNCTION(BlueprintCallable, Category = "Execution")
	bool CanExecute(AActor* Target, EExecutionType ExecutionType) const;

	/** 开始执行处决 */
	UFUNCTION(BlueprintCallable, Category = "Execution")
	bool StartExecution(AActor* Target, EExecutionType ExecutionType);

	/** 完成处决 */
	UFUNCTION(BlueprintCallable, Category = "Execution")
	void CompleteExecution();

	/** 取消处决 */
	UFUNCTION(BlueprintCallable, Category = "Execution")
	void CancelExecution();

	// ==================== 背刺系统函数 ====================
	/** 检查背刺机会 */
	UFUNCTION(BlueprintCallable, Category = "Backstab")
	bool CheckBackstabOpportunity(AActor* Target) const;

	/** 检查目标是否容易受到背刺攻击 */
	UFUNCTION(BlueprintCallable, Category = "Backstab")
	bool IsTargetVulnerableToBackstab(AActor* Target) const;

	/** 获取最佳背刺位置 */
	UFUNCTION(BlueprintCallable, Category = "Backstab")
	FVector GetOptimalBackstabPosition(AActor* Target) const;

	// ==================== 弹反系统函数 ====================
	/** 检查是否可以弹反 */
	UFUNCTION(BlueprintCallable, Category = "Riposte")
	bool CanRiposte(AActor* Target) const;

	/** 开始弹反时间窗口 */
	UFUNCTION(BlueprintCallable, Category = "Riposte")
	void StartRiposteWindow(float Duration = -1.0f);

	/** 结束弹反时间窗口 */
	UFUNCTION(BlueprintCallable, Category = "Riposte")
	void EndRiposteWindow();

	// ==================== 坠落攻击函数 ====================
	/** 检查是否可以进行坠落攻击 */
	UFUNCTION(BlueprintCallable, Category = "Plunge")
	bool CanPlungeAttack() const;

	/** 获取坠落攻击目标列表 */
	UFUNCTION(BlueprintCallable, Category = "Plunge")
	TArray<AActor*> GetPlungeTargets() const;

	// ==================== 状态查询函数 ====================
	/** 检查是否正在执行处决 */
	UFUNCTION(BlueprintCallable, Category = "Execution")
	bool IsExecuting() const { return CurrentExecutionType != EExecutionType::None; }

	/** 获取当前处决类型 */
	UFUNCTION(BlueprintCallable, Category = "Execution")
	EExecutionType GetCurrentExecutionType() const { return CurrentExecutionType; }

	/** 获取当前处决目标 */
	UFUNCTION(BlueprintCallable, Category = "Execution")
	AActor* GetExecutionTarget() const { return CurrentExecutionTarget; }

protected:
	// ==================== 内部状态变量 ====================
	/** 当前处决类型 */
	UPROPERTY(BlueprintReadOnly, Category = "Execution State")
	EExecutionType CurrentExecutionType = EExecutionType::None;

	/** 当前处决目标 */
	UPROPERTY(BlueprintReadOnly, Category = "Execution State")
	AActor* CurrentExecutionTarget = nullptr;

	/** 弹反窗口是否激活 */
	UPROPERTY(BlueprintReadOnly, Category = "Riposte State")
	bool bRiposteWindowActive = false;

	/** 弹反窗口定时器句柄 */
	FTimerHandle RiposteWindowTimerHandle;

	/** 处决位置调整状态 */
	bool bIsPositioningForExecution = false;

	/** 处决开始时间 */
	float ExecutionStartTime = 0.0f;

	/** 处决动画播放状态 */
	bool bExecutionAnimationPlaying = false;

	/** 角色起始位置（用于位置调整） */
	FVector ExecutionStartPosition = FVector::ZeroVector;

	/** 目标处决位置 */
	FVector TargetExecutionPosition = FVector::ZeroVector;

	// ==================== 私有辅助函数 ====================
	/** 验证处决目标的有效性 */
	bool ValidateExecutionTarget(AActor* Target) const;

	/** 将角色定位到处决位置 */
	void PositionCharacterForExecution(AActor* Target, EExecutionType ExecutionType);

	/** 计算处决伤害 */
	float CalculateExecutionDamage(EExecutionType ExecutionType) const;

	/** 检查角色是否在空中（用于坠落攻击） */
	bool IsCharacterInAir() const;

	/** 获取角色当前高度 */
	float GetCharacterHeight() const;

	/** 获取角色到地面的距离 */
	float GetDistanceToGround() const;

	/** 计算两个Actor之间的角度差 */
	float CalculateAngleBetweenActors(AActor* Actor1, AActor* Actor2) const;

	/** 获取角色的骨骼网格组件 */
	USkeletalMeshComponent* GetCharacterMesh() const;

	/** 播放处决动画 */
	bool PlayExecutionAnimation(EExecutionType ExecutionType);

	/** 停止当前处决动画 */
	void StopExecutionAnimation();

	/** 处决动画完成回调 */
	UFUNCTION()
	void OnExecutionAnimationCompleted(UAnimMontage* Montage, bool bInterrupted);

	/** 弹反窗口定时器回调 */
	UFUNCTION()
	void OnRiposteWindowExpired();

	/** 更新处决状态 */
	void UpdateExecutionState(float DeltaTime);

	/** 更新角色位置调整 */
	void UpdatePositioning(float DeltaTime);

	/** 应用处决伤害 */
	void ApplyExecutionDamage(AActor* Target, EExecutionType ExecutionType);

	/** 重置处决状态 */
	void ResetExecutionState();

	/** 获取目标背后的最佳位置 */
	FVector GetBackstabPositionBehindTarget(AActor* Target) const;

	/** 检查背刺角度是否合适 */
	bool IsBackstabAngleValid(AActor* Target) const;

	/** 检查背刺距离是否合适 */
	bool IsBackstabDistanceValid(AActor* Target) const;
};