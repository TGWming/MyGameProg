// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_StateMachine.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Core/SoulsCameraStateMachine.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_State, Log, All);

//========================================
// Constructor 构造函数
//========================================

UCameraStage_StateMachine::UCameraStage_StateMachine()
    : StateMachine(nullptr)
    , CurrentStateName(NAME_None)
    , PreviousStateName(NAME_None)
    , bCurrentConfigValid(false)
    , bPreviousConfigValid(false)
    , TransitionAlpha(1.0f)
    , TransitionDuration(0.0f)
    , TransitionElapsed(0.0f)
    , CurrentBlendType(ECameraBlendType::Linear)
    , bIsTransitioning(false)
{
}

//========================================
// ICameraStage Interface 接口实现
//========================================

EStageResult UCameraStage_StateMachine::Execute(FStageExecutionContext& Context)
{
    // 【诊断日志】记录Stage开始时的Distance
    float DistanceAtStart = Context.Output.Distance;

    // Get state machine from manager if not set
    if (!StateMachine && Context.Manager)
    {
        StateMachine = Context.Manager->GetStateMachine();
    }

    if (!StateMachine)
    {
        UE_LOG(LogCameraStage_State, Warning, TEXT("Execute: No state machine available"));
        return EStageResult::Failed;
    }

    // 诊断日志
    static int32 Stage2DiagCount = 0;
    bool bShouldLog = (Stage2DiagCount < 10);
    if (bShouldLog)
    {
        Stage2DiagCount++;
        UE_LOG(LogCameraStage_State, Warning, TEXT("【Stage2 StateMachine #%d】"), Stage2DiagCount);
        UE_LOG(LogCameraStage_State, Warning, TEXT("   StateMachine IsInitialized: %s"), StateMachine->IsInitialized() ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogCameraStage_State, Warning, TEXT("   CurrentState: %s"), *StateMachine->GetCurrentStateName().ToString());
    }

    // Step 1: Evaluate state based on input context
    EvaluateState(Context);

    // Step 2: Update transition blending
    UpdateTransition(Context);

    // Step 3: Get current state config
    if (!StateMachine->GetCurrentStateConfig(CurrentStateConfig))
    {
        UE_LOG(LogCameraStage_State, Warning, TEXT("Execute: Failed to get current state config"));
        // Use default config instead of failing
        CurrentStateConfig = FCameraStateConfig();
        bCurrentConfigValid = false;
        
        if (bShouldLog)
        {
            UE_LOG(LogCameraStage_State, Error, TEXT("   ⚠️ GetCurrentStateConfig FAILED!"));
        }
    }
    else
    {
        bCurrentConfigValid = true;
        
        if (bShouldLog)
        {
            UE_LOG(LogCameraStage_State, Warning, TEXT("   Config loaded - BaseDistance: %.1f, BaseFOV: %.1f"),
                CurrentStateConfig.StateBase.Distance.BaseDistance,
                CurrentStateConfig.StateBase.FOV.BaseFOV);
        }
    }

    // Step 4: Apply state config to output (with blending if transitioning)
    if (StateMachine->IsInTransition())
    {
        bIsTransitioning = true;
        PreviousStateName = StateMachine->GetPreviousStateName();
        
        if (StateMachine->GetStateConfig(PreviousStateName, PreviousStateConfig))
        {
            bPreviousConfigValid = true;
            const float RawAlpha = StateMachine->GetBlendAlpha();
            const float CurvedAlpha = ApplyBlendCurve(RawAlpha, CurrentBlendType);
            BlendStateConfigs(Context, PreviousStateConfig, CurrentStateConfig, CurvedAlpha);
            
            if (bShouldLog)
            {
                UE_LOG(LogCameraStage_State, Warning, TEXT("   Blending from %s to %s (Alpha: %.2f)"),
                    *PreviousStateName.ToString(), *StateMachine->GetCurrentStateName().ToString(), CurvedAlpha);
            }
        }
        else
        {
            bPreviousConfigValid = false;
            ApplyStateConfig(Context, CurrentStateConfig);
        }
    }
    else
    {
        bIsTransitioning = false;
        ApplyStateConfig(Context, CurrentStateConfig);
    }

    // Step 5: Set state info in output
    Context.Output.CurrentStateName = StateMachine->GetCurrentStateName();
    Context.Output.StateBlendAlpha = StateMachine->GetBlendAlpha();
    Context.Output.bIsInTransition = StateMachine->IsInTransition();

    // Cache current state name
    CurrentStateName = Context.Output.CurrentStateName;

    if (bShouldLog)
    {
        UE_LOG(LogCameraStage_State, Warning, TEXT("   Output after Stage2: Distance=%.1f, FOV=%.1f, State=%s"),
            Context.Output.Distance, Context.Output.FOV, *Context.Output.CurrentStateName.ToString());
    }

    // 【诊断日志】在return之前输出Distance变化
    static int32 Stage2LogCount = 0;
    if (Stage2LogCount < 10)
    {
        Stage2LogCount++;
        UE_LOG(LogTemp, Error, TEXT("【Stage2 StateMachine】 Distance: %.1f -> %.1f"), DistanceAtStart, Context.Output.Distance);
    }

    return EStageResult::Success;
}

void UCameraStage_StateMachine::OnPreExecute(const FStageExecutionContext& Context)
{
    UE_LOG(LogCameraStage_State, VeryVerbose, TEXT("Stage 2 StateMachine: PreExecute"));
}

void UCameraStage_StateMachine::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    UE_LOG(LogCameraStage_State, VeryVerbose, TEXT("Stage 2 StateMachine: PostExecute - State: %s, Transitioning: %s"),
        *CurrentStateName.ToString(),
        bIsTransitioning ? TEXT("Yes") : TEXT("No"));
}

//========================================
// State Machine Reference 状态机引用
//========================================

void UCameraStage_StateMachine::SetStateMachine(USoulsCameraStateMachine* InStateMachine)
{
    StateMachine = InStateMachine;
    
    if (StateMachine)
    {
        UE_LOG(LogCameraStage_State, Log, TEXT("State machine set successfully"));
    }
}

//========================================
// State Evaluation 状态评估
//========================================

void UCameraStage_StateMachine::EvaluateState(FStageExecutionContext& Context)
{
    if (!StateMachine)
    {
        return;
    }

    // If a manual state is active (requested by Blueprint/external code), 
    // skip automatic state determination to preserve the manual state
    USoulsCameraManager* CameraManager = Cast<USoulsCameraManager>(StateMachine->GetOuter());
    if (CameraManager && CameraManager->IsManualStateActive())
    {
        // Still let the state machine evaluate its own internal conditions
        StateMachine->EvaluateState(Context.InputContext);
        return;
    }

    // Determine desired state based on input context
    const FName DesiredState = DetermineDesiredState(Context);

    // Request state change if different from current
    if (DesiredState != NAME_None && DesiredState != StateMachine->GetCurrentStateName())
    {
        StateMachine->RequestStateChange(DesiredState);
    }

    // Let state machine evaluate its own conditions as well
    StateMachine->EvaluateState(Context.InputContext);
}

FName UCameraStage_StateMachine::DetermineDesiredState(const FStageExecutionContext& Context) const
{
    const FCameraInputContext& Input = Context.InputContext;

    // Priority-based state determination (higher priority first)
    
    // 1. Death state (highest priority)
    if (Input.HealthRatio <= 0.0f)
    {
        return FName(TEXT("Death"));
    }

    // 2. Boss lock-on
    if (Input.bHasTarget && Input.bTargetIsBoss)
    {
        return FName(TEXT("BossLockOn"));
    }

    // 3. Regular lock-on
    if (Input.bHasTarget)
    {
        return FName(TEXT("LockOn"));
    }

    // 4. Combat (no target but in combat)
    if (Input.bIsInCombat)
    {
        return FName(TEXT("Combat"));
    }

    // 5. Sprint
    if (Input.bIsSprinting)
    {
        return FName(TEXT("Sprint"));
    }

    // 6. In air
    if (Input.bIsInAir)
    {
        return FName(TEXT("InAir"));
    }

    // 7. Crouch
    if (Input.bIsCrouching)
    {
        return FName(TEXT("Crouch"));
    }

    // 8. Default exploration
    return FName(TEXT("Exploration"));
}

//========================================
// State Transition 状态过渡
//========================================

void UCameraStage_StateMachine::UpdateTransition(FStageExecutionContext& Context)
{
    if (!StateMachine)
    {
        return;
    }

    // Update transition timing in state machine
    StateMachine->UpdateTransition(Context.DeltaTime);

    // Cache transition state
    bIsTransitioning = StateMachine->IsInTransition();
    TransitionAlpha = StateMachine->GetBlendAlpha();
}

bool UCameraStage_StateMachine::IsInTransition() const
{
    return bIsTransitioning;
}

//========================================
// Configuration Application 配置应用
//========================================

void UCameraStage_StateMachine::ApplyStateConfig(FStageExecutionContext& Context, const FCameraStateConfig& Config)
{
    // Apply StateBase parameters to output
    const FStateBaseParams& StateBase = Config.StateBase;

    // 诊断日志
    static int32 ApplyConfigDiagCount = 0;
    bool bShouldLog = (ApplyConfigDiagCount < 5);
    if (bShouldLog)
    {
        ApplyConfigDiagCount++;
        UE_LOG(LogCameraStage_State, Warning, TEXT("【ApplyStateConfig #%d】"));
        UE_LOG(LogCameraStage_State, Warning, TEXT("   Input Config - BaseDistance: %.1f, BaseFOV: %.1f"),
            StateBase.Distance.BaseDistance, StateBase.FOV.BaseFOV);
    }

    // Distance
    Context.Output.Distance = StateBase.Distance.BaseDistance;

    // FOV
    Context.Output.FOV = StateBase.FOV.BaseFOV;

    // Calculate focus point from character location + offsets
    FVector FocusOffset = StateBase.Offset.FocusOffset;
    FocusOffset.Z += StateBase.Offset.VerticalOffset;
    Context.Output.FocusPoint = Context.InputContext.CharacterLocation + FocusOffset;

    // Socket and target offsets
    Context.Output.SocketOffset = StateBase.Offset.SocketOffset;
    Context.Output.TargetOffset = StateBase.Offset.TargetOffset;

    // Initial rotation from character (will be modified by rotation modules in Stage 3)
    Context.Output.Rotation = Context.InputContext.CharacterRotation;

    if (bShouldLog)
    {
        UE_LOG(LogCameraStage_State, Warning, TEXT("   Output Applied - Distance: %.1f, FOV: %.1f"),
            Context.Output.Distance, Context.Output.FOV);
        UE_LOG(LogCameraStage_State, Warning, TEXT("   FocusPoint: (%.1f, %.1f, %.1f)"),
            Context.Output.FocusPoint.X, Context.Output.FocusPoint.Y, Context.Output.FocusPoint.Z);
    }

    UE_LOG(LogCameraStage_State, VeryVerbose, TEXT("ApplyStateConfig: Distance=%.1f, FOV=%.1f"),
        Context.Output.Distance, Context.Output.FOV);
}

void UCameraStage_StateMachine::BlendStateConfigs(
    FStageExecutionContext& Context,
    const FCameraStateConfig& FromConfig,
    const FCameraStateConfig& ToConfig,
    float BlendAlpha)
{
    const FStateBaseParams& FromState = FromConfig.StateBase;
    const FStateBaseParams& ToState = ToConfig.StateBase;

    // Clamp alpha to valid range
    BlendAlpha = FMath::Clamp(BlendAlpha, 0.0f, 1.0f);

    // Blend Distance
    Context.Output.Distance = FMath::Lerp(
        FromState.Distance.BaseDistance,
        ToState.Distance.BaseDistance,
        BlendAlpha
    );

    // Blend FOV
    Context.Output.FOV = FMath::Lerp(
        FromState.FOV.BaseFOV,
        ToState.FOV.BaseFOV,
        BlendAlpha
    );

    // Blend Focus Offset (including vertical offset)
    FVector FromFocusOffset = FromState.Offset.FocusOffset;
    FVector ToFocusOffset = ToState.Offset.FocusOffset;
    FromFocusOffset.Z += FromState.Offset.VerticalOffset;
    ToFocusOffset.Z += ToState.Offset.VerticalOffset;

    const FVector BlendedFocusOffset = FMath::Lerp(FromFocusOffset, ToFocusOffset, BlendAlpha);
    Context.Output.FocusPoint = Context.InputContext.CharacterLocation + BlendedFocusOffset;

    // Blend Socket Offset
    Context.Output.SocketOffset = FMath::Lerp(
        FromState.Offset.SocketOffset,
        ToState.Offset.SocketOffset,
        BlendAlpha
    );

    // Blend Target Offset
    Context.Output.TargetOffset = FMath::Lerp(
        FromState.Offset.TargetOffset,
        ToState.Offset.TargetOffset,
        BlendAlpha
    );

    // Initial rotation from character
    Context.Output.Rotation = Context.InputContext.CharacterRotation;

    UE_LOG(LogCameraStage_State, VeryVerbose, TEXT("BlendStateConfigs: Alpha=%.2f, Distance=%.1f, FOV=%.1f"),
        BlendAlpha, Context.Output.Distance, Context.Output.FOV);
}

float UCameraStage_StateMachine::ApplyBlendCurve(float Alpha, ECameraBlendType BlendType) const
{
    // Clamp input
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    switch (BlendType)
    {
        case ECameraBlendType::Cut:
        case ECameraBlendType::Instant:
            return (Alpha >= 1.0f) ? 1.0f : 0.0f;

        case ECameraBlendType::Linear:
            return Alpha;

        case ECameraBlendType::SmoothStep:
            // Hermite interpolation (3t^2 - 2t^3)
            return Alpha * Alpha * (3.0f - 2.0f * Alpha);

        case ECameraBlendType::EaseIn:
            // Quadratic ease in (t^2)
            return Alpha * Alpha;

        case ECameraBlendType::EaseOut:
            // Quadratic ease out (1 - (1-t)^2)
            return 1.0f - FMath::Square(1.0f - Alpha);

        case ECameraBlendType::EaseInOut:
            // Quadratic ease in-out
            if (Alpha < 0.5f)
            {
                return 2.0f * Alpha * Alpha;
            }
            else
            {
                return 1.0f - FMath::Pow(-2.0f * Alpha + 2.0f, 2.0f) / 2.0f;
            }

        case ECameraBlendType::Cubic:
            // Cubic interpolation (t^3)
            return Alpha * Alpha * Alpha;

        case ECameraBlendType::Exponential:
            // Exponential ease out
            return (Alpha >= 1.0f) ? 1.0f : 1.0f - FMath::Pow(2.0f, -10.0f * Alpha);

        case ECameraBlendType::Circular:
            // Circular ease out
            return FMath::Sqrt(1.0f - FMath::Square(Alpha - 1.0f));

        case ECameraBlendType::Spring:
            // Simple spring approximation
            {
                const float C4 = (2.0f * PI) / 3.0f;
                return (Alpha >= 1.0f) ? 1.0f :
                    FMath::Pow(2.0f, -10.0f * Alpha) * FMath::Sin((Alpha * 10.0f - 0.75f) * C4) + 1.0f;
            }

        case ECameraBlendType::Custom:
        default:
            // Default to linear for custom (would need curve asset)
            return Alpha;
    }
}

//========================================
// State Priority 状态优先级
//========================================

int32 UCameraStage_StateMachine::GetStatePriority(ECameraStateCategory Category) const
{
    // Higher number = higher priority
    switch (Category)
    {
        case ECameraStateCategory::Death:
            return 1000;
        case ECameraStateCategory::Cinematic:
            return 900;
        case ECameraStateCategory::Boss:
            return 800;
        case ECameraStateCategory::LockOn:
            return 700;
        case ECameraStateCategory::Combat:
            return 600;
        case ECameraStateCategory::Sprint:
            return 500;
        case ECameraStateCategory::Mount:
            return 400;
        case ECameraStateCategory::Swim:
            return 350;
        case ECameraStateCategory::Climb:
            return 300;
        case ECameraStateCategory::NPC:
            return 250;
        case ECameraStateCategory::Item:
            return 200;
        case ECameraStateCategory::RestPoint:
            return 150;
        case ECameraStateCategory::Explore:
        case ECameraStateCategory::FreeExploration:
            return 100;
        case ECameraStateCategory::Environment:
            return 50;
        default:
            return 0;
    }
}
