// Copyright Epic Games, Inc. All Rights Reserved.
// 
// CameraStateData.h
// 相机状态数据结构定义
// 
// This file defines the row structure for importing camera state definitions from CSV.
// 此文件定义用于从CSV导入相机状态定义的行结构。
// 
// Maps to: camera_states_full_725.csv (725 camera state definitions)
// 对应CSV: camera_states_full_725.csv（725个相机状态定义）

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/Data/CameraStateEnums.h"
#include "CameraStateData.generated.h"

/**
 * FCameraStateRow - Row structure for camera state definitions
 * 相机状态定义的行结构
 * 
 * Maps to: camera_states_full_725.csv
 * 对应CSV: camera_states_full_725.csv
 * 
 * This struct defines the metadata for each camera state.
 * The actual parameter values come from UDefaultCameraParams + overrides.
 * 此结构体定义每个相机状态的元数据。
 * 实际参数值来自 UDefaultCameraParams + 覆盖表。
 * 
 * CSV Columns:
 * | Column      | Type                  | Description                              |
 * |-------------|-----------------------|------------------------------------------|
 * | ID          | FName (RowName)       | Row ID (e.g., CameraStatesFull_1)        |
 * | Category    | ECameraStateCategory  | Main category (e.g., FreeExploration)    |
 * | SubCategory | FName                 | Sub-category (e.g., BasicMovement)       |
 * | StateName   | FName                 | Unique state identifier                  |
 * | ChineseName | FString               | Chinese display name                     |
 * | Description | FString               | Description of the state                 |
 * | Priority    | int32                 | Priority (higher = more important)       |
 * | Reference   | FName                 | Reference game(s)                        |
 */
USTRUCT(BlueprintType)
struct SOUL_API FCameraStateRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    /** Default constructor / 默认构造函数 */
    FCameraStateRow();

    //========================================
    // Identity / 身份标识
    //========================================

    /**
     * Main category of the state
     * 状态的主分类
     * 
     * Maps to CSV column: Category
     * 对应CSV列: Category
     * 
     * Examples: FreeExploration, Combat, Boss, Environment, etc.
     * 示例: FreeExploration（自由探索）, Combat（战斗）, Boss, Environment（环境）等
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECameraStateCategory Category;

    /**
     * Sub-category for finer classification
     * 子分类，用于更细的分类
     * 
     * Maps to CSV column: SubCategory
     * 对应CSV列: SubCategory
     * 
     * Examples: BasicMovement, LockOn, Attack, Dodge, etc.
     * 示例: BasicMovement（基础移动）, LockOn（锁定）, Attack（攻击）, Dodge（闪避）等
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FName SubCategory;

    /**
     * Unique state name identifier
     * 状态唯一标识名
     * 
     * Maps to CSV column: StateName
     * 对应CSV列: StateName
     * 
     * This is the primary key used to reference the state.
     * 这是用于引用状态的主键。
     * 
     * Examples: Explore_Default, Combat_LockOn, Boss_Fight_Phase1
     * 示例: Explore_Default（默认探索）, Combat_LockOn（战斗锁定）, Boss_Fight_Phase1（Boss战第一阶段）
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FName StateName;

    /**
     * Chinese display name
     * 中文显示名称
     * 
     * Maps to CSV column: ChineseName
     * 对应CSV列: ChineseName
     * 
     * Used for localization and editor display in Chinese.
     * 用于本地化和编辑器中的中文显示。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString ChineseName;

    /**
     * Description of the state's purpose and behavior
     * 状态用途和行为的描述
     * 
     * Maps to CSV column: Description
     * 对应CSV列: Description
     * 
     * Detailed explanation of when and how this state should be used.
     * 详细说明此状态应何时以及如何使用。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (MultiLine = true))
    FString Description;

    //========================================
    // Priority / 优先级
    //========================================

    /**
     * State priority for conflict resolution
     * 状态优先级，用于冲突解决
     * 
     * Maps to CSV column: Priority
     * 对应CSV列: Priority
     * 
     * Higher values = higher priority.
     * When multiple states could be active, highest priority wins.
     * 数值越大优先级越高。
     * 当多个状态可能同时激活时，取最高优先级的状态。
     * 
     * Range: 0-255 (typically 1-100 used)
     * 范围: 0-255（通常使用1-100）
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority", meta = (ClampMin = "0", ClampMax = "255"))
    int32 Priority;

    //========================================
    // Reference / 参考
    //========================================

    /**
     * Reference game(s) this state is inspired by
     * 此状态参考的游戏
     * 
     * Maps to CSV column: Reference
     * 对应CSV列: Reference
     * 
     * Examples: All, EldenRing, Bloodborne, DarkSouls3, Sekiro, LiesOfP
     * 示例: All（全部）, EldenRing（艾尔登法环）, Bloodborne（血源诅咒）, 
     *       DarkSouls3（黑暗之魂3）, Sekiro（只狼）, LiesOfP（匹诺曹的谎言）
     * 
     * Used for documentation and filtering purposes.
     * 用于文档和过滤目的。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reference")
    FName Reference;

    //========================================
    // Helper Methods / 辅助方法
    //========================================

    /** 
     * Check if this is a valid state row
     * 检查此状态行是否有效
     * 
     * @return true if StateName is valid and Category is not None
     * @return 如果StateName有效且Category不为None则返回true
     */
    bool IsValid() const;

    /** 
     * Get a display string for debugging
     * 获取用于调试的显示字符串
     * 
     * @return Formatted string with key information
     * @return 包含关键信息的格式化字符串
     */
    FString GetDebugString() const;

    /**
     * Get the category as a string
     * 获取分类的字符串表示
     * 
     * @return Category name as FString
     * @return 分类名称的FString形式
     */
    FString GetCategoryString() const;
};
