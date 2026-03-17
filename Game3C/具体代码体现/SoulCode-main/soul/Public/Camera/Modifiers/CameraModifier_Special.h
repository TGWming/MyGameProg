// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierBase.h"
#include "CameraModifier_Special.generated.h"

/**
 * Special Modifier Declarations (X01-X03)
 * 
 * This file contains the 3 Special modifiers for the camera system.
 * Special modifiers provide time control and special camera behaviors for
 * critical gameplay moments.
 * 
 * All special modifiers inherit from UCameraModifier_SpecialBase which provides
 * common functionality for time manipulation and special effects.
 * 
 * Key Characteristics:
 * - Control time flow (slow motion, hit stop)
 * - Special camera behaviors (death cam)
 * - High/Highest priority (critical moments)
 * - Usually triggered by important gameplay events
 * - Cannot be interrupted by lower priority modifiers
 * 
 * Special Types:
 * - X01: SlowMotion - Time dilation for dramatic moments
 * - X02: HitStop - Brief freeze frame on impact
 * - X03: DeathCam - Player death cinematic camera
 */


//========================================
// Special Base Class
//========================================

/**
 * UCameraModifier_SpecialBase
 * 
 * Abstract base class for all special modifiers.
 * Provides common functionality for time manipulation and special effects.
 * 
 * Features:
 * - Time dilation control with smooth ramp in/out
 * - High priority handling
 * - Cannot be interrupted (critical moments)
 * - Visual effects synchronized with time changes
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraModifier_SpecialBase : public UCameraModifierBase
{
	GENERATED_BODY()

public:
	/** Constructor - sets default special parameters */
	UCameraModifier_SpecialBase();

	//========================================
	// Modifier Identity
	//========================================

	/** All special modifiers belong to the Special category */
	virtual EModifierCategory GetModifierCategory() const override { return EModifierCategory::Special; }

	/** Special modifiers use the Special type for output classification */
	virtual ECameraModifierType GetModifierType() const override { return ECameraModifierType::Special; }

	/** Special modifiers have high priority */
	virtual int32 GetPriority() const override { return 180; }

	/** Special modifiers cannot be interrupted */
	virtual bool CanBeInterrupted() const override { return false; }

	/** Special modifiers do not stack (except HitStop) */
	virtual bool CanStack() const override { return false; }

protected:
	//========================================
	// Effect Computation
	//========================================

	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void OnActivated() override;
	virtual void OnDeactivating() override;

	//========================================
	// Special Effect Helpers
	//========================================

	/**
	 * Calculate smooth time dilation curve
	 * @param NormalizedTime Current time in effect (0-1)
	 * @param TargetDilation Target time dilation value
	 * @param RampInRatio How much of duration to ramp in (0-0.5)
	 * @param RampOutRatio How much of duration to ramp out (0-0.5)
	 * @return Current time dilation value
	 */
	float CalculateTimeDilation(float NormalizedTime, float TargetDilation, float RampInRatio, float RampOutRatio) const;

	/**
	 * Apply radial blur approximation
	 * @param Intensity Blur intensity
	 */
	void ApplyRadialBlur(float Intensity);

	//========================================
	// Special Parameters (Configurable)
	//========================================

	/** Target time dilation (1.0 = normal, 0.5 = half speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Time", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float TargetTimeDilation;

	/** Time to ramp into effect (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Time", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeRampIn;

	/** Time to ramp out of effect (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Time", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeRampOut;

	/** Should apply visual effects during special */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Visual")
	bool bApplyVisualEffects;

	/** Vignette during special effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Visual", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bApplyVisualEffects"))
	float SpecialVignette;

	/** Saturation during special effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Visual", meta = (ClampMin = "0.0", ClampMax = "1.5", EditCondition = "bApplyVisualEffects"))
	float SpecialSaturation;

	//========================================
	// Runtime State
	//========================================

	/** Current interpolated time dilation */
	UPROPERTY(BlueprintReadOnly, Category = "Special|State")
	float CurrentTimeDilation;

	/** Is the special effect at full intensity */
	UPROPERTY(BlueprintReadOnly, Category = "Special|State")
	bool bAtFullIntensity;
};


//========================================
// X01: Slow Motion
//========================================

/**
 * UCameraModifier_X01_Special_SlowMotion
 * 
 * X01: Slow Motion - Time dilation for dramatic moments
 * 
 * Triggered during critical gameplay moments:
 * - Parry success
 * - Near-miss dodge
 * - Critical hit landing
 * - Boss phase transition
 * 
 * Effects:
 * - Smooth time dilation (configurable target)
 * - Optional visual enhancements (vignette, desaturation)
 * - Gradual ramp in and out
 * 
 * Typical Parameters:
 * - Time Dilation: 0.3-0.5 (30-50% speed)
 * - Duration: 0.5-2.0 seconds
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_X01_Special_SlowMotion : public UCameraModifier_SpecialBase
{
	GENERATED_BODY()

public:
	UCameraModifier_X01_Special_SlowMotion();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_X01_Special_SlowMotion; }
	virtual FString GetModifierDescription() const override { return TEXT("Slow motion for dramatic moments"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.8f; }
	virtual float GetBlendInTime() const override { return 0.05f; }
	virtual float GetBlendOutTime() const override { return 0.25f; }

	//========================================
	// Priority
	//========================================

	virtual int32 GetPriority() const override { return 185; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Slow Motion-Specific Parameters
	//========================================

	/** Slow motion time scale (0.3 = 30% speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|SlowMotion", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SlowMotionScale;

	/** Apply desaturation during slow motion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|SlowMotion")
	bool bDesaturateDuringSlowMo;

	/** Desaturation amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|SlowMotion", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bDesaturateDuringSlowMo"))
	float SlowMoDesaturation;

	/** FOV change during slow motion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|SlowMotion", meta = (ClampMin = "-10.0", ClampMax = "10.0"))
	float SlowMoFOVChange;
};


//========================================
// X02: Hit Stop
//========================================

/**
 * UCameraModifier_X02_Special_HitStop
 * 
 * X02: Hit Stop - Brief freeze frame on impact
 * 
 * Triggered on significant impacts:
 * - Heavy attack connects
 * - Parry success moment
 * - Shield break
 * - Critical hit
 * 
 * Effects:
 * - Near-complete time stop (0.01-0.05 time scale)
 * - Very brief duration (0.05-0.15 seconds)
 * - Slight FOV punch
 * - Optional shake on release
 * 
 * Typical Parameters:
 * - Time Dilation: 0.01-0.05 (near freeze)
 * - Duration: 0.05-0.12 seconds
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_X02_Special_HitStop : public UCameraModifier_SpecialBase
{
	GENERATED_BODY()

public:
	UCameraModifier_X02_Special_HitStop();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_X02_Special_HitStop; }
	virtual FString GetModifierDescription() const override { return TEXT("Brief freeze frame on impact"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 0.08f; }
	virtual float GetBlendInTime() const override { return 0.0f; }  // Instant
	virtual float GetBlendOutTime() const override { return 0.02f; }

	//========================================
	// Priority & Stacking
	//========================================

	virtual int32 GetPriority() const override { return 195; }
	virtual bool CanStack() const override { return true; }  // Can stack for multi-hit

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Hit Stop-Specific Parameters
	//========================================

	/** Time scale during hit stop (0.01 = near freeze) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float HitStopTimeScale;

	/** FOV punch on hit stop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|HitStop", meta = (ClampMin = "-10.0", ClampMax = "0.0"))
	float HitStopFOVPunch;

	/** Apply screen shake on release */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|HitStop")
	bool bShakeOnRelease;

	/** Release shake intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0", EditCondition = "bShakeOnRelease"))
	float ReleaseShakeIntensity;

	/** Chromatic aberration amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float HitStopChromaticAberration;

	/** Hit direction for directional effects */
	FVector HitDirection;
};


//========================================
// X03: Death Camera
//========================================

/**
 * UCameraModifier_X03_Special_DeathCam
 * 
 * X03: Death Camera - Player death cinematic camera
 * 
 * Triggered when player dies.
 * Provides dramatic death sequence with slow pull-back.
 * 
 * Effects:
 * - Slow orbit around death location
 * - Gradual pull-back and rise
 * - Strong desaturation/color grading
 * - Heavy vignette
 * - Slow time dilation
 * - Override mode (full camera control)
 * 
 * Typical Parameters:
 * - Duration: 3-5 seconds
 * - Orbit speed: 10-20 degrees/second
 * - Time Dilation: 0.3-0.5
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModifier_X03_Special_DeathCam : public UCameraModifier_SpecialBase
{
	GENERATED_BODY()

public:
	UCameraModifier_X03_Special_DeathCam();

	//========================================
	// Modifier Identity
	//========================================

	virtual ECameraModifierID GetModifierID() const override { return ECameraModifierID::Modifier_X03_Special_DeathCam; }
	virtual FString GetModifierDescription() const override { return TEXT("Cinematic camera for player death"); }

	//========================================
	// Timing Configuration
	//========================================

	virtual float GetDefaultDuration() const override { return 4.0f; }
	virtual float GetBlendInTime() const override { return 0.3f; }
	virtual float GetBlendOutTime() const override { return 0.5f; }

	//========================================
	// Override Mode
	//========================================

	/** Death cam takes full control of camera */
	virtual bool IsOverrideModifier() const override { return true; }

	//========================================
	// Priority
	//========================================

	/** Death cam has highest priority */
	virtual int32 GetPriority() const override { return 250; }

protected:
	virtual void OnTriggered(const FModifierTriggerData& InTriggerData) override;
	virtual void ComputeEffect(float DeltaTime, const FStageExecutionContext& Context) override;

	//========================================
	// Death Cam-Specific Parameters
	//========================================

	/** Orbit speed (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|DeathCam", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float OrbitSpeed;

	/** Pull-back distance over duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|DeathCam", meta = (ClampMin = "0.0", ClampMax = "400.0"))
	float PullBackDistance;

	/** Rise height over duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|DeathCam", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float RiseHeight;

	/** Desaturation at death (0 = full color, 1 = grayscale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|DeathCam", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeathDesaturation;

	/** Vignette at death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|DeathCam", meta = (ClampMin = "0.0", ClampMax = "0.6"))
	float DeathVignette;

	/** Time dilation during death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|DeathCam", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float DeathTimeDilation;

	//========================================
	// Runtime State
	//========================================

	/** Death location (stored at trigger) */
	FVector DeathLocation;

	/** Current orbit angle */
	float CurrentOrbitAngle;

	/** Starting orbit angle */
	float StartOrbitAngle;

	/** Stored player forward for orbit start */
	FVector StoredForward;
};
