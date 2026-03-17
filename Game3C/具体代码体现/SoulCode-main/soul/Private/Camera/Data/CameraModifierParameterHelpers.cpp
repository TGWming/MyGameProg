// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/CameraModifierParameterHelpers.h"

// ==================== Type Check Functions Implementation ====================

bool UCameraModifierParameterHelpers::IsFloatType(const FCameraModifierParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("float"), ESearchCase::IgnoreCase);
}

bool UCameraModifierParameterHelpers::IsColorType(const FCameraModifierParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("FLinearColor"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Contains(TEXT("LinearColor"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Contains(TEXT("Color"), ESearchCase::IgnoreCase);
}

// ==================== Value Parsing Functions Implementation ====================

float UCameraModifierParameterHelpers::GetDefaultValueAsFloat(const FCameraModifierParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return 0.0f;
	}
	return FCString::Atof(*Row.DefaultValue);
}

FLinearColor UCameraModifierParameterHelpers::GetDefaultValueAsLinearColor(const FCameraModifierParameterRow& Row)
{
	FLinearColor Result = FLinearColor::White;

	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-"))
	{
		return Result;
	}

	FString CleanedValue = Row.DefaultValue;
	CleanedValue.RemoveFromStart(TEXT("\""));
	CleanedValue.RemoveFromEnd(TEXT("\""));
	CleanedValue.RemoveFromStart(TEXT("("));
	CleanedValue.RemoveFromEnd(TEXT(")"));

	TArray<FString> Components;
	CleanedValue.ParseIntoArray(Components, TEXT(","), true);

	if (Components.Num() >= 4)
	{
		Result.R = FCString::Atof(*Components[0].TrimStartAndEnd());
		Result.G = FCString::Atof(*Components[1].TrimStartAndEnd());
		Result.B = FCString::Atof(*Components[2].TrimStartAndEnd());
		Result.A = FCString::Atof(*Components[3].TrimStartAndEnd());
	}
	else if (Components.Num() >= 3)
	{
		Result.R = FCString::Atof(*Components[0].TrimStartAndEnd());
		Result.G = FCString::Atof(*Components[1].TrimStartAndEnd());
		Result.B = FCString::Atof(*Components[2].TrimStartAndEnd());
		Result.A = 1.0f;
	}

	return Result;
}

// ==================== Limit Check Functions Implementation ====================

bool UCameraModifierParameterHelpers::HasMinLimit(const FCameraModifierParameterRow& Row)
{
	return !Row.MinValue.IsEmpty() && Row.MinValue != TEXT("-");
}

bool UCameraModifierParameterHelpers::HasMaxLimit(const FCameraModifierParameterRow& Row)
{
	return !Row.MaxValue.IsEmpty() && Row.MaxValue != TEXT("-");
}

float UCameraModifierParameterHelpers::GetMinValueAsFloat(const FCameraModifierParameterRow& Row)
{
	if (!HasMinLimit(Row))
	{
		return TNumericLimits<float>::Lowest();
	}
	return FCString::Atof(*Row.MinValue);
}

float UCameraModifierParameterHelpers::GetMaxValueAsFloat(const FCameraModifierParameterRow& Row)
{
	if (!HasMaxLimit(Row))
	{
		return TNumericLimits<float>::Max();
	}
	return FCString::Atof(*Row.MaxValue);
}

// ==================== Array Parsing Functions Implementation ====================

TArray<FString> UCameraModifierParameterHelpers::GetRelatedModifiersArray(const FCameraModifierParameterRow& Row)
{
	TArray<FString> Result;

	if (Row.RelatedModifiers.IsEmpty() || Row.RelatedModifiers == TEXT("-"))
	{
		return Result;
	}

	Row.RelatedModifiers.ParseIntoArray(Result, TEXT("/"), true);

	for (FString& Str : Result)
	{
		Str = Str.TrimStartAndEnd();
	}

	return Result;
}

// ==================== Category Check Functions Implementation ====================

bool UCameraModifierParameterHelpers::IsShakeParam(const FCameraModifierParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Shake"), ESearchCase::IgnoreCase);
}

bool UCameraModifierParameterHelpers::IsReactionParam(const FCameraModifierParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Reaction"), ESearchCase::IgnoreCase);
}

bool UCameraModifierParameterHelpers::IsCinematicParam(const FCameraModifierParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Cinematic"), ESearchCase::IgnoreCase);
}

bool UCameraModifierParameterHelpers::IsZoomParam(const FCameraModifierParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Zoom"), ESearchCase::IgnoreCase);
}

bool UCameraModifierParameterHelpers::IsEffectParam(const FCameraModifierParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Effect"), ESearchCase::IgnoreCase);
}

bool UCameraModifierParameterHelpers::IsSpecialParam(const FCameraModifierParameterRow& Row)
{
	return Row.Category.Equals(TEXT("Special"), ESearchCase::IgnoreCase);
}

// ==================== Enum Conversion Functions Implementation ====================

EModifierParamCategory UCameraModifierParameterHelpers::StringToCategory(const FString& CategoryString)
{
	FString TrimmedString = CategoryString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("Shake"), ESearchCase::IgnoreCase))
		return EModifierParamCategory::Shake;
	else if (TrimmedString.Equals(TEXT("Reaction"), ESearchCase::IgnoreCase))
		return EModifierParamCategory::Reaction;
	else if (TrimmedString.Equals(TEXT("Cinematic"), ESearchCase::IgnoreCase))
		return EModifierParamCategory::Cinematic;
	else if (TrimmedString.Equals(TEXT("Zoom"), ESearchCase::IgnoreCase))
		return EModifierParamCategory::Zoom;
	else if (TrimmedString.Equals(TEXT("Effect"), ESearchCase::IgnoreCase))
		return EModifierParamCategory::Effect;
	else if (TrimmedString.Equals(TEXT("Special"), ESearchCase::IgnoreCase))
		return EModifierParamCategory::Special;
	return EModifierParamCategory::Shake;
}

FString UCameraModifierParameterHelpers::CategoryToString(EModifierParamCategory Category)
{
	switch (Category)
	{
	case EModifierParamCategory::Shake:      return TEXT("Shake");
	case EModifierParamCategory::Reaction:   return TEXT("Reaction");
	case EModifierParamCategory::Cinematic:  return TEXT("Cinematic");
	case EModifierParamCategory::Zoom:       return TEXT("Zoom");
	case EModifierParamCategory::Effect:     return TEXT("Effect");
	case EModifierParamCategory::Special:    return TEXT("Special");
	default: return TEXT("Shake");
	}
}

EModifierParamDataType UCameraModifierParameterHelpers::StringToDataType(const FString& DataTypeString)
{
	FString TrimmedString = DataTypeString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("float"), ESearchCase::IgnoreCase))
		return EModifierParamDataType::Float;
	else if (TrimmedString.Equals(TEXT("FLinearColor"), ESearchCase::IgnoreCase) ||
	         TrimmedString.Contains(TEXT("LinearColor"), ESearchCase::IgnoreCase) ||
	         TrimmedString.Contains(TEXT("Color"), ESearchCase::IgnoreCase))
		return EModifierParamDataType::LinearColor;
	return EModifierParamDataType::Float;
}

FString UCameraModifierParameterHelpers::DataTypeToString(EModifierParamDataType DataType)
{
	switch (DataType)
	{
	case EModifierParamDataType::Float:        return TEXT("float");
	case EModifierParamDataType::LinearColor:  return TEXT("FLinearColor");
	default: return TEXT("float");
	}
}

// ==================== Display Name Functions Implementation ====================

FString UCameraModifierParameterHelpers::GetCategoryDisplayName(EModifierParamCategory Category)
{
	switch (Category)
	{
	case EModifierParamCategory::Shake:      return TEXT("Shake");
	case EModifierParamCategory::Reaction:   return TEXT("Reaction");
	case EModifierParamCategory::Cinematic:  return TEXT("Cinematic");
	case EModifierParamCategory::Zoom:       return TEXT("Zoom");
	case EModifierParamCategory::Effect:     return TEXT("Effect");
	case EModifierParamCategory::Special:    return TEXT("Special");
	default: return TEXT("Unknown");
	}
}

FString UCameraModifierParameterHelpers::GetDataTypeDisplayName(EModifierParamDataType DataType)
{
	switch (DataType)
	{
	case EModifierParamDataType::Float:        return TEXT("Float");
	case EModifierParamDataType::LinearColor:  return TEXT("LinearColor");
	default: return TEXT("Unknown");
	}
}
