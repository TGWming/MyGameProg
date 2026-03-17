// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SubTargetTypes.h"
#include "SubTargetContainerComponent.generated.h"

// ==================== 子锁点容器组件 ====================

/**
 * USubTargetContainerComponent
 *
 * 挂载在敌人身上的子锁点容器组件。
 * 持有一个 TArray<FSubTargetDefinition>，描述该敌人拥有的所有可锁定身体部位。
 * 小怪通常配置 1 个 SubTarget，大怪配置多个。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API USubTargetContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 构造函数
	USubTargetContainerComponent();

	// ==================== 公共属性 ====================

	/** 子锁点定义列表，在编辑器中按需配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget Config", meta = (TitleProperty = "Tag"))
	TArray<FSubTargetDefinition> SubTargets;

	// ==================== 公共查询函数 ====================

	/** 返回所有可用（IsAvailable）的子锁点 */
	UFUNCTION(BlueprintCallable, Category = "SubTarget")
	TArray<FSubTargetDefinition> GetAvailableSubTargets() const;

	/** 获取默认子锁点，成功返回 true */
	UFUNCTION(BlueprintCallable, Category = "SubTarget")
	bool GetDefaultSubTarget(FSubTargetDefinition& OutDefinition) const;

	/** 按 Tag 查找子锁点，成功返回 true */
	UFUNCTION(BlueprintCallable, Category = "SubTarget")
	bool GetSubTargetByTag(ESubTargetTag Tag, FSubTargetDefinition& OutDefinition) const;

	/** 按索引查找子锁点，成功返回 true */
	UFUNCTION(BlueprintCallable, Category = "SubTarget")
	bool GetSubTargetByIndex(int32 Index, FSubTargetDefinition& OutDefinition) const;

	/** 获取子锁点总数 */
	UFUNCTION(BlueprintPure, Category = "SubTarget")
	int32 GetSubTargetCount() const;

	/** 获取当前可用的子锁点数量 */
	UFUNCTION(BlueprintPure, Category = "SubTarget")
	int32 GetAvailableSubTargetCount() const;

	/** 判断是否为多锁点敌人（可用子锁点 > 1） */
	UFUNCTION(BlueprintPure, Category = "SubTarget")
	bool IsMultiTargetEntity() const;

	// ==================== 运行时控制函数 ====================

	/** 按 Tag 启用或禁用子锁点，成功返回 true */
	UFUNCTION(BlueprintCallable, Category = "SubTarget|Runtime")
	bool SetSubTargetEnabled(ESubTargetTag Tag, bool bEnabled);

	/** 按索引启用或禁用子锁点，成功返回 true */
	UFUNCTION(BlueprintCallable, Category = "SubTarget|Runtime")
	bool SetSubTargetEnabledByIndex(int32 Index, bool bEnabled);

	/** 禁用所有子锁点 */
	UFUNCTION(BlueprintCallable, Category = "SubTarget|Runtime")
	void DisableAllSubTargets();

	// ==================== 世界位置查询函数 ====================

	/** 获取指定子锁点的世界坐标，成功返回 true */
	UFUNCTION(BlueprintCallable, Category = "SubTarget|Position")
	bool GetSubTargetWorldLocation(const FSubTargetDefinition& Definition, FVector& OutWorldLocation) const;

protected:
	// ==================== 生命周期 ====================

	virtual void BeginPlay() override;

private:
	// ==================== 私有成员 ====================

	/** 缓存的骨骼网格体引用，用于 Socket 位置查询 */
	UPROPERTY()
	USkeletalMeshComponent* CachedMesh;

	/** 校验配置有效性，BeginPlay 时调用 */
	void ValidateConfiguration() const;
};
