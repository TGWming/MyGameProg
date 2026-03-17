// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_ModifierApply.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "Camera/Core/CameraModifierManager.h"
#include "Camera/Modifiers/CameraModifierBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_Modifier, Log, All);

//========================================
// Constructor
//========================================

UCameraStage_ModifierApply::UCameraStage_ModifierApply()
    : ModifierManager(nullptr)
    , ModifiersUpdatedCount(0)
    , ModifiersAppliedCount(0)
    , TotalModifierTimeMs(0.0f)
    , PlaceholderActiveModifierCount(0)
{
}

//========================================
// ICameraStage Interface
//========================================

bool UCameraStage_ModifierApply::ShouldExecute(const FStageExecutionContext& Context) const
{
    // Skip if no modifiers are active
    // This stage is optional and can be skipped for performance
    const bool bHasModifiers = HasActiveModifiers();
    
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("ShouldExecute: HasActiveModifiers=%s"),
        bHasModifiers ? TEXT("Yes") : TEXT("No"));
    
    return bHasModifiers;
}

EStageResult UCameraStage_ModifierApply::Execute(FStageExecutionContext& Context)
{
    // 【诊断日志】记录Stage开始时的Distance
    float DistanceAtStart = Context.Output.Distance;

    // Reset statistics
    ModifiersUpdatedCount = 0;
    ModifiersAppliedCount = 0;
    TotalModifierTimeMs = 0.0f;

    // Clear previous frame's modifier outputs
    Context.ModifierOutputs.Empty();
    
    // Reset override state
    Context.bHasOverrideModifier = false;
    Context.OverrideTransform = FTransform::Identity;

    // Check if we have a valid modifier manager
    if (!ModifierManager)
    {
        UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("Execute: No ModifierManager available"));
        // Fallback to placeholder methods for testing
        ApplyShakeModifiers(Context);
        ApplyReactionModifiers(Context);
        ApplyCinematicModifiers(Context);
        ApplyZoomModifiers(Context);
        ApplyEffectModifiers(Context);
        ApplySpecialModifiers(Context);
        
        // 【诊断日志】在return之前输出Distance变化
        static int32 Stage4LogCount_NoManager = 0;
        if (Stage4LogCount_NoManager < 10)
        {
            Stage4LogCount_NoManager++;
            UE_LOG(LogTemp, Error, TEXT("【Stage4 ModifierApply】 Distance: %.1f -> %.1f"), DistanceAtStart, Context.Output.Distance);
        }
        
        return EStageResult::Success;
    }

    const double StartTime = FPlatformTime::Seconds();

    // Update all modifiers for this frame
    UpdateModifiers(Context);

    // Check for override modifiers first (cinematics, death cam)
    if (HandleOverrideModifiers(Context))
    {
        // Override modifier is active, it takes full control
        TotalModifierTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);
        
        UE_LOG(LogCameraStage_Modifier, Verbose, TEXT("Execute: Override modifier active, time=%.2fms"),
            TotalModifierTimeMs);
        
        // 【诊断日志】在return之前输出Distance变化
        static int32 Stage4LogCount_Override = 0;
        if (Stage4LogCount_Override < 10)
        {
            Stage4LogCount_Override++;
            UE_LOG(LogTemp, Error, TEXT("【Stage4 ModifierApply】 Distance: %.1f -> %.1f"), DistanceAtStart, Context.Output.Distance);
        }
        
        return EStageResult::Success;
    }

    // Collect outputs from all active non-override modifiers
    CollectModifierOutputs(Context);

    TotalModifierTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

    UE_LOG(LogCameraStage_Modifier, Verbose, TEXT("Execute: Updated %d, Applied %d modifiers in %.2fms"),
        ModifiersUpdatedCount, ModifiersAppliedCount, TotalModifierTimeMs);

    // 【诊断日志】在return之前输出Distance变化
    static int32 Stage4LogCount = 0;
    if (Stage4LogCount < 10)
    {
        Stage4LogCount++;
        UE_LOG(LogTemp, Error, TEXT("【Stage4 ModifierApply】 Distance: %.1f -> %.1f"), DistanceAtStart, Context.Output.Distance);
    }

    return EStageResult::Success;
}

void UCameraStage_ModifierApply::OnPreExecute(const FStageExecutionContext& Context)
{
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("Stage 4 ModifierApply: PreExecute"));
}

void UCameraStage_ModifierApply::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("Stage 4 ModifierApply: PostExecute - Outputs: %d, Override: %s"),
        Context.ModifierOutputs.Num(),
        Context.bHasOverrideModifier ? TEXT("Yes") : TEXT("No"));
}

//========================================
// Modifier Manager Connection
//========================================

void UCameraStage_ModifierApply::SetModifierManager(UCameraModifierManager* InManager)
{
    ModifierManager = InManager;
    
    if (ModifierManager)
    {
        UE_LOG(LogCameraStage_Modifier, Log, TEXT("Stage4_ModifierApply: ModifierManager connected with %d modifiers"),
            ModifierManager->GetModifierCount());
    }
    else
    {
        UE_LOG(LogCameraStage_Modifier, Warning, TEXT("Stage4_ModifierApply: ModifierManager set to nullptr"));
    }
}

bool UCameraStage_ModifierApply::HasActiveModifiers() const
{
    if (ModifierManager)
    {
        return ModifierManager->HasActiveModifiers();
    }
    
    // Placeholder: return false (no modifiers active by default)
    return PlaceholderActiveModifierCount > 0;
}

int32 UCameraStage_ModifierApply::GetActiveModifierCount() const
{
    if (ModifierManager)
    {
        return ModifierManager->GetActiveModifierCount();
    }
    
    return PlaceholderActiveModifierCount;
}

//========================================
// Internal Methods
//========================================

void UCameraStage_ModifierApply::UpdateModifiers(FStageExecutionContext& Context)
{
    if (!ModifierManager)
    {
        return;
    }

    // Update all modifiers (this updates their internal state)
    ModifierManager->UpdateModifiers(Context.DeltaTime, Context);
    
    // Refresh cached active modifiers list
    CachedActiveModifiers.Reset();
    ModifierManager->GetActiveModifiers(CachedActiveModifiers);
    
    ModifiersUpdatedCount = CachedActiveModifiers.Num();
    
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("UpdateModifiers: %d active modifiers"),
        ModifiersUpdatedCount);
}

void UCameraStage_ModifierApply::CollectModifierOutputs(FStageExecutionContext& Context)
{
    if (!ModifierManager)
    {
        return;
    }

    // Get outputs from modifier manager (already sorted by priority)
    ModifierManager->GetActiveModifierOutputs(Context, Context.ModifierOutputs);
    ModifiersAppliedCount = Context.ModifierOutputs.Num();

    // Log active modifiers (verbose)
    if (Context.ModifierOutputs.Num() > 0)
    {
        UE_LOG(LogCameraStage_Modifier, Verbose, TEXT("Stage4_ModifierApply: Collected %d modifier outputs"),
            Context.ModifierOutputs.Num());

        for (const FModifierOutput& Output : Context.ModifierOutputs)
        {
            UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("  - %s: Weight=%.2f, FOV=%.1f, Pos=(%.1f,%.1f,%.1f)"),
                *UEnum::GetValueAsString(Output.ModifierType),
                Output.CurrentWeight,
                Output.FOVEffect,
                Output.PositionEffect.X, Output.PositionEffect.Y, Output.PositionEffect.Z);
        }
    }
}

bool UCameraStage_ModifierApply::HandleOverrideModifiers(FStageExecutionContext& Context)
{
    // Check for override modifiers in the active list
    for (UCameraModifierBase* Modifier : CachedActiveModifiers)
    {
        if (!Modifier)
        {
            continue;
        }
        
        // Check if this is an override modifier that is currently overriding
        if (Modifier->IsOverrideModifier() && Modifier->IsActive())
        {
            // Get the override output
            FModifierOutput OverrideOutput;
            if (Modifier->GetOutput(Context, OverrideOutput))
            {
                // Check if the output indicates override is active
                if (OverrideOutput.bOverrideCamera)
                {
                    // Mark as override in context
                    Context.bHasOverrideModifier = true;
                    Context.OverrideTransform = OverrideOutput.OverrideTransform;

                    // Still add the output for visual effects (vignette, color, etc.)
                    Context.ModifierOutputs.Add(OverrideOutput);
                    ModifiersAppliedCount = 1;

                    UE_LOG(LogCameraStage_Modifier, Verbose, TEXT("Stage4_ModifierApply: Override modifier active - %s"),
                        *Modifier->GetModifierDescription());

                    return true;
                }
            }
        }
    }

    return false;
}

//========================================
// Legacy Placeholder Methods
//========================================

void UCameraStage_ModifierApply::ApplyShakeModifiers(FStageExecutionContext& Context)
{
    // Placeholder: Shake modifiers (S01-S05)
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("ApplyShakeModifiers: Placeholder (no manager)"));
}

void UCameraStage_ModifierApply::ApplyReactionModifiers(FStageExecutionContext& Context)
{
    // Placeholder: Reaction modifiers (R01-R06)
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("ApplyReactionModifiers: Placeholder (no manager)"));
}

void UCameraStage_ModifierApply::ApplyCinematicModifiers(FStageExecutionContext& Context)
{
    // Placeholder: Cinematic modifiers (C01-C05)
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("ApplyCinematicModifiers: Placeholder (no manager)"));
}

void UCameraStage_ModifierApply::ApplyZoomModifiers(FStageExecutionContext& Context)
{
    // Placeholder: Zoom modifiers (Z01-Z04)
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("ApplyZoomModifiers: Placeholder (no manager)"));
}

void UCameraStage_ModifierApply::ApplyEffectModifiers(FStageExecutionContext& Context)
{
    // Placeholder: Effect modifiers (E01-E03)
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("ApplyEffectModifiers: Placeholder (no manager)"));
}

void UCameraStage_ModifierApply::ApplySpecialModifiers(FStageExecutionContext& Context)
{
    // Placeholder: Special modifiers (X01-X03)
    UE_LOG(LogCameraStage_Modifier, VeryVerbose, TEXT("ApplySpecialModifiers: Placeholder (no manager)"));
}
