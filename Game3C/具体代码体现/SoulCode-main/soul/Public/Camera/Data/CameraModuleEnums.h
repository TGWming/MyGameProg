// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleEnums.generated.h"

/**
 * NOTE: ECameraModuleCategory is now defined in SoulsCameraRuntimeTypes.h
 * with more complete values (CategoryNone, CategoryPosition, etc.)
 * 
 * This file only contains ECameraModuleBlendPolicy to avoid duplication.
 */

/**
 * Camera Module Blend Policy Enumeration
 */
UENUM(BlueprintType)
enum class ECameraModuleBlendPolicy : uint8
{
	Override        UMETA(DisplayName = "Override"),
	Additive        UMETA(DisplayName = "Additive"),
	Multiplicative  UMETA(DisplayName = "Multiplicative"),
	Minimum         UMETA(DisplayName = "Minimum"),
	Blend           UMETA(DisplayName = "Blend")
};
