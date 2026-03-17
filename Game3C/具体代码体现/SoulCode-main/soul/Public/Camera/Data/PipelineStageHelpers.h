// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PipelineStageEnums.h"
#include "PipelineStageHelpers.generated.h"

/**
 * Camera Pipeline Stage Helper Function Library
 * Provides string parsing, enum conversion and other utility functions
 */
UCLASS()
class SOUL_API UPipelineStageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==================== Enum Conversion Functions ====================
	
	/**
	 * Convert string to pipeline layer enumeration
	 * @param LayerString Layer string (InputLayer, DecisionLayer, ComputeLayer, SafetyLayer, OutputLayer)
	 * @return Corresponding enum value, returns None for invalid string
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline|Helpers")
	static EPipelineLayer StringToLayer(const FString& LayerString);

	/**
	 * Convert pipeline layer enumeration to string
	 * @param Layer Layer enumeration
	 * @return Corresponding string name
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline|Helpers")
	static FString LayerToString(EPipelineLayer Layer);

	/**
	 * Get pipeline layer Chinese display name
	 * @param Layer Layer enumeration
	 * @return Chinese display name
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline|Helpers")
	static FString GetLayerDisplayName(EPipelineLayer Layer);

	// ==================== Data Parsing Functions ====================
	
	/**
	 * Parse input data string to array
	 * Split string separated by / into multiple elements (e.g. "Input1/Input2/Input3" -> ["Input1", "Input2", "Input3"])
	 * @param InputDataString Input data string
	 * @return Parsed string array
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline|Helpers")
	static TArray<FString> ParseInputDataArray(const FString& InputDataString);

	/**
	 * Parse dependencies string to FName array
	 * Split comma-separated dependency stages (e.g. "Stage1, Stage2" -> [Stage1, Stage2])
	 * @param DependenciesString Dependencies string
	 * @return Parsed FName array
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline|Helpers")
	static TArray<FName> ParseDependencies(const FString& DependenciesString);

	/**
	 * Check if stage belongs to specified layer
	 * @param Layer Layer to check
	 * @param TargetLayer Target layer
	 * @return Whether it matches
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline|Helpers")
	static bool IsLayerMatching(EPipelineLayer Layer, EPipelineLayer TargetLayer);
};
