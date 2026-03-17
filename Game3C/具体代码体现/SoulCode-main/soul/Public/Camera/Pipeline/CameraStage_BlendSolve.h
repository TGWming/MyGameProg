// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "CameraStage_BlendSolve.generated.h"

/**
 * UCameraStage_BlendSolve - Stage 5: Blend & Solve
 * 阶段5：混合与求解
 * 
 * Fifth stage in the pipeline. Combines all module and modifier outputs:
 * Pipeline第五阶段，合并所有模块和修改器输出：
 * 
 * - Sorts outputs by priority 按优先级排序输出
 * - Applies blend policies (Override, Additive, Multiplicative, Blend, Min, Max) 应用混合策略
 * - Resolves conflicts between competing outputs 解决竞争输出之间的冲突
 * - Produces final FSoulsCameraOutput for collision stage 生成最终相机输出
 * 
 * Layer: COMPUTE (计算层) - Last stage in compute layer
 * Required: Yes (必需)
 * Skippable: No (不可跳过)
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_BlendSolve : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_BlendSolve();

    //========================================
    // ICameraStage Interface 接口实现
    //========================================

    /** Get stage index (5 for BlendSolve) */
    virtual int32 GetStageIndex() const override { return 5; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("BlendSolve")); }
    
    /** This stage cannot be skipped */
    virtual bool CanBeSkipped() const override { return false; }
    
    /** Execute the blend and solve stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

protected:
    //========================================
    // Sorting 排序
    //========================================

    /** 
     * Sort module outputs by priority (higher priority first)
     * 按优先级排序模块输出（高优先级优先）
     */
    void SortOutputsByPriority(TArray<FModuleOutput>& Outputs);

    /**
     * Sort modifier outputs by priority
     * 按优先级排序修改器输出
     */
    void SortModifiersByPriority(TArray<FModifierOutput>& Outputs);

    //========================================
    // Module Blending 模块混合
    //========================================

    /** 
     * Blend all module outputs into final values
     * 混合所有模块输出到最终值
     */
    void BlendModuleOutputs(FStageExecutionContext& Context);

    /** 
     * Blend position outputs from modules
     * 混合模块的位置输出
     */
    FVector BlendPositions(const TArray<FModuleOutput>& Outputs, const FVector& BasePosition);

    /** 
     * Blend rotation outputs from modules
     * 混合模块的旋转输出
     */
    FRotator BlendRotations(const TArray<FModuleOutput>& Outputs, const FRotator& BaseRotation);

    /** 
     * Blend distance outputs from modules
     * 混合模块的距离输出
     */
    float BlendDistances(const TArray<FModuleOutput>& Outputs, float BaseDistance);

    /** 
     * Blend FOV outputs from modules
     * 混合模块的FOV输出
     */
    float BlendFOVs(const TArray<FModuleOutput>& Outputs, float BaseFOV);

    /**
     * Blend socket offset outputs from modules
     * 混合模块的插槽偏移输出
     */
    FVector BlendSocketOffsets(const TArray<FModuleOutput>& Outputs, const FVector& BaseOffset);

    //========================================
    // Modifier Application 修改器应用
    //========================================

    /** 
     * Apply modifier effects on top of module results
     * 在模块结果之上应用修改器效果
     */
    void ApplyModifierOutputs(FStageExecutionContext& Context);

    /**
     * Apply a single modifier's effects to the output
     * 应用单个修改器的效果到输出
     */
    void ApplySingleModifier(FSoulsCameraOutput& Output, const FModifierOutput& Modifier);

    //========================================
    // Blend Policy Implementation 混合策略实现
    //========================================

    /** 
     * Apply single float value with blend policy
     * 应用混合策略到单个浮点值
     */
    float ApplyBlendPolicy(float CurrentValue, float NewValue, EBlendPolicy Policy, float Weight);

    /** 
     * Apply vector with blend policy
     * 应用混合策略到向量
     */
    FVector ApplyBlendPolicyVector(const FVector& CurrentValue, const FVector& NewValue, EBlendPolicy Policy, float Weight);

    /**
     * Apply rotator with blend policy
     * 应用混合策略到旋转
     */
    FRotator ApplyBlendPolicyRotator(const FRotator& CurrentValue, const FRotator& NewValue, EBlendPolicy Policy, float Weight);

    //========================================
    // Conflict Resolution 冲突解决
    //========================================

    /**
     * Resolve conflicts when multiple outputs want Override
     * 解决多个输出都要求Override时的冲突
     */
    void ResolveOverrideConflicts(TArray<FModuleOutput>& Outputs);

    /**
     * Check if a modifier should completely override camera
     * 检查修改器是否应该完全覆盖相机
     */
    bool ShouldModifierOverride(const FModifierOutput& Modifier) const;

private:
    //========================================
    // Execution Statistics 执行统计
    //========================================

    /** Number of module outputs processed */
    int32 ModuleOutputsProcessed;

    /** Number of modifier outputs processed */
    int32 ModifierOutputsProcessed;

    /** Number of override conflicts resolved */
    int32 OverrideConflictsResolved;

    /** Total blend time (ms) for profiling */
    float TotalBlendTimeMs;

    // ★ Debug LOG 开关已移至 SoulsCameraManager 统一管理
    // 使用 Context.Manager->IsBlendSolveDebugEnabled() 访问
};
