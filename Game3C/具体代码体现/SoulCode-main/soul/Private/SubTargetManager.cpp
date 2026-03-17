// Fill out your copyright notice in the Description page of Project Settings.

#include "SubTargetManager.h"
#include "SubTargetContainerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

// ==================== 构造函数 ====================

USubTargetManager::USubTargetManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentEntity = nullptr;
	CurrentContainer = nullptr;
	CurrentSubTargetIndex = -1;
	LockStartTime = 0.0f;
}

// ==================== 生命周期 ====================

void USubTargetManager::BeginPlay()
{
	Super::BeginPlay();

	const FString OwnerName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("[SubTargetManager] SubTargetManager initialized on %s"), *OwnerName);
}

// ==================== 核心接口 ====================

void USubTargetManager::SetLockedEntity(AActor* NewEntity)
{
	// 空指针 -> 解锁
	if (NewEntity == nullptr)
	{
		ClearLockedEntity();
		return;
	}

	// 避免重复设置同一 Entity
	if (NewEntity == CurrentEntity)
	{
		return;
	}

	CurrentEntity = NewEntity;
	CurrentContainer = NewEntity->FindComponentByClass<USubTargetContainerComponent>();
	LockStartTime = GetWorld()->GetTimeSeconds();

	if (CurrentContainer)
	{
		// 刷新可用列表并选择默认子锁点
		RefreshAvailableSubTargets();
		SelectDefaultSubTarget();

		UE_LOG(LogTemp, Log, TEXT("[SubTargetManager] Locked entity %s with %d available SubTargets"),
			*NewEntity->GetName(),
			CachedAvailableSubTargets.Num());
	}
	else
	{
		// 无容器组件，回退到 Actor 位置
		CurrentSubTargetIndex = -1;
		CachedAvailableSubTargets.Empty();

		UE_LOG(LogTemp, Log, TEXT("[SubTargetManager] Locked entity %s has no SubTargetContainer, using actor location"),
			*NewEntity->GetName());
	}
}

void USubTargetManager::ClearLockedEntity()
{
	CurrentEntity = nullptr;
	CurrentContainer = nullptr;
	CurrentSubTargetIndex = -1;
	CachedAvailableSubTargets.Empty();
	CurrentSubTarget = FSubTargetDefinition();
	LockStartTime = 0.0f;
}

FVector USubTargetManager::GetCurrentLockPosition() const
{
	// 优先使用子锁点世界位置
	if (CurrentContainer && CurrentSubTarget.IsAvailable())
	{
		FVector WorldLocation;
		if (CurrentContainer->GetSubTargetWorldLocation(CurrentSubTarget, WorldLocation))
		{
			return WorldLocation;
		}
	}

	// Fallback：无 Container 或无有效 SubTarget，使用 Actor 位置
	if (CurrentEntity)
	{
		return CurrentEntity->GetActorLocation();
	}

	// 全部失败
	return FVector::ZeroVector;
}

bool USubTargetManager::HasValidSubTarget() const
{
	return CurrentContainer != nullptr && CurrentSubTarget.IsAvailable();
}

bool USubTargetManager::IsMultiTargetEntity() const
{
	return CurrentContainer != nullptr && CurrentContainer->IsMultiTargetEntity();
}

// ==================== 切换相关 ====================

bool USubTargetManager::TrySwitchSubTarget(float DirectionX)
{
	// 无法内部切换的情况
	if (CurrentContainer == nullptr || CachedAvailableSubTargets.Num() <= 1)
	{
		return false;
	}

	// 获取 PlayerController 用于屏幕投影
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (PC == nullptr)
	{
		return false;
	}

	// 临时结构：存储子锁点索引和屏幕 X 坐标
	struct FScreenProjection
	{
		int32 Index;
		float ScreenX;
	};

	// 将所有可用子锁点投影到屏幕空间
	TArray<FScreenProjection> Projections;
	for (int32 i = 0; i < CachedAvailableSubTargets.Num(); i++)
	{
		FVector WorldLocation;
		if (CurrentContainer->GetSubTargetWorldLocation(CachedAvailableSubTargets[i], WorldLocation))
		{
			FVector2D ScreenPos;
			if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPos))
			{
				Projections.Add({ i, ScreenPos.X });
			}
		}
	}

	// 按 ScreenX 从左到右排序
	Projections.Sort([](const FScreenProjection& A, const FScreenProjection& B)
	{
		return A.ScreenX < B.ScreenX;
	});

	// 找到当前子锁点在排序后列表中的位置
	int32 CurrentPosInSorted = INDEX_NONE;
	for (int32 i = 0; i < Projections.Num(); i++)
	{
		if (Projections[i].Index == CurrentSubTargetIndex)
		{
			CurrentPosInSorted = i;
			break;
		}
	}

	if (CurrentPosInSorted == INDEX_NONE)
	{
		return false;
	}

	// 根据方向选择下一个子锁点
	int32 NewPosInSorted = INDEX_NONE;
	if (DirectionX > 0)
	{
		// 向右：选择排序后的下一个
		NewPosInSorted = CurrentPosInSorted + 1;
	}
	else if (DirectionX < 0)
	{
		// 向左：选择排序后的上一个
		NewPosInSorted = CurrentPosInSorted - 1;
	}
	else
	{
		return false;
	}

	// 检查是否超出边界
	if (NewPosInSorted < 0 || NewPosInSorted >= Projections.Num())
	{
		// 到达边界，输入未被消耗，应传递给 Entity 级切换逻辑
		return false;
	}

	// 成功切换
	int32 NewIndex = Projections[NewPosInSorted].Index;
	CurrentSubTargetIndex = NewIndex;
	CurrentSubTarget = CachedAvailableSubTargets[NewIndex];

	UE_LOG(LogTemp, Log, TEXT("[SubTargetManager] SubTarget switched to [%s] (index %d)"),
		*UEnum::GetValueAsString(CurrentSubTarget.Tag),
		CurrentSubTargetIndex);

	return true;
}

float USubTargetManager::CalculateStickinessBonus(AActor* InCurrentEntity, const TArray<AActor*>& AllCandidates) const
{
	if (InCurrentEntity == nullptr)
	{
		return 0.0f;
	}

	// 占屏比例因子
	float ScreenFactor = CalculateScreenOccupancy(InCurrentEntity) * ScreenOccupancyWeight;

	// 距离近度因子
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), InCurrentEntity->GetActorLocation());
	float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f) * DistanceWeight;

	// 锁定持续时间因子
	float Duration = GetWorld()->GetTimeSeconds() - LockStartTime;
	float DurationFactor = FMath::Clamp(Duration / LockDurationMax, 0.0f, 1.0f) * LockDurationWeight;

	// 基础黏性加分
	float BaseStickinessBonus = (ScreenFactor + DistanceFactor + DurationFactor) * StickinessWeight;

	// 近距离威胁衰减
	float NearbyThreatFactor = CalculateNearbyThreatFactor(InCurrentEntity, AllCandidates);

	return BaseStickinessBonus * NearbyThreatFactor;
}

// ==================== 信息查询 ====================

FSubTargetDefinition USubTargetManager::GetCurrentSubTargetDefinition() const
{
	return CurrentSubTarget;
}

int32 USubTargetManager::GetCurrentSubTargetIndex() const
{
	return CurrentSubTargetIndex;
}

AActor* USubTargetManager::GetLockedEntity() const
{
	return CurrentEntity;
}

// ==================== 私有函数 ====================

void USubTargetManager::SelectDefaultSubTarget()
{
	if (CurrentContainer == nullptr)
	{
		return;
	}

	FSubTargetDefinition DefaultDef;
	if (CurrentContainer->GetDefaultSubTarget(DefaultDef))
	{
		CurrentSubTarget = DefaultDef;

		// 在缓存列表中查找匹配索引（通过 SocketName 比较）
		CurrentSubTargetIndex = INDEX_NONE;
		for (int32 i = 0; i < CachedAvailableSubTargets.Num(); i++)
		{
			if (CachedAvailableSubTargets[i].SocketName == DefaultDef.SocketName)
			{
				CurrentSubTargetIndex = i;
				break;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[SubTargetManager] Default SubTarget selected: [%s] (Socket: %s)"),
			*UEnum::GetValueAsString(DefaultDef.Tag),
			*DefaultDef.SocketName.ToString());
	}
	else
	{
		const FString EntityName = CurrentEntity ? CurrentEntity->GetName() : TEXT("Unknown");
		UE_LOG(LogTemp, Warning, TEXT("[SubTargetManager] No valid SubTarget found for entity %s"), *EntityName);
	}
}

void USubTargetManager::RefreshAvailableSubTargets()
{
	if (CurrentContainer == nullptr)
	{
		CachedAvailableSubTargets.Empty();
		return;
	}

	CachedAvailableSubTargets = CurrentContainer->GetAvailableSubTargets();
}

float USubTargetManager::CalculateNearbyThreatFactor(AActor* InCurrentEntity, const TArray<AActor*>& AllCandidates) const
{
	if (GetOwner() == nullptr)
	{
		return 1.0f;
	}

	FVector PlayerLocation = GetOwner()->GetActorLocation();
	float ClosestOtherDistance = NearbyThreatThreshold;

	for (AActor* Candidate : AllCandidates)
	{
		// 跳过 nullptr 和当前 Entity
		if (Candidate == nullptr || Candidate == InCurrentEntity)
		{
			continue;
		}

		float Dist = FVector::Dist(PlayerLocation, Candidate->GetActorLocation());
		if (Dist < ClosestOtherDistance)
		{
			ClosestOtherDistance = Dist;
		}
	}

	// 没有近距离威胁
	if (ClosestOtherDistance >= NearbyThreatThreshold)
	{
		return 1.0f;
	}

	// 距离=0 时 Factor=NearbyThreatMinFactor(0.2)，距离=阈值时 Factor=1.0
	float Alpha = ClosestOtherDistance / NearbyThreatThreshold;
	return FMath::Lerp(NearbyThreatMinFactor, 1.0f, Alpha);
}

float USubTargetManager::CalculateScreenOccupancy(AActor* Entity) const
{
	if (Entity == nullptr)
	{
		return 0.0f;
	}

	// 获取 Entity 的 Bounds
	FVector Origin, BoxExtent;
	Entity->GetActorBounds(false, Origin, BoxExtent);

	// 获取 PlayerController
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (PC == nullptr)
	{
		return 0.0f;
	}

	// 获取 ViewportSize
	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);
	if (ViewportX == 0 || ViewportY == 0)
	{
		return 0.0f;
	}

	// 投影 Bounds 的 8 个角点到屏幕空间，找 Min/Max
	FVector2D ScreenMin(MAX_FLT, MAX_FLT);
	FVector2D ScreenMax(-MAX_FLT, -MAX_FLT);

	for (int32 i = 0; i < 8; i++)
	{
		// 构建 8 个角点：Origin ± BoxExtent 的各组合
		FVector Corner = Origin;
		Corner.X += (i & 1) ? BoxExtent.X : -BoxExtent.X;
		Corner.Y += (i & 2) ? BoxExtent.Y : -BoxExtent.Y;
		Corner.Z += (i & 4) ? BoxExtent.Z : -BoxExtent.Z;

		FVector2D ScreenPos;
		if (PC->ProjectWorldLocationToScreen(Corner, ScreenPos))
		{
			ScreenMin.X = FMath::Min(ScreenMin.X, ScreenPos.X);
			ScreenMin.Y = FMath::Min(ScreenMin.Y, ScreenPos.Y);
			ScreenMax.X = FMath::Max(ScreenMax.X, ScreenPos.X);
			ScreenMax.Y = FMath::Max(ScreenMax.Y, ScreenPos.Y);
		}
	}

	// 计算占屏比例
	float ScreenWidth = ScreenMax.X - ScreenMin.X;
	float ScreenHeight = ScreenMax.Y - ScreenMin.Y;
	float Occupancy = (ScreenWidth * ScreenHeight) / (static_cast<float>(ViewportX) * static_cast<float>(ViewportY));

	return FMath::Clamp(Occupancy, 0.0f, 1.0f);
}
