// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraModuleParameterData.h"
#include "CameraModuleParameterEnums.h"
#include "CameraModuleParameterHelpers.generated.h"

/**
 * Camera Module Parameter Helper Function Library
 * 
 * Provides static helper functions for parsing and processing camera module parameter data
 */
UCLASS()
class SOUL_API UCameraModuleParameterHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========== Type Check Functions ==========

	/** Check if parameter is Float type */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsFloatType(const FCameraModuleParameterRow& Row);

	/** Check if parameter is Vector type */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsVectorType(const FCameraModuleParameterRow& Row);

	/** Check if parameter is Name type */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsNameType(const FCameraModuleParameterRow& Row);

	// ========== Value Parsing Functions ==========

	/** Get default value (Float type) */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static float GetDefaultValueAsFloat(const FCameraModuleParameterRow& Row);

	/** Get default value (Vector type) */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static FVector GetDefaultValueAsVector(const FCameraModuleParameterRow& Row);

	/** Get default value (Name type) */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static FName GetDefaultValueAsName(const FCameraModuleParameterRow& Row);

	// ========== Limit Check Functions ==========

	/** Check if has minimum value limit */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool HasMinLimit(const FCameraModuleParameterRow& Row);

	/** Check if has maximum value limit */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool HasMaxLimit(const FCameraModuleParameterRow& Row);

	/** Get minimum value (Float type) */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static float GetMinValueAsFloat(const FCameraModuleParameterRow& Row);

	/** Get maximum value (Float type) */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static float GetMaxValueAsFloat(const FCameraModuleParameterRow& Row);

	// ========== Array Parsing Functions ==========

	/** Get related modules array */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static TArray<FString> GetRelatedModulesArray(const FCameraModuleParameterRow& Row);

	// ========== Category Check Functions ==========

	/** Check if is position parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsPositionParam(const FCameraModuleParameterRow& Row);

	/** Check if is rotation parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsRotationParam(const FCameraModuleParameterRow& Row);

	/** Check if is distance parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsDistanceParam(const FCameraModuleParameterRow& Row);

	/** Check if is FOV parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsFOVParam(const FCameraModuleParameterRow& Row);

	/** Check if is offset parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsOffsetParam(const FCameraModuleParameterRow& Row);

	/** Check if is constraint parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static bool IsConstraintParam(const FCameraModuleParameterRow& Row);

	// ========== Enum Conversion Functions ==========

	/** Convert string to category enum */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static ECameraParamCategory StringToCategory(const FString& CategoryString);

	/** Convert category enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static FString CategoryToString(ECameraParamCategory Category);

	/** Convert string to data type enum */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static ECameraParamDataType StringToDataType(const FString& DataTypeString);

	/** Convert data type enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static FString DataTypeToString(ECameraParamDataType DataType);

	// ========== Display Name Functions ==========

	/** Get category display name */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static FString GetCategoryDisplayName(ECameraParamCategory Category);

	/** Get data type display name */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleParameter")
	static FString GetDataTypeDisplayName(ECameraParamDataType DataType);
};
