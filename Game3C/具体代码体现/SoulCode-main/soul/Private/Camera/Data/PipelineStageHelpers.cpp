// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/PipelineStageHelpers.h"

EPipelineLayer UPipelineStageHelpers::StringToLayer(const FString& LayerString)
{
	// Remove leading and trailing spaces
	FString TrimmedString = LayerString.TrimStartAndEnd();

	if (TrimmedString.Equals(TEXT("InputLayer"), ESearchCase::IgnoreCase))
	{
		return EPipelineLayer::InputLayer;
	}
	else if (TrimmedString.Equals(TEXT("DecisionLayer"), ESearchCase::IgnoreCase))
	{
		return EPipelineLayer::DecisionLayer;
	}
	else if (TrimmedString.Equals(TEXT("ComputeLayer"), ESearchCase::IgnoreCase))
	{
		return EPipelineLayer::ComputeLayer;
	}
	else if (TrimmedString.Equals(TEXT("SafetyLayer"), ESearchCase::IgnoreCase))
	{
		return EPipelineLayer::SafetyLayer;
	}
	else if (TrimmedString.Equals(TEXT("OutputLayer"), ESearchCase::IgnoreCase))
	{
		return EPipelineLayer::OutputLayer;
	}

	return EPipelineLayer::None;
}

FString UPipelineStageHelpers::LayerToString(EPipelineLayer Layer)
{
	switch (Layer)
	{
	case EPipelineLayer::InputLayer:
		return TEXT("InputLayer");
	case EPipelineLayer::DecisionLayer:
		return TEXT("DecisionLayer");
	case EPipelineLayer::ComputeLayer:
		return TEXT("ComputeLayer");
	case EPipelineLayer::SafetyLayer:
		return TEXT("SafetyLayer");
	case EPipelineLayer::OutputLayer:
		return TEXT("OutputLayer");
	case EPipelineLayer::None:
	default:
		return TEXT("None");
	}
}

FString UPipelineStageHelpers::GetLayerDisplayName(EPipelineLayer Layer)
{
	switch (Layer)
	{
	case EPipelineLayer::InputLayer:
		return TEXT("Input Layer");
	case EPipelineLayer::DecisionLayer:
		return TEXT("Decision Layer");
	case EPipelineLayer::ComputeLayer:
		return TEXT("Compute Layer");
	case EPipelineLayer::SafetyLayer:
		return TEXT("Safety Layer");
	case EPipelineLayer::OutputLayer:
		return TEXT("Output Layer");
	case EPipelineLayer::None:
	default:
		return TEXT("None");
	}
}

TArray<FString> UPipelineStageHelpers::ParseInputDataArray(const FString& InputDataString)
{
	TArray<FString> Result;

	// If string is empty, return empty array
	if (InputDataString.IsEmpty())
	{
		return Result;
	}

	// Use / as separator to split string
	InputDataString.ParseIntoArray(Result, TEXT("/"), true);

	// Remove leading and trailing spaces from each element
	for (FString& Item : Result)
	{
		Item.TrimStartAndEndInline();
	}

	return Result;
}

TArray<FName> UPipelineStageHelpers::ParseDependencies(const FString& DependenciesString)
{
	TArray<FName> Result;

	// If string is empty, return empty array
	if (DependenciesString.IsEmpty())
	{
		return Result;
	}

	// Use comma as separator to split string
	TArray<FString> StringArray;
	DependenciesString.ParseIntoArray(StringArray, TEXT(","), true);

	// Convert string to FName and remove spaces
	for (const FString& Item : StringArray)
	{
		FString TrimmedItem = Item.TrimStartAndEnd();
		if (!TrimmedItem.IsEmpty())
		{
			Result.Add(FName(*TrimmedItem));
		}
	}

	return Result;
}

bool UPipelineStageHelpers::IsLayerMatching(EPipelineLayer Layer, EPipelineLayer TargetLayer)
{
	return Layer == TargetLayer;
}
