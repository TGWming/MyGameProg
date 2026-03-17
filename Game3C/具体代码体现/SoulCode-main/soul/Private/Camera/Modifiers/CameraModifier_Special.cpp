// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modifiers/CameraModifier_Special.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModifier_Special.cpp
 * 
 * This file contains the implementations of the Special modifier base class
 * and all 3 Special modifiers (X01-X03).
 * 
 * Special modifiers provide time control and special camera behaviors for
 * critical gameplay moments like slow motion, hit stops, and death sequences.
 * 
 * Implementation Structure:
 * - SpecialBase: Common time control functionality
 * - X01: SlowMotion
 * - X02: HitStop
 * - X03: DeathCam
 * 
 * Key Feature:
 * Special modifiers have the highest priority and control time dilation,
 * which affects the entire game feel during critical moments.
 */


//========================================
// SpecialBase - Common Special Functionality
//========================================

/**
 * Constructor - Initialize default special parameters
 * 
 * Default values provide a subtle time effect.
 * Derived classes configure specific parameters.
 */
UCameraModifier_SpecialBase::UCameraModifier_SpecialBase()
	: TargetTimeDilation(0.5f)
	, TimeRampIn(0.05f)
	, TimeRampOut(0.2f)
	, bApplyVisualEffects(true)
	, SpecialVignette(0.1f)
	, SpecialSaturation(0.9f)
	, CurrentTimeDilation(1.0f)
	, bAtFullIntensity(false)
{
}

/**
 * OnTriggered - Initialize special state when triggered
 * 
 * Prepares time dilation and visual effects.
 */
void UCameraModifier_SpecialBase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	// Initialize time dilation state
	CurrentTimeDilation = 1.0f;
	bAtFullIntensity = false;
	
	UE_LOG(LogTemp, Verbose, TEXT("SpecialBase: Triggered, TargetTimeDilation=%.2f"), TargetTimeDilation);
}

/**
 * OnActivated - Called when special effect is fully active
 */
void UCameraModifier_SpecialBase::OnActivated()
{
	Super::OnActivated();
	bAtFullIntensity = true;
	
	UE_LOG(LogTemp, Verbose, TEXT("SpecialBase: Fully activated"));
}

/**
 * OnDeactivating - Called when special effect begins to end
 */
void UCameraModifier_SpecialBase::OnDeactivating()
{
	Super::OnDeactivating();
	bAtFullIntensity = false;
	
	UE_LOG(LogTemp, Verbose, TEXT("SpecialBase: Deactivating"));
}

/**
 * ComputeEffect - Base special effect computation
 * 
 * Handles time dilation interpolation and basic visual effects.
 * Derived classes should call Super and add specific effects.
 */
void UCameraModifier_SpecialBase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	// Calculate ramp ratios based on duration
	float RampInRatio = (ActiveDuration > 0.0f) ? FMath::Min(TimeRampIn / ActiveDuration, 0.4f) : 0.0f;
	float RampOutRatio = (ActiveDuration > 0.0f) ? FMath::Min(TimeRampOut / ActiveDuration, 0.4f) : 0.0f;
	
	// Calculate current time dilation
	CurrentTimeDilation = CalculateTimeDilation(NormalizedTime, TargetTimeDilation, RampInRatio, RampOutRatio);
	
	// Apply time dilation effect
	float EffectiveWeight = CurrentWeight * CurrentIntensity;
	TimeDilationEffect = CurrentTimeDilation;
	
	// Apply visual effects if enabled
	if (bApplyVisualEffects)
	{
		// Calculate visual intensity (matches time dilation curve)
		float VisualIntensity = 0.0f;
		if (TargetTimeDilation < 1.0f)
		{
			VisualIntensity = (1.0f - CurrentTimeDilation) / (1.0f - TargetTimeDilation);
			VisualIntensity = FMath::Clamp(VisualIntensity, 0.0f, 1.0f) * EffectiveWeight;
		}
		
		VignetteEffect = SpecialVignette * VisualIntensity;
		SaturationEffect = FMath::Lerp(1.0f, SpecialSaturation, VisualIntensity);
	}
}

/**
 * CalculateTimeDilation - Smooth time dilation with ramp in/out
 * 
 * Creates professional-feeling time control with smooth transitions.
 * 
 * @param NormalizedTime Current time in effect (0-1)
 * @param TargetDilation Target time dilation (e.g., 0.3 for 30% speed)
 * @param RampInRatio Ratio of duration for ramp in (0-0.5)
 * @param RampOutRatio Ratio of duration for ramp out (0-0.5)
 * @return Current time dilation value (1.0 = normal, <1.0 = slow)
 */
float UCameraModifier_SpecialBase::CalculateTimeDilation(float NormalizedTime, float TargetDilation, float RampInRatio, float RampOutRatio) const
{
	float RampOutStart = 1.0f - RampOutRatio;
	
	if (NormalizedTime < RampInRatio && RampInRatio > 0.0f)
	{
		// Ramp in phase: 1.0 -> TargetDilation
		float RampProgress = NormalizedTime / RampInRatio;
		float EasedProgress = FMath::InterpEaseOut(0.0f, 1.0f, RampProgress, 2.0f);
		return FMath::Lerp(1.0f, TargetDilation, EasedProgress);
	}
	else if (NormalizedTime > RampOutStart && RampOutRatio > 0.0f)
	{
		// Ramp out phase: TargetDilation -> 1.0
		float RampProgress = (NormalizedTime - RampOutStart) / RampOutRatio;
		float EasedProgress = FMath::InterpEaseIn(0.0f, 1.0f, RampProgress, 2.0f);
		return FMath::Lerp(TargetDilation, 1.0f, EasedProgress);
	}
	else
	{
		// Hold at target
		return TargetDilation;
	}
}

/**
 * ApplyRadialBlur - Apply radial blur approximation
 * 
 * Uses FOV and vignette to approximate radial blur effect.
 * 
 * @param Intensity Blur intensity (0-1)
 */
void UCameraModifier_SpecialBase::ApplyRadialBlur(float Intensity)
{
	// Approximate radial blur with vignette and slight FOV push
	VignetteEffect += Intensity * 0.15f;
	FOVEffect += Intensity * 3.0f;
}


//========================================
// X01: Slow Motion Implementation
//========================================

/**
 * Constructor - Configure slow motion parameters
 * 
 * Slow motion provides dramatic time dilation:
 * - Configurable time scale
 * - Smooth ramp in and out
 * - Optional visual enhancements
 * - Used for parries, dodges, critical moments
 */
UCameraModifier_X01_Special_SlowMotion::UCameraModifier_X01_Special_SlowMotion()
	: SlowMotionScale(0.35f)
	, bDesaturateDuringSlowMo(true)
	, SlowMoDesaturation(0.15f)
	, SlowMoFOVChange(-3.0f)
{
	// Configure base special settings
	TargetTimeDilation = 0.35f;
	TimeRampIn = 0.05f;
	TimeRampOut = 0.25f;
	bApplyVisualEffects = true;
	SpecialVignette = 0.12f;
	SpecialSaturation = 0.85f;
}

/**
 * OnTriggered - Initialize slow motion state
 * 
 * Intensity can modify the slow motion scale.
 * Higher intensity = slower time.
 */
void UCameraModifier_X01_Special_SlowMotion::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Modify time scale based on intensity
	// Intensity 0.5 = 50% effect (faster), 1.0 = normal, 1.5 = 150% effect (slower)
	float IntensityModifier = FMath::Clamp(CurrentIntensity, 0.5f, 1.5f);
	
	// Lower intensity = faster (closer to 1.0), higher intensity = slower
	SlowMotionScale = FMath::Lerp(0.6f, 0.2f, (IntensityModifier - 0.5f));
	
	// Update base class target
	TargetTimeDilation = SlowMotionScale;
	
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	UE_LOG(LogTemp, Verbose, TEXT("X01 SlowMotion: Triggered, Scale=%.2f, Intensity=%.2f"),
		SlowMotionScale, CurrentIntensity);
}

/**
 * ComputeEffect - Calculate slow motion effect each frame
 * 
 * Applies time dilation with optional visual enhancements.
 */
void UCameraModifier_X01_Special_SlowMotion::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Call base to handle time dilation
	Super::ComputeEffect(DeltaTime, Context);
	
	// Calculate effect intensity based on current time dilation
	float EffectiveWeight = CurrentWeight * CurrentIntensity;
	float SlowMoIntensity = 0.0f;
	if (SlowMotionScale < 1.0f)
	{
		SlowMoIntensity = (1.0f - CurrentTimeDilation) / (1.0f - SlowMotionScale);
		SlowMoIntensity = FMath::Clamp(SlowMoIntensity, 0.0f, 1.0f);
	}
	
	// Apply FOV change during slow motion
	FOVEffect = SlowMoFOVChange * SlowMoIntensity * EffectiveWeight;
	
	// Apply desaturation if enabled
	if (bDesaturateDuringSlowMo)
	{
		float DesatAmount = SlowMoDesaturation * SlowMoIntensity * EffectiveWeight;
		SaturationEffect = FMath::Min(SaturationEffect, 1.0f - DesatAmount);
	}
	
	// Subtle color grade during slow motion (slightly cooler)
	float ColorGradeIntensity = SlowMoIntensity * EffectiveWeight * 0.05f;
	ColorTintEffect = FLinearColor(
		1.0f - ColorGradeIntensity * 0.5f,  // Slightly less red
		1.0f,                                // Normal green
		1.0f + ColorGradeIntensity,          // Slightly more blue
		1.0f
	);
}


//========================================
// X02: Hit Stop Implementation
//========================================

/**
 * Constructor - Configure hit stop parameters
 * 
 * Hit stop creates brief freeze frame:
 * - Near-complete time stop
 * - Very brief duration
 * - Slight FOV punch
 * - Can stack for multi-hit attacks
 */
UCameraModifier_X02_Special_HitStop::UCameraModifier_X02_Special_HitStop()
	: HitStopTimeScale(0.02f)
	, HitStopFOVPunch(-4.0f)
	, bShakeOnRelease(true)
	, ReleaseShakeIntensity(1.5f)
	, HitStopChromaticAberration(0.03f)
	, HitDirection(FVector::ForwardVector)
{
	// Configure base special settings
	TargetTimeDilation = 0.02f;
	TimeRampIn = 0.0f;   // Instant freeze
	TimeRampOut = 0.02f; // Quick release
	bApplyVisualEffects = true;
	SpecialVignette = 0.08f;
	SpecialSaturation = 0.95f;
}

/**
 * OnTriggered - Initialize hit stop state
 * 
 * Intensity affects freeze duration and intensity.
 * Hit direction used for directional effects.
 */
void UCameraModifier_X02_Special_HitStop::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Scale hit stop based on intensity
	float IntensityScale = FMath::Clamp(CurrentIntensity, 0.5f, 2.0f);
	
	// Stronger hits = more frozen and longer
	HitStopTimeScale = FMath::Lerp(0.05f, 0.01f, (IntensityScale - 0.5f) / 1.5f);
	TargetTimeDilation = HitStopTimeScale;
	
	// Store hit direction if provided
	if (!InTriggerData.SourceDirection.IsNearlyZero())
	{
		HitDirection = InTriggerData.SourceDirection.GetSafeNormal();
	}
	else
	{
		HitDirection = FVector::ForwardVector;
	}
	
	// Scale FOV punch with intensity
	HitStopFOVPunch = -4.0f * IntensityScale;
	
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	UE_LOG(LogTemp, Verbose, TEXT("X02 HitStop: Triggered, TimeScale=%.3f, Intensity=%.2f"),
		HitStopTimeScale, CurrentIntensity);
}

/**
 * ComputeEffect - Calculate hit stop effect each frame
 * 
 * Creates near-frozen moment with visual punch.
 */
void UCameraModifier_X02_Special_HitStop::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Calculate normalized time
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	float EffectiveWeight = CurrentWeight * CurrentIntensity;
	
	// Hit stop curve: instant freeze, quick release
	float FreezeIntensity;
	if (NormalizedTime < 0.7f)
	{
		// Full freeze
		FreezeIntensity = 1.0f;
		TimeDilationEffect = HitStopTimeScale;
	}
	else
	{
		// Quick release
		float ReleaseProgress = (NormalizedTime - 0.7f) / 0.3f;
		FreezeIntensity = 1.0f - FMath::InterpEaseIn(0.0f, 1.0f, ReleaseProgress, 2.0f);
		TimeDilationEffect = FMath::Lerp(HitStopTimeScale, 1.0f, 1.0f - FreezeIntensity);
	}
	
	// FOV punch (maximum at start, decreases)
	float FOVPunchCurve = FreezeIntensity;
	FOVEffect = HitStopFOVPunch * FOVPunchCurve * EffectiveWeight;
	
	// Vignette during freeze
	VignetteEffect = SpecialVignette * FreezeIntensity * EffectiveWeight;
	
	// Chromatic aberration approximation
	if (HitStopChromaticAberration > 0.0f && FreezeIntensity > 0.5f)
	{
		float ChromaIntensity = (FreezeIntensity - 0.5f) * 2.0f * HitStopChromaticAberration;
		ColorTintEffect = FLinearColor(
			1.0f + ChromaIntensity,
			1.0f,
			1.0f - ChromaIntensity * 0.5f,
			1.0f
		);
	}
	else
	{
		ColorTintEffect = FLinearColor::White;
	}
	
	// Release shake
	if (bShakeOnRelease && NormalizedTime > 0.7f)
	{
		float ShakeProgress = (NormalizedTime - 0.7f) / 0.3f;
		float ShakeDecay = 1.0f - ShakeProgress;
		float ShakeAmount = ReleaseShakeIntensity * ShakeDecay * EffectiveWeight;
		
		RotationEffect.Pitch = FMath::PerlinNoise1D(TotalActiveTime * 50.0f) * ShakeAmount;
		RotationEffect.Yaw = FMath::PerlinNoise1D(TotalActiveTime * 50.0f + 100.0f) * ShakeAmount * 0.5f;
		RotationEffect.Roll = 0.0f;
	}
	else
	{
		RotationEffect = FRotator::ZeroRotator;
	}
}


//========================================
// X03: Death Camera Implementation
//========================================

/**
 * Constructor - Configure death cam parameters
 * 
 * Death cam provides dramatic death sequence:
 * - Slow orbit around death location
 * - Gradual pull-back and rise
 * - Strong visual effects (desaturation, vignette)
 * - Override mode (full camera control)
 */
UCameraModifier_X03_Special_DeathCam::UCameraModifier_X03_Special_DeathCam()
	: OrbitSpeed(15.0f)
	, PullBackDistance(150.0f)
	, RiseHeight(80.0f)
	, DeathDesaturation(0.6f)
	, DeathVignette(0.35f)
	, DeathTimeDilation(0.4f)
	, DeathLocation(FVector::ZeroVector)
	, CurrentOrbitAngle(0.0f)
	, StartOrbitAngle(0.0f)
	, StoredForward(FVector::ForwardVector)
{
	// Configure base special settings
	TargetTimeDilation = 0.4f;
	TimeRampIn = 0.3f;
	TimeRampOut = 0.5f;
	bApplyVisualEffects = true;
	SpecialVignette = 0.35f;
	SpecialSaturation = 0.4f;  // Heavy desaturation
}

/**
 * OnTriggered - Initialize death cam state
 * 
 * Stores death location and calculates initial camera position.
 */
void UCameraModifier_X03_Special_DeathCam::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Store death location
	if (InTriggerData.SourceActor.IsValid())
	{
		DeathLocation = InTriggerData.SourceActor->GetActorLocation();
		StoredForward = InTriggerData.SourceActor->GetActorForwardVector();
	}
	else if (!InTriggerData.SourceLocation.IsNearlyZero())
	{
		DeathLocation = InTriggerData.SourceLocation;
		StoredForward = InTriggerData.SourceDirection.IsNearlyZero() ? 
			FVector::ForwardVector : InTriggerData.SourceDirection.GetSafeNormal();
	}
	else
	{
		DeathLocation = FVector::ZeroVector;
		StoredForward = FVector::ForwardVector;
	}
	
	// Calculate starting orbit angle (behind player)
	StartOrbitAngle = FMath::Atan2(StoredForward.Y, StoredForward.X) * (180.0f / PI);
	StartOrbitAngle += 180.0f;  // Start behind
	CurrentOrbitAngle = StartOrbitAngle;
	
	// Call parent
	Super::OnTriggered(InTriggerData);
	
	UE_LOG(LogTemp, Verbose, TEXT("X03 DeathCam: Triggered at (%.0f, %.0f, %.0f), StartAngle=%.1f"),
		DeathLocation.X, DeathLocation.Y, DeathLocation.Z, StartOrbitAngle);
}

/**
 * ComputeEffect - Calculate death cam effect each frame
 * 
 * Creates slow orbit with pull-back and dramatic visual effects.
 * Uses override mode for full camera control.
 */
void UCameraModifier_X03_Special_DeathCam::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Call base for time dilation
	Super::ComputeEffect(DeltaTime, Context);
	
	// Calculate normalized time
	float NormalizedTime = 0.0f;
	if (ActiveDuration > 0.0f)
	{
		NormalizedTime = FMath::Clamp(TotalActiveTime / ActiveDuration, 0.0f, 1.0f);
	}
	
	float EffectiveWeight = CurrentWeight * CurrentIntensity;
	
	// Update orbit angle
	CurrentOrbitAngle += OrbitSpeed * DeltaTime;
	
	// Calculate camera position
	float BaseDistance = 200.0f;
	float CurrentPullBack = PullBackDistance * NormalizedTime;
	float CurrentRise = RiseHeight * FMath::Sin(NormalizedTime * PI * 0.5f);  // Smooth rise
	
	float TotalDistance = BaseDistance + CurrentPullBack;
	float AngleRad = FMath::DegreesToRadians(CurrentOrbitAngle);
	
	FVector CameraOffset;
	CameraOffset.X = FMath::Cos(AngleRad) * TotalDistance;
	CameraOffset.Y = FMath::Sin(AngleRad) * TotalDistance;
	CameraOffset.Z = 80.0f + CurrentRise;  // Base height + rise
	
	FVector CameraPosition = DeathLocation + CameraOffset;
	
	// Look at death location (slightly above for better framing)
	FVector LookAtPoint = DeathLocation + FVector(0.0f, 0.0f, 50.0f);
	FVector LookDirection = (LookAtPoint - CameraPosition).GetSafeNormal();
	FRotator CameraRotation = LookDirection.Rotation();
	
	// Set override transform
	OverrideTransform.SetLocation(CameraPosition);
	OverrideTransform.SetRotation(CameraRotation.Quaternion());
	bIsOverriding = true;
	
	// Visual effects - gradually increase
	float VisualProgress = FMath::InterpEaseOut(0.0f, 1.0f, NormalizedTime, 2.0f);
	
	// Vignette
	VignetteEffect = DeathVignette * VisualProgress * EffectiveWeight;
	
	// Desaturation (heavy)
	SaturationEffect = FMath::Lerp(1.0f, 1.0f - DeathDesaturation, VisualProgress * EffectiveWeight);
	
	// Slight color grade (cooler, darker)
	float ColorProgress = VisualProgress * EffectiveWeight;
	ColorTintEffect = FLinearColor(
		1.0f - ColorProgress * 0.1f,   // Slightly less red
		1.0f - ColorProgress * 0.05f,  // Slightly less green
		1.0f,                          // Normal blue
		1.0f
	);
	
	// FOV - slightly wider for epic feel
	FOVEffect = 5.0f * VisualProgress * EffectiveWeight;
}
