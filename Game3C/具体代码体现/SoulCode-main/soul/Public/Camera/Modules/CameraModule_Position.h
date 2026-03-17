// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleBase.h"
#include "CameraModule_Position.generated.h"

/**
 * Position Module Declarations (P01-P08)
 * 
 * This file contains the 8 Position modules for the camera system.
 * Position modules are responsible for calculating the camera focus point (FocusPoint),
 * which determines where the camera should be looking at or following.
 * 
 * These modules execute in Stage 3 (Module Compute) of the camera pipeline,
 * and their outputs are blended together in Stage 5 (Blend & Solve).
 * 
 * Module List:
 * - P01: Follow Target - Basic character following
 * - P02: Follow Target Lagged - Smooth interpolated following
 * - P03: Follow Target Predictive - Velocity-based anticipation
 * - P04: Orbit Lock-On - Orbit around player when locked onto target
 * - P05: Orbit Boss - Special orbit for boss encounters
 * - P06: Fixed Point - Focus on fixed world location
 * - P07: Spline Follow - Follow predefined spline path
 * - P08: MidPoint Two Target - Focus between player and target
 */


//========================================
// P01: Follow Target
//========================================

/**
 * UCameraModule_P01_FollowTarget
 * 
 * P01: Follow Target - Basic character following
 * Simply follows the character position with optional offset
 * Simplest position module, directly uses character location
 * 
 * This is the foundational position module that all other position
 * modules can build upon or override.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P01_FollowTarget : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P01_FollowTarget; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Basic character following"); }

	//========================================
	// Priority & Blending
	//========================================

	virtual int32 GetDefaultPriority() const override { return 100; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// P02: Follow Target Lagged
//========================================

/**
 * UCameraModule_P02_FollowTarget_Lagged
 * 
 * P02: Follow Target Lagged - Character following with interpolation
 * Provides smooth, delayed following for a more cinematic feel
 * Uses exponential interpolation for frame-rate independent smoothing
 * 
 * The lag speed can be configured through StateConfig to achieve
 * different levels of smoothness.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P02_FollowTarget_Lagged : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes interpolation state */
	UCameraModule_P02_FollowTarget_Lagged();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P02_FollowTarget_Lagged; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Smooth lagged character following"); }

	//========================================
	// Priority & Blending
	//========================================

	virtual int32 GetDefaultPriority() const override { return 100; }

	//========================================
	// Lifecycle & Computation
	//========================================

	virtual void OnActivate(const FStageExecutionContext& Context) override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Current interpolated position, smoothly approaches target */
	UPROPERTY()
	FVector CurrentPosition;

	/** Whether we have a valid initial position to interpolate from */
	UPROPERTY()
	bool bHasInitialPosition;
};


//========================================
// P03: Follow Target Predictive
//========================================

/**
 * UCameraModule_P03_FollowTarget_Predictive
 * 
 * P03: Follow Target Predictive - Anticipates character movement
 * Looks ahead based on velocity for smoother tracking during fast movement
 * Calculates future position using character velocity
 * 
 * Prediction amount is scaled based on movement speed, providing
 * greater anticipation during fast movement.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P03_FollowTarget_Predictive : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P03_FollowTarget_Predictive; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Predictive character following"); }

	//========================================
	// Priority & Blending
	//========================================

	virtual int32 GetDefaultPriority() const override { return 100; }

	//========================================
	// Activation & Computation
	//========================================

	/** Typically activates during high-speed movement */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// P04: Orbit Lock-On
//========================================

/**
 * UCameraModule_P04_Orbit_LockOn
 * 
 * P04: Orbit Lock-On - Orbits around player when locked onto target
 * Positions camera to keep both player and target visible
 * Creates a dynamic orbit based on player-target relationship
 * 
 * The orbit position is calculated to maintain good visibility
 * of both the player character and the locked target.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P04_Orbit_LockOn : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P04_Orbit_LockOn; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Lock-on orbit positioning"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher priority than basic following when locked on */
	virtual int32 GetDefaultPriority() const override { return 150; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates when a target is locked on */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// P05: Orbit Boss
//========================================

/**
 * UCameraModule_P05_Orbit_Boss
 * 
 * P05: Orbit Boss - Special orbit behavior for boss fights
 * Wider orbit radius to capture large boss enemies
 * Adjusted positioning for cinematic boss encounters
 * 
 * This module provides wider framing than standard lock-on
 * to accommodate large boss models and their attack patterns.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P05_Orbit_Boss : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P05_Orbit_Boss; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Boss fight orbit positioning"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher priority than standard lock-on */
	virtual int32 GetDefaultPriority() const override { return 160; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates during boss encounters */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// P06: Fixed Point
//========================================

/**
 * UCameraModule_P06_FixedPoint
 * 
 * P06: Fixed Point - Camera focuses on a fixed world location
 * Used for cinematic shots and specific gameplay moments
 * Position is defined in state config or by external system
 * 
 * The fixed point location is retrieved from the StateConfig,
 * allowing level designers to specify exact camera positions.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P06_FixedPoint : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P06_FixedPoint; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Fixed world point focus"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority for special cinematic moments */
	virtual int32 GetDefaultPriority() const override { return 200; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when StateConfig defines a fixed point */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// P07: Spline Follow
//========================================

/**
 * UCameraModule_P07_Spline_Follow
 * 
 * P07: Spline Follow - Camera follows a predefined spline path
 * Used for cinematic sequences and scripted camera movements
 * Spline reference and progress are provided by state config
 * 
 * The spline path and current progress along it are managed
 * externally, this module simply converts that to a world position.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P07_Spline_Follow : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P07_Spline_Follow; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Spline path following"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority for cinematic sequences */
	virtual int32 GetDefaultPriority() const override { return 200; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when a valid spline reference exists */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Normalized position along spline (0.0 to 1.0) */
	UPROPERTY()
	float CurrentSplinePosition;
};


//========================================
// P08: MidPoint Two Target
//========================================

/**
 * UCameraModule_P08_MidPoint_TwoTarget
 * 
 * P08: MidPoint Two Target - Focuses between player and target
 * Keeps both characters visible with configurable bias
 * Calculates weighted midpoint between two actors
 * 
 * The weight/bias can be configured to favor either the player
 * or the target, depending on the gameplay situation.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_P08_MidPoint_TwoTarget : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_P08_MidPoint_TwoTarget; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Position; }
	virtual FString GetModuleDescription() const override { return TEXT("Mid-point between player and target"); }

	//========================================
	// Priority & Blending
	//========================================

	virtual int32 GetDefaultPriority() const override { return 140; }

	//========================================
	// Activation & Computation
	//========================================

	/** Requires a valid target to activate */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};
