// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modifiers/CameraModifier_Shake.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Pipeline/ICameraStage.h"

/**
 * CameraModifier_Shake.cpp
 * 
 * This file contains the implementations of the Shake modifier base class
 * and all 5 Shake modifiers (S01-S05).
 * 
 * Shake modifiers use Perlin noise to generate organic camera shake effects.
 * The base class provides common shake generation functionality, while
 * derived classes configure specific parameters for different use cases.
 * 
 * Implementation Structure:
 * - ShakeBase: Common shake generation (this Sub-Step)
 * - S01-S02: Hit shakes (Sub-Step 4.2.3)
 * - S03-S05: Attack, Landing, Environment shakes (Sub-Step 4.2.4)
 */


//========================================
// ShakeBase - Common Shake Functionality
//========================================

/**
 * Constructor - Initialize default shake parameters
 * 
 * Default values are moderate and suitable for most shakes.
 * Derived classes will override these in their constructors.
 */
UCameraModifier_ShakeBase::UCameraModifier_ShakeBase()
	: PositionAmplitude(5.0f)
	, RotationAmplitude(1.0f)
	, ShakeFrequency(25.0f)
	, bApplyPositionShake(true)
	, bApplyRotationShake(true)
	, RandomSeed(0.0f)
	, ShakeTime(0.0f)
	, HitDirection(FVector::ZeroVector)
	, DirectionalBias(0.0f)
{
}

/**
 * OnTriggered - Reset shake state when triggered
 * 
 * Generates a new random seed for variation and resets timing.
 * Also caches hit direction from trigger data for directional shakes.
 */
void UCameraModifier_ShakeBase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Reset shake time
	ShakeTime = 0.0f;
	
	// Generate new random seed for this shake instance
	// This ensures each trigger produces a unique shake pattern
	RandomSeed = FMath::FRand() * 1000.0f;
	
	// Cache hit direction for directional bias (if provided)
	if (!InTriggerData.SourceDirection.IsNearlyZero())
	{
		HitDirection = InTriggerData.SourceDirection.GetSafeNormal();
	}
	else
	{
		HitDirection = FVector::ZeroVector;
	}
}

/**
 * ComputeEffect - Generate shake effect values each frame
 * 
 * This is the core shake computation. It:
 * 1. Accumulates shake time
 * 2. Calculates effective amplitude (weight * intensity)
 * 3. Generates position shake using Perlin noise
 * 4. Generates rotation shake using Perlin noise
 * 5. Applies directional bias if configured
 * 
 * The generated values are stored in PositionEffect and RotationEffect,
 * which will be applied (with weight) in GetOutput().
 */
void UCameraModifier_ShakeBase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Accumulate shake time for noise sampling
	ShakeTime += DeltaTime;
	
	// Calculate effective amplitude based on current weight and intensity
	float EffectiveAmp = GetEffectiveAmplitude();
	
	// Skip computation if amplitude is negligible
	if (EffectiveAmp < KINDA_SMALL_NUMBER)
	{
		PositionEffect = FVector::ZeroVector;
		RotationEffect = FRotator::ZeroRotator;
		return;
	}
	
	// Generate position shake
	if (bApplyPositionShake)
	{
		FVector BasePositionShake = GeneratePositionShake(
			ShakeTime, 
			ShakeFrequency, 
			PositionAmplitude * EffectiveAmp
		);
		
		// Apply directional bias if configured and direction is valid
		if (DirectionalBias > 0.0f && !HitDirection.IsNearlyZero())
		{
			PositionEffect = ApplyDirectionalBias(BasePositionShake, HitDirection, DirectionalBias);
		}
		else
		{
			PositionEffect = BasePositionShake;
		}
	}
	else
	{
		PositionEffect = FVector::ZeroVector;
	}
	
	// Generate rotation shake
	if (bApplyRotationShake)
	{
		RotationEffect = GenerateRotationShake(
			ShakeTime, 
			ShakeFrequency, 
			RotationAmplitude * EffectiveAmp
		);
	}
	else
	{
		RotationEffect = FRotator::ZeroRotator;
	}
}

/**
 * GeneratePositionShake - Generate position offset using Perlin noise
 * 
 * Uses three separate Perlin noise samples for X, Y, Z axes.
 * Each axis has a slightly different frequency multiplier and offset
 * to prevent synchronized/repetitive motion.
 * 
 * Z (vertical) shake is reduced by 50% for a more natural feel,
 * as excessive vertical shake can be disorienting.
 * 
 * @param Time Current shake time
 * @param Frequency Oscillation frequency (Hz)
 * @param Amplitude Maximum offset magnitude (cm)
 * @return Position offset vector
 */
FVector UCameraModifier_ShakeBase::GeneratePositionShake(float Time, float Frequency, float Amplitude) const
{
	// Early out if no amplitude
	if (Amplitude <= KINDA_SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}
	
	// Scale time by frequency for noise sampling
	float TimeScaled = Time * Frequency;
	
	// Use different offsets for each axis to break synchronization
	// RandomSeed ensures each trigger has unique offsets
	float OffsetX = RandomSeed;
	float OffsetY = RandomSeed + 100.0f;
	float OffsetZ = RandomSeed + 200.0f;
	
	// Generate noise for each axis
	// Slightly different frequency multipliers add organic variation
	FVector Shake;
	Shake.X = FMath::PerlinNoise1D(TimeScaled * 1.0f + OffsetX) * Amplitude;
	Shake.Y = FMath::PerlinNoise1D(TimeScaled * 1.1f + OffsetY) * Amplitude;
	Shake.Z = FMath::PerlinNoise1D(TimeScaled * 0.9f + OffsetZ) * Amplitude * 0.5f;  // Reduced vertical
	
	return Shake;
}

/**
 * GenerateRotationShake - Generate rotation offset using Perlin noise
 * 
 * Uses three separate Perlin noise samples for Pitch, Yaw, Roll.
 * Roll is significantly reduced (30%) as excessive roll
 * is very disorienting for players.
 * 
 * @param Time Current shake time
 * @param Frequency Oscillation frequency (Hz)
 * @param Amplitude Maximum rotation magnitude (degrees)
 * @return Rotation offset
 */
FRotator UCameraModifier_ShakeBase::GenerateRotationShake(float Time, float Frequency, float Amplitude) const
{
	// Early out if no amplitude
	if (Amplitude <= KINDA_SMALL_NUMBER)
	{
		return FRotator::ZeroRotator;
	}
	
	// Scale time by frequency for noise sampling
	float TimeScaled = Time * Frequency;
	
	// Use different offsets for each rotation axis
	float OffsetPitch = RandomSeed + 300.0f;
	float OffsetYaw = RandomSeed + 400.0f;
	float OffsetRoll = RandomSeed + 500.0f;
	
	// Generate noise for each axis
	// Different frequency multipliers for organic motion
	FRotator Shake;
	Shake.Pitch = FMath::PerlinNoise1D(TimeScaled * 1.2f + OffsetPitch) * Amplitude;
	Shake.Yaw = FMath::PerlinNoise1D(TimeScaled * 0.8f + OffsetYaw) * Amplitude;
	Shake.Roll = FMath::PerlinNoise1D(TimeScaled * 1.0f + OffsetRoll) * Amplitude * 0.3f;  // Minimal roll
	
	return Shake;
}

/**
 * GetEffectiveAmplitude - Calculate amplitude considering weight and intensity
 * 
 * The effective amplitude is the product of:
 * - CurrentWeight: Blend weight from lifecycle (0 during blend, 1 when active)
 * - CurrentIntensity: Trigger intensity from FModifierTriggerData
 * 
 * This allows both smooth blending and per-trigger intensity scaling.
 * 
 * @return Amplitude multiplier (typically 0-1, can exceed 1 for intense triggers)
 */
float UCameraModifier_ShakeBase::GetEffectiveAmplitude() const
{
	return CurrentWeight * CurrentIntensity;
}

/**
 * ApplyDirectionalBias - Bias shake towards a specific direction
 * 
 * For hit reactions, we want the camera to shake more in the direction
 * of the hit. This function blends random shake with directional shake.
 * 
 * @param BaseShake The randomly generated shake vector
 * @param Direction The direction to bias towards (normalized)
 * @param Bias Blend factor (0 = pure random, 1 = pure directional)
 * @return Blended shake vector
 */
FVector UCameraModifier_ShakeBase::ApplyDirectionalBias(const FVector& BaseShake, const FVector& Direction, float Bias) const
{
	// Validate inputs
	if (Bias <= KINDA_SMALL_NUMBER || Direction.IsNearlyZero())
	{
		return BaseShake;
	}
	
	// Clamp bias to valid range
	float ClampedBias = FMath::Clamp(Bias, 0.0f, 1.0f);
	
	// Calculate directional component
	// Use the magnitude of the base shake but apply it in the hit direction
	float ShakeMagnitude = BaseShake.Size();
	FVector DirectionalShake = Direction * ShakeMagnitude;
	
	// Blend between random and directional
	// Add some oscillation to the directional component for variety
	float Oscillation = FMath::Sin(ShakeTime * ShakeFrequency * 2.0f * PI);
	DirectionalShake *= (0.5f + 0.5f * Oscillation);  // Oscillate between 0 and 1
	
	// Lerp between base shake and directional shake
	return FMath::Lerp(BaseShake, DirectionalShake, ClampedBias);
}


//========================================
// S01: Hit Light Shake Implementation
//========================================

/**
 * Constructor - Configure light hit shake parameters
 * 
 * Light shakes are quick and subtle:
 * - Low amplitude for subtle feedback
 * - High frequency for responsive feel
 * - No directional bias (random shake)
 */
UCameraModifier_S01_Shake_Hit_Light::UCameraModifier_S01_Shake_Hit_Light()
{
	// Position shake - subtle
	PositionAmplitude = 3.0f;  // 3 cm max offset
	bApplyPositionShake = true;
	
	// Rotation shake - very subtle
	RotationAmplitude = 0.5f;  // 0.5 degrees max
	bApplyRotationShake = true;
	
	// Fast shake for responsiveness
	ShakeFrequency = 30.0f;  // 30 Hz
	
	// No directional bias for light hits
	DirectionalBias = 0.0f;
}

void UCameraModifier_S01_Shake_Hit_Light::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	Super::OnTriggered(InTriggerData);
	
	// Light shake uses base parameters without modification
	// Intensity from trigger data still applies
}


//========================================
// S02: Hit Heavy Shake Implementation
//========================================

/**
 * Constructor - Configure heavy hit shake parameters
 * 
 * Heavy shakes are more intense:
 * - Higher amplitude for impactful feel
 * - Lower frequency for weighty feel
 * - Moderate directional bias towards hit direction
 */
UCameraModifier_S02_Shake_Hit_Heavy::UCameraModifier_S02_Shake_Hit_Heavy()
{
	// Position shake - strong
	PositionAmplitude = 10.0f;  // 10 cm max offset
	bApplyPositionShake = true;
	
	// Rotation shake - noticeable
	RotationAmplitude = 2.5f;  // 2.5 degrees max
	bApplyRotationShake = true;
	
	// Slower shake for impact feel
	ShakeFrequency = 22.0f;  // 22 Hz
	
	// Moderate directional bias for hit direction
	DirectionalBias = 0.3f;  // 30% bias towards hit direction
}

void UCameraModifier_S02_Shake_Hit_Heavy::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	Super::OnTriggered(InTriggerData);
	
	// Heavy shake may scale based on damage/intensity
	// Intensity from trigger data controls overall strength
}


//========================================
// S03: Attack Hit Shake Implementation
//========================================

/**
 * Constructor - Configure attack hit shake parameters
 * 
 * Attack shakes provide feedback when player lands hits:
 * - Very short duration to not interrupt combat flow
 * - Moderate amplitude for satisfying impact
 * - Slight directional bias towards attack direction
 */
UCameraModifier_S03_Shake_Attack_Hit::UCameraModifier_S03_Shake_Attack_Hit()
{
	// Position shake - moderate
	PositionAmplitude = 4.0f;  // 4 cm max offset
	bApplyPositionShake = true;
	
	// Rotation shake - subtle
	RotationAmplitude = 0.8f;  // 0.8 degrees max
	bApplyRotationShake = true;
	
	// Fast shake for snappy feel
	ShakeFrequency = 35.0f;  // 35 Hz
	
	// Slight directional bias
	DirectionalBias = 0.15f;  // 15% bias
}

void UCameraModifier_S03_Shake_Attack_Hit::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	Super::OnTriggered(InTriggerData);
	
	// Attack shake may be lighter for quick attacks, stronger for heavy attacks
	// Use CustomFloat or Intensity from trigger data to differentiate
}


//========================================
// S04: Environment Shake Implementation
//========================================

/**
 * Constructor - Configure environment shake parameters
 * 
 * Environment shakes are for world events:
 * - Lower frequency for rumble effect
 * - Longer duration than hit shakes
 * - Supports distance-based falloff
 */
UCameraModifier_S04_Shake_Environment::UCameraModifier_S04_Shake_Environment()
{
	// Position shake - strong for environmental events
	PositionAmplitude = 15.0f;  // 15 cm max offset
	bApplyPositionShake = true;
	
	// Rotation shake - moderate
	RotationAmplitude = 3.0f;  // 3 degrees max
	bApplyRotationShake = true;
	
	// Low frequency for rumble feel
	ShakeFrequency = 15.0f;  // 15 Hz
	
	// No directional bias for environment shakes
	DirectionalBias = 0.0f;
	
	// Distance falloff settings
	bUseDistanceFalloff = true;
	MinFalloffDistance = 500.0f;   // Full effect within 5 meters
	MaxShakeDistance = 3000.0f;    // No effect beyond 30 meters
	SourceLocation = FVector::ZeroVector;
}

void UCameraModifier_S04_Shake_Environment::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	Super::OnTriggered(InTriggerData);
	
	// Cache source location for distance falloff
	SourceLocation = InTriggerData.SourceLocation;
}

void UCameraModifier_S04_Shake_Environment::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate distance falloff if enabled
	float DistanceMultiplier = 1.0f;
	if (bUseDistanceFalloff)
	{
		DistanceMultiplier = CalculateDistanceFalloff(Context);
	}
	
	// Apply distance multiplier to intensity temporarily
	float OriginalIntensity = CurrentIntensity;
	CurrentIntensity *= DistanceMultiplier;
	
	// Call base computation
	Super::ComputeEffect(DeltaTime, Context);
	
	// Restore original intensity
	CurrentIntensity = OriginalIntensity;
}

float UCameraModifier_S04_Shake_Environment::CalculateDistanceFalloff(const FStageExecutionContext& Context) const
{
	// If no source location set, no falloff
	if (SourceLocation.IsNearlyZero())
	{
		return 1.0f;
	}
	
	// Calculate distance from camera to source
	float Distance = FVector::Dist(Context.InputContext.PreviousCameraLocation, SourceLocation);
	
	// Within minimum distance, full effect
	if (Distance <= MinFalloffDistance)
	{
		return 1.0f;
	}
	
	// Beyond maximum distance, no effect
	if (Distance >= MaxShakeDistance)
	{
		return 0.0f;
	}
	
	// Linear falloff between min and max distance
	float FalloffRange = MaxShakeDistance - MinFalloffDistance;
	float FalloffProgress = (Distance - MinFalloffDistance) / FalloffRange;
	
	// Use smooth falloff curve (squared for more gradual)
	return 1.0f - (FalloffProgress * FalloffProgress);
}


//========================================
// S05: Landing Impact Shake Implementation
//========================================

/**
 * Constructor - Configure landing shake parameters
 * 
 * Landing shakes emphasize vertical motion:
 * - Vertical emphasis for Z position and pitch rotation
 * - Intensity scales with fall distance (via trigger intensity)
 * - Lower frequency for heavier landing feel
 */
UCameraModifier_S05_Shake_Landing::UCameraModifier_S05_Shake_Landing()
{
	// Position shake - moderate with vertical emphasis
	PositionAmplitude = 8.0f;  // 8 cm max offset
	bApplyPositionShake = true;
	
	// Rotation shake - pitch focused
	RotationAmplitude = 1.5f;  // 1.5 degrees max
	bApplyRotationShake = true;
	
	// Lower frequency for heavy landing feel
	ShakeFrequency = 18.0f;  // 18 Hz
	
	// No directional bias (always downward impact)
	DirectionalBias = 0.0f;
	
	// Landing-specific emphasis
	VerticalEmphasis = 2.5f;   // 2.5x vertical shake
	PitchEmphasis = 1.8f;      // 1.8x pitch rotation
}

void UCameraModifier_S05_Shake_Landing::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	Super::OnTriggered(InTriggerData);
	
	// CustomFloat can contain fall distance for intensity scaling
	// Intensity should already be scaled by the triggering system
}

void UCameraModifier_S05_Shake_Landing::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Accumulate shake time for noise sampling
	ShakeTime += DeltaTime;
	
	// Calculate effective amplitude
	float EffectiveAmp = GetEffectiveAmplitude();
	
	// Skip computation if amplitude is negligible
	if (EffectiveAmp < KINDA_SMALL_NUMBER)
	{
		PositionEffect = FVector::ZeroVector;
		RotationEffect = FRotator::ZeroRotator;
		return;
	}
	
	// Generate position shake with vertical emphasis
	if (bApplyPositionShake)
	{
		FVector BaseShake = GeneratePositionShake(
			ShakeTime, 
			ShakeFrequency, 
			PositionAmplitude * EffectiveAmp
		);
		
		// Apply vertical emphasis - multiply Z component
		BaseShake.Z *= VerticalEmphasis;
		
		PositionEffect = BaseShake;
	}
	else
	{
		PositionEffect = FVector::ZeroVector;
	}
	
	// Generate rotation shake with pitch emphasis
	if (bApplyRotationShake)
	{
		FRotator BaseRotation = GenerateRotationShake(
			ShakeTime, 
			ShakeFrequency, 
			RotationAmplitude * EffectiveAmp
		);
		
		// Apply pitch emphasis - multiply pitch component
		BaseRotation.Pitch *= PitchEmphasis;
		
		RotationEffect = BaseRotation;
	}
	else
	{
		RotationEffect = FRotator::ZeroRotator;
	}
}
