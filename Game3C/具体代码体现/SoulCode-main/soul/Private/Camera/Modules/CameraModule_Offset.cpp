// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModule_Offset.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModule_Offset.cpp
 * 
 * This file contains the implementations of 5 Offset modules (O01-O05).
 * Offset modules are responsible for calculating camera position offsets (FVector),
 * which are applied relative to the camera's target/focus point.
 * 
 * Output is passed through FModuleOutput.SocketOffsetOutput field.
 * 
 * Offset Blend Policies:
 * - Override: Replace with this value (O01, O02, O03)
 * - Additive: Add to current offset (O04, O05)
 * 
 * Coordinate System (Local to Camera):
 * - X: Forward (towards character, negative = closer)
 * - Y: Right (positive = right shoulder)
 * - Z: Up (positive = higher camera)
 * 
 * Typical Offset Values:
 * - Centered: (0, 0, 0)
 * - Right shoulder: (0, 50, 0)
 * - Over-the-shoulder: (0, 50, 20)
 * - Crouch adjustment: (0, 0, -50)
 * - Mount adjustment: (0, 0, 80)
 * 
 * Module List:
 * - O01: SocketOffset Base - Foundation offset from state config (implemented)
 * - O02: Shoulder Offset - Over-the-shoulder offset (implemented)
 * - O03: Shoulder Switch - Dynamic side switching (implemented)
 * - O04: Crouch Offset - Vertical offset for crouching (implemented)
 * - O05: Mount Offset - Offset for mounted characters (implemented)
 */


//========================================
// O01: SocketOffset Base - Foundation offset from state config
//========================================

/**
 * ShouldActivate - Base socket offset is always active
 * 
 * O01 is the foundation Offset module. It should always be active
 * as long as the module is enabled, providing the starting point for
 * all offset calculations.
 * 
 * Other modules (O02-O05) build upon this base value using their
 * respective blend policies (Override or Additive).
 */
bool UCameraModule_O01_SocketOffset_Base::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Base socket offset is always active as the foundation
	// Only check if module is enabled
	return bIsEnabled;
}

/**
 * Compute - Return base socket offset from state configuration
 * 
 * This module reads the configured base socket offset from StateConfig
 * and outputs it directly. It serves as the starting point for
 * offset calculations.
 * 
 * Steps:
 * 1. Get base socket offset from state config
 * 2. Validate and apply defaults if needed
 * 3. Output the offset
 * 
 * The output uses Override policy, meaning this value will be
 * the starting point that other modules modify.
 * 
 * Typical base socket offsets:
 * - Exploration state: (0, 0, 0) centered
 * - Combat state: (0, 50, 20) over-shoulder
 * - Cinematic state: varies per shot
 * 
 * Coordinate System:
 * - X: Forward/Back (negative = towards character)
 * - Y: Left/Right (positive = right)
 * - Z: Up/Down (positive = up)
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_O01_SocketOffset_Base::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get base socket offset from state configuration
	// This is the foundation offset that other modules will modify
	FVector SocketOffset = StateConfig.StateBase.Offset.SocketOffset;
	
	// Step 2: Validate offset values
	// Unlike distance/FOV, offset can be zero or negative, so no default needed
	// However, we can clamp to reasonable limits to prevent extreme values
	const float MaxOffsetMagnitude = 500.0f;  // 5 meters max in any direction
	
	SocketOffset.X = FMath::Clamp(SocketOffset.X, -MaxOffsetMagnitude, MaxOffsetMagnitude);
	SocketOffset.Y = FMath::Clamp(SocketOffset.Y, -MaxOffsetMagnitude, MaxOffsetMagnitude);
	SocketOffset.Z = FMath::Clamp(SocketOffset.Z, -MaxOffsetMagnitude, MaxOffsetMagnitude);
	
	// Step 3: Fill output structure
	OutOutput.SocketOffsetOutput = SocketOffset;
	OutOutput.bHasSocketOffsetOutput = true;
	
	return true;
}

//========================================
// O02: Shoulder Offset
//========================================

/**
 * ShouldActivate - Determine if shoulder offset should be active
 * 
 * O02 activates when: 
 * - Module is enabled
 * - Shoulder offset is configured (non-zero value)
 * 
 * This module provides the classic "over-the-shoulder" camera view
 * common in third-person shooters and Souls-like games.
 * 
 * Priority 105 (after base socket offset), uses Override policy.
 * 
 * Use Cases:
 * - Combat stance (offset to see aiming direction)
 * - Lock-on combat (offset away from target for visibility)
 * - General exploration (slight offset for aesthetic)
 */
bool UCameraModule_O02_Shoulder_Offset::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Get shoulder offset from state config
	// Located at StateBase.Offset.ShoulderOffset (horizontal shoulder offset)
	float ShoulderOffset = StateConfig.StateBase.Offset.ShoulderOffset;
	
	// Activate if shoulder offset is configured (non-zero)
	// Use KINDA_SMALL_NUMBER for float comparison
	if (FMath::Abs(ShoulderOffset) <= KINDA_SMALL_NUMBER)
	{
		return false;  // No shoulder offset configured
	}
	
	// All conditions met - activate shoulder offset
	return true;
}

/**
 * Compute - Calculate shoulder offset for over-the-shoulder view
 * 
 * This module applies a horizontal offset to create the classic
 * over-the-shoulder camera perspective. The offset moves the camera
 * to the side of the character.
 * 
 * Steps:
 * 1. Get shoulder offset from config (horizontal Y-axis)
 * 2. Get optional vertical offset from config (Z-axis adjustment)
 * 3. Clamp values to reasonable range
 * 4. Output with Override policy
 * 
 * Coordinate System:
 * - Y positive: Camera moves right (right shoulder view)
 * - Y negative: Camera moves left (left shoulder view)
 * - Z positive: Camera moves up (higher viewpoint)
 * 
 * Typical Values:
 * - Subtle: 30cm
 * - Standard: 50cm (default)
 * - Aggressive: 70cm
 * 
 * Note: This module sets a fixed shoulder side. For dynamic
 * switching, use O03 (Shoulder Switch) instead.
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_O02_Shoulder_Offset::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Get shoulder offset from state config
	// ShoulderOffset is horizontal (Y-axis) offset for over-the-shoulder view
	float ShoulderOffset = StateConfig.StateBase.Offset.ShoulderOffset;
	
	// Step 2: Get optional vertical adjustment from state config
	// VerticalOffset can be used for height adjustment with shoulder view
	float ShoulderHeightOffset = StateConfig.StateBase.Offset.VerticalOffset;
	
	// If vertical offset is effectively zero, keep it at zero
	if (FMath::IsNearlyZero(ShoulderHeightOffset))
	{
		ShoulderHeightOffset = 0.0f;
	}
	
	// Step 3: Clamp to reasonable range to prevent extreme values
	// Shoulder offset: max 100cm (1 meter) horizontal in either direction
	// Height offset: max 50cm vertical in either direction
	const float MaxShoulderOffset = 100.0f;
	const float MaxHeightOffset = 50.0f;
	
	ShoulderOffset = FMath::Clamp(ShoulderOffset, -MaxShoulderOffset, MaxShoulderOffset);
	ShoulderHeightOffset = FMath::Clamp(ShoulderHeightOffset, -MaxHeightOffset, MaxHeightOffset);
	
	// Step 4: Create offset vector
	// X = 0.0 (no forward/back change from this module)
	// Y = ShoulderOffset (horizontal, positive = right shoulder)
	// Z = ShoulderHeightOffset (vertical, positive = up)
	FVector OffsetVector(0.0f, ShoulderOffset, ShoulderHeightOffset);
	
	// Step 5: Fill output structure
	OutOutput.SocketOffsetOutput = OffsetVector;
	OutOutput.bHasSocketOffsetOutput = true;
	
	return true;
}


//========================================
// O03: Shoulder Switch
//========================================

/**
 * Constructor - Initialize shoulder state
 * 
 * Default to right shoulder (positive blend)
 * Blend value: -1.0 = full left, +1.0 = full right
 */
UCameraModule_O03_Shoulder_Switch::UCameraModule_O03_Shoulder_Switch()
	: bCurrentRightShoulder(true)
	, bTargetRightShoulder(true)
	, CurrentShoulderBlend(1.0f)  // Start at right shoulder
{
}

/**
 * ShouldActivate - Determine if shoulder switching should be active
 * 
 * O03 activates when:  
 * - Module is enabled
 * - Shoulder offset is configured (has a value to switch)
 * 
 * This module provides dynamic shoulder switching that overrides O02.
 * It allows the camera to automatically or manually switch between
 * left and right shoulder views. 
 * 
 * Priority 110 (higher than O02), uses Override policy.  
 * 
 * Use Cases:
 * - Auto-switch based on target position (target on left = right shoulder)
 * - Manual switch via player input (toggle button)
 * - Cover system (switch to see around corners)
 */
bool UCameraModule_O03_Shoulder_Switch::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Get shoulder offset from config (need a base value to switch)
	float ShoulderOffset = StateConfig.StateBase.Offset.ShoulderOffset;
	
	// Activate if shoulder offset is configured (non-zero)
	if (FMath::Abs(ShoulderOffset) <= KINDA_SMALL_NUMBER)
	{
		return false;  // No shoulder offset to switch
	}
	
	// All conditions met - activate shoulder switch
	return true;
}

/**
 * Compute - Calculate dynamic shoulder offset with auto-switch and blending
 * 
 * This module dynamically switches the camera between left and right
 * shoulder views. It can auto-switch based on target position or
 * respond to manual switch commands.
 * 
 * Steps:
 * 1. Check for auto-switch based on target position
 * 2. Calculate target blend value
 * 3. Smoothly interpolate current blend towards target
 * 4. Update current shoulder state
 * 5. Calculate final offset based on blend
 * 6. Output with Override policy
 * 
 * Auto-Switch Logic:
 * - Target on player's left side -> Use right shoulder (better view)
 * - Target on player's right side -> Use left shoulder (better view)
 * - Target directly ahead -> Maintain current shoulder
 * 
 * Blend System:
 * - CurrentShoulderBlend: -1.0 (left) to +1.0 (right)
 * - Smooth interpolation prevents jarring camera jumps
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_O03_Shoulder_Switch::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Check for auto-switch based on target position
	bool bAutoSwitch = StateConfig.Module.Offset.bAutoShoulderSwitch;
	
	if (bAutoSwitch && Context.InputContext.bHasTarget)
	{
		// Get player and target positions
		FVector PlayerLocation = Context.InputContext.CharacterLocation;
		FVector TargetLocation = Context.InputContext.TargetLocation;
		
		// Get player's right direction
		FRotator CharacterRotation = Context.InputContext.CharacterRotation;
		FVector PlayerRight = FRotationMatrix(CharacterRotation).GetUnitAxis(EAxis::Y);
		
		// Calculate direction to target
		FVector ToTarget = TargetLocation - PlayerLocation;
		ToTarget.Z = 0.0f;  // Ignore vertical difference
		ToTarget.Normalize();
		
		// Determine which side the target is on
		// Positive dot = target on right, Negative = target on left
		float RightDot = FVector::DotProduct(ToTarget, PlayerRight);
		
		// Only switch if target is significantly to one side
		// Threshold of 0.3 prevents constant switching when target is ahead
		const float SwitchThreshold = 0.3f;
		
		if (RightDot > SwitchThreshold)
		{
			// Target on right side -> Use left shoulder for better view
			bTargetRightShoulder = false;
		}
		else if (RightDot < -SwitchThreshold)
		{
			// Target on left side -> Use right shoulder for better view
			bTargetRightShoulder = true;
		}
		// If within threshold, maintain current shoulder (no switch)
	}
	
	// Step 2: Calculate target blend value
	// Right shoulder = +1.0, Left shoulder = -1.0
	float TargetBlend = bTargetRightShoulder ? 1.0f : -1.0f;
	
	// Step 3: Smoothly interpolate current blend towards target
	float InterpSpeed = StateConfig.Module.Offset.ShoulderInterpSpeed;
	
	// Default interp speed if not configured
	if (InterpSpeed <= 0.0f)
	{
		InterpSpeed = 5.0f;  // Default: 5.0 (about 0.5 seconds to switch)
	}
	
	CurrentShoulderBlend = FMath::FInterpTo(
		CurrentShoulderBlend, 
		TargetBlend, 
		Context.DeltaTime, 
		InterpSpeed
	);
	
	// Step 4: Update current shoulder state
	// Used for IsRightShoulder() query
	bCurrentRightShoulder = (CurrentShoulderBlend > 0.0f);
	
	// Step 5: Calculate final offset based on blend
	float BaseShoulderOffset = StateConfig.StateBase.Offset.ShoulderOffset;
	
	// Default if not configured
	if (FMath::Abs(BaseShoulderOffset) <= KINDA_SMALL_NUMBER)
	{
		BaseShoulderOffset = 50.0f;  // Default: 50cm
	}
	
	// Apply blend to base offset
	// Blend of +1.0 -> full positive offset (right shoulder)
	// Blend of -1.0 -> full negative offset (left shoulder)
	// Blend of 0.0  -> centered (mid-switch)
	float FinalOffset = BaseShoulderOffset * CurrentShoulderBlend;
	
	// Clamp to reasonable range
	const float MaxOffset = 100.0f;
	FinalOffset = FMath::Clamp(FinalOffset, -MaxOffset, MaxOffset);
	
	// Create offset vector (Y-axis horizontal)
	FVector OffsetVector(0.0f, FinalOffset, 0.0f);
	
	// Step 6: Fill output structure
	OutOutput.SocketOffsetOutput = OffsetVector;
	OutOutput.bHasSocketOffsetOutput = true;
	
	return true;
}

/**
 * SwitchShoulder - Toggle shoulder side (left <-> right)
 * 
 * Call this when the player presses the shoulder switch button.
 * The actual switch is smoothly interpolated in Compute().
 * 
 * Example binding:
 * - Controller: Click right stick
 * - Keyboard: Press 'V' or middle mouse
 */
void UCameraModule_O03_Shoulder_Switch::SwitchShoulder()
{
	// Toggle target shoulder
	bTargetRightShoulder = !bTargetRightShoulder;
}

/**
 * SetShoulderSide - Set shoulder side directly
 * 
 * Use this for programmatic control of shoulder side.
 * The actual switch is smoothly interpolated in Compute().
 * 
 * @param bRightShoulder True for right shoulder, false for left
 * 
 * Example uses:
 * - Reset to default side at start of combat
 * - Force specific side for cinematic moments
 * - Cover system dictating camera side
 */
void UCameraModule_O03_Shoulder_Switch::SetShoulderSide(bool bRightShoulder)
{
	bTargetRightShoulder = bRightShoulder;
}


//========================================
// O04: Crouch Offset
//========================================

/**
 * Constructor - Initialize crouch blend state
 * 
 * Start with blend of 0 (standing position)
 */
UCameraModule_O04_Crouch_Offset::UCameraModule_O04_Crouch_Offset()
	: CurrentCrouchBlend(0.0f)
{
}

/**
 * ShouldActivate - Determine if crouch offset should be active
 * 
 * O04 activates when:
 * - Module is enabled
 * - Character is crouching (bIsCrouching)
 * - OR currently transitioning out of crouch (blend > threshold)
 * 
 * The second condition ensures smooth camera return when standing up.
 * Without it, camera would snap back instantly.
 * 
 * Priority 108 (after shoulder offsets), uses Additive policy.
 * 
 * Use Cases:
 * - Stealth sections (crouching through areas)
 * - Low passages (crouch to fit)
 * - Cover systems (crouch behind obstacles)
 */
bool UCameraModule_O04_Crouch_Offset::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if character is currently crouching
	bool bIsCrouching = Context.InputContext.bIsCrouching;
	
	// Also activate if we're transitioning out of crouch
	// This ensures smooth camera return when standing up
	const float TransitionThreshold = 0.01f;
	bool bIsTransitioning = CurrentCrouchBlend > TransitionThreshold;
	
	// Activate if crouching or transitioning
	if (!bIsCrouching && !bIsTransitioning)
	{
		return false;
	}
	
	// All conditions met - activate crouch offset
	return true;
}

/**
 * Compute - Calculate vertical offset for crouching
 * 
 * This module lowers the camera when the character crouches.
 * The offset is applied smoothly to prevent jarring transitions.
 * 
 * Steps:
 * 1. Determine target blend based on crouch state
 * 2. Smoothly interpolate current blend towards target
 * 3. Get crouch offset from config
 * 4. Calculate vertical offset based on blend
 * 5. Output with Additive policy
 * 
 * Blend System:
 * - CurrentCrouchBlend: 0.0 (standing) to 1.0 (fully crouched)
 * - Smooth interpolation for natural camera movement
 * 
 * Typical Crouch Offset:
 * - Light crouch: -30cm to -40cm
 * - Full crouch: -50cm to -60cm
 * - Prone (if supported): -80cm to -100cm
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_O04_Crouch_Offset::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Determine target blend based on crouch state
	// 1.0 when crouching, 0.0 when standing
	float TargetBlend = Context.InputContext.bIsCrouching ? 1.0f : 0.0f;
	
	// Step 2: Smoothly interpolate current blend towards target
	// Fast transition for responsive feel
	float InterpSpeed = 8.0f;  // About 0.3 seconds to fully transition
	
	// Try to get interp speed from config (use shoulder interp speed as fallback)
	float ConfigInterpSpeed = StateConfig.Module.Offset.ShoulderInterpSpeed;
	if (ConfigInterpSpeed > 0.0f)
	{
		InterpSpeed = ConfigInterpSpeed;
	}
	
	CurrentCrouchBlend = FMath::FInterpTo(
		CurrentCrouchBlend, 
		TargetBlend, 
		Context.DeltaTime, 
		InterpSpeed
	);
	
	// Step 3: Get crouch vertical offset from config
	// Should be negative to lower camera
	float CrouchVerticalOffset = StateConfig.Module.Offset.CrouchVerticalOffset;
	
	// Default if not configured (zero or very small value)
	if (FMath::IsNearlyZero(CrouchVerticalOffset))
	{
		CrouchVerticalOffset = -50.0f;  // Default: 50cm lower
	}
	
	// Step 4: Calculate vertical offset based on blend
	float VerticalOffset = CrouchVerticalOffset * CurrentCrouchBlend;
	
	// Clamp to reasonable range
	const float MaxCrouchOffset = 100.0f;  // Max 1 meter offset
	VerticalOffset = FMath::Clamp(VerticalOffset, -MaxCrouchOffset, MaxCrouchOffset);
	
	// Create offset vector (Z-axis vertical)
	// X = 0 (no forward/back change)
	// Y = 0 (no horizontal change)
	// Z = VerticalOffset (negative = lower camera)
	FVector OffsetVector(0.0f, 0.0f, VerticalOffset);
	
	// Step 5: Fill output structure
	OutOutput.SocketOffsetOutput = OffsetVector;
	OutOutput.bHasSocketOffsetOutput = true;
	
	return true;
}

//========================================
// O05: Mount Offset
//========================================

/**
 * Constructor - Initialize mount blend state
 * 
 * Start with blend of 0 (dismounted position)
 */
UCameraModule_O05_Mount_Offset::UCameraModule_O05_Mount_Offset()
	: CurrentMountBlend(0.0f)
{
}

/**
 * ShouldActivate - Determine if mount offset should be active
 * 
 * O05 activates when:
 * - Module is enabled
 * - Character is mounted (bIsMounted in InputContext)
 * - OR currently transitioning from mount (blend > threshold)
 * 
 * The second condition ensures smooth camera return when dismounting.
 * 
 * Priority 112 (highest among Offset modules), uses Additive policy.
 * 
 * Use Cases:
 * - Horseback riding (Elden Ring style)
 * - Vehicle mounting
 * - Flying mounts
 * - Any elevated mount position
 */
bool UCameraModule_O05_Mount_Offset::ShouldActivate(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig) const
{
	// Check if module is enabled
	if (!bIsEnabled)
	{
		return false;
	}
	
	// Check if character is currently mounted
	bool bIsMounted = Context.InputContext.bIsMounted;
	
	// Also activate if we're transitioning out of mount
	// This ensures smooth camera return when dismounting
	const float TransitionThreshold = 0.01f;
	bool bIsTransitioning = CurrentMountBlend > TransitionThreshold;
	
	// Activate if mounted or transitioning
	if (!bIsMounted && !bIsTransitioning)
	{
		return false;
	}
	
	// All conditions met - activate mount offset
	return true;
}

/**
 * Compute - Calculate vertical offset for mounted characters
 * 
 * This module raises the camera when the character is mounted.
 * The offset accounts for the increased height of being on a mount
 * (horse, vehicle, etc.) and provides appropriate combat visibility.
 * 
 * Steps:
 * 1. Determine target blend based on mount state
 * 2. Smoothly interpolate current blend towards target
 * 3. Get mount offset from config
 * 4. Optionally adjust for mount combat
 * 5. Calculate vertical offset based on blend
 * 6. Output with Additive policy
 * 
 * Blend System:
 * - CurrentMountBlend: 0.0 (dismounted) to 1.0 (fully mounted)
 * - Slower transition than crouch for cinematic mount/dismount
 * 
 * Typical Mount Offset:
 * - Small mount (pony): +50cm to +60cm
 * - Standard horse: +70cm to +90cm
 * - Large mount (elephant): +100cm to +150cm
 * 
 * @param Context The current pipeline execution context
 * @param StateConfig The current camera state configuration
 * @param OutOutput Output structure to fill with computed values
 * @return True if computation succeeded
 */
bool UCameraModule_O05_Mount_Offset::Compute(
	const FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig, 
	FModuleOutput& OutOutput)
{
	// Create base output with module identity
	OutOutput = CreateBaseOutput();
	
	// Step 1: Determine target blend based on mount state
	// 1.0 when mounted, 0.0 when dismounted
	float TargetBlend = Context.InputContext.bIsMounted ? 1.0f : 0.0f;
	
	// Step 2: Smoothly interpolate current blend towards target
	// Slower transition than crouch for more cinematic mount/dismount
	float InterpSpeed = 5.0f;  // About 0.5 seconds to fully transition
	
	// Try to get interp speed from config (use shoulder interp speed as fallback)
	float ConfigInterpSpeed = StateConfig.Module.Offset.ShoulderInterpSpeed;
	if (ConfigInterpSpeed > 0.0f)
	{
		// Use a slower speed for mount transitions (multiply by 0.6)
		InterpSpeed = ConfigInterpSpeed * 0.6f;
	}
	
	CurrentMountBlend = FMath::FInterpTo(
		CurrentMountBlend, 
		TargetBlend, 
		Context.DeltaTime, 
		InterpSpeed
	);
	
	// Step 3: Get mount vertical offset from config
	// Should be positive to raise camera
	float MountVerticalOffset = StateConfig.Module.Offset.MountVerticalOffset;
	
	// Default if not configured
	if (FMath::IsNearlyZero(MountVerticalOffset))
	{
		MountVerticalOffset = 80.0f;  // Default: 80cm higher (standard horse height)
	}
	
	// Step 4: Optionally add combat adjustment when mounted and in combat
	float CombatAdjustment = 0.0f;
	
	if (Context.InputContext.bIsInCombat && Context.InputContext.bIsMounted)
	{
		// Slightly higher camera in mounted combat for better visibility
		// Default: +20cm in combat
		CombatAdjustment = 20.0f;
	}
	
	// Step 5: Calculate vertical offset based on blend
	float VerticalOffset = (MountVerticalOffset + CombatAdjustment) * CurrentMountBlend;
	
	// Clamp to reasonable range
	const float MaxMountOffset = 200.0f;  // Max 2 meters offset
	VerticalOffset = FMath::Clamp(VerticalOffset, 0.0f, MaxMountOffset);
	
	// Create offset vector (Z-axis vertical)
	// X = 0 (no forward/back change)
	// Y = 0 (no horizontal change)
	// Z = VerticalOffset (positive = raise camera)
	FVector OffsetVector(0.0f, 0.0f, VerticalOffset);
	
	// Step 6: Fill output structure
	OutOutput.SocketOffsetOutput = OffsetVector;
	OutOutput.bHasSocketOffsetOutput = true;
	
	return true;
}
