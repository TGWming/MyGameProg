// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/CameraModuleParameterHelpers.h"

// ========== Type Check Functions Implementation ==========

bool UCameraModuleParameterHelpers::IsFloatType(const FCameraModuleParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("float"), ESearchCase::IgnoreCase);
}

bool UCameraModuleParameterHelpers::IsVectorType(const FCameraModuleParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("FVector"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Contains(TEXT("Vector"), ESearchCase::IgnoreCase);
}

bool UCameraModuleParameterHelpers::IsNameType(const FCameraModuleParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("FName"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Contains(TEXT("Name"), ESearchCase::IgnoreCase);
}

// ========== Value Parsing Functions Implementation ==========

float UCameraModuleParameterHelpers::GetDefaultValueAsFloat(const FCameraModuleParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return 0.0f;
	}
	return FCString::Atof(*Row.DefaultValue);
}

FVector UCameraModuleParameterHelpers::GetDefaultValueAsVector(const FCameraModuleParameterRow& Row)
{
	FVector Result = FVector::ZeroVector;

	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return Result;
	}

	// Remove parentheses and quotes
	FString CleanedValue = Row.DefaultValue;
	CleanedValue.RemoveFromStart(TEXT("\""));
	CleanedValue.RemoveFromEnd(TEXT("\""));
	CleanedValue.RemoveFromStart(TEXT("("));
	CleanedValue.RemoveFromEnd(TEXT(")"));

	// Split by comma
	TArray<FString> Components;
	CleanedValue.ParseIntoArray(Components, TEXT(","), true);

	if (Components.Num() >= 3)
	{
		Result.X = FCString::Atof(*Components[0].TrimStartAndEnd());
		Result.Y = FCString::Atof(*Components[1].TrimStartAndEnd());
		Result.Z = FCString::Atof(*Components[2].TrimStartAndEnd());
	}

	return Result;
}

FName UCameraModuleParameterHelpers::GetDefaultValueAsName(const FCameraModuleParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return NAME_None;
	}
	return FName(*Row.DefaultValue);
}

// ========== Limit Check Functions Implementation ==========

bool UCameraModuleParameterHelpers::HasMinLimit(const FCameraModuleParameterRow& Row)
{
	return !Row.MinValue.IsEmpty() && Row.MinValue != TEXT("-");
}

bool UCameraModuleParameterHelpers::HasMaxLimit(const FCameraModuleParameterRow& Row)
{
	return !Row.MaxValue.IsEmpty() && Row.MaxValue != TEXT("-");
}

float UCameraModuleParameterHelpers::GetMinValueAsFloat(const FCameraModuleParameterRow& Row)
{
	if (!HasMinLimit(Row))
	{
		return TNumericLimits<float>::Lowest();
	}
	return FCString::Atof(*Row.MinValue);
}

float UCameraModuleParameterHelpers::GetMaxValueAsFloat(const FCameraModuleParameterRow& Row)
{
	if (!HasMaxLimit(Row))
	{
		return TNumericLimits<float>::Max();
	}
	return FCString::Atof(*Row.MaxValue);
}

// ========== Array Parsing Functions Implementation ==========

TArray<FString> UCameraModuleParameterHelpers::GetRelatedModulesArray(const FCameraModuleParameterRow& Row)
{
	TArray<FString> Result;

	if (Row.RelatedModules.IsEmpty() || Row.RelatedModules == TEXT("-"))
	{
		return Result;
	}

	Row.RelatedModules.ParseIntoArray(Result, TEXT("/"), true);

	for (FString& Str : Result)
	{
		Str = Str.TrimStartAndEnd();
	}

	return Result;
}

// ========== Category Check Functions Implementation ==========

bool UCameraModuleParameterHelpers::IsPositionParam(const FCameraModuleParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Position"), ESearchCase::IgnoreCase);
}

bool UCameraModuleParameterHelpers::IsRotationParam(const FCameraModuleParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Rotation"), ESearchCase::IgnoreCase);
}

bool UCameraModuleParameterHelpers::IsDistanceParam(const FCameraModuleParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Distance"), ESearchCase::IgnoreCase);
}

bool UCameraModuleParameterHelpers::IsFOVParam(const FCameraModuleParameterRow& Row)
{
	return Row.Category.Equals(TEXT("FOV"), ESearchCase::IgnoreCase);
}

bool UCameraModuleParameterHelpers::IsOffsetParam(const FCameraModuleParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Offset"), ESearchCase::IgnoreCase);
}

bool UCameraModuleParameterHelpers::IsConstraintParam(const FCameraModuleParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Constraint"), ESearchCase::IgnoreCase);
}

// ========== Enum Conversion Functions Implementation ==========

ECameraParamCategory UCameraModuleParameterHelpers::StringToCategory(const FString& CategoryString)
{
	FString TrimmedString = CategoryString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("Position"), ESearchCase::IgnoreCase))
		return ECameraParamCategory::Position;
	else if (TrimmedString.Equals(TEXT("Rotation"), ESearchCase::IgnoreCase))
		return ECameraParamCategory::Rotation;
	else if (TrimmedString.Equals(TEXT("Distance"), ESearchCase::IgnoreCase))
		return ECameraParamCategory::Distance;
	else if (TrimmedString.Equals(TEXT("FOV"), ESearchCase::IgnoreCase))
		return ECameraParamCategory::FOV;
	else if (TrimmedString.Equals(TEXT("Offset"), ESearchCase::IgnoreCase))
		return ECameraParamCategory::Offset;
	else if (TrimmedString.Equals(TEXT("Constraint"), ESearchCase::IgnoreCase))
		return ECameraParamCategory::Constraint;
	return ECameraParamCategory::Position;
}

FString UCameraModuleParameterHelpers::CategoryToString(ECameraParamCategory Category)
{
	switch (Category)
	{
	case ECameraParamCategory::Position:    return TEXT("Position");
	case ECameraParamCategory::Rotation:    return TEXT("Rotation");
	case ECameraParamCategory::Distance:    return TEXT("Distance");
	case ECameraParamCategory::FOV:         return TEXT("FOV");
	case ECameraParamCategory::Offset:      return TEXT("Offset");
	case ECameraParamCategory::Constraint:  return TEXT("Constraint");
	default: return TEXT("Position");
	}
}

ECameraParamDataType UCameraModuleParameterHelpers::StringToDataType(const FString& DataTypeString)
{
	FString TrimmedString = DataTypeString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("float"), ESearchCase::IgnoreCase))
		return ECameraParamDataType::Float;
	else if (TrimmedString.Equals(TEXT("FVector"), ESearchCase::IgnoreCase) ||
	         TrimmedString.Contains(TEXT("Vector"), ESearchCase::IgnoreCase))
		return ECameraParamDataType::Vector;
	else if (TrimmedString.Equals(TEXT("FName"), ESearchCase::IgnoreCase) ||
	         TrimmedString.Contains(TEXT("Name"), ESearchCase::IgnoreCase))
		return ECameraParamDataType::Name;
	return ECameraParamDataType::Float;
}

FString UCameraModuleParameterHelpers::DataTypeToString(ECameraParamDataType DataType)
{
	switch (DataType)
	{
	case ECameraParamDataType::Float:   return TEXT("float");
	case ECameraParamDataType::Vector:  return TEXT("FVector");
	case ECameraParamDataType::Name:    return TEXT("FName");
	default: return TEXT("float");
	}
}

// ========== Display Name Functions Implementation ==========

FString UCameraModuleParameterHelpers::GetCategoryDisplayName(ECameraParamCategory Category)
{
	switch (Category)
	{
	case ECameraParamCategory::Position:    return TEXT("Position");
	case ECameraParamCategory::Rotation:    return TEXT("Rotation");
	case ECameraParamCategory::Distance:    return TEXT("Distance");
	case ECameraParamCategory::FOV:         return TEXT("FOV");
	case ECameraParamCategory::Offset:      return TEXT("Offset");
	case ECameraParamCategory::Constraint:  return TEXT("Constraint");
	default: return TEXT("Unknown");
	}
}

FString UCameraModuleParameterHelpers::GetDataTypeDisplayName(ECameraParamDataType DataType)
{
	switch (DataType)
	{
	case ECameraParamDataType::Float:   return TEXT("Float");
	case ECameraParamDataType::Vector:  return TEXT("Vector");
	case ECameraParamDataType::Name:    return TEXT("Name");
	default: return TEXT("Unknown");
	}
}
