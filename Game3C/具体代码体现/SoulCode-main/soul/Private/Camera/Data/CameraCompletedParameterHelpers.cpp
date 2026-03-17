// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/CameraCompletedParameterHelpers.h"

// ========== Value Parsing Functions ==========

float UCameraCompletedParameterHelpers::GetDefaultValueAsFloat(const FCameraCompletedParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-") || Row.DefaultValue == TEXT("nullptr"))
	{
		return 0.0f;
	}
	return FCString::Atof(*Row.DefaultValue);
}

int32 UCameraCompletedParameterHelpers::GetDefaultValueAsInt(const FCameraCompletedParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-") || Row.DefaultValue == TEXT("nullptr"))
	{
		return 0;
	}
	return FCString::Atoi(*Row.DefaultValue);
}

bool UCameraCompletedParameterHelpers::GetDefaultValueAsBool(const FCameraCompletedParameterRow& Row)
{
	return Row.DefaultValue.Equals(TEXT("true"), ESearchCase::IgnoreCase);
}

FName UCameraCompletedParameterHelpers::GetDefaultValueAsName(const FCameraCompletedParameterRow& Row)
{
	if (Row.DefaultValue.IsEmpty() || Row.DefaultValue == TEXT("-") || Row.DefaultValue == TEXT("nullptr"))
	{
		return NAME_None;
	}
	return FName(*Row.DefaultValue);
}

FString UCameraCompletedParameterHelpers::GetDefaultValueAsString(const FCameraCompletedParameterRow& Row)
{
	return Row.DefaultValue;
}

// ========== Limit Check Functions ==========

bool UCameraCompletedParameterHelpers::HasMinLimit(const FCameraCompletedParameterRow& Row)
{
	return !Row.MinValue.IsEmpty() && Row.MinValue != TEXT("-");
}

bool UCameraCompletedParameterHelpers::HasMaxLimit(const FCameraCompletedParameterRow& Row)
{
	return !Row.MaxValue.IsEmpty() && Row.MaxValue != TEXT("-");
}

float UCameraCompletedParameterHelpers::GetMinValueAsFloat(const FCameraCompletedParameterRow& Row)
{
	if (!HasMinLimit(Row))
	{
		return TNumericLimits<float>::Lowest();
	}
	return FCString::Atof(*Row.MinValue);
}

float UCameraCompletedParameterHelpers::GetMaxValueAsFloat(const FCameraCompletedParameterRow& Row)
{
	if (!HasMaxLimit(Row))
	{
		return TNumericLimits<float>::Max();
	}
	return FCString::Atof(*Row.MaxValue);
}

// ========== Array Parsing Functions ==========

TArray<FString> UCameraCompletedParameterHelpers::GetRelatedToArray(const FCameraCompletedParameterRow& Row)
{
	TArray<FString> Result;

	if (Row.RelatedTo.IsEmpty() || Row.RelatedTo == TEXT("-"))
	{
		return Result;
	}

	Row.RelatedTo.ParseIntoArray(Result, TEXT("/"), true);

	for (FString& Str : Result)
	{
		Str = Str.TrimStartAndEnd();
	}

	return Result;
}

// ========== Type Check Functions ==========

bool UCameraCompletedParameterHelpers::IsFloatType(const FCameraCompletedParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("float"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsIntType(const FCameraCompletedParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("int32"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Equals(TEXT("int"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsBoolType(const FCameraCompletedParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("bool"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsNameType(const FCameraCompletedParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("FName"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsStringType(const FCameraCompletedParameterRow& Row)
{
	return Row.DataType.Equals(TEXT("FString"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsEnumType(const FCameraCompletedParameterRow& Row)
{
	return Row.DataType.StartsWith(TEXT("E"), ESearchCase::CaseSensitive) ||
	       Row.DataType.Contains(TEXT("Blend"), ESearchCase::IgnoreCase) ||
	       Row.DataType.Contains(TEXT("Channel"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsPointerType(const FCameraCompletedParameterRow& Row)
{
	return Row.DataType.Contains(TEXT("*"), ESearchCase::IgnoreCase) ||
	       Row.DataType.StartsWith(TEXT("U"), ESearchCase::CaseSensitive) ||
	       Row.DefaultValue.Equals(TEXT("nullptr"), ESearchCase::IgnoreCase);
}

// ========== Group Check Functions ==========

bool UCameraCompletedParameterHelpers::IsStateBaseGroup(const FCameraCompletedParameterRow& Row)
{
	return Row.Group.Equals(TEXT("StateBase"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsModuleGroup(const FCameraCompletedParameterRow& Row)
{
	return Row.Group.Equals(TEXT("Module"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsModifierGroup(const FCameraCompletedParameterRow& Row)
{
	return Row.Group.Equals(TEXT("Modifier"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsCollisionGroup(const FCameraCompletedParameterRow& Row)
{
	return Row.Group.Equals(TEXT("Collision"), ESearchCase::IgnoreCase);
}

bool UCameraCompletedParameterHelpers::IsGlobalGroup(const FCameraCompletedParameterRow& Row)
{
	return Row.Group.Equals(TEXT("Global"), ESearchCase::IgnoreCase);
}

// ========== Enum Conversion Functions ==========

EParameterGroup UCameraCompletedParameterHelpers::StringToGroup(const FString& GroupString)
{
	FString TrimmedString = GroupString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("StateBase"), ESearchCase::IgnoreCase))
		return EParameterGroup::StateBase;
	else if (TrimmedString.Equals(TEXT("Module"), ESearchCase::IgnoreCase))
		return EParameterGroup::Module;
	else if (TrimmedString.Equals(TEXT("Modifier"), ESearchCase::IgnoreCase))
		return EParameterGroup::Modifier;
	else if (TrimmedString.Equals(TEXT("Collision"), ESearchCase::IgnoreCase))
		return EParameterGroup::Collision;
	else if (TrimmedString.Equals(TEXT("Global"), ESearchCase::IgnoreCase))
		return EParameterGroup::Global;
	return EParameterGroup::StateBase;
}

FString UCameraCompletedParameterHelpers::GroupToString(EParameterGroup Group)
{
	switch (Group)
	{
	case EParameterGroup::StateBase:   return TEXT("StateBase");
	case EParameterGroup::Module:      return TEXT("Module");
	case EParameterGroup::Modifier:    return TEXT("Modifier");
	case EParameterGroup::Collision:   return TEXT("Collision");
	case EParameterGroup::Global:      return TEXT("Global");
	default: return TEXT("StateBase");
	}
}

EParameterSubGroup UCameraCompletedParameterHelpers::StringToSubGroup(const FString& SubGroupString)
{
	FString TrimmedString = SubGroupString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("Identity"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Identity;
	else if (TrimmedString.Equals(TEXT("Distance"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Distance;
	else if (TrimmedString.Equals(TEXT("FOV"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::FOV;
	else if (TrimmedString.Equals(TEXT("Rotation"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Rotation;
	else if (TrimmedString.Equals(TEXT("Offset"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Offset;
	else if (TrimmedString.Equals(TEXT("Lag"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Lag;
	else if (TrimmedString.Equals(TEXT("Transition"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Transition;
	else if (TrimmedString.Equals(TEXT("Collision"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Collision;
	else if (TrimmedString.Equals(TEXT("AutoCorrect"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::AutoCorrect;
	else if (TrimmedString.Equals(TEXT("Flag"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Flag;
	else if (TrimmedString.Equals(TEXT("Hierarchy"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Hierarchy;
	else if (TrimmedString.Equals(TEXT("Position"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Position;
	else if (TrimmedString.Equals(TEXT("Constraint"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Constraint;
	else if (TrimmedString.Equals(TEXT("Shake"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Shake;
	else if (TrimmedString.Equals(TEXT("Reaction"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Reaction;
	else if (TrimmedString.Equals(TEXT("Cinematic"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Cinematic;
	else if (TrimmedString.Equals(TEXT("Zoom"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Zoom;
	else if (TrimmedString.Equals(TEXT("Effect"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Effect;
	else if (TrimmedString.Equals(TEXT("Special"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Special;
	else if (TrimmedString.Equals(TEXT("Detection"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Detection;
	else if (TrimmedString.Equals(TEXT("Response"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Response;
	else if (TrimmedString.Equals(TEXT("Occlusion"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Occlusion;
	else if (TrimmedString.Equals(TEXT("Recovery"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Recovery;
	else if (TrimmedString.Equals(TEXT("Limits"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Limits;
	else if (TrimmedString.Equals(TEXT("Smoothing"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Smoothing;
	else if (TrimmedString.Equals(TEXT("Debug"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Debug;
	else if (TrimmedString.Equals(TEXT("LockOn"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::LockOn;
	else if (TrimmedString.Equals(TEXT("Performance"), ESearchCase::IgnoreCase))
		return EParameterSubGroup::Performance;
	return EParameterSubGroup::Identity;
}

FString UCameraCompletedParameterHelpers::SubGroupToString(EParameterSubGroup SubGroup)
{
	switch (SubGroup)
	{
	case EParameterSubGroup::Identity:    return TEXT("Identity");
	case EParameterSubGroup::Distance:    return TEXT("Distance");
	case EParameterSubGroup::FOV:         return TEXT("FOV");
	case EParameterSubGroup::Rotation:    return TEXT("Rotation");
	case EParameterSubGroup::Offset:      return TEXT("Offset");
	case EParameterSubGroup::Lag:         return TEXT("Lag");
	case EParameterSubGroup::Transition:  return TEXT("Transition");
	case EParameterSubGroup::Collision:   return TEXT("Collision");
	case EParameterSubGroup::AutoCorrect: return TEXT("AutoCorrect");
	case EParameterSubGroup::Flag:        return TEXT("Flag");
	case EParameterSubGroup::Hierarchy:   return TEXT("Hierarchy");
	case EParameterSubGroup::Position:    return TEXT("Position");
	case EParameterSubGroup::Constraint:  return TEXT("Constraint");
	case EParameterSubGroup::Shake:       return TEXT("Shake");
	case EParameterSubGroup::Reaction:    return TEXT("Reaction");
	case EParameterSubGroup::Cinematic:   return TEXT("Cinematic");
	case EParameterSubGroup::Zoom:        return TEXT("Zoom");
	case EParameterSubGroup::Effect:      return TEXT("Effect");
	case EParameterSubGroup::Special:     return TEXT("Special");
	case EParameterSubGroup::Detection:   return TEXT("Detection");
	case EParameterSubGroup::Response:    return TEXT("Response");
	case EParameterSubGroup::Occlusion:   return TEXT("Occlusion");
	case EParameterSubGroup::Recovery:    return TEXT("Recovery");
	case EParameterSubGroup::Limits:      return TEXT("Limits");
	case EParameterSubGroup::Smoothing:   return TEXT("Smoothing");
	case EParameterSubGroup::Debug:       return TEXT("Debug");
	case EParameterSubGroup::LockOn:      return TEXT("LockOn");
	case EParameterSubGroup::Performance: return TEXT("Performance");
	default: return TEXT("Identity");
	}
}

EParameterDataType UCameraCompletedParameterHelpers::StringToDataType(const FString& DataTypeString)
{
	FString TrimmedString = DataTypeString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("FName"), ESearchCase::IgnoreCase))
		return EParameterDataType::Name;
	else if (TrimmedString.Equals(TEXT("FString"), ESearchCase::IgnoreCase))
		return EParameterDataType::String;
	else if (TrimmedString.Equals(TEXT("int32"), ESearchCase::IgnoreCase) ||
	         TrimmedString.Equals(TEXT("int"), ESearchCase::IgnoreCase))
		return EParameterDataType::Int32;
	else if (TrimmedString.Equals(TEXT("float"), ESearchCase::IgnoreCase))
		return EParameterDataType::Float;
	else if (TrimmedString.Equals(TEXT("bool"), ESearchCase::IgnoreCase))
		return EParameterDataType::Bool;
	else if (TrimmedString.Contains(TEXT("EBlendType"), ESearchCase::IgnoreCase))
		return EParameterDataType::BlendType;
	else if (TrimmedString.Contains(TEXT("ECollisionChannel"), ESearchCase::IgnoreCase))
		return EParameterDataType::CollisionChannel;
	else if (TrimmedString.Contains(TEXT("UCurveFloat"), ESearchCase::IgnoreCase) ||
	         TrimmedString.Contains(TEXT("*"), ESearchCase::IgnoreCase))
		return EParameterDataType::CurveFloat;
	return EParameterDataType::Float;
}

FString UCameraCompletedParameterHelpers::DataTypeToString(EParameterDataType DataType)
{
	switch (DataType)
	{
	case EParameterDataType::Name:             return TEXT("FName");
	case EParameterDataType::String:           return TEXT("FString");
	case EParameterDataType::Int32:            return TEXT("int32");
	case EParameterDataType::Float:            return TEXT("float");
	case EParameterDataType::Bool:             return TEXT("bool");
	case EParameterDataType::BlendType:        return TEXT("EBlendType");
	case EParameterDataType::CollisionChannel: return TEXT("ECollisionChannel");
	case EParameterDataType::CurveFloat:       return TEXT("UCurveFloat*");
	default: return TEXT("float");
	}
}

// ========== Display Name Functions ==========

FString UCameraCompletedParameterHelpers::GetGroupDisplayName(EParameterGroup Group)
{
	switch (Group)
	{
	case EParameterGroup::StateBase:  return TEXT("State Base");
	case EParameterGroup::Module:     return TEXT("Module");
	case EParameterGroup::Modifier:   return TEXT("Modifier");
	case EParameterGroup::Collision:  return TEXT("Collision");
	case EParameterGroup::Global:     return TEXT("Global");
	default: return TEXT("Unknown");
	}
}

FString UCameraCompletedParameterHelpers::GetSubGroupDisplayName(EParameterSubGroup SubGroup)
{
	switch (SubGroup)
	{
	case EParameterSubGroup::Identity:    return TEXT("Identity");
	case EParameterSubGroup::Distance:    return TEXT("Distance");
	case EParameterSubGroup::FOV:         return TEXT("FOV");
	case EParameterSubGroup::Rotation:    return TEXT("Rotation");
	case EParameterSubGroup::Offset:      return TEXT("Offset");
	case EParameterSubGroup::Lag:         return TEXT("Lag");
	case EParameterSubGroup::Transition:  return TEXT("Transition");
	case EParameterSubGroup::Collision:   return TEXT("Collision");
	case EParameterSubGroup::AutoCorrect: return TEXT("Auto Correct");
	case EParameterSubGroup::Flag:        return TEXT("Flag");
	case EParameterSubGroup::Hierarchy:   return TEXT("Hierarchy");
	case EParameterSubGroup::Position:    return TEXT("Position");
	case EParameterSubGroup::Constraint:  return TEXT("Constraint");
	case EParameterSubGroup::Shake:       return TEXT("Shake");
	case EParameterSubGroup::Reaction:    return TEXT("Reaction");
	case EParameterSubGroup::Cinematic:   return TEXT("Cinematic");
	case EParameterSubGroup::Zoom:        return TEXT("Zoom");
	case EParameterSubGroup::Effect:      return TEXT("Effect");
	case EParameterSubGroup::Special:     return TEXT("Special");
	case EParameterSubGroup::Detection:   return TEXT("Detection");
	case EParameterSubGroup::Response:    return TEXT("Response");
	case EParameterSubGroup::Occlusion:   return TEXT("Occlusion");
	case EParameterSubGroup::Recovery:    return TEXT("Recovery");
	case EParameterSubGroup::Limits:      return TEXT("Limits");
	case EParameterSubGroup::Smoothing:   return TEXT("Smoothing");
	case EParameterSubGroup::Debug:       return TEXT("Debug");
	case EParameterSubGroup::LockOn:      return TEXT("Lock On");
	case EParameterSubGroup::Performance: return TEXT("Performance");
	default: return TEXT("Unknown");
	}
}

FString UCameraCompletedParameterHelpers::GetDataTypeDisplayName(EParameterDataType DataType)
{
	switch (DataType)
	{
	case EParameterDataType::Name:             return TEXT("Name");
	case EParameterDataType::String:           return TEXT("String");
	case EParameterDataType::Int32:            return TEXT("Integer");
	case EParameterDataType::Float:            return TEXT("Float");
	case EParameterDataType::Bool:             return TEXT("Boolean");
	case EParameterDataType::BlendType:        return TEXT("Blend Type");
	case EParameterDataType::CollisionChannel: return TEXT("Collision Channel");
	case EParameterDataType::CurveFloat:       return TEXT("Float Curve");
	default: return TEXT("Unknown");
	}
}
