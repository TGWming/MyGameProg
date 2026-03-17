// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * CameraParamOverrideData.cpp
 * 
 * Implementation of Camera Parameter Override Data Structures
 * 相机参数覆盖数据结构的实现
 */

#include "Camera/Data/CameraParamOverrideData.h"
#include "Engine/DataTable.h"

//========================================
// FCameraParamOverrideRow Implementation
// FCameraParamOverrideRow 实现
//========================================

FCameraParamOverrideRow::FCameraParamOverrideRow()
	: StateName(NAME_None)
	, ParamPath(TEXT(""))
	, ValueType(EParamValueType::None)
	, FloatValue(0.0f)
	, IntValue(0)
	, BoolValue(false)
	, VectorValue(FVector::ZeroVector)
	, RotatorValue(FRotator::ZeroRotator)
	, NameValue(NAME_None)
	, StringValue(TEXT(""))
	, BlendTypeValue(ECameraBlendType::Linear)
{
}

bool FCameraParamOverrideRow::IsValid() const
{
	return !StateName.IsNone() 
		&& !ParamPath.IsEmpty() 
		&& ValueType != EParamValueType::None;
}

FString FCameraParamOverrideRow::ToString() const
{
	FString ValueStr;
	
	switch (ValueType)
	{
	case EParamValueType::Float:
		ValueStr = FString::Printf(TEXT("%f"), FloatValue);
		break;
	case EParamValueType::Int:
		ValueStr = FString::Printf(TEXT("%d"), IntValue);
		break;
	case EParamValueType::Bool:
		ValueStr = BoolValue ? TEXT("true") : TEXT("false");
		break;
	case EParamValueType::Vector:
		ValueStr = VectorValue.ToString();
		break;
	case EParamValueType::Rotator:
		ValueStr = RotatorValue.ToString();
		break;
	case EParamValueType::Name:
		ValueStr = NameValue.ToString();
		break;
	case EParamValueType::String:
		ValueStr = StringValue;
		break;
	case EParamValueType::BlendType:
		ValueStr = FString::Printf(TEXT("%d"), static_cast<int32>(BlendTypeValue));
		break;
	default:
		ValueStr = TEXT("None");
		break;
	}
	
	return FString::Printf(TEXT("[%s] %s = %s"), 
		*StateName.ToString(), 
		*ParamPath, 
		*ValueStr);
}

//========================================
// FStateParamOverrides Implementation
// FStateParamOverrides 实现
//========================================

FStateParamOverrides::FStateParamOverrides()
	: StateName(NAME_None)
{
}

FStateParamOverrides::FStateParamOverrides(FName InStateName)
	: StateName(InStateName)
{
}

const FCameraParamOverrideRow* FStateParamOverrides::FindOverride(const FString& ParamPath) const
{
	for (const FCameraParamOverrideRow& Override : Overrides)
	{
		if (Override.ParamPath.Equals(ParamPath, ESearchCase::IgnoreCase))
		{
			return &Override;
		}
	}
	return nullptr;
}

bool FStateParamOverrides::HasOverride(const FString& ParamPath) const
{
	return FindOverride(ParamPath) != nullptr;
}

void FStateParamOverrides::AddOverride(const FCameraParamOverrideRow& Override)
{
	Overrides.Add(Override);
}

int32 FStateParamOverrides::GetOverrideCount() const
{
	return Overrides.Num();
}

void FStateParamOverrides::ClearOverrides()
{
	Overrides.Empty();
}

//========================================
// UCameraParamOverrideHelpers Implementation
// UCameraParamOverrideHelpers 实现
//========================================

TMap<FName, FStateParamOverrides> UCameraParamOverrideHelpers::LoadOverridesFromDataTable(UDataTable* DataTable)
{
	TMap<FName, FStateParamOverrides> Result;
	
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCameraParamOverrideHelpers::LoadOverridesFromDataTable - DataTable is null"));
		return Result;
	}
	
	// Verify the row struct type
	// 验证行结构类型
	if (DataTable->GetRowStruct() != FCameraParamOverrideRow::StaticStruct())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCameraParamOverrideHelpers::LoadOverridesFromDataTable - DataTable row struct is not FCameraParamOverrideRow"));
		return Result;
	}
	
	// Get all row names
	// 获取所有行名称
	TArray<FName> RowNames = DataTable->GetRowNames();
	
	for (const FName& RowName : RowNames)
	{
		const FCameraParamOverrideRow* Row = DataTable->FindRow<FCameraParamOverrideRow>(RowName, TEXT(""));
		
		if (Row && Row->IsValid())
		{
			// Find or create the state override collection
			// 查找或创建状态覆盖集合
			FStateParamOverrides* StateOverrides = Result.Find(Row->StateName);
			
			if (!StateOverrides)
			{
				Result.Add(Row->StateName, FStateParamOverrides(Row->StateName));
				StateOverrides = Result.Find(Row->StateName);
			}
			
			if (StateOverrides)
			{
				StateOverrides->AddOverride(*Row);
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("UCameraParamOverrideHelpers::LoadOverridesFromDataTable - Loaded %d states with overrides"), Result.Num());
	
	return Result;
}

FStateParamOverrides UCameraParamOverrideHelpers::GetOverridesForState(
	const TMap<FName, FStateParamOverrides>& AllOverrides, 
	FName StateName)
{
	const FStateParamOverrides* Found = AllOverrides.Find(StateName);
	
	if (Found)
	{
		return *Found;
	}
	
	// Return empty overrides if not found
	// 如果未找到则返回空覆盖
	return FStateParamOverrides(StateName);
}

bool UCameraParamOverrideHelpers::IsValidParamPath(const FString& ParamPath)
{
	if (ParamPath.IsEmpty())
	{
		return false;
	}
	
	// Must contain at least one dot (Category.Parameter minimum)
	// 必须包含至少一个点（最小为 类别.参数）
	int32 DotCount = 0;
	for (TCHAR Char : ParamPath)
	{
		if (Char == TEXT('.'))
		{
			DotCount++;
		}
	}
	
	// At least one dot means at least 2 parts
	// 至少一个点意味着至少2个部分
	return DotCount >= 1;
}

TArray<FString> UCameraParamOverrideHelpers::ParseParamPath(const FString& ParamPath)
{
	TArray<FString> Components;
	
	if (!ParamPath.IsEmpty())
	{
		ParamPath.ParseIntoArray(Components, TEXT("."), true);
	}
	
	return Components;
}

FString UCameraParamOverrideHelpers::BuildParamPath(const TArray<FString>& Components)
{
	if (Components.Num() == 0)
	{
		return FString();
	}
	
	return FString::Join(Components, TEXT("."));
}

FString UCameraParamOverrideHelpers::GetCategoryFromPath(const FString& ParamPath)
{
	TArray<FString> Components = ParseParamPath(ParamPath);
	
	if (Components.Num() > 0)
	{
		return Components[0];
	}
	
	return FString();
}

FString UCameraParamOverrideHelpers::GetParamNameFromPath(const FString& ParamPath)
{
	TArray<FString> Components = ParseParamPath(ParamPath);
	
	if (Components.Num() > 0)
	{
		return Components.Last();
	}
	
	return FString();
}

bool UCameraParamOverrideHelpers::GetFloatValueFromOverride(const FCameraParamOverrideRow& Override, float& OutValue)
{
	if (Override.ValueType == EParamValueType::Float)
	{
		OutValue = Override.FloatValue;
		return true;
	}
	return false;
}

bool UCameraParamOverrideHelpers::GetIntValueFromOverride(const FCameraParamOverrideRow& Override, int32& OutValue)
{
	if (Override.ValueType == EParamValueType::Int)
	{
		OutValue = Override.IntValue;
		return true;
	}
	return false;
}

bool UCameraParamOverrideHelpers::GetBoolValueFromOverride(const FCameraParamOverrideRow& Override, bool& OutValue)
{
	if (Override.ValueType == EParamValueType::Bool)
	{
		OutValue = Override.BoolValue;
		return true;
	}
	return false;
}

bool UCameraParamOverrideHelpers::GetVectorValueFromOverride(const FCameraParamOverrideRow& Override, FVector& OutValue)
{
	if (Override.ValueType == EParamValueType::Vector)
	{
		OutValue = Override.VectorValue;
		return true;
	}
	return false;
}

bool UCameraParamOverrideHelpers::GetBlendTypeValueFromOverride(const FCameraParamOverrideRow& Override, ECameraBlendType& OutValue)
{
	if (Override.ValueType == EParamValueType::BlendType)
	{
		OutValue = Override.BlendTypeValue;
		return true;
	}
	return false;
}

void UCameraParamOverrideHelpers::LogAllOverrides(const TMap<FName, FStateParamOverrides>& AllOverrides)
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Camera Parameter Overrides Debug Log"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	
	int32 TotalOverrides = 0;
	
	for (const auto& Pair : AllOverrides)
	{
		const FName& StateName = Pair.Key;
		const FStateParamOverrides& StateOverrides = Pair.Value;
		
		UE_LOG(LogTemp, Log, TEXT("State: %s (%d overrides)"), 
			*StateName.ToString(), 
			StateOverrides.GetOverrideCount());
		
		for (const FCameraParamOverrideRow& Override : StateOverrides.Overrides)
		{
			UE_LOG(LogTemp, Log, TEXT("  - %s"), *Override.ToString());
		}
		
		TotalOverrides += StateOverrides.GetOverrideCount();
	}
	
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Total: %d states, %d overrides"), AllOverrides.Num(), TotalOverrides);
	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

void UCameraParamOverrideHelpers::GetOverrideStatistics(
	const TMap<FName, FStateParamOverrides>& AllOverrides,
	int32& OutTotalStates,
	int32& OutTotalOverrides)
{
	OutTotalStates = AllOverrides.Num();
	OutTotalOverrides = 0;
	
	for (const auto& Pair : AllOverrides)
	{
		OutTotalOverrides += Pair.Value.GetOverrideCount();
	}
}
