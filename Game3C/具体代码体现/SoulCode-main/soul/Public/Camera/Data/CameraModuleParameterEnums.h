// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModuleParameterEnums.generated.h"

/**
 * Camera Parameter Category Enumeration
 */
UENUM(BlueprintType)
enum class ECameraParamCategory : uint8
{
	Position    UMETA(DisplayName = "Position"),
	Rotation    UMETA(DisplayName = "Rotation"),
	Distance    UMETA(DisplayName = "Distance"),
	FOV         UMETA(DisplayName = "FOV"),
	Offset      UMETA(DisplayName = "Offset"),
	Constraint  UMETA(DisplayName = "Constraint")
};

/**
 * Camera Parameter Data Type Enumeration
 */
UENUM(BlueprintType)
enum class ECameraParamDataType : uint8
{
	Float   UMETA(DisplayName = "Float"),
	Vector  UMETA(DisplayName = "Vector"),
	Name    UMETA(DisplayName = "Name")
};
