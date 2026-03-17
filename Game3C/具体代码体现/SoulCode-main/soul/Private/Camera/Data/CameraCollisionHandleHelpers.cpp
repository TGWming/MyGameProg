// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/CameraCollisionHandleHelpers.h"

// ========================================
// 数组解析函数实现
// ========================================

TArray<FString> UCameraCollisionHandleHelpers::GetInputParamsArray(const FCameraCollisionHandleRow& Row)
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

TArray<FString> UCameraCollisionHandleHelpers::GetOutputParamsArray(const FCameraCollisionHandleRow& Row)
{
	TArray<FString> Result;
	if (Row.OutputParams.IsEmpty()) 
	{
		return Result;
	}
	
	Row.OutputParams.ParseIntoArray(Result, TEXT("/"), true);
	for (FString& Str : Result) 
	{ 
		Str = Str.TrimStartAndEnd(); 
	}
	
	return Result;
}

// ========================================
// 分类检查函数实现
// ========================================

bool UCameraCollisionHandleHelpers::IsDetectionStrategy(const FCameraCollisionHandleRow& Row)
{
	return Row.Category.Equals(TEXT("Detection"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionHandleHelpers::IsResponseStrategy(const FCameraCollisionHandleRow& Row)
{
	return Row.Category.Equals(TEXT("Response"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionHandleHelpers::IsOcclusionStrategy(const FCameraCollisionHandleRow& Row)
{
	return Row.Category.Equals(TEXT("Occlusion"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionHandleHelpers::IsRecoveryStrategy(const FCameraCollisionHandleRow& Row)
{
	return Row.Category.Equals(TEXT("Recovery"), ESearchCase::IgnoreCase);
}

bool UCameraCollisionHandleHelpers::IsSpecialStrategy(const FCameraCollisionHandleRow& Row)
{
	return Row.Category.Equals(TEXT("Special"), ESearchCase::IgnoreCase);
}

// ========================================
// 枚举转换函数实现
// ========================================

ECollisionHandleCategory UCameraCollisionHandleHelpers::StringToCategory(const FString& CategoryString)
{
	FString TrimmedString = CategoryString.TrimStartAndEnd();
	
	if (TrimmedString.Equals(TEXT("Detection"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleCategory::Detection;
	}
	else if (TrimmedString.Equals(TEXT("Response"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleCategory::Response;
	}
	else if (TrimmedString.Equals(TEXT("Occlusion"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleCategory::Occlusion;
	}
	else if (TrimmedString.Equals(TEXT("Recovery"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleCategory::Recovery;
	}
	else if (TrimmedString.Equals(TEXT("Special"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleCategory::Special;
	}
	
	return ECollisionHandleCategory::Detection;
}

FString UCameraCollisionHandleHelpers::CategoryToString(ECollisionHandleCategory Category)
{
	switch (Category)
	{
		case ECollisionHandleCategory::Detection:	return TEXT("Detection");
		case ECollisionHandleCategory::Response:	return TEXT("Response");
		case ECollisionHandleCategory::Occlusion:	return TEXT("Occlusion");
		case ECollisionHandleCategory::Recovery:	return TEXT("Recovery");
		case ECollisionHandleCategory::Special:		return TEXT("Special");
		default:									return TEXT("Detection");
	}
}

ECollisionHandleType UCameraCollisionHandleHelpers::StringToType(const FString& TypeString)
{
	FString TrimmedString = TypeString.TrimStartAndEnd();
	
	if (TrimmedString.Equals(TEXT("Trace"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Trace;
	}
	else if (TrimmedString.Equals(TEXT("Sweep"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Sweep;
	}
	else if (TrimmedString.Equals(TEXT("Position"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Position;
	}
	else if (TrimmedString.Equals(TEXT("FOV"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::FOV;
	}
	else if (TrimmedString.Equals(TEXT("Material"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Material;
	}
	else if (TrimmedString.Equals(TEXT("Visibility"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Visibility;
	}
	else if (TrimmedString.Equals(TEXT("Timing"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Timing;
	}
	else if (TrimmedString.Equals(TEXT("Interpolation"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Interpolation;
	}
	else if (TrimmedString.Equals(TEXT("Environment"), ESearchCase::IgnoreCase))
	{
		return ECollisionHandleType::Environment;
	}
	
	return ECollisionHandleType::Trace;
}

FString UCameraCollisionHandleHelpers::TypeToString(ECollisionHandleType Type)
{
	switch (Type)
	{
		case ECollisionHandleType::Trace:			return TEXT("Trace");
		case ECollisionHandleType::Sweep:			return TEXT("Sweep");
		case ECollisionHandleType::Position:		return TEXT("Position");
		case ECollisionHandleType::FOV:				return TEXT("FOV");
		case ECollisionHandleType::Material:		return TEXT("Material");
		case ECollisionHandleType::Visibility:		return TEXT("Visibility");
		case ECollisionHandleType::Timing:			return TEXT("Timing");
		case ECollisionHandleType::Interpolation:	return TEXT("Interpolation");
		case ECollisionHandleType::Environment:		return TEXT("Environment");
		default:									return TEXT("Trace");
	}
}

// ========================================
// 中文显示名称实现
// ========================================

FString UCameraCollisionHandleHelpers::GetCategoryDisplayName(ECollisionHandleCategory Category)
{
	switch (Category)
	{
		case ECollisionHandleCategory::Detection:	return TEXT("Detection");
		case ECollisionHandleCategory::Response:	return TEXT("Response");
		case ECollisionHandleCategory::Occlusion:	return TEXT("Occlusion");
		case ECollisionHandleCategory::Recovery:	return TEXT("Recovery");
		case ECollisionHandleCategory::Special:		return TEXT("Special");
		default:									return TEXT("Unknown");
	}
}

FString UCameraCollisionHandleHelpers::GetTypeDisplayName(ECollisionHandleType Type)
{
	switch (Type)
	{
		case ECollisionHandleType::Trace:			return TEXT("Trace");
		case ECollisionHandleType::Sweep:			return TEXT("Sweep");
		case ECollisionHandleType::Position:		return TEXT("Position");
		case ECollisionHandleType::FOV:				return TEXT("FOV");
		case ECollisionHandleType::Material:		return TEXT("Material");
		case ECollisionHandleType::Visibility:		return TEXT("Visibility");
		case ECollisionHandleType::Timing:			return TEXT("Timing");
		case ECollisionHandleType::Interpolation:	return TEXT("Interpolation");
		case ECollisionHandleType::Environment:		return TEXT("Environment");
		default:									return TEXT("Unknown");
	}
}
