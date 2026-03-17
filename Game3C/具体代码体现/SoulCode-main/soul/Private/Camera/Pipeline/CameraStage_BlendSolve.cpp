// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_BlendSolve.h"
#include "Camera/Core/SoulsCameraManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_Blend, Log, All);

//========================================
// Constructor 构造函数
//========================================

UCameraStage_BlendSolve::UCameraStage_BlendSolve()
    : ModuleOutputsProcessed(0)
    , ModifierOutputsProcessed(0)
    , OverrideConflictsResolved(0)
    , TotalBlendTimeMs(0.0f)
{
}

//========================================
// ICameraStage Interface 接口实现
//========================================

EStageResult UCameraStage_BlendSolve::Execute(FStageExecutionContext& Context)
{
    // 【诊断日志】记录Stage开始时的Distance
    float DistanceAtStart = Context.Output.Distance;

    // Reset statistics
    ModuleOutputsProcessed = 0;
    ModifierOutputsProcessed = 0;
    OverrideConflictsResolved = 0;

    const double StartTime = FPlatformTime::Seconds();

    // Step 1: Sort module outputs by priority
    SortOutputsByPriority(Context.ModuleOutputs);

    // Step 2: Resolve override conflicts
    ResolveOverrideConflicts(Context.ModuleOutputs);

    // Step 3: Blend all module outputs
    BlendModuleOutputs(Context);

    // Step 4: Sort modifier outputs by priority
    SortModifiersByPriority(Context.ModifierOutputs);

    // Step 5: Apply modifier effects
    ApplyModifierOutputs(Context);

    TotalBlendTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

    UE_LOG(LogCameraStage_Blend, Verbose, TEXT("Execute: Processed %d modules, %d modifiers in %.2fms"),
        ModuleOutputsProcessed, ModifierOutputsProcessed, TotalBlendTimeMs);

    // 【诊断日志】在return之前输出Distance变化
    static int32 Stage5LogCount = 0;
    if (Stage5LogCount < 10)
    {
        Stage5LogCount++;
        UE_LOG(LogTemp, Error, TEXT("【Stage5 BlendSolve】 Distance: %.1f -> %.1f"), DistanceAtStart, Context.Output.Distance);
    }

    return EStageResult::Success;
}

void UCameraStage_BlendSolve::OnPreExecute(const FStageExecutionContext& Context)
{
    UE_LOG(LogCameraStage_Blend, VeryVerbose, TEXT("Stage 5 BlendSolve: PreExecute - %d module outputs, %d modifier outputs"),
        Context.ModuleOutputs.Num(), Context.ModifierOutputs.Num());
}

void UCameraStage_BlendSolve::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    UE_LOG(LogCameraStage_Blend, VeryVerbose, TEXT("Stage 5 BlendSolve: PostExecute - Distance=%.1f, FOV=%.1f"),
        Context.Output.Distance, Context.Output.FOV);
}

//========================================
// Sorting 排序
//========================================

void UCameraStage_BlendSolve::SortOutputsByPriority(TArray<FModuleOutput>& Outputs)
{
    // Sort by priority (lower priority first, higher priority processed last and can override)
    Outputs.Sort([](const FModuleOutput& A, const FModuleOutput& B)
    {
        return A.Priority < B.Priority;
    });

    UE_LOG(LogCameraStage_Blend, VeryVerbose, TEXT("SortOutputsByPriority: Sorted %d outputs"), Outputs.Num());
}

void UCameraStage_BlendSolve::SortModifiersByPriority(TArray<FModifierOutput>& Outputs)
{
    // Sort by modifier type as pseudo-priority
    // Order: Shake < Reaction < Zoom < Effect < Cinematic < Override < TimeDilation < PostProcess
    // Lower values processed first, higher values (like Override/Cinematic) processed last
    Outputs.Sort([](const FModifierOutput& A, const FModifierOutput& B)
    {
        return static_cast<uint8>(A.ModifierType) < static_cast<uint8>(B.ModifierType);
    });

    UE_LOG(LogCameraStage_Blend, VeryVerbose, TEXT("SortModifiersByPriority: Sorted %d outputs"), Outputs.Num());
}

//========================================
// Module Blending 模块混合
//========================================

void UCameraStage_BlendSolve::BlendModuleOutputs(FStageExecutionContext& Context)
{
    // ★ 从 Context 获取 Manager 的 Debug 开关
    bool bShouldLogBlend = false;
    if (Context.Manager)
    {
        bShouldLogBlend = Context.Manager->IsBlendSolveDebugEnabled();
    }

    // ★ 诊断日志 ★
    // 检查是否有来自 R03 的 Rotation 输出（Pitch 不为 0 表示是锁定状态）
    bool bHasLockOnRotation = false;
    for (const FModuleOutput& CheckOutput : Context.ModuleOutputs)
    {
        if (CheckOutput.bHasRotationOutput && !FMath::IsNearlyZero(CheckOutput.RotationOutput.Pitch, 1.0f))
        {
            bHasLockOnRotation = true;
            break;
        }
    }
    
    if (bShouldLogBlend)
    {
        if (bHasLockOnRotation)
        {
            UE_LOG(LogTemp, Error, TEXT(""));
            UE_LOG(LogTemp, Error, TEXT("★★★ BlendModuleOutputs [LOCK-ON MODE] ★★★"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT(""));
            UE_LOG(LogTemp, Error, TEXT("===== BlendModuleOutputs ====="));
        }
        UE_LOG(LogTemp, Error, TEXT("Input Context.Output.Distance: %.1f"), Context.Output.Distance);
        UE_LOG(LogTemp, Error, TEXT("ModuleOutputs count: %d"), Context.ModuleOutputs.Num());
    }

    // ★★★ 检查是否有 Rotation 输出 ★★★
    int32 RotationOutputCount = 0;
    for (const FModuleOutput& CheckOutput : Context.ModuleOutputs)
    {
        if (CheckOutput.bHasRotationOutput)
        {
            RotationOutputCount++;
            if (bShouldLogBlend)
            {
                UE_LOG(LogTemp, Error, TEXT("★★★ Found Rotation output: ModuleID=%d, Pitch=%.1f, Yaw=%.1f, Weight=%.2f, Policy=%d"),
                    static_cast<int32>(CheckOutput.ModuleID),
                    CheckOutput.RotationOutput.Pitch,
                    CheckOutput.RotationOutput.Yaw,
                    CheckOutput.Weight,
                    static_cast<int32>(CheckOutput.BlendPolicy));
            }
        }
    }
    if (bShouldLogBlend)
    {
        UE_LOG(LogTemp, Error, TEXT("★★★ Total Rotation outputs: %d"), RotationOutputCount);
    }

    // Start with base values from Stage 2 (StateMachine)
    FVector BlendedPosition = Context.Output.FocusPoint;
    FRotator BlendedRotation = Context.Output.Rotation;
    float BlendedDistance = Context.Output.Distance;
    float BlendedFOV = Context.Output.FOV;
    FVector BlendedSocketOffset = Context.Output.SocketOffset;

    // ★★★ 安全检查：如果起始Distance异常小，使用默认值 ★★★
    if (BlendedDistance < 100.0f)
    {
        if (bShouldLogBlend)
        {
            UE_LOG(LogTemp, Error, TEXT("★★★ WARNING: Initial BlendedDistance (%.1f) is abnormally small! Using 400.0f"), BlendedDistance);
        }
        BlendedDistance = 400.0f;  // 使用默认值
    }

    if (bShouldLogBlend)
    {
        UE_LOG(LogTemp, Error, TEXT("Starting BlendedDistance: %.1f"), BlendedDistance);
    }

    // Process each module output in priority order
    int32 OutputIndex = 0;
    for (const FModuleOutput& Output : Context.ModuleOutputs)
    {
        if (bShouldLogBlend)
        {
            UE_LOG(LogTemp, Error, TEXT("  ModuleOutput[%d]: Category=%d, Weight=%.2f, bHasDistanceOutput=%s, DistanceOutput=%.1f"),
                OutputIndex,
                static_cast<int32>(Output.ModuleCategory),
                Output.Weight,
                Output.bHasDistanceOutput ? TEXT("YES") : TEXT("NO"),
                Output.DistanceOutput);
        }
        OutputIndex++;

        if (Output.Weight <= 0.0f)
        {
            if (bShouldLogBlend) UE_LOG(LogTemp, Error, TEXT("    -> Skipped (Weight <= 0)"));
            continue;
        }

        ModuleOutputsProcessed++;

        // Position
        if (Output.bHasPositionOutput)
        {
            BlendedPosition = ApplyBlendPolicyVector(
                BlendedPosition, 
                Output.PositionOffset, 
                Output.BlendPolicy, 
                Output.Weight
            );
        }

        // Rotation
        if (Output.bHasRotationOutput)
        {
            FRotator RotationBefore = BlendedRotation;
            
            BlendedRotation = ApplyBlendPolicyRotator(
                BlendedRotation,
                Output.RotationOutput,
                Output.BlendPolicy,
                Output.Weight
            );
            
            // ★★★ 调试日志 ★★★
            if (bShouldLogBlend)
            {
                UE_LOG(LogTemp, Error, TEXT("    -> Rotation: (Pitch=%.1f, Yaw=%.1f) -> (Pitch=%.1f, Yaw=%.1f)"),
                    RotationBefore.Pitch, RotationBefore.Yaw,
                    BlendedRotation.Pitch, BlendedRotation.Yaw);
            }
        }

        // Distance
        if (Output.bHasDistanceOutput)
        {
            float DistanceBefore = BlendedDistance;
            
            // ★★★ 安全检查：跳过异常小的Distance输出（可能是错误赋值） ★★★
            if (!Output.bDistanceIsMultiplier && Output.DistanceOutput < 100.0f && Output.DistanceOutput > 0.0f)
            {
                if (bShouldLogBlend)
                {
                    UE_LOG(LogTemp, Error, TEXT("    -> ★★★ SKIPPING suspicious DistanceOutput %.1f (possible ShoulderOffset bug)"), 
                        Output.DistanceOutput);
                }
                // 跳过这个可疑的输出
                continue;
            }
            
            if (Output.bDistanceIsMultiplier)
            {
                // Multiplier mode: lerp towards multiplier effect
                BlendedDistance *= FMath::Lerp(1.0f, Output.DistanceOutput, Output.Weight);
            }
            else
            {
                BlendedDistance = ApplyBlendPolicy(
                    BlendedDistance, 
                    Output.DistanceOutput, 
                    Output.BlendPolicy, 
                    Output.Weight
                );
            }
            
            if (bShouldLogBlend)
            {
                UE_LOG(LogTemp, Error, TEXT("    -> Distance: %.1f -> %.1f (Policy=%d, IsMultiplier=%s)"),
                    DistanceBefore, BlendedDistance,
                    static_cast<int32>(Output.BlendPolicy),
                    Output.bDistanceIsMultiplier ? TEXT("YES") : TEXT("NO"));
            }
        }

        // FOV
        if (Output.bHasFOVOutput)
        {
            BlendedFOV = ApplyBlendPolicy(
                BlendedFOV, 
                Output.FOVOutput, 
                Output.BlendPolicy, 
                Output.Weight
            );
        }

        // Socket Offset
        if (Output.bHasSocketOffsetOutput)
        {
            BlendedSocketOffset = ApplyBlendPolicyVector(
                BlendedSocketOffset, 
                Output.SocketOffsetOutput, 
                Output.BlendPolicy, 
                Output.Weight
            );
        }
    }

    // ★★★ 最终安全检查：确保输出的Distance是有效的 ★★★
    if (BlendedDistance < 100.0f)
    {
        if (bShouldLogBlend)
        {
            UE_LOG(LogTemp, Error, TEXT("★★★ FINAL FIX: BlendedDistance (%.1f) too small, forcing to 400.0f"), BlendedDistance);
        }
        BlendedDistance = 400.0f;
    }

    // === Framing Module FocusPoint 处理 ===
    // 如果有 Module 输出了 FocusPoint（来自 R10_Framing），
    // 使用该 FocusPoint 覆盖 Context.Output.FocusPoint
    // 优先级最高的 FocusPoint 胜出
    {
        bool bFoundFocusPoint = false;
        FVector BestFocusPoint = FVector::ZeroVector;
        int32 BestFocusPriority = -1;

        for (const FModuleOutput& Output : Context.ModuleOutputs)
        {
            if (Output.bHasFocusPoint && Output.Weight > 0.0f)
            {
                if (Output.Priority > BestFocusPriority)
                {
                    BestFocusPoint = Output.FocusPoint;
                    BestFocusPriority = Output.Priority;
                    bFoundFocusPoint = true;
                }
            }
        }

        if (bFoundFocusPoint)
        {
            BlendedPosition = BestFocusPoint;
            if (bShouldLogBlend)
            {
                UE_LOG(LogTemp, Error, TEXT("FocusPoint Override: Priority=%d, FocusPoint=(%s)"),
                    BestFocusPriority, *BestFocusPoint.ToString());
            }
        }
    }

    // Apply blended values to output
    Context.Output.FocusPoint = BlendedPosition;
    Context.Output.Rotation = BlendedRotation;
    Context.Output.Distance = BlendedDistance;
    Context.Output.FOV = BlendedFOV;
    Context.Output.SocketOffset = BlendedSocketOffset;

    if (bShouldLogBlend)
    {
        UE_LOG(LogTemp, Error, TEXT("Final BlendedDistance: %.1f"), BlendedDistance);
        UE_LOG(LogTemp, Error, TEXT("Final BlendedRotation: Pitch=%.1f, Yaw=%.1f"), BlendedRotation.Pitch, BlendedRotation.Yaw);
        UE_LOG(LogTemp, Error, TEXT("===== BlendModuleOutputs END ====="));
        UE_LOG(LogTemp, Error, TEXT(""));
    }

    UE_LOG(LogCameraStage_Blend, VeryVerbose, TEXT("BlendModuleOutputs: Final Distance=%.1f, FOV=%.1f"),
        BlendedDistance, BlendedFOV);
}

FVector UCameraStage_BlendSolve::BlendPositions(const TArray<FModuleOutput>& Outputs, const FVector& BasePosition)
{
    FVector Result = BasePosition;

    for (const FModuleOutput& Output : Outputs)
    {
        if (Output.bHasPositionOutput && Output.Weight > 0.0f)
        {
            Result = ApplyBlendPolicyVector(Result, Output.PositionOffset, Output.BlendPolicy, Output.Weight);
        }
    }

    return Result;
}

FRotator UCameraStage_BlendSolve::BlendRotations(const TArray<FModuleOutput>& Outputs, const FRotator& BaseRotation)
{
    FRotator Result = BaseRotation;

    for (const FModuleOutput& Output : Outputs)
    {
        if (Output.bHasRotationOutput && Output.Weight > 0.0f)
        {
            Result = ApplyBlendPolicyRotator(Result, Output.RotationOutput, Output.BlendPolicy, Output.Weight);
        }
    }

    return Result;
}

float UCameraStage_BlendSolve::BlendDistances(const TArray<FModuleOutput>& Outputs, float BaseDistance)
{
    float Result = BaseDistance;

    for (const FModuleOutput& Output : Outputs)
    {
        if (Output.bHasDistanceOutput && Output.Weight > 0.0f)
        {
            if (Output.bDistanceIsMultiplier)
            {
                Result *= FMath::Lerp(1.0f, Output.DistanceOutput, Output.Weight);
            }
            else
            {
                Result = ApplyBlendPolicy(Result, Output.DistanceOutput, Output.BlendPolicy, Output.Weight);
            }
        }
    }

    return Result;
}

float UCameraStage_BlendSolve::BlendFOVs(const TArray<FModuleOutput>& Outputs, float BaseFOV)
{
    float Result = BaseFOV;

    for (const FModuleOutput& Output : Outputs)
    {
        if (Output.bHasFOVOutput && Output.Weight > 0.0f)
        {
            Result = ApplyBlendPolicy(Result, Output.FOVOutput, Output.BlendPolicy, Output.Weight);
        }
    }

    return Result;
}

FVector UCameraStage_BlendSolve::BlendSocketOffsets(const TArray<FModuleOutput>& Outputs, const FVector& BaseOffset)
{
    FVector Result = BaseOffset;

    for (const FModuleOutput& Output : Outputs)
    {
        if (Output.bHasSocketOffsetOutput && Output.Weight > 0.0f)
        {
            Result = ApplyBlendPolicyVector(Result, Output.SocketOffsetOutput, Output.BlendPolicy, Output.Weight);
        }
    }

    return Result;
}

//========================================
// Modifier Application 修改器应用
//========================================

void UCameraStage_BlendSolve::ApplyModifierOutputs(FStageExecutionContext& Context)
{
    // Apply modifier effects on top of blended module outputs
    for (const FModifierOutput& Output : Context.ModifierOutputs)
    {
        if (!Output.IsActive())
        {
            continue;
        }

        // Check for complete camera override first
        if (ShouldModifierOverride(Output))
        {
            // Camera override takes full control
            Context.Output.FocusPoint = Output.OverrideTransform.GetLocation();
            Context.Output.Rotation = Output.OverrideTransform.GetRotation().Rotator();
            
            UE_LOG(LogCameraStage_Blend, Verbose, TEXT("ApplyModifierOutputs: Camera override applied"));
            
            // Skip remaining modifiers after override
            break;
        }

        ApplySingleModifier(Context.Output, Output);
        ModifierOutputsProcessed++;
    }

    UE_LOG(LogCameraStage_Blend, VeryVerbose, TEXT("ApplyModifierOutputs: Applied %d modifiers"),
        ModifierOutputsProcessed);
}

void UCameraStage_BlendSolve::ApplySingleModifier(FSoulsCameraOutput& Output, const FModifierOutput& Modifier)
{
    const float Weight = Modifier.CurrentWeight;

    if (Weight <= 0.0f)
    {
        return;
    }

    // Position effect (shake, knockback, etc.)
    if (!Modifier.PositionEffect.IsNearlyZero())
    {
        Output.FocusPoint += Modifier.PositionEffect * Weight;
    }

    // Rotation effect (shake, reaction, etc.)
    if (!Modifier.RotationEffect.IsNearlyZero())
    {
        Output.Rotation += Modifier.RotationEffect * Weight;
    }

    // Distance effect (zoom pulse, etc.)
    if (!FMath::IsNearlyZero(Modifier.DistanceEffect))
    {
        Output.Distance += Modifier.DistanceEffect * Weight;
    }

    // FOV effect (zoom, impact, etc.)
    if (!FMath::IsNearlyZero(Modifier.FOVEffect))
    {
        Output.FOV += Modifier.FOVEffect * Weight;
    }

    // Post-process effects
    if (!FMath::IsNearlyZero(Modifier.VignetteEffect))
    {
        Output.VignetteIntensity += Modifier.VignetteEffect * Weight;
    }

    // Color tint (for status effects, low health, etc.)
    if (Modifier.ColorTintEffect != FLinearColor::White)
    {
        Output.ColorTint = FMath::Lerp(Output.ColorTint, Modifier.ColorTintEffect, Weight);
    }

    // Time dilation (slow-mo, hit-stop)
    if (!FMath::IsNearlyZero(Modifier.TimeDilationEffect - 1.0f))
    {
        Output.TimeDilation = FMath::Lerp(1.0f, Modifier.TimeDilationEffect, Weight);
    }
}

//========================================
// Blend Policy Implementation 混合策略实现
//========================================

float UCameraStage_BlendSolve::ApplyBlendPolicy(float CurrentValue, float NewValue, EBlendPolicy Policy, float Weight)
{
    // Clamp weight to valid range
    Weight = FMath::Clamp(Weight, 0.0f, 1.0f);

    switch (Policy)
    {
        case EBlendPolicy::Override:
            // Smoothly transition to new value
            return FMath::Lerp(CurrentValue, NewValue, Weight);

        case EBlendPolicy::Additive:
            // Add new value scaled by weight
            return CurrentValue + NewValue * Weight;

        case EBlendPolicy::Multiplicative:
            // Multiply by new value (lerp towards multiplier effect)
            return CurrentValue * FMath::Lerp(1.0f, NewValue, Weight);

        case EBlendPolicy::Blend:
            // Same as Override for float values
            return FMath::Lerp(CurrentValue, NewValue, Weight);

        case EBlendPolicy::Minimum:
            // Lerp towards minimum of both values
            return FMath::Lerp(CurrentValue, FMath::Min(CurrentValue, NewValue), Weight);

        case EBlendPolicy::Maximum:
            // Lerp towards maximum of both values
            return FMath::Lerp(CurrentValue, FMath::Max(CurrentValue, NewValue), Weight);

        default:
            // Default to Blend behavior
            return FMath::Lerp(CurrentValue, NewValue, Weight);
    }
}

FVector UCameraStage_BlendSolve::ApplyBlendPolicyVector(const FVector& CurrentValue, const FVector& NewValue, EBlendPolicy Policy, float Weight)
{
    // Clamp weight to valid range
    Weight = FMath::Clamp(Weight, 0.0f, 1.0f);

    switch (Policy)
    {
        case EBlendPolicy::Override:
            return FMath::Lerp(CurrentValue, NewValue, Weight);

        case EBlendPolicy::Additive:
            return CurrentValue + NewValue * Weight;

        case EBlendPolicy::Multiplicative:
            return CurrentValue * FMath::Lerp(FVector::OneVector, NewValue, Weight);

        case EBlendPolicy::Blend:
            return FMath::Lerp(CurrentValue, NewValue, Weight);

        case EBlendPolicy::Minimum:
            return FMath::Lerp(CurrentValue, CurrentValue.ComponentMin(NewValue), Weight);

        case EBlendPolicy::Maximum:
            return FMath::Lerp(CurrentValue, CurrentValue.ComponentMax(NewValue), Weight);

        default:
            return FMath::Lerp(CurrentValue, NewValue, Weight);
    }
}

FRotator UCameraStage_BlendSolve::ApplyBlendPolicyRotator(const FRotator& CurrentValue, const FRotator& NewValue, EBlendPolicy Policy, float Weight)
{
    // Clamp weight to valid range
    Weight = FMath::Clamp(Weight, 0.0f, 1.0f);

    switch (Policy)
    {
        case EBlendPolicy::Override:
        case EBlendPolicy::Blend:
            // Use RInterpTo for smooth rotation blending
            return FMath::Lerp(CurrentValue, NewValue, Weight);

        case EBlendPolicy::Additive:
            // Add rotation (useful for shake effects)
            return CurrentValue + NewValue * Weight;

        case EBlendPolicy::Multiplicative:
            // Not meaningful for rotators, treat as blend
            return FMath::Lerp(CurrentValue, NewValue, Weight);

        case EBlendPolicy::Minimum:
            // Take minimum of each component
            return FRotator(
                FMath::Lerp(CurrentValue.Pitch, FMath::Min(CurrentValue.Pitch, NewValue.Pitch), Weight),
                FMath::Lerp(CurrentValue.Yaw, FMath::Min(CurrentValue.Yaw, NewValue.Yaw), Weight),
                FMath::Lerp(CurrentValue.Roll, FMath::Min(CurrentValue.Roll, NewValue.Roll), Weight)
            );

        case EBlendPolicy::Maximum:
            // Take maximum of each component
            return FRotator(
                FMath::Lerp(CurrentValue.Pitch, FMath::Max(CurrentValue.Pitch, NewValue.Pitch), Weight),
                FMath::Lerp(CurrentValue.Yaw, FMath::Max(CurrentValue.Yaw, NewValue.Yaw), Weight),
                FMath::Lerp(CurrentValue.Roll, FMath::Max(CurrentValue.Roll, NewValue.Roll), Weight)
            );

        default:
            return FMath::Lerp(CurrentValue, NewValue, Weight);
    }
}

//========================================
// Conflict Resolution 冲突解决
//========================================

void UCameraStage_BlendSolve::ResolveOverrideConflicts(TArray<FModuleOutput>& Outputs)
{
    // When multiple outputs with Override policy exist, only keep the highest priority one
    // Others are converted to Blend policy
    
    TArray<int32> OverrideIndices;
    
    // Find all Override outputs
    for (int32 i = 0; i < Outputs.Num(); ++i)
    {
        if (Outputs[i].BlendPolicy == EBlendPolicy::Override && Outputs[i].Weight > 0.0f)
        {
            OverrideIndices.Add(i);
        }
    }

    // If multiple overrides, convert all but the last (highest priority after sort) to Blend
    if (OverrideIndices.Num() > 1)
    {
        for (int32 i = 0; i < OverrideIndices.Num() - 1; ++i)
        {
            Outputs[OverrideIndices[i]].BlendPolicy = EBlendPolicy::Blend;
            OverrideConflictsResolved++;
        }

        UE_LOG(LogCameraStage_Blend, Verbose, TEXT("ResolveOverrideConflicts: Resolved %d conflicts"),
            OverrideConflictsResolved);
    }
}

bool UCameraStage_BlendSolve::ShouldModifierOverride(const FModifierOutput& Modifier) const
{
    // Check if modifier wants to completely override camera
    return Modifier.bOverrideCamera && Modifier.CurrentWeight >= 0.99f;
}
