// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleBase.h"
#include "CameraModule_Offset.generated.h"

/**
 * Offset Module Declarations (O01-O05)
 * 
 * This file contains the 5 Offset modules for the camera system.
 * Offset modules are responsible for calculating camera position offsets (FVector),
 * which are applied relative to the camera's target/focus point.
 * 
 * These modules execute in Stage 3 (Module Compute) of the camera pipeline,
 * and their outputs are combined in Stage 5 (Blend & Solve).
 * 
 * Output Field: OffsetOutput (FVector type, in centimeters)
 * 
 * Offset Types:
 * - Socket Offset: Applied to the camera boom socket location
 * - Shoulder Offset: Horizontal offset for over-the-shoulder view
 * - Vertical Offset: Height adjustments (crouch, mount, etc.)
 * 
 * Common Use Cases in Souls-like Games:
 * - Over-the-shoulder combat view
 * - Centered exploration view
 * - Crouching through low passages
 * - Mounted combat on horseback
 * 
 * Blend Policies for Offset:
 * - Override: Replace with this value (base offsets)
 * - Additive: Add to current offset (adjustments)
 * - Blend: Weighted average (smooth transitions)
 * 
 * Coordinate System (Local to Camera):
 * - X: Forward (towards character)
 * - Y: Right (positive = right shoulder)
 * - Z: Up (positive = higher camera)
 * 
 * Module List:
 * - O01: SocketOffset Base - Foundation socket offset from config
 * - O02: Shoulder Offset - Over-the-shoulder horizontal offset
 * - O03: Shoulder Switch - Dynamic shoulder side switching
 * - O04: Crouch Offset - Vertical offset for crouching
 * - O05: Mount Offset - Offset adjustment for mounted characters
 */


//========================================
// O01: SocketOffset Base
//========================================

/**
 * UCameraModule_O01_SocketOffset_Base
 * 
 * O01: SocketOffset Base - Provides base socket offset from state config
 * 
 * This is the foundation Offset module. It reads the configured base
 * socket offset from StateConfig and outputs it directly. Other Offset
 * modules typically modify this base value.
 * 
 * Always active as the starting point for offset calculations.
 * Uses Override policy with priority 100 (base level).
 * 
 * Socket Offset is applied to the camera boom's socket location,
 * affecting the final camera position relative to the character.
 * 
 * Typical base socket offsets:
 * - Centered: (0, 0, 0)
 * - Slight right: (0, 30, 0)
 * - Over shoulder: (0, 50, 20)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_O01_SocketOffset_Base : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_O01_SocketOffset_Base; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Offset; }
	virtual FString GetModuleDescription() const override { return TEXT("Base socket offset from state configuration"); }

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
// O02: Shoulder Offset
//========================================

/**
 * UCameraModule_O02_Shoulder_Offset
 * 
 * O02: Shoulder Offset - Over-the-shoulder camera offset
 * 
 * Provides horizontal offset for third-person shooter style view.
 * Positions the camera slightly to the side of the character,
 * allowing better visibility of aiming direction.
 * 
 * In Souls-like games, this creates the classic "over-shoulder"
 * view during combat, especially when locked onto enemies.
 * 
 * Uses Override policy - sets the shoulder offset directly.
 * Priority 105 (after base socket offset).
 * 
 * Example offsets:
 * - No shoulder: Y = 0cm
 * - Light shoulder: Y = 30cm
 * - Full shoulder: Y = 50-70cm
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_O02_Shoulder_Offset : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_O02_Shoulder_Offset; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Offset; }
	virtual FString GetModuleDescription() const override { return TEXT("Over-the-shoulder horizontal offset"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Applied after base socket offset */
	virtual int32 GetDefaultPriority() const override { return 105; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when shoulder offset is configured */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
};


//========================================
// O03: Shoulder Switch
//========================================

/**
 * UCameraModule_O03_Shoulder_Switch
 * 
 * O03: Shoulder Switch - Dynamic shoulder side switching
 * 
 * Allows switching the camera between left and right shoulder views.
 * Can be triggered manually (player input) or automatically
 * (based on target position or cover direction).
 * 
 * Uses Override policy - completely controls shoulder offset.
 * Priority 110 (higher than base shoulder offset).
 * 
 * Features:
 * - Manual switch via SwitchShoulder()
 * - Direct set via SetShoulderSide()
 * - Smooth blend between sides
 * - Auto-switch based on target (optional)
 * 
 * Example behavior:
 * - Right shoulder: Y = +50cm
 * - Left shoulder: Y = -50cm
 * - Transition: Smooth blend over ~0.3 seconds
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_O03_Shoulder_Switch : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes shoulder state */
	UCameraModule_O03_Shoulder_Switch();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_O03_Shoulder_Switch; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Offset; }
	virtual FString GetModuleDescription() const override { return TEXT("Dynamic shoulder side switching"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher priority to override shoulder offset */
	virtual int32 GetDefaultPriority() const override { return 110; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when shoulder switching is enabled */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

	//========================================
	// Shoulder Control Functions
	//========================================

	/**
	 * Toggle shoulder side (left <-> right)
	 * Call this when player presses shoulder switch button
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Offset")
	void SwitchShoulder();

	/**
	 * Set shoulder side directly
	 * @param bRightShoulder True for right shoulder, false for left
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Offset")
	void SetShoulderSide(bool bRightShoulder);

	/**
	 * Get current shoulder side
	 * @return True if camera is on right shoulder
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Offset")
	bool IsRightShoulder() const { return bCurrentRightShoulder; }

protected:
	/** Current shoulder side (true = right, false = left) */
	UPROPERTY()
	bool bCurrentRightShoulder;

	/** Target shoulder side for smooth blending */
	UPROPERTY()
	bool bTargetRightShoulder;

	/** Current blend value (-1.0 = full left, +1.0 = full right) */
	UPROPERTY()
	float CurrentShoulderBlend;
};


//========================================
// O04: Crouch Offset
//========================================

/**
 * UCameraModule_O04_Crouch_Offset
 * 
 * O04: Crouch Offset - Vertical offset when crouching
 * 
 * Lowers the camera to match the crouched character height.
 * Provides smooth transition when entering/exiting crouch.
 * 
 * Uses Additive policy - adds offset to current value.
 * Priority 108 (blends with other offsets).
 * 
 * In Souls-like games, crouching is less common but still
 * important for stealth sections or traversing low passages.
 * 
 * Example offsets:
 * - Standing: Z = 0cm (no offset)
 * - Crouching: Z = -40cm to -60cm (lower camera)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_O04_Crouch_Offset : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes crouch blend state */
	UCameraModule_O04_Crouch_Offset();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_O04_Crouch_Offset; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Offset; }
	virtual FString GetModuleDescription() const override { return TEXT("Vertical offset adjustment for crouching"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Additive priority for crouch */
	virtual int32 GetDefaultPriority() const override { return 108; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when character is crouching */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Current crouch blend value (0 = standing, 1 = fully crouched) */
	UPROPERTY()
	float CurrentCrouchBlend;
};


//========================================
// O05: Mount Offset
//========================================

/**
 * UCameraModule_O05_Mount_Offset
 * 
 * O05: Mount Offset - Offset adjustment for mounted characters
 * 
 * Raises and adjusts the camera when the character is mounted
 * (horseback, vehicle, etc.). Accounts for the increased height
 * and potentially different camera requirements.
 * 
 * Uses Additive policy - adds offset to current value.
 * Priority 112 (higher than crouch, affects mount state).
 * 
 * In Souls-like games (e.g., Elden Ring), mounted combat requires
 * different camera positioning for the taller viewpoint.
 * 
 * Example offsets:
 * - Dismounted: (0, 0, 0) no offset
 * - Mounted: (0, 0, 80) raise camera
 * - Mounted combat: (20, 30, 100) forward + right + up
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraModule_O05_Mount_Offset : public UCameraModuleBase
{
	GENERATED_BODY()

public:
	/** Constructor - initializes mount blend state */
	UCameraModule_O05_Mount_Offset();

	//========================================
	// Module Identity
	//========================================

	virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_O05_Mount_Offset; }
	virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Offset; }
	virtual FString GetModuleDescription() const override { return TEXT("Offset adjustment for mounted characters"); }

	//========================================
	// Priority & Blending
	//========================================

	/** Higher priority for mount state */
	virtual int32 GetDefaultPriority() const override { return 112; }
	virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Additive; }

	//========================================
	// Activation & Computation
	//========================================

	/** Activates when character is mounted */
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;

protected:
	/** Current mount blend value (0 = dismounted, 1 = fully mounted) */
	UPROPERTY()
	float CurrentMountBlend;
};
