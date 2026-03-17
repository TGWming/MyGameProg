// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PipelineStageEnums.generated.h"

/**
 * Pipeline Layer Enumeration
 * Defines the execution layers in camera rendering pipeline
 */
UENUM(BlueprintType)
enum class EPipelineLayer : uint8
{
	None = 0			UMETA(DisplayName = "None"),
	InputLayer			UMETA(DisplayName = "Input Layer"),
	DecisionLayer		UMETA(DisplayName = "Decision Layer"),
	ComputeLayer		UMETA(DisplayName = "Compute Layer"),
	SafetyLayer			UMETA(DisplayName = "Safety Layer"),
	OutputLayer			UMETA(DisplayName = "Output Layer")
};
