// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SoulsCameraParams_Modifier.generated.h"

/**
 * C-Group: Modifier Parameters (58 params total)
 * Controls behavior of the 26 camera modifiers
 */

//========================================
// C1: Shake Modifier Parameters (10 params)
//========================================
USTRUCT(BlueprintType)
struct FModifierParams_Shake
{
	GENERATED_BODY()

	/** Light hit shake amplitude (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Hit", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float LightHitAmplitude;

	/** Light hit shake duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Hit", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float LightHitDuration;

	/** Heavy hit shake amplitude (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Hit", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float HeavyHitAmplitude;

	/** Heavy hit shake duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Hit", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float HeavyHitDuration;

	/** Attack hit shake amplitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Attack", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float AttackHitAmplitude;

	/** Attack hit shake duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Attack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackHitDuration;

	/** Shake frequency (oscillations per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|General", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float ShakeFrequency;

	/** Shake decay rate (how fast shake diminishes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|General", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ShakeDecayRate;

	/** Use Perlin noise for organic shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|General")
	bool bUsePerlinNoise;

	/** Landing shake scale based on fall height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake|Landing", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float LandingShakeScale;

	FModifierParams_Shake()
		: LightHitAmplitude(1.5f)
		, LightHitDuration(0.2f)
		, HeavyHitAmplitude(4.0f)
		, HeavyHitDuration(0.4f)
		, AttackHitAmplitude(0.8f)
		, AttackHitDuration(0.15f)
		, ShakeFrequency(20.0f)
		, ShakeDecayRate(3.0f)
		, bUsePerlinNoise(true)
		, LandingShakeScale(0.02f)
	{}
};

//========================================
// C2: Reaction Modifier Parameters (14 params)
//========================================
USTRUCT(BlueprintType)
struct FModifierParams_Reaction
{
	GENERATED_BODY()

	//--- Parry Reaction ---
	/** Distance to pull camera on parry (negative = towards target) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Parry", meta = (ClampMin = "-200.0", ClampMax = "0.0"))
	float ParryPullDistance;

	/** Duration of parry camera effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Parry", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ParryDuration;

	/** FOV pulse amount on parry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Parry", meta = (ClampMin = "-20.0", ClampMax = "0.0"))
	float ParryFOVPulse;

	/** Perfect parry effect multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Parry", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float PerfectParryMultiplier;

	//--- Stagger Reaction ---
	/** Camera push back on enemy stagger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Stagger", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float StaggerPushDistance;

	/** Stagger effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Stagger", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StaggerDuration;

	//--- Knockback Reaction ---
	/** Camera follow speed during knockback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockback", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float KnockbackFollowSpeed;

	/** Additional distance during knockback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockback", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float KnockbackDistanceAdd;

	//--- Knockdown Reaction ---
	/** Vertical offset when player knocked down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockdown", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float KnockdownVerticalOffset;

	/** Recovery time from knockdown camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|Knockdown", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float KnockdownRecoveryTime;

	//--- Guard Break Reaction ---
	/** FOV pulse on guard break */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|GuardBreak", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float GuardBreakFOVPulse;

	/** Guard break effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|GuardBreak", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GuardBreakDuration;

	//--- General ---
	/** Blend in time for all reactions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|General", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float ReactionBlendInTime;

	/** Blend out time for all reactions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction|General", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ReactionBlendOutTime;

	FModifierParams_Reaction()
		: ParryPullDistance(-50.0f)
		, ParryDuration(0.3f)
		, ParryFOVPulse(-8.0f)
		, PerfectParryMultiplier(1.5f)
		, StaggerPushDistance(30.0f)
		, StaggerDuration(0.25f)
		, KnockbackFollowSpeed(8.0f)
		, KnockbackDistanceAdd(50.0f)
		, KnockdownVerticalOffset(40.0f)
		, KnockdownRecoveryTime(1.0f)
		, GuardBreakFOVPulse(10.0f)
		, GuardBreakDuration(0.4f)
		, ReactionBlendInTime(0.05f)
		, ReactionBlendOutTime(0.3f)
	{}
};

//========================================
// C3: Cinematic Modifier Parameters (12 params)
//========================================
USTRUCT(BlueprintType)
struct FModifierParams_Cinematic
{
	GENERATED_BODY()

	//--- Execution ---
	/** Camera angle offset for execution (degrees from side) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Execution", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float ExecutionAngle;

	/** Camera distance during execution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Execution", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float ExecutionDistance;

	/** Execution camera height offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Execution", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
	float ExecutionHeightOffset;

	//--- Backstab ---
	/** Camera angle for backstab (behind attacker) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Backstab", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float BackstabAngle;

	/** Backstab camera distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Backstab", meta = (ClampMin = "100.0", ClampMax = "400.0"))
	float BackstabDistance;

	//--- Riposte ---
	/** Riposte camera angle (front-side view) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Riposte", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float RiposteAngle;

	/** Riposte camera distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Riposte", meta = (ClampMin = "100.0", ClampMax = "400.0"))
	float RiposteDistance;

	//--- Boss Intro ---
	/** Boss intro camera distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Boss", meta = (ClampMin = "200.0", ClampMax = "1500.0"))
	float BossIntroDistance;

	/** Boss intro duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Boss", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float BossIntroDuration;

	/** Boss phase transition effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Boss", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float BossPhaseTransitionTime;

	//--- General Cinematic ---
	/** Transition time into cinematic camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|General", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float CinematicBlendInTime;

	/** Transition time out of cinematic camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|General", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float CinematicBlendOutTime;

	FModifierParams_Cinematic()
		: ExecutionAngle(60.0f)
		, ExecutionDistance(250.0f)
		, ExecutionHeightOffset(20.0f)
		, BackstabAngle(45.0f)
		, BackstabDistance(200.0f)
		, RiposteAngle(50.0f)
		, RiposteDistance(220.0f)
		, BossIntroDistance(600.0f)
		, BossIntroDuration(3.0f)
		, BossPhaseTransitionTime(1.5f)
		, CinematicBlendInTime(0.3f)
		, CinematicBlendOutTime(0.5f)
	{}
};

//========================================
// C4: Zoom Modifier Parameters (8 params)
//========================================
USTRUCT(BlueprintType)
struct FModifierParams_Zoom
{
	GENERATED_BODY()

	/** FOV reduction on attack impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Attack", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float AttackImpactFOVReduction;

	/** Attack impact effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Attack", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AttackImpactDuration;

	/** FOV pulse on charge release */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Charge", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float ChargeReleaseFOVPulse;

	/** Charge release effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Charge", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChargeReleaseDuration;

	/** FOV change on skill activation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Skill", meta = (ClampMin = "-20.0", ClampMax = "20.0"))
	float SkillActivateFOVChange;

	/** Skill activation effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Skill", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SkillActivateDuration;

	/** Critical hit FOV punch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Critical", meta = (ClampMin = "0.0", ClampMax = "25.0"))
	float CriticalHitFOVPunch;

	/** Critical hit effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom|Critical", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float CriticalHitDuration;

	FModifierParams_Zoom()
		: AttackImpactFOVReduction(5.0f)
		, AttackImpactDuration(0.1f)
		, ChargeReleaseFOVPulse(15.0f)
		, ChargeReleaseDuration(0.3f)
		, SkillActivateFOVChange(-10.0f)
		, SkillActivateDuration(0.5f)
		, CriticalHitFOVPunch(12.0f)
		, CriticalHitDuration(0.15f)
	{}
};

//========================================
// C5: Effect Modifier Parameters (8 params)
//========================================
USTRUCT(BlueprintType)
struct FModifierParams_Effect
{
	GENERATED_BODY()

	/** Health threshold to trigger low health effect (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowHealth", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float LowHealthThreshold;

	/** Vignette intensity at low health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowHealth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthVignetteIntensity;

	/** Subtle shake when at low health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowHealth", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float LowHealthShakeIntensity;

	/** Status ailment vignette color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Status")
	FLinearColor StatusAilmentTint;

	/** Status effect intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Status", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StatusEffectIntensity;

	/** Focus mode FOV reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Focus", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float FocusModeFOVReduction;

	/** Focus mode vignette */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Focus", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FocusModeVignette;

	/** Focus mode transition time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Focus", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float FocusModeTransitionTime;

	FModifierParams_Effect()
		: LowHealthThreshold(0.25f)
		, LowHealthVignetteIntensity(0.3f)
		, LowHealthShakeIntensity(0.2f)
		, StatusAilmentTint(FLinearColor(0.8f, 0.2f, 0.2f, 1.0f))
		, StatusEffectIntensity(0.5f)
		, FocusModeFOVReduction(10.0f)
		, FocusModeVignette(0.2f)
		, FocusModeTransitionTime(0.3f)
	{}
};

//========================================
// C6: Special Modifier Parameters (6 params)
//========================================
USTRUCT(BlueprintType)
struct FModifierParams_Special
{
	GENERATED_BODY()

	/** Time dilation during slow motion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|SlowMo", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float SlowMotionTimeDilation;

	/** Slow motion default duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|SlowMo", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float SlowMotionDuration;

	/** Hit stop duration (frame freeze) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float HitStopDuration;

	/** Death camera pull back distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Death", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float DeathCamPullBackDistance;

	/** Death camera rise height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Death", meta = (ClampMin = "0.0", ClampMax = "300.0"))
	float DeathCamRiseHeight;

	/** Death camera effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special|Death", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float DeathCamDuration;

	FModifierParams_Special()
		: SlowMotionTimeDilation(0.3f)
		, SlowMotionDuration(1.0f)
		, HitStopDuration(0.05f)
		, DeathCamPullBackDistance(200.0f)
		, DeathCamRiseHeight(100.0f)
		, DeathCamDuration(3.0f)
	{}
};

//========================================
// Complete C-Group Structure (58 params)
//========================================
/**
 * Complete modifier parameters (C-Group)
 * Contains all 58 parameters for modifier behavior configuration
 * 
 * Parameter count by sub-group:
 * - C1 Shake:     10 params
 * - C2 Reaction:  14 params
 * - C3 Cinematic: 12 params
 * - C4 Zoom:       8 params
 * - C5 Effect:     8 params
 * - C6 Special:    6 params
 */
USTRUCT(BlueprintType)
struct FModifierParams
{
	GENERATED_BODY()

	/** C1: Shake parameters (10 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C1_Shake")
	FModifierParams_Shake Shake;

	/** C2: Reaction parameters (14 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C2_Reaction")
	FModifierParams_Reaction Reaction;

	/** C3: Cinematic parameters (12 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C3_Cinematic")
	FModifierParams_Cinematic Cinematic;

	/** C4: Zoom parameters (8 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C4_Zoom")
	FModifierParams_Zoom Zoom;

	/** C5: Effect parameters (8 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C5_Effect")
	FModifierParams_Effect Effect;

	/** C6: Special parameters (6 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C6_Special")
	FModifierParams_Special Special;

	FModifierParams()
	{
		// All sub-structs use their own default constructors
	}
};
