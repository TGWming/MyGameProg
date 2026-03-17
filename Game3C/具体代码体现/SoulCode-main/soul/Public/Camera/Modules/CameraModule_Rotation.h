// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleBase.h"
#include "CameraModule_Rotation.generated.h"

/**
 * Rotation Module Declarations (R01-R09)
 * 
 * This file contains the 9 Rotation modules for the camera system.
 * Rotation modules are responsible for calculating the camera rotation (FRotator),
 * which determines the direction the camera is looking.
 * 
 * These modules execute in Stage 3 (Module Compute) of the camera pipeline,
 * and their outputs are blended together in Stage 5 (Blend & Solve).
 * 
 * Output Field: RotationOffset (FRotator type)
 * 
 * Module List:
 * - R01: Player Input Free - Free camera rotation from player input
 * - R02: Player Input Lagged - Smoothed camera rotation with lag
 * - R03: LookAt Target - Hard lock-on rotation to target
 * - R04: LookAt Target Soft - Soft lock with player input override
 * - R05: LookAt Boss - Boss-specific look-at behavior
 * - R06: AutoOrient Movement - Auto-rotate to face movement direction
 * - R07: AutoOrient Delayed - Auto-center after input timeout
 * - R08: LookAt Point - Look at specific world point
 * - R09: Spline Rotation - Rotation along spline path
 */


//========================================
// R01: Player Input Free
//========================================

/**
 * UCameraModule_R01_PlayerInput_Free
 * 
 * R01: Player Input Free - Free camera rotation from player input
 * Standard third-person camera control without any restrictions
 * Player has full control over camera yaw and pitch
 * This is the foundation rotation module for exploration and combat
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R01_PlayerInput_Free : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes rotation state */
	UCameraModule_R01_PlayerInput_Free();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R01_PlayerInput_Free; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Free camera rotation from player input"); }

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

	//========================================
	// Lifecycle
	//========================================

	virtual void OnActivate(const FStageExecutionContext& Context) override;

protected:
	/** Current accumulated rotation from player input */
	UPROPERTY()
	FRotator CurrentRotation;

	/** Base sensitivity multiplier for input (optional tuning) */
	UPROPERTY()
	float BaseSensitivity;
};


//========================================
// R02: Player Input Lagged
//========================================

/**
 * UCameraModule_R02_PlayerInput_Lagged
 * 
 * R02: Player Input Lagged - Player input with smoothing/lag
 * Provides smoother camera movement with interpolation
 * Reduces jittery input and creates more cinematic feel
 * Uses exponential interpolation for frame-rate independence
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R02_PlayerInput_Lagged : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes interpolation state */
	UCameraModule_R02_PlayerInput_Lagged();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R02_PlayerInput_Lagged; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Smoothed camera rotation with lag"); }

	//========================================
	// Priority & Blending
	//========================================

	virtual int32 GetDefaultPriority() const override { return 100; }

	//========================================
	// Activation & Computation
	//========================================

	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

	//========================================
	// Lifecycle
	//========================================

	virtual void OnActivate(const FStageExecutionContext& Context) override;

protected:
	/** Target rotation from player input */
	UPROPERTY()
	FRotator TargetRotation;

	/** Current smoothed rotation after interpolation */
	UPROPERTY()
	FRotator SmoothedRotation;

	/** Whether the module has been initialized with a valid rotation */
	UPROPERTY()
	bool bInitialized;
};


//========================================
// R03: LookAt Target
//========================================

/**
 * UCameraModule_R03_LookAt_Target
 * 
 * R03: LookAt Target - Hard lock rotation to target
 * Camera always faces the locked target directly
 * Calculates rotation from camera position to target position
 * Used for precise lock-on combat (Dark Souls style)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R03_LookAt_Target : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R03_LookAt_Target; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Hard lock-on rotation to target"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher priority than input, combat takes precedence */
	virtual int32 GetDefaultPriority() const override { return 150; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates when a target is locked on */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// R04: LookAt Target Soft
//========================================

/**
 * UCameraModule_R04_LookAt_Target_Soft
 * 
 * R04: LookAt Target Soft - Soft lock with player input override
 * Camera biases towards target but allows player adjustment
 * Provides target tracking while maintaining player control
 * Blends between lock-on direction and player input offset
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R04_LookAt_Target_Soft : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes blend state */
	UCameraModule_R04_LookAt_Target_Soft();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R04_LookAt_Target_Soft; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Soft lock-on with player input adjustment"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Slightly lower priority than hard lock-on */
	virtual int32 GetDefaultPriority() const override { return 140; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Blend; }

	//========================================
	// Activation & Computation
	//========================================

	/** Requires a target to be available */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

	//========================================
	// Lifecycle
	//========================================

	virtual void OnActivate(const FStageExecutionContext& Context) override;

protected:
	/** Player input offset from lock-on direction */
	UPROPERTY()
	FRotator InputOffset;

	/** Current blended rotation result */
	UPROPERTY()
	FRotator CurrentRotation;
};


//========================================
// R05: LookAt Boss
//========================================

/**
 * UCameraModule_R05_LookAt_Boss
 * 
 * R05: LookAt Boss - Special look-at for boss targets
 * Adjusts framing and rotation for large boss enemies
 * May include offset to focus on boss weak points or head
 * Provides cinematic boss encounter camera
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R05_LookAt_Boss : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R05_LookAt_Boss; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Boss-specific look-at behavior"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Highest priority lock-on for boss encounters */
	virtual int32 GetDefaultPriority() const override { return 160; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates when locked onto a boss target */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// R06: AutoOrient Movement
//========================================

/**
 * UCameraModule_R06_AutoOrient_Movement
 * 
 * R06: Auto Orient Movement - Auto-rotate to face movement direction
 * Camera gradually aligns behind character when moving
 * Provides natural camera behavior during exploration
 * Only activates during movement, blends with player input
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R06_AutoOrient_Movement : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R06_AutoOrient_Movement; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Auto-orient camera to movement direction"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Lower priority than input, acts as assistive feature */
	virtual int32 GetDefaultPriority() const override { return 90; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Blend; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when character is moving */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// R07: AutoOrient Delayed
//========================================

/**
 * UCameraModule_R07_AutoOrient_Delayed
 * 
 * R07: Auto Orient Delayed - Auto-center after input timeout
 * Camera returns to default position after no input for a period
 * Common in third-person games for improved navigation
 * Delay prevents interference with intentional camera angles
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R07_AutoOrient_Delayed : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes timer state */
	UCameraModule_R07_AutoOrient_Delayed();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R07_AutoOrient_Delayed; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Auto-center camera after input delay"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Lowest input priority, only acts when player is idle */
	virtual int32 GetDefaultPriority() const override { return 80; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Blend; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates after input timeout period */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Time elapsed since last camera input */
	UPROPERTY()
	float TimeSinceInput;

	/** Whether the camera is currently auto-centering */
	UPROPERTY()
	bool bIsAutoCentering;
};


//========================================
// R08: LookAt Point
//========================================

/**
 * UCameraModule_R08_LookAt_Point
 * 
 * R08: LookAt Point - Look at a specific world point
 * Used for points of interest and cinematic moments
 * Similar to R03 but targets a fixed point instead of actor
 * Configured via StateConfig for level-specific cameras
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R08_LookAt_Point : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R08_LookAt_Point; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Look at specific world point"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority for special cinematic moments */
	virtual int32 GetDefaultPriority() const override { return 180; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when StateConfig defines a look-at point */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// R09: Spline Rotation
//========================================

/**
 * UCameraModule_R09_Spline_Rotation
 * 
 * R09: Spline Rotation - Rotation along spline path
 * Used with P07 (Spline Follow) for scripted camera paths
 * Rotation can follow spline tangent or point at specific target
 * Provides cinematic camera movements along rails
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_R09_Spline_Rotation : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R09_Spline_Rotation; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
	virtual FString GetModuleDescription() const override { return TEXT("Rotation along spline path"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Highest priority for scripted/cinematic sequences */
	virtual int32 GetDefaultPriority() const override { return 200; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when a valid spline reference exists */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Current spline rotation value (optional caching) */
	UPROPERTY()
	FRotator SplineRotation;
};
