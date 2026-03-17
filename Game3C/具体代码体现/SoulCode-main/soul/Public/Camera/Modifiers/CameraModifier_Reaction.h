// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierBase.h"
#include "CameraModifier_Reaction.generated.h"

/**
 * Reaction Modifier Declarations (R01-R06)
 * 
 * This file contains the 6 Reaction modifiers for the camera system.
 * Reaction modifiers provide combat feedback effects triggered by
 * defensive actions and hit reactions.
 * 
 * All reaction modifiers inherit from UCameraModifier_ReactionBase which provides
 * common functionality for directional camera effects and smooth interpolation.
 * 
 * Reaction Types:
 * - R01: Parry - Camera pull towards parried enemy
 * - R02: Perfect Parry - Stronger pull with time dilation hint
 * - R03: Stagger - Disorienting wobble effect
 * - R04: Knockback - Camera follows knockback movement
 * - R05: Knockdown - Camera drops with player
 * - R06: Guard Break - Dramatic push and FOV punch
 * 
 * Key Characteristics:
 * - Directional effects based on attacker position
 * - Higher priority than Shake modifiers (120-145)
 * - Medium duration (0.3-0.8 seconds)
 * - Supports position, rotation, FOV, and distance effects
 */


//========================================
// Reaction Base Class
//========================================

/**
 * UCameraModifier_ReactionBase
 * 
 * Abstract base class for all reaction modifiers.
 * Provides common functionality for directional camera effects.
 * 
 * Features:
 * - Directional effect calculation (towards/away from source)
 * - Pull effect (camera moves towards target)
 * - Push effect (camera moves away from source)
 * - Smooth interpolation for offset changes
 * - Recovery interpolation back to neutral
 * 
 * Derived classes configure the specific reaction parameters
 * and may override ComputeEffect for specialized behavior.
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraModifier_ReactionBase : public UCameraModifierBase
{
	GENERATED_BODY()

public:
	/** Constructor - sets default reaction parameters */
	UCameraModifier_ReactionBase();

	//========================================
	// Modifier Identity
	//========================================

	/** All reaction modifiers belong to the Reaction category */
	virtual EModifierCategory GetModifierCategory() const override { return EModifierCategory::Reaction; }

	/** Reaction modifiers use the Reaction type for output classification */
	virtual ECameraModifierType GetModifierType() const override { return ECameraModifierType::Reaction; }

protected:
	//========================================
	// Effect Computation
	//========================================

	/**
	 * Compute reaction effect values
	 * Calculates position offset and other effects based on reaction type
	 * @param DeltaTime Time since last frame
	 * @param Context Current camera execution context
	 */
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	/**
	 * Called when modifier is triggered
	 * Caches reaction direction and initializes offset
	 * @param InTriggerData Trigger configuration
	 */
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;

	//========================================
	// Reaction Effect Helpers
	//========================================

	/**
	 * Calculate direction from player to source/target actor
	 * @param Context Current execution context
	 * @return Normalized direction vector (zero if no valid target)
	 */
	FVector GetReactionDirection(const FStageExecutionContext& Context) const;

	/**
	 * Apply a camera pull effect towards a direction
	 * Camera moves closer to the target
	 * @param DeltaTime Frame delta time
	 * @param Direction Direction to pull towards (normalized)
	 * @param PullStrength How quickly to reach target offset
	 * @param PullDistance Maximum pull distance in cm
	 */
	void ApplyPullEffect(float DeltaTime, const FVector& Direction, float PullStrength, float PullDistance);

	/**
	 * Apply a camera push effect away from a direction
	 * Camera moves away from the source
	 * @param DeltaTime Frame delta time
	 * @param Direction Direction to push away from (normalized)
	 * @param PushStrength How quickly to reach target offset
	 * @param PushDistance Maximum push distance in cm
	 */
	void ApplyPushEffect(float DeltaTime, const FVector& Direction, float PushStrength, float PushDistance);

	/**
	 * Apply a vertical drop effect (for knockdown)
	 * @param DeltaTime Frame delta time
	 * @param DropDistance How far to drop
	 * @param DropSpeed How quickly to drop
	 */
	void ApplyDropEffect(float DeltaTime, float DropDistance, float DropSpeed);

	/**
	 * Apply rotation wobble effect (for stagger/disorientation)
	 * @param DeltaTime Frame delta time
	 * @param Intensity Wobble amplitude in degrees
	 * @param Frequency Wobble speed
	 */
	void ApplyWobbleEffect(float DeltaTime, float Intensity, float Frequency);

	/**
	 * Interpolate current offset towards target (or back to zero during recovery)
	 * @param DeltaTime Frame delta time
	 */
	void UpdateOffsetInterpolation(float DeltaTime);

	//========================================
	// Reaction Parameters (Configurable)
	//========================================

	/** Offset interpolation speed (how fast offset reaches target) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Interpolation", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float OffsetInterpSpeed;

	/** Recovery interpolation speed (how fast offset returns to zero) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Interpolation", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float RecoveryInterpSpeed;

	/** Whether to include distance effect (camera arm length change) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Options")
	bool bApplyDistanceEffect;

	/** Whether to include FOV effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Options")
	bool bApplyFOVEffect;

	/** Whether to include rotation effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Options")
	bool bApplyRotationEffect;

	//========================================
	// Runtime State
	//========================================

	/** Reaction direction (normalized, from player to source/target) */
	UPROPERTY(BlueprintReadOnly, Category = "Reaction|State")
	FVector ReactionDirection;

	/** Current position offset (interpolating towards TargetOffset) */
	UPROPERTY(BlueprintReadOnly, Category = "Reaction|State")
	FVector CurrentOffset;

	/** Target position offset (set by Pull/Push effects) */
	UPROPERTY(BlueprintReadOnly, Category = "Reaction|State")
	FVector TargetOffset;

	/** Current rotation offset (for wobble effects) */
	UPROPERTY(BlueprintReadOnly, Category = "Reaction|State")
	FRotator CurrentRotationOffset;

	/** Wobble time accumulator */
	UPROPERTY(BlueprintReadOnly, Category = "Reaction|State")
	float WobbleTime;

	/** Cached source actor from trigger */
	UPROPERTY(BlueprintReadOnly, Category = "Reaction|State")
	TWeakObjectPtr<AActor> SourceActor;

	/** Cached target actor from trigger */
	UPROPERTY(BlueprintReadOnly, Category = "Reaction|State")
	TWeakObjectPtr<AActor> TargetActor;
};


//========================================
// R01: Parry Reaction
//========================================

/**
 * UCameraModifier_R01_Reaction_Parry
 * 
 * R01: Parry - Camera pull towards parried enemy
 * 
 * Triggered on successful parry. Camera briefly pulls towards
 * the parried enemy, creating a satisfying focus effect.
 * 
 * Effects:
 * - Position: Pull towards enemy
 * - Distance: Slight reduction (zoom in)
 * - FOV: Slight reduction for focus
 * 
 * Typical Parameters:
 * - Duration: 0.4 seconds
 * - Pull Distance: 15-25 cm
 * - FOV Reduction: 3-5 degrees
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_R01_Reaction_Parry : public UCameraModifier_ReactionBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures parry reaction parameters */
	UCameraModifier_R01_Reaction_Parry();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_R01_Reaction_Parry; }
	virtual FString GetModifierDescription() const override { return TEXT("Camera pull towards parried enemy"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.40f; }
	virtual float GetBlendInTime() const override { return 0.05f; }
	virtual float GetBlendOutTime() const override { return 0.25f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 120; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Parry-Specific Parameters
	//========================================

	/** Distance to pull camera towards target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Parry", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float PullDistance;

	/** FOV reduction on parry (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Parry", meta = (ClampMin = "0.0", ClampMax = "15.0"))
	float ParryFOVReduction;

	/** Distance reduction (zoom in) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Parry", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DistanceReduction;
};


//========================================
// R02: Perfect Parry Reaction
//========================================

/**
 * UCameraModifier_R02_Reaction_PerfectParry
 * 
 * R02: Perfect Parry - Enhanced parry with time dilation hint
 * 
 * Triggered on frame-perfect parry timing. Stronger effect than
 * normal parry with a subtle time dilation feel (visual only).
 * 
 * Effects:
 * - Position: Strong pull towards enemy
 * - Distance: Noticeable reduction
 * - FOV: Noticeable reduction
 * - Time Dilation: Visual hint (not actual game time change)
 * 
 * Typical Parameters:
 * - Duration: 0.5 seconds
 * - Pull Distance: 25-40 cm
 * - FOV Reduction: 5-8 degrees
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_R02_Reaction_PerfectParry : public UCameraModifier_ReactionBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures perfect parry reaction parameters */
	UCameraModifier_R02_Reaction_PerfectParry();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_R02_Reaction_PerfectParry; }
	virtual FString GetModifierDescription() const override { return TEXT("Enhanced camera effect for perfect parry"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.50f; }
	virtual float GetBlendInTime() const override { return 0.03f; }
	virtual float GetBlendOutTime() const override { return 0.30f; }

	//========================================
	// Priority & Stacking
	//========================================

	/** Higher priority than normal parry */
	virtual int32 GetPriority() const override { return 125; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return false; } // Perfect parry completes

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Perfect Parry-Specific Parameters
	//========================================

	/** Distance to pull camera towards target (stronger than normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|PerfectParry", meta = (ClampMin = "10.0", ClampMax = "80.0"))
	float PullDistance;

	/** FOV reduction on perfect parry (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|PerfectParry", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float PerfectParryFOVReduction;

	/** Distance reduction (zoom in) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|PerfectParry", meta = (ClampMin = "0.0", ClampMax = "150.0"))
	float DistanceReduction;

	/** Time dilation visual hint (affects motion blur, not actual time) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|PerfectParry", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeDilationHint;
};


//========================================
// R03: Stagger Reaction
//========================================

/**
 * UCameraModifier_R03_Reaction_Stagger
 * 
 * R03: Stagger - Disorienting wobble effect when player is staggered
 * 
 * Triggered when player enters stagger state from heavy hit.
 * Camera wobbles to simulate disorientation.
 * 
 * Effects:
 * - Rotation: Wobble/shake effect
 * - Position: Slight push back
 * - FOV: Slight increase (disorientation feel)
 * 
 * Typical Parameters:
 * - Duration: 0.6 seconds
 * - Wobble Intensity: 2-4 degrees
 * - Push Distance: 10-20 cm
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_R03_Reaction_Stagger : public UCameraModifier_ReactionBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures stagger reaction parameters */
	UCameraModifier_R03_Reaction_Stagger();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_R03_Reaction_Stagger; }
	virtual FString GetModifierDescription() const override { return TEXT("Disorienting camera wobble when staggered"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.60f; }
	virtual float GetBlendInTime() const override { return 0.05f; }
	virtual float GetBlendOutTime() const override { return 0.35f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 130; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Stagger-Specific Parameters
	//========================================

	/** Rotation wobble intensity (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Stagger", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float WobbleIntensity;

	/** Wobble frequency (oscillations per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Stagger", meta = (ClampMin = "5.0", ClampMax = "30.0"))
	float WobbleFrequency;

	/** Push back distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Stagger", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float PushDistance;

	/** FOV increase for disorientation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Stagger", meta = (ClampMin = "0.0", ClampMax = "15.0"))
	float DisorientationFOVIncrease;

	/** Random seed for wobble variation */
	float WobbleSeed;
};


//========================================
// R04: Knockback Reaction
//========================================

/**
 * UCameraModifier_R04_Reaction_Knockback
 * 
 * R04: Knockback - Camera follows player knockback movement
 * 
 * Triggered when player is knocked back by attack.
 * Camera follows the knockback direction with emphasis.
 * 
 * Effects:
 * - Position: Follows knockback direction
 * - Distance: Slight increase (zoom out)
 * - Rotation: Slight tilt in knockback direction
 * 
 * Typical Parameters:
 * - Duration: 0.5 seconds
 * - Knockback Emphasis: 1.2-1.5x
 * - Distance Increase: 20-40 cm
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_R04_Reaction_Knockback : public UCameraModifier_ReactionBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures knockback reaction parameters */
	UCameraModifier_R04_Reaction_Knockback();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_R04_Reaction_Knockback; }
	virtual FString GetModifierDescription() const override { return TEXT("Camera follows player knockback movement"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.50f; }
	virtual float GetBlendInTime() const override { return 0.03f; }
	virtual float GetBlendOutTime() const override { return 0.30f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 135; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Knockback-Specific Parameters
	//========================================

	/** Knockback direction emphasis multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockback", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float KnockbackEmphasis;

	/** Distance increase during knockback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockback", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DistanceIncrease;

	/** Rotation tilt in knockback direction (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockback", meta = (ClampMin = "0.0", ClampMax = "15.0"))
	float TiltAngle;
};


//========================================
// R05: Knockdown Reaction
//========================================

/**
 * UCameraModifier_R05_Reaction_Knockdown
 * 
 * R05: Knockdown - Camera drops with player when knocked down
 * 
 * Triggered when player is knocked down (falls to ground).
 * Camera drops and wobbles to match the fall.
 * 
 * Effects:
 * - Position: Vertical drop
 * - Rotation: Wobble during fall
 * - FOV: Slight increase
 * - Distance: Slight increase
 * 
 * Typical Parameters:
 * - Duration: 0.8 seconds
 * - Drop Distance: 30-50 cm
 * - Wobble during drop
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_R05_Reaction_Knockdown : public UCameraModifier_ReactionBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures knockdown reaction parameters */
	UCameraModifier_R05_Reaction_Knockdown();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_R05_Reaction_Knockdown; }
	virtual FString GetModifierDescription() const override { return TEXT("Camera drops with player when knocked down"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.80f; }
	virtual float GetBlendInTime() const override { return 0.05f; }
	virtual float GetBlendOutTime() const override { return 0.40f; }

	//========================================
	// Priority & Stacking
	//========================================

	/** High priority - knockdown is dramatic */
	virtual int32 GetPriority() const override { return 140; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return false; } // Complete the knockdown

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Knockdown-Specific Parameters
	//========================================

	/** Vertical drop distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockdown", meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float DropDistance;

	/** Drop speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockdown", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float DropSpeed;

	/** Wobble during fall */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockdown", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float FallWobbleIntensity;

	/** Distance increase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockdown", meta = (ClampMin = "0.0", ClampMax = "80.0"))
	float DistanceIncrease;

	/** FOV increase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockdown", meta = (ClampMin = "0.0", ClampMax = "15.0"))
	float FOVIncrease;
};


//========================================
// R06: Guard Break Reaction
//========================================

/**
 * UCameraModifier_R06_Reaction_GuardBreak
 * 
 * R06: Guard Break - Dramatic effect when guard is broken
 * 
 * Triggered when player's guard is broken (stamina depleted while blocking).
 * Dramatic camera effect to emphasize vulnerability.
 * 
 * Effects:
 * - Position: Push away from attacker
 * - FOV: Sharp increase (vulnerability feel)
 * - Distance: Increase (zoom out)
 * - Slight screen shake
 * 
 * Typical Parameters:
 * - Duration: 0.5 seconds
 * - Push Distance: 25-40 cm
 * - FOV Punch: 8-12 degrees
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_R06_Reaction_GuardBreak : public UCameraModifier_ReactionBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures guard break reaction parameters */
	UCameraModifier_R06_Reaction_GuardBreak();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_R06_Reaction_GuardBreak; }
	virtual FString GetModifierDescription() const override { return TEXT("Dramatic camera effect when guard is broken"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.50f; }
	virtual float GetBlendInTime() const override { return 0.02f; }
	virtual float GetBlendOutTime() const override { return 0.30f; }

	//========================================
	// Priority & Stacking
	//========================================

	/** Highest reaction priority - guard break is critical */
	virtual int32 GetPriority() const override { return 145; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return false; } // Complete the effect

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Guard Break-Specific Parameters
	//========================================

	/** Push away distance from attacker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|GuardBreak", meta = (ClampMin = "10.0", ClampMax = "80.0"))
	float PushAwayDistance;

	/** FOV punch on guard break (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|GuardBreak", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float GuardBreakFOVPunch;

	/** Distance increase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|GuardBreak", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DistanceIncrease;

	/** Screen shake intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|GuardBreak", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float ShakeIntensity;

	/** Time dilation visual hint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|GuardBreak", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeDilationHint;
};
