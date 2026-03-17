// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/CameraCollisionParameterHelpers.h"

// ==================== Type Check Functions Implementation ====================

bool UCameraCollisionParameterHelpers::IsFloatType(const FCameraCollisionParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("float"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionParameterHelpers::IsIntType(const FCameraCollisionParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("int"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionParameterHelpers::IsBoolType(const FCameraCollisionParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("bool"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionParameterHelpers::IsArrayType(const FCameraCollisionParameterRow& Row)
{
	return Row.DataType.Contains(TEXT("TArray"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Contains(TEXT("Array"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionParameterHelpers::IsEnumType(const FCameraCollisionParameterRow& Row)
{
	return Row.DataType.Contains(TEXT("ECollisionChannel"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Contains(TEXT("CollisionChannel"), ESearchCase::IgnoreCase);
}

// ==================== Value Parsing Functions Implementation ====================

float UCameraCollisionParameterHelpers::GetDefaultValueAsFloat(const FCameraCollisionParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return 0.0f;
	}
	return FCString::Atof(*Row.DefaultValue);
}

int32 UCameraCollisionParameterHelpers::GetDefaultValueAsInt(const FCameraCollisionParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return 0;
	}
	return FCString::Atoi(*Row.DefaultValue);
}

bool UCameraCollisionParameterHelpers::GetDefaultValueAsBool(const FCameraCollisionParameterRow& Row)
{
	return Row.DefaultValue.Equals(TEXT("true"), ESearchCase::IgnoreCase);
}

TArray<float> UCameraCollisionParameterHelpers::GetDefaultValueAsFloatArray(const FCameraCollisionParameterRow& Row)
{
	TArray<float> Result;

	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return Result;
	}

	// Remove square brackets
	FString CleanedValue = Row.DefaultValue;
	CleanedValue.RemoveFromStart(TEXT("["));
	CleanedValue.RemoveFromEnd(TEXT("]"));

	// Split by comma
	TArray<FString> StringArray;
	CleanedValue.ParseIntoArray(StringArray, TEXT(","), true);

	// Convert to float
	for (const FString& Str : StringArray)
	{
		FString TrimmedStr = Str.TrimStartAndEnd();
		if (!TrimmedStr.IsEmpty())
		{
			Result.Add(FCString::Atof(*TrimmedStr));
		}
	}

	return Result;
}

TEnumAsByte<ECollisionChannel> UCameraCollisionParameterHelpers::GetDefaultValueAsCollisionChannel(const FCameraCollisionParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return ECC_Camera;
	}

	if (Row.DefaultValue.Contains(TEXT("ECC_WorldStatic")))
		return ECC_WorldStatic;
	else if (Row.DefaultValue.Contains(TEXT("ECC_WorldDynamic")))
		return ECC_WorldDynamic;
	else if (Row.DefaultValue.Contains(TEXT("ECC_Pawn")))
		return ECC_Pawn;
	else if (Row.DefaultValue.Contains(TEXT("ECC_Visibility")))
		return ECC_Visibility;
	else if (Row.DefaultValue.Contains(TEXT("ECC_Camera")))
		return ECC_Camera;
	else if (Row.DefaultValue.Contains(TEXT("ECC_PhysicsBody")))
		return ECC_PhysicsBody;
	else if (Row.DefaultValue.Contains(TEXT("ECC_Vehicle")))
		return ECC_Vehicle;
	else if (Row.DefaultValue.Contains(TEXT("ECC_Destructible")))
		return ECC_Destructible;

	return ECC_Camera;
}

// ==================== Limit Check Functions Implementation ====================

bool UCameraCollisionParameterHelpers::HasMinLimit(const FCameraCollisionParameterRow& Row)
{
	return !Row.MinValue.IsEmpty() && Row.MinValue != TEXT("-");
}

bool UCameraCollisionParameterHelpers::HasMaxLimit(const FCameraCollisionParameterRow& Row)
{
	return !Row.MaxValue.IsEmpty() && Row.MaxValue != TEXT("-");
}

float UCameraCollisionParameterHelpers::GetMinValueAsFloat(const FCameraCollisionParameterRow& Row)
{
	if (!HasMinLimit(Row))
	{
		return TNumericLimits<float>::Lowest();
	}
	return FCString::Atof(*Row.MinValue);
}

float UCameraCollisionParameterHelpers::GetMaxValueAsFloat(const FCameraCollisionParameterRow& Row)
{
	if (!HasMaxLimit(Row))
	{
		return TNumericLimits<float>::Max();
	}
	return FCString::Atof(*Row.MaxValue);
}

// ==================== Other Helper Functions Implementation ====================

TArray<FString> UCameraCollisionParameterHelpers::GetRelatedStrategiesArray(const FCameraCollisionParameterRow& Row)
{
	TArray<FString> Result;

	if (Row.RelatedStrategies.IsEmpty() || Row.RelatedStrategies == TEXT("-"))
	{
		return Result;
	}

	Row.RelatedStrategies.ParseIntoArray(Result, TEXT("/"), true);

	for (FString& Str : Result)
	{
		Str = Str.TrimStartAndEnd();
	}

	return Result;
}
