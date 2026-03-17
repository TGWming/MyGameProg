// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraCollisionHandleEnums.generated.h"

/**
 * Collision Handle Category Enumeration
 * Detection: Detection strategies
 * Response: Response strategies
 * Occlusion: Occlusion handling strategies
 * Recovery: Recovery strategies
 * Special: Special case handling strategies
 */
UENUM(BlueprintType)
enum class ECollisionHandleCategory : uint8
{
	Detection		UMETA(DisplayName = "Detection"),
	Response		UMETA(DisplayName = "Response"),
	Occlusion		UMETA(DisplayName = "Occlusion"),
	Recovery		UMETA(DisplayName = "Recovery"),
	Special			UMETA(DisplayName = "Special")
};

/**
 * Collision Handle Type Enumeration
 * Trace: Ray trace detection
 * Sweep: Sweep detection
 * Position: Position adjustment
 * FOV: FOV adjustment
 * Material: Material handling
 * Visibility: Visibility handling
 * Timing: Timing control
 * Interpolation: Interpolation handling
 * Environment: Environment handling
 */
UENUM(BlueprintType)
enum class ECollisionHandleType : uint8
{
	Trace			UMETA(DisplayName = "Trace"),
	Sweep			UMETA(DisplayName = "Sweep"),
	Position		UMETA(DisplayName = "Position"),
	FOV				UMETA(DisplayName = "FOV"),
	Material		UMETA(DisplayName = "Material"),
	Visibility		UMETA(DisplayName = "Visibility"),
	Timing			UMETA(DisplayName = "Timing"),
	Interpolation	UMETA(DisplayName = "Interpolation"),
	Environment		UMETA(DisplayName = "Environment")
};
