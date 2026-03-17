// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraModuleData.generated.h"

/**
 * Camera Module Data Table Row Structure
 * Used for importing camera module configuration data from CSV
 */
USTRUCT(BlueprintType)
struct FCameraModuleRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** Module unique identifier (Note: This field will be empty after CSV import as it becomes the RowName) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FString ModuleID;

	/** Module name, used as program reference identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FName ModuleName;

	/** Chinese display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FString ChineseName;

	/** Main category (Position, Rotation, Distance, FOV, Offset, Constraint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Category;

	/** Sub category (Follow, Orbit, Cinematic, Special, Input, LookAt, AutoOrient, Base, Dynamic, Constraint, Screen, World, Effect, Visibility) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString SubCategory;

	/** Module function description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FString Description;

	/** Blend policy (Override, Additive, Multiplicative, Minimum, Blend) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Settings")
	FString BlendPolicy;

	/** Priority (higher value means higher priority, range 0-300) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Settings")
	int32 Priority;

	/** Active states list, separated by "/" (e.g. "Explore/Sprint/Combat" or "All") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
	FString ActiveStates;

	/** Input parameters list, separated by "/" (e.g. "CharacterLocation/FocusOffset/SocketName") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString InputParams;

	/** Output parameter name (e.g. PositionOverride, RotationOffset, DistanceMultiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString OutputParams;

	/** Lag speed coefficient (0 means no lag, range 0-12) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float LagSpeed;

	/** Whether is core module */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	bool bIsCore;

	// Constructor
	FCameraModuleRow()
		: ModuleID(TEXT(""))
		, ModuleName(NAME_None)
		, ChineseName(TEXT(""))
		, Category(TEXT("Position"))
		, SubCategory(TEXT(""))
		, Description(TEXT(""))
		, BlendPolicy(TEXT("Override"))
		, Priority(100)
		, ActiveStates(TEXT("All"))
		, InputParams(TEXT(""))
		, OutputParams(TEXT(""))
		, LagSpeed(0.0f)
		, bIsCore(false)
	{
	}
};
