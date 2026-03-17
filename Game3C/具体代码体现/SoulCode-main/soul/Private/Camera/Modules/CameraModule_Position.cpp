// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModule_Position.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModule_Position.cpp
 * 
 * This file contains the implementations of 8 Position modules (P01-P08).
 * Position modules are responsible for calculating the camera focus point (FocusPoint),
 * which determines where the camera should be looking at or following.
 * 
 * Output is passed through FModuleOutput.PositionOffset field.
 * 
 * Module List:
 * - P01: Follow Target - Basic character following (implemented)
 * - P02: Follow Target Lagged - Smooth interpolated following (implemented)
 * - P03: Follow Target Predictive - Velocity-based anticipation (implemented)
 * - P04: Orbit Lock-On - Orbit around player when locked on (implemented)
 * - P05: Orbit Boss - Special orbit for boss encounters (implemented)
 * - P06: Fixed Point - Focus on fixed world location (implemented)
 * - P07: Spline Follow - Follow predefined spline path (placeholder implemented)
 * - P08: MidPoint Two Target - Focus between player and target (TODO: Sub-Step 3.2.9)
 */


//========================================
// P01: Follow Target - Basic character following
//========================================

/**
 * ShouldActivate - Determines if P01 module should be active
 * 
 * P01 is the most basic Position module and serves as a fallback.
 * It should always be active as long as the module is enabled.
 * No special conditions are checked - this is the foundation module.
 */
bool UCameraModule_P01_FollowTarget::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Basic follow is always active as fallback
	// Only check if module is enabled
	return bIsEnabled;
}

/**
 * Compute - Calculate focus point for basic character following
 * 
 * Steps:
 * 1. Get character world location
 * 2. Apply focus offset from state config
 * 3. Apply additional vertical offset
 * 4. Output the final focus point
 * 
 * This module directly uses character location without interpolation,
 * providing immediate response to character movement.
 */
bool UCameraModule_P01_FollowTarget::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity (priority, blend policy, etc.)
	OutOutput = CreateBaseOutput();
	
	// Get character world location from input context
	FVector CharacterLocation = Context.InputContext.CharacterLocation;
	
	// Get focus offset from state configuration
	// FocusOffset is relative to character (world space offset)
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	
	// Get additional vertical offset for height adjustment
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	
	// Calculate final focus point
	// Start with character location, add configured offset
	FVector FocusPoint = CharacterLocation + FocusOffset;
	
	// Apply additional vertical offset on top
	FocusPoint.Z += VerticalOffset;
	
	// Fill output structure
	// PositionOffset stores the world-space focus point
	OutOutput.PositionOffset = FocusPoint;
	
	// Mark that this module provides position output
	// Stage 5 (Blend & Solve) uses this flag to know which outputs to blend
	OutOutput.bHasPositionOutput = true;
	
	return true;
}


//========================================
// P02: Follow Target Lagged - Smooth interpolated following
//========================================

/**
 * Constructor - Initializes interpolation state
 * 
 * Sets up initial values for smooth position interpolation.
 * CurrentPosition starts at zero and will be properly initialized
 * when the module activates or computes its first frame.
 */
UCameraModule_P02_FollowTarget_Lagged::UCameraModule_P02_FollowTarget_Lagged()
	: CurrentPosition(FVector::ZeroVector)
	, bHasInitialPosition(false)
{
}

/**
 * OnActivate - Initialize interpolation state when module becomes active
 * 
 * When P02 activates, we initialize CurrentPosition to the character's
 * current location to avoid a sudden jump from zero to the target position
 * on the first frame. This ensures smooth transition from the moment
 * the module activates.
 * 
 * @param Context The current pipeline execution context
 */
void UCameraModule_P02_FollowTarget_Lagged::OnActivate(const FStageExecutionContext& Context)
{
	Super::OnActivate(Context);
	
	// Initialize CurrentPosition to character location to avoid initial jump
	// This ensures smooth transition from the moment module activates
	CurrentPosition = Context.InputContext.CharacterLocation;
	
	// If the pipeline already has a valid focus point from previous frame,
	// use it to maintain continuity when switching from another position module
	if (!Context.Output.FocusPoint.IsNearlyZero())
	{
		CurrentPosition = Context.Output.FocusPoint;
	}
	
	// Mark that we now have a valid initial position
	bHasInitialPosition = true;
}

/**
 * Compute - Calculate smoothly lagged follow position
 * 
 * This module provides cinematic smooth following using exponential interpolation.
 * The CurrentPosition smoothly approaches the target position based on LagSpeed.
 * 
 * Steps:
 * 1. Calculate target position (character + offset)
 * 2. Initialize CurrentPosition if first frame
 * 3. Get lag speed from state config
 * 4. Interpolate CurrentPosition towards target using exponential decay
 * 5. Output the interpolated position
 * 
 * The exponential interpolation (via GetInterpAlpha) provides:
 * - Frame-rate independent smoothing
 * - Natural deceleration as it approaches target
 * - Consistent behavior across different frame rates
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_P02_FollowTarget_Lagged::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity (priority, blend policy, etc.)
	OutOutput = CreateBaseOutput();
	
	// Step 1: Calculate target position (same as P01)
	// Get character world location from input context
	FVector CharacterLocation = Context.InputContext.CharacterLocation;
	
	// Get focus offset from state configuration
	// FocusOffset is relative to character (world space offset)
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	
	// Get additional vertical offset for height adjustment
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	
	// Calculate target position (where we want to smoothly move towards)
	FVector TargetPosition = CharacterLocation + FocusOffset;
	TargetPosition.Z += VerticalOffset;
	
	// Step 2: Initialize if this is the first frame
	if (!bHasInitialPosition)
	{
		// First frame: snap to target to avoid interpolating from zero
		CurrentPosition = TargetPosition;
		bHasInitialPosition = true;
	}
	
	// Step 3: Get lag speed from state configuration
	// Higher LagSpeed = faster interpolation (less lag)
	// Lower LagSpeed = slower interpolation (more lag, more cinematic)
	float LagSpeed = StateConfig.StateBase.Lag.PositionLagSpeed;
	
	// Step 4: Calculate interpolation alpha using exponential decay
	// GetInterpAlpha provides frame-rate independent smoothing
	float InterpAlpha = GetInterpAlpha(Context.DeltaTime, LagSpeed);
	
	// Smoothly interpolate CurrentPosition towards TargetPosition
	// CurrentPosition will exponentially approach TargetPosition over time
	CurrentPosition = FMath::Lerp(CurrentPosition, TargetPosition, InterpAlpha);
	
	// Step 5: Output the smoothly interpolated position
	// PositionOffset stores the world-space focus point
	OutOutput.PositionOffset = CurrentPosition;
	
	// Mark that this module provides position output
	// Stage 5 (Blend & Solve) uses this flag to know which outputs to blend
	OutOutput.bHasPositionOutput = true;
	
	return true;
}


//========================================
// P03: Follow Target Predictive - Anticipates movement
//========================================

/**
 * ShouldActivate - Determine if predictive following should be active
 * 
 * P03 activates when the character is moving, as prediction is only
 * useful during movement. When stationary, basic follow (P01) is sufficient.
 * 
 * The module uses dynamic weight based on speed, so it can be active
 * even during slow movement (with low weight).
 */
bool UCameraModule_P03_FollowTarget_Predictive::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Activate when character is moving
	// bIsMoving flag indicates character has non-zero velocity
	return Context.InputContext.bIsMoving;
}

/**
 * Compute - Calculate predicted follow position based on velocity
 * 
 * This module implements velocity-based prediction to make the camera
 * look ahead in the direction of movement. This provides:
 * - Smoother tracking during fast movement
 * - More screen space in front of the character
 * - Anticipation of direction changes
 * 
 * Steps:
 * 1. Calculate base focus position (character + offset)
 * 2. Get character velocity from input context
 * 3. Calculate predicted offset based on velocity and prediction time
 * 4. Clamp prediction to maximum distance
 * 5. Apply only horizontal prediction (ignore vertical/falling)
 * 6. Calculate dynamic weight based on movement speed
 * 7. Output predicted position with weight
 * 
 * The weight system allows this module to blend with other position
 * modules - higher speed = more prediction influence.
 */
bool UCameraModule_P03_FollowTarget_Predictive::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Override blend policy to Blend (not Override)
	// This allows prediction to blend with other position modules
	OutOutput.BlendPolicy = EBlendPolicy::Blend;
	
	// Step 1: Get character location and velocity
	FVector CharacterLocation = Context.InputContext.CharacterLocation;
	FVector Velocity = Context.InputContext.CharacterVelocity;
	
	// Step 2: Apply standard focus offset
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	FocusOffset.Z += VerticalOffset;
	
	// Step 3: Get prediction time from state config
	// PredictionTime is in seconds - how far ahead to predict
	// Typical values: 0.2-0.5 seconds
	float PredictionTime = StateConfig.Module.Position.PredictionTime;
	
	// If prediction time not configured, use default
	if (PredictionTime <= 0.0f)
	{
		PredictionTime = 0.3f;  // Default: 300ms prediction
	}
	
	// Step 4: Calculate predicted offset
	// PredictedOffset = Velocity * Time (basic physics: distance = velocity * time)
	FVector PredictedOffset = Velocity * PredictionTime;
	
	// Step 5: Limit prediction distance to avoid excessive offset
	// MaxPrediction is in centimeters (UE4 units)
	const float MaxPrediction = 200.0f;  // Max 200cm (2 meters) prediction
	
	if (PredictedOffset.Size() > MaxPrediction)
	{
		// Clamp to max distance while preserving direction
		PredictedOffset = PredictedOffset.GetSafeNormal() * MaxPrediction;
	}
	
	// Step 6: Only predict horizontal movement
	// We don't want to predict falling/jumping (vertical velocity)
	// This keeps the camera stable during vertical movement
	PredictedOffset.Z = 0.0f;
	
	// Step 7: Calculate final focus point
	// Base position + standard offset + predicted offset
	FVector FocusPoint = CharacterLocation + FocusOffset + PredictedOffset;
	
	// Step 8: Fill output
	OutOutput.PositionOffset = FocusPoint;
	OutOutput.bHasPositionOutput = true;
	
	// Step 9: Calculate dynamic weight based on movement speed
	// Weight determines how much this module influences the final result
	// Faster movement = higher weight = more prediction
	// Slower movement = lower weight = less prediction (blends with basic follow)
	
	float CharacterSpeed = Context.InputContext.CharacterSpeed;
	
	// Normalize speed to 0-1 range
	// Assuming max run speed is around 600 cm/s (6 m/s)
	const float MaxRunSpeed = 600.0f;
	float SpeedRatio = FMath::Clamp(CharacterSpeed / MaxRunSpeed, 0.0f, 1.0f);
	
	// Apply speed ratio as weight
	// At low speed: weight ~0, prediction has minimal effect
	// At high speed: weight ~1, prediction has full effect
	OutOutput.Weight = SpeedRatio;
	
	return true;
}


//========================================
// P04: Orbit Lock-On - Orbits around player during lock-on
//========================================

/**
 * ShouldActivate - Determine if lock-on orbit should be active
 * 
 * P04 activates when:
 * - Module is enabled
 * - Player has a locked-on target
 * - Target is NOT a boss (boss uses P05 with wider orbit)
 * 
 * This module provides dynamic positioning to keep both player
 * and enemy visible during combat encounters.
 */
bool UCameraModule_P04_Orbit_LockOn::ShouldActivate(
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
	
	// Don't activate for boss targets (P05 handles those)
	// bTargetIsBoss flag indicates if locked target is a boss enemy
	if (Context.InputContext.bTargetIsBoss)
	{
		return false;
	}
	
	// All conditions met - activate lock-on orbit
	return true;
}

/**
 * Compute - Calculate orbit position for lock-on camera
 * 
 * This module calculates a focus point between the player and locked target
 * to ensure both are visible in frame. The calculation involves:
 * 
 * Steps:
 * 1. Validate target exists
 * 2. Get player and target locations
 * 3. Calculate weighted midpoint based on MidPointBias
 * 4. Apply standard focus offset
 * 5. Apply orbit radius adjustment to move focus toward player
 * 6. Output the calculated focus point
 * 
 * Design Goals:
 * - Keep both player and enemy visible
 * - Favor player framing slightly (via orbit offset)
 * - Provide stable focus during combat
 * - Allow configuration via StateConfig
 */
bool UCameraModule_P04_Orbit_LockOn::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Validate that we have a locked target
	// Safety check - ShouldActivate should prevent this, but double-check
	if (!Context.InputContext.bHasTarget)
	{
		// No target - cannot compute orbit position
		OutOutput.bHasPositionOutput = false;
		return false;
	}
	
	// Step 2: Get player and target world locations
	FVector PlayerLocation = Context.InputContext.CharacterLocation;
	FVector TargetLocation = Context.InputContext.TargetLocation;
	
	// Step 3: Calculate weighted midpoint between player and target
	// MidPointBias controls the weighting:
	// - 0.0 = focus on player
	// - 0.5 = focus on exact midpoint
	// - 1.0 = focus on target
	// Typical value: 0.3-0.5 (slight bias toward player)
	float MidPointBias = StateConfig.Module.Position.MidPointBias;
	
	// Validate MidPointBias is in valid range
	if (MidPointBias < 0.0f || MidPointBias > 1.0f)
	{
		MidPointBias = 0.3f;  // Default: slight bias toward player
	}
	
	// Calculate weighted midpoint using linear interpolation
	// This is the base focus point between player and target
	FVector FocusPoint = FMath::Lerp(PlayerLocation, TargetLocation, MidPointBias);
	
	// Step 4: Apply standard focus offset for height adjustment
	// This raises the focus point to upper body/head level
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	FocusOffset.Z += VerticalOffset;
	
	FocusPoint += FocusOffset;
	
	// Step 5: Apply orbit radius adjustment
	// This moves the focus point slightly toward the player for better framing
	// OrbitRadiusMultiplier allows configuration of how much to offset
	float OrbitMultiplier = StateConfig.Module.Position.OrbitRadiusMultiplier;
	
	// Validate OrbitMultiplier is in valid range
	if (OrbitMultiplier <= 0.0f)
	{
		OrbitMultiplier = 1.0f;  // Default: standard offset
	}
	
	// Calculate direction from target to player
	// GetSafeNormal() prevents division by zero if positions are identical
	FVector ToPlayer = (PlayerLocation - TargetLocation).GetSafeNormal();
	
	// Base orbit offset in centimeters
	// 50cm provides good framing without being too extreme
	const float BaseOrbitOffset = 50.0f;
	float OrbitOffset = BaseOrbitOffset * OrbitMultiplier;
	
	// Apply orbit offset along the player direction
	// This ensures the player is slightly favored in framing
	FocusPoint += ToPlayer * OrbitOffset;
	
	// Step 6: Fill output structure
	OutOutput.PositionOffset = FocusPoint;
	OutOutput.bHasPositionOutput = true;
	
	return true;
}


//========================================
// P05: Orbit Boss - Special boss fight orbit
//========================================

/**
 * ShouldActivate - Determine if boss orbit should be active
 * 
 * P05 activates when:
 * - Module is enabled
 * - Player has a locked-on target
 * - Target IS a boss (this is the key difference from P04)
 * 
 * Boss encounters require special camera handling due to:
 * - Larger enemy models (need wider framing)
 * - More dramatic attacks (need more screen space)
 * - Epic scale of combat (need cinematic positioning)
 * 
 * P05 uses different parameters than P04 to accommodate these needs.
 */
bool UCameraModule_P05_Orbit_Boss::ShouldActivate(
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
	
	// ONLY activate for boss targets
	// bTargetIsBoss flag indicates if locked target is a boss enemy
	if (!Context.InputContext.bTargetIsBoss)
	{
		return false;
	}
	
	// All conditions met - activate boss orbit
	return true;
}

/**
 * Compute - Calculate orbit position for boss fight camera
 * 
 * Boss fights require different camera positioning than regular combat:
 * - Wider orbit to capture large boss models
 * - More player-centric focus (Boss can be partially off-screen)
 * - Dynamic height adjustment based on boss size
 * - Larger orbit offset to provide better framing
 * 
 * Steps:
 * 1. Validate boss target exists
 * 2. Get player, target locations and target size
 * 3. Calculate weighted midpoint (more biased toward player)
 * 4. Apply enhanced vertical offset (bosses are taller)
 * 5. Add dynamic height based on boss size
 * 6. Apply larger orbit radius for wider framing
 * 7. Output the calculated focus point
 * 
 * Key Differences from P04:
 * - MidPointBias: 0.2 vs 0.5 (more toward player)
 * - OrbitOffset: 80cm vs 50cm (wider orbit)
 * - Height Offset: Dynamic based on boss size
 * - BossPhaseScale: Adjusts positioning per boss phase
 */
bool UCameraModule_P05_Orbit_Boss::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Validate that we have a locked boss target
	// Safety check - ShouldActivate should prevent this, but double-check
	if (!Context.InputContext.bHasTarget)
	{
		// No target - cannot compute boss orbit position
		OutOutput.bHasPositionOutput = false;
		return false;
	}
	
	// Step 2: Get player and boss world locations
	FVector PlayerLocation = Context.InputContext.CharacterLocation;
	FVector TargetLocation = Context.InputContext.TargetLocation;
	
	// Get boss size for dynamic adjustments
	// TargetSize is typically the boss's capsule radius or bounding sphere radius
	float TargetSize = Context.InputContext.TargetSize;
	
	// If TargetSize not available, use default boss size
	if (TargetSize <= 0.0f)
	{
		TargetSize = 150.0f;  // Default: 150cm radius (typical boss size)
	}
	
	// Step 3: Calculate weighted midpoint with strong bias toward player
	// Boss battles use MidPointBias = 0.2 (much closer to player)
	// This keeps the player centered while still showing the boss
	// 
	// Comparison: 
	// - Regular combat (P04): 0.5 (equal)
	// - Boss combat (P05): 0.2 (player-centric)
	const float BossMidPointBias = 0.2f;
	
	FVector FocusPoint = FMath::Lerp(PlayerLocation, TargetLocation, BossMidPointBias);
	
	// Step 4: Apply standard focus offset for base height
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	FocusOffset.Z += VerticalOffset;
	
	// Step 5: Add dynamic height offset based on boss size
	// Larger bosses need higher camera focus to capture their upper body/head
	// Scale factor: 0.2 means 20% of boss size is added as height
	// Clamped to max 100cm to prevent excessive height for giant bosses
	float BossHeightOffset = FMath::Clamp(TargetSize * 0.2f, 0.0f, 100.0f);
	FocusOffset.Z += BossHeightOffset;
	
	// Apply total vertical offset
	FocusPoint += FocusOffset;
	
	// Step 6: Get boss phase distance scale
	// BossPhaseDistanceScale adjusts camera behavior during different boss phases
	// - Phase 1 (start): 1.0 (normal distance)
	// - Phase 2 (enraged): 1.2 (pull back slightly)
	// - Phase 3 (final): 1.5 (pull back more for epic finale)
	float BossPhaseScale = StateConfig.Module.Distance.BossPhaseDistanceScale;
	
	// If BossPhaseDistanceScale not configured, use default
	if (BossPhaseScale <= 0.0f)
	{
		BossPhaseScale = 1.0f;  // Default: no scaling
	}
	
	// Step 7: Apply larger orbit offset for boss fights
	// Calculate direction from boss to player
	FVector ToPlayer = (PlayerLocation - TargetLocation).GetSafeNormal();
	
	// Boss orbit offset: 80cm (vs 50cm for regular combat)
	// This wider orbit ensures:
	// - Player has good visibility
	// - Boss attacks are visible from better angle
	// - More dramatic framing
	const float BaseBossOrbitOffset = 80.0f;
	float OrbitOffset = BaseBossOrbitOffset * BossPhaseScale;
	
	// Apply orbit offset along the player direction
	FocusPoint += ToPlayer * OrbitOffset;
	
	// Step 8: Fill output structure
	OutOutput.PositionOffset = FocusPoint;
	OutOutput.bHasPositionOutput = true;
	
	return true;
}


//========================================
// P06: Fixed Point - Focus on fixed world location
//========================================

/**
 * ShouldActivate - Determine if fixed point focus should be active
 * 
 * P06 activates when:
 * - Module is enabled
 * - Current camera state is cinematic (bIsCinematic flag)
 * - StateConfig defines a valid fixed point location (non-zero)
 * 
 * Use Cases:
 * - Cutscenes with pre-defined camera focus points
 * - Level-specific scripted camera moments
 * - Cinematic transitions between gameplay sections
 * - Fixed camera angles for specific areas
 * 
 * Level designers can set FixedPointLocation in the camera state
 * to direct the camera's attention to specific world positions
 * (e.g., important objects, dramatic vista points, boss entrances).
 */
bool UCameraModule_P06_FixedPoint::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if current state is cinematic
	// bIsCinematic flag indicates this is a non-gameplay camera state
	if (!StateConfig.StateBase.Flags.bIsCinematic)
	{
		return false;
	}
	
	// Check if a valid fixed point is defined
	// FixedPointLocation should be non-zero to be considered valid
	FVector FixedPoint = StateConfig.Module.Position.FixedPointLocation;
	if (FixedPoint.IsNearlyZero())
	{
		// No valid fixed point defined - don't activate
		return false;
	}
	
	// All conditions met - activate fixed point focus
	return true;
}

/**
 * Compute - Use fixed world position as camera focus point
 * 
 * This is the simplest Position module - it directly uses a pre-defined
 * world position from the StateConfig without any calculations or adjustments.
 * 
 * The fixed point is set by:
 * - Level designers in camera state blueprints
 * - Cinematic sequencers
 * - Scripted gameplay events
 * - Manual configuration in state configs
 * 
 * Steps:
 * 1. Read fixed point from StateConfig
 * 2. Output it directly as the focus position
 * 3. Use Override blend policy (this is authoritative)
 * 
 * No interpolation, no offsets, no dynamic adjustments.
 * What you configure is what you get - providing precise control
 * for cinematic and scripted camera moments.
 */
bool UCameraModule_P06_FixedPoint::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Use Override blend policy (authoritative for cinematics)
	// Fixed point is authoritative - it should not blend with other modules
	// This ensures cinematic moments use EXACTLY the configured position
	OutOutput.BlendPolicy = EBlendPolicy::Override;
	
	// Read fixed point location from state config
	// This is a world-space position set by designers/sequencers
	FVector FixedPoint = StateConfig.Module.Position.FixedPointLocation;
	
	// Safety check (shouldn't be needed if ShouldActivate works correctly)
	// But handle gracefully just in case
	if (FixedPoint.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("P06 FixedPoint: Activated but FixedPointLocation is zero!"));
		OutOutput.bHasPositionOutput = false;
		return false;
	}
	
	// Output fixed point directly - no calculations, no adjustments
	// Pure fixed position for precise cinematic control
	OutOutput.PositionOffset = FixedPoint;
	OutOutput.bHasPositionOutput = true;
	
	return true;
}


//========================================
// P07: Spline Follow - Follow predefined spline path
//========================================

/**
 * ShouldActivate - Activate for cinematic states (placeholder)
 * 
 * P07 activates when:
 * - Module is enabled
 * - Current state is cinematic (splines are typically for cutscenes)
 * - (Future) A valid spline component reference exists
 * 
 * Current Implementation:
 * Since the full spline management system is not yet implemented,
 * we use bIsCinematic as the activation condition. When spline system
 * is added, this will also check for a valid SplineComponent reference.
 * 
 * Use Cases:
 * - Cinematic camera paths along predefined routes
 * - Scripted camera movements in cutscenes
 * - Rail-based camera transitions
 * - Showcase sequences following specific paths
 */
bool UCameraModule_P07_Spline_Follow::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if current state is cinematic
	// Spline follow is typically used for cutscenes and scripted sequences
	if (!StateConfig.StateBase.Flags.bIsCinematic)
	{
		return false;
	}
	
	// TODO: When spline system is implemented, add check:
	// if (!StateConfig.Module.Position.SplineComponent.IsValid())
	// {
	//     return false;
	// }
	
	// For now, activate for all cinematic states
	// This allows testing the module even without a spline system
	return true;
}

/**
 * Compute - Calculate focus point along spline (placeholder orbit)
 * 
 * CURRENT IMPLEMENTATION (Placeholder):
 * Since the full spline system is not yet integrated, this module provides
 * a placeholder implementation that simulates spline-like behavior by
 * orbiting around the character. This allows:
 * - Testing the module activation/deactivation
 * - Verifying the pipeline integration
 * - Demonstrating smooth position transitions
 * 
 * FUTURE IMPLEMENTATION (When Spline System Available):
 * 1. Get SplineComponent reference from StateConfig
 * 2. Get current spline position (0-1 or distance along spline)
 * 3. Evaluate spline at current position -> GetLocationAtSplineInputKey()
 * 4. Apply any configured offsets
 * 5. Update spline position based on speed/time
 * 6. Output the evaluated spline position
 * 
 * Placeholder Behavior:
 * - Simulates a circular path around the character
 * - Position progresses over time (10% per second)
 * - Provides smooth, predictable camera movement
 * - Demonstrates Override blend policy
 */
bool UCameraModule_P07_Spline_Follow::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Use Override blend policy (spline paths are authoritative)
	// When following a spline, we want exact path adherence
	OutOutput.BlendPolicy = EBlendPolicy::Override;
	
	// ========================================
	// PLACEHOLDER IMPLEMENTATION
	// ========================================
	// This section simulates spline behavior until the real spline system
	// is integrated. It creates a circular orbit around the character.
	
	// Get character location as orbit center
	FVector CharacterLocation = Context.InputContext.CharacterLocation;
	
	// Get standard focus offset
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	FocusOffset.Z += VerticalOffset;
	
	// Update spline position over time
	// In real implementation, this would come from spline progression logic
	// CurrentSplinePosition ranges from 0.0 to 1.0 (normalized)
	const float SplineSpeed = 0.1f;  // 10% of path per second
	CurrentSplinePosition += Context.DeltaTime * SplineSpeed;
	
	// Wrap around when reaching end of spline
	// fmod ensures position stays in 0-1 range
	CurrentSplinePosition = FMath::Fmod(CurrentSplinePosition, 1.0f);
	
	// Simulate spline position by creating a circular orbit
	// Convert normalized position (0-1) to angle (0-360 degrees)
	float Angle = CurrentSplinePosition * 360.0f;
	
	// Orbit radius (in centimeters)
	// In real implementation, this would be the spline's actual shape
	const float OrbitRadius = 200.0f;  // 2 meters from character
	
	// Calculate position on circular path
	// X = cos(angle) * radius, Y = sin(angle) * radius
	FVector SplineOffset;
	SplineOffset.X = FMath::Cos(FMath::DegreesToRadians(Angle)) * OrbitRadius;
	SplineOffset.Y = FMath::Sin(FMath::DegreesToRadians(Angle)) * OrbitRadius;
	SplineOffset.Z = 0.0f;  // Keep at same height (horizontal orbit)
	
	// Calculate final focus point
	// Base position + configured offset + simulated spline offset
	FVector FocusPoint = CharacterLocation + FocusOffset + SplineOffset;
	
	// ========================================
	// END PLACEHOLDER IMPLEMENTATION
	// ========================================
	
	// Fill output structure
	OutOutput.PositionOffset = FocusPoint;
	OutOutput.bHasPositionOutput = true;
	
	// Optional debug logging
	// Shows current spline position for debugging/verification
	UE_LOG(LogTemp, VeryVerbose, TEXT("P07 Spline: Position %.2f%% (placeholder orbit)"), 
	       CurrentSplinePosition * 100.0f);
	
	return true;
}


//========================================
// P08: MidPoint Two Target - Focus between player and target
//========================================

/**
 * ShouldActivate - Determine if midpoint focus should be active
 * 
 * P08 activates when: 
 * - Module is enabled
 * - Player has a valid target
 * 
 * Differences from P04 (Orbit Lock-On):
 * - P04: Only activates during lock-on combat (bHasTarget && !bTargetIsBoss)
 * - P08: Activates for ANY target scenario (more general purpose)
 * 
 * P08 is more flexible and can be used in:
 * - Non-combat scenarios with targets
 * - Cinematic sequences showing two characters
 * - Any situation needing focus between two points
 * - Blended with other position modules (uses Blend policy)
 * 
 * The dynamic weight system allows P08 to gracefully reduce influence
 * when targets are far apart, preventing awkward framing.
 */
bool UCameraModule_P08_MidPoint_TwoTarget::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if player has a valid target
	// This is the only condition - P08 is very general purpose
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	// Activate for any target scenario
	// Unlike P04, we don't care about lock-on state or boss flags
	return true;
}

/**
 * Compute - Calculate weighted midpoint between player and target
 * 
 * This module provides a clean, general-purpose midpoint calculation
 * between two actors. Unlike P04 (combat-oriented), P08 focuses on
 * simplicity and flexibility.
 * 
 * Steps:
 * 1. Validate target exists
 * 2. Get player and target locations
 * 3. Calculate weighted midpoint based on MidPointBias
 * 4. Apply standard focus offset
 * 5. Add dynamic height adjustment based on target distance
 * 6. Calculate dynamic weight (decreases with distance)
 * 7. Output the calculated midpoint
 * 
 * Key Features:
 * - Configurable bias (favor player or target)
 * - Dynamic height adjustment (raises focus for distant targets)
 * - Distance-based weight (reduces influence at extreme distances)
 * - Blend policy (works well with other modules)
 * 
 * Use Cases:
 * - Cinematic two-character scenes
 * - Non-combat target tracking
 * - Flexible focus between any two points
 * - Situations where both actors should be visible
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_P08_MidPoint_TwoTarget::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Use Blend policy (not Override)
	// P08 is designed to work cooperatively with other position modules
	// This allows it to contribute to the final focus without dominating
	OutOutput.BlendPolicy = EBlendPolicy::Blend;
	
	// Step 1: Validate target exists
	// Safety check - ShouldActivate should prevent this, but double-check
	if (!Context.InputContext.bHasTarget)
	{
		// No target - cannot compute midpoint
		OutOutput.bHasPositionOutput = false;
		return false;
	}
	
	// Step 2: Get player and target world locations
	FVector PlayerLocation = Context.InputContext.CharacterLocation;
	FVector TargetLocation = Context.InputContext.TargetLocation;
	
	// Step 3: Calculate weighted midpoint
	// MidPointBias controls the weighting: 
	// - 0.0 = focus on player
	// - 0.5 = exact midpoint
	// - 1.0 = focus on target
	float MidPointBias = StateConfig.Module.Position.MidPointBias;
	
	// Validate bias is in valid range, use default if not configured
	if (MidPointBias < 0.0f || MidPointBias > 1.0f)
	{
		MidPointBias = 0.5f;  // Default: exact midpoint (equal weighting)
	}
	
	// Calculate weighted midpoint using linear interpolation
	FVector MidPoint = FMath::Lerp(PlayerLocation, TargetLocation, MidPointBias);
	
	// Step 4: Apply standard focus offset
	// This provides basic height and positional adjustment
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	FocusOffset.Z += VerticalOffset;
	
	MidPoint += FocusOffset;
	
	// Step 5: Dynamic height adjustment based on target distance
	// When targets are far apart, raise the focus slightly to keep both visible
	// This helps prevent either actor from being cut off at screen edges
	
	float TargetDistance = Context.InputContext.TargetDistance;
	
	// If TargetDistance not available, calculate from positions
	if (TargetDistance <= 0.0f)
	{
		TargetDistance = (TargetLocation - PlayerLocation).Size();
	}
	
	// Height adjustment: 5% of distance, clamped to max 50cm
	// This provides subtle lift for distant targets without being excessive
	// Example: 1000cm distance -> 50cm height boost
	float HeightAdjust = FMath::Clamp(TargetDistance * 0.05f, 0.0f, 50.0f);
	MidPoint.Z += HeightAdjust;
	
	// Step 6: Fill output
	OutOutput.PositionOffset = MidPoint;
	OutOutput.bHasPositionOutput = true;
	
	// Step 7: Calculate dynamic weight based on target distance
	// Weight decreases as targets get farther apart
	// This prevents awkward framing when targets are at extreme distances
	//
	// Weight curve:
	// - Distance 0-500cm:   Weight ~1.0 (full influence)
	// - Distance 1000cm:    Weight ~0.6 (moderate influence)
	// - Distance 1500cm+:   Weight ~0.2 (minimal influence)
	//
	// This allows other position modules (like P01) to take over
	// when P08's midpoint focus becomes less useful
	
	const float MaxEffectiveDistance = 1500.0f;  // 15 meters
	float DistanceRatio = TargetDistance / MaxEffectiveDistance;
	
	// Calculate weight: starts at 1.0, decreases to 0.2 at max distance
	// Using (1.0 - ratio * 0.8) keeps minimum weight at 0.2
	float DistanceWeight = 1.0f - FMath::Clamp(DistanceRatio, 0.0f, 1.0f) * 0.8f;
	
	OutOutput.Weight = DistanceWeight;
	
	return true;
}
