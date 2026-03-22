// Fill out your copyright notice in the Description page of Project Settings.

#include "PoiseComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UPoiseComponent::UPoiseComponent()
{
	// Set this component to be ticked every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

	// 初始化韧性设置
	PoiseSettings = FPoiseSettings();

	// 初始化状态变量
	CurrentPoise = PoiseSettings.MaxPoise;
	CurrentPoiseState = EPoiseState::Normal;
	LastDamageTime = 0.0f;
	PoiseImmuneEndTime = 0.0f;
	OwnerCharacter = nullptr;
	bIsStaggering = false;
	bManuallySetImmune = false;

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Constructor completed"));
}

// Called when the game starts
void UPoiseComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取拥有者角色引用
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("PoiseComponent: Owner is not a Character! Component will not function properly."));
		return;
	}

	// 初始化韧性值
	CurrentPoise = PoiseSettings.MaxPoise;
	CurrentPoiseState = EPoiseState::Normal;
	LastDamageTime = 0.0f;
	PoiseImmuneEndTime = 0.0f;
	bIsStaggering = false;
	bManuallySetImmune = false;

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: BeginPlay completed for %s"), 
		OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Initial Poise: %.1f/%.1f"), CurrentPoise, PoiseSettings.MaxPoise);
}

// Called every frame
void UPoiseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValidForPoiseOperations())
	{
		return;
	}

	// 更新韧性恢复
	UpdatePoiseRecovery(DeltaTime);

	// 检查韧性免疫状态
	if (IsPoiseImmune() && !bManuallySetImmune)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime >= PoiseImmuneEndTime)
		{
			EndPoiseImmune();
		}
	}
}

// ==================== 核心接口函数实现 ====================

bool UPoiseComponent::TakePoiseDamage(float PoiseDamage, AActor* DamageSource)
{
	if (!IsValidForPoiseOperations() || PoiseDamage <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Invalid poise damage attempt - Damage: %.1f"), PoiseDamage);
		return false;
	}

	// 检查韧性免疫
	if (IsPoiseImmune())
	{
		UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Poise damage blocked by immunity"));
		return false;
	}

	// 如果已经破坏，不能再受到韧性伤害
	if (IsPoiseBroken())
	{
		UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Poise damage blocked - already broken"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Taking poise damage %.1f from %s"), 
		PoiseDamage, DamageSource ? *DamageSource->GetName() : TEXT("Unknown"));

	// 记录伤害时间
	LastDamageTime = GetWorld()->GetTimeSeconds();

	// 减少韧性值
	float PreviousPoise = CurrentPoise;
	CurrentPoise = FMath::Max(0.0f, CurrentPoise - PoiseDamage);

	// 更新状态
	if (CurrentPoise <= 0.0f)
	{
		// 韧性破坏
		BreakPoise(DamageSource);
	}
	else if (CurrentPoise < PoiseSettings.MaxPoise)
	{
		// 韧性受损
		SetPoiseState(EPoiseState::Damaged);
	}

	// 广播韧性变化事件
	BroadcastPoiseChanged();

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Poise changed from %.1f to %.1f"), PreviousPoise, CurrentPoise);

	return true;
}

void UPoiseComponent::BreakPoise(AActor* DamageSource)
{
	if (!IsValidForPoiseOperations())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Breaking poise for %s"), 
		OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));

	// 设置韧性为0
	CurrentPoise = 0.0f;

	// 设置破坏状态
	SetPoiseState(EPoiseState::Broken);

	// 计算硬直持续时间
	float StaggerDuration = CalculateStaggerDuration(0.0f); // 使用基础硬直时间

	// 开始硬直
	StartStagger(StaggerDuration, DamageSource);

	// 广播韧性破坏事件
	OnPoiseBreak.Broadcast(GetOwner(), StaggerDuration, DamageSource);

	// 广播韧性变化事件
	BroadcastPoiseChanged();

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Poise broken - Stagger duration: %.2f"), StaggerDuration);
}

void UPoiseComponent::RecoverPoise(float RecoveryAmount)
{
	if (!IsValidForPoiseOperations())
	{
		return;
	}

	// 如果处于免疫状态或硬直中，不能恢复韧性
	if (IsPoiseImmune() || bIsStaggering)
	{
		return;
	}

	// 如果没有指定恢复量，使用默认速率
	if (RecoveryAmount <= 0.0f)
	{
		RecoveryAmount = PoiseSettings.PoiseRecoveryRate * GetWorld()->GetDeltaSeconds();
	}

	float PreviousPoise = CurrentPoise;
	CurrentPoise = FMath::Min(PoiseSettings.MaxPoise, CurrentPoise + RecoveryAmount);

	// 更新状态
	if (CurrentPoise >= PoiseSettings.MaxPoise)
	{
		SetPoiseState(EPoiseState::Normal);
	}
	else if (CurrentPoise > 0.0f && CurrentPoiseState == EPoiseState::Broken)
	{
		SetPoiseState(EPoiseState::Recovering);
	}

	// 广播韧性变化事件
	if (FMath::Abs(CurrentPoise - PreviousPoise) > 0.01f)
	{
		BroadcastPoiseChanged();
	}
}

void UPoiseComponent::ResetPoise()
{
	if (!IsValidForPoiseOperations())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Resetting poise to maximum"));

	CurrentPoise = PoiseSettings.MaxPoise;
	SetPoiseState(EPoiseState::Normal);

	// 清除硬直状态
	if (bIsStaggering)
	{
		EndStagger();
	}

	// 广播韧性变化事件
	BroadcastPoiseChanged();
}

void UPoiseComponent::SetPoiseImmune(bool bImmune, float Duration)
{
	if (!IsValidForPoiseOperations())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Setting poise immune: %s, Duration: %.2f"), 
		bImmune ? TEXT("True") : TEXT("False"), Duration);

	bManuallySetImmune = bImmune;

	if (bImmune)
	{
		SetPoiseState(EPoiseState::Immune);

		if (Duration > 0.0f)
		{
			// 设置定时免疫
			PoiseImmuneEndTime = GetWorld()->GetTimeSeconds() + Duration;
			
			// 清除之前的定时器
			GetWorld()->GetTimerManager().ClearTimer(ImmuneTimerHandle);
			
			// 设置新的定时器
			GetWorld()->GetTimerManager().SetTimer(
				ImmuneTimerHandle,
				this,
				&UPoiseComponent::EndPoiseImmune,
				Duration,
				false
			);
		}
		else
		{
			// 永久免疫
			PoiseImmuneEndTime = 0.0f;
		}
	}
	else
	{
		EndPoiseImmune();
	}
}

// ==================== 状态查询函数实现 ====================

bool UPoiseComponent::IsPoiseBroken() const
{
	return CurrentPoiseState == EPoiseState::Broken;
}

bool UPoiseComponent::IsPoiseImmune() const
{
	return CurrentPoiseState == EPoiseState::Immune;
}

float UPoiseComponent::GetCurrentPoise() const
{
	return CurrentPoise;
}

float UPoiseComponent::GetPoisePercentage() const
{
	if (PoiseSettings.MaxPoise <= 0.0f)
	{
		return 0.0f;
	}
	return (CurrentPoise / PoiseSettings.MaxPoise) * 100.0f;
}

EPoiseState UPoiseComponent::GetPoiseState() const
{
	return CurrentPoiseState;
}

float UPoiseComponent::GetMaxPoise() const
{
	return PoiseSettings.MaxPoise;
}

bool UPoiseComponent::IsStaggering() const
{
	return bIsStaggering;
}

// ==================== 配置函数实现 ====================

void UPoiseComponent::UpdatePoiseSettings(const FPoiseSettings& NewSettings)
{
	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Updating poise settings"));

	FPoiseSettings OldSettings = PoiseSettings;
	PoiseSettings = NewSettings;

	// 如果最大韧性值发生变化，调整当前韧性值
	if (OldSettings.MaxPoise != NewSettings.MaxPoise)
	{
		float PoiseRatio = (OldSettings.MaxPoise > 0.0f) ? (CurrentPoise / OldSettings.MaxPoise) : 1.0f;
		CurrentPoise = NewSettings.MaxPoise * PoiseRatio;
		CurrentPoise = FMath::Clamp(CurrentPoise, 0.0f, NewSettings.MaxPoise);

		// 广播韧性变化事件
		BroadcastPoiseChanged();
	}
}

void UPoiseComponent::SetMaxPoise(float NewMaxPoise)
{
	if (NewMaxPoise <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Invalid max poise value: %.1f"), NewMaxPoise);
		return;
	}

	float OldMaxPoise = PoiseSettings.MaxPoise;
	PoiseSettings.MaxPoise = NewMaxPoise;

	// 按比例调整当前韧性值
	if (OldMaxPoise > 0.0f)
	{
		float PoiseRatio = CurrentPoise / OldMaxPoise;
		CurrentPoise = NewMaxPoise * PoiseRatio;
	}
	else
	{
		CurrentPoise = NewMaxPoise;
	}

	CurrentPoise = FMath::Clamp(CurrentPoise, 0.0f, NewMaxPoise);

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Max poise changed from %.1f to %.1f, Current poise: %.1f"), 
		OldMaxPoise, NewMaxPoise, CurrentPoise);

	// 广播韧性变化事件
	BroadcastPoiseChanged();
}

// ==================== 私有辅助函数实现 ====================

void UPoiseComponent::UpdatePoiseRecovery(float DeltaTime)
{
	// 只有在正常或恢复状态下才能自动恢复韧性
	if (CurrentPoiseState != EPoiseState::Normal && 
		CurrentPoiseState != EPoiseState::Damaged && 
		CurrentPoiseState != EPoiseState::Recovering)
	{
		return;
	}

	// 检查是否需要等待恢复延迟
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastDamageTime < PoiseSettings.PoiseRecoveryDelay)
	{
		return;
	}

	// 如果韧性已满，不需要恢复
	if (CurrentPoise >= PoiseSettings.MaxPoise)
	{
		if (CurrentPoiseState != EPoiseState::Normal)
		{
			SetPoiseState(EPoiseState::Normal);
		}
		return;
	}

	// 执行韧性恢复
	float RecoveryAmount = PoiseSettings.PoiseRecoveryRate * DeltaTime;
	RecoverPoise(RecoveryAmount);
}

void UPoiseComponent::StartStagger(float StaggerDuration, AActor* DamageSource)
{
	if (!IsValidForPoiseOperations())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Starting stagger for %.2f seconds"), StaggerDuration);

	bIsStaggering = true;

	// 广播硬直开始事件
	OnStaggerStarted.Broadcast(GetOwner(), StaggerDuration);

	// 设置硬直结束定时器
	GetWorld()->GetTimerManager().ClearTimer(StaggerTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		StaggerTimerHandle,
		this,
		&UPoiseComponent::EndStagger,
		StaggerDuration,
		false
	);

	// 这里可以添加对角色移动的限制等逻辑
	// 例如：禁用角色移动、播放硬直动画等
	if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
	{
		// 可以在这里添加移动限制逻辑
		// OwnerCharacter->GetCharacterMovement()->DisableMovement();
	}
}

void UPoiseComponent::EndStagger()
{
	if (!IsValidForPoiseOperations())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Ending stagger"));

	bIsStaggering = false;

	// 清除硬直定时器
	GetWorld()->GetTimerManager().ClearTimer(StaggerTimerHandle);

	// 设置韧性破坏后的免疫时间
	if (PoiseSettings.PoiseImmuneTimeAfterBreak > 0.0f)
	{
		PoiseImmuneEndTime = GetWorld()->GetTimeSeconds() + PoiseSettings.PoiseImmuneTimeAfterBreak;
		SetPoiseState(EPoiseState::Immune);

		// 设置免疫结束定时器
		GetWorld()->GetTimerManager().SetTimer(
			ImmuneTimerHandle,
			this,
			&UPoiseComponent::EndPoiseImmune,
			PoiseSettings.PoiseImmuneTimeAfterBreak,
			false
		);
	}
	else
	{
		// 直接进入恢复状态
		SetPoiseState(EPoiseState::Recovering);
	}

	// 广播硬直结束事件
	OnStaggerEnded.Broadcast(GetOwner());

	// 恢复角色移动能力
	if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
	{
		// 可以在这里恢复移动能力
		// OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void UPoiseComponent::SetPoiseState(EPoiseState NewState)
{
	if (CurrentPoiseState == NewState)
	{
		return;
	}

	EPoiseState OldState = CurrentPoiseState;
	CurrentPoiseState = NewState;

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: State changed from %d to %d"), 
		(int32)OldState, (int32)NewState);

	// 广播状态变化事件
	OnPoiseStateChanged.Broadcast(GetOwner(), NewState);
}

void UPoiseComponent::EndPoiseImmune()
{
	if (!IsValidForPoiseOperations())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PoiseComponent: Ending poise immunity"));

	bManuallySetImmune = false;
	PoiseImmuneEndTime = 0.0f;

	// 清除免疫定时器
	GetWorld()->GetTimerManager().ClearTimer(ImmuneTimerHandle);

	// 根据当前韧性值设置合适的状态
	if (CurrentPoise >= PoiseSettings.MaxPoise)
	{
		SetPoiseState(EPoiseState::Normal);
	}
	else if (CurrentPoise > 0.0f)
	{
		SetPoiseState(EPoiseState::Recovering);
	}
	else
	{
		SetPoiseState(EPoiseState::Broken);
	}
}

float UPoiseComponent::CalculateStaggerDuration(float PoiseDamage) const
{
	// 基础硬直时间，可以根据伤害值进行调整
	float StaggerDuration = PoiseSettings.BaseStaggerDuration;

	// 可以根据韧性伤害值调整硬直时间
	// 例如：更高的伤害导致更长的硬直时间
	if (PoiseDamage > 0.0f)
	{
		float DamageMultiplier = FMath::Clamp(PoiseDamage / PoiseSettings.MaxPoise, 0.5f, 2.0f);
		StaggerDuration *= DamageMultiplier;
	}

	return FMath::Clamp(StaggerDuration, 0.1f, 10.0f);
}

void UPoiseComponent::BroadcastPoiseChanged()
{
	if (IsValidForPoiseOperations())
	{
		OnPoiseChanged.Broadcast(GetOwner(), CurrentPoise, PoiseSettings.MaxPoise);
	}
}

bool UPoiseComponent::IsValidForPoiseOperations() const
{
	return IsValid(this) && IsValid(GetOwner()) && GetWorld() != nullptr;
}