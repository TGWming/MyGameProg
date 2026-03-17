// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modifiers/CameraModifierBase.h"
#include "Camera/Core/SoulsCameraManager.h"

/**
 * CameraModifierBase.cpp
 * 
 * Implementation of the abstract base class for all camera modifiers.
 * Provides lifecycle management, blend weight calculation, and output generation.
 * 
 * Key Responsibilities:
 * - Manage modifier state transitions (Inactive -> BlendingIn -> Active -> BlendingOut -> Inactive)
 * - Calculate blend weight based on current state and timing
 * - Provide output structure with computed effect values
 * - Call virtual hooks for derived classes to implement specific effects
 * 
 * Derived classes MUST override:
 * - GetModifierID()
 * - GetModifierType()
 * - GetModifierCategory()
 * - ComputeEffect() (to calculate actual effect values)
 */


//========================================
// Constructor
//========================================

UCameraModifierBase::UCameraModifierBase()
	: CurrentState(EModifierState::Inactive)
	, PrePauseState(EModifierState::Inactive)
	, CurrentWeight(0.0f)
	, StateTime(0.0f)
	, TotalActiveTime(0.0f)
	, ActiveDuration(0.0f)
	, CurrentIntensity(1.0f)
	, PositionEffect(FVector::ZeroVector)
	, RotationEffect(FRotator::ZeroRotator)
	, DistanceEffect(0.0f)
	, FOVEffect(0.0f)
	, VignetteEffect(0.0f)
	, ColorTintEffect(FLinearColor::White)
	, SaturationEffect(1.0f)
	, TimeDilationEffect(1.0f)
	, OverrideTransform(FTransform::Identity)
	, bIsOverriding(false)
	, CameraManager(nullptr)
{
}


//========================================
// Modifier Identity
//========================================

FName UCameraModifierBase::GetModifierName() const
{
	// Default: return class name
	return GetClass()->GetFName();
}

FString UCameraModifierBase::GetModifierDescription() const
{
	return FString::Printf(TEXT("Camera Modifier: %s"), *GetModifierName().ToString());
}


//========================================
// Lifecycle Control
//========================================

/**
 * Trigger - Activate this modifier
 * 
 * This is the main entry point for activating a modifier.
 * It stores trigger data, resets effect values, and begins the blend in phase.
 * 
 * @param InTriggerData Configuration data for this activation
 */
void UCameraModifierBase::Trigger(const FModifierTriggerData& InTriggerData)
{
	// Store trigger data
	TriggerData = InTriggerData;
	CurrentIntensity = FMath::Max(0.0f, TriggerData.Intensity);
	
	// Determine duration (use override if provided, otherwise default)
	if (TriggerData.DurationOverride > 0.0f)
	{
		ActiveDuration = TriggerData.DurationOverride;
	}
	else
	{
		ActiveDuration = GetDefaultDuration();
	}
	
	// Reset effect values to defaults
	ResetEffectValues();
	
	// Reset timing
	TotalActiveTime = 0.0f;
	
	// Reset override state
	bIsOverriding = false;
	OverrideTransform = FTransform::Identity;
	
	// Start blend in phase
	TransitionToState(EModifierState::BlendingIn);
	
	// Call derived class handler
	OnTriggered(TriggerData);
}

/**
 * Stop - Deactivate this modifier
 * 
 * @param bImmediate If true, skip blend out and go directly to Inactive
 */
void UCameraModifierBase::Stop(bool bImmediate)
{
	// Already inactive, nothing to do
	if (CurrentState == EModifierState::Inactive)
	{
		return;
	}
	
	// Handle paused state - resume first then stop
	if (CurrentState == EModifierState::Paused)
	{
		// Use pre-pause state to determine behavior
		if (bImmediate)
		{
			TransitionToState(EModifierState::Inactive);
			OnDeactivated();
		}
		else
		{
			TransitionToState(EModifierState::BlendingOut);
			OnDeactivating();
		}
		return;
	}
	
	if (bImmediate)
	{
		// Immediately go to inactive
		TransitionToState(EModifierState::Inactive);
		OnDeactivated();
	}
	else
	{
		// Begin blend out (unless already blending out)
		if (CurrentState != EModifierState::BlendingOut)
		{
			TransitionToState(EModifierState::BlendingOut);
			OnDeactivating();
		}
	}
}

/**
 * Update - Update modifier state each frame
 * 
 * This method:
 * 1. Updates state timing
 * 2. Checks for state transitions
 * 3. Calculates blend weight
 * 4. Calls ComputeEffect for derived class to calculate effects
 * 
 * @param DeltaTime Time since last update
 * @param Context Current camera execution context
 */
void UCameraModifierBase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
	// Skip if inactive or paused
	if (CurrentState == EModifierState::Inactive || CurrentState == EModifierState::Paused)
	{
		return;
	}
	
	// Update timing
	StateTime += DeltaTime;
	TotalActiveTime += DeltaTime;
	
	// Check for state transitions based on timing
	switch (CurrentState)
	{
		case EModifierState::BlendingIn:
		{
			float BlendInTime = GetBlendInTime();
			if (BlendInTime <= 0.0f || StateTime >= BlendInTime)
			{
				// Blend in complete, transition to Active
				TransitionToState(EModifierState::Active);
				OnActivated();
			}
			break;
		}
		
		case EModifierState::Active:
		{
			// Check if duration has elapsed (0 = infinite)
			if (ActiveDuration > 0.0f && StateTime >= ActiveDuration)
			{
				// Duration complete, begin blend out
				TransitionToState(EModifierState::BlendingOut);
				OnDeactivating();
			}
			break;
		}
		
		case EModifierState::BlendingOut:
		{
			float BlendOutTime = GetBlendOutTime();
			if (BlendOutTime <= 0.0f || StateTime >= BlendOutTime)
			{
				// Blend out complete, go inactive
				TransitionToState(EModifierState::Inactive);
				OnDeactivated();
			}
			break;
		}
		
		default:
			break;
	}
	
	// Calculate current blend weight
	CurrentWeight = CalculateBlendWeight();
	
	// Compute effect values (derived class implementation)
	if (CurrentState != EModifierState::Inactive)
	{
		ComputeEffect(DeltaTime, Context);
	}
}


//========================================
// Pause Control
//========================================

/**
 * Pause - Freeze this modifier (maintains state but stops time progression)
 */
void UCameraModifierBase::Pause()
{
	// Can only pause if currently active (not already paused or inactive)
	if (CurrentState == EModifierState::Inactive || CurrentState == EModifierState::Paused)
	{
		return;
	}
	
	// Store current state for resume
	PrePauseState = CurrentState;
	
	// Transition to paused
	CurrentState = EModifierState::Paused;
	// Note: Don't reset StateTime - we want to preserve timing
	
	// Call derived class handler
	OnPaused();
}

/**
 * Resume - Resume this modifier from paused state
 */
void UCameraModifierBase::Resume()
{
	// Can only resume if currently paused
	if (CurrentState != EModifierState::Paused)
	{
		return;
	}
	
	// Restore previous state
	CurrentState = PrePauseState;
	// Note: StateTime is preserved from before pause
	
	// Call derived class handler
	OnResumed();
}


//========================================
// Output
//========================================

/**
 * GetOutput - Get the current modifier output
 * 
 * Fills the output structure with current effect values and state information.
 * The weight has already been calculated in Update().
 * 
 * @param Context Current camera execution context
 * @param OutOutput Output structure to fill
 * @return True if modifier has valid output (is active)
 */
bool UCameraModifierBase::GetOutput(const FStageExecutionContext& Context, FModifierOutput& OutOutput)
{
	// No output if inactive or zero weight
	if (CurrentState == EModifierState::Inactive || CurrentWeight <= KINDA_SMALL_NUMBER)
	{
		return false;
	}
	
	// Fill modifier info
	OutOutput.ModifierType = GetModifierType();
	OutOutput.State = CurrentState;
	OutOutput.CurrentWeight = CurrentWeight;
	OutOutput.TimeRemaining = GetTimeRemaining();
	
	// Fill transform effects (apply weight)
	OutOutput.PositionEffect = PositionEffect * CurrentWeight;
	OutOutput.RotationEffect = RotationEffect * CurrentWeight;
	OutOutput.DistanceEffect = DistanceEffect * CurrentWeight;
	OutOutput.FOVEffect = FOVEffect * CurrentWeight;
	
	// Fill post-process effects
	OutOutput.VignetteEffect = VignetteEffect * CurrentWeight;
	OutOutput.ColorTintEffect = FMath::Lerp(FLinearColor::White, ColorTintEffect, CurrentWeight);
	OutOutput.SaturationEffect = FMath::Lerp(1.0f, SaturationEffect, CurrentWeight);
	
	// Fill time dilation (special handling - lerp towards effect value)
	OutOutput.TimeDilationEffect = FMath::Lerp(1.0f, TimeDilationEffect, CurrentWeight);
	
	// Override camera settings
	OutOutput.bOverrideCamera = bIsOverriding && IsOverrideModifier();
	OutOutput.OverrideTransform = OverrideTransform;
	
	return true;
}


//========================================
// Virtual Callbacks (Default Implementations)
//========================================

void UCameraModifierBase::OnTriggered(const FModifierTriggerData& InTriggerData)
{
	// Default implementation - override in derived classes for custom setup
}

void UCameraModifierBase::OnActivated()
{
	// Default implementation - called when fully blended in
}

void UCameraModifierBase::OnDeactivating()
{
	// Default implementation - called when blend out begins
}

void UCameraModifierBase::OnDeactivated()
{
	// Default implementation - called when fully inactive
	// Reset effect values for next activation
	ResetEffectValues();
}

void UCameraModifierBase::OnPaused()
{
	// Default implementation - called when modifier is paused
}

void UCameraModifierBase::OnResumed()
{
	// Default implementation - called when modifier is resumed
}

void UCameraModifierBase::ComputeEffect(float DeltaTime, const FStageExecutionContext& Context)
{
	// Default implementation - MUST override in derived classes
	// Derived classes should set:
	// - PositionEffect
	// - RotationEffect
	// - DistanceEffect
	// - FOVEffect
	// - VignetteEffect
	// - ColorTintEffect
	// - SaturationEffect
	// - TimeDilationEffect
	// - OverrideTransform (for cinematic modifiers)
	// - bIsOverriding (for cinematic modifiers)
	//
	// Note: Weight is applied automatically in GetOutput()
}


//========================================
// Helper Methods
//========================================

/**
 * CalculateBlendWeight - Calculate the current blend weight based on state and timing
 * 
 * BlendingIn:  0 -> 1 over BlendInTime (using SmoothStep for smooth transition)
 * Active:      1
 * BlendingOut: 1 -> 0 over BlendOutTime (using SmoothStep for smooth transition)
 * Inactive:    0
 * Paused:      Maintains pre-pause weight
 * 
 * @return Current blend weight (0.0 to 1.0)
 */
float UCameraModifierBase::CalculateBlendWeight() const
{
	switch (CurrentState)
	{
		case EModifierState::BlendingIn:
		{
			float BlendInTime = GetBlendInTime();
			if (BlendInTime <= 0.0f)
			{
				return 1.0f;
			}
			// Calculate raw alpha and clamp
			float RawAlpha = StateTime / BlendInTime;
			float ClampedAlpha = FMath::Clamp(RawAlpha, 0.0f, 1.0f);
			// Use SmoothStep for smoother transition (slow-fast-slow curve)
			return FMath::SmoothStep(0.0f, 1.0f, ClampedAlpha);
		}
		
		case EModifierState::Active:
		{
			return 1.0f;
		}
		
		case EModifierState::BlendingOut:
		{
			float BlendOutTime = GetBlendOutTime();
			if (BlendOutTime <= 0.0f)
			{
				return 0.0f;
			}
			// Calculate raw alpha and clamp
			float RawAlpha = StateTime / BlendOutTime;
			float ClampedAlpha = FMath::Clamp(RawAlpha, 0.0f, 1.0f);
			// Use SmoothStep for smoother transition (slow-fast-slow curve)
			return FMath::SmoothStep(0.0f, 1.0f, 1.0f - ClampedAlpha);
		}
		
		case EModifierState::Paused:
		{
			// Return weight based on pre-pause state
			// (Maintains the weight from when pause was triggered)
			return CurrentWeight;
		}
		
		case EModifierState::Inactive:
		default:
		{
			return 0.0f;
		}
	}
}

/**
 * GetTimeRemaining - Get time remaining until modifier is fully inactive
 * 
 * @return Total remaining time in seconds
 */
float UCameraModifierBase::GetTimeRemaining() const
{
	switch (CurrentState)
	{
		case EModifierState::BlendingIn:
		{
			// Remaining BlendIn + Active duration + BlendOut
			float RemainingBlendIn = FMath::Max(0.0f, GetBlendInTime() - StateTime);
			return RemainingBlendIn + ActiveDuration + GetBlendOutTime();
		}
		
		case EModifierState::Active:
		{
			// Remaining Active + BlendOut
			float RemainingActive = (ActiveDuration > 0.0f) ? FMath::Max(0.0f, ActiveDuration - StateTime) : 0.0f;
			return RemainingActive + GetBlendOutTime();
		}
		
		case EModifierState::BlendingOut:
		{
			// Remaining BlendOut
			return FMath::Max(0.0f, GetBlendOutTime() - StateTime);
		}
		
		case EModifierState::Paused:
		{
			// Return time based on pre-pause state (but paused, so technically infinite)
			// Return 0 to indicate unknown/paused
			return 0.0f;
		}
		
		case EModifierState::Inactive:
		default:
		{
			return 0.0f;
		}
	}
}

/**
 * TransitionToState - Change to a new state
 * 
 * Resets StateTime and updates CurrentState.
 * 
 * @param NewState The new state to transition to
 */
void UCameraModifierBase::TransitionToState(EModifierState NewState)
{
	// Skip if already in this state
	if (CurrentState == NewState)
	{
		return;
	}
	
	// Update state
	CurrentState = NewState;
	StateTime = 0.0f;
}

/**
 * CreateBaseOutput - Create an output structure with current settings
 * 
 * @return FModifierOutput with basic info set
 */
FModifierOutput UCameraModifierBase::CreateBaseOutput() const
{
	FModifierOutput Output;
	Output.ModifierType = GetModifierType();
	Output.State = CurrentState;
	Output.CurrentWeight = CurrentWeight;
	Output.TimeRemaining = GetTimeRemaining();
	return Output;
}

/**
 * ResetEffectValues - Reset all effect values to defaults
 * 
 * Called at the start of Trigger() and end of OnDeactivated()
 */
void UCameraModifierBase::ResetEffectValues()
{
	PositionEffect = FVector::ZeroVector;
	RotationEffect = FRotator::ZeroRotator;
	DistanceEffect = 0.0f;
	FOVEffect = 0.0f;
	VignetteEffect = 0.0f;
	ColorTintEffect = FLinearColor::White;
	SaturationEffect = 1.0f;
	TimeDilationEffect = 1.0f;
	OverrideTransform = FTransform::Identity;
	bIsOverriding = false;
}
