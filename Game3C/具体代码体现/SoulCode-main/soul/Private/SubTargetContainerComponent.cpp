// Fill out your copyright notice in the Description page of Project Settings.

#include "SubTargetContainerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

// ==================== 构造函数 ====================

USubTargetContainerComponent::USubTargetContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CachedMesh = nullptr;
}

// ==================== 生命周期 ====================

void USubTargetContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 缓存 Owner 的 SkeletalMeshComponent
	if (AActor* Owner = GetOwner())
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			CachedMesh = Character->GetMesh();
		}
		else
		{
			CachedMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		}

		if (!CachedMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SubTargetContainer] %s: No SkeletalMeshComponent found"),
				*Owner->GetName());
		}
	}

	// 校验配置有效性
	ValidateConfiguration();
}

// ==================== 公共查询函数 ====================

TArray<FSubTargetDefinition> USubTargetContainerComponent::GetAvailableSubTargets() const
{
	TArray<FSubTargetDefinition> Result;
	for (const FSubTargetDefinition& Def : SubTargets)
	{
		if (Def.IsAvailable())
		{
			Result.Add(Def);
		}
	}
	return Result;
}

bool USubTargetContainerComponent::GetDefaultSubTarget(FSubTargetDefinition& OutDefinition) const
{
	// 第一轮：找标记为 bIsDefault 且可用的子锁点
	for (const FSubTargetDefinition& Def : SubTargets)
	{
		if (Def.bIsDefault && Def.IsAvailable())
		{
			OutDefinition = Def;
			return true;
		}
	}

	// 第二轮 Fallback：找第一个可用的子锁点
	for (const FSubTargetDefinition& Def : SubTargets)
	{
		if (Def.IsAvailable())
		{
			OutDefinition = Def;
			return true;
		}
	}

	return false;
}

bool USubTargetContainerComponent::GetSubTargetByTag(ESubTargetTag Tag, FSubTargetDefinition& OutDefinition) const
{
	for (const FSubTargetDefinition& Def : SubTargets)
	{
		if (Def.Tag == Tag && Def.IsAvailable())
		{
			OutDefinition = Def;
			return true;
		}
	}
	return false;
}

bool USubTargetContainerComponent::GetSubTargetByIndex(int32 Index, FSubTargetDefinition& OutDefinition) const
{
	if (SubTargets.IsValidIndex(Index))
	{
		OutDefinition = SubTargets[Index];
		return true;
	}
	return false;
}

int32 USubTargetContainerComponent::GetSubTargetCount() const
{
	return SubTargets.Num();
}

int32 USubTargetContainerComponent::GetAvailableSubTargetCount() const
{
	int32 Count = 0;
	for (const FSubTargetDefinition& Def : SubTargets)
	{
		if (Def.IsAvailable())
		{
			Count++;
		}
	}
	return Count;
}

bool USubTargetContainerComponent::IsMultiTargetEntity() const
{
	return GetAvailableSubTargetCount() > 1;
}

// ==================== 运行时控制函数 ====================

bool USubTargetContainerComponent::SetSubTargetEnabled(ESubTargetTag Tag, bool bEnabled)
{
	for (FSubTargetDefinition& Def : SubTargets)
	{
		if (Def.Tag == Tag)
		{
			Def.bIsEnabled = bEnabled;
			UE_LOG(LogTemp, Log, TEXT("[SubTargetContainer] %s: SubTarget '%s' set to %s"),
				*GetOwner()->GetName(),
				*UEnum::GetValueAsString(Tag),
				bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
			return true;
		}
	}
	return false;
}

bool USubTargetContainerComponent::SetSubTargetEnabledByIndex(int32 Index, bool bEnabled)
{
	if (SubTargets.IsValidIndex(Index))
	{
		SubTargets[Index].bIsEnabled = bEnabled;
		UE_LOG(LogTemp, Log, TEXT("[SubTargetContainer] %s: SubTarget[%d] '%s' set to %s"),
			*GetOwner()->GetName(),
			Index,
			*UEnum::GetValueAsString(SubTargets[Index].Tag),
			bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
		return true;
	}
	return false;
}

void USubTargetContainerComponent::DisableAllSubTargets()
{
	for (FSubTargetDefinition& Def : SubTargets)
	{
		Def.bIsEnabled = false;
	}
	UE_LOG(LogTemp, Log, TEXT("[SubTargetContainer] %s: All SubTargets disabled"),
		*GetOwner()->GetName());
}

// ==================== 世界位置查询函数 ====================

bool USubTargetContainerComponent::GetSubTargetWorldLocation(const FSubTargetDefinition& Definition, FVector& OutWorldLocation) const
{
	// 无效定义直接返回
	if (!Definition.IsValid())
	{
		return false;
	}

	// 优先使用 Socket 位置
	if (CachedMesh && CachedMesh->DoesSocketExist(Definition.SocketName))
	{
		OutWorldLocation = CachedMesh->GetSocketLocation(Definition.SocketName) + Definition.Offset;
		return true;
	}

	// Fallback：使用 Actor 位置
	if (AActor* Owner = GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SubTargetContainer] %s: Socket '%s' not found, using Actor location"),
			*Owner->GetName(),
			*Definition.SocketName.ToString());
		OutWorldLocation = Owner->GetActorLocation() + Definition.Offset;
		return true;
	}

	return false;
}

// ==================== 私有函数 ====================

void USubTargetContainerComponent::ValidateConfiguration() const
{
	// 获取 Owner 名称用于日志
	const FString OwnerName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");

	// 检查是否有子锁点配置
	if (SubTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SubTargetContainer] %s: SubTargets array is empty"), *OwnerName);
		return;
	}

	// 检查 bIsDefault 数量
	int32 DefaultCount = 0;
	for (const FSubTargetDefinition& Def : SubTargets)
	{
		if (Def.bIsDefault)
		{
			DefaultCount++;
		}
	}

	if (DefaultCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SubTargetContainer] %s: No SubTarget marked as bIsDefault"), *OwnerName);
	}
	else if (DefaultCount > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SubTargetContainer] %s: Multiple SubTargets marked as bIsDefault (%d found)"),
			*OwnerName, DefaultCount);
	}

	// 如果有缓存的骨骼网格体，检查 Socket 是否存在
	if (CachedMesh)
	{
		for (const FSubTargetDefinition& Def : SubTargets)
		{
			if (Def.SocketName != NAME_None && !CachedMesh->DoesSocketExist(Def.SocketName))
			{
				UE_LOG(LogTemp, Warning, TEXT("[SubTargetContainer] %s: Socket '%s' does not exist on SkeletalMesh"),
					*OwnerName, *Def.SocketName.ToString());
			}
		}
	}

	// 输出初始化摘要
	const int32 AvailableCount = GetAvailableSubTargetCount();
	UE_LOG(LogTemp, Log, TEXT("[SubTargetContainer] %s: Initialized with %d SubTargets (%d available, %s)"),
		*OwnerName,
		SubTargets.Num(),
		AvailableCount,
		AvailableCount > 1 ? TEXT("Multi-Target") : TEXT("Single-Target"));
}
