// Copyright Epic Games, Inc. All Rights Reserved.
// 
// CameraStateData.cpp
// 相机状态数据结构实现
// 
// Implementation of FCameraStateRow.
// FCameraStateRow 的实现。

#include "Camera/Data/CameraStateData.h"
#include "Camera/Data/CameraStateHelpers.h"
#include "Engine/DataTable.h"

//========================================
// FCameraStateRow Implementation
// FCameraStateRow 实现
//========================================

FCameraStateRow::FCameraStateRow()
    : Category(ECameraStateCategory::None)
    , SubCategory(NAME_None)
    , StateName(NAME_None)
    , ChineseName(TEXT(""))
    , Description(TEXT(""))
    , Priority(1)
    , Reference(NAME_None)
{
}

bool FCameraStateRow::IsValid() const
{
    // A valid row must have a non-empty StateName and a valid Category
    // 有效的行必须有非空的StateName和有效的Category
    return !StateName.IsNone() && Category != ECameraStateCategory::None;
}

FString FCameraStateRow::GetDebugString() const
{
    return FString::Printf(TEXT("[%s] %s (%s/%s) - Priority: %d, Ref: %s"),
        *GetCategoryString(),
        *StateName.ToString(),
        *ChineseName,
        *Description.Left(50),
        Priority,
        *Reference.ToString());
}

FString FCameraStateRow::GetCategoryString() const
{
    return UCameraStateHelpers::CategoryToString(Category);
}
