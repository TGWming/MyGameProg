// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/Data/CameraCompletedParameterData.h"
#include "Camera/Data/CameraCompletedParameterEnums.h"
#include "CameraCompletedParameterHelpers.generated.h"

/**
 * Camera Completed Parameter Helper Function Library
 * Provides parameter parsing, type checking, group checking and other utility functions
 */
UCLASS()
class SOUL_API UCameraCompletedParameterHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========== Value Parsing Functions ==========

	/** Parse default value as float */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static float GetDefaultValueAsFloat(const FCameraCompletedParameterRow& Row);

	/** Parse default value as integer */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static int32 GetDefaultValueAsInt(const FCameraCompletedParameterRow& Row);

	/** Parse default value as boolean */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool GetDefaultValueAsBool(const FCameraCompletedParameterRow& Row);

	/** Parse default value as name */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FName GetDefaultValueAsName(const FCameraCompletedParameterRow& Row);

	/** Parse default value as string */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FString GetDefaultValueAsString(const FCameraCompletedParameterRow& Row);

	// ========== Limit Check Functions ==========

	/** Check if has minimum value limit */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool HasMinLimit(const FCameraCompletedParameterRow& Row);

	/** Check if has maximum value limit */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool HasMaxLimit(const FCameraCompletedParameterRow& Row);

	/** Get minimum value (float) */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static float GetMinValueAsFloat(const FCameraCompletedParameterRow& Row);

	/** Get maximum value (float) */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static float GetMaxValueAsFloat(const FCameraCompletedParameterRow& Row);

	// ========== Array Parsing Functions ==========

	/** Parse RelatedTo field as array (separated by "/") */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static TArray<FString> GetRelatedToArray(const FCameraCompletedParameterRow& Row);

	// ========== Type Check Functions ==========

	/** Check if is float type */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsFloatType(const FCameraCompletedParameterRow& Row);

	/** Check if is integer type */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsIntType(const FCameraCompletedParameterRow& Row);

	/** Check if is boolean type */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsBoolType(const FCameraCompletedParameterRow& Row);

	/** Check if is name type */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsNameType(const FCameraCompletedParameterRow& Row);

	/** Check if is string type */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsStringType(const FCameraCompletedParameterRow& Row);

	/** Check if is enum type */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsEnumType(const FCameraCompletedParameterRow& Row);

	/** Check if is pointer type */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsPointerType(const FCameraCompletedParameterRow& Row);

	// ========== Group Check Functions ==========

	/** Check if is StateBase group */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsStateBaseGroup(const FCameraCompletedParameterRow& Row);

	/** Check if is Module group */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsModuleGroup(const FCameraCompletedParameterRow& Row);

	/** Check if is Modifier group */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsModifierGroup(const FCameraCompletedParameterRow& Row);

	/** Check if is Collision group */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
static bool IsCollisionGroup(const FCameraCompletedParameterRow& Row);

	/** Check if is Global group */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static bool IsGlobalGroup(const FCameraCompletedParameterRow& Row);

	// ========== Enum Conversion Functions ==========

	/** Convert string to main group enum */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static EParameterGroup StringToGroup(const FString& GroupString);

	/** Convert main group enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FString GroupToString(EParameterGroup Group);

	/** Convert string to sub group enum */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static EParameterSubGroup StringToSubGroup(const FString& SubGroupString);

	/** Convert sub group enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FString SubGroupToString(EParameterSubGroup SubGroup);

	/** Convert string to data type enum */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static EParameterDataType StringToDataType(const FString& DataTypeString);

	/** Convert data type enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FString DataTypeToString(EParameterDataType DataType);

	// ========== Display Names ==========

	/** Get main group display name */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FString GetGroupDisplayName(EParameterGroup Group);

	/** Get sub group display name */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FString GetSubGroupDisplayName(EParameterSubGroup SubGroup);

	/** Get data type display name */
	UFUNCTION(BlueprintPure, Category = "Camera|CompletedParameter")
	static FString GetDataTypeDisplayName(EParameterDataType DataType);
};
