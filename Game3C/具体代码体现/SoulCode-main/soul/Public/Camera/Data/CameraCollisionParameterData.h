// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "CameraCollisionParameterData.generated.h"

/**
 * Camera Collision Parameter Category Enumeration
 * Detection: Detection related parameters
 * Response: Response related parameters
 * Occlusion: Occlusion handling parameters
 * Recovery: Recovery related parameters
 * Special: Special case parameters
 */
UENUM(BlueprintType)
enum class ECollisionParamCategory : uint8
{
	Detection		UMETA(DisplayName = "Detection"),
	Response		UMETA(DisplayName = "Response"),
	Occlusion		UMETA(DisplayName = "Occlusion"),
	Recovery		UMETA(DisplayName = "Recovery"),
	Special			UMETA(DisplayName = "Special")
};

/**
 * Camera Collision Parameter Data Type Enumeration
 * Float: Floating point number
 * Int: Integer
 * Bool: Boolean value
 * CollisionChannel: Collision channel enum
 * FloatArray: Float array
 */
UENUM(BlueprintType)
enum class ECollisionParamDataType : uint8
{
	Float				UMETA(DisplayName = "Float"),
	Int					UMETA(DisplayName = "Int"),
	Bool				UMETA(DisplayName = "Bool"),
	CollisionChannel	UMETA(DisplayName = "ECollisionChannel"),
	FloatArray			UMETA(DisplayName = "TArray<float>")
};

/**
 * Camera Collision Parameter Data Table Row Structure
 * Used for importing camera collision parameter configuration data from CSV files
 */
USTRUCT(BlueprintType)
struct FCameraCollisionParameterRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Parameter unique identifier, format like "CollisionParameter_CD01", "CollisionParameter_CF01"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FString ParamID;

	// Parameter name, used as program reference identifier (e.g. ProbeRadius, OcclusionFadeSpeed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName ParamName;

	// Chinese display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FString ChineseName;

	// Parameter category (Detection, Response, Occlusion, Recovery, Special)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Category;

	// Data type (float, int, bool, ECollisionChannel, TArray<float>)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString DataType;

	// Default value (numeric like "12", bool like "true", array like "[15,30,45,60,90]", enum like "ECC_Camera")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString DefaultValue;

	// Minimum value ("-" means no limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MinValue;

	// Maximum value ("-" means no limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString MaxValue;

	// Unit (e.g. "cm", "degree", "1/s", "s", "-")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString Unit;

	// Parameter description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Documentation")
	FString Description;

	// Related strategy ID list, separated by slash (e.g. "D01/D02/D03/D04", "RS03")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Documentation")
	FString RelatedStrategies;

public:
	// Constructor
	FCameraCollisionParameterRow()
		: ParamID(TEXT(""))
		, ParamName(NAME_None)
		, ChineseName(TEXT(""))
		, Category(TEXT(""))
		, DataType(TEXT(""))
		, DefaultValue(TEXT(""))
		, MinValue(TEXT("-"))
		, MaxValue(TEXT("-"))
		, Unit(TEXT("-"))
		, Description(TEXT(""))
		, RelatedStrategies(TEXT(""))
	{
	}
};
