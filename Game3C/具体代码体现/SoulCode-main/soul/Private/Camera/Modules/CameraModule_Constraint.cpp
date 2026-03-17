// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModule_Constraint.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModule_Constraint.cpp
 * 
 * This file contains the implementations of 4 Constraint modules (C01-C04).
 * Constraint modules are responsible for enforcing limits and ensuring
 * the final camera values stay within acceptable ranges.
 * 
 * These modules execute LAST in Stage 3 with priority 200+,
 * after all other module types have calculated their values.
 * 
 * Output is typically Override policy to enforce hard limits.
 * 
 * Key Principle: 
 * - Other modules calculate what camera SHOULD do
 * - Constraint modules ensure values ARE valid
 * 
 * Constraint Types:
 * - Pitch Clamp: Limit vertical rotation angle
 * - Distance Clamp: Limit camera arm length
 * - FOV Clamp: Limit field of view angle
 * - Visibility Ensure: Keep target in view
 * 
 * Module List:
 * - C01: Pitch Clamp - Enforces pitch limits (implemented)
 * - C02: Distance Clamp - Enforces distance limits (implemented)
 * - C03: FOV Clamp - Enforces FOV limits (implemented)
 * - C04: Visibility Ensure - Keeps target visible (implemented)
 */


//========================================
// C01: Pitch Clamp - Enforces pitch angle limits
//========================================

/**
 * ShouldActivate - Pitch clamp is always active
 * 
 * C01 is a safety constraint that should always be active.
 * It ensures the camera pitch never exceeds configured limits,
 * regardless of what other modules have calculated.
 * 
 * Priority 200 (constraint level), uses Override policy.
 */
bool UCameraModule_C01_Pitch_Clamp::ShouldActivate(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig) const
{
	// Pitch clamp is always active as a safety constraint
	// Only check if module is enabled
	return bIsEnabled;
}

/**
 * Compute - Clamp pitch rotation to configured limits
 * 
 * This module reads the current accumulated rotation from the context
 * and clamps the pitch component to the configured min/max values.
 * 
 * Steps:
 * 1. Get current rotation from context output
 * 2. Get pitch limits from state config
 * 3. Apply default limits if not configured
 * 4. Clamp pitch to valid range
 * 5. Output the clamped rotation
 * 
 * Pitch Convention (UE4):
 * - Negative pitch = Looking UP (camera tilts up)
 * - Positive pitch = Looking DOWN (camera tilts down)
 * - Zero pitch = Looking straight ahead (horizontal)
 * 
 * Typical Limits:
 * - MinPitch: -80 to -60 degrees (looking up limit)
 * - MaxPitch: +60 to +80 degrees (looking down limit)
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_C01_Pitch_Clamp::Compute(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig,
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get current rotation from context output
	// This is the accumulated rotation from all previous modules
	FRotator CurrentRotation = Context.Output.Rotation;
	
	// Step 2: Get pitch limits from state config
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	// Step 3: Apply default limits if not configured
	// MinPitch should be negative (looking up limit)
	if (MinPitch >= 0.0f || MinPitch < -89.0f)
	{
		MinPitch = -80.0f;  // Default: can look up to 80 degrees
	}
	
	// MaxPitch should be positive (looking down limit)
	if (MaxPitch <= 0.0f || MaxPitch > 89.0f)
	{
		MaxPitch = 80.0f;  // Default: can look down to 80 degrees
	}
	
	// Ensure min < max (swap if inverted)
	if (MinPitch > MaxPitch)
	{
		float Temp = MinPitch;
		MinPitch = MaxPitch;
		MaxPitch = Temp;
	}
	
	// Step 4: Clamp pitch to valid range
	float ClampedPitch = FMath::Clamp(CurrentRotation.Pitch, MinPitch, MaxPitch);
	
	// Step 5: Create output rotation
	FRotator ClampedRotation = CurrentRotation;
	ClampedRotation.Pitch = ClampedPitch;
	
	// Fill output structure
	OutOutput.RotationOutput = ClampedRotation;
	OutOutput.bHasRotationOutput = true;
	
	return true;
}


//========================================
// C02: Distance Clamp - Enforces camera arm length limits
//========================================

/**
 * ShouldActivate - Distance clamp is always active
 * 
 * C02 is a safety constraint that should always be active.
 * It ensures the camera distance never exceeds configured limits,
 * regardless of what other modules have calculated.
 * 
 * Priority 200 (constraint level), uses Override policy.
 */
bool UCameraModule_C02_Distance_Clamp::ShouldActivate(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig) const
{
	// Distance clamp is always active as a safety constraint
	// Only check if module is enabled
	return bIsEnabled;
}

/**
 * Compute - Clamp camera distance to configured limits
 * 
 * This module reads the current accumulated distance from the context
 * and clamps it to the configured min/max values.
 * 
 * Steps:
 * 1. Get current distance from context output
 * 2. Get distance limits from state config
 * 3. Apply default limits if not configured
 * 4. Validate min/max relationship
 * 5. Clamp distance to valid range
 * 6. Output the clamped distance
 * 
 * Distance Units: Centimeters (UE4 standard)
 * 
 * Typical Limits:
 * - MinDistance: 100-200cm (prevent clipping into character)
 * - MaxDistance: 800-1500cm (prevent losing sight of character)
 * 
 * Important Considerations:
 * - MinDistance should account for character collision capsule
 * - MaxDistance should consider gameplay visibility needs
 * - After collision detection, distance might be reduced further
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_C02_Distance_Clamp::Compute(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig,
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get current distance from context output
	// This is the accumulated distance from all previous modules
	float CurrentDistance = Context.Output.Distance;
	
	// Step 2: Get distance limits from state config
	float MinDistance = StateConfig.StateBase.Distance.MinDistance;
	float MaxDistance = StateConfig.StateBase.Distance.MaxDistance;
	
	// Step 3: Apply default limits if not configured
	// MinDistance should be positive and reasonable
	if (MinDistance <= 0.0f)
	{
		MinDistance = 100.0f;  // Default: 1 meter minimum
	}
	
	// MaxDistance should be positive and greater than MinDistance
	if (MaxDistance <= 0.0f)
	{
		MaxDistance = 1000.0f;  // Default: 10 meters maximum
	}
	
	// Step 4: Validate min/max relationship
	// Ensure min < max (swap if inverted)
	if (MinDistance > MaxDistance)
	{
		float Temp = MinDistance;
		MinDistance = MaxDistance;
		MaxDistance = Temp;
	}
	
	// Also ensure there's at least some valid range
	if (MaxDistance - MinDistance < 10.0f)
	{
		// If range is too small, expand it
		MaxDistance = MinDistance + 100.0f;
	}
	
	// Step 5: Clamp distance to valid range
	float ClampedDistance = FMath::Clamp(CurrentDistance, MinDistance, MaxDistance);
	
	// Step 6: Fill output structure
	OutOutput.DistanceOutput = ClampedDistance;
	OutOutput.bHasDistanceOutput = true;
	
	return true;
}


//========================================
// C03: FOV Clamp - Enforces field of view limits
//========================================

/**
 * ShouldActivate - FOV clamp is always active
 * 
 * C03 is a safety constraint that should always be active.
 * It ensures the camera FOV never exceeds configured limits,
 * regardless of what other modules have calculated.
 * 
 * Priority 200 (constraint level), uses Override policy.
 */
bool UCameraModule_C03_FOV_Clamp::ShouldActivate(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig) const
{
	// FOV clamp is always active as a safety constraint
	// Only check if module is enabled
	return bIsEnabled;
}

/**
 * Compute - Clamp camera FOV to configured limits
 * 
 * This module reads the current accumulated FOV from the context
 * and clamps it to the configured min/max values.
 * 
 * Steps:
 * 1. Get current FOV from context output
 * 2. Get FOV limits from state config
 * 3. Apply default limits if not configured
 * 4. Validate min/max relationship
 * 5. Clamp FOV to valid range
 * 6. Output the clamped FOV
 * 
 * FOV Units: Degrees
 * 
 * Typical Limits:
 * - MinFOV: 50-60 degrees (prevent tunnel vision)
 * - MaxFOV: 120-130 degrees (prevent fisheye distortion)
 * 
 * FOV Effects on Player:
 * - Too narrow (< 60 deg): Uncomfortable tunnel vision, disorienting
 * - Too wide (> 120 deg): Fisheye distortion, motion sickness
 * - Sweet spot: 80-100 deg for third-person games
 * 
 * Why Constraint is Important:
 * - Multiple Additive FOV modules can stack (Speed + Combat + Boss)
 * - Without limits, FOV could reach extreme values
 * - Example: Base(90) + Speed(+15) + Combat(+5) + Boss(+15) = 125 deg
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_C03_FOV_Clamp::Compute(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig,
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get current FOV from context output
	// This is the accumulated FOV from all previous modules
	float CurrentFOV = Context.Output.FOV;
	
	// Step 2: Get FOV limits from state config
	float MinFOV = StateConfig.StateBase.FOV.MinFOV;
	float MaxFOV = StateConfig.StateBase.FOV.MaxFOV;
	
	// Step 3: Apply default limits if not configured
	// MinFOV should be reasonable (not too narrow)
	if (MinFOV <= 0.0f || MinFOV < 30.0f)
	{
		MinFOV = 60.0f;  // Default: 60 degrees minimum
	}
	
	// MaxFOV should be reasonable (not too wide)
	if (MaxFOV <= 0.0f || MaxFOV > 150.0f)
	{
		MaxFOV = 120.0f;  // Default: 120 degrees maximum
	}
	
	// Step 4: Validate min/max relationship
	// Ensure min < max (swap if inverted)
	if (MinFOV > MaxFOV)
	{
		float Temp = MinFOV;
		MinFOV = MaxFOV;
		MaxFOV = Temp;
	}
	
	// Also ensure there's at least some valid range
	if (MaxFOV - MinFOV < 10.0f)
	{
		// If range is too small, expand it symmetrically
		float Center = (MinFOV + MaxFOV) / 2.0f;
		MinFOV = Center - 15.0f;
		MaxFOV = Center + 15.0f;
	}
	
	// Step 5: Clamp FOV to valid range
	float ClampedFOV = FMath::Clamp(CurrentFOV, MinFOV, MaxFOV);
	
	// Step 6: Fill output structure
	OutOutput.FOVOutput = ClampedFOV;
	OutOutput.bHasFOVOutput = true;
	
	return true;
}


//========================================
// C04: Visibility Ensure - Ensures locked target remains visible
//========================================

/**
 * Constructor - Initialize visibility tracking state
 * 
 * Start with target assumed visible (TimeSinceTargetVisible = 0)
 * LastVisibleTargetPosition starts at zero, will be updated on first visible frame
 */
UCameraModule_C04_Visibility_Ensure::UCameraModule_C04_Visibility_Ensure()
	: TimeSinceTargetVisible(0.0f)
	, LastVisibleTargetPosition(FVector::ZeroVector)
{
}

/**
 * ShouldActivate - Activate when there is a locked target
 * 
 * C04 activates when:
 * - Module is enabled
 * - There is a locked target (bHasTarget)
 * 
 * Unlike C01-C03, this module only activates when there's
 * actually a target to keep visible. No target = no visibility to ensure.
 * 
 * Priority 210 (highest constraint), uses Blend policy.
 * 
 * Use Cases:
 * - Lock-on combat (keep enemy in view)
 * - Boss fights (prevent boss from going off-screen)
 * - Any situation with a tracked target
 */
bool UCameraModule_C04_Visibility_Ensure::ShouldActivate(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Only activate if there is a locked target
	if (!Context.InputContext.bHasTarget)
	{
		return false;
	}
	
	// All conditions met - activate visibility ensure
	return true;
}

/**
 * Compute - Ensure locked target remains visible on screen
 * 
 * This module checks if the current target is within the visible
 * screen area. If not, it calculates a rotation adjustment to
 * bring the target back into view.
 * 
 * Steps:
 * 1. Validate target exists
 * 2. Calculate target point (with height offset for center mass)
 * 3. Check if target is currently visible
 * 4. If visible: reset timer, no adjustment needed
 * 5. If not visible: track time and calculate adjustment
 * 6. Apply adjustment with weight based on invisibility duration
 * 
 * Key Design Decisions:
 * - Uses Blend policy for smooth adjustment (not jarring)
 * - Has a grace period (0.1s) before adjusting
 * - Weight increases over time (gradual correction)
 * - Maximum weight capped at 0.8 (allows other systems to contribute)
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_C04_Visibility_Ensure::Compute(
	const FStageExecutionContext& Context,
	const FCameraStateConfig& StateConfig,
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Validate target exists
	if (!Context.InputContext.bHasTarget)
	{
		// No target - reset state and return
		TimeSinceTargetVisible = 0.0f;
		OutOutput.Weight = 0.0f;
		return false;
	}
	
	// Step 2: Calculate target point with height offset
	// Look at target's center mass, not feet
	FVector TargetLocation = Context.InputContext.TargetLocation;
	float TargetSize = Context.InputContext.TargetSize;
	
	// Add height offset to look at center mass (30% of target height)
	float TargetHeightOffset = TargetSize * 0.3f;
	TargetLocation.Z += TargetHeightOffset;
	
	// Step 3: Check if target is currently visible
	// Use 15% screen margin (target must be within 15%-85% of screen)
	const float ScreenMargin = 0.15f;
	bool bTargetVisible = IsPointVisible(Context, TargetLocation, ScreenMargin);
	
	// Step 4: If target is visible, update state and return
	if (bTargetVisible)
	{
		// Reset invisibility timer
		TimeSinceTargetVisible = 0.0f;
		
		// Update last known visible position
		LastVisibleTargetPosition = TargetLocation;
		
		// No adjustment needed - output with zero weight
		OutOutput.RotationOutput = Context.Output.Rotation;
		OutOutput.bHasRotationOutput = true;
		OutOutput.Weight = 0.0f;
		
		return true;
	}
	
	// Step 5: Target not visible - track time and calculate adjustment
	TimeSinceTargetVisible += Context.DeltaTime;
	
	// Grace period: don't adjust too quickly
	// Let other systems (like lock-on rotation) try first
	const float GracePeriod = 0.1f;
	if (TimeSinceTargetVisible < GracePeriod)
	{
		OutOutput.RotationOutput = Context.Output.Rotation;
		OutOutput.bHasRotationOutput = true;
		OutOutput.Weight = 0.0f;
		return true;
	}
	
	// Calculate rotation adjustment to bring target into view
	FRotator Adjustment = CalculateVisibilityAdjustment(Context, TargetLocation);
	
	// Apply adjustment to current rotation
	FRotator AdjustedRotation = Context.Output.Rotation + Adjustment;
	
	// Clamp pitch after adjustment (respect pitch limits)
	float MinPitch = StateConfig.StateBase.Rotation.MinPitch;
	float MaxPitch = StateConfig.StateBase.Rotation.MaxPitch;
	
	// Apply defaults if not configured
	if (MinPitch >= 0.0f) MinPitch = -80.0f;
	if (MaxPitch <= 0.0f) MaxPitch = 80.0f;
	
	AdjustedRotation.Pitch = FMath::Clamp(AdjustedRotation.Pitch, MinPitch, MaxPitch);
	AdjustedRotation.Roll = 0.0f;  // Never allow roll
	
	// Step 6: Output with weight based on invisibility duration
	OutOutput.RotationOutput = AdjustedRotation;
	OutOutput.bHasRotationOutput = true;
	
	// Weight increases the longer target has been invisible
	// Starts at 0, reaches max (0.8) after MaxInvisibleTime
	const float MaxInvisibleTime = 0.5f;
	const float MaxWeight = 0.8f;  // Cap at 0.8 to allow other systems to contribute
	
	float Weight = (TimeSinceTargetVisible - GracePeriod) / MaxInvisibleTime;
	Weight = FMath::Clamp(Weight, 0.0f, MaxWeight);
	
	OutOutput.Weight = Weight;
	
	return true;
}

/**
 * IsPointVisible - Check if a world point is visible on screen
 * 
 * Projects a world point to screen coordinates and checks if it's
 * within the visible area (accounting for margin).
 * 
 * @param Context Current execution context
 * @param WorldPoint Point to check in world space
 * @param ScreenMargin Margin from screen edge (0.0 to 0.5, default 0.15 = 15%)
 * @return True if point is within screen bounds (accounting for margin)
 */
bool UCameraModule_C04_Visibility_Ensure::IsPointVisible(
	const FStageExecutionContext& Context,
	const FVector& WorldPoint,
	float ScreenMargin) const
{
	// Get camera manager for projection
	if (!Context.Manager)
	{
		return true;  // Assume visible if we can't check
	}
	
	// Get player controller for screen projection
	AActor* Owner = Context.Manager->GetOwner();
	if (!Owner)
	{
		return true;
	}
	
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn)
	{
		return true;
	}
	
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return true;
	}
	
	// Project world point to screen coordinates
	FVector2D ScreenPosition;
	bool bProjected = PC->ProjectWorldLocationToScreen(WorldPoint, ScreenPosition, false);
	
	if (!bProjected)
	{
		// Point is behind camera - definitely not visible
		return false;
	}
	
	// Get viewport size
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return true;  // Can't determine, assume visible
	}
	
	// Normalize screen position to 0-1 range
	float NormalizedX = ScreenPosition.X / static_cast<float>(ViewportSizeX);
	float NormalizedY = ScreenPosition.Y / static_cast<float>(ViewportSizeY);
	
	// Check if within screen bounds (accounting for margin)
	// With 0.15 margin: valid range is 0.15 to 0.85
	bool bWithinX = (NormalizedX >= ScreenMargin) && (NormalizedX <= (1.0f - ScreenMargin));
	bool bWithinY = (NormalizedY >= ScreenMargin) && (NormalizedY <= (1.0f - ScreenMargin));
	
	return bWithinX && bWithinY;
}

/**
 * CalculateVisibilityAdjustment - Calculate rotation to bring target into view
 * 
 * Calculates the rotation adjustment needed to rotate the camera
 * towards the target. Uses partial correction (30%) to avoid
 * jarring camera movements.
 * 
 * @param Context Current execution context
 * @param TargetPoint Target location in world space
 * @return Rotation offset to apply to bring target into view
 */
FRotator UCameraModule_C04_Visibility_Ensure::CalculateVisibilityAdjustment(
	const FStageExecutionContext& Context,
	const FVector& TargetPoint) const
{
	FRotator Adjustment = FRotator::ZeroRotator;
	
	// Get camera location (focus point)
	FVector CameraLocation = Context.Output.FocusPoint;
	
	// Calculate direction from camera to target
	FVector ToTarget = TargetPoint - CameraLocation;
	
	// Skip if target is at same location (avoid division by zero)
	if (ToTarget.IsNearlyZero())
	{
		return Adjustment;
	}
	
	// Get current camera rotation
	FRotator CurrentRotation = Context.Output.Rotation;
	
	// Calculate desired rotation to look at target
	FRotator DesiredRotation = ToTarget.Rotation();
	
	// Calculate difference between current and desired
	FRotator DeltaRotation = DesiredRotation - CurrentRotation;
	DeltaRotation.Normalize();  // Normalize to -180 to +180 range
	
	// Apply partial correction (gradual, not instant)
	// 30% correction per frame creates smooth tracking
	const float CorrectionStrength = 0.3f;
	
	Adjustment.Yaw = DeltaRotation.Yaw * CorrectionStrength;
	Adjustment.Pitch = DeltaRotation.Pitch * CorrectionStrength;
	Adjustment.Roll = 0.0f;  // Never adjust roll
	
	// Clamp adjustment magnitude to prevent wild camera swings
	const float MaxYawAdjustment = 15.0f;    // Max 15 degrees yaw per frame
	const float MaxPitchAdjustment = 7.5f;   // Max 7.5 degrees pitch per frame
	
	Adjustment.Yaw = FMath::Clamp(Adjustment.Yaw, -MaxYawAdjustment, MaxYawAdjustment);
	Adjustment.Pitch = FMath::Clamp(Adjustment.Pitch, -MaxPitchAdjustment, MaxPitchAdjustment);
	
	return Adjustment;
}
