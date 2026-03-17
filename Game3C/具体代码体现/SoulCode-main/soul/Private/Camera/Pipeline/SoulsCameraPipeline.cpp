// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/SoulsCameraPipeline.h"
#include "Camera/Core/SoulsCameraManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraPipeline, Log, All);

USoulsCameraPipeline::USoulsCameraPipeline()
    : CameraManager(nullptr)
    , bIsInitialized(false)
    , bCollectStats(false)
    , bPerformanceMode(false)
    , CurrentFrameNumber(0)
{
    // Initialize arrays for 8 stages
    RegisteredStages.SetNum(8);
    StageEnabled.SetNum(8);
    
    for (int32 i = 0; i < 8; ++i)
    {
        RegisteredStages[i] = nullptr;
        StageEnabled[i] = true;
    }
}

//========================================
// Initialization 初始化
//========================================

void USoulsCameraPipeline::Initialize(USoulsCameraManager* InManager)
{
    CameraManager = InManager;
    CurrentContext.Manager = InManager;
    bIsInitialized = (CameraManager != nullptr);
    
    if (bIsInitialized)
    {
        UE_LOG(LogCameraPipeline, Log, TEXT("Pipeline initialized successfully"));
    }
    else
    {
        UE_LOG(LogCameraPipeline, Warning, TEXT("Pipeline initialization failed: null manager"));
    }
}

//========================================
// Stage Registration Stage注册
//========================================

bool USoulsCameraPipeline::RegisterStage(int32 StageIndex, TScriptInterface<ICameraStage> Stage)
{
    const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
    
    if (ArrayIndex < 0)
    {
        UE_LOG(LogCameraPipeline, Warning, TEXT("RegisterStage: Invalid stage index %d (must be 1-8)"), StageIndex);
        return false;
    }
    
    if (!Stage)
    {
        UE_LOG(LogCameraPipeline, Warning, TEXT("RegisterStage: Cannot register null stage at index %d"), StageIndex);
        return false;
    }
    
    // Verify stage index matches
    if (Stage->GetStageIndex() != StageIndex)
    {
        UE_LOG(LogCameraPipeline, Warning, TEXT("RegisterStage: Stage reports index %d but registering at %d"),
            Stage->GetStageIndex(), StageIndex);
    }
    
    RegisteredStages[ArrayIndex] = Stage.GetObject();
    
    UE_LOG(LogCameraPipeline, Log, TEXT("Registered Stage %d: %s"), 
        StageIndex, *Stage->GetStageName().ToString());
    
    return true;
}

bool USoulsCameraPipeline::UnregisterStage(int32 StageIndex)
{
    const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
    
    if (ArrayIndex < 0)
    {
        UE_LOG(LogCameraPipeline, Warning, TEXT("UnregisterStage: Invalid stage index %d"), StageIndex);
        return false;
    }
    
    if (RegisteredStages[ArrayIndex])
    {
        UE_LOG(LogCameraPipeline, Log, TEXT("Unregistered Stage %d"), StageIndex);
        RegisteredStages[ArrayIndex] = nullptr;
        return true;
    }
    
    return false;
}

TScriptInterface<ICameraStage> USoulsCameraPipeline::GetStage(int32 StageIndex) const
{
    const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
    
    if (ArrayIndex < 0 || !RegisteredStages[ArrayIndex])
    {
        return nullptr;
    }
    
    return TScriptInterface<ICameraStage>(RegisteredStages[ArrayIndex]);
}

bool USoulsCameraPipeline::AreAllStagesRegistered() const
{
    // Required stages: 1, 2, 3, 5, 8
    // Optional stages: 4 (Modifier), 6 (Collision), 7 (PostProcess)
    static const TArray<int32> RequiredStages = {1, 2, 3, 5, 8};
    
    for (int32 StageIndex : RequiredStages)
    {
        const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
        if (!RegisteredStages[ArrayIndex])
        {
            return false;
        }
    }
    
    return true;
}

//========================================
// Execution 执行
//========================================

FSoulsCameraOutput USoulsCameraPipeline::Execute(float DeltaTime, const FCameraInputContext& InputContext)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogCameraPipeline, Warning, TEXT("Execute: Pipeline not initialized"));
        return FSoulsCameraOutput();
    }

    // Reset context for this frame
    CurrentContext.Reset();
    CurrentContext.DeltaTime = DeltaTime;
    CurrentContext.Manager = CameraManager;
    CurrentContext.InputContext = InputContext;

    // Update frame number
    CurrentFrameNumber = GFrameCounter;

    // Reset stats if collecting
    if (bCollectStats)
    {
        LastFrameStats.Reset();
        LastFrameStats.FrameNumber = CurrentFrameNumber;
    }

    const double PipelineStartTime = FPlatformTime::Seconds();

    // 诊断日志
    static int32 PipelineDiagCount = 0;
    bool bShouldLog = (PipelineDiagCount < 10);
    if (bShouldLog)
    {
        PipelineDiagCount++;
        UE_LOG(LogCameraPipeline, Warning, TEXT(""));
        UE_LOG(LogCameraPipeline, Warning, TEXT("【Pipeline Execute #%d】"), PipelineDiagCount);
    }

    // Execute all 8 stages in order
    for (int32 StageIndex = 1; StageIndex <= 8; ++StageIndex)
    {
        // Check for abort request
        if (CurrentContext.bShouldAbort)
        {
            UE_LOG(LogCameraPipeline, Log, TEXT("Pipeline aborted at Stage %d (requested by Stage %d)"),
                StageIndex, CurrentContext.AbortRequestedByStage);
            break;
        }

        const EStageResult Result = ExecuteStage(StageIndex);

        // 诊断日志 - 记录每个Stage的结果
        if (bShouldLog)
        {
            const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
            FString StageName = TEXT("Unknown");
            if (ArrayIndex >= 0 && RegisteredStages[ArrayIndex])
            {
                ICameraStage* Stage = Cast<ICameraStage>(RegisteredStages[ArrayIndex]);
                if (Stage)
                {
                    StageName = Stage->GetStageName().ToString();
                }
            }
            
            FString ResultStr;
            switch (Result)
            {
                case EStageResult::Success: ResultStr = TEXT("SUCCESS"); break;
                case EStageResult::Skipped: ResultStr = TEXT("SKIPPED"); break;
                case EStageResult::Failed: ResultStr = TEXT("FAILED <<<"); break;
                case EStageResult::Abort: ResultStr = TEXT("ABORT <<<"); break;
                default: ResultStr = TEXT("UNKNOWN"); break;
            }
            
            UE_LOG(LogCameraPipeline, Warning, TEXT("   Stage[%d] %s: %s"), StageIndex, *StageName, *ResultStr);
            
            // 在关键Stage后输出当前Output状态
            if (StageIndex == 2 || StageIndex == 3 || StageIndex == 5 || StageIndex == 8)
            {
                UE_LOG(LogCameraPipeline, Warning, TEXT("      -> Output: Distance=%.1f, FOV=%.1f, bIsValid=%s"),
                    CurrentContext.Output.Distance,
                    CurrentContext.Output.FOV,
                    CurrentContext.Output.bIsValid ? TEXT("true") : TEXT("false"));
            }
        }

        // Handle abort result
        if (Result == EStageResult::Abort)
        {
            CurrentContext.bShouldAbort = true;
            CurrentContext.AbortRequestedByStage = StageIndex;
            
            if (bShouldLog)
            {
                UE_LOG(LogCameraPipeline, Error, TEXT("   >>> Pipeline ABORTED at Stage[%d]"), StageIndex);
            }
        }
        
        // Handle failed result for required stages
        if (Result == EStageResult::Failed && IsStageRequired(StageIndex))
        {
            if (bShouldLog)
            {
                UE_LOG(LogCameraPipeline, Error, TEXT("   >>> Required Stage[%d] FAILED - Pipeline output will be INVALID"), StageIndex);
            }
        }
    }

    // Finalize stats
    if (bCollectStats)
    {
        LastFrameStats.TotalTimeMs = static_cast<float>((FPlatformTime::Seconds() - PipelineStartTime) * 1000.0);
    }

    // ★★★ 关键：只有当所有必需Stage成功时才设置bIsValid = true ★★★
    // 检查是否有Stage失败或被中止
    if (!CurrentContext.bShouldAbort && !CurrentContext.bHasStageError)
    {
        CurrentContext.Output.bIsValid = true;
    }
    
    if (bShouldLog)
    {
        UE_LOG(LogCameraPipeline, Warning, TEXT("【Pipeline Complete】 bIsValid=%s, Distance=%.1f, FOV=%.1f"),
            CurrentContext.Output.bIsValid ? TEXT("TRUE") : TEXT("FALSE"),
            CurrentContext.Output.Distance,
            CurrentContext.Output.FOV);
        UE_LOG(LogCameraPipeline, Warning, TEXT(""));
    }

    return CurrentContext.Output;
}

//========================================
// Stage Control Stage控制
//========================================

void USoulsCameraPipeline::SetStageEnabled(int32 StageIndex, bool bEnabled)
{
    const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
    
    if (ArrayIndex >= 0)
    {
        StageEnabled[ArrayIndex] = bEnabled;
        UE_LOG(LogCameraPipeline, Verbose, TEXT("Stage %d %s"), 
            StageIndex, bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

bool USoulsCameraPipeline::IsStageEnabled(int32 StageIndex) const
{
    const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
    
    if (ArrayIndex >= 0)
    {
        return StageEnabled[ArrayIndex];
    }
    
    return false;
}

void USoulsCameraPipeline::SetPerformanceMode(bool bEnabled)
{
    bPerformanceMode = bEnabled;
    
    if (bPerformanceMode)
    {
        // Disable optional stages: 4 (Modifier), 6 (Collision), 7 (PostProcess)
        SetStageEnabled(4, false);
        SetStageEnabled(6, false);
        SetStageEnabled(7, false);
        
        UE_LOG(LogCameraPipeline, Log, TEXT("Performance mode ENABLED - optional stages disabled"));
    }
    else
    {
        // Re-enable optional stages
        SetStageEnabled(4, true);
        SetStageEnabled(6, true);
        SetStageEnabled(7, true);
        
        UE_LOG(LogCameraPipeline, Log, TEXT("Performance mode DISABLED - all stages enabled"));
    }
}

//========================================
// Internal Methods 内部方法
//========================================

EStageResult USoulsCameraPipeline::ExecuteStage(int32 StageIndex)
{
    const int32 ArrayIndex = StageIndexToArrayIndex(StageIndex);
    
    // Validate array index
    if (ArrayIndex < 0)
    {
        UE_LOG(LogCameraPipeline, Error, TEXT("ExecuteStage: Invalid stage index %d"), StageIndex);
        return EStageResult::Failed;
    }

    // Check if stage is registered
    if (!RegisteredStages[ArrayIndex])
    {
        // Required stage missing is an error
        if (IsStageRequired(StageIndex))
        {
            UE_LOG(LogCameraPipeline, Warning, TEXT("ExecuteStage: Required Stage %d not registered"), StageIndex);
            return EStageResult::Failed;
        }
        
        // Optional stage missing is okay - just skip
        if (bCollectStats)
        {
            LastFrameStats.StagesSkipped++;
        }
        return EStageResult::Skipped;
    }

    // Check if stage is enabled
    if (!StageEnabled[ArrayIndex])
    {
        if (bCollectStats)
        {
            LastFrameStats.StagesSkipped++;
        }
        return EStageResult::Skipped;
    }

    // Get stage interface
    ICameraStage* Stage = Cast<ICameraStage>(RegisteredStages[ArrayIndex]);
    if (!Stage)
    {
        UE_LOG(LogCameraPipeline, Error, TEXT("ExecuteStage: Stage %d does not implement ICameraStage"), StageIndex);
        return EStageResult::Failed;
    }

    // Check if stage wants to execute this frame
    if (!Stage->ShouldExecute(CurrentContext))
    {
        if (bCollectStats)
        {
            LastFrameStats.StagesSkipped++;
        }
        return EStageResult::Skipped;
    }

    // Execute stage with optional timing
    double StageStartTime = 0.0;
    if (bCollectStats)
    {
        StageStartTime = FPlatformTime::Seconds();
    }

    // Pre-execute hook (for debugging/profiling)
    Stage->OnPreExecute(CurrentContext);
    
    // Main execution
    const EStageResult Result = Stage->Execute(CurrentContext);
    
    // Post-execute hook (for debugging/profiling)
    Stage->OnPostExecute(CurrentContext, Result);

    // Record timing and result
    if (bCollectStats)
    {
        LastFrameStats.StageTimesMs[ArrayIndex] = static_cast<float>((FPlatformTime::Seconds() - StageStartTime) * 1000.0);
        
        if (Result == EStageResult::Success)
        {
            LastFrameStats.StagesExecuted++;
        }
        else if (Result == EStageResult::Skipped)
        {
            LastFrameStats.StagesSkipped++;
        }
    }

    return Result;
}

bool USoulsCameraPipeline::IsStageRequired(int32 StageIndex) const
{
    // Required stages: 1 (Input), 2 (StateMachine), 3 (Module), 5 (Blend), 8 (Render)
    // Optional stages: 4 (Modifier), 6 (Collision), 7 (PostProcess)
    switch (StageIndex)
    {
        case 4:  // Stage 4: Modifier Apply - optional
        case 6:  // Stage 6: Collision Resolve - optional
        case 7:  // Stage 7: Post Process - optional
            return false;
        default:
            return true;
    }
}
