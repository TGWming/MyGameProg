// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierBase.h"
#include "CameraModifier_Effect.generated.h"

/**
 * Effect Modifier Declarations (E01-E03)
 * 
 * This file contains the 3 Effect modifiers for the camera system.
 * Effect modifiers provide persistent visual effects that reflect
 * player state or environmental conditions.
 * 
 * All effect modifiers inherit from UCameraModifier_EffectBase which provides
 * common functionality for post-process style effects.
 * 
 * Key Characteristics:
 * - Long duration or continuous activation
 * - Primarily affect visual post-processing
 * - Can dynamically scale with player state
 * - Lower priority (don't interrupt combat)
 * - Smooth transitions in and out
 * 
 * Effect Types:
 * - E01: LowHealth - Visual feedback when health is critical
 * - E02: StatusAilment - Status effect visual (poison, curse, etc.)
 * - E03: FocusMode - Concentration/aiming mode visual enhancement
 * 
 * Comparison with Other Modifiers:
 * - Shake: Brief position noise
 * - Reaction: Brief directional offset
 * - Zoom: Brief FOV pulse
 * - Effect: Persistent visual state (this category)
 */


//========================================
// Effect Base Class
//========================================

/**
 * UCameraModifier_EffectBase
 * 
 * Abstract base class for all effect modifiers.
 * Provides common functionality for persistent visual effects.
 * 
 * Features:
 * - Persistent or long-duration activation
 * - Dynamic intensity based on game state
 * - Post-process effect support (vignette, saturation, color)
 * - Smooth intensity interpolation
 * - Can be updated while active
 * 
 * Usage Pattern:
 * Unlike other modifiers that trigger and complete, Effect modifiers
 * can be continuously updated while active. Use UpdateIntensity()
 * to change effect strength based on game state (e.g., health percentage).
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraModifier_EffectBase : public UCameraModifierBase
{
	GENERATED_BODY()

public:
	/** Constructor - sets default effect parameters */
	UCameraModifier_EffectBase();

	//========================================
	// Modifier Identity
	//========================================

	/** All effect modifiers belong to the Effect category */
	virtual EModifierCategory GetModifierCategory() const override { return EModifierCategory::Effect; }

	/** Effect modifiers use the Effect type for output classification */
	virtual ECameraModifierType GetModifierType() const override { return ECameraModifierType::Effect; }

	/** Effects have low priority to not interfere with combat feedback */
	virtual int32 GetPriority() const override { return 50; }

	/** Effects can be interrupted by higher priority modifiers */
	virtual bool CanBeInterrupted() const override { return true; }

	//========================================
	// Lifecycle Override
	//========================================

	/**
	 * Override Update to handle smooth intensity transitions
	 * @param DeltaTime Time since last frame
	 * @param Context Current camera execution context
	 */
	virtual void Update(float DeltaTime, const FStageExecutionContext& Context) override;

	/**
	 * Override Trigger to set target intensity
	 * @param InTriggerData Trigger configuration
	 */
	virtual void Trigger(const FModifierTriggerData& InTriggerData) override;

	/**
	 * Override Stop to handle smooth fade out
	 * @param bImmediate If true, skip fade out
	 */
	virtual void Stop(bool bImmediate = false) override;

	//========================================
	// Dynamic Intensity Control
	//========================================

	/**
	 * Update the effect intensity while active
	 * Allows dynamic adjustment based on game state
	 * @param NewIntensity New intensity value (0-1 typical, can exceed for emphasis)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect")
	virtual void UpdateIntensity(float NewIntensity);

	/**
	 * Get current dynamic intensity
	 * @return Current interpolated intensity
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect")
	float GetDynamicIntensity() const { return DynamicIntensity; }

	/**
	 * Get current smoothed intensity
	 * @return Current smoothed intensity value
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect")
	float GetSmoothedIntensity() const { return SmoothedIntensity; }

protected:
	//========================================
	// Effect Computation
	//========================================

	/**
	 * Compute effect values
	 * Calculates post-process effects based on intensity
	 * @param DeltaTime Time since last frame
	 * @param Context Current camera execution context
	 */
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	/**
	 * Called when modifier is triggered
	 * Initializes effect state
	 * @param InTriggerData Trigger configuration
	 */
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;

	//========================================
	// Smooth Intensity Transition
	//========================================

	/**
	 * Update smoothed intensity towards target
	 * Uses FInterpTo for frame-rate independent interpolation
	 * @param DeltaTime Time since last frame
	 */
	void UpdateSmoothedIntensity(float DeltaTime);

	//========================================
	// Effect Helpers
	//========================================

	/**
	 * Calculate pulsing effect for visual feedback
	 * @param BaseValue Base effect value
	 * @param PulseAmount Amount of pulse variation
	 * @param PulseSpeed Speed of pulse (cycles per second)
	 * @return Pulsed value
	 */
	float CalculatePulse(float BaseValue, float PulseAmount, float PulseSpeed) const;

	/**
	 * Calculate heartbeat-style pulse for critical states
	 * @param Intensity Effect intensity
	 * @return Heartbeat curve value (0-1)
	 */
	float CalculateHeartbeatPulse(float Intensity) const;

	//========================================
	// Effect Parameters (Configurable)
	//========================================

	/** Vignette intensity for this effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Vignette", meta = (ClampMin = "0.0", ClampMax = "0.8"))
	float EffectVignetteIntensity;

	/** Saturation adjustment (1.0 = normal, <1.0 = desaturated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Color", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float EffectSaturation;

	/** Color tint for this effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Color")
	FLinearColor EffectColorTint;

	/** Should effect pulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Pulse")
	bool bShouldPulse;

	/** Pulse speed (cycles per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Pulse", meta = (ClampMin = "0.1", ClampMax = "5.0", EditCondition = "bShouldPulse"))
	float PulseSpeed;

	/** Pulse amount (variation from base) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Pulse", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bShouldPulse"))
	float PulseAmount;

	/** Intensity interpolation speed - higher values = faster transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Interpolation", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float IntensityInterpSpeed;

	//========================================
	// Runtime State
	//========================================

	/** Target intensity (can be updated dynamically) */
	UPROPERTY(BlueprintReadOnly, Category = "Effect|State")
	float TargetIntensity;

	/** Current interpolated intensity (legacy, kept for compatibility) */
	UPROPERTY(BlueprintReadOnly, Category = "Effect|State")
	float DynamicIntensity;

	/** Current smoothed intensity (frame-rate independent interpolation) */
	UPROPERTY(BlueprintReadOnly, Category = "Effect|State")
	float SmoothedIntensity;

	/** Pulse timer */
	float PulseTimer;
};


//========================================
// E01: Low Health Effect
//========================================

/**
 * UCameraModifier_E01_Effect_LowHealth
 * 
 * E01: Low Health - Visual feedback when health is critical
 * 
 * Activated when player health drops below threshold.
 * Provides urgent visual feedback without being distracting.
 * 
 * Effects:
 * - Red vignette that pulses with "heartbeat"
 * - Slight desaturation
 * - Subtle FOV reduction (tunnel vision)
 * - Intensity scales with how low health is
 * 
 * Typical Parameters:
 * - Activation threshold: 30% health
 * - Vignette: 0.2-0.4 based on health
 * - Pulse speed: increases as health decreases
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_E01_Effect_LowHealth : public UCameraModifier_EffectBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures low health effect parameters */
	UCameraModifier_E01_Effect_LowHealth();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_E01_Effect_LowHealth; }
	virtual FString GetModifierDescription() const override { return TEXT("Visual feedback for critical health"); }

	//========================================
	// Timing Configuration
	//========================================

	/** Low health effect is continuous while active */
	virtual float GetDefaultDuration() const override { return 0.0f; }  // Continuous
	virtual float GetBlendInTime() const override { return 0.5f; }
	virtual float GetBlendOutTime() const override { return 0.8f; }

	//========================================
	// Priority
	//========================================

	virtual int32 GetPriority() const override { return 45; }

	//========================================
	// Health-Specific Interface
	//========================================

	/**
	 * Update effect based on health percentage
	 * Calculates target intensity from health value
	 * @param HealthPercent Current health percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect|LowHealth")
	void UpdateHealthPercent(float HealthPercent);

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Low Health-Specific Parameters
	//========================================

	/** Health percentage threshold for effect (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowHealth", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float HealthThreshold;

	/** Maximum vignette at lowest health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowHealth", meta = (ClampMin = "0.0", ClampMax = "0.6"))
	float MaxVignetteIntensity;

	/** FOV reduction at lowest health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowHealth", meta = (ClampMin = "0.0", ClampMax = "15.0"))
	float MaxFOVReduction;

	/** Red tint color for vignette */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowHealth")
	FLinearColor LowHealthTint;

	/** Current health percentage (updated via UpdateHealthPercent) */
	float CurrentHealthPercent;
};


//========================================
// E02: Status Ailment Effect
//========================================

/**
 * UCameraModifier_E02_Effect_StatusAilment
 * 
 * E02: Status Ailment - Visual effect for status conditions (poison, curse, etc.)
 * 
 * Activated when player has an active status ailment.
 * Creates unsettling visual feedback to convey danger.
 * 
 * Effects:
 * - Color tint based on ailment type (green for poison, purple for curse)
 * - Slight vignette
 * - Subtle screen distortion (via FOV wobble)
 * - Desaturation
 * - Slow pulse for nausea feel
 * 
 * Typical Parameters:
 * - Poison tint: (0.7, 1.0, 0.7)
 * - Curse tint: (0.8, 0.6, 1.0)
 * - Vignette: 0.15
 * - Slow pulse: 0.5-1.0 Hz
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_E02_Effect_StatusAilment : public UCameraModifier_EffectBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures status ailment effect parameters */
	UCameraModifier_E02_Effect_StatusAilment();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_E02_Effect_StatusAilment; }
	virtual FString GetModifierDescription() const override { return TEXT("Visual effect for status ailments"); }

	//========================================
	// Timing Configuration
	//========================================

	/** Status ailment effect duration based on ailment timer */
	virtual float GetDefaultDuration() const override { return 0.0f; }  // Continuous until cured
	virtual float GetBlendInTime() const override { return 0.3f; }
	virtual float GetBlendOutTime() const override { return 1.0f; }

	//========================================
	// Priority
	//========================================

	virtual int32 GetPriority() const override { return 48; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Status Ailment-Specific Parameters
	//========================================

	/** Poison color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|StatusAilment")
	FLinearColor PoisonTint;

	/** Curse color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|StatusAilment")
	FLinearColor CurseTint;

	/** Vignette intensity for status ailment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|StatusAilment", meta = (ClampMin = "0.0", ClampMax = "0.4"))
	float AilmentVignetteIntensity;

	/** Desaturation amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|StatusAilment", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float AilmentDesaturation;

	/** FOV wobble amount for nausea */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|StatusAilment", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float FOVWobbleAmount;

	/** Wobble speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|StatusAilment", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float WobbleSpeed;

	/** Current ailment severity (updated via UpdateIntensity) */
	float AilmentSeverity;
};


//========================================
// E03: Focus Mode Effect
//========================================

/**
 * UCameraModifier_E03_Effect_FocusMode
 * 
 * E03: Focus Mode - Concentration/aiming mode visual enhancement
 * 
 * Activated during lock-on, aiming, or concentration modes.
 * Provides visual feedback for focused gameplay state.
 * 
 * Effects:
 * - Slight vignette for focus
 * - Subtle FOV reduction (tunnel vision)
 * - Enhanced saturation (heightened senses)
 * - Time feels slightly slower (perception)
 * 
 * Typical Parameters:
 * - Vignette: 0.08-0.12
 * - FOV reduction: 2-4 degrees
 * - Saturation boost: 1.05-1.1
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_E03_Effect_FocusMode : public UCameraModifier_EffectBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures focus mode effect parameters */
	UCameraModifier_E03_Effect_FocusMode();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_E03_Effect_FocusMode; }
	virtual FString GetModifierDescription() const override { return TEXT("Visual enhancement for focus/aim mode"); }

	//========================================
	// Timing Configuration
	//========================================

	/** Focus mode effect is continuous while aiming/locked */
	virtual float GetDefaultDuration() const override { return 0.0f; }  // Continuous
	virtual float GetBlendInTime() const override { return 0.15f; }
	virtual float GetBlendOutTime() const override { return 0.2f; }

	//========================================
	// Priority
	//========================================

	virtual int32 GetPriority() const override { return 55; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Focus Mode-Specific Parameters
	//========================================

	/** FOV reduction for focus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|FocusMode", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float FocusFOVReduction;

	/** Vignette for focus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|FocusMode", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float FocusVignetteIntensity;

	/** Saturation boost for heightened senses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|FocusMode", meta = (ClampMin = "1.0", ClampMax = "1.3"))
	float FocusSaturationBoost;

	/** Time dilation hint for focused perception */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|FocusMode", meta = (ClampMin = "0.8", ClampMax = "1.0"))
	float FocusTimeDilation;

	/** Is in strong focus mode (full lock-on vs light aim) */
	bool bStrongFocus;
};
