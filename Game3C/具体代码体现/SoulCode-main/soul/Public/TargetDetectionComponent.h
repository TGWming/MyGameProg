// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnConfig.h"
#include "TargetDetectionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetsUpdated, const TArray<AActor*>&, UpdatedTargets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnValidTargetFound, AActor*, Target, EEnemySizeCategory, SizeCategory);

/**
 * Target detection component for lock-on system
 * Handles finding, validating, and managing lockable targets
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UTargetDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetDetectionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== Properties ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float LockOnRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float LockOnAngle = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float SectorLockAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float EdgeDetectionAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float ExtendedLockRangeMultiplier = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float TargetSearchInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float RaycastHeightOffset = 50.0f;

	// ========== 垂直锁定扩展 ==========

	/** 是否启用垂直角度检测（高位/低位敌人支持） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Vertical")
	bool bEnableVerticalDetection = true;

	/** 垂直方向最大检测角度（度），从水平面算起，上下各这么多度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Vertical", meta = (EditCondition = "bEnableVerticalDetection", ClampMin = "0", ClampMax = "85"))
	float VerticalDetectionAngle = 45.0f;

	/** 视线检测是否使用目标中心而非固定偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|Vertical")
	bool bUseTargetCenterForLineOfSight = true;

	// ========== 大型敌人支持 ==========

	/** 大型敌人使用 Bounds 中心计算距离和方向（而非 ActorLocation） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|LargeEnemy")
	bool bUseBoundsCenterForLargeEnemies = true;

	/** 标记为大型敌人的 Actor Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On|LargeEnemy")
	FName LargeEnemyTag = FName("LargeEnemy");

	// ========== State ==========

	UPROPERTY(BlueprintReadOnly, Category = "Detection")
	TArray<AActor*> LockOnCandidates;

	// ========== Delegates ==========

	UPROPERTY(BlueprintAssignable, Category = "Lock-On Events")
	FOnTargetsUpdated OnTargetsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Lock-On Events")
	FOnValidTargetFound OnValidTargetFound;

	// ========== Public Functions ==========

	/**
	 * Find all valid lock-on candidates within range
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void FindLockOnCandidates();

	/**
	 * Check if a target is valid for lock-on
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	bool IsValidLockOnTarget(AActor* Target) const;

	/**
	 * Check if a currently locked target is still valid (more lenient)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	bool IsTargetStillLockable(AActor* Target) const;

	/**
	 * Try to get the best target in the sector lock zone (卤30 degrees from camera)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	AActor* TryGetSectorLockTarget();

	/**
	 * Get the best target from a list based on angle and distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	AActor* GetBestTargetFromList(const TArray<AActor*>& Targets);

	/**
	 * Sort candidates from left to right relative to camera
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void SortCandidatesByDirection(TArray<AActor*>& Targets);

	/**
	 * Calculate angle to target (0-180 degrees)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	float CalculateAngleToTarget(AActor* Target) const;

	/**
	 * Calculate directional angle to target (-180 to +180, negative=left, positive=right)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	float CalculateDirectionAngle(AActor* Target) const;

	/**
	 * Get target size category
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	EEnemySizeCategory GetTargetSizeCategory(AActor* Target);

	/**
	 * Set the detection sphere reference
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void SetDetectionSphere(class USphereComponent* Sphere);

private:
	// ========== Private State ==========

	TMap<AActor*, EEnemySizeCategory> EnemySizeCache;
	
	UPROPERTY()
	class USphereComponent* LockOnDetectionSphere;

	float LastTargetSearchTime = 0.0f;

	// ========== Private Functions ==========

	/**
	 * Perform line of sight check to target
	 */
	bool PerformLineOfSightCheck(AActor* Target) const;

	/**
	 * Get camera forward vector (horizontal projection)
	 */
	FVector GetCameraForwardVector() const;

	/** 获取相机前向向量（可选是否包含垂直分量） */
	FVector GetCameraForwardVector(bool bIncludeVertical) const;

	/**
	 * Get camera component from player controller
	 */
	class UCameraComponent* GetPlayerCamera() const;

	/** 获取目标的有效检测位置（大型敌人用 Bounds 中心，普通敌人用 ActorLocation） */
	FVector GetTargetDetectionLocation(AActor* Target) const;

	/** 获取玩家到目标最近 Bounds 表面点的距离（大型敌人用） */
	float GetDistanceToTargetBoundsEdge(AActor* Target) const;

	/** 补充搜索：通过 Bounds 范围检测大型敌人（Overlap 检测不到的） */
	void FindLargeEnemyCandidates(TArray<AActor*>& OutCandidates);
};
