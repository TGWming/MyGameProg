// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModule_Distance.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModule_Distance.cpp
 * 
 * This file contains the implementations of 7 Distance modules (D01-D07).
 * Distance modules are responsible for calculating the camera arm length (float),
 * which determines the distance between the camera and the focus point.
 * 
 * Output is passed through FModuleOutput.DistanceOutput field.
 * 
 * Distance Blend Policies:
 * - Override: Replace with this value (D01)
 * - Additive: Add offset to current value (D03, D04)
 * - Multiplicative: Multiply current value (D02, D07)
 * - Minimum: Use smaller of current and new (D05)
 * - Blend: Weighted average (D06)
 * 
 * Module List:
 * - D01: Base Distance - Foundation distance from state config (implemented)
 * - D02: Target Size Multiplier - Adjust based on target size (implemented)
 * - D03: Speed Offset - Increase distance when moving fast (implemented)
 * - D04: Combat Adjust - Combat-specific adjustments (implemented)
 * - D05: Environment Limit - Limit distance in tight spaces (implemented)
 * - D06: Proximity Adjust - Adjust based on target proximity (implemented)
 * - D07: Boss Phase - Boss phase-specific scaling (implemented)
 */


//========================================
// D01: Base Distance - Foundation distance from state config
//========================================

/**
 * ShouldActivate - Base distance is always active
 * 
 * D01 is the foundation Distance module. It should always be active
 * as long as the module is enabled, providing the starting point for
 * all distance calculations.
 * 
 * Other modules (D02-D07) build upon this base value using their
 * respective blend policies (Additive, Multiplicative, etc.).
 */
bool UCameraModule_D01_BaseDistance::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Base distance is always active as the foundation
	// Only check if module is enabled
	return bIsEnabled;
}

/**
 * Compute - Return base distance from state configuration
 * 
 * This module reads the configured base distance from StateConfig
 * and outputs it directly. It serves as the starting point for
 * distance calculations.
 * 
 * Steps:
 * 1. Get base distance from state config
 * 2. Get min/max limits from config
 * 3. Clamp distance to valid range
 * 4. Output the clamped distance
 * 
 * The output uses Override policy, meaning this value will be
 * the starting point that other modules modify.
 * 
 * Typical base distance values:
 * - Exploration state: 400cm
 * - Combat state: 350cm
 * - Boss state: 500cm
 * - Cinematic state: varies
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_D01_BaseDistance::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get base distance from state configuration
	// This is the foundation distance that other modules will modify
	float BaseDistance = StateConfig.StateBase.Distance.BaseDistance;
	
	// If BaseDistance not configured, use default
	if (BaseDistance <= 0.0f)
	{
		BaseDistance = 400.0f;  // Default: 4 meters (standard third-person)
	}
	
	// Step 2: Get min/max distance limits from config
	float MinDistance = StateConfig.StateBase.Distance.MinDistance;
	float MaxDistance = StateConfig.StateBase.Distance.MaxDistance;
	
	// Validate limits
	if (MinDistance <= 0.0f)
	{
		MinDistance = 100.0f;  // Default min: 1 meter
	}
	
	if (MaxDistance <= 0.0f || MaxDistance <= MinDistance)
	{
		MaxDistance = 800.0f;  // Default max: 8 meters
	}
	
	// Step 3: Clamp distance to valid range
	BaseDistance = FMath::Clamp(BaseDistance, MinDistance, MaxDistance);
	
	// Step 4: Fill output structure
	OutOutput.DistanceOutput = BaseDistance;
	OutOutput.bHasDistanceOutput = true;
	
	// Mark that this is an absolute value, not a multiplier
	// This helps Stage 5 know how to combine distance outputs
	OutOutput.bDistanceIsMultiplier = false;
	
	return true;
}


//========================================
// D02: Target Size Multiplier
//========================================

/**
 * ShouldActivate - Determine if target size multiplier should be active
 * 
 * D02 activates when:
 * - Module is enabled
 * - Player has a locked target
 * - Target size exceeds the threshold (significant size)
 * 
 * The threshold prevents small enemies from triggering distance changes.
 * Only when the target is noticeably larger than normal does the
 * camera pull back.
 * 
 * Priority 110 (after D01), uses Multiplicative policy.
 * 
 * Use Cases:
 * - Large bosses (dragons, giants)
 * - Mini-bosses larger than regular enemies
 * - Any enemy that needs more screen space
 */
bool UCameraModule_D02_TargetSize_Multiplier::ShouldActivate(
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
	
	// Get target size threshold from config
	float SizeThreshold = StateConfig.Module.Distance.TargetSizeThreshold;
	
	// If threshold not configured, use default
	if (SizeThreshold <= 0.0f)
	{
		SizeThreshold = 150.0f;  // Default: 1.5 meters (larger than humanoid)
	}
	
	// Check if target size exceeds threshold
	float TargetSize = Context.InputContext.TargetSize;
	if (TargetSize <= SizeThreshold)
	{
		return false;  // Target not large enough to trigger
	}
	
	// All conditions met - activate target size multiplier
	return true;
}

/**
 * Compute - Calculate distance multiplier based on target size
 * 
 * This module calculates a multiplier that increases camera distance
 * for larger targets. The goal is to keep large enemies (bosses, giants)
 * properly framed in the camera view.
 * 
 * Steps:
 * 1. Validate target exists
 * 2. Get target size and config values
 * 3. Calculate size above threshold
 * 4. Calculate multiplier based on size difference
 * 5. Clamp multiplier to valid range
 * 6. Calculate dynamic weight
 * 7. Output with Multiplicative policy
 * 
 * Multiplier Calculation:
 * - Every 100cm above threshold adds (Scale - 1.0) to multiplier
 * - Example: Scale=1.5, Target=350cm, Threshold=150cm
 *   - SizeAbove = 200cm
 *   - MultiplierAdd = (200/100) * (1.5-1.0) = 1.0
 *   - Multiplier = 1.0 + 1.0 = 2.0 (capped)
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_D02_TargetSize_Multiplier::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Validate target exists
	if (!Context.InputContext.bHasTarget)
	{
		// No target - return multiplier of 1.0 (no change)
		OutOutput.DistanceOutput = 1.0f;
		OutOutput.bHasDistanceOutput = true;
		OutOutput.bDistanceIsMultiplier = true;
		return false;
	}
	
	// Step 2: Get target size and config values
	float TargetSize = Context.InputContext.TargetSize;
	
	// Get threshold from config
	float SizeThreshold = StateConfig.Module.Distance.TargetSizeThreshold;
	if (SizeThreshold <= 0.0f)
	{
		SizeThreshold = 150.0f;  // Default: 1.5 meters
	}
	
	// Get distance scale factor from config
	// This determines how much the multiplier increases per 100cm
	float DistanceScale = StateConfig.Module.Distance.TargetSizeDistanceScale;
	if (DistanceScale <= 0.0f)
	{
		DistanceScale = 1.5f;  // Default: 1.5x at 100cm above threshold
	}
	
	// Step 3: Calculate how much larger target is than threshold
	float SizeAboveThreshold = FMath::Max(0.0f, TargetSize - SizeThreshold);
	
	// Step 4: Calculate multiplier addition
	// Formula: Every 100cm above threshold adds (DistanceScale - 1.0) to multiplier
	// 
	// Example calculations:
	// - Target=200cm, Threshold=150cm, Scale=1.5:
	//   SizeAbove=50, Add=(50/100)*(1.5-1.0)=0.25, Mult=1.25
	// - Target=350cm, Threshold=150cm, Scale=1.5:
	//   SizeAbove=200, Add=(200/100)*(1.5-1.0)=1.0, Mult=2.0 (capped)
	// - Target=500cm (large boss), Threshold=150cm, Scale=1.5:
	//   SizeAbove=350, Add=(350/100)*(1.5-1.0)=1.75, Mult=2.0 (capped)
	float MultiplierAdd = (SizeAboveThreshold / 100.0f) * (DistanceScale - 1.0f);
	
	// Step 5: Calculate final multiplier and clamp to valid range
	// Minimum: 1.0 (no reduction)
	// Maximum: 2.0 (double distance, prevents excessive pullback)
	float Multiplier = 1.0f + MultiplierAdd;
	Multiplier = FMath::Clamp(Multiplier, 1.0f, 2.0f);
	
	// Step 6: Calculate dynamic weight based on size significance
	// Larger targets = higher weight = more influence
	// Weight reaches 1.0 when target is 200cm above threshold
	float Weight = FMath::Clamp(SizeAboveThreshold / 200.0f, 0.0f, 1.0f);
	
	// Step 7: Fill output structure
	OutOutput.DistanceOutput = Multiplier;
	OutOutput.bHasDistanceOutput = true;
	OutOutput.bDistanceIsMultiplier = true;  // Mark as multiplier for Stage 5
	OutOutput.Weight = Weight;
	
	return true;
}


//========================================
// D03: Speed Offset
//========================================

/**
 * Constructor - Initialize smooth offset state
 */
UCameraModule_D03_Speed_Offset::UCameraModule_D03_Speed_Offset()
	: CurrentSpeedOffset(0.0f)
{
}

/**
 * ShouldActivate - Determine if speed-based distance offset should be active
 * 
 * D03 activates when:
 * - Module is enabled
 * - Character is moving (bIsMoving)
 * - Character speed exceeds threshold
 * 
 * This module provides better forward visibility during fast movement
 * by pulling the camera back. Common in third-person action games.
 * 
 * Priority 105 (after D01, before D02), uses Additive policy.
 * 
 * Use Cases:
 * - Sprinting through open areas
 * - Running to objectives
 * - Escape sequences
 */
bool UCameraModule_D03_Speed_Offset::ShouldActivate(
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
	float SpeedThreshold = StateConfig.Module.Distance.SpeedDistanceThreshold;
	
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
	
	// All conditions met - activate speed offset
	return true;
}

/**
 * Compute - Calculate distance offset based on movement speed
 * 
 * This module calculates an additive distance offset based on how fast
 * the character is moving. Faster movement = larger offset = camera farther back.
 * 
 * Steps:
 * 1. Get current speed and config values
 * 2. Calculate speed above threshold
 * 3. Calculate speed ratio (0 to 1)
 * 4. Calculate target offset from ratio
 * 5. Smooth the offset transition
 * 6. Output with Additive policy
 * 
 * Speed to Offset Mapping:
 * - At threshold (200cm/s): offset = 0cm
 * - At run speed (400cm/s): offset = ~50cm
 * - At sprint speed (600cm/s): offset = ~100cm (max)
 * 
 * The offset is smoothed to prevent sudden camera distance changes
 * when starting/stopping sprints.
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_D03_Speed_Offset::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get current speed and config values
	float CurrentSpeed = Context.InputContext.CharacterSpeed;
	
	// Get speed threshold from config
	float SpeedThreshold = StateConfig.Module.Distance.SpeedDistanceThreshold;
	if (SpeedThreshold <= 0.0f)
	{
		SpeedThreshold = 200.0f;  // Default: 2 m/s (walking speed)
	}
	
	// Get maximum speed offset from config
	float MaxSpeedOffset = StateConfig.Module.Distance.SpeedDistanceMax;
	if (MaxSpeedOffset <= 0.0f)
	{
		MaxSpeedOffset = 100.0f;  // Default: 1 meter max offset
	}
	
	// Step 2: Calculate speed above threshold
	float SpeedAboveThreshold = FMath::Max(0.0f, CurrentSpeed - SpeedThreshold);
	
	// Step 3: Calculate speed ratio (0 to 1)
	// Assume max effective speed range is 400cm/s above threshold
	// (threshold + 400 = typical sprint speed of 600cm/s)
	const float MaxSpeedRange = 400.0f;
	float SpeedRatio = FMath::Clamp(SpeedAboveThreshold / MaxSpeedRange, 0.0f, 1.0f);
	
	// Step 4: Calculate target offset from ratio
	// Linear mapping: ratio 0 -> 0cm, ratio 1 -> MaxSpeedOffset
	float TargetOffset = SpeedRatio * MaxSpeedOffset;
	
	// Step 5: Smooth the offset transition
	// This prevents jarring camera distance changes
	float InterpSpeed = 5.0f;  // Smoothing speed
	
	// Try to get interp speed from config
	float ConfigLagSpeed = StateConfig.StateBase.Lag.DistanceLagSpeed;
	if (ConfigLagSpeed > 0.0f)
	{
		InterpSpeed = ConfigLagSpeed;
	}
	
	// Interpolate current offset towards target
	CurrentSpeedOffset = FMath::FInterpTo(
		CurrentSpeedOffset, 
		TargetOffset, 
		Context.DeltaTime, 
		InterpSpeed
	);
	
	// If speed drops below threshold, smoothly return to zero
	if (CurrentSpeed <= SpeedThreshold)
	{
		CurrentSpeedOffset = FMath::FInterpTo(
			CurrentSpeedOffset, 
			0.0f, 
			Context.DeltaTime, 
			InterpSpeed
		);
	}
	
	// Step 6: Fill output structure
	OutOutput.DistanceOutput = CurrentSpeedOffset;
	OutOutput.bHasDistanceOutput = true;
	OutOutput.bDistanceIsMultiplier = false;  // Additive policy
	
	// Full weight for speed offset
	OutOutput.Weight = 1.0f;
	
	return true;
}


//========================================
// D04: Combat Adjust
//========================================

/**
 * ShouldActivate - Determine if combat distance adjustment should be active
 * 
 * D04 activates when: 
 * - Module is enabled
 * - Player is in combat state (bIsInCombat)
 * 
 * Note: This activates whether or not a target is locked. 
 * - With locked target: Full combat adjustment
 * - Without locked target: Partial adjustment (combat awareness)
 * 
 * Priority 115 (higher than D03 speed offset), uses Additive policy. 
 * Combat typically brings camera closer for more intense framing.
 * 
 * Use Cases:
 * - Lock-on melee combat (closer camera)
 * - General combat awareness (moderate adjustment)
 * - Boss fights (combined with D07 phase scaling)
 */
bool UCameraModule_D04_Combat_Adjust::ShouldActivate(
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
	
	// All conditions met - activate combat adjustment
	return true;
}

/**
 * Compute - Calculate combat distance adjustment
 * 
 * This module adjusts camera distance during combat. 
 * Typically brings camera closer for more intense, focused framing.
 * 
 * Steps:
 * 1. Get combat distance offset from config
 * 2. Adjust offset based on target lock status
 * 3. Apply validation and clamping
 * 4. Output with Additive policy
 * 
 * Offset Behavior:
 * - Negative offset: Camera moves closer (typical for melee combat)
 * - Positive offset: Camera moves farther (for ranged combat overview)
 * - With target locked: Full offset applied
 * - Without target: Partial offset (50%) for general combat awareness
 * 
 * Typical Values:
 * - Melee combat offset: -50cm to -100cm (closer)
 * - Ranged combat offset: +50cm to +100cm (farther)
 * - Default: -50cm (slightly closer)
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_D04_Combat_Adjust::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get combat distance offset from config
	// Negative value = closer camera (typical for melee)
	// Positive value = farther camera (for ranged overview)
	float CombatOffset = StateConfig.Module.Distance.CombatDistanceOffset;
	
	// If not configured, use default (closer for melee combat)
	if (FMath::IsNearlyZero(CombatOffset))
	{
		CombatOffset = -50.0f;  // Default: 50cm closer
	}
	
	// Step 2: Adjust offset based on target lock status
	float FinalOffset = 0.0f;
	
	if (Context.InputContext.bHasTarget)
	{
		// With locked target: Apply full combat offset
		// This provides tight framing on the target
		FinalOffset = CombatOffset;
	}
	else
	{
		// Without locked target: Apply partial offset (50%)
		// This provides general combat awareness without being too close
		// Player is in combat but not focused on specific enemy
		FinalOffset = CombatOffset * 0.5f;
	}
	
	// Step 3: Clamp offset to reasonable range
	// Prevent extreme values that could make camera too close or too far
	const float MinOffset = -150.0f;  // Max 1.5m closer
	const float MaxOffset = 150.0f;   // Max 1.5m farther
	FinalOffset = FMath::Clamp(FinalOffset, MinOffset, MaxOffset);
	
	// Step 4: Fill output structure
	OutOutput.DistanceOutput = FinalOffset;
	OutOutput.bHasDistanceOutput = true;
	OutOutput.bDistanceIsMultiplier = false;  // Additive policy
	
	// Full weight in combat (this is an authoritative adjustment)
	OutOutput.Weight = 1.0f;
	
	return true;
}

//========================================
// D05: Environment Limit
//========================================

/**
 * ShouldActivate - Determine if environment distance limiting should be active
 * 
 * D05 activates when: 
 * - Module is enabled
 * - Player is in a tight space OR under a low ceiling
 * 
 * These flags are typically set by trigger volumes or environment detection
 * systems in the game world. 
 * 
 * Priority 120 (high), uses Minimum policy. 
 * This ensures the camera distance doesn't exceed what the environment allows,
 * preventing wall clipping and awkward camera positioning.
 * 
 * Use Cases:
 * - Indoor corridors
 * - Caves and tunnels
 * - Low ceiling areas
 * - Small rooms
 */
bool UCameraModule_D05_Environment_Limit::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check environment flags
	bool bInTightSpace = Context.InputContext.bInTightSpace;
	bool bUnderLowCeiling = Context.InputContext.bUnderLowCeiling;
	
	// Activate if either condition is true
	if (!bInTightSpace && !bUnderLowCeiling)
	{
		return false;
	}
	
	// Environmental constraint detected - activate limiting
	return true;
}

/**
 * Compute - Calculate maximum allowed distance based on environment
 * 
 * This module calculates a distance limit based on environmental constraints. 
 * Using Minimum blend policy, it ensures the final distance doesn't exceed
 * what the current environment allows.
 * 
 * Steps:
 * 1. Start with configured max distance as baseline
 * 2. If in tight space, apply tight space reduction
 * 3. If under low ceiling, apply ceiling limit
 * 4. Ensure result doesn't go below minimum distance
 * 5. Output with Minimum policy
 * 
 * Environment Types:
 * - Tight Space: Narrow corridors, small rooms
 *   -> Reduces distance significantly (75% of base)
 * - Low Ceiling: Caves, tunnels, low roofs
 *   -> Limits distance to prevent ceiling clipping (1.5x min)
 * 
 * The Minimum policy means:
 * FinalDistance = min(CurrentDistance, ThisLimit)
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_D05_Environment_Limit::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Set Minimum blend policy (critical)
	// This ensures we take the smaller of current and limit
	OutOutput.BlendPolicy = EBlendPolicy::Minimum;
	
	// Step 1: Get configured distances
	float MaxAllowedDistance = StateConfig.StateBase.Distance.MaxDistance;
	if (MaxAllowedDistance <= 0.0f)
	{
		MaxAllowedDistance = 800.0f;  // Default max: 8 meters
	}
	
	float BaseDistance = StateConfig.StateBase.Distance.BaseDistance;
	if (BaseDistance <= 0.0f)
	{
		BaseDistance = 400.0f;  // Default: 4 meters
	}
	
	float MinDistance = StateConfig.StateBase.Distance.MinDistance;
	if (MinDistance <= 0.0f)
	{
		MinDistance = 100.0f;  // Default min: 1 meter
	}
	
	// Step 2: Apply tight space reduction
	if (Context.InputContext.bInTightSpace)
	{
		// Tight spaces require shorter camera distance
		// Reduce to 75% of base distance
		float TightSpaceLimit = BaseDistance * 0.75f;
		MaxAllowedDistance = FMath::Min(MaxAllowedDistance, TightSpaceLimit);
	}
	
	// Step 3: Apply low ceiling limit
	if (Context.InputContext.bUnderLowCeiling)
	{
		// Low ceiling typically requires even shorter distance
		// Use 1.5x minimum distance as the ceiling limit
		float LowCeilingLimit = MinDistance * 1.5f;
		MaxAllowedDistance = FMath::Min(MaxAllowedDistance, LowCeilingLimit);
	}
	
	// Step 4: Ensure we don't go below minimum distance
	// Even in tight spaces, camera needs some distance to function
	MaxAllowedDistance = FMath::Max(MaxAllowedDistance, MinDistance);
	
	// Step 5: Fill output structure
	OutOutput.DistanceOutput = MaxAllowedDistance;
	OutOutput.bHasDistanceOutput = true;
	OutOutput.bDistanceIsMultiplier = false;  // This is an absolute limit
	
	// Weight is 1.0 since this is a hard constraint
	OutOutput.Weight = 1.0f;
	
	return true;
}

//========================================
// D06: Proximity Adjust
//========================================

/**
 * ShouldActivate - Determine if proximity-based adjustment should be active
 * 
 * D06 activates when:
 * - Module is enabled
 * - Player has a locked target
 * 
 * This module creates a dynamic relationship between player-target distance
 * and camera distance. When player is close to target, camera pulls in;
 * when far from target, camera pulls back.
 * 
 * Priority 112, uses Blend policy with dynamic weight.
 * Blends with other distance calculations rather than overriding.
 * 
 * Use Cases:
 * - Close combat: Camera tightens for intimate combat feel
 * - Ranged combat: Camera pulls back for better overview
 * - Dynamic combat: Smooth transitions as distance changes
 */
bool UCameraModule_D06_Proximity_Adjust::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if player has a locked target
	// Proximity adjustment only makes sense with a target
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	// All conditions met - activate proximity adjustment
	return true;
}

/**
 * Compute - Calculate camera distance based on player-target proximity
 * 
 * This module adjusts camera distance dynamically based on how close
 * the player is to the locked target. It creates a natural camera
 * behavior that responds to combat engagement distance.
 * 
 * Steps:
 * 1. Validate target exists
 * 2. Get player-target distance
 * 3. Define proximity ranges (close, mid, far)
 * 4. Calculate distance adjustment based on range
 * 5. Apply adjustment to base distance
 * 6. Set appropriate blend weight
 * 7. Output with Blend policy
 * 
 * Proximity Ranges:
 * - Close (<200cm): Camera pulls in (up to -50cm)
 * - Mid (200-500cm): Slight adjustment (-20cm to 0)
 * - Far (500-1000cm): Camera pulls back (0 to +30cm)
 * - Very Far (>1000cm): Max pullback (+30cm)
 * 
 * The result is a camera that "breathes" with the combat distance,
 * creating a more dynamic and responsive feel.
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_D06_Proximity_Adjust::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Validate target exists
	if (!Context.InputContext.bHasTarget)
	{
		OutOutput.bHasDistanceOutput = false;
		return false;
	}
	
	// Step 2: Get player-target distance
	float TargetDistance = Context.InputContext.TargetDistance;
	
	// If TargetDistance not available, calculate from positions
	if (TargetDistance <= 0.0f)
	{
		FVector PlayerLocation = Context.InputContext.CharacterLocation;
		FVector TargetLocation = Context.InputContext.TargetLocation;
		TargetDistance = FVector::Dist(PlayerLocation, TargetLocation);
	}
	
	// Step 3: Get base distance for reference
	float BaseDistance = StateConfig.StateBase.Distance.BaseDistance;
	if (BaseDistance <= 0.0f)
	{
		BaseDistance = 400.0f;  // Default: 4 meters
	}
	
	// Step 4: Define proximity ranges (in centimeters)
	// These define how player-target distance maps to camera adjustment
	const float CloseRange = 200.0f;    // Very close combat (< 2m)
	const float MidRange = 500.0f;      // Normal combat range (2-5m)
	const float FarRange = 1000.0f;     // Far engagement (5-10m)
	
	// Step 5: Calculate distance adjustment based on proximity range
	float DistanceAdjustment = 0.0f;
	
	if (TargetDistance < CloseRange)
	{
		// Very close combat - pull camera in significantly
		// At 0m: adjustment = -50cm (closest)
		// At 2m: adjustment = 0cm (transition to mid)
		float Ratio = TargetDistance / CloseRange;  // 0 to 1
		float MaxCloseAdjust = -50.0f;  // Max pull-in at closest
		DistanceAdjustment = MaxCloseAdjust * (1.0f - Ratio);
	}
	else if (TargetDistance < MidRange)
	{
		// Normal combat range - slight pull-in that fades to neutral
		// At 2m: adjustment = -20cm
		// At 5m: adjustment = 0cm
		float Ratio = (TargetDistance - CloseRange) / (MidRange - CloseRange);  // 0 to 1
		DistanceAdjustment = FMath::Lerp(-20.0f, 0.0f, Ratio);
	}
	else if (TargetDistance < FarRange)
	{
		// Far range - pull camera back for better overview
		// At 5m: adjustment = 0cm
		// At 10m: adjustment = +30cm
		float Ratio = (TargetDistance - MidRange) / (FarRange - MidRange);  // 0 to 1
		DistanceAdjustment = FMath::Lerp(0.0f, 30.0f, Ratio);
	}
	else
	{
		// Very far (>10m) - max pullback
		// Camera stays pulled back for distant engagement
		DistanceAdjustment = 30.0f;
	}
	
	// Step 6: Calculate final distance
	float AdjustedDistance = BaseDistance + DistanceAdjustment;
	
	// Ensure adjusted distance stays within reasonable bounds
	float MinDistance = StateConfig.StateBase.Distance.MinDistance;
	if (MinDistance <= 0.0f)
	{
		MinDistance = 100.0f;
	}
	
	float MaxDistance = StateConfig.StateBase.Distance.MaxDistance;
	if (MaxDistance <= 0.0f)
	{
		MaxDistance = 800.0f;
	}
	
	AdjustedDistance = FMath::Clamp(AdjustedDistance, MinDistance, MaxDistance);
	
	// Step 7: Fill output structure
	OutOutput.DistanceOutput = AdjustedDistance;
	OutOutput.bHasDistanceOutput = true;
	OutOutput.bDistanceIsMultiplier = false;  // This is an absolute value
	
	// Set blend weight - partial influence to allow other modules to contribute
	// 0.6 means 60% influence, blends with other distance calculations
	OutOutput.Weight = 0.6f;
	
	return true;
}


//========================================
// D07: Boss Phase
//========================================

/**
 * ShouldActivate - Determine if boss phase distance scaling should be active
 * 
 * D07 activates when:
 * - Module is enabled
 * - Player has a locked target
 * - Target IS a boss (bTargetIsBoss)
 * 
 * This module provides phase-specific distance scaling for boss fights.
 * Different boss phases may require different camera distances for
 * optimal gameplay visibility and cinematic effect.
 * 
 * Priority 125 (highest among Distance modules), uses Multiplicative policy.
 * This ensures boss-specific scaling is applied last, on top of all other
 * distance calculations.
 * 
 * Use Cases:
 * - Phase 1: Normal distance (1.0x)
 * - Phase 2 (Enraged): Pull back slightly (1.2x)
 * - Phase 3 (Final): Maximum pullback for dramatic effect (1.4x)
 * - Large bosses: Additional size-based scaling
 */
bool UCameraModule_D07_Boss_Phase::ShouldActivate(
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
	// This is the key condition that distinguishes D07 from D02
	if (!Context.InputContext.bTargetIsBoss)
	{
		return false;
	}
	
	// All conditions met - activate boss phase scaling
	return true;
}

/**
 * Compute - Calculate boss phase distance multiplier
 * 
 * This module calculates a distance multiplier specifically for boss fights.
 * It considers both the boss phase (from config) and the boss size to
 * determine the optimal camera distance.
 * 
 * Steps:
 * 1. Validate boss target
 * 2. Get boss phase distance scale from config
 * 3. Calculate size-based modifier for large bosses
 * 4. Combine phase scale with size modifier
 * 5. Clamp to valid range
 * 6. Output with Multiplicative policy
 * 
 * Phase Scaling Examples:
 * - Phase 1 (Normal):  BossPhaseDistanceScale = 1.0
 * - Phase 2 (Enraged): BossPhaseDistanceScale = 1.2
 * - Phase 3 (Final):   BossPhaseDistanceScale = 1.4
 * 
 * Size Modifier (for very large bosses):
 * - Boss < 300cm:   modifier = 1.0 (no additional scaling)
 * - Boss 300-800cm: modifier = 1.0 to 1.3 (gradual increase)
 * - Boss > 800cm:   modifier = 1.3 (capped)
 * 
 * Final Multiplier = PhaseScale x SizeModifier
 * Clamped to 0.8 - 2.0 range
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_D07_Boss_Phase::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Validate boss target
	if (!Context.InputContext.bTargetIsBoss)
	{
		// Not a boss - return multiplier of 1.0 (no change)
		OutOutput.DistanceOutput = 1.0f;
		OutOutput.bHasDistanceOutput = true;
		OutOutput.bDistanceIsMultiplier = true;
		return false;
	}
	
	// Step 2: Get boss phase distance scale from config
	// This value is typically set by the boss fight system based on current phase
	float BossPhaseScale = StateConfig.Module.Distance.BossPhaseDistanceScale;
	
	// If not configured, use default (Phase 1 = no scaling)
	if (BossPhaseScale <= 0.0f)
	{
		BossPhaseScale = 1.0f;  // Default: no phase scaling
	}
	
	// Step 3: Calculate size-based modifier for large bosses
	// Very large bosses may need additional distance beyond phase scaling
	float TargetSize = Context.InputContext.TargetSize;
	float SizeModifier = 1.0f;
	
	// Size threshold for additional scaling
	const float LargeBossThreshold = 300.0f;  // 3 meters
	const float MaxSizeForScaling = 800.0f;   // 8 meters (cap)
	const float MaxSizeModifier = 1.3f;       // Max 30% additional
	
	if (TargetSize > LargeBossThreshold)
	{
		// Calculate how much larger than threshold
		float SizeAboveThreshold = TargetSize - LargeBossThreshold;
		float ScalingRange = MaxSizeForScaling - LargeBossThreshold;  // 500cm
		
		// Calculate modifier (0 to 0.3, then add 1.0)
		// At 300cm: modifier = 1.0
		// At 550cm: modifier = 1.15
		// At 800cm+: modifier = 1.3 (capped)
		float ModifierAdd = (SizeAboveThreshold / ScalingRange) * (MaxSizeModifier - 1.0f);
		SizeModifier = 1.0f + FMath::Clamp(ModifierAdd, 0.0f, MaxSizeModifier - 1.0f);
	}
	
	// Step 4: Combine phase scale with size modifier
	// Both are multipliers, so they compound
	float FinalMultiplier = BossPhaseScale * SizeModifier;
	
	// Step 5: Clamp to reasonable range
	// Minimum 0.8: Prevent camera from getting too close to boss
	// Maximum 2.0: Prevent camera from getting too far (match D02 limit)
	FinalMultiplier = FMath::Clamp(FinalMultiplier, 0.8f, 2.0f);
	
	// Step 6: Fill output structure
	OutOutput.DistanceOutput = FinalMultiplier;
	OutOutput.bHasDistanceOutput = true;
	OutOutput.bDistanceIsMultiplier = true;  // This is a multiplier, not absolute
	
	// Full weight for boss fights - authoritative scaling
	OutOutput.Weight = 1.0f;
	
	return true;
}
