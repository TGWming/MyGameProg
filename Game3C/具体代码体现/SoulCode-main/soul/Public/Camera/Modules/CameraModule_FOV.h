// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleBase.h"
#include "CameraModule_FOV.generated.h"

/**
 * FOV Module Declarations (F01-F06)
 * 
 * This file contains the 6 FOV modules for the camera system.
 * FOV modules are responsible for calculating the camera field of view (float),
 * which determines the angular extent of the observable game world.
 * 
 * These modules execute in Stage 3 (Module Compute) of the camera pipeline,
 * and their outputs are combined in Stage 5 (Blend & Solve).
 * 
 * Output Field: FOVOutput (float type, in degrees)
 * 
 * Typical FOV Values:
 * - Narrow: 60-70 degrees (aiming, focused)
 * - Standard: 80-90 degrees (exploration, general gameplay)
 * - Wide: 100-110 degrees (sprinting, high speed)
 * - Ultra Wide: 120+ degrees (special effects, impact moments)
 * 
 * FOV Effects on Gameplay:
 * - Higher FOV: Wider view, more peripheral vision, sense of speed
 * - Lower FOV: Narrower view, more focus, precision aiming
 * 
 * Blend Policies for FOV:
 * - Override: Replace with this value (F01 base)
 * - Additive: Add offset to base FOV (most adjustments)
 * - Blend: Weighted average (smooth transitions)
 * 
 * Module List:
 * - F01: Base FOV - Foundation FOV from state config
 * - F02: Speed FOV - Increase FOV when moving fast
 * - F03: Aim FOV - Reduce FOV when aiming/focusing
 * - F04: Combat FOV - Combat-specific adjustments
 * - F05: Boss FOV - Boss fight FOV adjustment
 * - F06: Impact FOV - Momentary FOV pulse on impacts
 */


//========================================
// F01: Base FOV
//========================================

/**
 * UCameraModule_F01_BaseFOV
 * 
 * F01: Base FOV - Provides base field of view from state config
 * 
 * This is the foundation FOV module. It reads the configured base
 * FOV from StateConfig and outputs it directly. Other FOV modules
 * typically modify this base value using Additive policy.
 * 
 * Always active as the starting point for FOV calculations.
 * Uses Override policy with priority 100 (base level).
 * 
 * Typical base FOV values:
 * - Exploration: 90 degrees
 * - Combat: 85 degrees
 * - Cinematic: 70 degrees
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_F01_BaseFOV : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_F01_BaseFOV; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::FOV; }
	virtual FString GetModuleDescription() const override { return TEXT("Base field of view from state configuration"); }

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
// F02: Speed FOV
//========================================

/**
 * UCameraModule_F02_Speed_FOV
 * 
 * F02: Speed FOV - Increases FOV when moving fast
 * 
 * When the character is running or sprinting, FOV increases to create
 * a sense of speed and provide better peripheral vision.
 * This is a common technique in action games.
 * 
 * Uses Additive policy - adds offset to base FOV.
 * Offset is smoothed to prevent jarring FOV changes.
 * 
 * Example:
 * - Walking: offset = 0 degrees
 * - Running: offset = +5 degrees
 * - Sprinting: offset = +15 degrees
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_F02_Speed_FOV : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes smooth offset state */
	UCameraModule_F02_Speed_FOV();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_F02_Speed_FOV; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::FOV; }
	virtual FString GetModuleDescription() const override { return TEXT("FOV increase based on movement speed"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Applied after base FOV */
	virtual int32 GetDefaultPriority() const override { return 105; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when character is moving fast */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Current smoothed FOV offset (for smooth transitions) */
	UPROPERTY()
	float CurrentSpeedFOVOffset;
};


//========================================
// F03: Aim FOV
//========================================

/**
 * UCameraModule_F03_Aim_FOV
 * 
 * F03: Aim FOV - Reduces FOV when aiming/focusing
 * 
 * When the player is aiming (ranged weapons) or in focus mode,
 * FOV decreases to provide a more zoomed-in, precise view.
 * Common in shooter games and games with ranged combat.
 * 
 * Uses Additive policy with negative offset - reduces FOV.
 * Smooth transition when entering/exiting aim mode.
 * 
 * Example:
 * - Normal: offset = 0 degrees
 * - Aiming: offset = -15 to -25 degrees
 * - Scoped: offset = -40 degrees or more
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_F03_Aim_FOV : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes aim blend state */
	UCameraModule_F03_Aim_FOV();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_F03_Aim_FOV; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::FOV; }
	virtual FString GetModuleDescription() const override { return TEXT("FOV reduction for aiming/focus mode"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher priority - aiming takes precedence */
	virtual int32 GetDefaultPriority() const override { return 115; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when player is aiming */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Current aim FOV blend value (0 to 1, for smooth transitions) */
	UPROPERTY()
	float CurrentAimFOVBlend;
};


//========================================
// F04: Combat FOV
//========================================

/**
 * UCameraModule_F04_Combat_FOV
 * 
 * F04: Combat FOV - FOV adjustment during combat
 * 
 * Provides wider or tighter view based on combat state.
 * Typically slightly wider to improve situational awareness.
 * 
 * Uses Additive policy - adds/subtracts from base FOV.
 * Activates when in combat state.
 * 
 * Example:
 * - Lock-on combat: offset = +5 degrees (slightly wider)
 * - Defensive stance: offset = -5 degrees (more focused)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_F04_Combat_FOV : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_F04_Combat_FOV; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::FOV; }
	virtual FString GetModuleDescription() const override { return TEXT("Combat-specific FOV adjustment"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Combat priority */
	virtual int32 GetDefaultPriority() const override { return 110; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when in combat */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// F05: Boss FOV
//========================================

/**
 * UCameraModule_F05_Boss_FOV
 * 
 * F05: Boss FOV - FOV adjustment for boss fights
 * 
 * Typically provides wider FOV to capture large boss enemies
 * and give the player better awareness of boss attacks.
 * 
 * Uses Additive policy - adds to base FOV.
 * Only activates when fighting a boss.
 * 
 * Example:
 * - Small boss: offset = +5 degrees
 * - Large boss: offset = +10 degrees
 * - Giant boss: offset = +15 degrees
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_F05_Boss_FOV : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_F05_Boss_FOV; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::FOV; }
	virtual FString GetModuleDescription() const override { return TEXT("Boss fight FOV adjustment"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority for boss fights */
	virtual int32 GetDefaultPriority() const override { return 120; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates for boss targets */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// F06: Impact FOV
//========================================

/**
 * UCameraModule_F06_Impact_FOV
 * 
 * F06: Impact FOV - Momentary FOV pulse on impacts
 * 
 * Provides a quick FOV change (usually decrease then return) when
 * the player takes damage or lands a heavy hit. This creates
 * visceral impact feedback.
 * 
 * Uses Additive policy - adds momentary offset.
 * Triggered externally via TriggerImpact() function.
 * 
 * Behavior:
 * - On impact: FOV quickly decreases (punch-in effect)
 * - Recovery: FOV smoothly returns to normal
 * - Duration: typically 0.1-0.3 seconds (configurable via ImpactDuration)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_F06_Impact_FOV : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes impact state */
	UCameraModule_F06_Impact_FOV();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_F06_Impact_FOV; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::FOV; }
	virtual FString GetModuleDescription() const override { return TEXT("Impact/hit FOV pulse effect"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Highest priority for immediate feedback */
	virtual int32 GetDefaultPriority() const override { return 125; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when impact is triggered */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

	//========================================
	// Impact Trigger
	//========================================

	/**
	 * Trigger an impact FOV pulse
	 * Call this when player takes damage or lands a heavy hit
	 * 
	 * @param Intensity Strength of the effect (0.0 to 1.0, default 1.0)
	 * @param Duration Optional duration override. If <= 0, uses cached config value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|FOV")
	void TriggerImpact(float Intensity = 1.0f, float Duration = 0.0f);

	/**
	 * Update cached config values from StateConfig
	 * Call this when state config changes to refresh cached values
	 * 
	 * @param StateConfig The current camera state configuration
	 */
	void UpdateCachedConfig(const FCameraStateConfig& StateConfig);

protected:
	/** Current impact intensity (decays over time) */
	UPROPERTY()
	float CurrentImpactIntensity;

	/** Is impact effect currently active */
	UPROPERTY()
	bool bImpactActive;

	/** Time remaining for impact effect */
	UPROPERTY()
	float ImpactTimeRemaining;

	/** Cached impact duration from config (for TriggerImpact to use) */
	UPROPERTY()
	float CachedImpactDuration;
};
