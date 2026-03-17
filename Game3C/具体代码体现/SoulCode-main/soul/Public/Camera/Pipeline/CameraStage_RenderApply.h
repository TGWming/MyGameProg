// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "CameraStage_RenderApply.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;

/**
 * UCameraStage_RenderApply - Stage 8: Render Apply
 * 阶段8：渲染应用
 * 
 * Final stage in the pipeline. Applies output to UE4 components:
 * Pipeline最后阶段，将输出应用到UE4组件：
 * 
 * - SpringArmComponent: TargetArmLength, SocketOffset, TargetOffset, Rotation, RelativeLocation
 * - CameraComponent: FieldOfView, PostProcessSettings, PostProcessBlendWeight
 * 
 * This is where our calculated values become actual camera properties.
 * 这是计算结果变为实际相机属性的地方。
 * 
 * Layer: OUTPUT (输出层) - Final stage
 * Required: Yes (必需)
 * Skippable: No (不可跳过)
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_RenderApply : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_RenderApply();

    //========================================
    // ICameraStage Interface 接口实现
    //========================================

    /** Get stage index (8 for RenderApply - final stage) */
    virtual int32 GetStageIndex() const override { return 8; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("RenderApply")); }
    
    /** This stage cannot be skipped (required for actual camera update) */
    virtual bool CanBeSkipped() const override { return false; }
    
    /** Execute the render apply stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

protected:
    //========================================
    // SpringArm Application 弹簧臂应用
    //========================================

    /** 
     * Apply output to SpringArm component
     * 将输出应用到弹簧臂组件
     */
    void ApplyToSpringArm(FStageExecutionContext& Context, USpringArmComponent* SpringArm);

    /**
     * Apply arm length (distance)
     * 应用臂长（距离）
     */
    void ApplyArmLength(USpringArmComponent* SpringArm, float Distance);

    /**
     * Apply socket offset
     * 应用插槽偏移
     */
    void ApplySocketOffset(USpringArmComponent* SpringArm, const FVector& Offset);

    /**
     * Apply target offset
     * 应用目标偏移
     */
    void ApplyTargetOffset(USpringArmComponent* SpringArm, const FVector& Offset);

    /**
     * Apply rotation to spring arm
     * 应用旋转到弹簧臂
     */
    void ApplySpringArmRotation(USpringArmComponent* SpringArm, const FRotator& Rotation);

    //========================================
    // Camera Application 相机应用
    //========================================

    /** 
     * Apply output to Camera component
     * 将输出应用到相机组件
     */
    void ApplyToCamera(FStageExecutionContext& Context, UCameraComponent* Camera);

    /**
     * Apply field of view
     * 应用视野角度
     */
    void ApplyFieldOfView(UCameraComponent* Camera, float FOV);

    /** 
     * Apply post-process settings
     * 应用后处理设置
     */
    void ApplyPostProcessSettings(FStageExecutionContext& Context, UCameraComponent* Camera);

    /**
     * Apply vignette effect to post-process
     * 应用暗角效果到后处理
     */
    void ApplyVignetteToPostProcess(UCameraComponent* Camera, float Intensity);

    /**
     * Apply color tint to post-process
     * 应用颜色色调到后处理
     */
    void ApplyColorTintToPostProcess(UCameraComponent* Camera, const FLinearColor& ColorTint);

    /**
     * Apply DOF settings to post-process
     * 应用景深设置到后处理
     */
    void ApplyDOFToPostProcess(UCameraComponent* Camera, float FocalDistance, float FocalRadius, float NearBlur, float FarBlur);

    //========================================
    // Time Dilation 时间膨胀
    //========================================

    /**
     * Apply time dilation effect
     * 应用时间膨胀效果
     */
    void ApplyTimeDilation(FStageExecutionContext& Context);

    //========================================
    // Validation 验证
    //========================================

    /**
     * Get SpringArm component from manager
     * 从管理器获取弹簧臂组件
     */
    USpringArmComponent* GetSpringArm(const FStageExecutionContext& Context) const;

    /**
     * Get Camera component from manager
     * 从管理器获取相机组件
     */
    UCameraComponent* GetCamera(const FStageExecutionContext& Context) const;

private:
    //========================================
    // Previous Frame State 上一帧状态
    //========================================

    /** Previous frame's applied distance (for delta tracking) */
    float PreviousAppliedDistance;

    /** Previous frame's applied FOV */
    float PreviousAppliedFOV;

    /** Previous frame's applied rotation */
    FRotator PreviousAppliedRotation;

    /** Is this the first frame */
    bool bFirstFrame;

    //========================================
    // Execution Statistics 执行统计
    //========================================

    /** Total apply time (ms) for profiling */
    float TotalApplyTimeMs;

    /** Were post-process settings applied this frame */
    bool bPostProcessApplied;

    /** Was time dilation applied this frame */
    bool bTimeDilationApplied;
};
