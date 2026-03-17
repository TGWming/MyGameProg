// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleBase.h"
#include "CameraModule_Distance.generated.h"

/**
 * Distance Module Declarations (D01-D07)
 * 
 * This file contains the 7 Distance modules for the camera system.
 * Distance modules are responsible for calculating the camera arm length (float),
 * which determines the distance between the camera and the focus point.
 * 
 * These modules execute in Stage 3 (Module Compute) of the camera pipeline,
 * and their outputs are combined in Stage 5 (Blend & Solve).
 * 
 * Output Field: DistanceOutput (float type, in centimeters)
 * 
 * Typical Distance Values:
 * - Close: 200-300cm (indoor, intimate)
 * - Standard: 350-450cm (exploration, general combat)
 * - Far: 500-700cm (boss fights, open areas)
 * - Very Far: 800+cm (cinematic, panorama)
 * 
 * Blend Policies for Distance:
 * - Override: Replace with this value
 * - Additive: Add offset to base distance
 * - Multiplicative: Multiply base distance by factor
 * - Minimum: Use the smaller of current and new value
 * - Maximum: Use the larger of current and new value
 * - Blend: Weighted average based on Weight
 * 
 * Module List:
 * - D01: Base Distance - Foundation distance from state config
 * - D02: Target Size Multiplier - Adjust based on target size
 * - D03: Speed Offset - Increase distance when moving fast
 * - D04: Combat Adjust - Combat-specific adjustments
 * - D05: Environment Limit - Limit distance in tight spaces
 * - D06: Proximity Adjust - Adjust based on target proximity
 * - D07: Boss Phase - Boss phase-specific scaling
 */


//========================================
// D01: Base Distance
//========================================

/**
 * UCameraModule_D01_BaseDistance
 * 
 * D01: Base Distance - Provides base arm length from state config
 * 
 * This is the foundation Distance module. It reads the configured base
 * distance from StateConfig and outputs it directly. Other Distance modules
 * typically modify this base value using Additive or Multiplicative policies.
 * 
 * Always active as the starting point for distance calculations.
 * Uses Override policy with priority 100 (base level).
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_D01_BaseDistance : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_D01_BaseDistance; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Distance; }
	virtual FString GetModuleDescription() const override { return TEXT("Base arm length from state configuration"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Base priority - other modules build on top of this */
	virtual int32 GetDefaultPriority() const override { return 100; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// D02: Target Size Multiplier
//========================================

/**
 * UCameraModule_D02_TargetSize_Multiplier
 * 
 * D02: Target Size Multiplier - Adjusts distance based on target size
 * 
 * When locked onto larger enemies, the camera pulls back to keep them
 * in frame. This module calculates a multiplier based on target size
 * and applies it to the base distance.
 * 
 * Uses Multiplicative policy - multiplies the base distance.
 * Only activates when a target is locked.
 * 
 * Example:
 * - Small enemy (100cm): multiplier = 1.0 (no change)
 * - Medium enemy (200cm): multiplier = 1.2 (20% farther)
 * - Large boss (500cm): multiplier = 1.5 (50% farther)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_D02_TargetSize_Multiplier : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_D02_TargetSize_Multiplier; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Distance; }
	virtual FString GetModuleDescription() const override { return TEXT("Distance multiplier based on target size"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher than base, applies after D01 */
	virtual int32 GetDefaultPriority() const override { return 110; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Multiplicative; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates when target is locked */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// D03: Speed Offset
//========================================

/**
 * UCameraModule_D03_Speed_Offset
 * 
 * D03: Speed Offset - Increases distance when moving fast
 * 
 * When the character is running or sprinting, the camera pulls back
 * to provide better forward visibility. This is common in third-person
 * games for improved navigation at high speeds.
 * 
 * Uses Additive policy - adds offset to base distance.
 * Offset is smoothed to prevent sudden distance changes.
 * 
 * Example:
 * - Walking (100cm/s): offset = 0cm
 * - Running (400cm/s): offset = +50cm
 * - Sprinting (600cm/s): offset = +100cm
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_D03_Speed_Offset : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes smooth offset state */
	UCameraModule_D03_Speed_Offset();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_D03_Speed_Offset; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Distance; }
	virtual FString GetModuleDescription() const override { return TEXT("Distance offset based on movement speed"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Applied after base distance */
	virtual int32 GetDefaultPriority() const override { return 105; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when character is moving */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Current smoothed distance offset (for smooth transitions) */
	UPROPERTY()
	float CurrentSpeedOffset;
};


//========================================
// D04: Combat Adjust
//========================================

/**
 * UCameraModule_D04_Combat_Adjust
 * 
 * D04: Combat Adjust - Distance adjustment during combat
 * 
 * Provides tighter or wider framing based on combat state.
 * Can pull camera closer for intense melee combat or push it
 * back for better tactical overview.
 * 
 * Uses Additive policy - adds/subtracts from base distance.
 * Activates when target is locked or in combat state.
 * 
 * Example:
 * - Lock-on combat: offset = -50cm (closer, more intimate)
 * - Ranged combat: offset = +100cm (farther, better overview)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_D04_Combat_Adjust : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_D04_Combat_Adjust; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Distance; }
	virtual FString GetModuleDescription() const override { return TEXT("Combat-specific distance adjustment"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher priority for combat situations */
	virtual int32 GetDefaultPriority() const override { return 115; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when in combat or target locked */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// D05: Environment Limit
//========================================

/**
 * UCameraModule_D05_Environment_Limit
 * 
 * D05: Environment Limit - Limits distance based on environment
 * 
 * In tight spaces (corridors, indoor areas), this module limits
 * the maximum camera distance to prevent camera clipping through
 * walls or awkward positioning.
 * 
 * Uses Minimum policy - takes the smaller of current and limit value.
 * This ensures the camera doesn't exceed environment constraints.
 * 
 * Note: This is a simplified version. Full collision detection
 * is handled by the Constraint modules (Stage 6).
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_D05_Environment_Limit : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_D05_Environment_Limit; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Distance; }
	virtual FString GetModuleDescription() const override { return TEXT("Environment-based distance limiting"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority - safety constraint */
	virtual int32 GetDefaultPriority() const override { return 120; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Minimum; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when environment limit is defined */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// D06: Proximity Adjust
//========================================

/**
 * UCameraModule_D06_Proximity_Adjust
 * 
 * D06: Proximity Adjust - Distance based on target proximity
 * 
 * When the player is close to a locked target, the camera distance
 * may need adjustment to maintain good framing. This module
 * adjusts distance based on the distance between player and target.
 * 
 * Uses Blend policy with dynamic weight based on proximity.
 * Only activates when target is locked.
 * 
 * Example:
 * - Target far (>800cm): no adjustment
 * - Target medium (400-800cm): slight adjustment
 * - Target close (<400cm): significant adjustment
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_D06_Proximity_Adjust : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_D06_Proximity_Adjust; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Distance; }
	virtual FString GetModuleDescription() const override { return TEXT("Distance adjustment based on target proximity"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Applied after size multiplier */
	virtual int32 GetDefaultPriority() const override { return 112; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Blend; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when target is locked */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// D07: Boss Phase
//========================================

/**
 * UCameraModule_D07_Boss_Phase
 * 
 * D07: Boss Phase - Distance scaling for boss fight phases
 * 
 * Boss fights often have multiple phases with different requirements.
 * This module applies distance scaling based on the current boss phase.
 * 
 * Uses Multiplicative policy - multiplies current distance.
 * Only activates when locked onto a boss target.
 * 
 * Example:
 * - Phase 1 (Normal): multiplier = 1.0
 * - Phase 2 (Enraged): multiplier = 1.2 (pull back)
 * - Phase 3 (Final): multiplier = 1.4 (dramatic framing)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_D07_Boss_Phase : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_D07_Boss_Phase; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Distance; }
	virtual FString GetModuleDescription() const override { return TEXT("Boss phase-specific distance scaling"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Highest priority among Distance modules */
	virtual int32 GetDefaultPriority() const override { return 125; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Multiplicative; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates for boss targets */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};
