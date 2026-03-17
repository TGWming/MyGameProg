// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraCompletedParameterEnums.generated.h"

/**
 * Parameter main group enumeration
 */
UENUM(BlueprintType)
enum class EParameterGroup : uint8
{
	StateBase    UMETA(DisplayName = "State Base"),
	Module       UMETA(DisplayName = "Module"),
	Modifier     UMETA(DisplayName = "Modifier"),
	Collision    UMETA(DisplayName = "Collision"),
	Global       UMETA(DisplayName = "Global")
};

/**
 * Parameter sub-group enumeration
 */
UENUM(BlueprintType)
enum class EParameterSubGroup : uint8
{
	Identity     UMETA(DisplayName = "Identity"),
	Distance     UMETA(DisplayName = "Distance"),
	FOV          UMETA(DisplayName = "FOV"),
	Rotation     UMETA(DisplayName = "Rotation"),
	Offset       UMETA(DisplayName = "Offset"),
	Lag          UMETA(DisplayName = "Lag"),
	Transition   UMETA(DisplayName = "Transition"),
	Collision    UMETA(DisplayName = "Collision"),
	AutoCorrect  UMETA(DisplayName = "Auto Correct"),
	Flag         UMETA(DisplayName = "Flag"),
	Hierarchy    UMETA(DisplayName = "Hierarchy"),
	Position     UMETA(DisplayName = "Position"),
	Constraint   UMETA(DisplayName = "Constraint"),
	Shake        UMETA(DisplayName = "Shake"),
	Reaction     UMETA(DisplayName = "Reaction"),
	Cinematic    UMETA(DisplayName = "Cinematic"),
	Zoom         UMETA(DisplayName = "Zoom"),
	Effect       UMETA(DisplayName = "Effect"),
	Special      UMETA(DisplayName = "Special"),
	Detection    UMETA(DisplayName = "Detection"),
	Response     UMETA(DisplayName = "Response"),
	Occlusion    UMETA(DisplayName = "Occlusion"),
	Recovery     UMETA(DisplayName = "Recovery"),
	Limits       UMETA(DisplayName = "Limits"),
	Smoothing    UMETA(DisplayName = "Smoothing"),
	Debug        UMETA(DisplayName = "Debug"),
	LockOn       UMETA(DisplayName = "Lock On"),
	Performance  UMETA(DisplayName = "Performance")
};

/**
 * Data type enumeration
 */
UENUM(BlueprintType)
enum class EParameterDataType : uint8
{
	Name             UMETA(DisplayName = "Name"),
	String           UMETA(DisplayName = "String"),
	Int32            UMETA(DisplayName = "Int32"),
	Float            UMETA(DisplayName = "Float"),
	Bool             UMETA(DisplayName = "Bool"),
	BlendType        UMETA(DisplayName = "Blend Type"),
	CollisionChannel UMETA(DisplayName = "Collision Channel"),
	CurveFloat       UMETA(DisplayName = "Curve Float")
};
