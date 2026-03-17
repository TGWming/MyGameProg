// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modifiers/CameraModifier_Reaction.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModifier_Reaction.cpp
 * 
 * This file contains the implementations of the Reaction modifier base class
 * and all 6 Reaction modifiers (R01-R06).
 * 
 * Reaction modifiers provide directional camera effects for combat feedback.
 * The base class provides common functionality for Pull/Push/Drop/Wobble effects,
 * while derived classes configure specific parameters for different reactions.
 * 
 * Implementation Structure:
 * - ReactionBase: Common reaction functionality (this Sub-Step)
 * - R01-R03: Parry, PerfectParry, Stagger (Sub-Step 4.3.3)
 * - R04-R06: Knockback, Knockdown, GuardBreak (Sub-Step 4.3.4)
 */


//========================================
// ReactionBase - Common Reaction Functionality
//========================================

/**
 * Constructor - Initialize default reaction parameters
 * 
 * Default values provide moderate interpolation speeds.
 * Derived classes will configure specific parameters.
 */
UCameraModifier_ReactionBase::UCameraModifier_ReactionBase()
	: OffsetInterpSpeed(15.0f)
	, RecoveryInterpSpeed(8.0f)
	, bApplyDistanceEffect(true)
	, bApplyFOVEffect(true)
	, bApplyRotationEffect(false)
	, ReactionDirection(FVector::ForwardVector)
	, CurrentOffset(FVector::ZeroVector)
	, TargetOffset(FVector::ZeroVector)
	, CurrentRotationOffset(FRotator::ZeroRotator)
	, WobbleTime(0.0f)
	, SourceActor(nullptr)
	, TargetActor(nullptr)
{
}

/**
 * OnTriggered - Initialize reaction state when triggered
 * 
 * Caches the reaction direction and actor references for use during effect computation.
 */
void UCameraModifier_ReactionBase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Reset offsets
	CurrentOffset = FVector::ZeroVector;
	TargetOffset = FVector::ZeroVector;
	CurrentRotationOffset = FRotator::ZeroRotator;
	WobbleTime = 0.0f;
	
	// Cache actor references
	SourceActor = InTriggerData.SourceActor;
	TargetActor = InTriggerData.TargetActor;
	
	// Calculate and cache reaction direction
	// This will be updated each frame in ComputeEffect if actors move
	if (!InTriggerData.SourceDirection.IsNearlyZero())
	{
		ReactionDirection = InTriggerData.SourceDirection.GetSafeNormal();
	}
	else
	{
		ReactionDirection = FVector::ForwardVector;
	}
}

/**
 * ComputeEffect - Update reaction effect values each frame
 * 
 * This is the core reaction computation. It:
 * 1. Updates offset interpolation towards target (or zero during recovery)
 * 2. Applies the offset to PositionEffect
 * 3. Applies any rotation/FOV/distance effects
 * 
 * Derived classes typically call base and then add their specific effects.
 */
void UCameraModifier_ReactionBase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Update wobble time for wobble effects
	WobbleTime += DeltaTime;
	
	// Update offset interpolation
	UpdateOffsetInterpolation(DeltaTime);
	
	// Apply position effect with weight and intensity
	PositionEffect = CurrentOffset * CurrentWeight * CurrentIntensity;
	
	// Apply rotation effect if enabled
	if (bApplyRotationEffect)
	{
		RotationEffect = CurrentRotationOffset * CurrentWeight * CurrentIntensity;
	}
	else
	{
		RotationEffect = FRotator::ZeroRotator;
	}
	
	// Distance and FOV effects are set by derived classes via ApplyPull/Push
	// They are already scaled, so just apply weight
	if (bApplyDistanceEffect)
	{
		DistanceEffect *= CurrentWeight * CurrentIntensity;
	}
	else
	{
		DistanceEffect = 0.0f;
	}
	
	if (bApplyFOVEffect)
	{
		FOVEffect *= CurrentWeight * CurrentIntensity;
	}
	else
	{
		FOVEffect = 0.0f;
	}
}

/**
 * GetReactionDirection - Calculate direction from player to source/target
 * 
 * Priority order:
 * 1. Cached SourceActor (if still valid)
 * 2. TriggerData.SourceLocation
 * 3. Context's lock-on target
 * 4. Cached TargetActor
 * 5. Cached ReactionDirection
 * 6. Character forward direction (fallback)
 * 
 * @param Context Current execution context
 * @return Normalized direction vector
 */
FVector UCameraModifier_ReactionBase::GetReactionDirection(const FStageExecutionContext& Context) const
{
	FVector PlayerLocation = Context.InputContext.CharacterLocation;
	
	// Priority 1: Use cached source actor if valid
	if (SourceActor.IsValid())
	{
		FVector SourceLocation = SourceActor->GetActorLocation();
		FVector Direction = (SourceLocation - PlayerLocation).GetSafeNormal();
		
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}
	
	// Priority 2: Use source location from trigger data
	if (!TriggerData.SourceLocation.IsNearlyZero())
	{
		FVector Direction = (TriggerData.SourceLocation - PlayerLocation).GetSafeNormal();
		
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}
	
	// Priority 3: Use lock-on target if available
	if (Context.InputContext.bHasTarget && Context.InputContext.TargetActor.IsValid())
	{
		FVector TargetLocation = Context.InputContext.TargetLocation;
		FVector Direction = (TargetLocation - PlayerLocation).GetSafeNormal();
		
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}
	
	// Priority 4: Use cached target actor
	if (TargetActor.IsValid())
	{
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector Direction = (TargetLocation - PlayerLocation).GetSafeNormal();
		
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}
	
	// Priority 5: Use cached reaction direction
	if (!ReactionDirection.IsNearlyZero())
	{
		return ReactionDirection;
	}
	
	// Priority 6 (Fallback): Use character forward direction
	return Context.InputContext.CharacterRotation.Vector();
}

/**
 * ApplyPullEffect - Camera pulls towards a direction
 * 
 * Used for parry effects where camera moves closer to the target.
 * Also reduces camera distance for zoom-in feel.
 * 
 * @param DeltaTime Frame delta time (for future use)
 * @param Direction Direction to pull towards (normalized)
 * @param PullStrength Interpolation strength multiplier
 * @param PullDistance Maximum pull distance in cm
 */
void UCameraModifier_ReactionBase::ApplyPullEffect(float DeltaTime, const FVector& Direction, float PullStrength, float PullDistance)
{
	// Set target offset in the pull direction
	TargetOffset = Direction * PullDistance;
	
	// Increase interpolation speed based on strength
	OffsetInterpSpeed = 15.0f * PullStrength;
	
	// Also reduce camera distance (zoom in effect)
	if (bApplyDistanceEffect)
	{
		DistanceEffect = -PullDistance * 0.5f;  // Negative = closer
	}
}

/**
 * ApplyPushEffect - Camera pushes away from a direction
 * 
 * Used for guard break, knockback where camera moves away from attacker.
 * Also increases camera distance for zoom-out feel.
 * 
 * @param DeltaTime Frame delta time (for future use)
 * @param Direction Direction of the attack/source (normalized)
 * @param PushStrength Interpolation strength multiplier
 * @param PushDistance Maximum push distance in cm
 */
void UCameraModifier_ReactionBase::ApplyPushEffect(float DeltaTime, const FVector& Direction, float PushStrength, float PushDistance)
{
	// Set target offset away from the push direction
	TargetOffset = -Direction * PushDistance;
	
	// Increase interpolation speed based on strength
	OffsetInterpSpeed = 15.0f * PushStrength;
	
	// Also increase camera distance (zoom out effect)
	if (bApplyDistanceEffect)
	{
		DistanceEffect = PushDistance * 0.3f;  // Positive = farther
	}
}

/**
 * ApplyDropEffect - Camera drops vertically
 * 
 * Used for knockdown effects where camera drops with the player.
 * 
 * @param DeltaTime Frame delta time (for future use)
 * @param DropDistance How far to drop (cm)
 * @param DropSpeed How quickly to drop
 */
void UCameraModifier_ReactionBase::ApplyDropEffect(float DeltaTime, float DropDistance, float DropSpeed)
{
	// Set target offset downward
	TargetOffset.Z = -DropDistance;
	
	// Use drop speed for interpolation
	OffsetInterpSpeed = DropSpeed;
}

/**
 * ApplyWobbleEffect - Camera wobbles for disorientation
 * 
 * Used for stagger effects where camera shakes in a disoriented manner.
 * Uses sine waves with slight randomization for organic feel.
 * 
 * @param DeltaTime Frame delta time
 * @param Intensity Wobble amplitude in degrees
 * @param Frequency Wobble speed (oscillations per second)
 */
void UCameraModifier_ReactionBase::ApplyWobbleEffect(float DeltaTime, float Intensity, float Frequency)
{
	// Generate wobble using sine waves with different frequencies per axis
	float TimeScaled = WobbleTime * Frequency * 2.0f * PI;
	
	// Different frequency multipliers for organic feel
	float PitchWobble = FMath::Sin(TimeScaled * 1.0f) * Intensity;
	float YawWobble = FMath::Sin(TimeScaled * 0.7f + 1.5f) * Intensity * 0.6f;
	float RollWobble = FMath::Sin(TimeScaled * 1.3f + 3.0f) * Intensity * 0.3f;
	
	// Apply decay based on time (wobble reduces over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	float Decay = 1.0f - NormalizedTime;
	
	// Set rotation offset
	CurrentRotationOffset.Pitch = PitchWobble * Decay;
	CurrentRotationOffset.Yaw = YawWobble * Decay;
	CurrentRotationOffset.Roll = RollWobble * Decay;
	
	// Enable rotation effect
	bApplyRotationEffect = true;
}

/**
 * UpdateOffsetInterpolation - Interpolate current offset towards target
 * 
 * During active phase: interpolates towards TargetOffset
 * During blend out or late active phase: interpolates back to zero
 * 
 * @param DeltaTime Frame delta time
 */
void UCameraModifier_ReactionBase::UpdateOffsetInterpolation(float DeltaTime)
{
	// Calculate normalized time in effect
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Determine target and speed based on current phase
	FVector DesiredOffset;
	float InterpSpeed;
	
	if (CurrentState == EModifierState::BlendingOut || NormalizedTime > 0.7f)
	{
		// Recovery phase: return to neutral
		DesiredOffset = FVector::ZeroVector;
		InterpSpeed = RecoveryInterpSpeed;
		
		// Also recover rotation
		CurrentRotationOffset = FMath::RInterpTo(
			CurrentRotationOffset,
			FRotator::ZeroRotator,
			DeltaTime,
			RecoveryInterpSpeed
		);
		
		// Also recover distance and FOV
		DistanceEffect = FMath::FInterpTo(DistanceEffect, 0.0f, DeltaTime, RecoveryInterpSpeed);
		FOVEffect = FMath::FInterpTo(FOVEffect, 0.0f, DeltaTime, RecoveryInterpSpeed);
	}
	else
	{
		// Active phase: move towards target
		DesiredOffset = TargetOffset;
		InterpSpeed = OffsetInterpSpeed;
	}
	
	// Interpolate position offset
	CurrentOffset = FMath::VInterpTo(CurrentOffset, DesiredOffset, DeltaTime, InterpSpeed);
}

//========================================
// R01: Parry Reaction Implementation
//========================================

/**
 * Constructor - Configure parry reaction parameters
 * 
 * Parry creates a satisfying "focus" effect:
 * - Camera pulls towards the parried enemy
 * - Slight zoom in (distance reduction)
 * - Slight FOV reduction for focus feel
 */
UCameraModifier_R01_Reaction_Parry::UCameraModifier_R01_Reaction_Parry()
	: PullDistance(20.0f)
	, ParryFOVReduction(4.0f)
	, DistanceReduction(30.0f)
{
	// Configure base reaction settings
	OffsetInterpSpeed = 20.0f;    // Fast pull
	RecoveryInterpSpeed = 10.0f;  // Medium recovery
	bApplyDistanceEffect = true;
	bApplyFOVEffect = true;
	bApplyRotationEffect = false;
}

void UCameraModifier_R01_Reaction_Parry::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Scale effect based on intensity
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.5f, 1.5f);
	
	// Calculate reaction direction (towards the parried enemy)
	// Direction will be updated in ComputeEffect
	
	UE_LOG(LogTemp, Verbose, TEXT("R01 Parry: Triggered with intensity %.2f"), CurrentIntensity);
}

void UCameraModifier_R01_Reaction_Parry::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Get current direction to target
	FVector Direction = GetReactionDirection(Context);
	ReactionDirection = Direction;
	
	// Calculate normalized time for effect curve
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Effect curve: quick pull in, hold briefly, then release
	float EffectCurve;
	if (NormalizedTime < 0.2f)
	{
		// Quick pull in (0-20%)
		EffectCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.2f, 2.0f);
	}
	else if (NormalizedTime < 0.5f)
	{
		// Hold at peak (20-50%)
		EffectCurve = 1.0f;
	}
	else
	{
		// Smooth release (50-100%)
		float ReleaseProgress = (NormalizedTime - 0.5f) / 0.5f;
		EffectCurve = FMath::InterpEaseIn(1.0f, 0.0f, ReleaseProgress, 1.5f);
	}
	
	// Apply pull effect
	float ScaledPullDistance = PullDistance * EffectCurve * CurrentIntensity;
	ApplyPullEffect(DeltaTime, Direction, 1.0f, ScaledPullDistance);
	
	// Apply FOV reduction
	FOVEffect = -ParryFOVReduction * EffectCurve;
	
	// Override distance effect with our scaled version
	DistanceEffect = -DistanceReduction * EffectCurve;
	
	// Call base to apply weight and interpolation
	Super::ComputeEffect(DeltaTime, Context);
}


//========================================
// R02: Perfect Parry Reaction Implementation
//========================================

/**
 * Constructor - Configure perfect parry reaction parameters
 * 
 * Perfect parry is an enhanced version of normal parry:
 * - Stronger camera pull
 * - More noticeable zoom
 * - Time dilation visual hint (not actual game time change)
 * - Cannot be interrupted (completes fully)
 */
UCameraModifier_R02_Reaction_PerfectParry::UCameraModifier_R02_Reaction_PerfectParry()
	: PullDistance(35.0f)
	, PerfectParryFOVReduction(7.0f)
	, DistanceReduction(50.0f)
	, TimeDilationHint(0.3f)
{
	// Configure base reaction settings
	OffsetInterpSpeed = 25.0f;    // Very fast pull
	RecoveryInterpSpeed = 8.0f;   // Slower recovery (savor the moment)
	bApplyDistanceEffect = true;
	bApplyFOVEffect = true;
	bApplyRotationEffect = false;
}

void UCameraModifier_R02_Reaction_PerfectParry::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	UE_LOG(LogTemp, Verbose, TEXT("R02 PerfectParry: Triggered with intensity %.2f"), CurrentIntensity);
}

void UCameraModifier_R02_Reaction_PerfectParry::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Get current direction to target
	FVector Direction = GetReactionDirection(Context);
	ReactionDirection = Direction;
	
	// Calculate normalized time for effect curve
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Effect curve: very quick pull, longer hold, smooth release
	float EffectCurve;
	if (NormalizedTime < 0.15f)
	{
		// Very quick pull in (0-15%)
		EffectCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.15f, 2.5f);
	}
	else if (NormalizedTime < 0.6f)
	{
		// Extended hold at peak (15-60%)
		EffectCurve = 1.0f;
	}
	else
	{
		// Smooth release (60-100%)
		float ReleaseProgress = (NormalizedTime - 0.6f) / 0.4f;
		EffectCurve = FMath::InterpEaseInOut(1.0f, 0.0f, ReleaseProgress, 2.0f);
	}
	
	// Apply stronger pull effect
	float ScaledPullDistance = PullDistance * EffectCurve * CurrentIntensity;
	ApplyPullEffect(DeltaTime, Direction, 1.2f, ScaledPullDistance);
	
	// Apply stronger FOV reduction
	FOVEffect = -PerfectParryFOVReduction * EffectCurve;
	
	// Override distance effect
	DistanceEffect = -DistanceReduction * EffectCurve;
	
	// Time dilation hint during hold phase (visual effect only)
	if (NormalizedTime >= 0.1f && NormalizedTime < 0.5f)
	{
		TimeDilationEffect = 1.0f - (TimeDilationHint * EffectCurve);
	}
	else
	{
		TimeDilationEffect = 1.0f;
	}
	
	// Subtle vignette during peak effect
	VignetteEffect = 0.15f * EffectCurve;
	
	// Call base to apply weight and interpolation
	Super::ComputeEffect(DeltaTime, Context);
}


//========================================
// R03: Stagger Reaction Implementation
//========================================

/**
 * Constructor - Configure stagger reaction parameters
 * 
 * Stagger creates a disorienting effect:
 * - Camera wobbles/shakes
 * - Slight push back from attacker
 * - FOV increase for vulnerability feel
 */
UCameraModifier_R03_Reaction_Stagger::UCameraModifier_R03_Reaction_Stagger()
	: WobbleIntensity(3.0f)
	, WobbleFrequency(12.0f)
	, PushDistance(15.0f)
	, DisorientationFOVIncrease(5.0f)
	, WobbleSeed(0.0f)
{
	// Configure base reaction settings
	OffsetInterpSpeed = 18.0f;
	RecoveryInterpSpeed = 6.0f;   // Slower recovery (dazed)
	bApplyDistanceEffect = true;
	bApplyFOVEffect = true;
	bApplyRotationEffect = true;  // Enable rotation for wobble
}

void UCameraModifier_R03_Reaction_Stagger::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Generate random seed for wobble variation
	WobbleSeed = FMath::FRand() * 1000.0f;
	
	// Scale wobble based on intensity (heavier hits = more wobble)
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.5f, 2.0f);
	WobbleIntensity = 3.0f * IntensityScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("R03 Stagger: Triggered with intensity %.2f"), CurrentIntensity);
}

void UCameraModifier_R03_Reaction_Stagger::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Get direction from attacker
	FVector Direction = GetReactionDirection(Context);
	ReactionDirection = Direction;
	
	// Calculate normalized time
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Effect intensity curve: peak early, then decay
	float EffectCurve;
	if (NormalizedTime < 0.1f)
	{
		// Quick ramp up (0-10%)
		EffectCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.1f, 2.0f);
	}
	else
	{
		// Gradual decay (10-100%)
		float DecayProgress = (NormalizedTime - 0.1f) / 0.9f;
		EffectCurve = FMath::InterpEaseIn(1.0f, 0.0f, DecayProgress, 1.5f);
	}
	
	// Apply push effect (away from attacker)
	float ScaledPushDistance = PushDistance * EffectCurve * CurrentIntensity;
	ApplyPushEffect(DeltaTime, Direction, 1.0f, ScaledPushDistance);
	
	// Apply wobble effect
	float ScaledWobbleIntensity = WobbleIntensity * EffectCurve;
	ApplyWobbleEffect(DeltaTime, ScaledWobbleIntensity, WobbleFrequency);
	
	// Apply FOV increase (vulnerability/disorientation)
	FOVEffect = DisorientationFOVIncrease * EffectCurve;
	
	// Slight distance increase
	DistanceEffect = 10.0f * EffectCurve;
	
	// Call base to apply weight and interpolation
	Super::ComputeEffect(DeltaTime, Context);
}


//========================================
// R04: Knockback Reaction Implementation
//========================================

/**
 * Constructor - Configure knockback reaction parameters
 * 
 * Knockback follows the player's forced movement: 
 * - Camera position lags behind player
 * - Distance increases (zoom out)
 * - Slight tilt in knockback direction
 */
UCameraModifier_R04_Reaction_Knockback::UCameraModifier_R04_Reaction_Knockback()
	: KnockbackEmphasis(1.3f)
	, DistanceIncrease(35.0f)
	, TiltAngle(5.0f)
{
	// Configure base reaction settings
	OffsetInterpSpeed = 20.0f;   // Fast response to knockback
	RecoveryInterpSpeed = 8.0f;  // Medium recovery
	bApplyDistanceEffect = true;
	bApplyFOVEffect = true;
	bApplyRotationEffect = true;  // Enable tilt
}

void UCameraModifier_R04_Reaction_Knockback::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Scale effect based on knockback intensity
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.5f, 2.0f);
	KnockbackEmphasis = 1.3f * IntensityScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("R04 Knockback: Triggered with intensity %.2f"), CurrentIntensity);
}

void UCameraModifier_R04_Reaction_Knockback::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Get knockback direction (from attacker towards player = knockback direction)
	FVector AttackerDirection = GetReactionDirection(Context);
	FVector KnockbackDir = -AttackerDirection;  // Opposite of attacker direction
	KnockbackDir.Z = 0.0f;  // Keep horizontal
	if (!KnockbackDir.IsNearlyZero())
	{
		KnockbackDir.Normalize();
	}
	ReactionDirection = KnockbackDir;
	
	// Calculate normalized time
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Effect curve: quick reaction, gradual recovery
	float EffectCurve;
	if (NormalizedTime < 0.15f)
	{
		// Quick onset (0-15%)
		EffectCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.15f, 2.0f);
	}
	else
	{
		// Gradual recovery (15-100%)
		float RecoveryProgress = (NormalizedTime - 0.15f) / 0.85f;
		EffectCurve = FMath::InterpEaseOut(1.0f, 0.0f, RecoveryProgress, 1.5f);
	}
	
	// Calculate position offset (camera lags behind knockback)
	float OffsetDistance = 25.0f * KnockbackEmphasis * EffectCurve;
	TargetOffset = -KnockbackDir * OffsetDistance;  // Opposite direction = lag behind
	
	// Apply distance increase (zoom out)
	DistanceEffect = DistanceIncrease * EffectCurve;
	
	// Apply slight FOV increase for impact feel
	FOVEffect = 4.0f * EffectCurve;
	
	// Apply tilt in knockback direction (roll effect)
	float TiltAmount = TiltAngle * EffectCurve;
	// Convert knockback direction to roll: left = negative roll, right = positive roll
	float RollDirection = FVector::CrossProduct(FVector::ForwardVector, KnockbackDir).Z;
	CurrentRotationOffset.Roll = TiltAmount * RollDirection;
	CurrentRotationOffset.Pitch = -TiltAmount * 0.3f;  // Slight pitch back
	CurrentRotationOffset.Yaw = 0.0f;
	
	// Brief shake at start of knockback
	if (NormalizedTime < 0.1f)
	{
		float ShakeIntensity = (1.0f - NormalizedTime / 0.1f) * 2.0f;
		CurrentRotationOffset.Pitch += FMath::PerlinNoise1D(WobbleTime * 25.0f) * ShakeIntensity;
		CurrentRotationOffset.Yaw += FMath::PerlinNoise1D(WobbleTime * 25.0f + 100.0f) * ShakeIntensity * 0.5f;
	}
	
	// Call base to apply weight and interpolation
	Super::ComputeEffect(DeltaTime, Context);
}


//========================================
// R05: Knockdown Reaction Implementation
//========================================

/**
 * Constructor - Configure knockdown reaction parameters
 * 
 * Knockdown simulates player falling to ground:
 * - Camera drops vertically
 * - Wobble during fall
 * - Distance and FOV increase (vulnerability)
 */
UCameraModifier_R05_Reaction_Knockdown::UCameraModifier_R05_Reaction_Knockdown()
	: DropDistance(40.0f)
	, DropSpeed(15.0f)
	, FallWobbleIntensity(2.5f)
	, DistanceIncrease(50.0f)
	, FOVIncrease(8.0f)
{
	// Configure base reaction settings
	OffsetInterpSpeed = 12.0f;   // Medium speed for falling feel
	RecoveryInterpSpeed = 5.0f;  // Slow recovery (getting up takes time)
	bApplyDistanceEffect = true;
	bApplyFOVEffect = true;
	bApplyRotationEffect = true;  // Enable wobble
}

void UCameraModifier_R05_Reaction_Knockdown::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Scale effect based on knockdown severity
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.5f, 1.5f);
	DropDistance = 40.0f * IntensityScale;
	FallWobbleIntensity = 2.5f * IntensityScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("R05 Knockdown: Triggered with intensity %.2f"), CurrentIntensity);
}

void UCameraModifier_R05_Reaction_Knockdown::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Drop curve: quick fall, impact, slow recovery
	float DropCurve;
	float WobbleCurve;
	if (NormalizedTime < 0.2f)
	{
		// Falling phase (0-20%): accelerating drop
		float FallProgress = NormalizedTime / 0.2f;
		DropCurve = FMath::InterpEaseIn(0.0f, 1.0f, FallProgress, 2.0f);
		WobbleCurve = FallProgress;  // Wobble increases during fall
	}
	else if (NormalizedTime < 0.3f)
	{
		// Impact phase (20-30%): hold at bottom with strong wobble
		DropCurve = 1.0f;
		float ImpactProgress = (NormalizedTime - 0.2f) / 0.1f;
		WobbleCurve = 1.0f - ImpactProgress * 0.3f;  // Wobble peaks then starts to reduce
	}
	else
	{
		// Recovery phase (30-100%): slowly rise back
		float RecoveryProgress = (NormalizedTime - 0.3f) / 0.7f;
		DropCurve = FMath::InterpEaseOut(1.0f, 0.0f, RecoveryProgress, 1.5f);
		WobbleCurve = FMath::InterpEaseOut(0.7f, 0.0f, RecoveryProgress, 2.0f);
	}
	
	// Apply drop effect (vertical offset)
	ApplyDropEffect(DeltaTime, DropDistance * DropCurve, DropSpeed);
	
	// Apply wobble during fall and impact
	if (WobbleCurve > 0.1f)
	{
		ApplyWobbleEffect(DeltaTime, FallWobbleIntensity * WobbleCurve, 10.0f);
	}
	
	// Apply distance increase (camera pulls back)
	DistanceEffect = DistanceIncrease * DropCurve;
	
	// Apply FOV increase (vulnerability feel)
	FOVEffect = FOVIncrease * DropCurve;
	
	// Slight vignette at impact
	if (NormalizedTime >= 0.15f && NormalizedTime < 0.4f)
	{
		float VignetteProgress = (NormalizedTime < 0.25f) 
			? (NormalizedTime - 0.15f) / 0.1f 
			: 1.0f - (NormalizedTime - 0.25f) / 0.15f;
		VignetteEffect = 0.2f * FMath::Clamp(VignetteProgress, 0.0f, 1.0f);
	}
	else
	{
		VignetteEffect = 0.0f;
	}
	
	// Call base to apply weight and interpolation
	Super::ComputeEffect(DeltaTime, Context);
}


//========================================
// R06: Guard Break Reaction Implementation
//========================================

/**
 * Constructor - Configure guard break reaction parameters
 * 
 * Guard break is a dramatic vulnerability effect:
 * - Strong push away from attacker
 * - Large FOV punch (vulnerability)
 * - Screen shake
 * - Time dilation hint
 */
UCameraModifier_R06_Reaction_GuardBreak::UCameraModifier_R06_Reaction_GuardBreak()
	: PushAwayDistance(35.0f)
	, GuardBreakFOVPunch(10.0f)
	, DistanceIncrease(60.0f)
	, ShakeIntensity(4.0f)
	, TimeDilationHint(0.4f)
{
	// Configure base reaction settings
	OffsetInterpSpeed = 30.0f;   // Very fast push
	RecoveryInterpSpeed = 6.0f;  // Slow recovery (vulnerable state)
	bApplyDistanceEffect = true;
	bApplyFOVEffect = true;
	bApplyRotationEffect = true;  // Enable shake
}

void UCameraModifier_R06_Reaction_GuardBreak::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Guard break is always dramatic, intensity affects magnitude
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.8f, 1.5f);
	PushAwayDistance = 35.0f * IntensityScale;
	GuardBreakFOVPunch = 10.0f * IntensityScale;
	ShakeIntensity = 4.0f * IntensityScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("R06 GuardBreak: Triggered with intensity %.2f"), CurrentIntensity);
}

void UCameraModifier_R06_Reaction_GuardBreak::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Get direction from attacker
	FVector AttackerDirection = GetReactionDirection(Context);
	ReactionDirection = AttackerDirection;
	
	// Calculate normalized time
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Effect curve: instant punch, hold briefly, slow recovery
	float EffectCurve;
	float ShakeCurve;
	if (NormalizedTime < 0.1f)
	{
		// Instant punch (0-10%)
		EffectCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.1f, 3.0f);
		ShakeCurve = 1.0f;
	}
	else if (NormalizedTime < 0.3f)
	{
		// Hold peak (10-30%)
		EffectCurve = 1.0f;
		ShakeCurve = 1.0f - (NormalizedTime - 0.1f) / 0.2f;  // Shake fades
	}
	else
	{
		// Slow recovery (30-100%)
		float RecoveryProgress = (NormalizedTime - 0.3f) / 0.7f;
		EffectCurve = FMath::InterpEaseIn(1.0f, 0.0f, RecoveryProgress, 1.5f);
		ShakeCurve = 0.0f;
	}
	
	// Apply strong push effect (away from attacker)
	ApplyPushEffect(DeltaTime, AttackerDirection, 1.5f, PushAwayDistance * EffectCurve);
	
	// Apply large FOV punch
	FOVEffect = GuardBreakFOVPunch * EffectCurve;
	
	// Apply distance increase
	DistanceEffect = DistanceIncrease * EffectCurve;
	
	// Apply screen shake during initial impact
	if (ShakeCurve > 0.0f)
	{
		float ScaledShake = ShakeIntensity * ShakeCurve;
		CurrentRotationOffset.Pitch = FMath::PerlinNoise1D(WobbleTime * 30.0f) * ScaledShake;
		CurrentRotationOffset.Yaw = FMath::PerlinNoise1D(WobbleTime * 30.0f + 100.0f) * ScaledShake * 0.7f;
		CurrentRotationOffset.Roll = FMath::PerlinNoise1D(WobbleTime * 30.0f + 200.0f) * ScaledShake * 0.3f;
	}
	
	// Time dilation hint during peak effect
	if (NormalizedTime >= 0.05f && NormalizedTime < 0.25f)
	{
		TimeDilationEffect = 1.0f - (TimeDilationHint * EffectCurve);
	}
	else
	{
		TimeDilationEffect = 1.0f;
	}
	
	// Strong vignette for vulnerability feel
	VignetteEffect = 0.3f * EffectCurve;
	
	// Call base to apply weight and interpolation
	Super::ComputeEffect(DeltaTime, Context);
}

