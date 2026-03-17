// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/Data/CameraModifierData.h"
#include "Camera/Data/CameraModifierEnums.h"
#include "CameraModifierHelpers.generated.h"

/**
 * Camera Modifier Helper Functions
 * Camera modifier helper function library
 * Provides data parsing, category checking and conversion functions
 */
UCLASS()
class SOUL_API UCameraModifierHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================
	// Duration Parsing Functions
	// ========================================

	/** 
	 * Parse Duration field as float
	 * @return Fixed duration time, -1 means non-fixed time or need to use GetDurationRange
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static float GetDurationAsFloat(const FCameraModifierRow& Row);

	/** 
	 * Check if has fixed duration
	 * @return true means has explicit fixed duration
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool HasFixedDuration(const FCameraModifierRow& Row);

	/** 
	 * Get duration range
	 * @param OutMin Minimum time
	 * @param OutMax Maximum time
	 * @return true means successfully parsed as range format
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool GetDurationRange(const FCameraModifierRow& Row, float& OutMin, float& OutMax);

	/** 
	 * Check if is range duration
	 * @return true means Duration is range format (like "3.0-5.0")
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsRangeDuration(const FCameraModifierRow& Row);

	// ========================================
	// Category Check Functions
	// ========================================

	/** Check if is Shake type modifier */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsShakeModifier(const FCameraModifierRow& Row);

	/** Check if is Reaction type modifier */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsReactionModifier(const FCameraModifierRow& Row);

	/** Check if is Cinematic type modifier */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsCinematicModifier(const FCameraModifierRow& Row);

	/** Check if is Zoom type modifier */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsZoomModifier(const FCameraModifierRow& Row);

	/** Check if is Effect type modifier */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsEffectModifier(const FCameraModifierRow& Row);

	/** Check if is Special type modifier */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsSpecialModifier(const FCameraModifierRow& Row);

	// ========================================
	// Sub Category Check Functions
	// ========================================

	/** Check if is Combat sub category */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsCombatSubCategory(const FCameraModifierRow& Row);

	/** Check if is Boss sub category */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsBossSubCategory(const FCameraModifierRow& Row);

	/** Check if is Environment sub category */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool IsEnvironmentSubCategory(const FCameraModifierRow& Row);

	// ========================================
	// Affects Check Functions
	// ========================================

	/** Check if affects Transform (position or rotation) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool AffectsTransform(const FCameraModifierRow& Row);

	/** Check if affects visual effects (FOV or post process) */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static bool AffectsVisuals(const FCameraModifierRow& Row);

	// ========================================
	// Enum Conversion Functions
	// ========================================

	/** Convert string to Category enum */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static ECameraModifierCategory StringToCategory(const FString& CategoryString);

	/** Convert Category enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static FString CategoryToString(ECameraModifierCategory Category);

	/** Convert string to SubCategory enum */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static ECameraModifierSubCategory StringToSubCategory(const FString& SubCategoryString);

	/** Convert SubCategory enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static FString SubCategoryToString(ECameraModifierSubCategory SubCategory);

	// ========================================
	// Display Names
	// ========================================

	/** Get Category display name */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static FString GetCategoryDisplayName(ECameraModifierCategory Category);

	/** Get SubCategory display name */
	UFUNCTION(BlueprintPure, Category = "Camera|Modifier")
	static FString GetSubCategoryDisplayName(ECameraModifierSubCategory SubCategory);
};
