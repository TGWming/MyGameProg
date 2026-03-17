// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/Data/CameraModuleData.h"
#include "Camera/Data/CameraModuleEnums.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"  // For ECameraModuleCategory
#include "CameraModuleHelpers.generated.h"

/**
 * Camera Module Helper Function Library
 * Provides blueprint-callable static helper functions
 */
UCLASS()
class SOUL_API UCameraModuleHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================
	// Array Parsing Functions
	// ========================================

	/** Get active states array */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static TArray<FString> GetActiveStatesArray(const FCameraModuleRow& Row);

	/** Get input parameters array */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static TArray<FString> GetInputParamsArray(const FCameraModuleRow& Row);

	// ========================================
	// Category Check Functions
	// ========================================

	/** Check if is position module */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsPositionModule(const FCameraModuleRow& Row);

	/** Check if is rotation module */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsRotationModule(const FCameraModuleRow& Row);

	/** Check if is distance module */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsDistanceModule(const FCameraModuleRow& Row);

	/** Check if is FOV module */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsFOVModule(const FCameraModuleRow& Row);

	/** Check if is offset module */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsOffsetModule(const FCameraModuleRow& Row);

	/** Check if is constraint module */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsConstraintModule(const FCameraModuleRow& Row);

	// ========================================
	// Blend Policy Check Functions
	// ========================================

	/** Check if is override blend */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsOverrideBlend(const FCameraModuleRow& Row);

	/** Check if is additive blend */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsAdditiveBlend(const FCameraModuleRow& Row);

	/** Check if is multiplicative blend */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsMultiplicativeBlend(const FCameraModuleRow& Row);

	// ========================================
	// State Check Functions
	// ========================================

	/** Check if module is active in specified state */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsActiveInState(const FCameraModuleRow& Row, const FString& StateName);

	/** Check if module is active in all states */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static bool IsActiveInAllStates(const FCameraModuleRow& Row);

	// ========================================
	// Enum Conversion Functions
	// ========================================

	/** Convert string to category enum */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static ECameraModuleCategory StringToCategory(const FString& CategoryString);

	/** Convert category enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static FString CategoryToString(ECameraModuleCategory Category);

	/** Convert string to blend policy enum */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static ECameraModuleBlendPolicy StringToBlendPolicy(const FString& BlendPolicyString);

	/** Convert blend policy enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static FString BlendPolicyToString(ECameraModuleBlendPolicy BlendPolicy);

	// ========================================
	// Display Name Functions
	// ========================================

	/** Get category display name */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static FString GetCategoryDisplayName(ECameraModuleCategory Category);

	/** Get blend policy display name */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	static FString GetBlendPolicyDisplayName(ECameraModuleBlendPolicy BlendPolicy);
};
