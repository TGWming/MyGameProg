// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Modifiers/CameraModifierBase.h"
#include "CameraModifierManager.generated.h"

// Forward declarations
class USoulsCameraManager;
struct FStageExecutionContext;

/**
 * UCameraModifierManager - Central manager for all camera modifiers
 * 
 * This class is responsible for:
 * - Creating and registering all 26 camera modifiers
 * - Handling modifier triggering from game events
 * - Updating all active modifiers each frame
 * - Providing active modifier outputs to Stage 4 (ModifierApply)
 * - Managing modifier priorities and conflicts
 * 
 * Modifier Categories (26 total):
 * - Shake (S01-S05): 5 modifiers
 * - Reaction (R01-R06): 6 modifiers
 * - Cinematic (C01-C05): 5 modifiers
 * - Zoom (Z01-Z04): 4 modifiers
 * - Effect (E01-E03): 3 modifiers
 * - Special (X01-X03): 3 modifiers
 * 
 * Usage:
 * 1. Created by SoulsCameraManager in CreateSubSystems()
 * 2. Stage 4 calls UpdateModifiers() each frame
 * 3. Game systems call TriggerModifier() when events occur
 * 4. Stage 4 retrieves outputs via GetActiveModifierOutputs()
 * 
 * Example:
 * @code
 * // Trigger a camera shake when player takes damage
 * ModifierManager->TriggerModifier(ECameraModifierID::Modifier_S02_Shake_Hit_Heavy, 
 *     FModifierTriggerData::MakeSimple(DamageIntensity));
 * @endcode
 */
UCLASS(BlueprintType)
class SOUL_API UCameraModifierManager : public UObject
{
	GENERATED_BODY()

public:
	/** Constructor */
	UCameraModifierManager();

	//========================================
	// Initialization
	//========================================

	/**
	 * Initialize the manager and create all modifiers
	 * @param InCameraManager Reference to the camera manager
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	void Initialize(USoulsCameraManager* InCameraManager);

	/**
	 * Check if manager is initialized
	 * @return True if Initialize() has been called successfully
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	bool IsInitialized() const { return bIsInitialized; }

	/**
	 * Get the number of registered modifiers
	 * @return Number of modifiers (should be 26 after initialization)
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	int32 GetModifierCount() const { return RegisteredModifiers.Num(); }

	//========================================
	// Modifier Registration
	//========================================

	/**
	 * Register a modifier instance
	 * @param Modifier The modifier to register
	 * @return True if registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	bool RegisterModifier(UCameraModifierBase* Modifier);

	/**
	 * Unregister a modifier by ID
	 * @param ModifierID ID of modifier to unregister
	 * @return True if unregistered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	bool UnregisterModifier(ECameraModifierID ModifierID);

	/**
	 * Get a modifier by ID
	 * @param ModifierID ID of modifier to get
	 * @return Modifier instance, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	UCameraModifierBase* GetModifier(ECameraModifierID ModifierID) const;

	/**
	 * Get all registered modifiers
	 * @return Array of all registered modifiers
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	TArray<UCameraModifierBase*> GetAllModifiers() const;

	/**
	 * Get all modifiers in a specific category
	 * @param Category Category to filter by
	 * @return Array of modifiers in the category
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	TArray<UCameraModifierBase*> GetModifiersInCategory(EModifierCategory Category) const;

	//========================================
	// Modifier Triggering
	//========================================

	/**
	 * Trigger a modifier by ID with full trigger data
	 * @param ModifierID ID of modifier to trigger
	 * @param TriggerData Configuration for this trigger
	 * @return True if modifier was triggered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	bool TriggerModifier(ECameraModifierID ModifierID, const FModifierTriggerData& TriggerData);

	/**
	 * Trigger a modifier with simple intensity (convenience function)
	 * @param ModifierID ID of modifier to trigger
	 * @param Intensity Effect intensity (default 1.0)
	 * @return True if modifier was triggered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	bool TriggerModifierSimple(ECameraModifierID ModifierID, float Intensity = 1.0f);

	/**
	 * Trigger a modifier with intensity and duration
	 * @param ModifierID ID of modifier to trigger
	 * @param Intensity Effect intensity
	 * @param Duration Effect duration in seconds
	 * @return True if modifier was triggered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	bool TriggerModifierWithDuration(ECameraModifierID ModifierID, float Intensity, float Duration);

	/**
	 * Stop a specific modifier
	 * @param ModifierID ID of modifier to stop
	 * @param bImmediate If true, stop immediately without blend out
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	void StopModifier(ECameraModifierID ModifierID, bool bImmediate = false);

	/**
	 * Stop all active modifiers
	 * @param bImmediate If true, stop immediately without blend out
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	void StopAllModifiers(bool bImmediate = false);

	/**
	 * Stop all modifiers in a specific category
	 * @param Category Category to stop
	 * @param bImmediate If true, stop immediately without blend out
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	void StopModifiersInCategory(EModifierCategory Category, bool bImmediate = false);

	//========================================
	// Update & Query
	//========================================

	/**
	 * Update all modifiers (called each frame by Stage 4)
	 * @param DeltaTime Time since last frame
	 * @param Context Current camera execution context
	 */
	void UpdateModifiers(float DeltaTime, const FStageExecutionContext& Context);

	/**
	 * Get all currently active modifiers
	 * @param OutActiveModifiers Output array of active modifiers
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager")
	void GetActiveModifiers(TArray<UCameraModifierBase*>& OutActiveModifiers) const;

	/**
	 * Get outputs from all active modifiers
	 * @param Context Current camera execution context
	 * @param OutOutputs Output array of modifier outputs
	 */
	void GetActiveModifierOutputs(const FStageExecutionContext& Context, TArray<FModifierOutput>& OutOutputs);

	/**
	 * Check if any modifiers are currently active
	 * @return True if at least one modifier is active
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	bool HasActiveModifiers() const;

	/**
	 * Check if a specific modifier is currently active
	 * @param ModifierID ID to check
	 * @return True if the modifier is active
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	bool IsModifierActive(ECameraModifierID ModifierID) const;

	/**
	 * Get the number of currently active modifiers
	 * @return Count of active modifiers
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierManager")
	int32 GetActiveModifierCount() const;

	//========================================
	// Convenience Trigger Functions
	//========================================

	/**
	 * Trigger a camera shake effect
	 * @param ShakeID Which shake modifier to trigger (S01-S05)
	 * @param Intensity Shake intensity (default 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerShake(ECameraModifierID ShakeID, float Intensity = 1.0f);

	/**
	 * Trigger a hit reaction effect
	 * @param bHeavyHit True for heavy hit (S02), false for light hit (S01)
	 * @param Intensity Effect intensity
	 * @param HitDirection Direction of the hit (for directional shake)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerHitReaction(bool bHeavyHit, float Intensity = 1.0f, FVector HitDirection = FVector::ZeroVector);

	/**
	 * Trigger a parry effect
	 * @param bPerfectParry True for perfect parry (R02), false for normal (R01)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerParryEffect(bool bPerfectParry = false);

	/**
	 * Trigger a stagger effect
	 * @param Intensity Effect intensity (default 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerStaggerEffect(float Intensity = 1.0f);

	/**
	 * Trigger a knockback effect with direction
	 * @param Intensity Effect intensity (default 1.0)
	 * @param Direction Knockback direction vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerKnockbackEffect(float Intensity = 1.0f, FVector Direction = FVector::ZeroVector);

	/**
	 * Trigger a knockdown effect
	 * @param Intensity Effect intensity (default 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerKnockdownEffect(float Intensity = 1.0f);

	/**
	 * Trigger a guard break effect
	 * @param Intensity Effect intensity (default 1.0)
	 * @param Attacker The attacking actor that caused the guard break
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerGuardBreakEffect(float Intensity = 1.0f, AActor* Attacker = nullptr);

	/**
	 * Trigger an execution/finisher camera
	 * @param Victim The target being executed
	 * @param ExecutionID Type of execution (C01=Execution, C02=Backstab, C03=Riposte)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerExecutionCamera(AActor* Victim, ECameraModifierID ExecutionID = ECameraModifierID::Modifier_C01_Cinematic_Execution);

	/**
	 * Trigger slow motion effect
	 * @param Duration How long the slow motion lasts
	 * @param TimeDilation Time scale (0.1 = 10% speed, 0.5 = 50% speed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerSlowMotion(float Duration = 0.5f, float TimeDilation = 0.3f);

	/**
	 * Trigger hit stop (freeze frame) effect
	 * @param Duration How long to freeze (typically 0.05-0.15 seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerHitStop(float Duration = 0.1f);

	/**
	 * Trigger low health visual effect
	 * @param HealthPercent Current health percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerLowHealthEffect(float HealthPercent);

	/**
	 * Trigger boss intro camera
	 * @param Boss The boss actor
	 * @param Duration Intro duration
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModifierManager|Quick")
	void TriggerBossIntro(AActor* Boss, float Duration = 3.0f);

protected:
	//========================================
	// Internal Methods
	//========================================

	/**
	 * Create all modifier instances
	 * Called by Initialize()
	 * @param Outer Outer object for NewObject calls
	 */
	void CreateAllModifiers(UObject* Outer);

	/**
	 * Refresh the cached list of active modifiers
	 * Called after Update to maintain the active list
	 */
	void RefreshActiveModifiersList();

	/**
	 * Sort active modifiers by priority (ascending)
	 * Lower priority modifiers are applied first, higher ones can override
	 */
	void SortActiveModifiersByPriority();

	/**
	 * Check if a modifier can be triggered (handles stacking rules)
	 * @param Modifier The modifier to check
	 * @return True if the modifier can be triggered
	 */
	bool CanTriggerModifier(UCameraModifierBase* Modifier) const;

private:
	//========================================
	// State
	//========================================

	/** Map of registered modifiers by ID for O(1) lookup */
	UPROPERTY()
	TMap<ECameraModifierID, UCameraModifierBase*> RegisteredModifiers;

	/** Cached list of currently active modifiers (refreshed each frame) */
	UPROPERTY()
	TArray<UCameraModifierBase*> ActiveModifiersCache;

	/** Reference to the camera manager */
	UPROPERTY()
	USoulsCameraManager* CameraManager;

	/** Is manager initialized */
	UPROPERTY()
	bool bIsInitialized;
};
