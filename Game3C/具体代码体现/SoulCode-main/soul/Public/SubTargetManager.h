// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SubTargetTypes.h"
#include "SubTargetManager.generated.h"

// ==================== 前置声明 ====================

class USubTargetContainerComponent;
class AActor;

// ==================== 子锁点管理组件 ====================

/**
 * USubTargetManager
 *
 * 挂载在玩家角色身上的子锁点管理组件，是多锁点锁定系统的"决策大脑"。
 *
 * 核心职责：
 * - 当 Entity 切换时，从新 Entity 的 SubTargetContainerComponent 读取子锁点列表
 * - 自动选择 bIsDefault 的子锁点作为初始锁定
 * - 响应左右输入，判断是 Entity 内部切换还是 Entity Escape
 * - 锁点失效时 Fallback 到 Cluster 内最近合法点
 * - 对外暴露 GetCurrentLockPosition() 供相机和 UI 使用
 * - 计算黏性评分（含近距离威胁衰减）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API USubTargetManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// 构造函数
	USubTargetManager();

	// ==================== 黏性参数 ====================

	/** 黏性加分权重，越大越难切出当前 Entity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Stickiness", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float StickinessWeight = 0.5f;

	/** 占屏比例对黏性的贡献权重 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Stickiness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScreenOccupancyWeight = 0.3f;

	/** 距离近度对黏性的贡献权重 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Stickiness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DistanceWeight = 0.4f;

	/** 锁定持续时间对黏性的贡献权重 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Stickiness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LockDurationWeight = 0.3f;

	/** 锁定持续时间达到此值后黏性贡献封顶 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Stickiness", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float LockDurationMax = 5.0f;

	// ==================== 近距离威胁衰减参数 ====================

	/** 近距离威胁触发阈值（UU），300=3米 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|NearbyThreat", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float NearbyThreatThreshold = 300.0f;

	/** 黏性最低衰减系数，0.2=黏性降到20% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|NearbyThreat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NearbyThreatMinFactor = 0.2f;

	// ==================== 核心接口 ====================

	/** 当 Entity 切换时调用，读取新 Entity 的 SubTargetContainer 并选择默认子锁点 */
	UFUNCTION(BlueprintCallable, Category = "SubTarget")
	void SetLockedEntity(AActor* NewEntity);

	/** 解锁时调用，清除所有状态 */
	UFUNCTION(BlueprintCallable, Category = "SubTarget")
	void ClearLockedEntity();

	/** 返回当前子锁点的世界坐标（相机和 UI 读这个） */
	UFUNCTION(BlueprintPure, Category = "SubTarget")
	FVector GetCurrentLockPosition() const;

	/** 当前是否有有效的子锁点 */
	UFUNCTION(BlueprintPure, Category = "SubTarget")
	bool HasValidSubTarget() const;

	/** 当前锁定的 Entity 是否为多锁点敌人 */
	UFUNCTION(BlueprintPure, Category = "SubTarget")
	bool IsMultiTargetEntity() const;

	// ==================== 切换相关 ====================

	/**
	 * 尝试在当前 Entity 内部按屏幕 X 轴方向切换子锁点。
	 * DirectionX > 0 = 右，< 0 = 左。
	 * 返回 true 表示成功切换（消耗了输入），false 表示没有可切换的（输入应传递给 Entity 级切换）
	 */
	UFUNCTION(BlueprintCallable, Category = "SubTarget|Switch")
	bool TrySwitchSubTarget(float DirectionX);

	/**
	 * 计算当前 Entity 的黏性加分值，AllCandidates 用于检测近距离威胁
	 */
	UFUNCTION(BlueprintCallable, Category = "SubTarget|Switch")
	float CalculateStickinessBonus(AActor* CurrentEntity, const TArray<AActor*>& AllCandidates) const;

	// ==================== 信息查询 ====================

	/** 返回当前子锁点的定义副本 */
	UFUNCTION(BlueprintPure, Category = "SubTarget|Info")
	FSubTargetDefinition GetCurrentSubTargetDefinition() const;

	/** 返回当前子锁点在可用列表中的索引 */
	UFUNCTION(BlueprintPure, Category = "SubTarget|Info")
	int32 GetCurrentSubTargetIndex() const;

	/** 返回当前锁定的 Entity */
	UFUNCTION(BlueprintPure, Category = "SubTarget|Info")
	AActor* GetLockedEntity() const;

protected:
	// ==================== 生命周期 ====================

	virtual void BeginPlay() override;

private:
	// ==================== 私有成员 ====================

	/** 当前锁定的 Entity */
	UPROPERTY()
	AActor* CurrentEntity;

	/** 当前 Entity 的容器组件缓存 */
	UPROPERTY()
	USubTargetContainerComponent* CurrentContainer;

	/** 当前选中的子锁点定义 */
	FSubTargetDefinition CurrentSubTarget;

	/** 当前子锁点在可用列表中的索引 */
	int32 CurrentSubTargetIndex;

	/** 锁定开始时间（用于持续时间计算） */
	float LockStartTime;

	/** 缓存的可用子锁点列表 */
	TArray<FSubTargetDefinition> CachedAvailableSubTargets;

	// ==================== 私有函数 ====================

	/** 内部函数：从 CurrentContainer 选择默认子锁点 */
	void SelectDefaultSubTarget();

	/** 内部函数：刷新缓存的可用子锁点列表 */
	void RefreshAvailableSubTargets();

	/** 内部函数：计算近距离威胁衰减因子 */
	float CalculateNearbyThreatFactor(AActor* CurrentEntity, const TArray<AActor*>& AllCandidates) const;

	/** 内部函数：计算 Entity 的屏幕占比（0-1） */
	float CalculateScreenOccupancy(AActor* Entity) const;
};
