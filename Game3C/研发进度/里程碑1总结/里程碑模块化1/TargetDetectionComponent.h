// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnConfig.h"
#include "TargetDetectionComponent.generated.h"

// 前置声明
class USphereComponent;

// 目标更新事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetsUpdated, const TArray<AActor*>&, UpdatedTargets);

// 有效目标发现事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnValidTargetFound, AActor*, Target, EEnemySizeCategory, SizeCategory);

/**
 * 独立的目标检测组件
 * 负责处理所有目标搜索、验证、排序和尺寸分析功能
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UTargetDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetDetectionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== 事件委托 ====================
	/** 目标列表更新时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Target Detection Events")
	FOnTargetsUpdated OnTargetsUpdated;

	/** 发现有效目标时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Target Detection Events")
	FOnValidTargetFound OnValidTargetFound;

	// ==================== 配置参数 ====================
	/** 锁定系统配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On Settings")
	FLockOnSettings LockOnSettings;

	/** 相机系统配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FCameraSettings CameraSettings;

	/** 高级相机配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Camera Settings")
	FAdvancedCameraSettings AdvancedCameraSettings;

	/** Socket投射配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Projection Settings")
	FSocketProjectionSettings SocketProjectionSettings;

	// ==================== 检测球体组件引用 ====================
	/** 锁定检测球体组件（由拥有者传入） */
	UPROPERTY()
	USphereComponent* LockOnDetectionSphere;

	// ==================== 调试控制 ====================
	/** 是否启用目标检测调试日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableTargetDetectionDebugLogs;

	/** 是否启用尺寸分析调试日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableSizeAnalysisDebugLogs;

protected:
	// ==================== 内部状态变量 ====================
	/** 当前可锁定目标列表 */
	UPROPERTY()
	TArray<AActor*> LockOnCandidates;

	/** 敌人尺寸分类缓存 */
	UPROPERTY()
	TMap<AActor*, EEnemySizeCategory> EnemySizeCache;

	/** 上次目标搜索时间 */
	float LastTargetSearchTime;

	/** 上次尺寸更新时间 */
	float LastSizeUpdateTime;

	/** 性能优化：目标搜索间隔 */
	static constexpr float TARGET_SEARCH_INTERVAL = 0.2f;

	/** 性能优化：尺寸更新间隔 */
	static constexpr float SIZE_UPDATE_INTERVAL = 1.0f;

public:
	// ==================== 主要接口函数 ====================
	
	/** 设置检测球体组件引用 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	void SetLockOnDetectionSphere(USphereComponent* DetectionSphere);

	/** 获取当前可锁定目标列表 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	const TArray<AActor*>& GetLockOnCandidates() const { return LockOnCandidates; }

	/** 获取目标的尺寸分类 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	EEnemySizeCategory GetTargetSizeCategory(AActor* Target);

	// ==================== 从MyCharacter迁移的函数 ====================
	
	/** 查找可锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	void FindLockOnCandidates();

	/** 检查目标是否有效 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	bool IsValidLockOnTarget(AActor* Target);

	/** 检查目标是否仍然可以保持锁定 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	bool IsTargetStillLockable(AActor* Target);

	/** 从目标列表中获取最佳目标 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	AActor* GetBestTargetFromList(const TArray<AActor*>& TargetList);

	/** 获取扇形区域内的最佳锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	AActor* GetBestSectorLockTarget();

	/** 尝试获取扇形区域内的锁定目标 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	AActor* TryGetSectorLockTarget();

	/** 尝试获取需要相机修正的目标 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	AActor* TryGetCameraCorrectionTarget();

	/** 按方向角度排序候选目标 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	void SortCandidatesByDirection(TArray<AActor*>& Targets);

	/** 检查球体内是否有候选目标 */
	UFUNCTION(BlueprintCallable, Category = "Target Detection")
	bool HasCandidatesInSphere();

	// ==================== 新增的敌人尺寸分析功能 ====================
	
	/** 根据尺寸分类获取目标列表 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Size Analysis")
	TArray<AActor*> GetTargetsBySize(EEnemySizeCategory SizeCategory);

	/** 获取指定尺寸分类中最近的目标 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Size Analysis")
	AActor* GetNearestTargetBySize(EEnemySizeCategory SizeCategory);

	/** 获取所有尺寸分类统计信息 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Size Analysis")
	TMap<EEnemySizeCategory, int32> GetSizeCategoryStatistics();

	/** 强制更新指定目标的尺寸分类 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Size Analysis")
	void UpdateTargetSizeCategory(AActor* Target);

	/** 清理无效目标的尺寸缓存 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Size Analysis")
	void CleanupSizeCache();

protected:
	// ==================== 内部辅助函数 ====================
	
	/** 计算目标的边界盒尺寸 */
	float CalculateTargetBoundingBoxSize(AActor* Target) const;

	/** 分析目标的尺寸分类 */
	EEnemySizeCategory AnalyzeTargetSize(AActor* Target);

	/** 检查目标是否在扇形锁定区域内 */
	bool IsTargetInSectorLockZone(AActor* Target) const;

	/** 检查目标是否在边缘检测区域内 */
	bool IsTargetInEdgeDetectionZone(AActor* Target) const;

	/** 计算到目标的角度差异 */
	float CalculateAngleToTarget(AActor* Target) const;

	/** 计算目标相对于玩家的方向角度 */
	float CalculateDirectionAngle(AActor* Target) const;

	/** 更新敌人尺寸缓存 */
	void UpdateEnemySizeCache();

	/** 获取拥有者角色 */
	class ACharacter* GetOwnerCharacter() const;

	/** 获取拥有者的控制器 */
	class AController* GetOwnerController() const;

private:
	// ==================== 私有辅助函数 ====================
	
	/** 内部函数：执行射线检测 */
	bool PerformLineOfSightCheck(AActor* Target) const;

	/** 内部函数：计算目标评分 */
	float CalculateTargetScore(AActor* Target) const;

	/** 内部函数：验证目标的基本条件 */
	bool ValidateBasicTargetConditions(AActor* Target) const;
};