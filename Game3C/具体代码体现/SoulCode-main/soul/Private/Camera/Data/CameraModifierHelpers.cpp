// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/Data/CameraModifierHelpers.h"

// ========================================
// Duration Parsing Functions Implementation
// ========================================

float UCameraModifierHelpers::GetDurationAsFloat(const FCameraModifierRow& Row)
{
	if (Row.Duration.IsEmpty() || Row.Duration == TEXT("-"))
	{
		return -1.0f;
	}

	if (Row.Duration.Contains(TEXT("-")) && Row.Duration.Left(1) != TEXT("-"))
	{
		return -1.0f;
	}

	if (Row.Duration.IsNumeric() || 
		(Row.Duration.Contains(TEXT("."))))
	{
		return FCString::Atof(*Row.Duration);
	}

	return -1.0f;
}

bool UCameraModifierHelpers::HasFixedDuration(const FCameraModifierRow& Row)
{
	float Duration = GetDurationAsFloat(Row);
	return Duration >= 0.0f;
}

bool UCameraModifierHelpers::GetDurationRange(const FCameraModifierRow& Row, float& OutMin, float& OutMax)
{
	OutMin = 0.0f;
	OutMax = 0.0f;

	if (Row.Duration.IsEmpty())
	{
		return false;
	}

	int32 DashIndex = INDEX_NONE;
	FString CheckString = Row.Duration;
	if (CheckString.StartsWith(TEXT("-")))
	{
		return false;
	}

	if (CheckString.FindChar(TEXT('-'), DashIndex) && DashIndex > 0)
	{
		FString MinStr = CheckString.Left(DashIndex).TrimStartAndEnd();
		FString MaxStr = CheckString.Mid(DashIndex + 1).TrimStartAndEnd();
		
		if (!MinStr.IsEmpty() && !MaxStr.IsEmpty())
		{
			OutMin = FCString::Atof(*MinStr);
			OutMax = FCString::Atof(*MaxStr);
			return OutMax > OutMin;
		}
	}

	return false;
}

bool UCameraModifierHelpers::IsRangeDuration(const FCameraModifierRow& Row)
{
	float Min, Max;
	return GetDurationRange(Row, Min, Max);
}

// ========================================
// Category Check Functions Implementation
// ========================================

bool UCameraModifierHelpers::IsShakeModifier(const FCameraModifierRow& Row)
{
	return Row.Category.Equals(TEXT("Shake"), ESearchCase::IgnoreCase);
}

bool UCameraModifierHelpers::IsReactionModifier(const FCameraModifierRow& Row)
{
	return Row.Category.Equals(TEXT("Reaction"), ESearchCase::IgnoreCase);
}

bool UCameraModifierHelpers::IsCinematicModifier(const FCameraModifierRow& Row)
{
	return Row.Category.Equals(TEXT("Cinematic"), ESearchCase::IgnoreCase);
}

bool UCameraModifierHelpers::IsZoomModifier(const FCameraModifierRow& Row)
{
	return Row.Category.Equals(TEXT("Zoom"), ESearchCase::IgnoreCase);
}

bool UCameraModifierHelpers::IsEffectModifier(const FCameraModifierRow& Row)
{
	return Row.Category.Equals(TEXT("Effect"), ESearchCase::IgnoreCase);
}

bool UCameraModifierHelpers::IsSpecialModifier(const FCameraModifierRow& Row)
{
	return Row.Category.Equals(TEXT("Special"), ESearchCase::IgnoreCase);
}

// ========================================
// SubCategory Check Functions Implementation
// ========================================

bool UCameraModifierHelpers::IsCombatSubCategory(const FCameraModifierRow& Row)
{
	return Row.SubCategory.Equals(TEXT("Combat"), ESearchCase::IgnoreCase);
}

bool UCameraModifierHelpers::IsBossSubCategory(const FCameraModifierRow& Row)
{
	return Row.SubCategory.Equals(TEXT("Boss"), ESearchCase::IgnoreCase);
}

bool UCameraModifierHelpers::IsEnvironmentSubCategory(const FCameraModifierRow& Row)
{
	return Row.SubCategory.Equals(TEXT("Environment"), ESearchCase::IgnoreCase);
}

// ========================================
// Effect Check Functions Implementation
// ========================================

bool UCameraModifierHelpers::AffectsTransform(const FCameraModifierRow& Row)
{
	return Row.bAffectsPosition || Row.bAffectsRotation;
}

bool UCameraModifierHelpers::AffectsVisuals(const FCameraModifierRow& Row)
{
	return Row.bAffectsFOV || Row.bAffectsPostProcess;
}

// ========================================
// Enum Conversion Functions Implementation
// ========================================

ECameraModifierCategory UCameraModifierHelpers::StringToCategory(const FString& CategoryString)
{
	FString TrimmedString = CategoryString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("Shake"), ESearchCase::IgnoreCase))
		return ECameraModifierCategory::Shake;
	else if (TrimmedString.Equals(TEXT("Reaction"), ESearchCase::IgnoreCase))
		return ECameraModifierCategory::Reaction;
	else if (TrimmedString.Equals(TEXT("Cinematic"), ESearchCase::IgnoreCase))
		return ECameraModifierCategory::Cinematic;
	else if (TrimmedString.Equals(TEXT("Zoom"), ESearchCase::IgnoreCase))
		return ECameraModifierCategory::Zoom;
	else if (TrimmedString.Equals(TEXT("Effect"), ESearchCase::IgnoreCase))
		return ECameraModifierCategory::Effect;
	else if (TrimmedString.Equals(TEXT("Special"), ESearchCase::IgnoreCase))
		return ECameraModifierCategory::Special;
	return ECameraModifierCategory::Shake;
}

FString UCameraModifierHelpers::CategoryToString(ECameraModifierCategory Category)
{
	switch (Category)
	{
	case ECameraModifierCategory::Shake:     return TEXT("Shake");
	case ECameraModifierCategory::Reaction:  return TEXT("Reaction");
	case ECameraModifierCategory::Cinematic: return TEXT("Cinematic");
	case ECameraModifierCategory::Zoom:      return TEXT("Zoom");
	case ECameraModifierCategory::Effect:    return TEXT("Effect");
	case ECameraModifierCategory::Special:   return TEXT("Special");
	default:  return TEXT("Shake");
	}
}

ECameraModifierSubCategory UCameraModifierHelpers::StringToSubCategory(const FString& SubCategoryString)
{
	FString TrimmedString = SubCategoryString.TrimStartAndEnd();
	if (TrimmedString.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Combat;
	else if (TrimmedString.Equals(TEXT("Environment"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Environment;
	else if (TrimmedString.Equals(TEXT("Movement"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Movement;
	else if (TrimmedString.Equals(TEXT("Special"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Special;
	else if (TrimmedString.Equals(TEXT("Boss"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Boss;
	else if (TrimmedString.Equals(TEXT("Status"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Status;
	else if (TrimmedString.Equals(TEXT("Aim"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Aim;
	else if (TrimmedString.Equals(TEXT("Time"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Time;
	else if (TrimmedString.Equals(TEXT("Death"), ESearchCase::IgnoreCase))
		return ECameraModifierSubCategory::Death;
	return ECameraModifierSubCategory::Combat;
}

FString UCameraModifierHelpers::SubCategoryToString(ECameraModifierSubCategory SubCategory)
{
	switch (SubCategory)
	{
	case ECameraModifierSubCategory::Combat:      return TEXT("Combat");
	case ECameraModifierSubCategory::Environment: return TEXT("Environment");
	case ECameraModifierSubCategory::Movement:    return TEXT("Movement");
	case ECameraModifierSubCategory::Special:     return TEXT("Special");
	case ECameraModifierSubCategory::Boss:        return TEXT("Boss");
	case ECameraModifierSubCategory::Status:      return TEXT("Status");
	case ECameraModifierSubCategory::Aim:         return TEXT("Aim");
	case ECameraModifierSubCategory::Time:        return TEXT("Time");
	case ECameraModifierSubCategory::Death:       return TEXT("Death");
	default: return TEXT("Combat");
	}
}

// ========================================
// Display Name Functions Implementation
// ========================================

FString UCameraModifierHelpers::GetCategoryDisplayName(ECameraModifierCategory Category)
{
	switch (Category)
	{
	case ECameraModifierCategory::Shake:     return TEXT("Shake");
	case ECameraModifierCategory::Reaction:  return TEXT("Reaction");
	case ECameraModifierCategory::Cinematic: return TEXT("Cinematic");
	case ECameraModifierCategory::Zoom:      return TEXT("Zoom");
	case ECameraModifierCategory::Effect:    return TEXT("Effect");
	case ECameraModifierCategory::Special:   return TEXT("Special");
	default: return TEXT("Unknown");
	}
}

FString UCameraModifierHelpers::GetSubCategoryDisplayName(ECameraModifierSubCategory SubCategory)
{
	switch (SubCategory)
	{
	case ECameraModifierSubCategory::Combat:      return TEXT("Combat");
	case ECameraModifierSubCategory::Environment: return TEXT("Environment");
	case ECameraModifierSubCategory::Movement:    return TEXT("Movement");
	case ECameraModifierSubCategory::Special:     return TEXT("Special");
	case ECameraModifierSubCategory::Boss:        return TEXT("Boss");
	case ECameraModifierSubCategory::Status:      return TEXT("Status");
	case ECameraModifierSubCategory::Aim:         return TEXT("Aim");
	case ECameraModifierSubCategory::Time:        return TEXT("Time");
	case ECameraModifierSubCategory::Death:       return TEXT("Death");
	default: return TEXT("Unknown");
	}
}
