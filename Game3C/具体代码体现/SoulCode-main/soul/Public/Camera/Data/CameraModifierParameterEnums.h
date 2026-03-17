// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierParameterEnums.generated.h"

/**
 * Modifier Parameter Category Enumeration
 */
UENUM(BlueprintType)
enum class EModifierParamCategory : uint8
{
	Shake       UMETA(DisplayName = "Shake"),
	Reaction    UMETA(DisplayName = "Reaction"),
	Cinematic   UMETA(DisplayName = "Cinematic"),
	Zoom        UMETA(DisplayName = "Zoom"),
	Effect      UMETA(DisplayName = "Effect"),
	Special     UMETA(DisplayName = "Special")
};

/**
 * Modifier Parameter Data Type Enumeration
 */
UENUM(BlueprintType)
enum class EModifierParamDataType : uint8
{
	Float       UMETA(DisplayName = "Float"),
	LinearColor UMETA(DisplayName = "LinearColor")
};
