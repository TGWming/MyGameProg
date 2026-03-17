// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "Camera/Pipeline/ICameraStage.h"
#include "CameraModifierBase.generated.h"

// Forward declarations
class USoulsCameraManager;

/**
 * EModifierCategory - Modifier category for organization
 * 
 * Categories help organize the 26 modifiers into logical groups:
 * - Shake: Camera shake effects from various sources
 * - Reaction: Combat feedback and hit reactions
 * - Cinematic: Cutscene and special kill cameras
 * - Zoom: FOV and distance pulse effects
 * - Effect: Post-process visual effects
 * - Special: Time manipulation and death cameras
 */
UENUM(BlueprintType)
enum class EModifierCategory : uint8
{
	Shake       UMETA(DisplayName = "Shake"),       // S01-S05: Camera shake effects
	Reaction    UMETA(DisplayName = "Reaction"),    // R01-R06: Combat feedback reactions
	Cinematic   UMETA(DisplayName = "Cinematic"),   // C01-C05: Cutscene camera overrides
	Zoom        UMETA(DisplayName = "Zoom"),        // Z01-Z04: FOV/distance pulses
	Effect      UMETA(DisplayName = "Effect"),      // E01-E03: Post-process effects
	Special     UMETA(DisplayName = "Special")      // X01-X03: Slow-mo, hit-stop, death cam
};

/**
 * FModifierTriggerData - Data passed when activating a modifier
 * 
 * Contains all the information needed to configure a modifier when triggered.
 * Some fields are optional and only used by specific modifier types.
 */
USTRUCT(BlueprintType)
struct SOUL_API FModifierTriggerData
{
	GENERATED_BODY()

	//========================================
	// Core Parameters
	//========================================

	/** Intensity multiplier (0-1 typical, can exceed 1 for stronger effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float Intensity;

	/** Optional duration override in seconds (0 = use modifier's default) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float DurationOverride;

	//========================================
	// Spatial Parameters (for directional effects)
	//========================================

	/** Source location for directional effects (e.g., impact point) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Spatial")
	FVector SourceLocation;

	/** Direction of the effect (e.g., hit direction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Spatial")
	FVector SourceDirection;

	//========================================
	// Actor References
	//========================================

	/** Source actor that caused this trigger (e.g., attacker) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Actors")
	TWeakObjectPtr<AActor> SourceActor;

	/** Target actor affected by this trigger (e.g., victim) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Actors")
	TWeakObjectPtr<AActor> TargetActor;

	//========================================
	// Custom Data
	//========================================

	/** Custom tag for specific modifier behaviors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Custom")
	FName CustomTag;

	/** Custom float parameter for modifier-specific use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger|Custom")
	float CustomFloat;

	//========================================
	// Constructor & Factory Methods
	//========================================

	FModifierTriggerData()
		: Intensity(1.0f)
		, DurationOverride(0.0f)
		, SourceLocation(FVector::ZeroVector)
		, SourceDirection(FVector::ForwardVector)
		, SourceActor(nullptr)
		, TargetActor(nullptr)
		, CustomTag(NAME_None)
		, CustomFloat(0.0f)
	{}

	/** Create simple trigger with just intensity */
	static FModifierTriggerData MakeSimple(float InIntensity = 1.0f)
	{
		FModifierTriggerData Data;
		Data.Intensity = FMath::Max(0.0f, InIntensity);
		return Data;
	}

	/** Create trigger with intensity and duration */
	static FModifierTriggerData MakeWithDuration(float InIntensity, float InDuration)
	{
		FModifierTriggerData Data;
		Data.Intensity = FMath::Max(0.0f, InIntensity);
		Data.DurationOverride = FMath::Max(0.0f, InDuration);
		return Data;
	}

	/** Create directional trigger (for impact effects) */
	static FModifierTriggerData MakeDirectional(float InIntensity, const FVector& InLocation, const FVector& InDirection)
	{
		FModifierTriggerData Data;
		Data.Intensity = FMath::Max(0.0f, InIntensity);
		Data.SourceLocation = InLocation;
		Data.SourceDirection = InDirection.GetSafeNormal();
		return Data;
	}
};

/**
 * UCameraModifierBase - Abstract base class for all camera modifiers
 * 
 * Camera modifiers are event-triggered temporary effects that overlay
 * on top of the base camera behavior. Unlike modules (which are state-based),
 * modifiers have a lifecycle: Trigger -> BlendIn -> Active -> BlendOut -> Inactive
 * 
 * Modifier Categories (26 total):
 * - Shake (S01-S05): Camera shake from hits, attacks, landing, environment
 * - Reaction (R01-R06): Combat feedback like parry, stagger, knockback
 * - Cinematic (C01-C05): Execution, backstab, riposte, boss intro cameras
 * - Zoom (Z01-Z04): FOV pulses for impact, charge release, skills
 * - Effect (E01-E03): Post-process effects like low health, status ailments
 * - Special (X01-X03): Slow motion, hit stop, death camera
 * 
 * Key Differences from Modules:
 * - Modules: State-based, always computing based on game state
 * - Modifiers: Event-triggered, temporary effects with lifecycle
 * 
 * Lifecycle:
 * 1. Inactive: Waiting for Trigger()
 * 2. BlendingIn: Weight increasing from 0 to 1
 * 3. Active: Full effect, weight = 1
 * 4. BlendingOut: Weight decreasing from 1 to 0
 * 5. Back to Inactive
 * 
 * Note: Uses EModifierState from SoulsCameraRuntimeTypes.h
 * Note: Uses ECameraModifierID from CameraStateEnums.h for specific modifier identification
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SOUL_API UCameraModifierBase : public UObject
{
	GENERATED_BODY()

public:
	/** Constructor */
	UCameraModifierBase();

	//========================================
	// Modifier Identity
	//========================================

	/** Get the modifier ID enum value (specific modifier identifier) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	virtual ECameraModifierID GetModifierID() const PURE_VIRTUAL(UCameraModifierBase::GetModifierID, return ECameraModifierID::None;);

	/** Get the modifier type/category for output classification */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	virtual ECameraModifierType GetModifierType() const PURE_VIRTUAL(UCameraModifierBase::GetModifierType, return ECameraModifierType::None;);

	/** Get the modifier category for organization */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	virtual EModifierCategory GetModifierCategory() const PURE_VIRTUAL(UCameraModifierBase::GetModifierCategory, return EModifierCategory::Shake;);

	/** Get human-readable modifier name */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	virtual FName GetModifierName() const;

	/** Get modifier description */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	virtual FString GetModifierDescription() const;

	//========================================
	// Lifecycle Control
	//========================================

	/**
	 * Trigger/activate this modifier
	 * @param InTriggerData Configuration data for this activation
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Modifier")
	virtual void Trigger(const FModifierTriggerData& InTriggerData);

	/**
	 * Stop this modifier
	 * @param bImmediate If true, skip blend out and go directly to Inactive
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Modifier")
	virtual void Stop(bool bImmediate = false);

	/**
	 * Update modifier state (called each frame by ModifierManager)
	 * @param DeltaTime Time since last update
	 * @param Context Current camera execution context
	 */
	virtual void Update(float DeltaTime, const FStageExecutionContext& Context);

	//========================================
	// State Queries
	//========================================

	/** Get current modifier state */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	EModifierState GetState() const { return CurrentState; }

	/** Check if modifier is currently active (any state except Inactive) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	bool IsActive() const { return CurrentState != EModifierState::Inactive; }

	/** Check if modifier is in the Active phase (fully blended in) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	bool IsFullyActive() const { return CurrentState == EModifierState::Active; }

	/** Check if modifier is blending (in or out) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	bool IsBlending() const { return CurrentState == EModifierState::BlendingIn || CurrentState == EModifierState::BlendingOut; }

	/** Check if modifier is paused */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	bool IsPaused() const { return CurrentState == EModifierState::Paused; }

	//========================================
	// Pause Control
	//========================================

	/** Pause this modifier (freezes time but maintains state) */
	UFUNCTION(BlueprintCallable, Category = "Camera|Modifier")
	virtual void Pause();

	/** Resume this modifier from paused state */
	UFUNCTION(BlueprintCallable, Category = "Camera|Modifier")
	virtual void Resume();

	//========================================
	// Output
	//========================================

	/**
	 * Get current modifier output
	 * @param Context Current camera execution context
	 * @param OutOutput Output structure to fill
	 * @return True if modifier has valid output
	 */
	virtual bool GetOutput(const FStageExecutionContext& Context, FModifierOutput& OutOutput);

	/** Get current blend weight (0-1) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	float GetCurrentWeight() const { return CurrentWeight; }

	/** Get current intensity (from trigger data, scaled by weight) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	float GetEffectiveIntensity() const { return CurrentIntensity * CurrentWeight; }

	//========================================
	// Configuration (Override in derived classes)
	//========================================

	/** Get default duration in seconds (can be overridden by trigger data) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier|Config")
	virtual float GetDefaultDuration() const { return 1.0f; }

	/** Get blend in time in seconds */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier|Config")
	virtual float GetBlendInTime() const { return 0.1f; }

	/** Get blend out time in seconds */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier|Config")
	virtual float GetBlendOutTime() const { return 0.2f; }

	/** Can this modifier stack with itself? (multiple instances) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier|Config")
	virtual bool CanStack() const { return false; }

	/** Should this modifier override camera completely? (for cinematics) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier|Config")
	virtual bool IsOverrideModifier() const { return false; }

	/** Get priority (higher = processed later, can override others) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier|Config")
	virtual int32 GetPriority() const { return 100; }

	/** Can this modifier be interrupted by the same type? */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier|Config")
	virtual bool CanBeInterrupted() const { return true; }

protected:
	//========================================
	// Virtual Callbacks for Derived Classes
	//========================================

	/** Called when modifier is triggered - setup initial state */
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData);

	/** Called when modifier enters Active state (blend in complete) */
	virtual void OnActivated();

	/** Called when modifier begins blend out */
	virtual void OnDeactivating();

	/** Called when modifier becomes fully inactive */
	virtual void OnDeactivated();

	/** Called when modifier is paused */
	virtual void OnPaused();

	/** Called when modifier is resumed */
	virtual void OnResumed();

	/**
	 * Compute the actual effect values - MUST override in derived classes
	 * This is where the modifier calculates its camera adjustments
	 * @param DeltaTime Time since last update
	 * @param Context Current camera execution context
	 */
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context);

	//========================================
	// Helper Methods
	//========================================

	/** Calculate blend weight based on current state and timing */
	float CalculateBlendWeight() const;

	/** Get time remaining in current phase */
	float GetTimeRemaining() const;

	/** Transition to a new state */
	void TransitionToState(EModifierState NewState);

	/** Create base output with current settings */
	FModifierOutput CreateBaseOutput() const;

	/** Reset all effect values to defaults */
	void ResetEffectValues();

	//========================================
	// State Variables
	//========================================

	/** Current modifier state */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EModifierState CurrentState;

	/** State before pause (for resume) */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EModifierState PrePauseState;

	/** Current blend weight (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float CurrentWeight;

	/** Time spent in current state */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float StateTime;

	/** Total time this modifier has been active (across all states) */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float TotalActiveTime;

	/** Active duration for this trigger (from trigger data or default) */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float ActiveDuration;

	/** Current intensity (from trigger data) */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float CurrentIntensity;

	/** Stored trigger data for this activation */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FModifierTriggerData TriggerData;

	//========================================
	// Effect Values (set by derived classes in ComputeEffect)
	//========================================

	/** Position offset effect */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	FVector PositionEffect;

	/** Rotation offset effect */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	FRotator RotationEffect;

	/** Distance offset effect */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	float DistanceEffect;

	/** FOV offset effect */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	float FOVEffect;

	/** Vignette intensity effect (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	float VignetteEffect;

	/** Color tint effect */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	FLinearColor ColorTintEffect;

	/** Saturation effect (1 = normal) */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	float SaturationEffect;

	/** Time dilation effect (1 = normal) */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	float TimeDilationEffect;

	/** Override transform (for cinematic modifiers) */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	FTransform OverrideTransform;

	/** Whether this modifier is currently overriding the camera */
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	bool bIsOverriding;

	//========================================
	// Reference to Manager
	//========================================

	/** Reference to camera manager (set when added to ModifierManager) */
	UPROPERTY()
	USoulsCameraManager* CameraManager;

	/** Allow ModifierManager to set the camera manager reference */
	friend class UCameraModifierManager;
};
