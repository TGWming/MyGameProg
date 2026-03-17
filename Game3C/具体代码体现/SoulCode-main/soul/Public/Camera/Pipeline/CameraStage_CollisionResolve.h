// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "Camera/Data/CameraStateEnums.h"
#include "CameraStage_CollisionResolve.generated.h"

// Forward declarations
class UCameraCollisionResolver;

/**
 * UCameraStage_CollisionResolve - Stage 6: Collision Resolve
 * 阶段6：碰撞解算
 * 
 * Sixth stage in the pipeline. Handles camera collision:
 * Pipeline第六阶段，处理相机碰撞：
 * 
 * - Detection (D01-D04): SingleRay, SphereSweep, MultiRay, MultiSphereSweep 检测策略
 * - Response (RS01-RS05): PullIn, Slide, Orbit, FOVCompensate, InstantSnap 响应策略
 * - Occlusion (OC01-OC04): FadeOut, Hide, PullInFurther, DitherFade 遮挡策略
 * - Recovery (RC01-RC03): DelayedRecovery, SmoothRecovery, StepRecovery 恢复策略
 * - Special (SP01-SP04): TightSpace, LowCeiling, CliffEdge, CornerCase 特殊策略
 * 
 * Layer: SAFETY (安全层)
 * Required: No (非必需)
 * Skippable: Yes (可跳过 - 当状态禁用碰撞时)
 * 
 * Note: This is a skeleton implementation. Full collision system will be implemented in Phase 5.
 * 注意：这是骨架实现，完整碰撞系统将在Phase 5实现。
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_CollisionResolve : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_CollisionResolve();

    //========================================
    // ICameraStage Interface 接口实现
    //========================================

    /** Get stage index (6 for CollisionResolve) */
    virtual int32 GetStageIndex() const override { return 6; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("CollisionResolve")); }
    
    /** This stage CAN be skipped */
    virtual bool CanBeSkipped() const override { return true; }
    
    /** Check if this stage should execute (skip if collision disabled for current state) */
    virtual bool ShouldExecute(const FStageExecutionContext& Context) const override;
    
    /** Execute the collision resolve stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

    //========================================
    // Collision Resolver Reference 碰撞解算器引用
    //========================================

    /** 
     * Set reference to collision resolver
     * @param InResolver Collision resolver instance
     */
    void SetCollisionResolver(UCameraCollisionResolver* InResolver);

    /**
     * Get the collision resolver reference
     * @return Collision resolver instance, or nullptr if not set
     */
    FORCEINLINE UCameraCollisionResolver* GetCollisionResolver() const { return CollisionResolver; }

    /**
     * Check if collision is currently active (camera pulled in)
     * @return True if collision is active
     */
    FORCEINLINE bool IsCollisionActive() const { return bCollisionActive; }

protected:
    //========================================
    // Collision Detection 碰撞检测
    //========================================

    /** 
     * Perform collision detection using current strategy
     * 使用当前策略执行碰撞检测
     * @param Context Execution context
     * @param OutHitResult Output hit result
     * @return True if collision detected
     */
    bool DetectCollision(FStageExecutionContext& Context, FHitResult& OutHitResult);

    /**
     * Perform single ray collision detection (D01)
     * 单射线碰撞检测
     */
    bool DetectCollision_SingleRay(const FStageExecutionContext& Context, FHitResult& OutHitResult);

    /**
     * Perform sphere sweep collision detection (D02)
     * 球体扫描碰撞检测
     */
    bool DetectCollision_SphereSweep(const FStageExecutionContext& Context, FHitResult& OutHitResult);

    //========================================
    // Collision Response 碰撞响应
    //========================================

    /** 
     * Apply collision response based on strategy
     * 根据策略应用碰撞响应
     * @param Context Execution context
     * @param HitResult Collision hit result
     */
    void ApplyCollisionResponse(FStageExecutionContext& Context, const FHitResult& HitResult);

    /**
     * Apply pull-in response (RS01)
     * 应用拉近响应
     */
    void ApplyResponse_PullIn(FStageExecutionContext& Context, const FHitResult& HitResult);

    /**
     * Apply FOV compensation response (RS04)
     * 应用FOV补偿响应
     */
    void ApplyResponse_FOVCompensate(FStageExecutionContext& Context, float DistanceReduction);

    //========================================
    // Collision Recovery 碰撞恢复
    //========================================

    /**
     * Update collision recovery (when no longer colliding)
     * 更新碰撞恢复
     */
    void UpdateCollisionRecovery(FStageExecutionContext& Context);

    /**
     * Apply smooth recovery (RC02)
     * 应用平滑恢复
     */
    void ApplyRecovery_Smooth(FStageExecutionContext& Context);

    //========================================
    // Occlusion Handling 遮挡处理
    //========================================

    /**
     * Handle character occlusion
     * 处理角色遮挡
     */
    void HandleCharacterOcclusion(FStageExecutionContext& Context, const FHitResult& HitResult);

    //========================================
    // Placeholder Methods (Phase 5) 占位方法
    //========================================

    /**
     * Placeholder: Get current detection strategy from state config
     * 占位：从状态配置获取当前检测策略
     */
    ECollisionStrategyID GetCurrentDetectionStrategy(const FStageExecutionContext& Context) const;

    /**
     * Placeholder: Get current response strategy from state config
     * 占位：从状态配置获取当前响应策略
     */
    ECollisionStrategyID GetCurrentResponseStrategy(const FStageExecutionContext& Context) const;

    /**
     * Placeholder: Get current recovery strategy from state config
     * 占位：从状态配置获取当前恢复策略
     */
    ECollisionStrategyID GetCurrentRecoveryStrategy(const FStageExecutionContext& Context) const;

private:
    //========================================
    // Collision Resolver 碰撞解算器
    //========================================

    /** 
     * Reference to collision resolver (will be set in Phase 5) 
     * Note: Not a UPROPERTY because UCameraCollisionResolver is not yet defined
     * 注意：非UPROPERTY，因为UCameraCollisionResolver尚未定义
     */
    UCameraCollisionResolver* CollisionResolver;

    //========================================
    // Collision State 碰撞状态
    //========================================

    /** Is collision currently active (camera pulled in) */
    bool bCollisionActive;

    /** Was collision active last frame */
    bool bWasCollisionActive;

    /** Time since collision ended (for recovery delay) */
    float TimeSinceCollisionEnd;

    /** Current collision distance (0 = no collision, 1 = fully pulled in) */
    float CurrentCollisionRatio;

    /** Target collision ratio (for smooth transitions) */
    float TargetCollisionRatio;

    /** Original distance before collision adjustment */
    float OriginalDistance;

    //========================================
    // Execution Statistics 执行统计
    //========================================

    /** Number of collision traces performed this frame */
    int32 TracesPerformed;

    /** Total collision stage time (ms) for profiling */
    float TotalCollisionTimeMs;

    //========================================
    // Configuration 配置
    //========================================

    /** Default collision probe radius for sphere sweep */
    float DefaultProbeRadius;

    /** Default recovery delay (seconds) */
    float DefaultRecoveryDelay;

    /** Default recovery speed (units per second) */
    float DefaultRecoverySpeed;
};
