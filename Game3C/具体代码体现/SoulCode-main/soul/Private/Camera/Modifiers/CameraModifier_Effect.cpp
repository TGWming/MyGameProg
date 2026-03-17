// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modifiers/CameraModifier_Effect.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModifier_Effect.cpp
 * 
 * This file contains the implementations of the Effect modifier base class
 * and all 3 Effect modifiers (E01-E03).
 * 
 * Effect modifiers provide persistent visual effects that reflect player state.
 * Unlike brief modifiers (Shake/Zoom), Effect modifiers can run continuously
 * and have their intensity dynamically updated.
 * 
 * Implementation Structure:
 * - EffectBase: Common effect functionality
 * - E01: LowHealth - Critical health visual feedback
 * - E02: StatusAilment - Toxic/curse status effect
 * - E03: FocusMode - Concentration mode enhancement
 * 
 * Key Feature:
 * Effect modifiers support dynamic intensity updates via UpdateIntensity(),
 * allowing effects to scale with game state (e.g., health percentage).
 * 
 * Smooth Intensity Transition:
 * All effect modifiers now use SmoothedIntensity which interpolates
 * towards TargetIntensity using FInterpTo for frame-rate independent
 * smooth transitions. This prevents jarring visual changes when
 * effects activate, deactivate, or change intensity.
 */


//========================================
// EffectBase - Common Effect Functionality
//========================================

/**
 * Constructor - Initialize default effect parameters
 * 
 * Default values provide subtle visual enhancement.
 * Derived classes configure specific effect parameters.
 */
UCameraModifier_EffectBase::UCameraModifier_EffectBase()
	: EffectVignetteIntensity(0.15f)
	, EffectSaturation(1.0f)
	, EffectColorTint(FLinearColor::White)
	, bShouldPulse(false)
	, PulseSpeed(1.0f)
	, PulseAmount(0.1f)
	, IntensityInterpSpeed(5.0f)
	, TargetIntensity(1.0f)
	, DynamicIntensity(0.0f)
	, SmoothedIntensity(0.0f)
	, PulseTimer(0.0f)
{
}

/**
 * Trigger - Activate the effect modifier
 * 
 * Sets target intensity from trigger data.
 * SmoothedIntensity will interpolate towards target in Update.
 */
void UCameraModifier_EffectBase::Trigger(const FModifierTriggerData& InTriggerData)
{
	// Call parent trigger
	Super::Trigger(InTriggerData);
	
	// Set target intensity from trigger data
	TargetIntensity = InTriggerData.Intensity;
	
	// Note: Do NOT reset SmoothedIntensity to 0 here
	// Let it naturally interpolate from current value to target
	// This allows smooth transitions when re-triggering an active effect
	
	UE_LOG(LogTemp, Verbose, TEXT("EffectBase::Trigger - TargetIntensity=%.2f, SmoothedIntensity=%.2f"),
		TargetIntensity, SmoothedIntensity);
}

/**
 * Stop - Deactivate the effect modifier
 * 
 * @param bImmediate If true, immediately set intensity to 0
 */
void UCameraModifier_EffectBase::Stop(bool bImmediate)
{
	if (bImmediate)
	{
		// Immediate stop - reset both target and smoothed intensity
		SmoothedIntensity = 0.0f;
		TargetIntensity = 0.0f;
		DynamicIntensity = 0.0f;
	}
	else
	{
		// Gradual fade out - set target to 0, let interpolation handle the rest
		TargetIntensity = 0.0f;
		// SmoothedIntensity will naturally interpolate to 0 in Update
	}
	
	// Call parent stop
	Super::Stop(bImmediate);
	
	UE_LOG(LogTemp, Verbose, TEXT("EffectBase::Stop - bImmediate=%d, TargetIntensity=%.2f, SmoothedIntensity=%.2f"),
		bImmediate ? 1 : 0, TargetIntensity, SmoothedIntensity);
}

/**
 * Update - Update effect state each frame
 * 
 * Handles smooth intensity interpolation before calling parent Update.
 */
void UCameraModifier_EffectBase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
	// Update smoothed intensity towards target
	UpdateSmoothedIntensity(DeltaTime);
	
	// Call parent update
	Super::Update(DeltaTime, Context);
}

/**
 * UpdateSmoothedIntensity - Interpolate smoothed intensity towards target
 * 
 * Uses FInterpTo for frame-rate independent smooth interpolation.
 * This ensures consistent transition speed regardless of frame rate.
 * 
 * @param DeltaTime Time since last frame
 */
void UCameraModifier_EffectBase::UpdateSmoothedIntensity(float DeltaTime)
{
	// Use FInterpTo for frame-rate independent smooth interpolation
	SmoothedIntensity = FMath::FInterpTo(
		SmoothedIntensity,
		TargetIntensity,
		DeltaTime,
		IntensityInterpSpeed
	);
	
	// Also update DynamicIntensity for backwards compatibility
	DynamicIntensity = SmoothedIntensity;
}

/**
 * OnTriggered - Initialize effect state when triggered
 * 
 * Sets up initial intensity and resets pulse timer.
 */
void UCameraModifier_EffectBase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Initialize target intensity from trigger data
	TargetIntensity = CurrentIntensity;
	// SmoothedIntensity starts at current value (may be 0 or may have residual from previous activation)
	// Do NOT reset to 0 - allow natural interpolation
	PulseTimer = 0.0f;
	
	UE_LOG(LogTemp, Verbose, TEXT("EffectBase::OnTriggered - TargetIntensity=%.2f, SmoothedIntensity=%.2f"),
		TargetIntensity, SmoothedIntensity);
}

/**
 * UpdateIntensity - Dynamically update effect intensity
 * 
 * Allows effects to scale with game state (e.g., health level).
 * The actual effect will smoothly interpolate to the new intensity.
 * 
 * @param NewIntensity New target intensity (0-1 typical, can exceed)
 */
void UCameraModifier_EffectBase::UpdateIntensity(float NewIntensity)
{
	TargetIntensity = FMath::Max(0.0f, NewIntensity);
	
	// Also update base intensity for weight calculations
	CurrentIntensity = TargetIntensity;
	
	// Note: SmoothedIntensity will interpolate to new target in Update
}

/**
 * ComputeEffect - Base effect computation
 * 
 * Uses SmoothedIntensity for all effect calculations.
 * Handles intensity interpolation and pulse calculation.
 * Derived classes should call Super and add specific effects.
 */
void UCameraModifier_EffectBase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Update pulse timer
	if (bShouldPulse)
	{
		PulseTimer += DeltaTime * PulseSpeed;
	}
	
	// Calculate effective weight using SmoothedIntensity for smooth transitions
	float EffectiveWeight = CurrentWeight * SmoothedIntensity;
	
	// Apply base effects
	VignetteEffect = EffectVignetteIntensity * EffectiveWeight;
	SaturationEffect = FMath::Lerp(1.0f, EffectSaturation, EffectiveWeight);
	ColorTintEffect = FMath::Lerp(FLinearColor::White, EffectColorTint, EffectiveWeight);
}

/**
 * CalculatePulse - Calculate pulsing effect value
 * 
 * Creates a smooth oscillation for visual feedback.
 * 
 * @param BaseValue Base value to pulse around
 * @param PulseAmt Amount of variation
 * @param Speed Pulse speed (already factored into PulseTimer)
 * @return Pulsed value
 */
float UCameraModifier_EffectBase::CalculatePulse(float BaseValue, float PulseAmt, float Speed) const
{
	float PulseWave = FMath::Sin(PulseTimer * PI * 2.0f);
	return BaseValue + (PulseWave * PulseAmt);
}

/**
 * CalculateHeartbeatPulse - Calculate heartbeat-style pulse
 * 
 * Creates a double-beat pattern like a heartbeat for critical states.
 * Pattern: beat-beat-pause-beat-beat-pause...
 * 
 * @param Intensity Effect intensity (affects pulse amplitude)
 * @return Heartbeat curve value (0-1)
 */
float UCameraModifier_EffectBase::CalculateHeartbeatPulse(float Intensity) const
{
	// Heartbeat pattern: two quick pulses followed by pause
	float CyclePosition = FMath::Fmod(PulseTimer, 1.0f);
	
	float Beat = 0.0f;
	
	// First beat (0.0 - 0.15)
	if (CyclePosition < 0.15f)
	{
		float BeatProgress = CyclePosition / 0.15f;
		Beat = FMath::Sin(BeatProgress * PI);
	}
	// Second beat (0.2 - 0.35)
	else if (CyclePosition >= 0.2f && CyclePosition < 0.35f)
	{
		float BeatProgress = (CyclePosition - 0.2f) / 0.15f;
		Beat = FMath::Sin(BeatProgress * PI) * 0.7f;  // Second beat slightly weaker
	}
	// Pause (0.35 - 1.0)
	
	return Beat * Intensity;
}


//========================================
// E01: Low Health Effect Implementation
//========================================

/**
 * Constructor - Configure low health effect parameters
 * 
 * Low health creates urgent visual feedback:
 * - Red vignette that pulses with heartbeat
 * - Desaturation for danger feel
 * - Slight FOV reduction (tunnel vision)
 * - Pulse rate increases as health decreases
 */
UCameraModifier_E01_Effect_LowHealth::UCameraModifier_E01_Effect_LowHealth()
	: HealthThreshold(0.3f)
	, MaxVignetteIntensity(0.35f)
	, MaxFOVReduction(5.0f)
	, LowHealthTint(FLinearColor(1.0f, 0.3f, 0.3f, 1.0f))
	, CurrentHealthPercent(1.0f)
{
	// Configure base effect settings
	EffectVignetteIntensity = 0.35f;
	EffectSaturation = 0.7f;
	EffectColorTint = LowHealthTint;
	bShouldPulse = true;
	PulseSpeed = 1.2f;  // Base heartbeat rate
	PulseAmount = 0.15f;
	IntensityInterpSpeed = 3.0f;  // Slower for smooth health transitions
}

/**
 * OnTriggered - Initialize low health state
 * 
 * Intensity represents how critical health is (1.0 = very low, 0.0 = at threshold).
 */
void UCameraModifier_E01_Effect_LowHealth::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Intensity represents severity (1.0 - health_percent/threshold)
	// Higher intensity = lower health = more urgent effect
	CurrentHealthPercent = 1.0f - FMath::Clamp(CurrentIntensity, 0.0f, 1.0f);
	
	// Adjust pulse rate based on health (faster when more critical)
	PulseSpeed = FMath::Lerp(1.0f, 2.5f, CurrentIntensity);
	
	UE_LOG(LogTemp, Verbose, TEXT("E01 LowHealth: Triggered, HealthPercent=%.2f, PulseSpeed=%.2f, SmoothedIntensity=%.2f"),
		CurrentHealthPercent, PulseSpeed, SmoothedIntensity);
}

/**
 * UpdateHealthPercent - Update effect based on health percentage
 * 
 * Calculates target intensity from health value.
 * SmoothedIntensity will interpolate towards target for smooth transitions.
 * 
 * @param HealthPercent Current health percentage (0-1)
 */
void UCameraModifier_E01_Effect_LowHealth::UpdateHealthPercent(float HealthPercent)
{
	// Store current health percentage
	CurrentHealthPercent = FMath::Clamp(HealthPercent, 0.0f, 1.0f);
	
	// Calculate target intensity based on health threshold
	// Effect starts when health drops below threshold
	// Intensity increases as health decreases further
	if (CurrentHealthPercent < HealthThreshold)
	{
		// Map health to intensity: at threshold = 0, at 0% = 1
		TargetIntensity = 1.0f - (CurrentHealthPercent / HealthThreshold);
	}
	else
	{
		// Above threshold - no effect
		TargetIntensity = 0.0f;
	}
	
	// Note: SmoothedIntensity will interpolate to target in Update
	// This ensures smooth visual transitions as health changes
	
	UE_LOG(LogTemp, Verbose, TEXT("E01 LowHealth: UpdateHealthPercent=%.2f, TargetIntensity=%.2f"),
		CurrentHealthPercent, TargetIntensity);
}

/**
 * ComputeEffect - Calculate low health visual feedback each frame
 * 
 * Creates pulsing red vignette with heartbeat pattern.
 * Uses SmoothedIntensity for smooth transitions.
 */
void UCameraModifier_E01_Effect_LowHealth::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Update pulse speed based on SmoothedIntensity (smooth transition)
	PulseSpeed = FMath::Lerp(1.0f, 2.5f, SmoothedIntensity);
	
	// Call base to handle pulse timer update
	Super::ComputeEffect(DeltaTime, Context);
	
	// Calculate heartbeat pulse using SmoothedIntensity
	float HeartbeatValue = CalculateHeartbeatPulse(SmoothedIntensity);
	
	// Calculate effective weight using SmoothedIntensity
	float EffectiveWeight = CurrentWeight * SmoothedIntensity;
	
	// Vignette with heartbeat pulse
	float BaseVignette = MaxVignetteIntensity * EffectiveWeight;
	float PulsedVignette = BaseVignette * (1.0f + HeartbeatValue * 0.3f);
	VignetteEffect = PulsedVignette;
	
	// Red tint with pulse
	float TintStrength = EffectiveWeight * (1.0f + HeartbeatValue * 0.2f);
	ColorTintEffect = FMath::Lerp(FLinearColor::White, LowHealthTint, TintStrength);
	
	// Desaturation (more desaturated when more critical)
	SaturationEffect = FMath::Lerp(1.0f, 0.6f, EffectiveWeight);
	
	// FOV reduction (tunnel vision) with slight pulse
	float FOVChange = MaxFOVReduction * EffectiveWeight * (1.0f + HeartbeatValue * 0.1f);
	FOVEffect = -FOVChange;  // Negative = tighter FOV
}


//========================================
// E02: Status Ailment Effect Implementation
//========================================

/**
 * Constructor - Configure status ailment effect parameters
 * 
 * Status ailment creates unsettling visual feedback:
 * - Green/purple color tint (poison/curse)
 * - Slight vignette
 * - FOV wobble for nausea
 * - Desaturation
 * - Slow pulse for sickly feel
 */
UCameraModifier_E02_Effect_StatusAilment::UCameraModifier_E02_Effect_StatusAilment()
	: PoisonTint(FLinearColor(0.7f, 1.0f, 0.7f, 1.0f))
	, CurseTint(FLinearColor(0.8f, 0.6f, 1.0f, 1.0f))
	, AilmentVignetteIntensity(0.18f)
	, AilmentDesaturation(0.75f)
	, FOVWobbleAmount(2.0f)
	, WobbleSpeed(1.5f)
	, AilmentSeverity(1.0f)
{
	// Configure base effect settings
	EffectVignetteIntensity = 0.18f;
	EffectSaturation = 0.75f;
	EffectColorTint = PoisonTint;
	bShouldPulse = true;
	PulseSpeed = 0.8f;  // Slow, sickly pulse
	PulseAmount = 0.1f;
	IntensityInterpSpeed = 4.0f;
}

/**
 * OnTriggered - Initialize status ailment state
 * 
 * Intensity represents ailment severity.
 * CustomTag determines ailment type (Poison, Curse, etc.)
 */
void UCameraModifier_E02_Effect_StatusAilment::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Store ailment severity
	AilmentSeverity = FMath::Clamp(CurrentIntensity, 0.3f, 1.5f);
	
	// Determine ailment type from custom tag
	if (InTriggerData.CustomTag == FName(TEXT("Curse")))
	{
		EffectColorTint = CurseTint;
	}
	else
	{
		// Default to poison
		EffectColorTint = PoisonTint;
	}
	
	// Adjust effect parameters based on severity
	AilmentVignetteIntensity = 0.18f * AilmentSeverity;
	FOVWobbleAmount = 2.0f * AilmentSeverity;
	
	UE_LOG(LogTemp, Verbose, TEXT("E02 StatusAilment: Triggered, Severity=%.2f, Type=%s, SmoothedIntensity=%.2f"),
		AilmentSeverity, *InTriggerData.CustomTag.ToString(), SmoothedIntensity);
}

/**
 * ComputeEffect - Calculate status ailment visual effect each frame
 * 
 * Creates nauseating effect with color tint and FOV wobble.
 * Uses SmoothedIntensity for smooth transitions.
 */
void UCameraModifier_E02_Effect_StatusAilment::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Call base to handle interpolation and pulse timer
	Super::ComputeEffect(DeltaTime, Context);
	
	// Calculate effective weight using SmoothedIntensity
	float EffectiveWeight = CurrentWeight * SmoothedIntensity;
	
	// Status ailment vignette with slow pulse
	float PulseFactor = CalculatePulse(1.0f, 0.15f, PulseSpeed);
	VignetteEffect = AilmentVignetteIntensity * EffectiveWeight * PulseFactor;
	
	// Color tint with slight variation
	float TintPulse = CalculatePulse(1.0f, 0.1f, PulseSpeed * 0.7f);
	FLinearColor PulsedTint = EffectColorTint;
	PulsedTint.G *= TintPulse;  // Vary green channel
	ColorTintEffect = FMath::Lerp(FLinearColor::White, PulsedTint, EffectiveWeight);
	
	// Desaturation
	SaturationEffect = FMath::Lerp(1.0f, AilmentDesaturation, EffectiveWeight);
	
	// FOV wobble for nausea effect
	float WobbleTime = TotalActiveTime * WobbleSpeed;
	float WobbleX = FMath::Sin(WobbleTime * 2.0f) * FOVWobbleAmount;
	float WobbleY = FMath::Cos(WobbleTime * 1.7f) * FOVWobbleAmount * 0.5f;
	
	FOVEffect = WobbleX * EffectiveWeight;
	
	// Subtle rotation wobble for disorientation
	RotationEffect.Pitch = WobbleY * EffectiveWeight * 0.3f;
	RotationEffect.Roll = FMath::Sin(WobbleTime * 1.3f) * EffectiveWeight * 0.5f;
	RotationEffect.Yaw = 0.0f;
}


//========================================
// E03: Focus Mode Effect Implementation
//========================================

/**
 * Constructor - Configure focus mode effect parameters
 * 
 * Focus creates heightened awareness visual feedback:
 * - Slight vignette for concentration
 * - Subtle FOV reduction (tunnel vision)
 * - Enhanced saturation (heightened senses)
 * - Clean, professional look
 */
UCameraModifier_E03_Effect_FocusMode::UCameraModifier_E03_Effect_FocusMode()
	: FocusFOVReduction(3.0f)
	, FocusVignetteIntensity(0.10f)
	, FocusSaturationBoost(1.08f)
	, FocusTimeDilation(0.95f)
	, bStrongFocus(false)
{
	// Configure base effect settings
	EffectVignetteIntensity = 0.10f;
	EffectSaturation = 1.08f;
	EffectColorTint = FLinearColor::White;  // No tint for focus
	bShouldPulse = false;  // Clean, stable effect
	IntensityInterpSpeed = 8.0f;  // Quick response for aiming
}

/**
 * OnTriggered - Initialize focus state
 * 
 * Strong focus (intensity >= 1.0) provides more pronounced effect.
 */
void UCameraModifier_E03_Effect_FocusMode::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Determine focus strength
	bStrongFocus = (CurrentIntensity >= 1.0f);
	
	// Scale parameters based on focus strength
	float FocusScale = FMath::Clamp(CurrentIntensity, 0.5f, 1.5f);
	
	FocusFOVReduction = 3.0f * FocusScale;
	FocusVignetteIntensity = 0.10f * FocusScale;
	
	if (bStrongFocus)
	{
		// Strong focus gets more pronounced effects
		FocusSaturationBoost = 1.12f;
		FocusTimeDilation = 0.92f;
	}
	else
	{
		FocusSaturationBoost = 1.08f;
		FocusTimeDilation = 0.95f;
	}
	
	UE_LOG(LogTemp, Verbose, TEXT("E03 FocusMode: Triggered, StrongFocus=%d, FOVReduction=%.2f, SmoothedIntensity=%.2f"),
		bStrongFocus ? 1 : 0, FocusFOVReduction, SmoothedIntensity);
}

/**
 * ComputeEffect - Calculate focus visual enhancement each frame
 * 
 * Creates clean, professional visual enhancement for aiming/lock-on.
 * Uses SmoothedIntensity for smooth transitions.
 */
void UCameraModifier_E03_Effect_FocusMode::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Call base to handle interpolation
	Super::ComputeEffect(DeltaTime, Context);
	
	// Calculate effective weight using SmoothedIntensity
	float EffectiveWeight = CurrentWeight * SmoothedIntensity;
	
	// Subtle vignette for focus
	VignetteEffect = FocusVignetteIntensity * EffectiveWeight;
	
	// FOV reduction (tunnel vision for focus)
	FOVEffect = -FocusFOVReduction * EffectiveWeight;
	
	// Saturation boost (heightened senses)
	SaturationEffect = FMath::Lerp(1.0f, FocusSaturationBoost, EffectiveWeight);
	
	// Very subtle color enhancement (slightly warmer for focus)
	ColorTintEffect = FMath::Lerp(
		FLinearColor::White,
		FLinearColor(1.02f, 1.01f, 1.0f, 1.0f),  // Very subtle warm
		EffectiveWeight
	);
	
	// Time dilation hint for focused perception
	TimeDilationEffect = FMath::Lerp(1.0f, FocusTimeDilation, EffectiveWeight);
	
	// No rotation effects - focus should be stable
	RotationEffect = FRotator::ZeroRotator;
}
