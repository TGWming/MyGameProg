// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modifiers/CameraModifier_Cinematic.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "GameFramework/Character.h"

/**
 * CameraModifier_Cinematic.cpp
 * 
 * This file contains the implementations of the Cinematic modifier base class
 * and all 5 Cinematic modifiers (C01-C05).
 * 
 * Cinematic modifiers provide camera override functionality for dramatic moments.
 * Unlike Shake/Reaction modifiers that add offsets, Cinematic modifiers take
 * full control of the camera transform.
 * 
 * Implementation Structure:
 * - CinematicBase: Common override functionality (this Sub-Step)
 * - C01: Execution camera (Sub-Step 4.4.3)
 * - C02: Backstab camera (Sub-Step 4.4.4)
 * - C03: Riposte camera (Sub-Step 4.4.5)
 * - C04: BossIntro camera (Sub-Step 4.4.6)
 * - C05: BossPhase camera (Sub-Step 4.4.7)
 * 
 * Override Mode:
 * When IsOverrideModifier() returns true, Stage 4 uses the OverrideTransform
 * directly instead of blending with other modifiers. This allows cinematics
 * to have complete control over the camera.
 */


//========================================
// CinematicBase - Common Cinematic Functionality
//========================================

/**
 * Constructor - Initialize default cinematic parameters
 * 
 * Default values provide a reasonable starting point for most cinematics.
 * Derived classes will configure specific parameters.
 */
UCameraModifier_CinematicBase::UCameraModifier_CinematicBase()
	: CameraOffset(FVector(-150.0f, 50.0f, 30.0f))
	, HeightOffset(50.0f)
	, CameraDistance(200.0f)
	, CinematicFOV(70.0f)
	, bApplyLetterbox(true)
	, LetterboxAmount(0.12f)
	, TransformInterpSpeed(8.0f)
	, bBlockPlayerInput(true)
	, StoredPlayerLocation(FVector::ZeroVector)
	, StoredTargetLocation(FVector::ZeroVector)
	, StoredTargetActor(nullptr)
	, CurrentCameraTransform(FTransform::Identity)
	, bTransformInitialized(false)
	, ActionDirection(FVector::ForwardVector)
{
}

/**
 * OnTriggered - Initialize cinematic state when triggered
 * 
 * Stores initial positions for reference during the cinematic.
 * These stored positions are used even if actors move during the cinematic
 * to maintain consistent framing.
 */
void UCameraModifier_CinematicBase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Reset transform state
	bTransformInitialized = false;
	CurrentCameraTransform = FTransform::Identity;
	
	// Store target actor reference
	StoredTargetActor = InTriggerData.TargetActor;
	
	// Store player location (from trigger data or will be updated in ComputeEffect)
	if (!InTriggerData.SourceLocation.IsNearlyZero())
	{
		StoredPlayerLocation = InTriggerData.SourceLocation;
	}
	
	// Store target location
	if (StoredTargetActor.IsValid())
	{
		StoredTargetLocation = StoredTargetActor->GetActorLocation();
	}
	else if (!InTriggerData.SourceDirection.IsNearlyZero())
	{
		// Use source direction to estimate target location
		StoredTargetLocation = StoredPlayerLocation + InTriggerData.SourceDirection * 200.0f;
	}
	
	// Calculate action direction (from player to target)
	FVector DirectionToTarget = StoredTargetLocation - StoredPlayerLocation;
	DirectionToTarget.Z = 0.0f;  // Keep horizontal
	if (!DirectionToTarget.IsNearlyZero())
	{
		ActionDirection = DirectionToTarget.GetSafeNormal();
	}
	else
	{
		ActionDirection = FVector::ForwardVector;
	}
	
	UE_LOG(LogTemp, Verbose, TEXT("CinematicBase: Triggered, Player at (%.1f, %.1f, %.1f), Target at (%.1f, %.1f, %.1f)"),
		StoredPlayerLocation.X, StoredPlayerLocation.Y, StoredPlayerLocation.Z,
		StoredTargetLocation.X, StoredTargetLocation.Y, StoredTargetLocation.Z);
}

/**
 * OnActivated - Called when cinematic is fully active (blend in complete)
 * 
 * Can be used to trigger gameplay effects like disabling player input.
 */
void UCameraModifier_CinematicBase::OnActivated()
{
	Super::OnActivated();
	
	// Could notify game systems to disable player camera input here
	// if (bBlockPlayerInput && CameraManager)
	// {
	//     CameraManager->SetPlayerInputEnabled(false);
	// }
	
	UE_LOG(LogTemp, Verbose, TEXT("CinematicBase: Activated (blend in complete)"));
}

/**
 * OnDeactivating - Called when cinematic begins blend out
 * 
 * Prepares for return to normal camera.
 */
void UCameraModifier_CinematicBase::OnDeactivating()
{
	Super::OnDeactivating();
	
	// Could notify game systems to re-enable player camera input here
	// if (bBlockPlayerInput && CameraManager)
	// {
	//     CameraManager->SetPlayerInputEnabled(true);
	// }
	
	UE_LOG(LogTemp, Verbose, TEXT("CinematicBase: Deactivating (starting blend out)"));
}

/**
 * ComputeEffect - Compute cinematic camera transform
 * 
 * Unlike Shake/Reaction modifiers that compute offsets, Cinematic modifiers
 * compute a full camera transform. This is stored in OverrideTransform and
 * bIsOverriding is set to true.
 * 
 * Derived classes should:
 * 1. Calculate desired camera position
 * 2. Calculate look-at rotation
 * 3. Call InterpolateCameraTransform() for smooth motion
 * 4. Call base ComputeEffect to apply FOV and letterbox
 */
void UCameraModifier_CinematicBase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Update stored player location if not set
	if (StoredPlayerLocation.IsNearlyZero())
	{
		StoredPlayerLocation = Context.InputContext.CharacterLocation;
	}
	
	// Update target location if actor is valid and moving
	if (StoredTargetActor.IsValid())
	{
		// Option: Track moving target or use stored position
		// For most cinematics, we use stored position for stability
		// StoredTargetLocation = StoredTargetActor->GetActorLocation();
	}
	
	// Calculate effective weight
	float EffectiveWeight = CurrentWeight * CurrentIntensity;
	
	// Base implementation computes a default cinematic position
	// Derived classes should override and call Super at the end
	
	if (!bTransformInitialized)
	{
		// Initialize transform from current camera
		CurrentCameraTransform.SetLocation(Context.InputContext.PreviousCameraLocation);
		CurrentCameraTransform.SetRotation(Context.InputContext.PreviousCameraRotation.Quaternion());
		bTransformInitialized = true;
	}
	
	// Calculate default cinematic position (can be overridden)
	FVector DesiredPosition = CalculateCinematicPosition(Context);
	FVector LookAtPoint = CalculateMidpoint(Context);
	FRotator DesiredRotation = CalculateLookAtRotation(DesiredPosition, LookAtPoint);
	
	// Create target transform
	FTransform TargetTransform;
	TargetTransform.SetLocation(DesiredPosition);
	TargetTransform.SetRotation(DesiredRotation.Quaternion());
	
	// Interpolate towards target
	InterpolateCameraTransform(DeltaTime, TargetTransform, TransformInterpSpeed);
	
	// Set override transform (this is what Stage 4 will use)
	OverrideTransform = CurrentCameraTransform;
	bIsOverriding = true;
	
	// Apply FOV override
	if (CinematicFOV > 0.0f)
	{
		FOVEffect = (CinematicFOV - Context.Output.FOV) * EffectiveWeight;
	}
	
	// Apply letterbox effect (using vignette as approximation)
	if (bApplyLetterbox)
	{
		VignetteEffect = LetterboxAmount * EffectiveWeight;
	}
}

/**
 * CalculateCinematicPosition - Calculate camera position for cinematic shot
 * 
 * Default implementation places camera at an offset from the midpoint
 * between player and target. Derived classes override for specific behaviors.
 * 
 * @param Context Current execution context
 * @return World position for camera
 */
FVector UCameraModifier_CinematicBase::CalculateCinematicPosition(const FStageExecutionContext& Context) const
{
	// Calculate midpoint between player and target
	FVector MidPoint = CalculateMidpoint(Context);
	
	// Calculate coordinate system based on action direction
	FVector Forward = ActionDirection;
	FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	FVector Up = FVector::UpVector;
	
	// Handle edge case where action direction is vertical
	if (Right.IsNearlyZero())
	{
		Right = FVector::RightVector;
	}
	
	// Apply offset in local space
	// CameraOffset.X = forward/back (negative = behind action)
	// CameraOffset.Y = left/right (positive = right side)
	// CameraOffset.Z = up/down (positive = above)
	FVector CameraPos = MidPoint;
	CameraPos += Forward * CameraOffset.X;
	CameraPos += Right * CameraOffset.Y;
	CameraPos += Up * (CameraOffset.Z + HeightOffset);
	
	return CameraPos;
}

/**
 * CalculateLookAtRotation - Calculate rotation to look at target
 * 
 * @param CameraPos Current camera position
 * @param TargetPos Target to look at
 * @return Rotation that points camera at target
 */
FRotator UCameraModifier_CinematicBase::CalculateLookAtRotation(const FVector& CameraPos, const FVector& TargetPos) const
{
	FVector LookDirection = TargetPos - CameraPos;
	
	if (LookDirection.IsNearlyZero())
	{
		return FRotator::ZeroRotator;
	}
	
	return LookDirection.GetSafeNormal().Rotation();
}

/**
 * CalculateMidpoint - Calculate midpoint between player and target
 * 
 * Useful for framing both characters in shot.
 * 
 * @param Context Current execution context
 * @return Midpoint location
 */
FVector UCameraModifier_CinematicBase::CalculateMidpoint(const FStageExecutionContext& Context) const
{
	FVector PlayerPos = StoredPlayerLocation;
	FVector TargetPos = StoredTargetLocation;
	
	// If stored positions are invalid, use context
	if (PlayerPos.IsNearlyZero())
	{
		PlayerPos = Context.InputContext.CharacterLocation;
	}
	
	if (TargetPos.IsNearlyZero())
	{
		// Default to a point in front of player
		TargetPos = PlayerPos + ActionDirection * 200.0f;
	}
	
	// Calculate midpoint, slightly biased towards target for better framing
	// 0.5 = exact midpoint, 0.6 = closer to target
	float TargetBias = 0.55f;
	return FMath::Lerp(PlayerPos, TargetPos, TargetBias);
}

/**
 * InterpolateCameraTransform - Smoothly interpolate camera transform
 * 
 * Uses separate interpolation for position and rotation for smoother results.
 * 
 * @param DeltaTime Frame delta time
 * @param TargetTransform Desired transform
 * @param InterpSpeed Interpolation speed
 */
void UCameraModifier_CinematicBase::InterpolateCameraTransform(float DeltaTime, const FTransform& TargetTransform, float InterpSpeed)
{
	// Interpolate position
	FVector CurrentPos = CurrentCameraTransform.GetLocation();
	FVector TargetPos = TargetTransform.GetLocation();
	FVector NewPos = FMath::VInterpTo(CurrentPos, TargetPos, DeltaTime, InterpSpeed);
	
	// Interpolate rotation
	FQuat CurrentRot = CurrentCameraTransform.GetRotation();
	FQuat TargetRot = TargetTransform.GetRotation();
	FQuat NewRot = FMath::QInterpTo(CurrentRot, TargetRot, DeltaTime, InterpSpeed);
	
	// Update current transform
	CurrentCameraTransform.SetLocation(NewPos);
	CurrentCameraTransform.SetRotation(NewRot);
}

/**
 * ApplyLetterbox - Apply letterbox effect
 * 
 * Uses VignetteEffect as an approximation for letterbox bars.
 * For true letterbox, would need to render actual black bars.
 * 
 * @param Amount Letterbox amount (0-1, typically 0.1-0.2)
 */
void UCameraModifier_CinematicBase::ApplyLetterbox(float Amount)
{
	VignetteEffect = FMath::Clamp(Amount, 0.0f, 0.5f);
}


//========================================
// C01-C05: Implementations in subsequent Sub-Steps
//========================================

// C01: UCameraModifier_C01_Cinematic_Execution - Sub-Step 4.4.3
// C02: UCameraModifier_C02_Cinematic_Backstab - Sub-Step 4.4.4
// C03: UCameraModifier_C03_Cinematic_Riposte - Sub-Step 4.4.5
// C04: UCameraModifier_C04_Cinematic_BossIntro - Sub-Step 4.4.6
// C05: UCameraModifier_C05_Cinematic_BossPhase - Sub-Step 4.4.7


//========================================
// C01: Execution Camera Implementation
//========================================

/**
 * Constructor - Configure execution camera parameters
 * 
 * Execution camera orbits around the action:
 * - Slow orbit speed for cinematic feel
 * - Low angle to emphasize the brutal action
 * - Tight FOV for dramatic focus
 * - Letterbox for cinematic framing
 */
UCameraModifier_C01_Cinematic_Execution::UCameraModifier_C01_Cinematic_Execution()
	: OrbitSpeed(35.0f)
	, OrbitRadius(220.0f)
	, OrbitHeight(-20.0f)
	, StartOrbitAngle(0.0f)
	, CurrentOrbitAngle(0.0f)
{
	// Configure cinematic settings
	CameraOffset = FVector(-180.0f, 80.0f, 40.0f);
	HeightOffset = 30.0f;
	CameraDistance = 220.0f;
	CinematicFOV = 65.0f;
	
	// Letterbox for cinematic feel
	bApplyLetterbox = true;
	LetterboxAmount = 0.15f;
	
	// Smooth camera movement
	TransformInterpSpeed = 6.0f;
	
	// Block player input during execution
	bBlockPlayerInput = true;
}

/**
 * OnTriggered - Initialize execution camera state
 * 
 * Stores positions and calculates initial orbit angle based on
 * the current camera position relative to the action.
 */
void UCameraModifier_C01_Cinematic_Execution::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent to store basic positions
	Super::OnTriggered(InTriggerData);
	
	// Get victim/target location
	if (InTriggerData.TargetActor.IsValid())
	{
		StoredTargetLocation = InTriggerData.TargetActor->GetActorLocation();
		StoredTargetActor = InTriggerData.TargetActor;
	}
	
	// Get player/attacker location
	if (InTriggerData.SourceActor.IsValid())
	{
		StoredPlayerLocation = InTriggerData.SourceActor->GetActorLocation();
	}
	
	// Calculate action center (midpoint)
	FVector ActionCenter = (StoredPlayerLocation + StoredTargetLocation) * 0.5f;
	
	// Calculate initial orbit angle from current camera direction
	// This ensures smooth transition from current camera position
	FVector FromCenter = StoredPlayerLocation - ActionCenter;
	FromCenter.Z = 0.0f;
	if (!FromCenter.IsNearlyZero())
	{
		StartOrbitAngle = FMath::Atan2(FromCenter.Y, FromCenter.X) * (180.0f / PI);
		// Offset by 90 degrees to start from side view
		StartOrbitAngle += 90.0f;
	}
	else
	{
		StartOrbitAngle = 0.0f;
	}
	
	CurrentOrbitAngle = StartOrbitAngle;
	
	// Randomize orbit direction for variety
	if (FMath::RandBool())
	{
		OrbitSpeed = FMath::Abs(OrbitSpeed);  // Counter-clockwise
	}
	else
	{
		OrbitSpeed = -FMath::Abs(OrbitSpeed);  // Clockwise
	}
	
	// Scale orbit radius based on intensity
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.8f, 1.3f);
	OrbitRadius = 220.0f * IntensityScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("C01 Execution: Triggered, Start Angle=%.1f, Speed=%.1f, Radius=%.1f"),
		StartOrbitAngle, OrbitSpeed, OrbitRadius);
}

/**
 * ComputeEffect - Calculate orbiting camera position each frame
 * 
 * Camera orbits around the midpoint between player and victim,
 * looking at the action center. Includes vertical wave motion
 * for added dynamism.
 */
void UCameraModifier_C01_Cinematic_Execution::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Update orbit angle
	CurrentOrbitAngle += OrbitSpeed * DeltaTime;
	
	// Keep angle in reasonable range
	if (CurrentOrbitAngle > 360.0f) CurrentOrbitAngle -= 360.0f;
	if (CurrentOrbitAngle < -360.0f) CurrentOrbitAngle += 360.0f;
	
	// Calculate action center (midpoint, slightly biased towards target)
	FVector ActionCenter = FMath::Lerp(StoredPlayerLocation, StoredTargetLocation, 0.55f);
	
	// Calculate camera position on orbit
	float AngleRad = FMath::DegreesToRadians(CurrentOrbitAngle);
	
	// Orbit radius with slight pulse
	float PulseRadius = OrbitRadius + FMath::Sin(NormalizedTime * PI * 2.0f) * 15.0f;
	
	FVector OrbitOffset;
	OrbitOffset.X = FMath::Cos(AngleRad) * PulseRadius;
	OrbitOffset.Y = FMath::Sin(AngleRad) * PulseRadius;
	
	// Height: start low, rise slightly during execution, then lower
	// Creates a subtle crane-like motion
	float HeightCurve = FMath::Sin(NormalizedTime * PI);  // 0 -> 1 -> 0
	float DynamicHeight = OrbitHeight + HeightCurve * 40.0f;
	OrbitOffset.Z = DynamicHeight;
	
	// Final camera position
	FVector DesiredPosition = ActionCenter + OrbitOffset;
	
	// Look at point: slightly above action center for better framing
	FVector LookAtPoint = ActionCenter + FVector(0.0f, 0.0f, 60.0f);
	
	// Calculate look-at rotation
	FRotator DesiredRotation = CalculateLookAtRotation(DesiredPosition, LookAtPoint);
	
	// Create target transform
	FTransform TargetTransform;
	TargetTransform.SetLocation(DesiredPosition);
	TargetTransform.SetRotation(DesiredRotation.Quaternion());
	
	// Smooth interpolation to target
	InterpolateCameraTransform(DeltaTime, TargetTransform, TransformInterpSpeed);
	
	// Set override transform
	OverrideTransform = CurrentCameraTransform;
	bIsOverriding = true;
	
	// Apply FOV with slight pulse for impact moments
	float FOVPulse = FMath::Sin(NormalizedTime * PI * 3.0f) * 2.0f;
	FOVEffect = (CinematicFOV + FOVPulse) - Context.Output.FOV;
	
	// Letterbox effect
	if (bApplyLetterbox)
	{
		VignetteEffect = LetterboxAmount;
	}
	
	// Time dilation hint for dramatic effect
	// Slow during main action, speed up at end
	if (NormalizedTime < 0.75f)
	{
		TimeDilationEffect = 0.75f;  // 75% speed
	}
	else
	{
		// Smooth return to normal speed
		float ReturnProgress = (NormalizedTime - 0.75f) / 0.25f;
		TimeDilationEffect = FMath::Lerp(0.75f, 1.0f, ReturnProgress);
	}
	
	// Slight desaturation for cinematic look (optional)
	SaturationEffect = 0.9f;
}


//========================================
// C02: Backstab Camera Implementation
//========================================

/**
 * Constructor - Configure backstab camera parameters
 * 
 * Backstab camera provides over-shoulder view:
 * - Camera behind and to the side of player
 * - Tight FOV for intimate kill moment
 * - Minimal movement for clean shot
 * - Emphasis on the stealth/surprise attack
 */
UCameraModifier_C02_Cinematic_Backstab::UCameraModifier_C02_Cinematic_Backstab()
	: SideOffset(50.0f)
	, BackDistance(140.0f)
	, BackstabHeightOffset(30.0f)
	, PushInAmount(20.0f)
{
	// Configure cinematic settings
	CameraOffset = FVector(-140.0f, 50.0f, 30.0f);
	HeightOffset = 40.0f;
	CameraDistance = 160.0f;
	CinematicFOV = 62.0f;  // Tighter FOV for close-up
	
	// Letterbox for cinematic feel
	bApplyLetterbox = true;
	LetterboxAmount = 0.12f;
	
	// Smooth but responsive camera
	TransformInterpSpeed = 8.0f;
	
	// Block player input during backstab
	bBlockPlayerInput = true;
}

/**
 * OnTriggered - Initialize backstab camera state
 * 
 * Stores attack direction and randomizes which side to view from.
 * The camera will be positioned behind the player, offset to one side.
 */
void UCameraModifier_C02_Cinematic_Backstab::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent to store basic positions
	Super::OnTriggered(InTriggerData);
	
	// Get player/attacker location
	if (InTriggerData.SourceActor.IsValid())
	{
		StoredPlayerLocation = InTriggerData.SourceActor->GetActorLocation();
	}
	
	// Get victim/target location
	if (InTriggerData.TargetActor.IsValid())
	{
		StoredTargetLocation = InTriggerData.TargetActor->GetActorLocation();
		StoredTargetActor = InTriggerData.TargetActor;
	}
	
	// Calculate attack direction (player to target, horizontal only)
	FVector AttackDir = StoredTargetLocation - StoredPlayerLocation;
	AttackDir.Z = 0.0f;
	if (!AttackDir.IsNearlyZero())
	{
		ActionDirection = AttackDir.GetSafeNormal();
	}
	else
	{
		ActionDirection = FVector::ForwardVector;
	}
	
	// Randomize which side to view from for variety
	if (FMath::RandBool())
	{
		SideOffset = FMath::Abs(SideOffset);   // Right side
	}
	else
	{
		SideOffset = -FMath::Abs(SideOffset);  // Left side
	}
	
	UE_LOG(LogTemp, Verbose, TEXT("C02 Backstab: Triggered, SideOffset=%.1f"), SideOffset);
}

/**
 * ComputeEffect - Calculate over-shoulder camera position each frame
 * 
 * Camera starts behind player and slowly moves to the side during
 * the backstab animation, providing a cinematic reveal of the kill.
 */
void UCameraModifier_C02_Cinematic_Backstab::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Calculate action center (midpoint between player and target)
	FVector ActionCenter = FMath::Lerp(StoredPlayerLocation, StoredTargetLocation, 0.5f);
	
	// Calculate coordinate axes
	FVector Forward = ActionDirection;
	FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Right = FVector::RightVector;
	}
	FVector Up = FVector::UpVector;
	
	// Camera movement: starts behind, moves to side during animation
	// Use smooth curve for natural motion
	float MovementCurve = FMath::InterpEaseInOut(0.0f, 1.0f, NormalizedTime, 2.0f);
	
	// Behind offset: starts far behind, moves closer
	float CurrentBackDistance = FMath::Lerp(BackDistance, BackDistance - PushInAmount, MovementCurve);
	
	// Side offset: starts centered, moves to side
	float CurrentSideOffset = FMath::Lerp(SideOffset * 0.3f, SideOffset, MovementCurve);
	
	// Height: slight rise during animation
	float CurrentHeight = FMath::Lerp(BackstabHeightOffset, BackstabHeightOffset + 15.0f, MovementCurve);
	
	// Calculate camera position
	FVector DesiredPosition = ActionCenter;
	DesiredPosition -= Forward * CurrentBackDistance;  // Behind the action
	DesiredPosition += Right * CurrentSideOffset;      // To the side
	DesiredPosition += Up * CurrentHeight;             // Height offset
	
	// Look at target's upper body
	FVector LookAtPoint = StoredTargetLocation + FVector(0.0f, 0.0f, 80.0f);
	
	// Blend look-at point from target towards midpoint over time
	FVector MidLookAt = ActionCenter + FVector(0.0f, 0.0f, 70.0f);
	LookAtPoint = FMath::Lerp(LookAtPoint, MidLookAt, NormalizedTime * 0.4f);
	
	// Calculate look-at rotation
	FRotator DesiredRotation = CalculateLookAtRotation(DesiredPosition, LookAtPoint);
	
	// Create target transform
	FTransform TargetTransform;
	TargetTransform.SetLocation(DesiredPosition);
	TargetTransform.SetRotation(DesiredRotation.Quaternion());
	
	// Smooth interpolation to target
	InterpolateCameraTransform(DeltaTime, TargetTransform, TransformInterpSpeed);
	
	// Set override transform
	OverrideTransform = CurrentCameraTransform;
	bIsOverriding = true;
	
	// FOV with impact punch
	float FOVValue = CinematicFOV;
	
	// Impact moment: slight FOV punch at 30-50% of animation
	if (NormalizedTime > 0.30f && NormalizedTime < 0.50f)
	{
		float ImpactProgress = (NormalizedTime - 0.30f) / 0.20f;
		float ImpactPunch = FMath::Sin(ImpactProgress * PI) * 4.0f;
		FOVValue += ImpactPunch;
	}
	
	FOVEffect = FOVValue - Context.Output.FOV;
	
	// Letterbox effect
	if (bApplyLetterbox)
	{
		VignetteEffect = LetterboxAmount;
	}
	
	// Time dilation at moment of impact
	if (NormalizedTime > 0.25f && NormalizedTime < 0.45f)
	{
		// Slow motion during the actual backstab
		float SlowMoProgress = (NormalizedTime - 0.25f) / 0.20f;
		float SlowMoCurve = FMath::Sin(SlowMoProgress * PI);
		TimeDilationEffect = FMath::Lerp(1.0f, 0.6f, SlowMoCurve);
	}
	else if (NormalizedTime >= 0.45f && NormalizedTime < 0.55f)
	{
		// Smooth return to normal
		float ReturnProgress = (NormalizedTime - 0.45f) / 0.10f;
		TimeDilationEffect = FMath::Lerp(0.6f, 1.0f, ReturnProgress);
	}
	else
	{
		TimeDilationEffect = 1.0f;
	}
}


	//========================================
	// C03: Riposte Camera Implementation
	//========================================

/**
 * Constructor - Configure riposte camera parameters
 * 
 * Riposte camera is the snappiest cinematic:
 * - Quick pull-in for dramatic emphasis
 * - Low angle to show power of the counter
 * - Tightest FOV for maximum focus
 * - Strongest time dilation (reward for skill)
 * - Shortest duration (player earned quick kill)
 */
UCameraModifier_C03_Cinematic_Riposte::UCameraModifier_C03_Cinematic_Riposte()
	: PullInDistance(80.0f)
	, LowAngleOffset(-35.0f)
	, SideAngle(50.0f)
	, RiposteTimeDilation(0.4f)
{
	// Configure cinematic settings
	CameraOffset = FVector(-120.0f, 70.0f, -20.0f);
	HeightOffset = -20.0f;  // Low angle
	CameraDistance = 150.0f;
	CinematicFOV = 55.0f;   // Tightest FOV
	
	// Letterbox for cinematic feel
	bApplyLetterbox = true;
	LetterboxAmount = 0.10f;
	
	// Fast camera for snappy feel
	TransformInterpSpeed = 12.0f;  // Faster than other cinematics
	
	// Block player input during riposte
	bBlockPlayerInput = true;
}

/**
 * OnTriggered - Initialize riposte camera state
 * 
 * Riposte is a reward for skillful play, so the camera
 * emphasizes the satisfying counter-attack moment.
 */
void UCameraModifier_C03_Cinematic_Riposte::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent to store basic positions
	Super::OnTriggered(InTriggerData);
	
	// Get player/attacker location
	if (InTriggerData.SourceActor.IsValid())
	{
		StoredPlayerLocation = InTriggerData.SourceActor->GetActorLocation();
	}
	
	// Get victim/target location (the staggered enemy)
	if (InTriggerData.TargetActor.IsValid())
	{
		StoredTargetLocation = InTriggerData.TargetActor->GetActorLocation();
		StoredTargetActor = InTriggerData.TargetActor;
	}
	
	// Calculate attack direction
	FVector AttackDir = StoredTargetLocation - StoredPlayerLocation;
	AttackDir.Z = 0.0f;
	if (!AttackDir.IsNearlyZero())
	{
		ActionDirection = AttackDir.GetSafeNormal();
	}
	else
	{
		ActionDirection = FVector::ForwardVector;
	}
	
	// Scale pull-in based on intensity (higher intensity = closer camera)
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.8f, 1.5f);
	PullInDistance = 80.0f * IntensityScale;
	
	// Randomize side angle direction
	if (FMath::RandBool())
	{
		SideAngle = FMath::Abs(SideAngle);
	}
	else
	{
		SideAngle = -FMath::Abs(SideAngle);
	}
	
	UE_LOG(LogTemp, Verbose, TEXT("C03 Riposte: Triggered, Intensity=%.2f, PullIn=%.1f"),
		CurrentIntensity, PullInDistance);
}

/**
 * ComputeEffect - Calculate dramatic low-angle camera each frame
 * 
 * Camera quickly pulls in to a low angle, emphasizing the
 * power and skill of the riposte. This is the most "rewarding"
 * feeling cinematic.
 */
void UCameraModifier_C03_Cinematic_Riposte::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Calculate action center (slightly biased towards target - the staggered enemy)
	FVector ActionCenter = FMath::Lerp(StoredPlayerLocation, StoredTargetLocation, 0.55f);
	
	// Calculate coordinate axes
	FVector Forward = ActionDirection;
	FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Right = FVector::RightVector;
	}
	FVector Up = FVector::UpVector;
	
	// Pull-in curve: very fast at start (reward feeling), then hold
	float PullInCurve;
	if (NormalizedTime < 0.15f)
	{
		// Very fast pull-in (0-15%)
		PullInCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.15f, 3.0f);
	}
	else
	{
		// Hold at maximum
		PullInCurve = 1.0f;
	}
	
	// Calculate camera distance with pull-in
	float BaseDistance = 180.0f;
	float CurrentDistance = BaseDistance - (PullInDistance * PullInCurve);
	
	// Side offset with angle
	float AngleRad = FMath::DegreesToRadians(SideAngle);
	float SideOffset = FMath::Sin(AngleRad) * CurrentDistance * 0.5f;
	float ForwardOffset = FMath::Cos(AngleRad) * CurrentDistance;
	
	// Low angle: start low, stay low
	float CurrentHeight = LowAngleOffset;
	
	// Slight rise at the end for transition back
	if (NormalizedTime > 0.8f)
	{
		float RiseProgress = (NormalizedTime - 0.8f) / 0.2f;
		CurrentHeight = FMath::Lerp(LowAngleOffset, LowAngleOffset + 30.0f, RiseProgress);
	}
	
	// Calculate camera position
	FVector DesiredPosition = ActionCenter;
	DesiredPosition -= Forward * ForwardOffset;      // In front/behind based on angle
	DesiredPosition += Right * SideOffset;           // To the side
	DesiredPosition += Up * CurrentHeight;           // Low angle
	
	// Look at point: action center, at chest height
	FVector LookAtPoint = ActionCenter + FVector(0.0f, 0.0f, 70.0f);
	
	// Calculate look-at rotation
	FRotator DesiredRotation = CalculateLookAtRotation(DesiredPosition, LookAtPoint);
	
	// Create target transform
	FTransform TargetTransform;
	TargetTransform.SetLocation(DesiredPosition);
	TargetTransform.SetRotation(DesiredRotation.Quaternion());
	
	// Fast interpolation for snappy feel
	InterpolateCameraTransform(DeltaTime, TargetTransform, TransformInterpSpeed);
	
	// Set override transform
	OverrideTransform = CurrentCameraTransform;
	bIsOverriding = true;
	
	// FOV with tightening pulse at impact
	float FOVValue = CinematicFOV;
	
	// FOV tightens during impact (15-35%)
	if (NormalizedTime > 0.15f && NormalizedTime < 0.35f)
	{
		float PulseProgress = (NormalizedTime - 0.15f) / 0.20f;
		float FOVTighten = FMath::Sin(PulseProgress * PI) * 5.0f;
		FOVValue -= FOVTighten;  // Tighter FOV = more intense
	}
	
	FOVEffect = FOVValue - Context.Output.FOV;
	
	// Letterbox effect
	if (bApplyLetterbox)
	{
		VignetteEffect = LetterboxAmount;
	}
	
	// Strong time dilation - this is the reward!
	// Riposte has the strongest slow-mo of all cinematics
	if (NormalizedTime < 0.10f)
	{
		// Quick start
		TimeDilationEffect = 1.0f;
	}
	else if (NormalizedTime < 0.40f)
	{
		// Strong slow-mo during impact
		TimeDilationEffect = RiposteTimeDilation;  // 0.4 = 40% speed
	}
	else if (NormalizedTime < 0.60f)
	{
		// Gradual return to normal
		float ReturnProgress = (NormalizedTime - 0.40f) / 0.20f;
		TimeDilationEffect = FMath::Lerp(RiposteTimeDilation, 1.0f, FMath::InterpEaseIn(0.0f, 1.0f, ReturnProgress, 2.0f));
	}
	else
	{
		TimeDilationEffect = 1.0f;
	}
	
	// Slight saturation reduction for dramatic feel
	SaturationEffect = 0.85f;
}


	//========================================
	// C04: Boss Intro Camera Implementation
	//========================================

/**
 * Constructor - Configure boss intro camera parameters
 * 
 * Boss intro is the longest and most dramatic cinematic:
 * - Three-phase structure (establish -> reveal -> return)
 * - Wide FOV to show boss scale
 * - Sweeping camera motion around boss
 * - Strong letterbox for cinematic impact
 * - Highest priority among cinematics
 */
UCameraModifier_C04_Cinematic_BossIntro::UCameraModifier_C04_Cinematic_BossIntro()
	: SweepAngle(100.0f)
	, SweepHeight(180.0f)
	, ArenaDistance(450.0f)
	, Phase1Ratio(0.25f)
	, Phase2Ratio(0.45f)
	, StartPosition(FVector::ZeroVector)
	, BossRevealPosition(FVector::ZeroVector)
	, IntroPhase(0.0f)
{
	// Configure cinematic settings
	CameraOffset = FVector(-350.0f, 0.0f, 150.0f);
	HeightOffset = 150.0f;
	CameraDistance = 450.0f;
	CinematicFOV = 75.0f;  // Wide FOV to show boss scale
	
	// Strong letterbox for maximum cinematic impact
	bApplyLetterbox = true;
	LetterboxAmount = 0.18f;
	
	// Slower camera for epic feel
	TransformInterpSpeed = 4.0f;
	
	// Block player input during boss intro
	bBlockPlayerInput = true;
}

/**
 * OnTriggered - Initialize boss intro camera state
 * 
 * Calculates the camera path for the three-phase intro sequence.
 * The camera will sweep from behind the player, around the boss,
 * then return to a combat-ready position.
 */
void UCameraModifier_C04_Cinematic_BossIntro::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent to store basic positions
	Super::OnTriggered(InTriggerData);
	
	// Get player location
	if (InTriggerData.SourceActor.IsValid())
	{
		StoredPlayerLocation = InTriggerData.SourceActor->GetActorLocation();
	}
	
	// Get boss location (target actor)
	if (InTriggerData.TargetActor.IsValid())
	{
		StoredTargetLocation = InTriggerData.TargetActor->GetActorLocation();
		StoredTargetActor = InTriggerData.TargetActor;
	}
	
	// Calculate direction from player to boss
	FVector ToBoss = StoredTargetLocation - StoredPlayerLocation;
	ToBoss.Z = 0.0f;
	if (!ToBoss.IsNearlyZero())
	{
		ActionDirection = ToBoss.GetSafeNormal();
	}
	else
	{
		ActionDirection = FVector::ForwardVector;
	}
	
	// Calculate right vector for sweep
	FVector Right = FVector::CrossProduct(FVector::UpVector, ActionDirection).GetSafeNormal();
	
	// Calculate start position (behind player, elevated)
	StartPosition = StoredPlayerLocation;
	StartPosition -= ActionDirection * 300.0f;  // Behind player
	StartPosition += FVector(0.0f, 0.0f, 150.0f);  // Elevated
	
	// Calculate boss reveal position (where to look during reveal)
	// Assuming large boss, look at upper body/head area
	BossRevealPosition = StoredTargetLocation;
	BossRevealPosition.Z += 200.0f;  // Boss head height (adjust based on boss size)
	
	// Reset intro phase
	IntroPhase = 0.0f;
	
	UE_LOG(LogTemp, Verbose, TEXT("C04 BossIntro: Triggered, Boss at (%.0f, %.0f, %.0f), SweepAngle=%.1f"),
		StoredTargetLocation.X, StoredTargetLocation.Y, StoredTargetLocation.Z, SweepAngle);
}

/**
 * ComputeEffect - Calculate sweeping intro camera each frame
 * 
 * Three-phase camera movement:
 * Phase 1: Establish - Start behind player, push towards boss
 * Phase 2: Reveal - Sweep around boss in an arc
 * Phase 3: Return - Transition back to combat camera position
 */
void UCameraModifier_C04_Cinematic_BossIntro::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	IntroPhase = NormalizedTime;
	
	// Calculate phase boundaries
	float Phase1End = Phase1Ratio;                    // 0.25
	float Phase2End = Phase1Ratio + Phase2Ratio;      // 0.70
	// Phase 3 is the remainder                       // 0.70 - 1.00
	
	FVector DesiredPosition;
	FVector LookAtPoint;
	
	//========================================
	// Phase 1: Establishing Shot (0% - 25%)
	//========================================
	if (NormalizedTime < Phase1End)
	{
		float PhaseProgress = NormalizedTime / Phase1End;
		float EasedProgress = FMath::InterpEaseInOut(0.0f, 1.0f, PhaseProgress, 2.0f);
		
		// Start behind player, slowly push towards boss
		FVector PushDirection = ActionDirection;
		float PushDistance = 150.0f * EasedProgress;
		
		DesiredPosition = StartPosition + PushDirection * PushDistance;
		
		// Slight rise during establish
		DesiredPosition.Z += 30.0f * EasedProgress;
		
		// Look at: transition from player area to boss reveal point
		FVector PlayerLookAt = StoredPlayerLocation + FVector(0.0f, 0.0f, 100.0f);
		LookAtPoint = FMath::Lerp(PlayerLookAt, BossRevealPosition, EasedProgress);
	}
	//========================================
	// Phase 2: Boss Reveal Sweep (25% - 70%)
	//========================================
	else if (NormalizedTime < Phase2End)
	{
		float PhaseProgress = (NormalizedTime - Phase1End) / Phase2Ratio;
		float EasedProgress = FMath::InterpEaseInOut(0.0f, 1.0f, PhaseProgress, 2.0f);
		
		// Arc around the boss
		float CurrentAngle = FMath::Lerp(0.0f, SweepAngle, EasedProgress);
		float AngleRad = FMath::DegreesToRadians(CurrentAngle);
		
		// Starting direction for sweep (perpendicular to player-boss line)
		FVector Right = FVector::CrossProduct(FVector::UpVector, ActionDirection).GetSafeNormal();
		
		// Calculate position on arc around boss
		float CosAngle = FMath::Cos(AngleRad);
		float SinAngle = FMath::Sin(AngleRad);
		
		// Orbit position relative to boss
		FVector OrbitOffset;
		OrbitOffset = -ActionDirection * ArenaDistance * CosAngle;  // Forward/back
		OrbitOffset += Right * ArenaDistance * SinAngle;            // Side
		
		// Height varies during sweep (rises to peak, then descends)
		float HeightCurve = FMath::Sin(PhaseProgress * PI);
		float CurrentHeight = SweepHeight + HeightCurve * 80.0f;
		OrbitOffset.Z = CurrentHeight;
		
		DesiredPosition = StoredTargetLocation + OrbitOffset;
		
		// Always look at boss reveal point during sweep
		LookAtPoint = BossRevealPosition;
	}
	//========================================
	// Phase 3: Return to Combat (70% - 100%)
	//========================================
	else
	{
		float PhaseProgress = (NormalizedTime - Phase2End) / (1.0f - Phase2End);
		float EasedProgress = FMath::InterpEaseInOut(0.0f, 1.0f, PhaseProgress, 2.0f);
		
		// End position of sweep (where we are at end of Phase 2)
		float EndAngleRad = FMath::DegreesToRadians(SweepAngle);
		FVector Right = FVector::CrossProduct(FVector::UpVector, ActionDirection).GetSafeNormal();
		
		FVector SweepEndPos = StoredTargetLocation;
		SweepEndPos -= ActionDirection * ArenaDistance * FMath::Cos(EndAngleRad);
		SweepEndPos += Right * ArenaDistance * FMath::Sin(EndAngleRad);
		SweepEndPos.Z = StoredTargetLocation.Z + SweepHeight;
		
		// Calculate combat-ready position (behind and above player, looking at boss)
		FVector CombatPosition = StoredPlayerLocation;
		CombatPosition -= ActionDirection * 350.0f;  // Behind player
		CombatPosition += FVector(0.0f, 0.0f, 120.0f);  // Elevated
		
		// Interpolate from sweep end to combat position
		DesiredPosition = FMath::Lerp(SweepEndPos, CombatPosition, EasedProgress);
		
		// Look at: transition from boss to midpoint (combat framing)
		FVector CombatLookAt = FMath::Lerp(StoredPlayerLocation, StoredTargetLocation, 0.6f);
		CombatLookAt.Z += 80.0f;
		LookAtPoint = FMath::Lerp(BossRevealPosition, CombatLookAt, EasedProgress);
	}
	
	// Calculate look-at rotation
	FRotator DesiredRotation = CalculateLookAtRotation(DesiredPosition, LookAtPoint);
	
	// Create target transform
	FTransform TargetTransform;
	TargetTransform.SetLocation(DesiredPosition);
	TargetTransform.SetRotation(DesiredRotation.Quaternion());
	
	// Smooth interpolation (slower for epic feel)
	InterpolateCameraTransform(DeltaTime, TargetTransform, TransformInterpSpeed);
	
	// Set override transform
	OverrideTransform = CurrentCameraTransform;
	bIsOverriding = true;
	
	// Dynamic FOV - wider during boss reveal (Phase 2)
	float FOVValue = CinematicFOV;
	if (NormalizedTime > Phase1End && NormalizedTime < Phase2End)
	{
		float RevealProgress = (NormalizedTime - Phase1End) / Phase2Ratio;
		float FOVPunch = FMath::Sin(RevealProgress * PI) * 10.0f;
		FOVValue += FOVPunch;  // Wider during reveal
	}
	FOVEffect = FOVValue - Context.Output.FOV;
	
	// Strong letterbox for cinematic impact
	if (bApplyLetterbox)
	{
		VignetteEffect = LetterboxAmount;
	}
	
	// Time dilation: slow at the very start for dramatic entrance
	if (NormalizedTime < 0.10f)
	{
		// Dramatic slow-mo at start
		TimeDilationEffect = 0.5f;
	}
	else if (NormalizedTime < 0.20f)
	{
		// Return to normal
		float ReturnProgress = (NormalizedTime - 0.10f) / 0.10f;
		TimeDilationEffect = FMath::Lerp(0.5f, 1.0f, ReturnProgress);
	}
	else
	{
		TimeDilationEffect = 1.0f;
	}
	
	// Slight desaturation during intro for dramatic effect
	SaturationEffect = 0.9f;
}


//========================================
// C05: Boss Phase Transition Camera Implementation
//========================================

/**
 * Constructor - Configure boss phase camera parameters
 * 
 * Boss phase camera emphasizes the transformation moment:
 * - Focus entirely on boss (player is secondary)
 * - Pull back and rise to show transformation scale
 * - Screen shake during power-up
 * - FOV punch for impact
 * - Used for phase 2, phase 3, or enrage transitions
 */
UCameraModifier_C05_Cinematic_BossPhase::UCameraModifier_C05_Cinematic_BossPhase()
	: PullBackDistance(120.0f)
	, RiseHeight(100.0f)
	, TransformationShakeIntensity(4.0f)
	, FOVPunch(12.0f)
	, TransformationPhase(0.0f)
{
	// Configure cinematic settings
	CameraOffset = FVector(-280.0f, 60.0f, 80.0f);
	HeightOffset = 80.0f;
	CameraDistance = 320.0f;
	CinematicFOV = 72.0f;
	
	// Letterbox for dramatic effect
	bApplyLetterbox = true;
	LetterboxAmount = 0.14f;
	
	// Medium interpolation speed
	TransformInterpSpeed = 5.0f;
	
	// Block player input during phase transition
	bBlockPlayerInput = true;
}

/**
 * OnTriggered - Initialize boss phase camera state
 * 
 * Stores boss location as the primary focus.
 * Player location is secondary for this cinematic.
 */
void UCameraModifier_C05_Cinematic_BossPhase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent to store basic positions
	Super::OnTriggered(InTriggerData);
	
	// For boss phase, target is the boss (primary focus)
	if (InTriggerData.TargetActor.IsValid())
	{
		StoredTargetLocation = InTriggerData.TargetActor->GetActorLocation();
		StoredTargetActor = InTriggerData.TargetActor;
	}
	
	// Player location (secondary)
	if (InTriggerData.SourceActor.IsValid())
	{
		StoredPlayerLocation = InTriggerData.SourceActor->GetActorLocation();
	}
	
	// Calculate direction from player to boss
	FVector ToBoss = StoredTargetLocation - StoredPlayerLocation;
	ToBoss.Z = 0.0f;
	if (!ToBoss.IsNearlyZero())
	{
		ActionDirection = ToBoss.GetSafeNormal();
	}
	else
	{
		ActionDirection = FVector::ForwardVector;
	}
	
	// Reset transformation phase
	TransformationPhase = 0.0f;
	
	// Scale effects based on intensity (phase 3 more dramatic than phase 2)
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.8f, 1.5f);
	PullBackDistance = 120.0f * IntensityScale;
	RiseHeight = 100.0f * IntensityScale;
	TransformationShakeIntensity = 4.0f * IntensityScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("C05 BossPhase: Triggered, Intensity=%.2f, PullBack=%.1f"),
		CurrentIntensity, PullBackDistance);
}

/**
 * ComputeEffect - Calculate boss-focused dramatic camera each frame
 * 
 * Camera pulls back and rises to show boss transformation,
 * with screen shake during the power-up moment.
 */
void UCameraModifier_C05_Cinematic_BossPhase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	TransformationPhase = NormalizedTime;
	
	// Update stored target location if boss is moving during transformation
	if (StoredTargetActor.IsValid())
	{
		// Option: track boss or use stored position
		// For stability, we use stored position but can update if needed
		// StoredTargetLocation = StoredTargetActor->GetActorLocation();
	}
	
	// Calculate right vector for camera positioning
	FVector Right = FVector::CrossProduct(FVector::UpVector, ActionDirection).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Right = FVector::RightVector;
	}
	
	//========================================
	// Camera Movement: Pull back and rise
	//========================================
	
	// Pull back curve: quick pull at start, hold, then slight return
	float PullBackCurve;
	if (NormalizedTime < 0.25f)
	{
		// Quick pull back
		PullBackCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.25f, 2.0f);
	}
	else if (NormalizedTime < 0.75f)
	{
		// Hold at maximum
		PullBackCurve = 1.0f;
	}
	else
	{
		// Slight return for transition back
		float ReturnProgress = (NormalizedTime - 0.75f) / 0.25f;
		PullBackCurve = FMath::Lerp(1.0f, 0.7f, ReturnProgress);
	}
	
	// Rise curve: gradual rise, peak at middle, then descend
	float RiseCurve = FMath::Sin(NormalizedTime * PI);
	
	// Calculate base camera position
	float CurrentPullBack = CameraDistance + PullBackDistance * PullBackCurve;
	float CurrentRise = HeightOffset + RiseHeight * RiseCurve;
	
	FVector DesiredPosition = StoredTargetLocation;
	DesiredPosition -= ActionDirection * CurrentPullBack;  // Behind relative to player
	DesiredPosition += Right * 80.0f;  // Slight side offset
	DesiredPosition.Z += CurrentRise;
	
	//========================================
	// Screen Shake during transformation
	//========================================
	
	// Shake is strongest in the middle of the transition
	float ShakeCurve = 0.0f;
	if (NormalizedTime > 0.2f && NormalizedTime < 0.7f)
	{
		float ShakeProgress = (NormalizedTime - 0.2f) / 0.5f;
		ShakeCurve = FMath::Sin(ShakeProgress * PI);  // Peak at middle
	}
	
	float ScaledShake = TransformationShakeIntensity * ShakeCurve;
	if (ScaledShake > 0.1f)
	{
		// Apply perlin noise shake using TotalActiveTime for consistent timing
		FVector ShakeOffset;
		ShakeOffset.X = FMath::PerlinNoise1D(TotalActiveTime * 20.0f) * ScaledShake;
		ShakeOffset.Y = FMath::PerlinNoise1D(TotalActiveTime * 20.0f + 100.0f) * ScaledShake;
		ShakeOffset.Z = FMath::PerlinNoise1D(TotalActiveTime * 15.0f + 200.0f) * ScaledShake * 0.5f;
		
		DesiredPosition += ShakeOffset;
	}
	
	// Look at boss (upper body for large bosses)
	FVector LookAtPoint = StoredTargetLocation + FVector(0.0f, 0.0f, 150.0f);
	
	// Calculate look-at rotation
	FRotator DesiredRotation = CalculateLookAtRotation(DesiredPosition, LookAtPoint);
	
	// Add rotation shake during transformation
	if (ScaledShake > 0.1f)
	{
		DesiredRotation.Pitch += FMath::PerlinNoise1D(TotalActiveTime * 18.0f + 300.0f) * ScaledShake * 0.3f;
		DesiredRotation.Yaw += FMath::PerlinNoise1D(TotalActiveTime * 18.0f + 400.0f) * ScaledShake * 0.2f;
	}
	
	// Create target transform
	FTransform TargetTransform;
	TargetTransform.SetLocation(DesiredPosition);
	TargetTransform.SetRotation(DesiredRotation.Quaternion());
	
	// Interpolate to target
	InterpolateCameraTransform(DeltaTime, TargetTransform, TransformInterpSpeed);
	
	// Set override transform
	OverrideTransform = CurrentCameraTransform;
	bIsOverriding = true;
	
	//========================================
	// FOV Punch during transformation
	//========================================
	
	float FOVValue = CinematicFOV;
	
	// FOV punch: widen dramatically during power-up, then return
	if (NormalizedTime > 0.15f && NormalizedTime < 0.6f)
	{
		float PunchProgress = (NormalizedTime - 0.15f) / 0.45f;
		float PunchCurve = FMath::Sin(PunchProgress * PI);
		FOVValue += FOVPunch * PunchCurve;
	}
	
	FOVEffect = FOVValue - Context.Output.FOV;
	
	// Letterbox effect
	if (bApplyLetterbox)
	{
		VignetteEffect = LetterboxAmount;
	}
	
	//========================================
	// Time dilation during power-up
	//========================================
	
	if (NormalizedTime < 0.15f)
	{
		// Start slightly slowed
		TimeDilationEffect = 0.7f;
	}
	else if (NormalizedTime < 0.50f)
	{
		// Very slow during transformation peak
		float SlowProgress = (NormalizedTime - 0.15f) / 0.35f;
		float SlowCurve = FMath::Sin(SlowProgress * PI);
		TimeDilationEffect = FMath::Lerp(0.7f, 0.5f, SlowCurve);
	}
	else if (NormalizedTime < 0.70f)
	{
		// Return to normal
		float ReturnProgress = (NormalizedTime - 0.50f) / 0.20f;
		TimeDilationEffect = FMath::Lerp(0.5f, 1.0f, ReturnProgress);
	}
	else
	{
		TimeDilationEffect = 1.0f;
	}
	
	// Slight saturation reduction during power-up
	if (NormalizedTime > 0.2f && NormalizedTime < 0.6f)
	{
		SaturationEffect = 0.85f;
	}
	else
	{
		SaturationEffect = 1.0f;
	}
}
