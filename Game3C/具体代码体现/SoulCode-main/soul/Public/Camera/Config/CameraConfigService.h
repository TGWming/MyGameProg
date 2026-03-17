// Copyright (c) 2024. All Rights Reserved.

/**
 * CameraConfigService.h
 * 
 * Central configuration service for Souls-like camera system.
 * 魂类游戏相机系统的中央配置服务。
 * 
 * This service implements the "Default + Override" architecture:
 * 该服务实现"默认值 + 覆盖"架构：
 * - Loads state definitions (725 states metadata)
 *   加载状态定义（725个状态的元数据）
 * - Loads parameter overrides from DataTable
 *   从DataTable加载参数覆盖
 * - Combines them into complete FCameraStateConfig
 *   将它们组合成完整的FCameraStateConfig
 * - Caches frequently accessed configurations for performance
 *   缓存频繁访问的配置以提高性能
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraConfig.h"
#include "Camera/Data/CameraParamOverrideData.h"
#include "CameraConfigService.generated.h"

// Forward declarations / 前向声明
class UDataTable;
class UDefaultCameraParams;

/**
 * UCameraConfigService - Central configuration service for camera system
 * 相机配置服务 - 相机系统的中央配置服务
 * 
 * Responsibilities / 职责:
 * - Load state definitions from DataTable (725 states)
 *   从DataTable加载状态定义（725个状态）
 * - Load parameter overrides from DataTable
 *   从DataTable加载参数覆盖
 * - Combine default params + overrides into complete FCameraStateConfig
 *   将默认参数和覆盖组合成完整的FCameraStateConfig
 * - Cache frequently accessed configurations
 *   缓存频繁访问的配置
 * 
 * Architecture / 架构:
 * StateDefinitions (725 rows) + DefaultParams + Overrides = Complete Config
 * 状态定义（725行）+ 默认参数 + 覆盖 = 完整配置
 * 
 * Usage Example / 使用示例:
 * @code
 * UCameraConfigService* ConfigService = NewObject<UCameraConfigService>();
 * ConfigService->Initialize(StatesTable, OverridesTable);
 * 
 * FCameraStateConfig Config;
 * if (ConfigService->GetStateConfig(TEXT("Combat_Locked"), Config))
 * {
 *     // Use configuration...
 * }
 * @endcode
 */
UCLASS(BlueprintType)
class SOUL_API UCameraConfigService : public UObject
{
    GENERATED_BODY()

public:
    /** Constructor / 构造函数 */
    UCameraConfigService();

    //========================================
    // Initialization / 初始化
    //========================================

    /**
     * Initialize the config service with DataTable references
     * 使用DataTable引用初始化配置服务
     * 
     * @param InStatesDataTable DataTable containing state definitions (FCameraStateRow)
     *                          包含状态定义的DataTable（FCameraStateRow）
     * @param InOverridesDataTable DataTable containing parameter overrides (FCameraParamOverrideRow)
     *                             包含参数覆盖的DataTable（FCameraParamOverrideRow）
     * @return True if initialization successful / 初始化成功返回true
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Config")
    bool Initialize(UDataTable* InStatesDataTable, UDataTable* InOverridesDataTable);

    /**
     * Check if service is initialized
     * 检查服务是否已初始化
     * 
     * @return True if initialized and ready to use / 已初始化并可使用返回true
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    bool IsInitialized() const;

    /**
     * Reload all data from DataTables (hot reload support)
     * 从DataTable重新加载所有数据（支持热重载）
     * 
     * Useful for runtime updates during development.
     * 在开发过程中进行运行时更新时很有用。
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Config")
    void ReloadData();

    //========================================
    // Configuration Query / 配置查询
    //========================================

    /**
     * Get complete camera state configuration by state name
     * 根据状态名称获取完整的相机状态配置
     * 
     * This is the main query method. It:
     * 这是主要的查询方法。它：
     * 1. Gets default params for the state's category
     *    获取状态类别的默认参数
     * 2. Applies any overrides for this specific state
     *    应用此特定状态的任何覆盖
     * 3. Returns the combined configuration
     *    返回组合后的配置
     * 
     * @param StateName Name of the camera state / 相机状态名称
     * @param OutConfig Output configuration struct / 输出配置结构体
     * @return True if state was found and config is valid / 找到状态且配置有效返回true
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Config")
    bool GetStateConfig(FName StateName, FCameraStateConfig& OutConfig) const;

    /**
     * Get state configuration with caching
     * 获取状态配置（带缓存）
     * 
     * More efficient for repeated access to the same state.
     * 对于重复访问同一状态更高效。
     * 
     * @param StateName Name of the camera state / 相机状态名称
     * @return Pointer to cached config, or nullptr if not found
     *         指向缓存配置的指针，未找到返回nullptr
     */
    const FCameraStateConfig* GetCachedStateConfig(FName StateName);

    /**
     * Check if a state exists in the loaded definitions
     * 检查状态是否存在于已加载的定义中
     * 
     * @param StateName Name of the camera state / 相机状态名称
     * @return True if state exists / 状态存在返回true
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    bool DoesStateExist(FName StateName) const;

    /**
     * Get all state names loaded from DataTable
     * 获取从DataTable加载的所有状态名称
     * 
     * @return Array of all state names / 所有状态名称的数组
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    TArray<FName> GetAllStateNames() const;

    /**
     * Get state names in a specific category
     * 获取特定类别中的状态名称
     * 
     * @param Category Category to filter by / 要筛选的类别
     * @return Array of state names in the category / 该类别中的状态名称数组
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    TArray<FName> GetStateNamesInCategory(ECameraStateCategory Category) const;

    //========================================
    // State Metadata Query / 状态元数据查询
    //========================================

    /**
     * Get state category by name
     * 根据名称获取状态类别
     * 
     * @param StateName Name of the camera state / 相机状态名称
     * @return Category of the state, or default if not found
     *         状态的类别，未找到返回默认值
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    ECameraStateCategory GetStateCategory(FName StateName) const;

    /**
     * Get state priority by name
     * 根据名称获取状态优先级
     * 
     * Higher priority states take precedence in state transitions.
     * 优先级更高的状态在状态转换中优先。
     * 
     * @param StateName Name of the camera state / 相机状态名称
     * @return Priority value, or 0 if not found / 优先级值，未找到返回0
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    int32 GetStatePriority(FName StateName) const;

    //========================================
    // Override Query / 覆盖查询
    //========================================

    /**
     * Check if state has any overrides defined
     * 检查状态是否定义了任何覆盖
     * 
     * @param StateName Name of the camera state / 相机状态名称
     * @return True if state has overrides / 状态有覆盖返回true
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    bool HasOverrides(FName StateName) const;

    /**
     * Get override count for a state
     * 获取状态的覆盖数量
     * 
     * @param StateName Name of the camera state / 相机状态名称
     * @return Number of overrides for this state / 此状态的覆盖数量
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config")
    int32 GetOverrideCount(FName StateName) const;

    //========================================
    // Cache Management / 缓存管理
    //========================================

    /**
     * Clear all cached configurations
     * 清除所有缓存的配置
     * 
     * Call this after modifying DataTables to ensure fresh data.
     * 修改DataTable后调用此方法以确保获取最新数据。
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Config")
    void ClearCache();

    /**
     * Pre-cache configurations for specified states
     * 预缓存指定状态的配置
     * 
     * Useful for pre-loading commonly used states at level start.
     * 在关卡开始时预加载常用状态很有用。
     * 
     * @param StateNames Array of state names to pre-cache
     *                   要预缓存的状态名称数组
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Config")
    void PreCacheStates(const TArray<FName>& StateNames);

    //========================================
    // Debug / 调试
    //========================================

    /**
     * Get statistics about loaded data
     * 获取已加载数据的统计信息
     * 
     * @param OutStateCount Number of loaded states / 已加载状态数量
     * @param OutOverrideCount Number of loaded overrides / 已加载覆盖数量
     * @param OutCachedCount Number of cached configurations / 已缓存配置数量
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Config|Debug")
    void GetStatistics(int32& OutStateCount, int32& OutOverrideCount, int32& OutCachedCount) const;

    /**
     * Log all loaded states and overrides for debugging
     * 记录所有已加载的状态和覆盖用于调试
     * 
     * Only available in development builds.
     * 仅在开发版本中可用。
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Config|Debug", meta = (DevelopmentOnly))
    void LogAllData() const;

protected:
    //========================================
    // Internal Methods / 内部方法
    //========================================

    /**
     * Load state definitions from DataTable
     * 从DataTable加载状态定义
     * 
     * Populates StateCategories and StatePriorities maps.
     * 填充StateCategories和StatePriorities映射。
     */
    void LoadStateDefinitions();

    /**
     * Load parameter overrides from DataTable
     * 从DataTable加载参数覆盖
     * 
     * Populates AllOverrides map grouped by state name.
     * 填充按状态名称分组的AllOverrides映射。
     */
    void LoadOverrides();

    /**
     * Build complete config from defaults + overrides
     * 从默认值和覆盖构建完整配置
     * 
     * Core method that implements the "Default + Override" pattern.
     * 实现"默认值 + 覆盖"模式的核心方法。
     * 
     * @param StateName State to build config for / 要构建配置的状态
     * @param Category State's category (for default selection)
     *                 状态的类别（用于选择默认值）
     * @param OutConfig Output configuration / 输出配置
     */
    void BuildCompleteConfig(FName StateName, ECameraStateCategory Category, FCameraStateConfig& OutConfig) const;

    /**
     * Apply overrides to a configuration
     * 将覆盖应用到配置
     * 
     * Iterates through all overrides for a state and applies them.
     * 遍历状态的所有覆盖并应用它们。
     * 
     * @param Overrides Override collection for this state / 此状态的覆盖集合
     * @param OutConfig Configuration to modify / 要修改的配置
     */
    void ApplyOverridesToConfig(const FStateParamOverrides& Overrides, FCameraStateConfig& OutConfig) const;

    /**
     * Apply a single override to configuration
     * 应用单个覆盖到配置
     * 
     * Handles the specific logic for each parameter type.
     * 处理每种参数类型的特定逻辑。
     * 
     * @param Override Single override row to apply / 要应用的单个覆盖行
     * @param OutConfig Configuration to modify / 要修改的配置
     */
    void ApplySingleOverride(const FCameraParamOverrideRow& Override, FCameraStateConfig& OutConfig) const;

private:
    //========================================
    // DataTable References / 数据表引用
    //========================================

    /** 
     * DataTable containing state definitions (FCameraStateRow)
     * 包含状态定义的DataTable（FCameraStateRow）
     */
    UPROPERTY()
    UDataTable* StatesDataTable;

    /** 
     * DataTable containing parameter overrides (FCameraParamOverrideRow)
     * 包含参数覆盖的DataTable（FCameraParamOverrideRow）
     */
    UPROPERTY()
    UDataTable* OverridesDataTable;

    //========================================
    // Loaded Data / 已加载数据
    //========================================

    /** 
     * Map of state name to category (from state definitions)
     * 状态名称到类别的映射（来自状态定义）
     * 
     * Populated during LoadStateDefinitions().
     * 在LoadStateDefinitions()期间填充。
     */
    TMap<FName, ECameraStateCategory> StateCategories;

    /** 
     * Map of state name to priority
     * 状态名称到优先级的映射
     * 
     * Used for state transition conflict resolution.
     * 用于状态转换冲突解决。
     */
    TMap<FName, int32> StatePriorities;

    /** 
     * All loaded overrides grouped by state name
     * 按状态名称分组的所有已加载覆盖
     * 
     * FStateParamOverrides contains array of FCameraParamOverrideRow.
     * FStateParamOverrides包含FCameraParamOverrideRow数组。
     */
    TMap<FName, FStateParamOverrides> AllOverrides;

    //========================================
    // Cache / 缓存
    //========================================

    /** 
     * Cached complete configurations
     * 缓存的完整配置
     * 
     * mutable allows modification in const methods (GetCachedStateConfig).
     * mutable允许在const方法中修改（GetCachedStateConfig）。
     * Key: State name, Value: Complete FCameraStateConfig
     * 键：状态名称，值：完整的FCameraStateConfig
     */
    mutable TMap<FName, FCameraStateConfig> ConfigCache;

    //========================================
    // State / 状态
    //========================================

    /** 
     * Initialization flag
     * 初始化标志
     * 
     * True after successful Initialize() call.
     * 成功调用Initialize()后为true。
     */
    bool bIsInitialized;
};
