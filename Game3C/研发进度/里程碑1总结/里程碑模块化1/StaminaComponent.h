// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "StaminaComponent.generated.h"

// 精力状态枚举
UENUM(BlueprintType)
enum class EStaminaState : uint8
{
	Normal		UMETA(DisplayName = "Normal"),		// 正常状态
	Depleted	UMETA(DisplayName = "Depleted"),	// 精力耗尽
	Recovering	UMETA(DisplayName = "Recovering"),	// 恢复中
	Exhausted	UMETA(DisplayName = "Exhausted")	// 过度疲劳状态
};

// 精力消耗动作枚举
UENUM(BlueprintType)
enum class EStaminaAction : uint8
{
	Attack			UMETA(DisplayName = "Attack"),			// 普通攻击
	HeavyAttack		UMETA(DisplayName = "Heavy Attack"),	// 重攻击
	Dodge			UMETA(DisplayName = "Dodge"),			// 闪避
	Block			UMETA(DisplayName = "Block"),			// 格挡
	Sprint			UMETA(DisplayName = "Sprint"),			// 冲刺
	WeaponSkill		UMETA(DisplayName = "Weapon Skill")		// 武器技能
};

// 精力设置结构体
USTRUCT(BlueprintType)
struct FStaminaSettings
{
	GENERATED_BODY()

	// 最大精力值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Settings", meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float MaxStamina = 100.0f;

	// 精力恢复速度（每秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Settings", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float StaminaRecoveryRate = 15.0f;

	// 精力恢复延迟（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Settings", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float StaminaRecoveryDelay = 1.0f;

	// 过度疲劳时的恢复惩罚（倍数）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Settings", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ExhaustedRecoveryPenalty = 0.5f;

	// 普通攻击精力消耗
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Costs", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float AttackStaminaCost = 20.0f;

	// 重攻击精力消耗
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Costs", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float HeavyAttackStaminaCost = 35.0f;

	// 闪避精力消耗
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Costs", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float DodgeStaminaCost = 25.0f;

	// 格挡精力消耗
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Costs", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float BlockStaminaCost = 5.0f;

	// 冲刺每秒精力消耗
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Costs", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float SprintStaminaCostPerSecond = 10.0f;

	// 武器技能精力消耗
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Costs", meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float WeaponSkillStaminaCost = 50.0f;

	// 构造函数
	FStaminaSettings()
	{
		// 使用默认值
	}
};

// 精力变化事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);

// 精力耗尽事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepleted);

// 精力完全恢复事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaFullyRecovered);

// 精力状态变化事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaStateChanged, EStaminaState, OldState, EStaminaState, NewState);

/**
 * 精力管理组件
 * 
 * 该组件负责管理角色的精力系统，包括：
 * - 精力消耗和恢复
 * - 不同动作的精力成本计算
 * - 精力状态管理（正常、耗尽、恢复中、过度疲劳）
 * - 精力相关事件的触发
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// 构造函数
	UStaminaComponent();

protected:
	// 游戏开始时调用
	virtual void BeginPlay() override;

public:	
	// 每帧调用
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 核心精力管理接口 ====================
	
	/**
	 * 消耗指定数量的精力
	 * @param Amount 要消耗的精力值
	 * @return 是否成功消耗（精力充足时返回true）
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool ConsumeStamina(float Amount);

	/**
	 * 为特定动作消耗精力
	 * @param Action 要执行的动作类型
	 * @return 是否成功消耗精力
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool ConsumeStaminaForAction(EStaminaAction Action);

	/**
	 * 检查是否能够执行指定动作（是否有足够精力）
	 * @param Action 要检查的动作类型
	 * @return 是否有足够精力执行该动作
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	bool CanPerformAction(EStaminaAction Action) const;

	/**
	 * 恢复指定数量的精力
	 * @param Amount 要恢复的精力值
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void RecoverStamina(float Amount);

	/**
	 * 重置精力到最大值
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ResetStamina();

	/**
	 * 设置精力恢复是否启用
	 * @param bEnabled 是否启用自动恢复
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetStaminaRecoveryEnabled(bool bEnabled);

	// ==================== 状态查询接口 ====================

	/**
	 * 获取当前精力值
	 * @return 当前精力值
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	/**
	 * 获取精力百分比（0.0 - 1.0）
	 * @return 精力百分比
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	float GetStaminaPercentage() const;

	/**
	 * 检查精力是否已耗尽
	 * @return 精力是否已耗尽
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	bool IsStaminaDepleted() const { return CurrentStaminaState == EStaminaState::Depleted; }

	/**
	 * 获取当前精力状态
	 * @return 当前精力状态
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	EStaminaState GetStaminaState() const { return CurrentStaminaState; }

	/**
	 * 获取最大精力值
	 * @return 最大精力值
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	float GetMaxStamina() const { return StaminaSettings.MaxStamina; }

	/**
	 * 检查精力是否已满
	 * @return 精力是否已满
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	bool IsStaminaFull() const;

	// ==================== 设置接口 ====================

	/**
	 * 设置精力设置
	 * @param NewSettings 新的精力设置
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetStaminaSettings(const FStaminaSettings& NewSettings);

	/**
	 * 获取精力设置
	 * @return 当前精力设置
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	FStaminaSettings GetStaminaSettings() const { return StaminaSettings; }

	/**
	 * 设置最大精力值（会调整当前精力比例）
	 * @param NewMaxStamina 新的最大精力值
	 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetMaxStamina(float NewMaxStamina);

	// ==================== 事件委托 ====================

	// 精力变化事件
	UPROPERTY(BlueprintAssignable, Category = "Stamina Events")
	FOnStaminaChanged OnStaminaChanged;

	// 精力耗尽事件
	UPROPERTY(BlueprintAssignable, Category = "Stamina Events")
	FOnStaminaDepleted OnStaminaDepleted;

	// 精力完全恢复事件
	UPROPERTY(BlueprintAssignable, Category = "Stamina Events")
	FOnStaminaFullyRecovered OnStaminaFullyRecovered;

	// 精力状态变化事件
	UPROPERTY(BlueprintAssignable, Category = "Stamina Events")
	FOnStaminaStateChanged OnStaminaStateChanged;

protected:
	// ==================== 配置属性 ====================

	// 精力设置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Settings")
	FStaminaSettings StaminaSettings;

	// ==================== 运行时状态 ====================

	// 当前精力值
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina State")
	float CurrentStamina;

	// 当前精力状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina State")
	EStaminaState CurrentStaminaState;

	// 精力恢复是否启用
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina State")
	bool bStaminaRecoveryEnabled;

	// 最后一次使用精力的时间
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina State")
	float LastStaminaUseTime;

	// 精力恢复开始时间
	float StaminaRecoveryStartTime;

	// 是否正在恢复精力
	bool bIsRecoveringStamina;

	// 过度疲劳计数器
	int32 ExhaustedCounter;

private:
	// ==================== 私有辅助函数 ====================

	/**
	 * 更新精力恢复逻辑
	 * @param DeltaTime 时间增量
	 */
	void UpdateStaminaRecovery(float DeltaTime);

	/**
	 * 获取指定动作的精力消耗
	 * @param Action 动作类型
	 * @return 精力消耗值
	 */
	float GetActionStaminaCost(EStaminaAction Action) const;

	/**
	 * 更新精力状态
	 * @param NewState 新状态
	 */
	void UpdateStaminaState(EStaminaState NewState);

	/**
	 * 触发精力变化事件
	 */
	void TriggerStaminaChangedEvent();

	/**
	 * 检查并处理精力耗尽状态
	 */
	void CheckStaminaDepletion();

	/**
	 * 检查并处理精力完全恢复
	 */
	void CheckStaminaFullRecovery();

	/**
	 * 获取当前恢复速度（考虑过度疲劳惩罚）
	 * @return 当前恢复速度
	 */
	float GetCurrentRecoveryRate() const;

	/**
	 * 验证精力值范围
	 */
	void ClampStaminaValue();
};