// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/CameraModuleHelpers.h"

// ========================================
// Array Parsing Functions
// ========================================

TArray<FString> UCameraModuleHelpers::GetActiveStatesArray(const FCameraModuleRow& Row)
{
	TArray<FString> Result;
	if (Row.ActiveStates.IsEmpty())
	{
		return Result;
	}
	
	Row.ActiveStates.ParseIntoArray(Result, TEXT("/"), true);
	for (FString& Str : Result)
	{
		Str = Str.TrimStartAndEnd();
	}
	return Result;
}

TArray<FString> UCameraModuleHelpers::GetInputParamsArray(const FCameraModuleRow& Row)
{
	TArray<FString> Result;
	if (Row.InputParams.IsEmpty())
	{
		return Result;
	}
	
	Row.InputParams.ParseIntoArray(Result, TEXT("/"), true);
	for (FString& Str : Result)
	{
		Str = Str.TrimStartAndEnd();
	}
	return Result;
}

// ========================================
// Category Check Functions
// ========================================

bool UCameraModuleHelpers::IsPositionModule(const FCameraModuleRow& Row)
{
	return Row.Category.Equals(TEXT("Position"), ESearchCase::IgnoreCase);
}

bool UCameraModuleHelpers::IsRotationModule(const FCameraModuleRow& Row)
{
	return Row.Category.Equals(TEXT("Rotation"), ESearchCase::IgnoreCase);
}

bool UCameraModuleHelpers::IsDistanceModule(const FCameraModuleRow& Row)
{
	return Row.Category.Equals(TEXT("Distance"), ESearchCase::IgnoreCase);
}

bool UCameraModuleHelpers::IsFOVModule(const FCameraModuleRow& Row)
{
	return Row.Category.Equals(TEXT("FOV"), ESearchCase::IgnoreCase);
}

bool UCameraModuleHelpers::IsOffsetModule(const FCameraModuleRow& Row)
{
	return Row.Category.Equals(TEXT("Offset"), ESearchCase::IgnoreCase);
}

bool UCameraModuleHelpers::IsConstraintModule(const FCameraModuleRow& Row)
{
	return Row.Category.Equals(TEXT("Constraint"), ESearchCase::IgnoreCase);
}

// ========================================
// Blend Policy Check Functions
// ========================================

bool UCameraModuleHelpers::IsOverrideBlend(const FCameraModuleRow& Row)
{
	return Row.BlendPolicy.Equals(TEXT("Override"), ESearchCase::IgnoreCase);
}

bool UCameraModuleHelpers::IsAdditiveBlend(const FCameraModuleRow& Row)
{
	return Row.BlendPolicy.Equals(TEXT("Additive"), ESearchCase::IgnoreCase);
}

bool UCameraModuleHelpers::IsMultiplicativeBlend(const FCameraModuleRow& Row)
{
	return Row.BlendPolicy.Equals(TEXT("Multiplicative"), ESearchCase::IgnoreCase);
}

// ========================================
// State Check Functions
// ========================================

bool UCameraModuleHelpers::IsActiveInState(const FCameraModuleRow& Row, const FString& StateName)
{
	if (Row.ActiveStates.Equals(TEXT("All"), ESearchCase::IgnoreCase))
	{
		return true;
	}
	
	TArray<FString> States = GetActiveStatesArray(Row);
	for (const FString& State : States)
	{
		if (State.Equals(StateName, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}
	return false;
}

bool UCameraModuleHelpers::IsActiveInAllStates(const FCameraModuleRow& Row)
{
	return Row.ActiveStates.Equals(TEXT("All"), ESearchCase::IgnoreCase);
}

// ========================================
// Enum Conversion Functions
// ========================================

ECameraModuleCategory UCameraModuleHelpers::StringToCategory(const FString& CategoryString)
{
	FString TrimmedString = CategoryString.TrimStartAndEnd();
	
	if (TrimmedString.Equals(TEXT("Position"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryPosition;
	else if (TrimmedString.Equals(TEXT("Rotation"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryRotation;
	else if (TrimmedString.Equals(TEXT("Distance"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryDistance;
	else if (TrimmedString.Equals(TEXT("FOV"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryFOV;
	else if (TrimmedString.Equals(TEXT("Offset"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryOffset;
	else if (TrimmedString.Equals(TEXT("Constraint"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryConstraint;
	else if (TrimmedString.Equals(TEXT("LockOn"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryLockOn;
	else if (TrimmedString.Equals(TEXT("Follow"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryFollow;
	else if (TrimmedString.Equals(TEXT("Orbit"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryOrbit;
	else if (TrimmedString.Equals(TEXT("Cinematic"), ESearchCase::IgnoreCase))
		return ECameraModuleCategory::CategoryCinematic;
	
	return ECameraModuleCategory::CategoryNone;
}

FString UCameraModuleHelpers::CategoryToString(ECameraModuleCategory Category)
{
	switch (Category)
	{
	case ECameraModuleCategory::CategoryNone:
		return TEXT("None");
	case ECameraModuleCategory::CategoryPosition:
		return TEXT("Position");
	case ECameraModuleCategory::CategoryRotation:
		return TEXT("Rotation");
	case ECameraModuleCategory::CategoryDistance:
		return TEXT("Distance");
	case ECameraModuleCategory::CategoryFOV:
		return TEXT("FOV");
	case ECameraModuleCategory::CategoryOffset:
		return TEXT("Offset");
	case ECameraModuleCategory::CategoryConstraint:
		return TEXT("Constraint");
	case ECameraModuleCategory::CategoryLockOn:
		return TEXT("LockOn");
	case ECameraModuleCategory::CategoryFollow:
		return TEXT("Follow");
	case ECameraModuleCategory::CategoryOrbit:
		return TEXT("Orbit");
	case ECameraModuleCategory::CategoryCinematic:
		return TEXT("Cinematic");
	default:
		return TEXT("None");
	}
}

ECameraModuleBlendPolicy UCameraModuleHelpers::StringToBlendPolicy(const FString& BlendPolicyString)
{
	FString TrimmedString = BlendPolicyString.TrimStartAndEnd();
	
	if (TrimmedString.Equals(TEXT("Override"), ESearchCase::IgnoreCase))
		return ECameraModuleBlendPolicy::Override;
	else if (TrimmedString.Equals(TEXT("Additive"), ESearchCase::IgnoreCase))
		return ECameraModuleBlendPolicy::Additive;
	else if (TrimmedString.Equals(TEXT("Multiplicative"), ESearchCase::IgnoreCase))
		return ECameraModuleBlendPolicy::Multiplicative;
	else if (TrimmedString.Equals(TEXT("Minimum"), ESearchCase::IgnoreCase))
		return ECameraModuleBlendPolicy::Minimum;
	else if (TrimmedString.Equals(TEXT("Blend"), ESearchCase::IgnoreCase))
		return ECameraModuleBlendPolicy::Blend;
	
	return ECameraModuleBlendPolicy::Override;
}

FString UCameraModuleHelpers::BlendPolicyToString(ECameraModuleBlendPolicy BlendPolicy)
{
	switch (BlendPolicy)
	{
	case ECameraModuleBlendPolicy::Override:
		return TEXT("Override");
	case ECameraModuleBlendPolicy::Additive:
		return TEXT("Additive");
	case ECameraModuleBlendPolicy::Multiplicative:
		return TEXT("Multiplicative");
	case ECameraModuleBlendPolicy::Minimum:
		return TEXT("Minimum");
	case ECameraModuleBlendPolicy::Blend:
		return TEXT("Blend");
	default:
		return TEXT("Override");
	}
}

// ========================================
// Display Name Functions Implementation
// ========================================

FString UCameraModuleHelpers::GetCategoryDisplayName(ECameraModuleCategory Category)
{
	switch (Category)
	{
	case ECameraModuleCategory::CategoryNone:
		return TEXT("None");
	case ECameraModuleCategory::CategoryPosition:
		return TEXT("Position");
	case ECameraModuleCategory::CategoryRotation:
		return TEXT("Rotation");
	case ECameraModuleCategory::CategoryDistance:
		return TEXT("Distance");
	case ECameraModuleCategory::CategoryFOV:
		return TEXT("FOV");
	case ECameraModuleCategory::CategoryOffset:
		return TEXT("Offset");
	case ECameraModuleCategory::CategoryConstraint:
		return TEXT("Constraint");
	case ECameraModuleCategory::CategoryLockOn:
		return TEXT("LockOn");
	case ECameraModuleCategory::CategoryFollow:
		return TEXT("Follow");
	case ECameraModuleCategory::CategoryOrbit:
		return TEXT("Orbit");
	case ECameraModuleCategory::CategoryCinematic:
		return TEXT("Cinematic");
	default:
		return TEXT("Unknown");
	}
}

FString UCameraModuleHelpers::GetBlendPolicyDisplayName(ECameraModuleBlendPolicy BlendPolicy)
{
	switch (BlendPolicy)
	{
	case ECameraModuleBlendPolicy::Override:
		return TEXT("Override");
	case ECameraModuleBlendPolicy::Additive:
		return TEXT("Additive");
	case ECameraModuleBlendPolicy::Multiplicative:
		return TEXT("Multiplicative");
	case ECameraModuleBlendPolicy::Minimum:
		return TEXT("Minimum");
	case ECameraModuleBlendPolicy::Blend:
		return TEXT("Blend");
	default:
		return TEXT("Unknown");
	}
}
