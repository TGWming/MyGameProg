// Fill out your copyright notice in the Description page of Project Settings.

#include "StaminaComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

// 构造函数
UStaminaComponent::UStaminaComponent()
{
	// 设置每帧更新
	PrimaryComponentTick.bCanEverTick = true;

	// 初始化精力设置为默认值
	StaminaSettings = FStaminaSettings();

	// 初始化运行时状态
	CurrentStamina = StaminaSettings.MaxStamina;
	CurrentStaminaState = EStaminaState::Normal;
	bStaminaRecoveryEnabled = true;
	LastStaminaUseTime = 0.0f;
	StaminaRecoveryStartTime = 0.0f;
	bIsRecoveringStamina = false;
	ExhaustedCounter = 0;

	UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Component created with MaxStamina=%.1f"), StaminaSettings.MaxStamina);
}

// 游戏开始时调用
void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	// 确保精力值在有效范围内
	CurrentStamina = StaminaSettings.MaxStamina;
	ClampStaminaValue();

	// 重置时间记录
	if (UWorld* World = GetWorld())
	{
		LastStaminaUseTime = World->GetTimeSeconds();
		StaminaRecoveryStartTime = LastStaminaUseTime;
	}

	// 触发初始事件
	TriggerStaminaChangedEvent();

	UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: BeginPlay completed. Current stamina: %.1f/%.1f"), 
		CurrentStamina, StaminaSettings.MaxStamina);
}

// 每帧更新
void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新精力恢复逻辑
	if (bStaminaRecoveryEnabled && CurrentStamina < StaminaSettings.MaxStamina)
	{
		UpdateStaminaRecovery(DeltaTime);
	}
}

// ==================== 核心精力管理接口实现 ====================

bool UStaminaComponent::ConsumeStamina(float Amount)
{
	// 检查输入有效性
	if (Amount <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Invalid stamina amount: %.1f"), Amount);
		return false;
	}

	// 检查是否有足够精力
	if (CurrentStamina < Amount)
	{
		UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Insufficient stamina. Required: %.1f, Available: %.1f"), 
			Amount, CurrentStamina);
		return false;
	}

	// 消耗精力
	float OldStamina = CurrentStamina;
	CurrentStamina -= Amount;
	ClampStaminaValue();

	// 更新时间记录
	if (UWorld* World = GetWorld())
	{
		LastStaminaUseTime = World->GetTimeSeconds();
		bIsRecoveringStamina = false; // 停止恢复状态
	}

	// 检查精力耗尽
	CheckStaminaDepletion();

	// 触发事件
	TriggerStaminaChangedEvent();

	UE_LOG(LogTemp, Verbose, TEXT("StaminaComponent: Consumed %.1f stamina. %.1f -> %.1f"), 
		Amount, OldStamina, CurrentStamina);

	return true;
}

bool UStaminaComponent::ConsumeStaminaForAction(EStaminaAction Action)
{
	float ActionCost = GetActionStaminaCost(Action);
	
	if (ActionCost <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Invalid action cost for action: %d"), (int32)Action);
		return false;
	}

	bool bSuccess = ConsumeStamina(ActionCost);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Action executed. Type: %d, Cost: %.1f"), 
			(int32)Action, ActionCost);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Action failed due to insufficient stamina. Type: %d, Cost: %.1f, Available: %.1f"), 
			(int32)Action, ActionCost, CurrentStamina);
	}

	return bSuccess;
}

bool UStaminaComponent::CanPerformAction(EStaminaAction Action) const
{
	float ActionCost = GetActionStaminaCost(Action);
	bool bCanPerform = (CurrentStamina >= ActionCost) && (ActionCost > 0.0f);
	
	return bCanPerform;
}

void UStaminaComponent::RecoverStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	float OldStamina = CurrentStamina;
	CurrentStamina += Amount;
	ClampStaminaValue();

	// 检查完全恢复
	CheckStaminaFullRecovery();

	// 更新状态
	if (CurrentStamina > 0.0f && CurrentStaminaState == EStaminaState::Depleted)
	{
		UpdateStaminaState(EStaminaState::Recovering);
	}

	// 触发事件
	TriggerStaminaChangedEvent();

	UE_LOG(LogTemp, Verbose, TEXT("StaminaComponent: Recovered %.1f stamina. %.1f -> %.1f"), 
		Amount, OldStamina, CurrentStamina);
}

void UStaminaComponent::ResetStamina()
{
	float OldStamina = CurrentStamina;
	CurrentStamina = StaminaSettings.MaxStamina;
	
	// 重置状态
	UpdateStaminaState(EStaminaState::Normal);
	ExhaustedCounter = 0;
	bIsRecoveringStamina = false;

	// 触发事件
	TriggerStaminaChangedEvent();
	OnStaminaFullyRecovered.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Stamina reset. %.1f -> %.1f"), OldStamina, CurrentStamina);
}

void UStaminaComponent::SetStaminaRecoveryEnabled(bool bEnabled)
{
	bool bOldValue = bStaminaRecoveryEnabled;
	bStaminaRecoveryEnabled = bEnabled;

	if (bEnabled && !bOldValue)
	{
		// 恢复刚被启用，重置恢复时间
		if (UWorld* World = GetWorld())
		{
			StaminaRecoveryStartTime = World->GetTimeSeconds();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Stamina recovery %s"), 
		bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

// ==================== 状态查询接口实现 ====================

float UStaminaComponent::GetStaminaPercentage() const
{
	if (StaminaSettings.MaxStamina <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentStamina / StaminaSettings.MaxStamina, 0.0f, 1.0f);
}

bool UStaminaComponent::IsStaminaFull() const
{
	return FMath::IsNearlyEqual(CurrentStamina, StaminaSettings.MaxStamina, 0.01f);
}

// ==================== 设置接口实现 ====================

void UStaminaComponent::SetStaminaSettings(const FStaminaSettings& NewSettings)
{
	// 保存当前精力百分比
	float CurrentPercentage = GetStaminaPercentage();

	// 更新设置
	StaminaSettings = NewSettings;

	// 根据新的最大值调整当前精力
	CurrentStamina = StaminaSettings.MaxStamina * CurrentPercentage;
	ClampStaminaValue();

	// 触发事件
	TriggerStaminaChangedEvent();

	UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Settings updated. New MaxStamina: %.1f, Current: %.1f"), 
		StaminaSettings.MaxStamina, CurrentStamina);
}

void UStaminaComponent::SetMaxStamina(float NewMaxStamina)
{
	if (NewMaxStamina <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Invalid max stamina value: %.1f"), NewMaxStamina);
		return;
	}

	// 保存当前精力百分比
	float CurrentPercentage = GetStaminaPercentage();

	// 更新最大精力
	StaminaSettings.MaxStamina = NewMaxStamina;

	// 根据新的最大值调整当前精力
	CurrentStamina = StaminaSettings.MaxStamina * CurrentPercentage;
	ClampStaminaValue();

	// 触发事件
	TriggerStaminaChangedEvent();

	UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Max stamina updated to %.1f, Current: %.1f"), 
		NewMaxStamina, CurrentStamina);
}

// ==================== 私有辅助函数实现 ====================

void UStaminaComponent::UpdateStaminaRecovery(float DeltaTime)
{
	if (!bStaminaRecoveryEnabled || CurrentStamina >= StaminaSettings.MaxStamina)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	float TimeSinceLastUse = CurrentTime - LastStaminaUseTime;

	// 检查是否已过延迟时间
	if (TimeSinceLastUse < StaminaSettings.StaminaRecoveryDelay)
	{
		return;
	}

	// 开始恢复（如果还没开始）
	if (!bIsRecoveringStamina)
	{
		bIsRecoveringStamina = true;
		StaminaRecoveryStartTime = CurrentTime;
		
		// 更新状态为恢复中
		if (CurrentStaminaState != EStaminaState::Recovering && CurrentStaminaState != EStaminaState::Normal)
		{
			UpdateStaminaState(EStaminaState::Recovering);
		}
	}

	// 计算恢复量
	float RecoveryRate = GetCurrentRecoveryRate();
	float RecoveryAmount = RecoveryRate * DeltaTime;
	
	// 应用恢复
	if (RecoveryAmount > 0.0f)
	{
		RecoverStamina(RecoveryAmount);
	}
}

float UStaminaComponent::GetActionStaminaCost(EStaminaAction Action) const
{
	switch (Action)
	{
		case EStaminaAction::Attack:
			return StaminaSettings.AttackStaminaCost;
		case EStaminaAction::HeavyAttack:
			return StaminaSettings.HeavyAttackStaminaCost;
		case EStaminaAction::Dodge:
			return StaminaSettings.DodgeStaminaCost;
		case EStaminaAction::Block:
			return StaminaSettings.BlockStaminaCost;
		case EStaminaAction::Sprint:
			return StaminaSettings.SprintStaminaCostPerSecond;
		case EStaminaAction::WeaponSkill:
			return StaminaSettings.WeaponSkillStaminaCost;
		default:
			UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Unknown action type: %d"), (int32)Action);
			return 0.0f;
	}
}

void UStaminaComponent::UpdateStaminaState(EStaminaState NewState)
{
	if (CurrentStaminaState == NewState)
	{
		return;
	}

	EStaminaState OldState = CurrentStaminaState;
	CurrentStaminaState = NewState;

	// 触发状态变化事件
	OnStaminaStateChanged.Broadcast(OldState, NewState);

	UE_LOG(LogTemp, Log, TEXT("StaminaComponent: State changed from %d to %d"), 
		(int32)OldState, (int32)NewState);
}

void UStaminaComponent::TriggerStaminaChangedEvent()
{
	OnStaminaChanged.Broadcast(CurrentStamina, StaminaSettings.MaxStamina);
}

void UStaminaComponent::CheckStaminaDepletion()
{
	if (CurrentStamina <= 0.0f && CurrentStaminaState != EStaminaState::Depleted)
	{
		UpdateStaminaState(EStaminaState::Depleted);
		ExhaustedCounter++;
		
		// 检查是否进入过度疲劳状态
		if (ExhaustedCounter >= 3) // 连续耗尽3次进入过度疲劳
		{
			UpdateStaminaState(EStaminaState::Exhausted);
			UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Entered exhausted state after %d depletions"), ExhaustedCounter);
		}

		OnStaminaDepleted.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Stamina depleted! Count: %d"), ExhaustedCounter);
	}
}

void UStaminaComponent::CheckStaminaFullRecovery()
{
	if (IsStaminaFull() && CurrentStaminaState != EStaminaState::Normal)
	{
		UpdateStaminaState(EStaminaState::Normal);
		bIsRecoveringStamina = false;
		
		// 重置过度疲劳计数器（完全恢复后）
		if (ExhaustedCounter > 0)
		{
			ExhaustedCounter = FMath::Max(0, ExhaustedCounter - 1);
			UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Exhausted counter reduced to %d"), ExhaustedCounter);
		}

		OnStaminaFullyRecovered.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("StaminaComponent: Stamina fully recovered"));
	}
}

float UStaminaComponent::GetCurrentRecoveryRate() const
{
	float BaseRate = StaminaSettings.StaminaRecoveryRate;

	// 应用过度疲劳惩罚
	if (CurrentStaminaState == EStaminaState::Exhausted)
	{
		BaseRate *= StaminaSettings.ExhaustedRecoveryPenalty;
	}

	return BaseRate;
}

void UStaminaComponent::ClampStaminaValue()
{
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, StaminaSettings.MaxStamina);
}