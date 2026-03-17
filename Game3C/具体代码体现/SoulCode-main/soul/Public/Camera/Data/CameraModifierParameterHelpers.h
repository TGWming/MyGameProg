// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Color.h"
#include "CameraModifierParameterData.h"
#include "CameraModifierParameterEnums.h"
#include "CameraModifierParameterHelpers.generated.h"

/**
 * Camera Modifier Parameter Helper Function Library
 * 
 * Provides blueprint-callable static helper functions for processing FCameraModifierParameterRow data
 */
UCLASS()
class SOUL_API UCameraModifierParameterHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==================== Type Check Functions ====================
	
	/** Check if parameter is float type */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsFloatType(const FCameraModifierParameterRow& Row);

	/** Check if parameter is color type */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsColorType(const FCameraModifierParameterRow& Row);

	// ==================== Value Parsing Functions ====================
	
	/** Get default value as float */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static float GetDefaultValueAsFloat(const FCameraModifierParameterRow& Row);

	/** Get default value as linear color */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static FLinearColor GetDefaultValueAsLinearColor(const FCameraModifierParameterRow& Row);

	// ==================== Limit Check Functions ====================
	
	/** Check if has minimum value limit */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool HasMinLimit(const FCameraModifierParameterRow& Row);

	/** Check if has maximum value limit */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool HasMaxLimit(const FCameraModifierParameterRow& Row);

	/** Get minimum value as float */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static float GetMinValueAsFloat(const FCameraModifierParameterRow& Row);

	/** Get maximum value as float */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static float GetMaxValueAsFloat(const FCameraModifierParameterRow& Row);

	// ==================== Array Parsing Functions ====================
	
	/** Get related modifier IDs array */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static TArray<FString> GetRelatedModifiersArray(const FCameraModifierParameterRow& Row);

	// ==================== Category Check Functions ====================
	
	/** Check if is shake parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsShakeParam(const FCameraModifierParameterRow& Row);

	/** Check if is reaction parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsReactionParam(const FCameraModifierParameterRow& Row);

	/** Check if is cinematic parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsCinematicParam(const FCameraModifierParameterRow& Row);

	/** Check if is zoom parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsZoomParam(const FCameraModifierParameterRow& Row);

	/** Check if is effect parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsEffectParam(const FCameraModifierParameterRow& Row);

	/** Check if is special parameter */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static bool IsSpecialParam(const FCameraModifierParameterRow& Row);

	// ==================== Enum Conversion Functions ====================
	
	/** Convert string to category enum */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static EModifierParamCategory StringToCategory(const FString& CategoryString);

	/** Convert category enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static FString CategoryToString(EModifierParamCategory Category);

	/** Convert string to data type enum */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static EModifierParamDataType StringToDataType(const FString& DataTypeString);

	/** Convert data type enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static FString DataTypeToString(EModifierParamDataType DataType);

	// ==================== Display Name Functions ====================
	
	/** Get category display name */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static FString GetCategoryDisplayName(EModifierParamCategory Category);

	/** Get data type display name */
	UFUNCTION(BlueprintPure, Category = "Camera|ModifierParameter")
	static FString GetDataTypeDisplayName(EModifierParamDataType DataType);
};
