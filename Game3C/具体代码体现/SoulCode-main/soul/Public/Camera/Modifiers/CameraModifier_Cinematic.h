// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierBase.h"
#include "CameraModifier_Cinematic.generated.h"

/**
 * Cinematic Modifier Declarations (C01-C05)
 * 
 * This file contains the 5 Cinematic modifiers for the camera system.
 * Cinematic modifiers provide dramatic camera takeover effects for
 * special gameplay moments like executions, backstabs, and boss intros.
 * 
 * All cinematic modifiers inherit from UCameraModifier_CinematicBase which provides
 * common functionality for camera override and smooth transitions.
 * 
 * Key Difference from Other Modifiers:
 * - Override mode: Takes full control of camera (IsOverrideModifier = true)
 * - Longer duration: Typically 1-5 seconds
 * - Highest priority: 200-300 range
 * - Sets absolute camera transform, not offsets
 * 
 * Comparison with Other Modifier Types:
 * 
 * Shake/Reaction Modifiers:
 * - Additive effects on current camera
 * - Only affect offset/shake
 * - Short duration (0.1-0.8 seconds)
 * 
 * Cinematic Modifiers:
 * - Complete camera takeover
 * - Sets absolute position/rotation
 * - Longer duration (1-5 seconds)
 * - IsOverrideModifier() = true
 * - Highest priority (200-300)
 * 
 * Cinematic Types:
 * - C01: Execution - Visceral attack / finishing blow camera
 * - C02: Backstab - Stealth kill camera from behind
 * - C03: Riposte - Parry counter-attack camera
 * - C04: BossIntro - Boss reveal sweeping shot
 * - C05: BossPhase - Boss phase transition camera
 * 
 * Usage:
 * @code
 * // Trigger execution camera when performing visceral attack
 * FModifierTriggerData TriggerData;
 * TriggerData.TargetActor = VictimEnemy;
 * TriggerData.Intensity = 1.0f;
 * ModifierManager->TriggerModifier(ECameraModifierID::Modifier_C01_Cinematic_Execution, TriggerData);
 * @endcode
 */


//========================================
// Cinematic Base Class
//========================================

/**
 * UCameraModifier_CinematicBase
 * 
 * Abstract base class for all cinematic modifiers.
 * Provides camera override functionality for dramatic shots.
 * 
 * Features:
 * - Full camera takeover (override mode)
 * - Smooth blend in/out transitions
 * - Look-at target calculation
 * - Cinematic FOV control
 * - Letterbox support (optional)
 * - Player input blocking during cinematic
 * 
 * Override Mode:
 * When IsOverrideModifier() returns true, Stage 4 (ModifierApply) will
 * use this modifier's OverrideTransform directly instead of blending
 * with other effects. Only one override modifier can be active at a time.
 * 
 * Derived classes configure specific camera behaviors:
 * - Position calculation (orbit, fixed, sweep, etc.)
 * - Rotation calculation (look-at, fixed angle, etc.)
 * - Timing curves for dramatic effect
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraModifier_CinematicBase : public UCameraModifierBase
{
	GENERATED_BODY()

public:
	/** Constructor - sets default cinematic parameters */
	UCameraModifier_CinematicBase();

	//========================================
	// Modifier Identity
	//========================================

	/** All cinematic modifiers belong to the Cinematic category */
	virtual EModifierCategory GetModifierCategory() const override { return EModifierCategory::Cinematic; }

	/** Cinematic modifiers use the Cinematic type for output classification */
	virtual ECameraModifierType GetModifierType() const override { return ECameraModifierType::Cinematic; }

	/** Cinematics take full control of camera */
	virtual bool IsOverrideModifier() const override { return true; }

	/** Cinematics cannot be interrupted by lower priority modifiers */
	virtual bool CanBeInterrupted() const override { return false; }

	/** Cinematics do not stack */
	virtual bool CanStack() const override { return false; }

protected:
	//========================================
	// Effect Computation
	//========================================

	/**
	 * Compute cinematic camera transform
	 * Sets OverrideTransform instead of offset effects
	 * @param DeltaTime Time since last frame
	 * @param Context Current camera execution context
	 */
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	/**
	 * Called when modifier is triggered
	 * Stores initial positions and prepares cinematic
	 * @param InTriggerData Trigger configuration
	 */
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;

	/**
	 * Called when cinematic becomes fully active (blend in complete)
	 * Can be used to start gameplay effects (disable player input, etc.)
	 */
	virtual void OnActivated() override;

	/**
	 * Called when cinematic begins blend out
	 * Can be used to prepare return to normal camera
	 */
	virtual void OnDeactivating() override;

	//========================================
	// Cinematic Camera Helpers
	//========================================

	/**
	 * Calculate camera position for cinematic shot
	 * Override in derived classes for specific behaviors
	 * @param Context Current execution context
	 * @return World position for camera
	 */
	virtual FVector CalculateCinematicPosition(const FStageExecutionContext& Context) const;

	/**
	 * Calculate camera rotation to look at target
	 * @param CameraPos Current camera position
	 * @param TargetPos Target to look at
	 * @return Rotation that points camera at target
	 */
	FRotator CalculateLookAtRotation(const FVector& CameraPos, const FVector& TargetPos) const;

	/**
	 * Calculate midpoint between player and target
	 * Useful for framing both characters
	 * @param Context Current execution context
	 * @return Midpoint location
	 */
	FVector CalculateMidpoint(const FStageExecutionContext& Context) const;

	/**
	 * Interpolate camera transform smoothly
	 * @param DeltaTime Frame delta time
	 * @param TargetTransform Desired transform
	 * @param InterpSpeed Interpolation speed
	 */
	void InterpolateCameraTransform(float DeltaTime, const FTransform& TargetTransform, float InterpSpeed);

	/**
	 * Apply letterbox effect
	 * Sets VignetteEffect to simulate cinematic bars
	 * @param Amount Letterbox amount (0-1, typically 0.1-0.2)
	 */
	void ApplyLetterbox(float Amount);

	//========================================
	// Cinematic Parameters (Configurable)
	//========================================

	/** Camera position offset relative to focus point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Position")
	FVector CameraOffset;

	/** Height offset for camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Position", meta = (ClampMin = "-500.0", ClampMax = "500.0"))
	float HeightOffset;

	/** Distance from focus point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Position", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float CameraDistance;

	/** Override FOV for cinematic (0 = use current FOV) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|FOV", meta = (ClampMin = "0.0", ClampMax = "120.0"))
	float CinematicFOV;

	/** Should letterbox be applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Effects")
	bool bApplyLetterbox;

	/** Letterbox amount (0-0.2 typical) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Effects", meta = (ClampMin = "0.0", ClampMax = "0.3", EditCondition = "bApplyLetterbox"))
	float LetterboxAmount;

	/** Camera transform interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Interpolation", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float TransformInterpSpeed;

	/** Should block player camera input during cinematic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Control")
	bool bBlockPlayerInput;

	//========================================
	// Runtime State
	//========================================

	/** Stored player location at trigger time */
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|State")
	FVector StoredPlayerLocation;

	/** Stored target/victim location at trigger time */
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|State")
	FVector StoredTargetLocation;

	/** Stored target actor reference */
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|State")
	TWeakObjectPtr<AActor> StoredTargetActor;

	/** Current interpolated camera transform */
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|State")
	FTransform CurrentCameraTransform;

	/** Is camera transform initialized */
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|State")
	bool bTransformInitialized;

	/** Direction from player to target (normalized) */
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|State")
	FVector ActionDirection;
};


//========================================
// C01: Execution Camera
//========================================

/**
 * UCameraModifier_C01_Cinematic_Execution
 * 
 * C01: Execution - Cinematic camera for visceral attack / finishing blow
 * 
 * Triggered during execution/visceral attack animations.
 * Camera orbits around the action, providing dramatic angles.
 * 
 * Behavior:
 * - Camera orbits around player-enemy midpoint
 * - Low angle for dramatic effect
 * - Slow orbit speed for cinematic feel
 * - Optional time dilation hint
 * 
 * Typical Parameters:
 * - Duration: 2.0 seconds
 * - Orbit radius: 150-250 cm
 * - Orbit height: 50-100 cm below eye level
 * - FOV: 60-70 degrees (tighter than normal)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_C01_Cinematic_Execution : public UCameraModifier_CinematicBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures execution camera parameters */
	UCameraModifier_C01_Cinematic_Execution();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_C01_Cinematic_Execution; }
	virtual FString GetModifierDescription() const override { return TEXT("Cinematic camera for execution/visceral attack"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 2.0f; }
	virtual float GetBlendInTime() const override { return 0.15f; }
	virtual float GetBlendOutTime() const override { return 0.30f; }

	//========================================
	// Priority
	//========================================

	/** High priority for cinematic moments */
	virtual int32 GetPriority() const override { return 200; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Execution-Specific Parameters
	//========================================

	/** Orbit speed (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Execution", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float OrbitSpeed;

	/** Orbit radius from midpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Execution", meta = (ClampMin = "100.0", ClampMax = "400.0"))
	float OrbitRadius;

	/** Orbit height offset (negative = below) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Execution", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	float OrbitHeight;

	/** Starting orbit angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Execution")
	float StartOrbitAngle;

	/** Current orbit angle (runtime) */
	float CurrentOrbitAngle;
};


//========================================
// C02: Backstab Camera
//========================================

/**
 * UCameraModifier_C02_Cinematic_Backstab
 * 
 * C02: Backstab - Cinematic camera for backstab / stealth kill
 * 
 * Triggered during backstab animations.
 * Camera positioned behind and to the side, emphasizing the stealth kill.
 * 
 * Behavior:
 * - Camera starts behind player
 * - Over-shoulder angle showing the attack
 * - Close-up for intimacy of the kill
 * - Minimal movement for clean shot
 * 
 * Typical Parameters:
 * - Duration: 1.5 seconds
 * - Side offset: 30-50 cm
 * - Height: Eye level or slightly above
 * - FOV: 65-75 degrees
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_C02_Cinematic_Backstab : public UCameraModifier_CinematicBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures backstab camera parameters */
	UCameraModifier_C02_Cinematic_Backstab();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_C02_Cinematic_Backstab; }
	virtual FString GetModifierDescription() const override { return TEXT("Cinematic camera for backstab/stealth kill"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 1.5f; }
	virtual float GetBlendInTime() const override { return 0.10f; }
	virtual float GetBlendOutTime() const override { return 0.25f; }

	//========================================
	// Priority
	//========================================

	virtual int32 GetPriority() const override { return 200; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Backstab-Specific Parameters
	//========================================

	/** Side offset for over-shoulder view (positive = right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Backstab", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
	float SideOffset;

	/** Distance behind the action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Backstab", meta = (ClampMin = "50.0", ClampMax = "300.0"))
	float BackDistance;

	/** Height offset from action center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Backstab", meta = (ClampMin = "-100.0", ClampMax = "150.0"))
	float BackstabHeightOffset;

	/** Slight push-in during animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Backstab", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float PushInAmount;
};


//========================================
// C03: Riposte Camera
//========================================

/**
 * UCameraModifier_C03_Cinematic_Riposte
 * 
 * C03: Riposte - Cinematic camera for parry counter-attack
 * 
 * Triggered during riposte/critical attack after parry.
 * Quick dramatic shot emphasizing the counter-attack.
 * 
 * Behavior:
 * - Camera pulls in quickly for dramatic effect
 * - Low angle to emphasize power
 * - Faster than other cinematics (reward for skill)
 * - Side angle showing both characters
 * 
 * Typical Parameters:
 * - Duration: 1.2 seconds (shorter = snappier)
 * - Pull-in: 50-80 cm
 * - Low angle: 20-40 cm below eye level
 * - FOV: 55-65 degrees (tighter)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_C03_Cinematic_Riposte : public UCameraModifier_CinematicBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures riposte camera parameters */
	UCameraModifier_C03_Cinematic_Riposte();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_C03_Cinematic_Riposte; }
	virtual FString GetModifierDescription() const override { return TEXT("Cinematic camera for parry riposte"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 1.2f; }
	virtual float GetBlendInTime() const override { return 0.08f; }  // Very fast blend in
	virtual float GetBlendOutTime() const override { return 0.20f; }

	//========================================
	// Priority
	//========================================

	virtual int32 GetPriority() const override { return 200; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Riposte-Specific Parameters
	//========================================

	/** Pull-in distance for dramatic close-up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Riposte", meta = (ClampMin = "0.0", ClampMax = "150.0"))
	float PullInDistance;

	/** Low angle offset (negative = camera below eye level) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Riposte", meta = (ClampMin = "-100.0", ClampMax = "50.0"))
	float LowAngleOffset;

	/** Side angle offset for profile view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Riposte", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float SideAngle;

	/** Time dilation hint during riposte */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Riposte", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiposteTimeDilation;
};


//========================================
// C04: Boss Intro Camera
//========================================

/**
 * UCameraModifier_C04_Cinematic_BossIntro
 * 
 * C04: Boss Intro - Cinematic camera for boss encounter introduction
 * 
 * Triggered when entering boss arena for first time.
 * Sweeping cinematic shot revealing the boss.
 * 
 * Behavior:
 * - Camera starts on player, sweeps to reveal boss
 * - Dramatic wide shot of arena
 * - Multiple phases: player -> sweep -> boss reveal
 * - Longest cinematic (3-5 seconds)
 * 
 * Typical Parameters:
 * - Duration: 4.0 seconds
 * - Sweep angle: 90-180 degrees
 * - Wide FOV for arena reveal
 * - Letterbox for cinematic feel
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_C04_Cinematic_BossIntro : public UCameraModifier_CinematicBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures boss intro camera parameters */
	UCameraModifier_C04_Cinematic_BossIntro();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_C04_Cinematic_BossIntro; }
	virtual FString GetModifierDescription() const override { return TEXT("Cinematic camera for boss introduction"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 4.0f; }
	virtual float GetBlendInTime() const override { return 0.50f; }
	virtual float GetBlendOutTime() const override { return 0.50f; }

	//========================================
	// Priority
	//========================================

	/** Highest priority - boss intro is critical */
	virtual int32 GetPriority() const override { return 250; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Boss Intro-Specific Parameters
	//========================================

	/** Sweep arc angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossIntro", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float SweepAngle;

	/** Sweep height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossIntro", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float SweepHeight;

	/** Distance from arena center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossIntro", meta = (ClampMin = "200.0", ClampMax = "1000.0"))
	float ArenaDistance;

	/** Phase 1 duration ratio (player focus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossIntro", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float Phase1Ratio;

	/** Phase 2 duration ratio (sweep) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossIntro", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float Phase2Ratio;

	/** Starting camera position (calculated at trigger) */
	FVector StartPosition;

	/** Boss reveal position */
	FVector BossRevealPosition;

	/** Current intro phase (0-1 normalized) */
	float IntroPhase;
};


//========================================
// C05: Boss Phase Transition Camera
//========================================

/**
 * UCameraModifier_C05_Cinematic_BossPhase
 * 
 * C05: Boss Phase - Cinematic camera for boss phase transitions
 * 
 * Triggered when boss transitions between phases.
 * Dramatic shot emphasizing the transformation/power-up.
 * 
 * Behavior:
 * - Camera focuses on boss during transition
 * - Pull back to show transformation
 * - Possible screen effects (shake, flash)
 * - Returns to combat camera after
 * 
 * Typical Parameters:
 * - Duration: 2.5 seconds
 * - Focus on boss
 * - Wide shot for transformation
 * - Screen shake during power-up
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_C05_Cinematic_BossPhase : public UCameraModifier_CinematicBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures boss phase camera parameters */
	UCameraModifier_C05_Cinematic_BossPhase();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_C05_Cinematic_BossPhase; }
	virtual FString GetModifierDescription() const override { return TEXT("Cinematic camera for boss phase transition"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 2.5f; }
	virtual float GetBlendInTime() const override { return 0.30f; }
	virtual float GetBlendOutTime() const override { return 0.40f; }

	//========================================
	// Priority
	//========================================

	/** Very high priority */
	virtual int32 GetPriority() const override { return 240; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Boss Phase-Specific Parameters
	//========================================

	/** Pull back distance during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossPhase", meta = (ClampMin = "0.0", ClampMax = "300.0"))
	float PullBackDistance;

	/** Height rise during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossPhase", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float RiseHeight;

	/** Screen shake intensity during power-up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossPhase", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float TransformationShakeIntensity;

	/** FOV punch during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|BossPhase", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float FOVPunch;

	/** Current transformation phase (0-1) */
	float TransformationPhase;
};
