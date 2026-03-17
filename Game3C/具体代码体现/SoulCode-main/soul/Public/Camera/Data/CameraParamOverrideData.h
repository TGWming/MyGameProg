// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * CameraParamOverrideData.h
 * 
 * Camera Parameter Override Data Structures for Souls-like Camera System
 * 相机参数覆盖数据结构 - 用于魂类游戏相机系统
 * 
 * This file defines the data structures used for CSV-imported parameter overrides.
 * 此文件定义用于CSV导入的参数覆盖数据结构。
 * 
 * Architecture: "Default + Override" Pattern
 * 架构模式："默认值 + 特殊覆盖"
 * - 725 camera states share default parameters (defined elsewhere)
 *   725个相机状态共享默认参数（在别处定义）
 * - Only states requiring special values are defined in the override table
 *   只有需要特殊值的状态才在覆盖表中定义
 * - This file defines the DataTable row structure for the override table
 *   此文件定义覆盖表的DataTable行结构
 * 
 * CSV Format Example / CSV格式示例:
 * StateName,ParamPath,FloatValue,IntValue,BoolValue,VectorValue,StringValue
 * Explore_Sprint,StateBase.Distance.BaseDistance,450.0,,,,
 * Combat_LockOn,StateBase.Flags.bRequiresTarget,,,true,,
 * 
 * @author Camera System Team
 * @version 1.0
 * @date 2024
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/Data/CameraStateEnums.h"
#include "CameraParamOverrideData.generated.h"

// Forward declarations / 前向声明
class UDataTable;

/**
 * Parameter Value Type Enumeration
 * 参数值类型枚举
 * 
 * Defines the type of value stored in a parameter override row.
 * 定义参数覆盖行中存储的值类型。
 * 
 * Used for type-safe parameter value retrieval and validation.
 * 用于类型安全的参数值获取和验证。
 */
UENUM(BlueprintType)
enum class EParamValueType : uint8
{
	/** No value type specified / 未指定值类型 */
	None        UMETA(DisplayName = "None"),
	
	/** Floating point value (float) / 浮点值 */
	Float       UMETA(DisplayName = "Float"),
	
	/** Integer value (int32) / 整数值 */
	Int         UMETA(DisplayName = "Int"),
	
	/** Boolean value (bool) / 布尔值 */
	Bool        UMETA(DisplayName = "Bool"),
	
	/** 3D Vector value (FVector) / 三维向量值 */
	Vector      UMETA(DisplayName = "Vector"),
	
	/** Rotator value (FRotator - Pitch, Yaw, Roll) / 旋转值 */
	Rotator     UMETA(DisplayName = "Rotator"),
	
	/** Name value (FName) / 名称值 */
	Name        UMETA(DisplayName = "Name"),
	
	/** String value (FString) / 字符串值 */
	String      UMETA(DisplayName = "String"),
	
	/** Camera blend type enum value / 相机混合类型枚举值 */
	BlendType   UMETA(DisplayName = "Blend Type")
};

/**
 * Single Parameter Override Row Structure
 * 单个参数覆盖行结构（主要结构）
 * 
 * This is the primary structure for DataTable CSV import.
 * 这是用于DataTable CSV导入的主要结构。
 * 
 * Each row represents a single parameter override for a specific state.
 * 每行代表特定状态的单个参数覆盖。
 * 
 * Design Philosophy / 设计理念:
 * - Flexible: Can override any parameter using "ParamPath + Value"
 *   灵活：使用"参数路径 + 值"可覆盖任意参数
 * - Sparse: Only stores overrides, not all 154 parameters
 *   稀疏：只存储覆盖值，不存储全部154个参数
 * - Type-safe: ValueType indicates which value field is active
 *   类型安全：ValueType指示哪个值字段有效
 * 
 * Valid Parameter Paths (Examples) / 有效参数路径（示例）:
 * - StateBase.Distance.BaseDistance
 * - StateBase.FOV.BaseFOV
 * - StateBase.Flags.bRequiresTarget
 * - Module.Position.PredictionTime
 * - Modifier.Shake.LightHitAmplitude
 */
USTRUCT(BlueprintType)
struct SOUL_API FCameraParamOverrideRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	//========================================
	// Identification / 标识信息
	//========================================
	
	/**
	 * State Name - Corresponds to one of the 725 camera states
	 * 状态名称 - 对应725个相机状态之一
	 * 
	 * Examples: "Explore_Default", "Combat_LockOn", "Boss_Phase1"
	 * 示例："Explore_Default", "Combat_LockOn", "Boss_Phase1"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Override",
		meta = (DisplayName = "State Name", ToolTip = "The name of the camera state to override"))
	FName StateName;

	/**
	 * Parameter Path - Dot-separated path to the parameter
	 * 参数路径 - 点分隔的参数路径
	 * 
	 * Format: "Category.Group.Parameter"
	 * 格式："类别.组.参数"
	 * 
	 * Examples / 示例:
	 * - "StateBase.Distance.BaseDistance"
	 * - "StateBase.FOV.BaseFOV"
	 * - "Module.Rotation.InputSensitivity"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Override",
		meta = (DisplayName = "Parameter Path", ToolTip = "Dot-separated path to the parameter (e.g., StateBase.Distance.BaseDistance)"))
	FString ParamPath;

	/**
	 * Value Type - Indicates which value field contains the override
	 * 值类型 - 指示哪个值字段包含覆盖值
	 * 
	 * Must be set correctly for proper value retrieval.
	 * 必须正确设置以便正确获取值。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Override",
		meta = (DisplayName = "Value Type", ToolTip = "The type of value being overridden"))
	EParamValueType ValueType;

	//========================================
	// Value Storage / 值存储
	//========================================

	/**
	 * Float Value - For float parameters
	 * 浮点值 - 用于浮点参数
	 * 
	 * Used for: Distance, FOV, Speed, Time, etc.
	 * 用于：距离、FOV、速度、时间等
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "Float Value", EditCondition = "ValueType == EParamValueType::Float"))
	float FloatValue;

	/**
	 * Integer Value - For integer parameters
	 * 整数值 - 用于整数参数
	 * 
	 * Used for: Priority, Count, Index, etc.
	 * 用于：优先级、计数、索引等
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "Int Value", EditCondition = "ValueType == EParamValueType::Int"))
	int32 IntValue;

	/**
	 * Boolean Value - For boolean parameters
	 * 布尔值 - 用于布尔参数
	 * 
	 * Used for: Flags like bRequiresTarget, bIsCinematic, bEnableCollision
	 * 用于：标志位如 bRequiresTarget, bIsCinematic, bEnableCollision
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "Bool Value", EditCondition = "ValueType == EParamValueType::Bool"))
	bool BoolValue;

	/**
	 * Vector Value - For FVector parameters
	 * 向量值 - 用于FVector参数
	 * 
	 * Used for: Offsets (FocusOffset, SocketOffset, ShoulderOffset)
	 * 用于：偏移量（FocusOffset, SocketOffset, ShoulderOffset）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "Vector Value", EditCondition = "ValueType == EParamValueType::Vector"))
	FVector VectorValue;

	/**
	 * Rotator Value - For FRotator parameters
	 * 旋转值 - 用于FRotator参数
	 * 
	 * Used for: Initial rotations, rotation offsets
	 * 用于：初始旋转、旋转偏移
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "Rotator Value", EditCondition = "ValueType == EParamValueType::Rotator"))
	FRotator RotatorValue;

	/**
	 * Name Value - For FName parameters
	 * 名称值 - 用于FName参数
	 * 
	 * Used for: References to other states, socket names
	 * 用于：其他状态的引用、插槽名称
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "Name Value", EditCondition = "ValueType == EParamValueType::Name"))
	FName NameValue;

	/**
	 * String Value - For FString parameters
	 * 字符串值 - 用于FString参数
	 * 
	 * Used for: Descriptions, custom data
	 * 用于：描述、自定义数据
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "String Value", EditCondition = "ValueType == EParamValueType::String"))
	FString StringValue;

	/**
	 * Blend Type Value - For ECameraBlendType parameters
	 * 混合类型值 - 用于ECameraBlendType参数
	 * 
	 * Used for: BlendType in transition settings
	 * 用于：过渡设置中的BlendType
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value",
		meta = (DisplayName = "Blend Type Value", EditCondition = "ValueType == EParamValueType::BlendType"))
	ECameraBlendType BlendTypeValue;

	/** Chinese name for human-readable identification in DataTable / 中文名称，用于配置表可读标识 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Override",
		meta = (DisplayName = "Chinese Name"))
	FString ChineseName;

public:
	/**
	 * Default Constructor - Initializes all values to defaults
	 * 默认构造函数 - 初始化所有默认值
	 */
	FCameraParamOverrideRow();

	/**
	 * Check if this override row is valid
	 * 检查此覆盖行是否有效
	 * 
	 * @return true if StateName and ParamPath are set and ValueType is not None
	 *         如果StateName和ParamPath已设置且ValueType不为None则返回true
	 */
	bool IsValid() const;

	/**
	 * Get a debug string representation of this override
	 * 获取此覆盖的调试字符串表示
	 * 
	 * @return Formatted string showing state, path, type and value
	 *         显示状态、路径、类型和值的格式化字符串
	 */
	FString ToString() const;
};

/**
 * State-Level Override Collection Structure
 * 状态级覆盖汇总结构（运行时使用）
 * 
 * Groups all parameter overrides for a single camera state.
 * 将单个相机状态的所有参数覆盖分组在一起。
 * 
 * This structure is used at runtime for efficient override lookup.
 * 此结构在运行时用于高效的覆盖查找。
 * 
 * Usage / 用法:
 * 1. Load all FCameraParamOverrideRow from DataTable
 *    从DataTable加载所有FCameraParamOverrideRow
 * 2. Group by StateName into FStateParamOverrides
 *    按StateName分组为FStateParamOverrides
 * 3. Query overrides by ParamPath at runtime
 *    运行时按ParamPath查询覆盖
 */
USTRUCT(BlueprintType)
struct SOUL_API FStateParamOverrides
{
	GENERATED_BODY()

public:
	/**
	 * State Name - The camera state these overrides apply to
	 * 状态名称 - 这些覆盖适用的相机状态
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Override",
		meta = (DisplayName = "State Name"))
	FName StateName;

	/**
	 * Overrides - All parameter overrides for this state
	 * 覆盖列表 - 该状态的所有参数覆盖
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Override",
		meta = (DisplayName = "Overrides"))
	TArray<FCameraParamOverrideRow> Overrides;

public:
	/**
	 * Default Constructor
	 * 默认构造函数
	 */
	FStateParamOverrides();

	/**
	 * Constructor with state name
	 * 带状态名称的构造函数
	 * 
	 * @param InStateName The name of the camera state
	 *                    相机状态的名称
	 */
	explicit FStateParamOverrides(FName InStateName);

	/**
	 * Find Override by Parameter Path
	 * 根据参数路径查找覆盖值
	 * 
	 * @param ParamPath The dot-separated parameter path to search for
	 *                  要搜索的点分隔参数路径
	 * @return Pointer to the override row if found, nullptr otherwise
	 *         如果找到则返回覆盖行的指针，否则返回nullptr
	 */
	const FCameraParamOverrideRow* FindOverride(const FString& ParamPath) const;

	/**
	 * Check if Override Exists
	 * 检查是否有指定参数的覆盖
	 * 
	 * @param ParamPath The dot-separated parameter path to check
	 *                  要检查的点分隔参数路径
	 * @return true if an override exists for this path
	 *         如果存在此路径的覆盖则返回true
	 */
	bool HasOverride(const FString& ParamPath) const;

	/**
	 * Add Override
	 * 添加覆盖
	 * 
	 * @param Override The override row to add
	 *                 要添加的覆盖行
	 */
	void AddOverride(const FCameraParamOverrideRow& Override);

	/**
	 * Get Override Count
	 * 获取覆盖数量
	 * 
	 * @return Number of overrides for this state
	 *         该状态的覆盖数量
	 */
	int32 GetOverrideCount() const;

	/**
	 * Clear all overrides
	 * 清除所有覆盖
	 */
	void ClearOverrides();
};

/**
 * Camera Parameter Override Helper Function Library
 * 相机参数覆盖辅助函数库
 * 
 * Provides utility functions for loading, parsing and querying parameter overrides.
 * 提供用于加载、解析和查询参数覆盖的实用函数。
 * 
 * All functions are BlueprintCallable for easy integration with Blueprints.
 * 所有函数都可从蓝图调用，便于与蓝图集成。
 */
UCLASS()
class SOUL_API UCameraParamOverrideHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//========================================
	// DataTable Loading / 数据表加载
	//========================================

	/**
	 * Load All Overrides from DataTable
	 * 从DataTable加载所有覆盖，按状态名分组
	 * 
	 * Reads all rows from the DataTable and groups them by StateName.
	 * 从DataTable读取所有行并按StateName分组。
	 * 
	 * @param DataTable The DataTable containing FCameraParamOverrideRow data
	 *                  包含FCameraParamOverrideRow数据的DataTable
	 * @return Map of StateName to FStateParamOverrides
	 *         StateName到FStateParamOverrides的映射
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Override",
		meta = (DisplayName = "Load Overrides From DataTable"))
	static TMap<FName, FStateParamOverrides> LoadOverridesFromDataTable(UDataTable* DataTable);

	/**
	 * Get Overrides for Specific State
	 * 获取指定状态的所有覆盖
	 * 
	 * @param AllOverrides The map of all overrides (from LoadOverridesFromDataTable)
	 *                     所有覆盖的映射（来自LoadOverridesFromDataTable）
	 * @param StateName The state name to get overrides for
	 *                  要获取覆盖的状态名称
	 * @return The overrides for the specified state (empty if not found)
	 *         指定状态的覆盖（如果未找到则为空）
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Overrides For State"))
	static FStateParamOverrides GetOverridesForState(
		const TMap<FName, FStateParamOverrides>& AllOverrides, 
		FName StateName);

	//========================================
	// Parameter Path Utilities / 参数路径工具
	//========================================

	/**
	 * Validate Parameter Path Format
	 * 检查参数路径格式是否有效
	 * 
	 * A valid path has at least 2 parts separated by dots.
	 * 有效路径至少有2个由点分隔的部分。
	 * 
	 * @param ParamPath The parameter path to validate
	 *                  要验证的参数路径
	 * @return true if the path format is valid
	 *         如果路径格式有效则返回true
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Is Valid Param Path"))
	static bool IsValidParamPath(const FString& ParamPath);

	/**
	 * Parse Parameter Path into Components
	 * 解析参数路径为组件
	 * 
	 * Splits the path by dots into individual parts.
	 * 按点将路径拆分为各个部分。
	 * 
	 * Example / 示例:
	 * "StateBase.Distance.BaseDistance" -> ["StateBase", "Distance", "BaseDistance"]
	 * 
	 * @param ParamPath The parameter path to parse
	 *                  要解析的参数路径
	 * @return Array of path components
	 *         路径组件数组
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Parse Param Path"))
	static TArray<FString> ParseParamPath(const FString& ParamPath);

	/**
	 * Build Parameter Path from Components
	 * 从组件构建参数路径
	 * 
	 * Joins components with dots.
	 * 用点连接组件。
	 * 
	 * @param Components Array of path components
	 *                   路径组件数组
	 * @return The combined parameter path
	 *         组合的参数路径
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Build Param Path"))
	static FString BuildParamPath(const TArray<FString>& Components);

	/**
	 * Get Category from Parameter Path
	 * 从参数路径获取类别
	 * 
	 * Returns the first component of the path.
	 * 返回路径的第一个组件。
	 * 
	 * @param ParamPath The parameter path
	 *                  参数路径
	 * @return The category (first component) or empty string if invalid
	 *         类别（第一个组件）或无效时返回空字符串
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Category From Path"))
	static FString GetCategoryFromPath(const FString& ParamPath);

	/**
	 * Get Parameter Name from Path
	 * 从路径获取参数名称
	 * 
	 * Returns the last component of the path.
	 * 返回路径的最后一个组件。
	 * 
	 * @param ParamPath The parameter path
	 *                  参数路径
	 * @return The parameter name (last component) or empty string if invalid
	 *         参数名称（最后一个组件）或无效时返回空字符串
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Param Name From Path"))
	static FString GetParamNameFromPath(const FString& ParamPath);

	//========================================
	// Value Retrieval Helpers / 值获取辅助函数
	//========================================

	/**
	 * Get Float Value from Override
	 * 从覆盖获取浮点值
	 * 
	 * @param Override The override row
	 *                 覆盖行
	 * @param OutValue Output float value
	 *                 输出浮点值
	 * @return true if the override contains a valid float value
	 *         如果覆盖包含有效的浮点值则返回true
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Float Value From Override"))
	static bool GetFloatValueFromOverride(const FCameraParamOverrideRow& Override, float& OutValue);

	/**
	 * Get Int Value from Override
	 * 从覆盖获取整数值
	 * 
	 * @param Override The override row
	 *                 覆盖行
	 * @param OutValue Output int value
	 *                 输出整数值
	 * @return true if the override contains a valid int value
	 *         如果覆盖包含有效的整数值则返回true
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Int Value From Override"))
	static bool GetIntValueFromOverride(const FCameraParamOverrideRow& Override, int32& OutValue);

	/**
	 * Get Bool Value from Override
	 * 从覆盖获取布尔值
	 * 
	 * @param Override The override row
	 *                 覆盖行
	 * @param OutValue Output bool value
	 *                 输出布尔值
	 * @return true if the override contains a valid bool value
	 *         如果覆盖包含有效的布尔值则返回true
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Bool Value From Override"))
	static bool GetBoolValueFromOverride(const FCameraParamOverrideRow& Override, bool& OutValue);

	/**
	 * Get Vector Value from Override
	 * 从覆盖获取向量值
	 * 
	 * @param Override The override row
	 *                 覆盖行
	 * @param OutValue Output vector value
	 *                 输出向量值
	 * @return true if the override contains a valid vector value
	 *         如果覆盖包含有效的向量值则返回true
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Vector Value From Override"))
	static bool GetVectorValueFromOverride(const FCameraParamOverrideRow& Override, FVector& OutValue);

	/**
	 * Get Blend Type Value from Override
	 * 从覆盖获取混合类型值
	 * 
	 * @param Override The override row
	 *                 覆盖行
	 * @param OutValue Output blend type value
	 *                 输出混合类型值
	 * @return true if the override contains a valid blend type value
	 *         如果覆盖包含有效的混合类型值则返回true
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override",
		meta = (DisplayName = "Get Blend Type Value From Override"))
	static bool GetBlendTypeValueFromOverride(const FCameraParamOverrideRow& Override, ECameraBlendType& OutValue);

	//========================================
	// Debugging / 调试
	//========================================

	/**
	 * Log All Overrides
	 * 记录所有覆盖到日志
	 * 
	 * Outputs all overrides to the log for debugging.
	 * 将所有覆盖输出到日志用于调试。
	 * 
	 * @param AllOverrides The map of all overrides
	 *                     所有覆盖的映射
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Override|Debug",
		meta = (DisplayName = "Log All Overrides", DevelopmentOnly))
	static void LogAllOverrides(const TMap<FName, FStateParamOverrides>& AllOverrides);

	/**
	 * Get Override Statistics
	 * 获取覆盖统计信息
	 * 
	 * @param AllOverrides The map of all overrides
	 *                     所有覆盖的映射
	 * @param OutTotalStates Number of states with overrides
	 *                       有覆盖的状态数量
	 * @param OutTotalOverrides Total number of override entries
	 *                          覆盖条目总数
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Override|Debug",
		meta = (DisplayName = "Get Override Statistics"))
	static void GetOverrideStatistics(
		const TMap<FName, FStateParamOverrides>& AllOverrides,
		int32& OutTotalStates,
		int32& OutTotalOverrides);
};
