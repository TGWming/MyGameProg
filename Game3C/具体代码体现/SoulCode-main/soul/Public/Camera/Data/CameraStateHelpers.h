// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraStateEnums.h"
#include "CameraStateData.h"
#include "CameraStateHelpers.generated.h"

/**
 * Camera State Helper Function Library
 * Provides enum conversion, category checking, priority checking and other helper functions
 */
UCLASS()
class SOUL_API UCameraStateHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========== Category Enum Conversion ==========
	
	/** Convert string to Category enum */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static ECameraStateCategory StringToCategory(const FString& CategoryString);

	/** Convert Category enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static FString CategoryToString(ECameraStateCategory Category);

	// ========== SubCategory Enum Conversion ==========
	
	/** Convert string to SubCategory enum */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static ECameraStateSubCategory StringToSubCategory(const FString& SubCategoryString);

	/** Convert SubCategory enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static FString SubCategoryToString(ECameraStateSubCategory SubCategory);

	// ========== Reference Enum Conversion ==========
	
	/** Convert string to Reference enum */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static ECameraStateReference StringToReference(const FString& ReferenceString);

	/** Convert Reference enum to string */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static FString ReferenceToString(ECameraStateReference Reference);

	// ========== Category Check Functions ==========
	
	/** Check if is free exploration category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsFreeExplorationCategory(const FCameraStateRow& Row);

	/** Check if is combat category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsCombatCategory(const FCameraStateRow& Row);

	/** Check if is environment interaction category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsEnvironmentCategory(const FCameraStateRow& Row);

	/** Check if is item category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsItemCategory(const FCameraStateRow& Row);

	/** Check if is NPC interaction category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsNPCCategory(const FCameraStateRow& Row);

	/** Check if is rest point category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsRestPointCategory(const FCameraStateRow& Row);

	/** Check if is death category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsDeathCategory(const FCameraStateRow& Row);

	/** Check if is cinematic category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsCinematicCategory(const FCameraStateRow& Row);

	/** Check if is magic category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsMagicCategory(const FCameraStateRow& Row);

	/** Check if is multiplayer category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsMultiplayerCategory(const FCameraStateRow& Row);

	/** Check if is user interface category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsUICategory(const FCameraStateRow& Row);

	/** Check if is modifier category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsModifierCategory(const FCameraStateRow& Row);

	// ========== Priority Check Functions ==========
	
	/** Check if is highest priority (Priority == 1) */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsHighestPriority(const FCameraStateRow& Row);

	/** Check if is high priority (Priority == 2) */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsHighPriority(const FCameraStateRow& Row);

	/** Check if is medium priority (Priority == 3) */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsMediumPriority(const FCameraStateRow& Row);

	/** Check if is low priority (Priority == 4) */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsLowPriority(const FCameraStateRow& Row);

	// ========== Reference Check Functions ==========
	
	/** Check if is universal reference */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsUniversalReference(const FCameraStateRow& Row);

	/** Check if is from Elden Ring */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsFromEldenRing(const FCameraStateRow& Row);

	/** Check if is from Bloodborne */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsFromBloodborne(const FCameraStateRow& Row);

	/** Check if is from Dark Souls series */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsFromDarkSouls(const FCameraStateRow& Row);

	/** Check if is from Sekiro */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static bool IsFromSekiro(const FCameraStateRow& Row);

	// ========== Display Name Functions ==========
	
	/** Get Category display name */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static FString GetCategoryDisplayName(ECameraStateCategory Category);

	/** Get SubCategory display name */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static FString GetSubCategoryDisplayName(ECameraStateSubCategory SubCategory);

	/** Get Reference display name */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	static FString GetReferenceDisplayName(ECameraStateReference Reference);
};
