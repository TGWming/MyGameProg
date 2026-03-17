// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierBase.h"
#include "CameraModifier_Zoom.generated.h"

/**
 * Zoom Modifier Declarations (Z01-Z04)
 * 
 * This file contains the 4 Zoom modifiers for the camera system.
 * Zoom modifiers provide FOV and distance pulse effects for
 * combat feedback and dramatic moments.
 * 
 * All zoom modifiers inherit from UCameraModifier_ZoomBase which provides
 * common functionality for FOV/distance interpolation.
 * 
 * Key Characteristics:
 * - Primary effect: FOV changes (reduction or expansion)
 * - Secondary effect: Distance changes (pull in or push out)
 * - Short to medium duration (0.15-1.5 seconds)
 * - Medium priority (150-165)
 * - Some can stack for cumulative effect
 * 
 * Zoom Types:
 * - Z01: AttackImpact - Quick FOV punch on hit (tighten)
 * - Z02: ChargeRelease - FOV burst on charged attack release (widen)
 * - Z03: SkillActivate - Focus zoom during skill/magic use
 * - Z04: CriticalHit - Dramatic zoom for critical hits
 * 
 * Comparison with Other Modifiers:
 * - Shake: Random position/rotation noise
 * - Reaction: Directional position offset
 * - Zoom: FOV/distance pulse (this category)
 * - Cinematic: Full camera override
 */


//========================================
// Zoom Base Class
//========================================

/**
 * UCameraModifier_ZoomBase
 * 
 * Abstract base class for all zoom modifiers.
 * Provides common functionality for FOV and distance effects.
 * 
 * Features:
 * - FOV pulse (tighten or widen)
 * - Distance pulse (pull in or push out)
 * - Smooth interpolation with configurable curves
 * - Optional vignette for focus effect
 * 
 * Effect Curves:
 * - Punch: Quick in, slow out (attack impact)
 * - Burst: Quick expansion, medium recovery (charge release)
 * - Sustain: Gradual change, hold, gradual return (skill use)
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraModifier_ZoomBase : public UCameraModifierBase
{
	GENERATED_BODY()

public:
	/** Constructor - sets default zoom parameters */
	UCameraModifier_ZoomBase();

	//========================================
	// Modifier Identity
	//========================================

	/** All zoom modifiers belong to the Zoom category */
	virtual EModifierCategory GetModifierCategory() const override { return EModifierCategory::Zoom; }

	/** Zoom modifiers use the Zoom type for output classification */
	virtual ECameraModifierType GetModifierType() const override { return ECameraModifierType::Zoom; }

protected:
	//========================================
	// Effect Computation
	//========================================

	/**
	 * Compute zoom effect values
	 * Calculates FOV and distance changes based on zoom type
	 * @param DeltaTime Time since last frame
	 * @param Context Current camera execution context
	 */
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	/**
	 * Called when modifier is triggered
	 * Initializes zoom state
	 * @param InTriggerData Trigger configuration
	 */
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;

	//========================================
	// Zoom Effect Helpers
	//========================================

	/**
	 * Apply FOV punch effect (quick tighten, slow release)
	 * Used for attack impacts
	 * @param NormalizedTime Current time in effect (0-1)
	 * @param PunchAmount Maximum FOV reduction in degrees
	 * @return FOV change value (negative = tighten)
	 */
	float CalculateFOVPunch(float NormalizedTime, float PunchAmount) const;

	/**
	 * Apply FOV burst effect (quick expand, medium recovery)
	 * Used for charge releases
	 * @param NormalizedTime Current time in effect (0-1)
	 * @param BurstAmount Maximum FOV expansion in degrees
	 * @return FOV change value (positive = widen)
	 */
	float CalculateFOVBurst(float NormalizedTime, float BurstAmount) const;

	/**
	 * Apply sustained FOV change (gradual in, hold, gradual out)
	 * Used for skill activation
	 * @param NormalizedTime Current time in effect (0-1)
	 * @param TargetChange Target FOV change (negative = tighten, positive = widen)
	 * @param HoldRatio How much of duration to hold at peak (0-1)
	 * @return FOV change value
	 */
	float CalculateFOVSustain(float NormalizedTime, float TargetChange, float HoldRatio) const;

	/**
	 * Calculate distance pulse
	 * @param NormalizedTime Current time in effect (0-1)
	 * @param PullAmount Distance change (negative = closer, positive = farther)
	 * @return Distance change value
	 */
	float CalculateDistancePulse(float NormalizedTime, float PullAmount) const;

	//========================================
	// Zoom Parameters (Configurable)
	//========================================

	/** Base FOV change amount (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|FOV", meta = (ClampMin = "-30.0", ClampMax = "30.0"))
	float BaseFOVChange;

	/** Base distance change amount (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Distance", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	float BaseDistanceChange;

	/** Whether to apply vignette during zoom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Effects")
	bool bApplyVignette;

	/** Vignette intensity when zoomed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Effects", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bApplyVignette"))
	float ZoomVignetteIntensity;

	/** Interpolation speed for FOV changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Interpolation", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float FOVInterpSpeed;

	//========================================
	// Runtime State
	//========================================

	/** Current FOV change value */
	UPROPERTY(BlueprintReadOnly, Category = "Zoom|State")
	float CurrentFOVChange;

	/** Target FOV change value */
	UPROPERTY(BlueprintReadOnly, Category = "Zoom|State")
	float TargetFOVChange;

	/** Current distance change value */
	UPROPERTY(BlueprintReadOnly, Category = "Zoom|State")
	float CurrentDistanceChange;
};


//========================================
// Z01: Attack Impact Zoom
//========================================

/**
 * UCameraModifier_Z01_Zoom_AttackImpact
 * 
 * Z01: Attack Impact - Quick FOV punch on hit
 * 
 * Triggered when player's attack connects with enemy.
 * Provides satisfying hit feedback through brief FOV tightening.
 * 
 * Effects:
 * - FOV: Quick reduction (punch), slow recovery
 * - Distance: Slight pull in
 * - Duration: Very short (0.15 seconds)
 * - Can stack for combo hits
 * 
 * Typical Parameters:
 * - FOV Punch: -3 to -6 degrees
 * - Distance Pull: -10 to -20 cm
 * - Very fast blend in (0.01s)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_Z01_Zoom_AttackImpact : public UCameraModifier_ZoomBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures attack impact zoom parameters */
	UCameraModifier_Z01_Zoom_AttackImpact();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_Z01_Zoom_AttackImpact; }
	virtual FString GetModifierDescription() const override { return TEXT("Quick FOV punch on attack impact"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.15f; }
	virtual float GetBlendInTime() const override { return 0.01f; }
	virtual float GetBlendOutTime() const override { return 0.10f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 150; }
	virtual bool CanStack() const override { return true; }  // Stack for combos
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Attack Impact-Specific Parameters
	//========================================

	/** FOV punch amount (negative = tighten) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|AttackImpact", meta = (ClampMin = "-15.0", ClampMax = "0.0"))
	float FOVPunchAmount;

	/** Distance pull amount (negative = closer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|AttackImpact", meta = (ClampMin = "-50.0", ClampMax = "0.0"))
	float DistancePullAmount;

	/** Scale punch based on hit strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|AttackImpact")
	bool bScaleWithHitStrength;
};


//========================================
// Z02: Charge Release Zoom
//========================================

/**
 * UCameraModifier_Z02_Zoom_ChargeRelease
 * 
 * Z02: Charge Release - FOV burst on charged attack release
 * 
 * Triggered when releasing a fully charged attack.
 * Dramatic FOV expansion to emphasize power release.
 * 
 * Effects:
 * - FOV: Quick expansion (burst), medium recovery
 * - Distance: Push out for dramatic effect
 * - Duration: Short (0.25 seconds)
 * - Scales with charge level
 * 
 * Typical Parameters:
 * - FOV Burst: +5 to +10 degrees
 * - Distance Push: +20 to +40 cm
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_Z02_Zoom_ChargeRelease : public UCameraModifier_ZoomBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures charge release zoom parameters */
	UCameraModifier_Z02_Zoom_ChargeRelease();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_Z02_Zoom_ChargeRelease; }
	virtual FString GetModifierDescription() const override { return TEXT("FOV burst on charged attack release"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.25f; }
	virtual float GetBlendInTime() const override { return 0.02f; }
	virtual float GetBlendOutTime() const override { return 0.18f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 155; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Charge Release-Specific Parameters
	//========================================

	/** FOV burst amount (positive = widen) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|ChargeRelease", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float FOVBurstAmount;

	/** Distance push amount (positive = farther) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|ChargeRelease", meta = (ClampMin = "0.0", ClampMax = "80.0"))
	float DistancePushAmount;

	/** Charge level from trigger (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Zoom|ChargeRelease")
	float ChargeLevel;

	/** Minimum charge level for full effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|ChargeRelease", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinChargeForFullEffect;
};


//========================================
// Z03: Skill Activate Zoom
//========================================

/**
 * UCameraModifier_Z03_Zoom_SkillActivate
 * 
 * Z03: Skill Activate - Focus zoom during skill/magic use
 * 
 * Triggered when activating a skill or casting magic.
 * Sustained zoom effect for the duration of skill activation.
 * 
 * Effects:
 * - FOV: Gradual reduction (focus), hold, gradual return
 * - Distance: Varies based on skill type
 * - Duration: Medium (0.6 seconds)
 * - Vignette for focus effect
 * 
 * Typical Parameters:
 * - FOV Focus: -5 to -8 degrees
 * - Vignette: 0.1-0.15
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_Z03_Zoom_SkillActivate : public UCameraModifier_ZoomBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures skill activate zoom parameters */
	UCameraModifier_Z03_Zoom_SkillActivate();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_Z03_Zoom_SkillActivate; }
	virtual FString GetModifierDescription() const override { return TEXT("Focus zoom during skill activation"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.60f; }
	virtual float GetBlendInTime() const override { return 0.08f; }
	virtual float GetBlendOutTime() const override { return 0.15f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 158; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return true; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Skill Activate-Specific Parameters
	//========================================

	/** FOV focus amount (negative = tighten for focus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|SkillActivate", meta = (ClampMin = "-20.0", ClampMax = "0.0"))
	float FocusFOVAmount;

	/** Distance change for skill (negative = closer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|SkillActivate", meta = (ClampMin = "-50.0", ClampMax = "50.0"))
	float SkillDistanceChange;

	/** Hold ratio (how long to hold at peak) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|SkillActivate", meta = (ClampMin = "0.0", ClampMax = "0.6"))
	float HoldRatio;

	/** Is this an AoE skill (affects zoom direction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|SkillActivate")
	bool bIsAoESkill;
};


//========================================
// Z04: Critical Hit Zoom
//========================================

/**
 * UCameraModifier_Z04_Zoom_CriticalHit
 * 
 * Z04: Critical Hit - Dramatic zoom for critical hits
 * 
 * Triggered on critical hit / weak point hit.
 * Emphasizes the critical damage with dramatic zoom.
 * 
 * Effects:
 * - FOV: Significant reduction for dramatic focus
 * - Distance: Pull in towards action
 * - Duration: Medium (0.4 seconds)
 * - Strong vignette
 * - Optional time dilation hint
 * 
 * Typical Parameters:
 * - FOV Change: -8 to -12 degrees
 * - Distance Pull: -30 to -50 cm
 * - Vignette: 0.15-0.2
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_Z04_Zoom_CriticalHit : public UCameraModifier_ZoomBase
{
	GENERATED_BODY()

public:
	/** Constructor - configures critical hit zoom parameters */
	UCameraModifier_Z04_Zoom_CriticalHit();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_Z04_Zoom_CriticalHit; }
	virtual FString GetModifierDescription() const override { return TEXT("Dramatic zoom for critical hits"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.40f; }
	virtual float GetBlendInTime() const override { return 0.03f; }
	virtual float GetBlendOutTime() const override { return 0.25f; }

	//========================================
	// Priority & Stacking
	//========================================

	/** Higher priority than regular attack impact */
	virtual int32 GetPriority() const override { return 165; }
	virtual bool CanStack() const override { return false; }
	virtual bool CanBeInterrupted() const override { return false; }  // Critical hit completes

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Critical Hit-Specific Parameters
	//========================================

	/** FOV change for critical (negative = tighten) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|CriticalHit", meta = (ClampMin = "-25.0", ClampMax = "0.0"))
	float CriticalFOVChange;

	/** Distance pull for critical (negative = closer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|CriticalHit", meta = (ClampMin = "-100.0", ClampMax = "0.0"))
	float CriticalDistancePull;

	/** Vignette intensity for critical */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|CriticalHit", meta = (ClampMin = "0.0", ClampMax = "0.4"))
	float CriticalVignetteIntensity;

	/** Time dilation hint for critical (1.0 = no effect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|CriticalHit", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float CriticalTimeDilation;

	/** Saturation reduction for dramatic effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|CriticalHit", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float CriticalSaturation;
};
