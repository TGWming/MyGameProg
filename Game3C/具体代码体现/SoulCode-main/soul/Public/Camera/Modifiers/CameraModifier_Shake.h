// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierBase.h"
#include "CameraModifier_Shake.generated.h"

/**
 * Shake Modifier Declarations (S01-S05)
 * 
 * This file contains the 5 Shake modifiers for the camera system.
 * Shake modifiers provide camera shake effects triggered by various game events.
 * 
 * All shake modifiers inherit from UCameraModifier_ShakeBase which provides
 * common functionality for generating procedural camera shake using Perlin noise.
 * 
 * Shake Types:
 * - S01: Hit Light - Subtle shake from minor damage
 * - S02: Hit Heavy - Strong shake from major damage
 * - S03: Attack Hit - Feedback when player attacks connect
 * - S04: Environment - Environmental events (explosions, earthquakes)
 * - S05: Landing - Shake when landing from height
 * 
 * Key Characteristics:
 * - Short duration (0.1-0.5 seconds typically)
 * - Fast blend in, slower blend out for natural feel
 * - Uses Perlin noise for organic motion
 * - Supports both position and rotation shake
 * - Priority typically 85-110 (lower than reactions)
 */


//========================================
// Shake Base Class
//========================================

/**
 * UCameraModifier_ShakeBase
 * 
 * Abstract base class for all shake modifiers.
 * Provides common shake generation functionality using Perlin noise.
 * 
 * Features:
 * - Procedural position shake (X, Y, Z offsets)
 * - Procedural rotation shake (Pitch, Yaw, Roll offsets)
 * - Configurable frequency and amplitude
 * - Automatic decay based on blend weight
 * - Random seed for variation between triggers
 * 
 * Derived classes configure the shake parameters and may override
 * ComputeEffect for specialized behavior.
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraModifier_ShakeBase : public UCameraModifierBase
{
	GENERATED_BODY()

public:
	/** Constructor - sets default shake parameters */
	UCameraModifier_ShakeBase();

	//========================================
	// Modifier Identity
	//========================================

	/** All shake modifiers belong to the Shake category */
	virtual EModifierCategory GetModifierCategory() const override { return EModifierCategory::Shake; }

	/** Shake modifiers use the Shake type for output classification */
	virtual ECameraModifierType GetModifierType() const override { return ECameraModifierType::Shake; }

protected:
	//========================================
	// Effect Computation
	//========================================

	/**
	 * Compute shake effect values
	 * Generates position and rotation offsets using Perlin noise
	 * @param DeltaTime Time since last frame
	 * @param Context Current camera execution context
	 */
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	/**
	 * Called when modifier is triggered
	 * Resets shake time and generates new random seed
	 * @param InTriggerData Trigger configuration
	 */
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;

	//========================================
	// Shake Generation Helpers
	//========================================

	/**
	 * Generate position shake offset using Perlin noise
	 * @param Time Current shake time
	 * @param Frequency Oscillation frequency
	 * @param Amplitude Maximum offset magnitude
	 * @return Position offset vector
	 */
	FVector GeneratePositionShake(float Time, float Frequency, float Amplitude) const;

	/**
	 * Generate rotation shake offset using Perlin noise
	 * @param Time Current shake time
	 * @param Frequency Oscillation frequency
	 * @param Amplitude Maximum rotation magnitude (degrees)
	 * @return Rotation offset
	 */
	FRotator GenerateRotationShake(float Time, float Frequency, float Amplitude) const;

	/**
	 * Get effective amplitude considering weight and intensity
	 * @return Amplitude multiplier (0-1+)
	 */
	float GetEffectiveAmplitude() const;

	/**
	 * Apply directional bias to shake based on hit direction
	 * @param BaseShake Base shake vector
	 * @param Direction Hit direction
	 * @param Bias How much to bias towards direction (0-1)
	 * @return Biased shake vector
	 */
	FVector ApplyDirectionalBias(const FVector& BaseShake, const FVector& Direction, float Bias) const;

	//========================================
	// Shake Parameters (Configurable)
	//========================================

	/** Position shake amplitude in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Position", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float PositionAmplitude;

	/** Rotation shake amplitude in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Rotation", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float RotationAmplitude;

	/** Shake frequency (oscillations per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Timing", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float ShakeFrequency;

	/** Whether to apply position shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Options")
	bool bApplyPositionShake;

	/** Whether to apply rotation shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Options")
	bool bApplyRotationShake;

	//========================================
	// Runtime State
	//========================================

	/** Random seed for noise variation (regenerated each trigger) */
	UPROPERTY(BlueprintReadOnly, Category = "Shake|State")
	float RandomSeed;

	/** Accumulated shake time since trigger */
	UPROPERTY(BlueprintReadOnly, Category = "Shake|State")
	float ShakeTime;

	/** Cached hit direction for directional shakes */
	UPROPERTY(BlueprintReadOnly, Category = "Shake|State")
	FVector HitDirection;

	/** Directional bias strength (0 = no bias, 1 = full bias) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Options", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DirectionalBias;
};


//========================================
// S01: Hit Shake Light
//========================================

/**
 * UCameraModifier_S01_Shake_Hit_Light
 * 
 * S01: Hit Light - Subtle camera shake from minor hits
 * 
 * Triggered when player receives light damage (quick attacks, minor enemies).
 * Quick, subtle shake that provides feedback without being disorienting.
 * 
 * Typical Parameters:
 * - Duration: 0.15 seconds
 * - Position Amplitude: 2-5 cm
 * - Rotation Amplitude: 0.5-1 degrees
 * - Frequency: 25-35 Hz
 * 
 * Usage:
 * @code
 * ModifierManager->TriggerModifier(ECameraModifierID::Modifier_S01_Shake_Hit_Light,
 *     FModifierTriggerData::MakeDirectional(Intensity, HitLocation, HitDirection));
 * @endcode
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_S01_Shake_Hit_Light : public UCameraModifier_ShakeBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures light hit shake parameters */
	UCameraModifier_S01_Shake_Hit_Light();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_S01_Shake_Hit_Light; }
	virtual FString GetModifierDescription() const override { return TEXT("Subtle camera shake from minor hits"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.15f; }
	virtual float GetBlendInTime() const override { return 0.02f; }
	virtual float GetBlendOutTime() const override { return 0.10f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 100; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
};


//========================================
// S02: Hit Shake Heavy
//========================================

/**
 * UCameraModifier_S02_Shake_Hit_Heavy
 * 
 * S02: Hit Heavy - Strong camera shake from major hits
 * 
 * Triggered when player receives heavy damage (boss attacks, charged attacks).
 * More intense shake that emphasizes the impact of significant damage.
 * 
 * Typical Parameters:
 * - Duration: 0.3 seconds
 * - Position Amplitude: 8-15 cm
 * - Rotation Amplitude: 2-4 degrees
 * - Frequency: 20-30 Hz
 * 
 * Can interrupt lighter shakes due to higher priority.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_S02_Shake_Hit_Heavy : public UCameraModifier_ShakeBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures heavy hit shake parameters */
	UCameraModifier_S02_Shake_Hit_Heavy();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_S02_Shake_Hit_Heavy; }
	virtual FString GetModifierDescription() const override { return TEXT("Strong camera shake from major hits"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.30f; }
	virtual float GetBlendInTime() const override { return 0.02f; }
	virtual float GetBlendOutTime() const override { return 0.20f; }

	//========================================
	// Priority & Stacking
	//========================================

	/** Higher priority than light shake */
	virtual int32 GetPriority() const override { return 110; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
};


//========================================
// S03: Attack Hit Shake
//========================================

/**
 * UCameraModifier_S03_Shake_Attack_Hit
 * 
 * S03: Attack Hit - Camera shake when player attacks connect
 * 
 * Triggered when player's attack hits an enemy.
 * Provides satisfying feedback for combat actions.
 * Very brief shake to not interfere with combat flow.
 * 
 * Typical Parameters:
 * - Duration: 0.12 seconds
 * - Position Amplitude: 3-6 cm
 * - Rotation Amplitude: 0.5-1.5 degrees
 * - Frequency: 30-40 Hz
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_S03_Shake_Attack_Hit : public UCameraModifier_ShakeBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures attack shake parameters */
	UCameraModifier_S03_Shake_Attack_Hit();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_S03_Shake_Attack_Hit; }
	virtual FString GetModifierDescription() const override { return TEXT("Camera shake when player attacks connect"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.12f; }
	virtual float GetBlendInTime() const override { return 0.01f; }
	virtual float GetBlendOutTime() const override { return 0.08f; }

	//========================================
	// Priority & Stacking
	//========================================

	/** Lower priority - doesn't override hit shakes */
	virtual int32 GetPriority() const override { return 95; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
};


//========================================
// S04: Environment Shake
//========================================

/**
 * UCameraModifier_S04_Shake_Environment
 * 
 * S04: Environment - Camera shake from environmental events
 * 
 * Triggered by explosions, earthquakes, boss abilities, etc.
 * Supports distance-based falloff from source location.
 * Can stack with other shakes (multiple explosions).
 * 
 * Typical Parameters:
 * - Duration: 0.5-2.0 seconds (varies by event)
 * - Position Amplitude: 10-30 cm
 * - Rotation Amplitude: 2-5 degrees
 * - Frequency: 10-20 Hz (low rumble feel)
 * 
 * Uses SourceLocation in TriggerData for distance falloff.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_S04_Shake_Environment : public UCameraModifier_ShakeBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures environment shake parameters */
	UCameraModifier_S04_Shake_Environment();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_S04_Shake_Environment; }
	virtual FString GetModifierDescription() const override { return TEXT("Camera shake from environmental events"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.50f; }
	virtual float GetBlendInTime() const override { return 0.05f; }
	virtual float GetBlendOutTime() const override { return 0.30f; }

	//========================================
	// Priority & Stacking
	//========================================

	/** Lower priority - environmental backdrop */
	virtual int32 GetPriority() const override { return 85; }
	
	/** Environment shakes CAN stack (multiple explosions) */
	virtual bool CanStack() const override { return true; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Distance Falloff
	//========================================

	/**
	 * Calculate intensity falloff based on distance from source
	 * @param Context Current execution context (for camera position)
	 * @return Falloff multiplier (0-1)
	 */
	float CalculateDistanceFalloff(const FStageExecutionContext& Context) const;

	/** Whether to apply distance-based falloff */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Environment")
	bool bUseDistanceFalloff;

	/** Maximum distance for full effect (no falloff) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Environment", meta = (ClampMin = "0.0", EditCondition = "bUseDistanceFalloff"))
	float MinFalloffDistance;

	/** Distance at which effect reaches zero */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Environment", meta = (ClampMin = "100.0", EditCondition = "bUseDistanceFalloff"))
	float MaxShakeDistance;

	/** Cached source location from trigger */
	UPROPERTY(BlueprintReadOnly, Category = "Shake|State")
	FVector SourceLocation;
};


//========================================
// S05: Landing Impact Shake
//========================================

/**
 * UCameraModifier_S05_Shake_Landing
 * 
 * S05: Landing - Camera shake when landing from height
 * 
 * Triggered when character lands after falling.
 * Intensity scales with fall distance.
 * Emphasizes vertical shake to match the landing motion.
 * 
 * Typical Parameters:
 * - Duration: 0.25 seconds (scales with intensity)
 * - Position Amplitude: 5-20 cm (mainly vertical)
 * - Rotation Amplitude: 1-3 degrees (pitch emphasis)
 * - Frequency: 15-25 Hz (lower for heavier feel)
 * 
 * Uses CustomFloat in TriggerData to receive fall distance.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_S05_Shake_Landing : public UCameraModifier_ShakeBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures landing shake parameters */
	UCameraModifier_S05_Shake_Landing();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_S05_Shake_Landing; }
	virtual FString GetModifierDescription() const override { return TEXT("Camera shake when landing from height"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.25f; }
	virtual float GetBlendInTime() const override { return 0.01f; }
	virtual float GetBlendOutTime() const override { return 0.15f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 90; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Landing-Specific Parameters
	//========================================

	/** Vertical emphasis multiplier (how much more vertical shake vs horizontal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Landing", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float VerticalEmphasis;

	/** Pitch rotation emphasis for landing feel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Landing", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float PitchEmphasis;
};
