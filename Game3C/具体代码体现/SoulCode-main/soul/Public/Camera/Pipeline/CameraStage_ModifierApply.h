// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "Camera/Data/CameraStateEnums.h"
#include "CameraStage_ModifierApply.generated.h"

// Forward declarations
class UCameraModifierManager;
class UCameraModifierBase;

/**
 * UCameraStage_ModifierApply - Stage 4: Modifier Apply
 * 
 * Fourth stage in the pipeline. Applies active camera modifiers:
 * 
 * - Shake modifiers (S01-S05): Camera shake effects
 * - Reaction modifiers (R01-R06): Combat feedback reactions
 * - Cinematic modifiers (C01-C05): Cutscene camera overrides
 * - Zoom modifiers (Z01-Z04): FOV/distance pulses
 * - Effect modifiers (E01-E03): Post-process effects
 * - Special modifiers (X01-X03): Slow-mo, hit-stop, death cam
 * 
 * Each modifier produces FModifierOutput which is collected for application.
 * 
 * Layer: COMPUTE
 * Required: No
 * Skippable: Yes (when no active modifiers)
 * 
 * Execution Flow:
 * 1. Clear previous frame's ModifierOutputs
 * 2. Update all modifiers via ModifierManager
 * 3. Check for override modifiers (cinematics, death cam)
 *    - If override active: set Context flags and return
 * 4. Collect outputs from all active non-override modifiers
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_ModifierApply : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_ModifierApply();

    //========================================
    // ICameraStage Interface
    //========================================

    /** Get stage index (4 for ModifierApply) */
    virtual int32 GetStageIndex() const override { return 4; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("ModifierApply")); }
    
    /** This stage CAN be skipped (first skippable stage) */
    virtual bool CanBeSkipped() const override { return true; }
    
    /** Check if this stage should execute (skip if no active modifiers) */
    virtual bool ShouldExecute(const FStageExecutionContext& Context) const override;
    
    /** Execute the modifier apply stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

    //========================================
    // Modifier Manager Connection
    //========================================

    /** 
     * Set reference to modifier manager
     * Called by SoulsCameraManager during initialization
     * @param InManager Modifier manager instance
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Stage")
    void SetModifierManager(UCameraModifierManager* InManager);

    /**
     * Get the modifier manager reference
     * @return Modifier manager instance, or nullptr if not set
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Stage")
    UCameraModifierManager* GetModifierManager() const { return ModifierManager; }

    /** 
     * Check if any modifiers are currently active
     * @return True if at least one modifier is active
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Stage")
    bool HasActiveModifiers() const;

    /**
     * Get count of active modifiers
     * @return Number of active modifiers
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Stage")
    int32 GetActiveModifierCount() const;

protected:
    //========================================
    // Internal Methods
    //========================================

    /** 
     * Update all modifier lifecycles (blend in/out, duration)
     * @param Context Current execution context
     */
    void UpdateModifiers(FStageExecutionContext& Context);

    /**
     * Collect outputs from all active modifiers
     * @param Context Current execution context
     */
    void CollectModifierOutputs(FStageExecutionContext& Context);

    /**
     * Handle override modifiers (cinematics, death cam)
     * Override modifiers take full control of the camera transform
     * @param Context Current execution context
     * @return True if an override modifier is active
     */
    bool HandleOverrideModifiers(FStageExecutionContext& Context);

    //========================================
    // Legacy Placeholder Methods (for backward compatibility)
    //========================================

    void ApplyShakeModifiers(FStageExecutionContext& Context);
    void ApplyReactionModifiers(FStageExecutionContext& Context);
    void ApplyCinematicModifiers(FStageExecutionContext& Context);
    void ApplyZoomModifiers(FStageExecutionContext& Context);
    void ApplyEffectModifiers(FStageExecutionContext& Context);
    void ApplySpecialModifiers(FStageExecutionContext& Context);

private:
    //========================================
    // Modifier Manager
    //========================================

    /** Reference to modifier manager */
    UPROPERTY()
    UCameraModifierManager* ModifierManager;

    /** Cached array of active modifiers (refreshed each frame) */
    UPROPERTY()
    TArray<UCameraModifierBase*> CachedActiveModifiers;

    //========================================
    // Execution Statistics
    //========================================

    /** Number of modifiers updated this frame */
    int32 ModifiersUpdatedCount;

    /** Number of modifiers that produced output this frame */
    int32 ModifiersAppliedCount;

    /** Total modifier execution time (ms) for profiling */
    float TotalModifierTimeMs;

    /** Placeholder: Simulated active modifier count (for testing when no manager) */
    int32 PlaceholderActiveModifierCount;
};
