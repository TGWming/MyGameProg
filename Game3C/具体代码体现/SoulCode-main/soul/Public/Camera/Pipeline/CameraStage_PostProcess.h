// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "CameraStage_PostProcess.generated.h"

// Forward declarations
class USoulsCameraGlobalConfig;

/**
 * UCameraStage_PostProcess - Stage 7: Post Process
 * 阶段7：后处理
 * 
 * Seventh stage in the pipeline. Applies final adjustments:
 * Pipeline第七阶段，应用最终调整：
 * 
 * - Global limits (min/max distance, FOV, pitch) 全局限制
 * - Final smoothing pass 最终平滑
 * - Post-process effect settings 后处理效果设置
 * - Rotation normalization 旋转归一化
 * - Any last-minute corrections 最终校正
 * 
 * Layer: OUTPUT (输出层)
 * Required: No (非必需)
 * Skippable: Yes (可跳过)
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_PostProcess : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_PostProcess();

    //========================================
    // ICameraStage Interface 接口实现
    //========================================

    /** Get stage index (7 for PostProcess) */
    virtual int32 GetStageIndex() const override { return 7; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("PostProcess")); }
    
    /** This stage CAN be skipped */
    virtual bool CanBeSkipped() const override { return true; }
    
    /** Check if this stage should execute */
    virtual bool ShouldExecute(const FStageExecutionContext& Context) const override;
    
    /** Execute the post process stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

    //========================================
    // Configuration 配置
    //========================================

    /**
     * Set reference to global config
     * @param InConfig Global config instance
     */
    void SetGlobalConfig(USoulsCameraGlobalConfig* InConfig);

    /**
     * Get global config reference
     * @return Global config instance, or nullptr if not set
     */
    FORCEINLINE USoulsCameraGlobalConfig* GetGlobalConfig() const { return GlobalConfig; }

protected:
    //========================================
    // Global Limits 全局限制
    //========================================

    /** 
     * Apply global limits to output values
     * 应用全局限制到输出值
     */
    void ApplyGlobalLimits(FStageExecutionContext& Context);

    /**
     * Clamp distance to global min/max
     * 限制距离到全局最小/最大值
     */
    float ClampDistance(float Distance) const;

    /**
     * Clamp FOV to global min/max
     * 限制FOV到全局最小/最大值
     */
    float ClampFOV(float FOV) const;

    /**
     * Clamp pitch to global min/max
     * 限制俯仰角到全局最小/最大值
     */
    float ClampPitch(float Pitch) const;

    //========================================
    // Smoothing 平滑
    //========================================

    /** 
     * Apply final smoothing pass
     * 应用最终平滑
     */
    void ApplyFinalSmoothing(FStageExecutionContext& Context);

    /**
     * Smooth position between frames
     * 帧间位置平滑
     */
    FVector SmoothPosition(const FVector& Current, const FVector& Previous, float DeltaTime);

    /**
     * Smooth rotation between frames
     * 帧间旋转平滑
     */
    FRotator SmoothRotation(const FRotator& Current, const FRotator& Previous, float DeltaTime);

    //========================================
    // Post-Process Effects 后处理效果
    //========================================

    /** 
     * Build post-process settings based on output
     * 基于输出构建后处理设置
     */
    void BuildPostProcessSettings(FStageExecutionContext& Context);

    /**
     * Apply vignette effect
     * 应用暗角效果
     */
    void ApplyVignetteEffect(FStageExecutionContext& Context);

    /**
     * Apply color tint effect
     * 应用颜色色调效果
     */
    void ApplyColorTintEffect(FStageExecutionContext& Context);

    //========================================
    // Normalization 归一化
    //========================================

    /**
     * Normalize rotation values (wrap angles)
     * 归一化旋转值（角度回环）
     */
    void NormalizeRotation(FStageExecutionContext& Context);

    /**
     * Validate and fix any invalid output values
     * 验证并修复任何无效输出值
     */
    void ValidateOutput(FStageExecutionContext& Context);

private:
    //========================================
    // Global Config Reference
    //========================================

    /** Reference to global config (for limits and settings) */
    UPROPERTY()
    USoulsCameraGlobalConfig* GlobalConfig;

    //========================================
    // Smoothing State 平滑状态
    //========================================

    /** Previous frame output for smoothing */
    FSoulsCameraOutput PreviousOutput;

    /** Is first frame (no previous data) */
    bool bFirstFrame;

    /** Smoothing speed for position */
    float PositionSmoothSpeed;

    /** Smoothing speed for rotation */
    float RotationSmoothSpeed;

    //========================================
    // Default Limits 默认限制
    //========================================

    /** Global minimum distance (cm) */
    UPROPERTY(EditAnywhere, Category = "Limits")
    float GlobalMinDistance;

    /** Global maximum distance (cm) */
    UPROPERTY(EditAnywhere, Category = "Limits")
    float GlobalMaxDistance;

    /** Global minimum FOV (degrees) */
    UPROPERTY(EditAnywhere, Category = "Limits")
    float GlobalMinFOV;

    /** Global maximum FOV (degrees) */
    UPROPERTY(EditAnywhere, Category = "Limits")
    float GlobalMaxFOV;

    /** Global minimum pitch (degrees, negative = look down) */
    UPROPERTY(EditAnywhere, Category = "Limits")
    float GlobalMinPitch;

    /** Global maximum pitch (degrees, positive = look up) */
    UPROPERTY(EditAnywhere, Category = "Limits")
    float GlobalMaxPitch;

    //========================================
    // Execution Statistics 执行统计
    //========================================

    /** Total post-process time (ms) for profiling */
    float TotalPostProcessTimeMs;

    /** Number of values clamped this frame */
    int32 ValuesClampedCount;
};
