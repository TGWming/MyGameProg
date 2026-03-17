// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierEnums.generated.h"

/**
 * Camera Modifier Category Enum
 * Camera modifier main category
 */
UENUM(BlueprintType)
enum class ECameraModifierCategory : uint8
{
	None = 0    UMETA(DisplayName = "None"),
	Shake       UMETA(DisplayName = "Shake"),
	Reaction    UMETA(DisplayName = "Reaction"),
	Cinematic   UMETA(DisplayName = "Cinematic"),
	Zoom        UMETA(DisplayName = "Zoom"),
	Effect      UMETA(DisplayName = "Effect"),
	Special     UMETA(DisplayName = "Special")
};

/**
 * Camera Modifier Sub-Category Enum
 * Camera modifier sub category
 */
UENUM(BlueprintType)
enum class ECameraModifierSubCategory : uint8
{
	None = 0    UMETA(DisplayName = "None"),
	Combat      UMETA(DisplayName = "Combat"),
	Environment UMETA(DisplayName = "Environment"),
	Movement    UMETA(DisplayName = "Movement"),
	Special     UMETA(DisplayName = "Special"),
	Boss        UMETA(DisplayName = "Boss"),
	Status      UMETA(DisplayName = "Status"),
	Aim         UMETA(DisplayName = "Aim"),
	Time        UMETA(DisplayName = "Time"),
	Death       UMETA(DisplayName = "Death")
};
