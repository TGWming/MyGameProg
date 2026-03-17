// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModule_FOV.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModule_FOV.cpp
 * 
 * This file contains the implementations of 6 FOV modules (F01-F06).
 * FOV modules are responsible for calculating the camera field of view (float),
 * which determines the angular extent of the observable game world.
 * 
 * Output is passed through FModuleOutput.FOVOutput field.
 * 
 * FOV Blend Policies:
 * - Override: Replace with this value (F01)
 * - Additive: Add offset to current value (F02-F06)
 * 
 * Typical FOV Values:
 * - Narrow: 60-70 degrees (aiming, focused)
 * - Standard: 80-90 degrees (exploration)
 * - Wide: 100-110 degrees (sprinting)
 * - Ultra Wide: 120+ degrees (impact effects)
 * 
 * Module List:
 * - F01: Base FOV - Foundation FOV from state config (implemented)
 * - F02: Speed FOV - Increase FOV when moving fast (implemented)
 * - F03: Aim FOV - Reduce FOV when aiming (implemented)
 * - F04: Combat FOV - Combat-specific adjustment (implemented)
 * - F05: Boss FOV - Boss fight adjustment (implemented)
 * - F06: Impact FOV - Momentary pulse on impacts (implemented)
 */


//========================================
// F01: Base FOV - Foundation FOV from state config
//========================================

/**
 * ShouldActivate - Base FOV is always active
 * 
 * F01 is the foundation FOV module. It should always be active
 * as long as the module is enabled, providing the starting point for
 * all FOV calculations.
 * 
 * Other modules (F02-F06) build upon this base value using
 * Additive policy.
 */
bool UCameraModule_F01_BaseFOV::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Base FOV is always active as the foundation
	// Only check if module is enabled
	return bIsEnabled;
}

/**
 * Compute - Return base FOV from state configuration
 * 
 * This module reads the configured base FOV from StateConfig
 * and outputs it directly. It serves as the starting point for
 * FOV calculations.
 * 
 * Steps:
 * 1. Get base FOV from state config
 * 2. Get min/max limits from config
 * 3. Clamp FOV to valid range
 * 4. Output the clamped FOV
 * 
 * The output uses Override policy, meaning this value will be
 * the starting point that other modules modify.
 * 
 * Typical base FOV values:
 * - Exploration state: 90 degrees
 * - Combat state: 85 degrees
 * - Cinematic state: 70 degrees
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_F01_BaseFOV::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get base FOV from state configuration
	// This is the foundation FOV that other modules will modify
	float BaseFOV = StateConfig.StateBase.FOV.BaseFOV;
	
	// If BaseFOV not configured, use default
	if (BaseFOV <= 0.0f)
	{
		BaseFOV = 90.0f;  // Default: 90 degrees (standard third-person)
	}
	
	// Step 2: Get min/max FOV limits from config
	float MinFOV = StateConfig.StateBase.FOV.MinFOV;
	float MaxFOV = StateConfig.StateBase.FOV.MaxFOV;
	
	// Validate limits
	if (MinFOV <= 0.0f)
	{
		MinFOV = 60.0f;  // Default min: 60 degrees
	}
	
	if (MaxFOV <= 0.0f || MaxFOV <= MinFOV)
	{
		MaxFOV = 120.0f;  // Default max: 120 degrees
	}
	
	// Step 3: Clamp FOV to valid range
	BaseFOV = FMath::Clamp(BaseFOV, MinFOV, MaxFOV);
	
	// Step 4: Fill output structure
	OutOutput.FOVOutput = BaseFOV;
	OutOutput.bHasFOVOutput = true;
	
	return true;
}


//========================================
// F02: Speed FOV
//========================================

/**
 * Constructor - Initialize smooth offset state
 */
UCameraModule_F02_Speed_FOV::UCameraModule_F02_Speed_FOV()
	: CurrentSpeedFOVOffset(0.0f)
{
}

/**
 * ShouldActivate - Determine if speed-based FOV increase should be active
 * 
 * F02 activates when:
 * - Module is enabled
 * - Character is moving (bIsMoving)
 * - Character speed exceeds threshold
 * 
 * This module creates a sense of speed by increasing FOV during fast
 * movement. Common in action games to enhance the feeling of sprinting.
 * 
 * Priority 105 (after F01), uses Additive policy.
 * 
 * Use Cases:
 * - Sprinting through open areas
 * - Running to objectives
 * - Escape sequences
 * - High-speed chase moments
 */
bool UCameraModule_F02_Speed_FOV::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if character is moving
	if (!Context.InputContext.bIsMoving)
	{
		return false;
	}
	
	// Get speed threshold from config
	float SpeedThreshold = StateConfig.Module.FOV.SpeedFOVThreshold;
	
	// If threshold not configured, use default
	if (SpeedThreshold <= 0.0f)
	{
		SpeedThreshold = 200.0f;  // Default: 2 m/s (walking speed)
	}
	
	// Check if speed exceeds threshold
	float CurrentSpeed = Context.InputContext.CharacterSpeed;
	if (CurrentSpeed <= SpeedThreshold)
	{
		return false;  // Not fast enough to trigger
	}
	
	// All conditions met - activate speed FOV
	return true;
}

/**
 * Compute - Calculate FOV increase based on movement speed
 * 
 * This module calculates an additive FOV offset based on how fast
 * the character is moving. Faster movement = larger FOV = wider view.
 * 
 * Steps:
 * 1. Get current speed and config values
 * 2. Calculate speed above threshold
 * 3. Calculate speed ratio (0 to 1)
 * 4. Apply curve for dramatic high-speed effect
 * 5. Calculate target FOV offset
 * 6. Smooth the offset transition
 * 7. Output with Additive policy
 * 
 * Speed to FOV Mapping (with quadratic curve):
 * - At threshold (200cm/s): offset = 0 degrees
 * - At run speed (400cm/s): offset = ~4 degrees
 * - At sprint speed (600cm/s): offset = ~15 degrees (max)
 * 
 * The quadratic curve (ratio squared) makes the effect more pronounced at
 * higher speeds, giving a more dramatic speed sensation.
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_F02_Speed_FOV::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get current speed and config values
	float CurrentSpeed = Context.InputContext.CharacterSpeed;
	
	// Get speed threshold from config
	float SpeedThreshold = StateConfig.Module.FOV.SpeedFOVThreshold;
	if (SpeedThreshold <= 0.0f)
	{
		SpeedThreshold = 200.0f;  // Default: 2 m/s
	}
	
	// Get maximum FOV increase from config
	float MaxFOVIncrease = StateConfig.Module.FOV.SpeedFOVIncrease;
	if (MaxFOVIncrease <= 0.0f)
	{
		MaxFOVIncrease = 15.0f;  // Default: 15 degrees max increase
	}
	
	// Step 2: Calculate speed above threshold
	float SpeedAboveThreshold = FMath::Max(0.0f, CurrentSpeed - SpeedThreshold);
	
	// Step 3: Calculate speed ratio (0 to 1)
	// Assume max effective speed range is 400cm/s above threshold
	// (threshold + 400 = typical sprint speed of 600cm/s)
	const float MaxSpeedRange = 400.0f;
	float SpeedRatio = FMath::Clamp(SpeedAboveThreshold / MaxSpeedRange, 0.0f, 1.0f);
	
	// Step 4: Apply quadratic curve for dramatic high-speed effect
	// ratio squared makes the effect more pronounced at higher speeds
	// At 50% speed ratio: 25% of max FOV increase
	// At 100% speed ratio: 100% of max FOV increase
	float CurvedRatio = SpeedRatio * SpeedRatio;
	
	// Step 5: Calculate target FOV offset
	float TargetFOVOffset = CurvedRatio * MaxFOVIncrease;
	
	// Step 6: Smooth the FOV transition
	// This prevents jarring FOV changes
	float InterpSpeed = 5.0f;  // Default smoothing speed
	
	// Try to get interp speed from config
	float ConfigLagSpeed = StateConfig.StateBase.Lag.FOVLagSpeed;
	if (ConfigLagSpeed > 0.0f)
	{
		InterpSpeed = ConfigLagSpeed;
	}
	
	// Interpolate current offset towards target
	CurrentSpeedFOVOffset = FMath::FInterpTo(
		CurrentSpeedFOVOffset, 
		TargetFOVOffset, 
		Context.DeltaTime, 
		InterpSpeed
	);
	
	// If speed drops below threshold, smoothly return to zero
	if (CurrentSpeed <= SpeedThreshold)
	{
		CurrentSpeedFOVOffset = FMath::FInterpTo(
			CurrentSpeedFOVOffset, 
			0.0f, 
			Context.DeltaTime, 
			InterpSpeed
		);
	}
	
	// Step 7: Fill output structure
	OutOutput.FOVOutput = CurrentSpeedFOVOffset;
	OutOutput.bHasFOVOutput = true;
	
	// Full weight for speed FOV
	OutOutput.Weight = 1.0f;
	
	return true;
}


//========================================
// F03: Aim FOV
//========================================

/**
 * Constructor - Initialize aim blend state
 */
UCameraModule_F03_Aim_FOV::UCameraModule_F03_Aim_FOV()
	: CurrentAimFOVBlend(0.0f)
{
}

/**
 * ShouldActivate - Determine if aim FOV reduction should be active
 * 
 * F03 activates when:
 * - Module is enabled
 * - Player has a locked target (bHasTarget)
 * 
 * In Souls-like games, locking onto an enemy triggers a slight
 * FOV reduction to create a more focused, intense combat feel.
 * This is different from explicit aiming (bow/crossbow) which
 * would use a separate input flag if available.
 * 
 * Priority 115 (higher than speed/combat), uses Additive policy.
 * Negative offset to reduce FOV.
 * 
 * Use Cases:
 * - Lock-on combat (common in Souls-like games)
 * - Target focus during boss fights
 * - Precision combat moments
 * 
 * Note: If explicit aiming (bIsAiming) is needed in the future,
 * add it to FCameraInputContext and check it here for full aiming
 * vs partial lock-on intensity differentiation.
 */
bool UCameraModule_F03_Aim_FOV::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// In Souls-like games, locking onto a target triggers FOV reduction
	// This creates a more focused, intense combat feel
	bool bHasLockedTarget = Context.InputContext.bHasTarget;
	
	// Only activate if we have a locked target
	if (!bHasLockedTarget)
	{
		return false;
	}
	
	// All conditions met - activate aim FOV
	return true;
}

/**
 * Compute - Calculate FOV reduction for lock-on/focus mode
 * 
 * This module calculates a negative FOV offset to narrow the view
 * during lock-on. This creates a more focused, intense visual
 * experience during combat - a signature effect in Souls-like games.
 * 
 * Steps:
 * 1. Determine lock-on state and target blend
 * 2. Get aim FOV reduction from config
 * 3. Smooth the blend transition
 * 4. Calculate final FOV offset (negative)
 * 5. Output with Additive policy
 * 
 * Lock-On Intensity:
 * - Currently uses 100% when locked on (full AimFOVReduction)
 * - Future enhancement: differentiate between soft/hard lock
 * 
 * Typical FOV reductions:
 * - Light focus: -5 to -10 degrees
 * - Standard aim: -10 to -15 degrees
 * - Heavy zoom: -20 to -30 degrees
 * 
 * The smooth transition (using FInterpTo) prevents jarring FOV changes
 * when locking/unlocking targets. Typical transition time is ~0.3-0.5s.
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_F03_Aim_FOV::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Determine target blend based on lock-on state
	bool bHasLockedTarget = Context.InputContext.bHasTarget;
	
	float TargetBlend = 0.0f;
	
	if (bHasLockedTarget)
	{
		// Lock-on active: apply full FOV reduction
		// In Souls-like games, lock-on is the primary "aiming" mechanism
		// Using 100% intensity for now; could be adjusted for soft vs hard lock
		TargetBlend = 1.0f;
	}
	
	// Step 2: Get aim FOV reduction from config
	float AimFOVReduction = StateConfig.Module.FOV.AimFOVReduction;
	
	// If not configured or invalid, use default
	if (AimFOVReduction <= 0.0f)
	{
		AimFOVReduction = 10.0f;  // Default: 10 degrees reduction (matches config default)
	}
	
	// Step 3: Smooth the blend transition
	// Fast blend speed for responsive feel during combat
	// This prevents jarring FOV jumps when locking/unlocking targets
	float BlendSpeed = 8.0f;  // Quick response (~0.3-0.5s to reach target)
	
	// Interpolate current blend towards target
	CurrentAimFOVBlend = FMath::FInterpTo(
		CurrentAimFOVBlend, 
		TargetBlend, 
		Context.DeltaTime, 
		BlendSpeed
	);
	
	// Step 4: Calculate FOV reduction (negative value)
	// Negative because we want to REDUCE the FOV (narrower view = more focus)
	// Example: AimFOVReduction=10, Blend=1.0 -> FOVOffset=-10
	float FOVOffset = -AimFOVReduction * CurrentAimFOVBlend;
	
	// Step 5: Fill output structure
	OutOutput.FOVOutput = FOVOffset;
	OutOutput.bHasFOVOutput = true;
	
	// Full weight for aim FOV
	OutOutput.Weight = 1.0f;
	
	return true;
}


//========================================
// F04: Combat FOV
//========================================

/**
 * ShouldActivate - Determine if combat FOV adjustment should be active
 * 
 * F04 activates when: 
 * - Module is enabled
 * - Player is in combat state (bIsInCombat)
 * 
 * Combat FOV provides slightly wider view for better situational awareness
 * during fights. This helps players track multiple enemies and react to
 * attacks from different directions.
 * 
 * Priority 110 (between speed and aim), uses Additive policy. 
 * 
 * Use Cases:
 * - General combat awareness
 * - Multi-enemy encounters
 * - Boss arena fights
 * - Defensive situations
 */
bool UCameraModule_F04_Combat_FOV::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if player is in combat state
	// This flag is set by gameplay systems when enemies are engaged
	if (!Context.InputContext.bIsInCombat)
	{
		return false;
	}
	
	// All conditions met - activate combat FOV
	return true;
}

/**
 * Compute - Calculate combat FOV adjustment
 * 
 * This module adjusts FOV during combat. Typically provides a slightly
 * wider view for better situational awareness, but can be configured
 * for narrower view in certain combat styles.
 * 
 * Steps:
 * 1. Get combat FOV offset from config
 * 2. Adjust intensity based on target lock status
 * 3. Apply offset
 * 4. Output with Additive policy
 * 
 * Offset Behavior:
 * - Positive offset: Wider FOV (better peripheral vision)
 * - Negative offset: Narrower FOV (more focused)
 * - With target locked: Full offset applied
 * - Without target: Partial offset (50%) for general combat
 * 
 * Typical Values:
 * - Aggressive combat: +5 to +10 degrees (wider)
 * - Defensive combat: -5 degrees (more focused)
 * - Default: +5 degrees (slightly wider)
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_F04_Combat_FOV::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get combat FOV offset from config
	// Positive = wider view, Negative = narrower view
	float CombatFOVOffset = StateConfig.Module.FOV.CombatFOVOffset;
	
	// If not configured (nearly zero), use default (slightly wider for awareness)
	// Note: Using IsNearlyZero instead of <= 0 because offset can be negative
	if (FMath::IsNearlyZero(CombatFOVOffset))
	{
		CombatFOVOffset = 5.0f;  // Default: 5 degrees wider
	}
	
	// Step 2: Adjust intensity based on target lock status
	float EffectiveOffset = 0.0f;
	
	if (Context.InputContext.bHasTarget)
	{
		// With locked target: Apply full combat offset
		// Player is focused on specific enemy
		EffectiveOffset = CombatFOVOffset;
	}
	else
	{
		// Without locked target: Apply partial offset (50%)
		// General combat awareness, not focused on specific enemy
		EffectiveOffset = CombatFOVOffset * 0.5f;
	}
	
	// Step 3: Clamp offset to reasonable range
	// Prevent extreme FOV changes
	const float MinOffset = -15.0f;  // Max 15 degrees narrower
	const float MaxOffset = 15.0f;   // Max 15 degrees wider
	EffectiveOffset = FMath::Clamp(EffectiveOffset, MinOffset, MaxOffset);
	
	// Step 4: Fill output structure
	OutOutput.FOVOutput = EffectiveOffset;
	OutOutput.bHasFOVOutput = true;
	
	// Full weight in combat
	OutOutput.Weight = 1.0f;
	
	return true;
}


//========================================
// F05: Boss FOV
//========================================

/**
 * ShouldActivate - Determine if boss FOV adjustment should be active
 * 
 * F05 activates when:
 * - Module is enabled
 * - Player has a locked target
 * - Target IS a boss (bTargetIsBoss)
 * 
 * Boss FOV provides wider view to accommodate large boss enemies
 * and give players better awareness of boss attacks and arena.
 * 
 * Priority 120 (high), uses Additive policy.
 * 
 * Use Cases:
 * - Large boss fights (dragons, giants)
 * - Multi-phase boss battles
 * - Arena-style boss encounters
 * - Any fight where boss size requires wider view
 */
bool UCameraModule_F05_Boss_FOV::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if player has a locked target
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	// Check if target IS a boss
	// This is the key condition that distinguishes F05 from F04
	if (!Context.InputContext.bTargetIsBoss)
	{
		return false;
	}
	
	// All conditions met - activate boss FOV
	return true;
}

/**
 * Compute - Calculate boss fight FOV adjustment
 * 
 * This module calculates an FOV increase specifically for boss fights.
 * It considers both the boss size and player-boss distance to
 * determine the optimal FOV for the encounter.
 * 
 * Steps:
 * 1. Validate boss target
 * 2. Get base boss FOV offset from config
 * 3. Calculate size-based modifier (larger boss = more FOV)
 * 4. Calculate distance-based modifier (closer = more FOV)
 * 5. Combine modifiers with base offset
 * 6. Clamp to valid range
 * 7. Output with Additive policy
 * 
 * Size Modifier:
 * - Boss < 300cm: modifier = 1.0 (no additional)
 * - Boss 300-800cm: modifier = 1.0 to 1.5 (gradual increase)
 * - Boss > 800cm: modifier = 1.5 (capped)
 * 
 * Distance Modifier (for large bosses):
 * - Far from boss: modifier = 1.0
 * - Close to large boss: modifier = up to 1.3 (need more FOV)
 * 
 * Final Offset = BaseOffset x SizeModifier x DistanceModifier
 * Clamped to -10 to +25 degrees range
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_F05_Boss_FOV::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Validate boss target
	if (!Context.InputContext.bTargetIsBoss)
	{
		// Not a boss - return offset of 0 (no change)
		OutOutput.FOVOutput = 0.0f;
		OutOutput.bHasFOVOutput = true;
		return false;
	}
	
	// Step 2: Get base boss FOV offset from config
	float BossFOVOffset = StateConfig.Module.FOV.BossFOVOffset;
	
	// If not configured, use default
	if (BossFOVOffset <= 0.0f)
	{
		BossFOVOffset = 10.0f;  // Default: 10 degrees wider
	}
	
	// Step 3: Calculate size-based modifier
	// Larger bosses need more FOV to fit in view
	float TargetSize = Context.InputContext.TargetSize;
	float SizeModifier = 1.0f;
	
	// Size thresholds for FOV scaling
	const float LargeBossThreshold = 300.0f;  // 3 meters
	const float MaxSizeForScaling = 800.0f;   // 8 meters (cap)
	const float MaxSizeModifier = 1.5f;       // Max 50% additional
	
	if (TargetSize > LargeBossThreshold)
	{
		// Calculate how much larger than threshold
		float SizeAboveThreshold = TargetSize - LargeBossThreshold;
		float ScalingRange = MaxSizeForScaling - LargeBossThreshold;  // 500cm
		
		// Calculate modifier (1.0 to 1.5)
		// At 300cm: modifier = 1.0
		// At 550cm: modifier = 1.25
		// At 800cm+: modifier = 1.5 (capped)
		float ModifierAdd = (SizeAboveThreshold / ScalingRange) * (MaxSizeModifier - 1.0f);
		SizeModifier = 1.0f + FMath::Clamp(ModifierAdd, 0.0f, MaxSizeModifier - 1.0f);
	}
	
	// Step 4: Calculate distance-based modifier
	// When close to a large boss, need even more FOV
	float TargetDistance = Context.InputContext.TargetDistance;
	float DistanceModifier = 1.0f;
	
	// Only apply distance modifier for large bosses
	const float CloseDistanceThreshold = 500.0f;  // 5 meters
	const float MaxDistanceModifier = 1.3f;       // Max 30% additional
	
	if (TargetDistance < CloseDistanceThreshold && TargetSize > LargeBossThreshold)
	{
		// Calculate closeness factor (0 = at threshold, 1 = very close)
		float ClosenessFactor = 1.0f - (TargetDistance / CloseDistanceThreshold);
		
		// Apply closeness to distance modifier
		// Very close to large boss = need more FOV
		DistanceModifier = 1.0f + ClosenessFactor * (MaxDistanceModifier - 1.0f);
	}
	
	// Step 5: Combine modifiers with base offset
	float FinalFOVOffset = BossFOVOffset * SizeModifier * DistanceModifier;
	
	// Step 6: Clamp to reasonable range
	// Minimum -10: Allow slight FOV reduction for special boss phases
	// Maximum +25: Prevent excessive FOV that distorts view
	FinalFOVOffset = FMath::Clamp(FinalFOVOffset, -10.0f, 25.0f);
	
	// Step 7: Fill output structure
	OutOutput.FOVOutput = FinalFOVOffset;
	OutOutput.bHasFOVOutput = true;
	
	// Full weight for boss fights
	OutOutput.Weight = 1.0f;
	
	return true;
}


//========================================
// F06: Impact FOV
//========================================

/**
 * Constructor - Initialize impact state
 */
UCameraModule_F06_Impact_FOV::UCameraModule_F06_Impact_FOV()
	: CurrentImpactIntensity(0.0f)
	, bImpactActive(false)
	, ImpactTimeRemaining(0.0f)
	, CachedImpactDuration(0.15f)
{
}

/**
 * ShouldActivate - Activate when impact effect is active
 * 
 * F06 activates when:
 * - Module is enabled
 * - Impact effect is currently active (bImpactActive)
 * - Impact intensity is above threshold
 * 
 * Unlike other FOV modules, F06 is triggered externally via TriggerImpact()
 * rather than being based on game state. This allows precise control over
 * when the impact effect occurs.
 * 
 * Priority 125 (highest among FOV modules), uses Additive policy.
 * The high priority ensures the impact effect is visible on top of
 * all other FOV adjustments.
 * 
 * Use Cases:
 * - Player takes damage
 * - Player lands a heavy hit
 * - Parry/counter timing
 * - Explosion nearby
 * - Boss attack impacts
 */
bool UCameraModule_F06_Impact_FOV::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if impact is active
	if (!bImpactActive)
	{
		return false;
	}
	
	// Check intensity threshold
	const float MinIntensityThreshold = 0.01f;
	if (CurrentImpactIntensity <= MinIntensityThreshold)
	{
		return false;
	}
	
	return true;
}

/**
 * Compute - Calculate impact FOV pulse with exponential decay
 * 
 * This module creates a momentary FOV change (typically reduction)
 * when an impact is triggered. The effect quickly decays back to normal.
 * 
 * Steps:
 * 1. Validate impact is active
 * 2. Update impact timer
 * 3. Check if effect has expired
 * 4. Get impact settings from config (with fallback defaults)
 * 5. Calculate decayed intensity
 * 6. Apply FOV offset
 * 7. Output with Additive policy
 * 
 * Impact Behavior:
 * - Trigger: FOV instantly reduces (punch-in)
 * - Decay: FOV smoothly returns to normal
 * - Duration: Typically 0.1-0.3 seconds
 * - Curve: Exponential decay for snappy feel
 * 
 * FOV Change:
 * - Negative offset: Punch-in effect (zoom in feeling)
 * - Positive offset: Punch-out effect (explosion feeling)
 * - Default: Negative (punch-in)
 * 
 * Exponential Decay (t squared):
 * At t=1.0 (start): intensity = 1.0
 * At t=0.5 (half): intensity = 0.25
 * At t=0.0 (end): intensity = 0.0
 * This creates a snappy "punch" feel that quickly fades.
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_F06_Impact_FOV::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output
	OutOutput = CreateBaseOutput();
	
	// Update cached config values for TriggerImpact to use
	UpdateCachedConfig(StateConfig);
	
	// Validate impact is active
	if (!bImpactActive)
	{
		OutOutput.FOVOutput = 0.0f;
		OutOutput.bHasFOVOutput = true;
		return false;
	}
	
	// Update timer
	ImpactTimeRemaining -= Context.DeltaTime;
	
	// Check if expired
	if (ImpactTimeRemaining <= 0.0f)
	{
		bImpactActive = false;
		CurrentImpactIntensity = 0.0f;
		ImpactTimeRemaining = 0.0f;
		
		OutOutput.FOVOutput = 0.0f;
		OutOutput.bHasFOVOutput = true;
		return false;
	}
	
	// Get impact settings from config (with fallback to defaults)
	float ConfigImpactFOV = StateConfig.Module.FOV.ImpactFOVAmount;
	float ConfigDuration = StateConfig.Module.FOV.ImpactDuration;
	
	// Use config values if valid, otherwise use defaults
	float EffectiveImpactFOV = (ConfigImpactFOV > 0.0f) ? ConfigImpactFOV : 10.0f;
	float EffectiveDuration = (ConfigDuration > 0.0f) ? ConfigDuration : 0.15f;
	
	// Calculate decayed intensity (exponential)
	float NormalizedTime = ImpactTimeRemaining / EffectiveDuration;
	float DecayedIntensity = CurrentImpactIntensity * NormalizedTime * NormalizedTime;
	
	// Calculate FOV offset (negative for punch-in)
	float FOVOffset = -EffectiveImpactFOV * DecayedIntensity;
	FOVOffset = FMath::Clamp(FOVOffset, -25.0f, 10.0f);
	
	// Fill output
	OutOutput.FOVOutput = FOVOffset;
	OutOutput.bHasFOVOutput = true;
	OutOutput.Weight = 1.0f;
	
	return true;
}

/**
 * TriggerImpact - Trigger the FOV impact pulse
 * 
 * Call this function when the player takes damage, lands a hit,
 * or any other moment that should have FOV feedback.
 * 
 * The intensity parameter scales the effect:
 * - 0.5: Light hit (small enemy attack)
 * - 1.0: Normal hit (standard attack)
 * - 1.5: Heavy hit (boss attack, critical hit)
 * - 2.0: Maximum impact (massive damage, near-death)
 * 
 * @param Intensity Strength of the impact effect (0.0 to 2.0)
 * @param Duration Optional duration override. If <= 0, uses cached config value.
 */
void UCameraModule_F06_Impact_FOV::TriggerImpact(float Intensity, float Duration)
{
	CurrentImpactIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
	bImpactActive = true;
	
	// Use provided duration if valid, otherwise use cached config value
	if (Duration > 0.0f)
	{
		ImpactTimeRemaining = Duration;
	}
	else
	{
		ImpactTimeRemaining = CachedImpactDuration;
	}
}

/**
 * UpdateCachedConfig - Update cached config values from StateConfig
 * 
 * This method caches configuration values that are needed by TriggerImpact,
 * which doesn't have direct access to StateConfig.
 * 
 * @param StateConfig The current camera state configuration
 */
void UCameraModule_F06_Impact_FOV::UpdateCachedConfig(const FCameraStateConfig& StateConfig)
{
	float ConfigDuration = StateConfig.Module.FOV.ImpactDuration;
	
	// Update cached value if config is valid
	if (ConfigDuration > 0.0f)
	{
		CachedImpactDuration = ConfigDuration;
	}
	else
	{
		CachedImpactDuration = 0.15f;  // Default fallback
	}
}
