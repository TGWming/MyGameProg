// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "PoiseComponent.generated.h"

// 韧性状态枚举
UENUM(BlueprintType)
enum class EPoiseState : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	Damaged		UMETA(DisplayName = "Damaged"),
	Broken		UMETA(DisplayName = "Broken"),
	Recovering	UMETA(DisplayName = "Recovering"),
	Immune		UMETA(DisplayName = "Immune")
};

// 韧性设置结构体
USTRUCT(BlueprintType)
struct FPoiseSettings
{
	GENERATED_BODY()

	// 最大韧性值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise Settings", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float MaxPoise = 100.0f;

	// 韧性恢复速率（每秒恢复的韧性值）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise Settings", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float PoiseRecoveryRate = 10.0f;

	// 韧性恢复延迟（受到伤害后多久开始恢复）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float PoiseRecoveryDelay = 3.0f;

	// 基础硬直持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float BaseStaggerDuration = 1.5f;

	// 韧性破坏后的免疫时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise Settings", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float PoiseImmuneTimeAfterBreak = 0.5f;

	// 构造函数
	FPoiseSettings()
		: MaxPoise(100.0f)
		, PoiseRecoveryRate(10.0f)
		, PoiseRecoveryDelay(3.0f)
		, BaseStaggerDuration(1.5f)
		, PoiseImmuneTimeAfterBreak(0.5f)
	{
	}
};

// 事件委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPoiseBreak, AActor*, OwnerActor, float, StaggerDuration, AActor*, DamageSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPoiseChanged, AActor*, OwnerActor, float, CurrentPoise, float, MaxPoise);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaggerStarted, AActor*, OwnerActor, float, StaggerDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaggerEnded, AActor*, OwnerActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoiseStateChanged, AActor*, OwnerActor, EPoiseState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UPoiseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPoiseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 韧性设置 ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise Settings")
	FPoiseSettings PoiseSettings;

	// ==================== 事件委托 ====================
	UPROPERTY(BlueprintAssignable, Category = "Poise Events")
	FOnPoiseBreak OnPoiseBreak;

	UPROPERTY(BlueprintAssignable, Category = "Poise Events")
	FOnPoiseChanged OnPoiseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Poise Events")
	FOnStaggerStarted OnStaggerStarted;

	UPROPERTY(BlueprintAssignable, Category = "Poise Events")
	FOnStaggerEnded OnStaggerEnded;

	UPROPERTY(BlueprintAssignable, Category = "Poise Events")
	FOnPoiseStateChanged OnPoiseStateChanged;

	// ==================== 核心接口函数 ====================
	
	/**
	 * 对韧性造成伤害
	 * @param PoiseDamage 韧性伤害值
	 * @param DamageSource 伤害来源（可选）
	 * @return 是否成功造成韧性伤害
	 */
	UFUNCTION(BlueprintCallable, Category = "Poise System")
	bool TakePoiseDamage(float PoiseDamage, AActor* DamageSource = nullptr);

	/**
	 * 强制破坏韧性
	 * @param DamageSource 伤害来源（可选）
	 */
	UFUNCTION(BlueprintCallable, Category = "Poise System")
	void BreakPoise(AActor* DamageSource = nullptr);

	/**
	 * 恢复韧性
	 * @param RecoveryAmount 恢复量（如果为0则使用默认恢复速率）
	 */
	UFUNCTION(BlueprintCallable, Category = "Poise System")
	void RecoverPoise(float RecoveryAmount = 0.0f);

	/**
	 * 重置韧性到最大值
	 */
	UFUNCTION(BlueprintCallable, Category = "Poise System")
	void ResetPoise();

	/**
	 * 设置韧性免疫状态
	 * @param bImmune 是否免疫
	 * @param Duration 免疫持续时间（0表示永久免疫直到手动取消）
	 */
	UFUNCTION(BlueprintCallable, Category = "Poise System")
	void SetPoiseImmune(bool bImmune, float Duration = 0.0f);

	// ==================== 状态查询函数 ====================

	/**
	 * 检查韧性是否已破坏
	 */
	UFUNCTION(BlueprintPure, Category = "Poise System")
	bool IsPoiseBroken() const;

	/**
	 * 检查是否处于韧性免疫状态
	 */
	UFUNCTION(BlueprintPure, Category = "Poise System")
	bool IsPoiseImmune() const;

	/**
	 * 获取当前韧性值
	 */
	UFUNCTION(BlueprintPure, Category = "Poise System")
	float GetCurrentPoise() const;

	/**
	 * 获取韧性百分比
	 */
	UFUNCTION(BlueprintPure, Category = "Poise System")
	float GetPoisePercentage() const;

	/**
	 * 获取当前韧性状态
	 */
	UFUNCTION(BlueprintPure, Category = "Poise System")
	EPoiseState GetPoiseState() const;

	/**
	 * 获取最大韧性值
	 */
	UFUNCTION(BlueprintPure, Category = "Poise System")
	float GetMaxPoise() const;

	/**
	 * 检查是否正在硬直中
	 */
	UFUNCTION(BlueprintPure, Category = "Poise System")
	bool IsStaggering() const;

	// ==================== 配置函数 ====================

	/**
	 * 更新韧性设置
	 */
	UFUNCTION(BlueprintCallable, Category = "Poise System")
	void UpdatePoiseSettings(const FPoiseSettings& NewSettings);

	/**
	 * 设置最大韧性值
	 */
	UFUNCTION(BlueprintCallable, Category = "Poise System")
	void SetMaxPoise(float NewMaxPoise);

protected:
	// ==================== Protected成员变量 ====================

	// 当前韧性值
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Poise State")
	float CurrentPoise;

	// 当前韧性状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Poise State")
	EPoiseState CurrentPoiseState;

	// 最后受到伤害的时间
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Poise State")
	float LastDamageTime;

	// 韧性免疫结束时间
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Poise State")
	float PoiseImmuneEndTime;

	// 拥有者角色引用
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Poise State")
	ACharacter* OwnerCharacter;

	// 硬直定时器句柄
	FTimerHandle StaggerTimerHandle;

	// 免疫定时器句柄
	FTimerHandle ImmuneTimerHandle;

	// 是否正在硬直中
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Poise State")
	bool bIsStaggering;

	// 是否手动设置免疫（区别于破坏后的自动免疫）
	bool bManuallySetImmune;

	// ==================== 私有辅助函数 ====================

private:
	/**
	 * 更新韧性恢复逻辑
	 */
	void UpdatePoiseRecovery(float DeltaTime);

	/**
	 * 开始硬直
	 */
	void StartStagger(float StaggerDuration, AActor* DamageSource = nullptr);

	/**
	 * 结束硬直
	 */
	void EndStagger();

	/**
	 * 设置韧性状态
	 */
	void SetPoiseState(EPoiseState NewState);

	/**
	 * 结束韧性免疫
	 */
	void EndPoiseImmune();

	/**
	 * 计算硬直持续时间
	 */
	float CalculateStaggerDuration(float PoiseDamage) const;

	/**
	 * 广播韧性变化事件
	 */
	void BroadcastPoiseChanged();

	/**
	 * 验证组件状态
	 */
	bool IsValidForPoiseOperations() const;
};