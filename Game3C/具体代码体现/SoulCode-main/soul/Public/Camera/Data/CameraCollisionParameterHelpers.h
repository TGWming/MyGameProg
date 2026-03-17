// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraCollisionParameterData.h"
#include "CameraCollisionParameterHelpers.generated.h"

/**
 * Camera Collision Parameter Helper Function Library
 * Provides blueprint-callable static helper functions for parsing and processing FCameraCollisionParameterRow data
 */
UCLASS()
class SOUL_API UCameraCollisionParameterHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==================== Type Check Functions ====================
	
	/** Check if DataType is float */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool IsFloatType(const FCameraCollisionParameterRow& Row);

	/** Check if DataType is int */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool IsIntType(const FCameraCollisionParameterRow& Row);

	/** Check if DataType is bool */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool IsBoolType(const FCameraCollisionParameterRow& Row);

	/** Check if DataType is TArray<float> */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool IsArrayType(const FCameraCollisionParameterRow& Row);

	/** Check if DataType is ECollisionChannel */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool IsEnumType(const FCameraCollisionParameterRow& Row);

	// ==================== Value Parsing Functions ====================
	
	/** Parse DefaultValue as float */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static float GetDefaultValueAsFloat(const FCameraCollisionParameterRow& Row);

	/** Parse DefaultValue as int32 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static int32 GetDefaultValueAsInt(const FCameraCollisionParameterRow& Row);

	/** Parse DefaultValue as bool ("true"/"false") */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool GetDefaultValueAsBool(const FCameraCollisionParameterRow& Row);

	/** Parse DefaultValue as float array, handles "[x,y,z,...]" format */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static TArray<float> GetDefaultValueAsFloatArray(const FCameraCollisionParameterRow& Row);

	/** Parse DefaultValue as ECollisionChannel enum */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static TEnumAsByte<ECollisionChannel> GetDefaultValueAsCollisionChannel(const FCameraCollisionParameterRow& Row);

	// ==================== Limit Check Functions ====================
	
	/** Check if MinValue is not "-" */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool HasMinLimit(const FCameraCollisionParameterRow& Row);

	/** Check if MaxValue is not "-" */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static bool HasMaxLimit(const FCameraCollisionParameterRow& Row);

	/** Get minimum value (returns TNumericLimits<float>::Lowest() when no limit) */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static float GetMinValueAsFloat(const FCameraCollisionParameterRow& Row);

	/** Get maximum value (returns TNumericLimits<float>::Max() when no limit) */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static float GetMaxValueAsFloat(const FCameraCollisionParameterRow& Row);

	// ==================== Other Helper Functions ====================
	
	/** Parse RelatedStrategies as array */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionParameter")
	static TArray<FString> GetRelatedStrategiesArray(const FCameraCollisionParameterRow& Row);
};
