// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleBase.h"
#include "CameraModule_Constraint.generated.h"

/**
 * Constraint Module Declarations (C01-C04)
 * 
 * This file contains the 4 Constraint modules for the camera system.
 * Constraint modules are responsible for enforcing limits and ensuring
 * the final camera values stay within acceptable ranges.
 * 
 * These modules execute LAST in Stage 3 (Module Compute) of the camera pipeline,
 * with priority 200+ to ensure they run after all other module types.
 * 
 * Purpose of Constraint Modules:
 * - Enforce hard limits on camera values (pitch, distance, FOV)
 * - Prevent invalid or extreme camera configurations
 * - Ensure critical targets remain visible
 * - Act as the final safety net before output
 * 
 * Key Characteristics:
 * - High priority (200+) - execute after Position/Rotation/Distance/FOV/Offset
 * - Primarily use Override policy to enforce limits
 * - Read from accumulated values, output clamped/adjusted values
 * - Should be lightweight and fast
 * 
 * Constraint vs Other Modules:
 * - Other modules: "Calculate what the camera SHOULD do"
 * - Constraint modules: "Ensure camera values ARE valid"
 * 
 * Module List:
 * - C01: Pitch Clamp - Enforces pitch angle limits
 * - C02: Distance Clamp - Enforces arm length limits
 * - C03: FOV Clamp - Enforces field of view limits
 * - C04: Visibility Ensure - Ensures target remains visible
 */


//========================================
// C01: Pitch Clamp
//========================================

/**
 * UCameraModule_C01_Pitch_Clamp
 * 
 * C01: Pitch Clamp - Enforces camera pitch angle limits
 * 
 * This module clamps the camera's pitch rotation to configured
 * min/max values. Prevents the camera from looking too far up
 * or too far down.
 * 
 * Always active as a safety constraint.
 * Uses Override policy with priority 200 (constraint level).
 * 
 * Typical pitch limits:
 * - Minimum: -80 to -60 degrees (looking up)
 * - Maximum: +60 to +80 degrees (looking down)
 * - Default: -80 to +80 degrees
 * 
 * Note: In UE4, negative pitch = looking up, positive = looking down
 * 
 * Use Cases:
 * - Prevent camera from flipping over character's head
 * - Maintain comfortable viewing angles
 * - Avoid ground clipping when looking down
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_C01_Pitch_Clamp : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_C01_Pitch_Clamp; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Constraint; }
	virtual FString GetModuleDescription() const override { return TEXT("Clamps camera pitch angle to configured limits"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority - runs after all calculation modules */
	virtual int32 GetDefaultPriority() const override { return 200; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// C02: Distance Clamp
//========================================

/**
 * UCameraModule_C02_Distance_Clamp
 * 
 * C02: Distance Clamp - Enforces camera arm length limits
 * 
 * This module clamps the camera's distance (arm length) to configured
 * min/max values. Ensures the camera doesn't get too close to or
 * too far from the character.
 * 
 * Always active as a safety constraint.
 * Uses Override policy with priority 200 (constraint level).
 * 
 * Typical distance limits:
 * - Minimum: 100-200cm (prevent clipping into character)
 * - Maximum: 800-1500cm (prevent losing sight of character)
 * - Default: 100 to 1000cm
 * 
 * Use Cases:
 * - Prevent camera from entering character mesh
 * - Keep character visible at all times
 * - Maintain playable camera distance
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_C02_Distance_Clamp : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_C02_Distance_Clamp; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Constraint; }
	virtual FString GetModuleDescription() const override { return TEXT("Clamps camera distance to configured limits"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority - runs after all calculation modules */
	virtual int32 GetDefaultPriority() const override { return 200; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// C03: FOV Clamp
//========================================

/**
 * UCameraModule_C03_FOV_Clamp
 * 
 * C03: FOV Clamp - Enforces field of view limits
 * 
 * This module clamps the camera's FOV to configured min/max values.
 * Ensures the field of view stays within reasonable bounds after
 * all FOV modules have applied their adjustments.
 * 
 * Always active as a safety constraint.
 * Uses Override policy with priority 200 (constraint level).
 * 
 * Typical FOV limits:
 * - Minimum: 50-60 degrees (prevent tunnel vision)
 * - Maximum: 120-130 degrees (prevent fisheye distortion)
 * - Default: 60 to 120 degrees
 * 
 * Use Cases:
 * - Prevent extreme FOV from multiple additive modules
 * - Maintain comfortable viewing experience
 * - Avoid motion sickness from extreme FOV
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_C03_FOV_Clamp : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_C03_FOV_Clamp; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Constraint; }
	virtual FString GetModuleDescription() const override { return TEXT("Clamps camera FOV to configured limits"); }

	//========================================
	// Priority & Blending
	//========================================

	/** High priority - runs after all calculation modules */
	virtual int32 GetDefaultPriority() const override { return 200; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// C04: Visibility Ensure
//========================================

/**
 * UCameraModule_C04_Visibility_Ensure
 * 
 * C04: Visibility Ensure - Ensures locked target remains visible
 * 
 * This module checks if the locked target is within the camera's
 * view frustum and makes adjustments if necessary to keep it visible.
 * This is especially important during lock-on combat.
 * 
 * Only active when there is a locked target.
 * Uses Blend policy with priority 210 (highest - final adjustment).
 * 
 * Key Features:
 * - Detects when target is about to leave screen
 * - Calculates rotation adjustment to keep target visible
 * - Tracks time since target was last visible
 * - Stores last known visible position for recovery
 * 
 * Screen Margin:
 * - Uses configurable margin from screen edge
 * - Default: 10% margin (target within 10-90% of screen)
 * 
 * Use Cases:
 * - Lock-on combat (keep enemy visible)
 * - Boss fights (keep boss in frame)
 * - Cinematic moments (ensure important targets visible)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_C04_Visibility_Ensure : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes visibility tracking state */
	UCameraModule_C04_Visibility_Ensure();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_C04_Visibility_Ensure; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Constraint; }
	virtual FString GetModuleDescription() const override { return TEXT("Ensures locked target remains visible on screen"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Highest priority - final adjustment after all constraints */
	virtual int32 GetDefaultPriority() const override { return 210; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Blend; }

	//========================================
	// Activation & Computation
	//========================================

	/** Only activates when there is a locked target */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	//========================================
	// Visibility Helper Functions
	//========================================

	/**
	 * Check if a world point would be visible on screen
	 * 
	 * @param Context Current execution context
	 * @param WorldPoint Point to check in world space
	 * @param ScreenMargin Margin from screen edge (0.0 to 0.5, default 0.1 = 10%)
	 * @return True if point is within screen bounds (accounting for margin)
	 */
	bool IsPointVisible(const FStageExecutionContext& Context, const FVector& WorldPoint, float ScreenMargin = 0.1f) const;

	/**
	 * Calculate rotation adjustment needed to bring target into view
	 * 
	 * @param Context Current execution context
	 * @param TargetPoint Target location in world space
	 * @return Rotation offset to apply to bring target into view
	 */
	FRotator CalculateVisibilityAdjustment(const FStageExecutionContext& Context, const FVector& TargetPoint) const;

	//========================================
	// Visibility Tracking State
	//========================================

	/** Time since target was last confirmed visible (seconds) */
	UPROPERTY()
	float TimeSinceTargetVisible;

	/** Last known position where target was visible */
	UPROPERTY()
	FVector LastVisibleTargetPosition;
};
