// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modifiers/CameraModifier_Zoom.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModifier_Zoom.cpp
 * 
 * This file contains the implementations of the Zoom modifier base class
 * and all 4 Zoom modifiers (Z01-Z04).
 * 
 * Zoom modifiers provide FOV and distance pulse effects for combat feedback.
 * Unlike Shake modifiers that add position noise, Zoom modifiers create
 * smooth FOV/distance changes with specific curves.
 * 
 * Implementation Structure:
 * - ZoomBase: Common zoom functionality
 * - Z01: AttackImpact - Quick punch for hit feedback
 * - Z02: ChargeRelease - Explosive burst for power release
 * - Z03: SkillActivate - Sustained focus for skill channeling
 * - Z04: CriticalHit - Most dramatic multi-layer effect
 * 
 * Effect Types:
 * - Punch: Quick peak, slow decay (Z01, Z04)
 * - Burst: Quick expansion, medium decay (Z02)
 * - Sustain: Gradual in, hold, gradual out (Z03)
 */


//========================================
// ZoomBase - Common Zoom Functionality
//========================================

/**
 * Constructor - Initialize default zoom parameters
 * 
 * Default values provide a subtle zoom effect.
 * Derived classes will configure specific parameters.
 */
UCameraModifier_ZoomBase::UCameraModifier_ZoomBase()
	: BaseFOVChange(0.0f)
	, BaseDistanceChange(0.0f)
	, bApplyVignette(false)
	, ZoomVignetteIntensity(0.1f)
	, FOVInterpSpeed(15.0f)
	, CurrentFOVChange(0.0f)
	, TargetFOVChange(0.0f)
	, CurrentDistanceChange(0.0f)
{
}

/**
 * OnTriggered - Initialize zoom state when triggered
 * 
 * Resets zoom values for fresh effect.
 */
void UCameraModifier_ZoomBase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Reset zoom state
	CurrentFOVChange = 0.0f;
	TargetFOVChange = 0.0f;
	CurrentDistanceChange = 0.0f;
	
	UE_LOG(LogTemp, Verbose, TEXT("ZoomBase: Triggered with intensity %.2f"), CurrentIntensity);
}

/**
 * ComputeEffect - Base zoom effect computation
 * 
 * Applies interpolation to smooth FOV/distance changes.
 * Derived classes set TargetFOVChange and this method smooths it.
 */
void UCameraModifier_ZoomBase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Interpolate FOV change
	CurrentFOVChange = FMath::FInterpTo(CurrentFOVChange, TargetFOVChange, DeltaTime, FOVInterpSpeed);
	
	// Apply weight and intensity
	float EffectiveWeight = CurrentWeight * CurrentIntensity;
	
	// Set FOV effect
	FOVEffect = CurrentFOVChange * EffectiveWeight;
	
	// Set distance effect
	DistanceEffect = CurrentDistanceChange * EffectiveWeight;
	
	// Apply vignette if enabled
	if (bApplyVignette && FMath::Abs(CurrentFOVChange) > 0.1f)
	{
		// Vignette intensity scales with zoom amount
		float VignetteScale = FMath::Abs(CurrentFOVChange) / FMath::Max(FMath::Abs(BaseFOVChange), 1.0f);
		VignetteEffect = ZoomVignetteIntensity * VignetteScale * EffectiveWeight;
	}
	else
	{
		VignetteEffect = 0.0f;
	}
}

/**
 * CalculateFOVPunch - Quick peak, slow decay curve
 * 
 * Used for attack impacts and critical hits.
 * Creates a sharp initial response that fades out.
 * 
 * Curve shape:
 *   ___
 *  /   \____
 * 0%      100%
 * 
 * @param NormalizedTime Current time in effect (0-1)
 * @param PunchAmount Maximum FOV change
 * @return FOV change value
 */
float UCameraModifier_ZoomBase::CalculateFOVPunch(float NormalizedTime, float PunchAmount) const
{
	float PunchCurve;
	
	if (NormalizedTime < 0.1f)
	{
		// Very fast ramp up (0-10%)
		PunchCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.1f, 3.0f);
	}
	else
	{
		// Gradual decay (10-100%)
		float DecayProgress = (NormalizedTime - 0.1f) / 0.9f;
		PunchCurve = FMath::InterpEaseIn(1.0f, 0.0f, DecayProgress, 2.0f);
	}
	
	return PunchAmount * PunchCurve;
}

/**
 * CalculateFOVBurst - Quick expansion, medium decay curve
 * 
 * Used for charge releases.
 * Creates a burst effect that settles back.
 * 
 * Curve shape:
 * ___
 *    \___
 *       \____
 * 0%       100%
 * 
 * @param NormalizedTime Current time in effect (0-1)
 * @param BurstAmount Maximum FOV expansion
 * @return FOV change value
 */
float UCameraModifier_ZoomBase::CalculateFOVBurst(float NormalizedTime, float BurstAmount) const
{
	float BurstCurve;
	
	if (NormalizedTime < 0.15f)
	{
		// Quick burst up (0-15%)
		BurstCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.15f, 2.5f);
	}
	else
	{
		// Medium decay (15-100%)
		float DecayProgress = (NormalizedTime - 0.15f) / 0.85f;
		BurstCurve = FMath::InterpEaseOut(1.0f, 0.0f, DecayProgress, 1.5f);
	}
	
	return BurstAmount * BurstCurve;
}

/**
 * CalculateFOVSustain - Gradual in, hold, gradual out curve
 * 
 * Used for skill activation.
 * Creates a sustained effect with smooth transitions.
 * 
 * Curve shape:
 *     ____
 *    /    \
 * __/      \__
 * 0%  50%   100%
 * 
 * @param NormalizedTime Current time in effect (0-1)
 * @param TargetChange Target FOV change
 * @param HoldRatio How much of duration to hold at peak (0-1)
 * @return FOV change value
 */
float UCameraModifier_ZoomBase::CalculateFOVSustain(float NormalizedTime, float TargetChange, float HoldRatio) const
{
	// Calculate phase boundaries
	float RampInEnd = (1.0f - HoldRatio) * 0.5f;
	float HoldEnd = RampInEnd + HoldRatio;
	
	float SustainCurve;
	
	if (NormalizedTime < RampInEnd)
	{
		// Ramp in
		float RampProgress = NormalizedTime / RampInEnd;
		SustainCurve = FMath::InterpEaseInOut(0.0f, 1.0f, RampProgress, 2.0f);
	}
	else if (NormalizedTime < HoldEnd)
	{
		// Hold at peak
		SustainCurve = 1.0f;
	}
	else
	{
		// Ramp out
		float RampOutProgress = (NormalizedTime - HoldEnd) / (1.0f - HoldEnd);
		SustainCurve = FMath::InterpEaseInOut(1.0f, 0.0f, RampOutProgress, 2.0f);
	}
	
	return TargetChange * SustainCurve;
}

/**
 * CalculateDistancePulse - Distance change with same timing as FOV
 * 
 * Typically mirrors the FOV change for cohesive effect.
 * 
 * @param NormalizedTime Current time in effect (0-1)
 * @param PullAmount Distance change amount
 * @return Distance change value
 */
float UCameraModifier_ZoomBase::CalculateDistancePulse(float NormalizedTime, float PullAmount) const
{
	// Use same curve as punch for consistency
	return CalculateFOVPunch(NormalizedTime, PullAmount);
}


//========================================
// Z01: Attack Impact Zoom Implementation
//========================================

/**
 * Constructor - Configure attack impact zoom parameters
 * 
 * Attack impact creates a quick FOV punch:
 * - Fast onset for immediate feedback
 * - Brief tightening for impact feel
 * - Quick recovery to not disrupt gameplay
 * - Can stack for combo hits
 */
UCameraModifier_Z01_Zoom_AttackImpact::UCameraModifier_Z01_Zoom_AttackImpact()
	: FOVPunchAmount(-5.0f)
	, DistancePullAmount(-15.0f)
	, bScaleWithHitStrength(true)
{
	// Configure base zoom settings
	BaseFOVChange = -5.0f;
	BaseDistanceChange = -15.0f;
	bApplyVignette = false;  // Too quick for vignette
	FOVInterpSpeed = 25.0f;  // Fast response
}

/**
 * OnTriggered - Initialize attack impact state
 * 
 * Scales effect based on attack intensity (damage, weapon type, etc.)
 */
void UCameraModifier_Z01_Zoom_AttackImpact::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Scale based on attack intensity
	float IntensityScale = 1.0f;
	if (bScaleWithHitStrength)
	{
		IntensityScale = FMath::Clamp(CurrentIntensity, 0.5f, 2.0f);
	}
	
	// Apply scaling to punch amounts
	FOVPunchAmount = -5.0f * IntensityScale;
	DistancePullAmount = -15.0f * IntensityScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("Z01 AttackImpact: Triggered, Intensity=%.2f, FOVPunch=%.1f"),
		CurrentIntensity, FOVPunchAmount);
}

/**
 * ComputeEffect - Calculate attack impact FOV punch each frame
 * 
 * Uses punch curve: instant peak, quick decay.
 * Effect is very brief (0.15s) for responsive feedback.
 */
void UCameraModifier_Z01_Zoom_AttackImpact::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Calculate FOV punch using base class helper
	TargetFOVChange = CalculateFOVPunch(NormalizedTime, FOVPunchAmount);
	
	// Calculate distance pull
	CurrentDistanceChange = CalculateDistancePulse(NormalizedTime, DistancePullAmount);
	
	// Call base to apply interpolation and weight
	Super::ComputeEffect(DeltaTime, Context);
}


//========================================
// Z02: Charge Release Zoom Implementation
//========================================

/**
 * Constructor - Configure charge release zoom parameters
 * 
 * Charge release creates an explosive FOV burst: 
 * - Quick expansion for power release feel
 * - Brief hold at peak
 * - Gradual settle back to normal
 * - Scales with charge level
 */
UCameraModifier_Z02_Zoom_ChargeRelease::UCameraModifier_Z02_Zoom_ChargeRelease()
	: FOVBurstAmount(8.0f)
	, DistancePushAmount(25.0f)
	, ChargeLevel(1.0f)
	, MinChargeForFullEffect(0.8f)
{
	// Configure base zoom settings
	BaseFOVChange = 8.0f;
	BaseDistanceChange = 25.0f;
	bApplyVignette = false;
	FOVInterpSpeed = 20.0f;
}

/**
 * OnTriggered - Initialize charge release state
 * 
 * Charge level from trigger data scales the effect. 
 * Higher charge = more dramatic FOV burst. 
 */
void UCameraModifier_Z02_Zoom_ChargeRelease::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Get charge level from intensity (0.5 = half, 1.0 = full, 1.5+ = overcharge)
	ChargeLevel = FMath::Clamp(CurrentIntensity, 0.5f, 2.0f);
	
	// Scale burst amounts based on charge level
	float ChargeScale = ChargeLevel;
	
	// Only apply full effect if charge is above minimum
	if (ChargeLevel < MinChargeForFullEffect)
	{
		ChargeScale *= (ChargeLevel / MinChargeForFullEffect);
	}
	
	FOVBurstAmount = 8.0f * ChargeScale;
	DistancePushAmount = 25.0f * ChargeScale;
	
	UE_LOG(LogTemp, Verbose, TEXT("Z02 ChargeRelease: Triggered, ChargeLevel=%.2f, FOVBurst=%.1f"),
		ChargeLevel, FOVBurstAmount);
}

/**
 * ComputeEffect - Calculate charge release FOV burst each frame
 * 
 * Uses burst curve: explosive expansion, brief hold, gradual settle. 
 * Effect emphasizes the power release moment.
 */
void UCameraModifier_Z02_Zoom_ChargeRelease::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Charge release curve: explosive start, brief hold, gradual settle
	float ReleaseCurve;
	if (NormalizedTime < 0.12f)
	{
		// Explosive expansion (0-12%)
		ReleaseCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.12f, 2.5f);
	}
	else if (NormalizedTime < 0.25f)
	{
		// Brief hold at peak (12-25%)
		ReleaseCurve = 1.0f;
	}
	else
	{
		// Gradual settle (25-100%)
		float SettleProgress = (NormalizedTime - 0.25f) / 0.75f;
		ReleaseCurve = FMath::InterpEaseOut(1.0f, 0.0f, SettleProgress, 1.5f);
	}
	
	// Calculate FOV burst using curve (positive = widen)
	TargetFOVChange = FOVBurstAmount * ReleaseCurve;
	
	// Calculate distance push (positive = farther)
	CurrentDistanceChange = DistancePushAmount * ReleaseCurve;
	
	// Call base to apply interpolation and weight
	Super::ComputeEffect(DeltaTime, Context);
	
	// Additional effects for full charge
	if (ChargeLevel >= 1.0f)
	{
		// Brief time dilation at release moment
		if (NormalizedTime < 0.08f)
		{
			float SlowMoProgress = NormalizedTime / 0.08f;
			TimeDilationEffect = FMath::Lerp(0.7f, 1.0f, SlowMoProgress);
		}
		else
		{
			TimeDilationEffect = 1.0f;
		}
		
		// Subtle screen shake at release
		if (NormalizedTime < 0.15f)
		{
			float ShakeDecay = 1.0f - (NormalizedTime / 0.15f);
			float ShakeIntensity = ShakeDecay * ChargeLevel * 0.8f * CurrentWeight * CurrentIntensity;
			
			RotationEffect.Pitch = FMath::PerlinNoise1D(TotalActiveTime * 35.0f) * ShakeIntensity;
			RotationEffect.Yaw = FMath::PerlinNoise1D(TotalActiveTime * 35.0f + 100.0f) * ShakeIntensity * 0.5f;
			RotationEffect.Roll = 0.0f;
		}
		else
		{
			RotationEffect = FRotator::ZeroRotator;
		}
	}
}


//========================================
// Z03: Skill Activate Zoom Implementation
//========================================

/**
 * Constructor - Configure skill activate zoom parameters
 * 
 * Skill activation creates a sustained focus effect:
 * - Gradual FOV tightening for focus feel
 * - Hold at peak during skill activation
 * - Gradual return to normal
 * - Vignette for concentration effect
 */
UCameraModifier_Z03_Zoom_SkillActivate::UCameraModifier_Z03_Zoom_SkillActivate()
	: FocusFOVAmount(-6.0f)
	, SkillDistanceChange(-20.0f)
	, HoldRatio(0.4f)
	, bIsAoESkill(false)
{
	// Configure base zoom settings
	BaseFOVChange = -6.0f;
	BaseDistanceChange = -20.0f;
	bApplyVignette = true;
	ZoomVignetteIntensity = 0.12f;
	FOVInterpSpeed = 12.0f;  // Smoother for sustained effect
}

/**
 * OnTriggered - Initialize skill activate state
 * 
 * Skill type can affect zoom direction:
 * - Single target: FOV tightens (focus on target)
 * - AoE: FOV widens (see the area effect)
 */
void UCameraModifier_Z03_Zoom_SkillActivate::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Check if AoE skill (could be passed via CustomFloat or CustomInt)
	bIsAoESkill = (InTriggerData.CustomFloat > 0.5f);
	
	// Scale effect based on intensity (skill power)
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.6f, 1.5f);
	
	if (bIsAoESkill)
	{
		// AoE skills: widen FOV to show area
		FocusFOVAmount = 5.0f * IntensityScale;
		SkillDistanceChange = 30.0f * IntensityScale;  // Pull back to see area
	}
	else
	{
		// Single target: tighten FOV for focus
		FocusFOVAmount = -6.0f * IntensityScale;
		SkillDistanceChange = -20.0f * IntensityScale;  // Push in for focus
	}
	
	UE_LOG(LogTemp, Verbose, TEXT("Z03 SkillActivate: Triggered, IsAoE=%d, FocusFOV=%.1f"),
		bIsAoESkill ? 1 : 0, FocusFOVAmount);
}

/**
 * ComputeEffect - Calculate skill activation focus zoom each frame
 * 
 * Uses sustain curve: gradual in, hold at peak, gradual out.
 * Effect provides visual feedback for skill channeling.
 */
void UCameraModifier_Z03_Zoom_SkillActivate::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Calculate FOV using sustain curve from base class
	TargetFOVChange = CalculateFOVSustain(NormalizedTime, FocusFOVAmount, HoldRatio);
	
	// Calculate distance using same sustain pattern
	float DistanceCurve = CalculateFOVSustain(NormalizedTime, 1.0f, HoldRatio);
	CurrentDistanceChange = SkillDistanceChange * DistanceCurve;
	
	// Call base to apply interpolation and weight
	Super::ComputeEffect(DeltaTime, Context);
	
	// Vignette scales with zoom amount for focus effect
	// (already handled in base class, but we can enhance)
	if (bApplyVignette && !bIsAoESkill)
	{
		// Extra vignette during hold phase for focus feel
		float VignetteBoost = 0.0f;
		float RampInEnd = (1.0f - HoldRatio) * 0.5f;
		float HoldEnd = RampInEnd + HoldRatio;
		
		if (NormalizedTime >= RampInEnd && NormalizedTime < HoldEnd)
		{
			VignetteBoost = 0.05f;
		}
		
		VignetteEffect += VignetteBoost * CurrentWeight * CurrentIntensity;
	}
	
	// Slight saturation reduction for magical/focused feel
	if (NormalizedTime > 0.1f && NormalizedTime < 0.9f)
	{
		float SaturationCurve = FMath::Sin((NormalizedTime - 0.1f) / 0.8f * PI);
		SaturationEffect = FMath::Lerp(1.0f, 0.9f, SaturationCurve * 0.3f);
	}
	else
	{
		SaturationEffect = 1.0f;
	}
}


//========================================
// Z04: Critical Hit Zoom Implementation
//========================================

/**
 * Constructor - Configure critical hit zoom parameters
 * 
 * Critical hit creates the most dramatic zoom effect: 
 * - Significant FOV tightening for focus
 * - Strong distance pull for impact
 * - Heavy vignette for dramatic framing
 * - Optional time dilation for emphasis
 * - Cannot be interrupted (critical moment completes)
 */
UCameraModifier_Z04_Zoom_CriticalHit::UCameraModifier_Z04_Zoom_CriticalHit()
	: CriticalFOVChange(-10.0f)
	, CriticalDistancePull(-40.0f)
	, CriticalVignetteIntensity(0.18f)
	, CriticalTimeDilation(0.65f)
	, CriticalSaturation(0.85f)
{
	// Configure base zoom settings
	BaseFOVChange = -10.0f;
	BaseDistanceChange = -40.0f;
	bApplyVignette = true;
	ZoomVignetteIntensity = 0.18f;
	FOVInterpSpeed = 18.0f;  // Fast but not instant
}

/**
 * OnTriggered - Initialize critical hit state
 * 
 * Critical hits are special moments that deserve emphasis. 
 * Effect scales with damage multiplier passed as intensity.
 */
void UCameraModifier_Z04_Zoom_CriticalHit::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Scale based on critical damage multiplier
	// Intensity 1.0 = normal crit, 1.5 = super crit, 2.0 = ultra crit
	float CritScale = FMath::Clamp(CurrentIntensity, 0.8f, 2.0f);
	
	// Apply scaling to critical effects
	CriticalFOVChange = -10.0f * CritScale;
	CriticalDistancePull = -40.0f * CritScale;
	CriticalVignetteIntensity = 0.18f * FMath::Min(CritScale, 1.5f);  // Cap vignette
	
	// Stronger crits get more time dilation
	if (CritScale >= 1.5f)
	{
		CriticalTimeDilation = 0.5f;  // 50% speed for super crits
	}
	else
	{
		CriticalTimeDilation = 0.65f;  // 65% speed for normal crits
	}
	
	UE_LOG(LogTemp, Verbose, TEXT("Z04 CriticalHit: Triggered, CritScale=%.2f, FOV=%.1f, TimeDilation=%.2f"),
		CritScale, CriticalFOVChange, CriticalTimeDilation);
}

/**
 * ComputeEffect - Calculate critical hit dramatic zoom each frame
 * 
 * Uses a punchy curve with hold: quick in, brief hold, medium out. 
 * This is the most dramatic of all zoom effects. 
 */
void UCameraModifier_Z04_Zoom_CriticalHit::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time (0-1 over duration)
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Critical hit curve: fast in, brief hold, gradual out
	// More dramatic than regular attack impact
	float CritCurve;
	if (NormalizedTime < 0.08f)
	{
		// Very fast zoom in (0-8%)
		CritCurve = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime / 0.08f, 3.0f);
	}
	else if (NormalizedTime < 0.25f)
	{
		// Hold at peak (8-25%)
		CritCurve = 1.0f;
	}
	else if (NormalizedTime < 0.40f)
	{
		// Slight release from peak (25-40%)
		float ReleaseProgress = (NormalizedTime - 0.25f) / 0.15f;
		CritCurve = FMath::Lerp(1.0f, 0.85f, ReleaseProgress);
	}
	else
	{
		// Gradual return to normal (40-100%)
		float ReturnProgress = (NormalizedTime - 0.40f) / 0.60f;
		CritCurve = FMath::InterpEaseIn(0.85f, 0.0f, ReturnProgress, 2.0f);
	}
	
	// Calculate FOV change (negative = tighten)
	TargetFOVChange = CriticalFOVChange * CritCurve;
	
	// Calculate distance pull (negative = closer)
	CurrentDistanceChange = CriticalDistancePull * CritCurve;
	
	// Call base to apply interpolation and weight
	Super::ComputeEffect(DeltaTime, Context);
	
	// Override vignette with critical-specific intensity
	float VignetteCurve = CritCurve;
	// Extra vignette punch at peak
	if (NormalizedTime > 0.05f && NormalizedTime < 0.30f)
	{
		float PeakProgress = (NormalizedTime - 0.05f) / 0.25f;
		float PeakBoost = FMath::Sin(PeakProgress * PI) * 0.08f;
		VignetteCurve = FMath::Min(1.0f, CritCurve + PeakBoost);
	}
	VignetteEffect = CriticalVignetteIntensity * VignetteCurve * CurrentWeight * CurrentIntensity;
	
	// Time dilation: slow during impact, return to normal
	if (NormalizedTime < 0.05f)
	{
		// Instant slow-mo
		TimeDilationEffect = CriticalTimeDilation;
	}
	else if (NormalizedTime < 0.25f)
	{
		// Hold slow-mo during peak
		TimeDilationEffect = CriticalTimeDilation;
	}
	else if (NormalizedTime < 0.40f)
	{
		// Gradual return to normal
		float ReturnProgress = (NormalizedTime - 0.25f) / 0.15f;
		TimeDilationEffect = FMath::Lerp(CriticalTimeDilation, 1.0f, FMath::InterpEaseIn(0.0f, 1.0f, ReturnProgress, 2.0f));
	}
	else
	{
		TimeDilationEffect = 1.0f;
	}
	
	// Saturation reduction for dramatic effect during peak
	if (NormalizedTime < 0.30f)
	{
		float SatProgress = NormalizedTime / 0.30f;
		float SatCurve = FMath::Sin(SatProgress * PI);
		SaturationEffect = FMath::Lerp(1.0f, CriticalSaturation, SatCurve);
	}
	else if (NormalizedTime < 0.50f)
	{
		// Gradual return
		float ReturnProgress = (NormalizedTime - 0.30f) / 0.20f;
		SaturationEffect = FMath::Lerp(CriticalSaturation, 1.0f, ReturnProgress);
	}
	else
	{
		SaturationEffect = 1.0f;
	}
	
	// Slight chromatic aberration hint during peak (using color tint as approximation)
	if (NormalizedTime > 0.03f && NormalizedTime < 0.20f)
	{
		float AberrationProgress = (NormalizedTime - 0.03f) / 0.17f;
		float AberrationCurve = FMath::Sin(AberrationProgress * PI);
		// Subtle red/blue shift
		float Shift = AberrationCurve * 0.02f * CurrentWeight * CurrentIntensity;
		ColorTintEffect = FLinearColor(1.0f + Shift, 1.0f, 1.0f - Shift * 0.5f, 1.0f);
	}
	else
	{
		ColorTintEffect = FLinearColor::White;
	}
}
