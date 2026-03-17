// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_PostProcess.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Core/SoulsCameraGlobalConfig.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_PostProcess, Log, All);

//========================================
// Constructor 构造函数
//========================================

UCameraStage_PostProcess::UCameraStage_PostProcess()
    : GlobalConfig(nullptr)
    , bFirstFrame(true)
    , PositionSmoothSpeed(20.0f)
    , RotationSmoothSpeed(15.0f)
    , GlobalMinDistance(100.0f)  // ★★★ 修复：从50改为100，防止相机过近 ★★★
    , GlobalMaxDistance(1500.0f)
    , GlobalMinFOV(40.0f)
    , GlobalMaxFOV(120.0f)
    , GlobalMinPitch(-80.0f)
    , GlobalMaxPitch(80.0f)
    , TotalPostProcessTimeMs(0.0f)
    , ValuesClampedCount(0)
{
}

//========================================
// ICameraStage Interface 接口实现
//========================================

bool UCameraStage_PostProcess::ShouldExecute(const FStageExecutionContext& Context) const
{
    // Post-process stage is generally useful, so execute by default
    // Could be skipped in performance-critical scenarios
    
    UE_LOG(LogCameraStage_PostProcess, VeryVerbose, TEXT("ShouldExecute: true"));
    return true;
}

EStageResult UCameraStage_PostProcess::Execute(FStageExecutionContext& Context)
{
    // ★★★ 诊断日志开始 ★★★
    static int32 Stage7DiagCount = 0;
    bool bShouldLog = (Stage7DiagCount < 10);
    
    float DistanceAtStart = Context.Output.Distance;
    float FOVAtStart = Context.Output.FOV;
    float PitchAtStart = Context.Output.Rotation.Pitch;
    
    if (bShouldLog)
    {
        Stage7DiagCount++;
        UE_LOG(LogTemp, Error, TEXT(""));
        UE_LOG(LogTemp, Error, TEXT("╔══════════════════════════════════════════════════════════════╗"));
        UE_LOG(LogTemp, Error, TEXT("║  Stage7 PostProcess 诊断 #%d                                 ║"), Stage7DiagCount);
        UE_LOG(LogTemp, Error, TEXT("╚══════════════════════════════════════════════════════════════╝"));
        UE_LOG(LogTemp, Error, TEXT("【输入】"));
        UE_LOG(LogTemp, Error, TEXT("   Context.Output.Distance: %.1f"), DistanceAtStart);
        UE_LOG(LogTemp, Error, TEXT("   Context.Output.FOV: %.1f"), FOVAtStart);
        UE_LOG(LogTemp, Error, TEXT("   Context.Output.Rotation.Pitch: %.1f"), PitchAtStart);
        UE_LOG(LogTemp, Error, TEXT("   bFirstFrame: %s"), bFirstFrame ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Error, TEXT("   PreviousOutput.Distance: %.1f"), PreviousOutput.Distance);
        UE_LOG(LogTemp, Error, TEXT("【限制参数】"));
        UE_LOG(LogTemp, Error, TEXT("   GlobalMinDistance: %.1f"), GlobalMinDistance);
        UE_LOG(LogTemp, Error, TEXT("   GlobalMaxDistance: %.1f"), GlobalMaxDistance);
    }
    // ★★★ 诊断日志结束 ★★★

    // Reset statistics
    ValuesClampedCount = 0;
    const double StartTime = FPlatformTime::Seconds();

    // Step 1: Apply global limits
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【Step 1: ApplyGlobalLimits】"));
        UE_LOG(LogTemp, Error, TEXT("   调用前 Distance: %.1f"), Context.Output.Distance);
    }
    
    ApplyGlobalLimits(Context);
    
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("   调用后 Distance: %.1f (变化: %.1f)"), 
            Context.Output.Distance, Context.Output.Distance - DistanceAtStart);
    }

    // Step 2: Apply final smoothing (skip on first frame)
    if (!bFirstFrame)
    {
        if (bShouldLog)
        {
            UE_LOG(LogTemp, Error, TEXT("【Step 2: ApplyFinalSmoothing】"));
            UE_LOG(LogTemp, Error, TEXT("   调用前 Distance: %.1f"), Context.Output.Distance);
            UE_LOG(LogTemp, Error, TEXT("   PreviousOutput.Distance: %.1f"), PreviousOutput.Distance);
            UE_LOG(LogTemp, Error, TEXT("   PositionSmoothSpeed: %.1f"), PositionSmoothSpeed);
        }
        
        float DistanceBeforeSmoothing = Context.Output.Distance;
        ApplyFinalSmoothing(Context);
        
        if (bShouldLog)
        {
            UE_LOG(LogTemp, Error, TEXT("   调用后 Distance: %.1f (变化: %.1f)"), 
                Context.Output.Distance, Context.Output.Distance - DistanceBeforeSmoothing);
            
            if (FMath::Abs(Context.Output.Distance - DistanceBeforeSmoothing) > 1.0f)
            {
                UE_LOG(LogTemp, Error, TEXT("   ⚠️ 平滑阶段修改了距离！"));
            }
        }
    }
    else
    {
        if (bShouldLog)
        {
            UE_LOG(LogTemp, Error, TEXT("【Step 2: ApplyFinalSmoothing - 跳过（首帧）】"));
        }
    }

    // Step 3: Build post-process settings
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【Step 3: BuildPostProcessSettings】"));
        float DistanceBefore = Context.Output.Distance;
        BuildPostProcessSettings(Context);
        UE_LOG(LogTemp, Error, TEXT("   Distance 变化: %.1f -> %.1f"), DistanceBefore, Context.Output.Distance);
    }
    else
    {
        BuildPostProcessSettings(Context);
    }

    // Step 4: Normalize and validate output
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【Step 4: NormalizeRotation + ValidateOutput】"));
        float DistanceBefore = Context.Output.Distance;
        NormalizeRotation(Context);
        UE_LOG(LogTemp, Error, TEXT("   NormalizeRotation 后 Distance: %.1f"), Context.Output.Distance);
        ValidateOutput(Context);
        UE_LOG(LogTemp, Error, TEXT("   ValidateOutput 后 Distance: %.1f (变化: %.1f)"), 
            Context.Output.Distance, Context.Output.Distance - DistanceBefore);
    }
    else
    {
        NormalizeRotation(Context);
        ValidateOutput(Context);
    }

    // Store for next frame's smoothing
    PreviousOutput = Context.Output;
    bFirstFrame = false;

    TotalPostProcessTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

    UE_LOG(LogCameraStage_PostProcess, Verbose, TEXT("Execute: %d values clamped, Time=%.2fms"),
        ValuesClampedCount, TotalPostProcessTimeMs);

    // ★★★ 最终诊断日志 ★★★
    if (bShouldLog)
    {
        float DistanceAtEnd = Context.Output.Distance;
        UE_LOG(LogTemp, Error, TEXT("【输出】"));
        UE_LOG(LogTemp, Error, TEXT("   Context.Output.Distance: %.1f"), DistanceAtEnd);
        UE_LOG(LogTemp, Error, TEXT("   总变化: %.1f -> %.1f (差异: %.1f)"), 
            DistanceAtStart, DistanceAtEnd, DistanceAtEnd - DistanceAtStart);
        UE_LOG(LogTemp, Error, TEXT("   ValuesClampedCount: %d"), ValuesClampedCount);
        
        if (FMath::Abs(DistanceAtStart - DistanceAtEnd) > 1.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("   ⚠️ Stage7 总共修改了距离 %.1f"), DistanceAtEnd - DistanceAtStart);
        }
        UE_LOG(LogTemp, Error, TEXT("════════════════════════════════════════════════════════════════"));
    }

    return EStageResult::Success;
}

void UCameraStage_PostProcess::OnPreExecute(const FStageExecutionContext& Context)
{
    UE_LOG(LogCameraStage_PostProcess, VeryVerbose, TEXT("Stage 7 PostProcess: PreExecute - Distance=%.1f, FOV=%.1f"),
        Context.Output.Distance, Context.Output.FOV);
}

void UCameraStage_PostProcess::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    UE_LOG(LogCameraStage_PostProcess, VeryVerbose, TEXT("Stage 7 PostProcess: PostExecute - Distance=%.1f, FOV=%.1f, Pitch=%.1f"),
        Context.Output.Distance, Context.Output.FOV, Context.Output.Rotation.Pitch);
}

//========================================
// Configuration 配置
//========================================

void UCameraStage_PostProcess::SetGlobalConfig(USoulsCameraGlobalConfig* InConfig)
{
    GlobalConfig = InConfig;
    
    if (GlobalConfig)
    {
        // Load limits from global config
        // TODO: Implement when GlobalConfig is fully available
        // GlobalMinDistance = GlobalConfig->GlobalMinDistance;
        // GlobalMaxDistance = GlobalConfig->GlobalMaxDistance;
        // etc...
        
        UE_LOG(LogCameraStage_PostProcess, Log, TEXT("Global config set successfully"));
    }
}

//========================================
// Global Limits 全局限制
//========================================

void UCameraStage_PostProcess::ApplyGlobalLimits(FStageExecutionContext& Context)
{
    // Clamp distance
    const float OriginalDistance = Context.Output.Distance;
    Context.Output.Distance = ClampDistance(Context.Output.Distance);
    if (!FMath::IsNearlyEqual(OriginalDistance, Context.Output.Distance))
    {
        ValuesClampedCount++;
        UE_LOG(LogCameraStage_PostProcess, VeryVerbose, TEXT("Distance clamped: %.1f -> %.1f"),
            OriginalDistance, Context.Output.Distance);
    }

    // Clamp FOV
    const float OriginalFOV = Context.Output.FOV;
    Context.Output.FOV = ClampFOV(Context.Output.FOV);
    if (!FMath::IsNearlyEqual(OriginalFOV, Context.Output.FOV))
    {
        ValuesClampedCount++;
        UE_LOG(LogCameraStage_PostProcess, VeryVerbose, TEXT("FOV clamped: %.1f -> %.1f"),
            OriginalFOV, Context.Output.FOV);
    }

    // Clamp pitch
    const float OriginalPitch = Context.Output.Rotation.Pitch;
    Context.Output.Rotation.Pitch = ClampPitch(Context.Output.Rotation.Pitch);
    if (!FMath::IsNearlyEqual(OriginalPitch, Context.Output.Rotation.Pitch))
    {
        ValuesClampedCount++;
        UE_LOG(LogCameraStage_PostProcess, VeryVerbose, TEXT("Pitch clamped: %.1f -> %.1f"),
            OriginalPitch, Context.Output.Rotation.Pitch);
    }
}

float UCameraStage_PostProcess::ClampDistance(float Distance) const
{
    return FMath::Clamp(Distance, GlobalMinDistance, GlobalMaxDistance);
}

float UCameraStage_PostProcess::ClampFOV(float FOV) const
{
    return FMath::Clamp(FOV, GlobalMinFOV, GlobalMaxFOV);
}

float UCameraStage_PostProcess::ClampPitch(float Pitch) const
{
    return FMath::Clamp(Pitch, GlobalMinPitch, GlobalMaxPitch);
}

//========================================
// Smoothing 平滑
//========================================

void UCameraStage_PostProcess::ApplyFinalSmoothing(FStageExecutionContext& Context)
{
    // Light smoothing for any remaining jitter
    // This is a final pass after all other processing
    
    Context.Output.FocusPoint = SmoothPosition(
        Context.Output.FocusPoint,
        PreviousOutput.FocusPoint,
        Context.DeltaTime
    );

    Context.Output.Rotation = SmoothRotation(
        Context.Output.Rotation,
        PreviousOutput.Rotation,
        Context.DeltaTime
    );

    // Smooth distance
    const float DistanceAlpha = FMath::Clamp(Context.DeltaTime * PositionSmoothSpeed, 0.0f, 1.0f);
    Context.Output.Distance = FMath::Lerp(PreviousOutput.Distance, Context.Output.Distance, DistanceAlpha);

    // Smooth FOV
    const float FOVAlpha = FMath::Clamp(Context.DeltaTime * PositionSmoothSpeed, 0.0f, 1.0f);
    Context.Output.FOV = FMath::Lerp(PreviousOutput.FOV, Context.Output.FOV, FOVAlpha);
}

FVector UCameraStage_PostProcess::SmoothPosition(const FVector& Current, const FVector& Previous, float DeltaTime)
{
    const float Alpha = FMath::Clamp(DeltaTime * PositionSmoothSpeed, 0.0f, 1.0f);
    return FMath::Lerp(Previous, Current, Alpha);
}

FRotator UCameraStage_PostProcess::SmoothRotation(const FRotator& Current, const FRotator& Previous, float DeltaTime)
{
    // Use quaternion slerp for better rotation interpolation
    const FQuat PrevQuat = Previous.Quaternion();
    const FQuat CurrQuat = Current.Quaternion();
    
    const float Alpha = FMath::Clamp(DeltaTime * RotationSmoothSpeed, 0.0f, 1.0f);
    const FQuat SmoothedQuat = FQuat::Slerp(PrevQuat, CurrQuat, Alpha);
    
    return SmoothedQuat.Rotator();
}

//========================================
// Post-Process Effects 后处理效果
//========================================

void UCameraStage_PostProcess::BuildPostProcessSettings(FStageExecutionContext& Context)
{
    // Apply and validate post-process effects
    ApplyVignetteEffect(Context);
    ApplyColorTintEffect(Context);

    // DOF focal distance defaults to target distance if not set
    if (Context.Output.DOFFocalDistance <= 0.0f && Context.InputContext.bHasTarget)
    {
        Context.Output.DOFFocalDistance = FMath::Max(Context.InputContext.TargetDistance, 300.0f);
    }

    // Time dilation clamping
    Context.Output.TimeDilation = FMath::Clamp(Context.Output.TimeDilation, 0.01f, 2.0f);
}

void UCameraStage_PostProcess::ApplyVignetteEffect(FStageExecutionContext& Context)
{
    // Clamp vignette intensity to valid range
    Context.Output.VignetteIntensity = FMath::Clamp(Context.Output.VignetteIntensity, 0.0f, 1.0f);
}

void UCameraStage_PostProcess::ApplyColorTintEffect(FStageExecutionContext& Context)
{
    // Ensure color tint alpha is valid
    Context.Output.ColorTint.A = 1.0f;
    
    // Clamp color components
    Context.Output.ColorTint.R = FMath::Clamp(Context.Output.ColorTint.R, 0.0f, 2.0f);
    Context.Output.ColorTint.G = FMath::Clamp(Context.Output.ColorTint.G, 0.0f, 2.0f);
    Context.Output.ColorTint.B = FMath::Clamp(Context.Output.ColorTint.B, 0.0f, 2.0f);
}

//========================================
// Normalization 归一化
//========================================

void UCameraStage_PostProcess::NormalizeRotation(FStageExecutionContext& Context)
{
    // Normalize rotation to prevent angle wraparound issues
    Context.Output.Rotation.Normalize();
    
    // Ensure yaw is in -180 to 180 range
    Context.Output.Rotation.Yaw = FMath::UnwindDegrees(Context.Output.Rotation.Yaw);
}

void UCameraStage_PostProcess::ValidateOutput(FStageExecutionContext& Context)
{
    // Check for NaN or invalid values and fix them
    
    // Position
    if (!Context.Output.FocusPoint.ContainsNaN())
    {
        // Position is valid
    }
    else
    {
        UE_LOG(LogCameraStage_PostProcess, Warning, TEXT("ValidateOutput: FocusPoint contains NaN, using fallback"));
        Context.Output.FocusPoint = Context.InputContext.CharacterLocation;
        ValuesClampedCount++;
    }

    // Rotation
    if (Context.Output.Rotation.ContainsNaN())
    {
        UE_LOG(LogCameraStage_PostProcess, Warning, TEXT("ValidateOutput: Rotation contains NaN, using fallback"));
        Context.Output.Rotation = Context.InputContext.CharacterRotation;
        ValuesClampedCount++;
    }

    // Distance
    // ★★★ 修复：使用合理的默认值400，而不是GlobalMinDistance(之前是50) ★★★
    if (!FMath::IsFinite(Context.Output.Distance) || Context.Output.Distance <= 0.0f)
    {
        UE_LOG(LogCameraStage_PostProcess, Warning, TEXT("ValidateOutput: Invalid distance %.1f, using fallback 400.0f"), Context.Output.Distance);
        Context.Output.Distance = 400.0f;  // 使用标准默认距离，而不是GlobalMinDistance
        ValuesClampedCount++;
    }

    // FOV
    if (!FMath::IsFinite(Context.Output.FOV) || Context.Output.FOV <= 0.0f)
    {
        UE_LOG(LogCameraStage_PostProcess, Warning, TEXT("ValidateOutput: Invalid FOV, using fallback"));
        Context.Output.FOV = 90.0f;
        ValuesClampedCount++;
    }
}
