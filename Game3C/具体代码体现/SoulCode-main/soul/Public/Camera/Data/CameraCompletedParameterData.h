// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraCompletedParameterData.generated.h"

/**
 * Camera Completed Parameter Data Table Row Structure
 * Used for importing camera system complete parameter configuration data from CSV
 */
USTRUCT(BlueprintType)
struct FCameraCompletedParameterRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 
	 * Parameter unique identifier (format: "CompleteParameter_A01")
	 * Note: This field will automatically become RowName, can be kept in struct but will be empty after import
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString ParamID;

	/** Parameter name, used as program reference identifier (e.g. StateName, BaseDistance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ParamName;

	/** Chinese display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString ChineseName;

	/** Main group (StateBase, Module, Modifier, Collision, Global) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Group;

	/** Sub group (Identity, Distance, FOV, Rotation, Offset, Lag, Transition, Collision, AutoCorrect, Flag, Hierarchy, Position, Constraint, Shake, Reaction, Cinematic, Zoom, Effect, Special, Detection, Response, Occlusion, Recovery, Limits, Smoothing, Debug, LockOn, Performance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString SubGroup;

	/** Data type (FName, FString, int32, float, bool, EBlendType, ECollisionChannel, UCurveFloat*) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	FString DataType;

	/** Default value (numeric, string, boolean, enum, pointer, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString DefaultValue;

	/** Minimum value ("-" means no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MinValue;

	/** Maximum value ("-" means no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MaxValue;

	/** Unit (e.g. "cm", "degree", "1/s", "s", "Hz", "-") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FString Unit;

	/** Parameter description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FString Description;

	/** Related to (e.g. "All", "Explore", "LockOn/Boss", "P01/P02") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FString RelatedTo;

	/** Constructor */
	FCameraCompletedParameterRow()
		: ParamID(TEXT(""))
		, ParamName(NAME_None)
		, ChineseName(TEXT(""))
		, Group(TEXT(""))
		, SubGroup(TEXT(""))
		, DataType(TEXT(""))
		, DefaultValue(TEXT(""))
		, MinValue(TEXT("-"))
		, MaxValue(TEXT("-"))
		, Unit(TEXT("-"))
		, Description(TEXT(""))
		, RelatedTo(TEXT(""))
	{
	}
};
