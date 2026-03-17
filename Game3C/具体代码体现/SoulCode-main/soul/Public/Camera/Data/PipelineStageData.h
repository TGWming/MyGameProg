// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PipelineStageEnums.h"
#include "PipelineStageData.generated.h"

/**
 * Camera Pipeline Stage Configuration Data Structure
 * Defines the configuration information for each execution stage in the camera rendering pipeline
 * Corresponds to CSV file: Pipeline_Stages_Complete.csv
 */
USTRUCT(BlueprintType)
struct SOUL_API FPipelineStageData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// ==================== Identity ====================
	
	/** Stage unique identifier ID (e.g. PipelineStageComplete_1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName StageID;

	/** Stage English name (e.g. InputContextGather) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName StageName;

	/** Stage Chinese name (e.g. Input Context Gather) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString ChineseName;

	// ==================== Classification ====================
	
	/** Layer enumeration (InputLayer, DecisionLayer, ComputeLayer, SafetyLayer, OutputLayer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	EPipelineLayer Layer;

	// ==================== Execution ====================
	
	/** Execution order (1-100), smaller number executes first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution", meta = (ClampMin = "1", ClampMax = "100"))
	int32 ExecutionOrder;

	/** Dependent prerequisite stages (separated by comma, e.g. Stage1, Stage2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	FString Dependencies;

	/** Whether this stage can be skipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	bool bCanSkip;

	/** Skip condition description (describes under what conditions it can be skipped) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	FString SkipCondition;

	// ==================== Description ====================
	
	/** Stage function detailed description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description", meta = (MultiLine = "true"))
	FString Description;

	// ==================== DataFlow ====================
	
	/** Input data description (multiple data separated by /) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataFlow")
	FString InputData;

	/** Output data description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataFlow")
	FString OutputData;

	// ==================== Performance ====================
	
	/** Estimated performance cost percentage (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "100"))
	int32 EstimatedCostPercent;

	// ==================== Constructor ====================
	
	/** Default constructor - initializes all member variables */
	FPipelineStageData()
		: StageID(NAME_None)
		, StageName(NAME_None)
		, ChineseName(TEXT(""))
		, Layer(EPipelineLayer::None)
		, ExecutionOrder(1)
		, Dependencies(TEXT(""))
		, bCanSkip(false)
		, SkipCondition(TEXT(""))
		, Description(TEXT(""))
		, InputData(TEXT(""))
		, OutputData(TEXT(""))
		, EstimatedCostPercent(0)
	{
	}
};
