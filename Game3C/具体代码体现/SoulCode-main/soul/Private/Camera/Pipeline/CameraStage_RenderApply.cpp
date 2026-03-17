// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_RenderApply.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_Render, Log, All);

//========================================
// Constructor 构造函数
//========================================

UCameraStage_RenderApply::UCameraStage_RenderApply()
    : PreviousAppliedDistance(0.0f)
    , PreviousAppliedFOV(90.0f)
    , PreviousAppliedRotation(FRotator::ZeroRotator)
    , bFirstFrame(true)
    , TotalApplyTimeMs(0.0f)
    , bPostProcessApplied(false)
    , bTimeDilationApplied(false)
{
}

//========================================
// ICameraStage Interface 接口实现
//========================================

EStageResult UCameraStage_RenderApply::Execute(FStageExecutionContext& Context)
{
    // 【诊断日志】记录Stage开始时的Distance
    float DistanceAtStart = Context.Output.Distance;

    if (!Context.Manager)
    {
        UE_LOG(LogCameraStage_Render, Warning, TEXT("Execute: No manager reference"));
        return EStageResult::Failed;
    }

    // Reset statistics
    bPostProcessApplied = false;
    bTimeDilationApplied = false;
    const double StartTime = FPlatformTime::Seconds();

    // Get component references
    USpringArmComponent* SpringArm = GetSpringArm(Context);
    UCameraComponent* Camera = GetCamera(Context);

    if (!SpringArm || !Camera)
    {
        UE_LOG(LogCameraStage_Render, Warning, TEXT("Execute: Missing SpringArm or Camera component"));
        return EStageResult::Failed;
    }

    // Apply to components
    ApplyToSpringArm(Context, SpringArm);
    ApplyToCamera(Context, Camera);

    // Apply time dilation if needed
    ApplyTimeDilation(Context);

    // Store for next frame
    PreviousAppliedDistance = Context.Output.Distance;
    PreviousAppliedFOV = Context.Output.FOV;
    PreviousAppliedRotation = Context.Output.Rotation;
    bFirstFrame = false;

    TotalApplyTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

    UE_LOG(LogCameraStage_Render, Verbose, TEXT("Execute: Applied Distance=%.1f, FOV=%.1f, Time=%.2fms"),
        Context.Output.Distance, Context.Output.FOV, TotalApplyTimeMs);

    // 【诊断日志】在return之前输出Distance变化
    static int32 Stage8LogCount = 0;
    if (Stage8LogCount < 10)
    {
        Stage8LogCount++;
        UE_LOG(LogTemp, Error, TEXT("【Stage8 RenderApply】 Distance: %.1f -> %.1f"), DistanceAtStart, Context.Output.Distance);
    }

    return EStageResult::Success;
}

void UCameraStage_RenderApply::OnPreExecute(const FStageExecutionContext& Context)
{
    UE_LOG(LogCameraStage_Render, VeryVerbose, TEXT("Stage 8 RenderApply: PreExecute"));
}

void UCameraStage_RenderApply::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    UE_LOG(LogCameraStage_Render, VeryVerbose, TEXT("Stage 8 RenderApply: PostExecute - PP=%s, TimeDilation=%s"),
        bPostProcessApplied ? TEXT("Yes") : TEXT("No"),
        bTimeDilationApplied ? TEXT("Yes") : TEXT("No"));
}

//========================================
// SpringArm Application 弹簧臂应用
//========================================

void UCameraStage_RenderApply::ApplyToSpringArm(FStageExecutionContext& Context, USpringArmComponent* SpringArm)
{
    if (!SpringArm)
    {
        return;
    }

    const FSoulsCameraOutput& Output = Context.Output;

    // Apply arm length (distance)
    ApplyArmLength(SpringArm, Output.Distance);

    // Apply socket offset
    ApplySocketOffset(SpringArm, Output.SocketOffset);

    // Apply target offset
    ApplyTargetOffset(SpringArm, Output.TargetOffset);

    // Apply rotation
    ApplySpringArmRotation(SpringArm, Output.Rotation);

    UE_LOG(LogCameraStage_Render, VeryVerbose, TEXT("ApplyToSpringArm: Distance=%.1f, SocketOffset=%s"),
        Output.Distance, *Output.SocketOffset.ToString());
}

void UCameraStage_RenderApply::ApplyArmLength(USpringArmComponent* SpringArm, float Distance)
{
    if (SpringArm)
    {
        // ★★★ 安全检查：确保距离不会小于合理的最小值 ★★★
        // 这是最后一道防线，防止ShoulderOffset(50)被错误赋值给Distance
        const float MinSafeDistance = 100.0f;  // 最小1米
        const float DefaultDistance = 400.0f;  // 默认4米
        
        if (Distance < MinSafeDistance)
        {
            UE_LOG(LogCameraStage_Render, Warning, 
                TEXT("ApplyArmLength: Distance (%.1f) below minimum (%.1f), using default (%.1f)"),
                Distance, MinSafeDistance, DefaultDistance);
            Distance = DefaultDistance;
        }
        
        SpringArm->TargetArmLength = Distance;
    }
}

void UCameraStage_RenderApply::ApplySocketOffset(USpringArmComponent* SpringArm, const FVector& Offset)
{
    if (SpringArm)
    {
        SpringArm->SocketOffset = Offset;
    }
}

void UCameraStage_RenderApply::ApplyTargetOffset(USpringArmComponent* SpringArm, const FVector& Offset)
{
    if (SpringArm)
    {
        SpringArm->TargetOffset = Offset;
    }
}

void UCameraStage_RenderApply::ApplySpringArmRotation(USpringArmComponent* SpringArm, const FRotator& Rotation)
{
    if (!SpringArm)
    {
        return;
    }

    // Use SetWorldRotation for absolute control over camera rotation
    // This bypasses the SpringArm's normal control rotation behavior
    SpringArm->SetWorldRotation(Rotation);

    UE_LOG(LogCameraStage_Render, VeryVerbose, TEXT("ApplySpringArmRotation: Pitch=%.1f, Yaw=%.1f, Roll=%.1f"),
        Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
}

//========================================
// Camera Application 相机应用
//========================================

void UCameraStage_RenderApply::ApplyToCamera(FStageExecutionContext& Context, UCameraComponent* Camera)
{
    if (!Camera)
    {
        return;
    }

    const FSoulsCameraOutput& Output = Context.Output;

    // Apply field of view
    ApplyFieldOfView(Camera, Output.FOV);

    // Apply post-process settings
    ApplyPostProcessSettings(Context, Camera);

    UE_LOG(LogCameraStage_Render, VeryVerbose, TEXT("ApplyToCamera: FOV=%.1f"),
        Output.FOV);
}

void UCameraStage_RenderApply::ApplyFieldOfView(UCameraComponent* Camera, float FOV)
{
    if (Camera)
    {
        Camera->SetFieldOfView(FOV);
    }
}

void UCameraStage_RenderApply::ApplyPostProcessSettings(FStageExecutionContext& Context, UCameraComponent* Camera)
{
    if (!Camera)
    {
        return;
    }

    const FSoulsCameraOutput& Output = Context.Output;

    // Enable post-process blend weight
    Camera->PostProcessBlendWeight = 1.0f;

    // Apply individual effects
    ApplyVignetteToPostProcess(Camera, Output.VignetteIntensity);
    ApplyColorTintToPostProcess(Camera, Output.ColorTint);
    ApplyDOFToPostProcess(Camera, Output.DOFFocalDistance, Output.DOFFocalRegion, Output.DOFNearBlurSize, Output.DOFFarBlurSize);

    // Motion blur
    if (Output.MotionBlurAmount >= 0.0f)
    {
        Camera->PostProcessSettings.bOverride_MotionBlurAmount = true;
        Camera->PostProcessSettings.MotionBlurAmount = Output.MotionBlurAmount;
    }
    else
    {
        Camera->PostProcessSettings.bOverride_MotionBlurAmount = false;
    }

    bPostProcessApplied = true;
}

void UCameraStage_RenderApply::ApplyVignetteToPostProcess(UCameraComponent* Camera, float Intensity)
{
    if (!Camera)
    {
        return;
    }

    if (Intensity > 0.0f)
    {
        Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
        Camera->PostProcessSettings.VignetteIntensity = Intensity;
    }
    else
    {
        Camera->PostProcessSettings.bOverride_VignetteIntensity = false;
    }
}

void UCameraStage_RenderApply::ApplyColorTintToPostProcess(UCameraComponent* Camera, const FLinearColor& ColorTint)
{
    if (!Camera)
    {
        return;
    }

    if (ColorTint != FLinearColor::White)
    {
        Camera->PostProcessSettings.bOverride_ColorGain = true;
        Camera->PostProcessSettings.ColorGain = FVector4(
            ColorTint.R, 
            ColorTint.G, 
            ColorTint.B, 
            ColorTint.A
        );
    }
    else
    {
        Camera->PostProcessSettings.bOverride_ColorGain = false;
    }
}

void UCameraStage_RenderApply::ApplyDOFToPostProcess(UCameraComponent* Camera, float FocalDistance, float FocalRadius, float NearBlur, float FarBlur)
{
    if (!Camera)
    {
        return;
    }

    // Focal distance
    if (FocalDistance > 0.0f)
    {
        Camera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
        Camera->PostProcessSettings.DepthOfFieldFocalDistance = FocalDistance;
    }
    else
    {
        Camera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = false;
    }

    // Focal region (UE4.27 uses different DOF settings)
    if (FocalRadius > 0.0f)
    {
        Camera->PostProcessSettings.bOverride_DepthOfFieldFocalRegion = true;
        Camera->PostProcessSettings.DepthOfFieldFocalRegion = FocalRadius;
    }

    // Near blur
    if (NearBlur > 0.0f)
    {
        Camera->PostProcessSettings.bOverride_DepthOfFieldNearBlurSize = true;
        Camera->PostProcessSettings.DepthOfFieldNearBlurSize = NearBlur;
    }

    // Far blur
    if (FarBlur > 0.0f)
    {
        Camera->PostProcessSettings.bOverride_DepthOfFieldFarBlurSize = true;
        Camera->PostProcessSettings.DepthOfFieldFarBlurSize = FarBlur;
    }
}

//========================================
// Time Dilation 时间膨胀
//========================================

void UCameraStage_RenderApply::ApplyTimeDilation(FStageExecutionContext& Context)
{
    const FSoulsCameraOutput& Output = Context.Output;

    // Only apply if time dilation is different from normal (1.0)
    if (!FMath::IsNearlyEqual(Output.TimeDilation, 1.0f))
    {
        if (UWorld* World = Context.Manager->GetWorld())
        {
            // Get game instance for time dilation
            AWorldSettings* WorldSettings = World->GetWorldSettings();
            if (WorldSettings)
            {
                WorldSettings->TimeDilation = Output.TimeDilation;
                bTimeDilationApplied = true;

                UE_LOG(LogCameraStage_Render, Verbose, TEXT("ApplyTimeDilation: TimeDilation=%.2f"),
                    Output.TimeDilation);
            }
        }
    }
}

//========================================
// Validation 验证
//========================================

USpringArmComponent* UCameraStage_RenderApply::GetSpringArm(const FStageExecutionContext& Context) const
{
    if (Context.Manager)
    {
        return Context.Manager->GetSpringArm();
    }
    return nullptr;
}

UCameraComponent* UCameraStage_RenderApply::GetCamera(const FStageExecutionContext& Context) const
{
    if (Context.Manager)
    {
        return Context.Manager->GetCamera();
    }
    return nullptr;
}
