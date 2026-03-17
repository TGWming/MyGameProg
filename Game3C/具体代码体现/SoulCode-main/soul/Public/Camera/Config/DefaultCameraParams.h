// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * DefaultCameraParams.h
 * 
 * Default Camera Parameters Configuration Class
 * 默认相机参数配置类
 * 
 * This class provides the default parameter values for the Souls-like camera system.
 * All 725 camera states share these default values. Only states requiring special
 * behavior need to define overrides.
 * 
 * 该类为类魂相机系统提供默认参数值。
 * 所有725个相机状态共享这些默认值。只有需要特殊行为的状态才需要定义覆盖。
 * 
 * Architecture: "Default Values + Override Table"
 * 架构："默认值 + 覆盖表"
 * 
 * Parameter Groups:
 * 参数组：
 * - A: StateBase (34 params) - 基础状态参数
 * - B: Module (32 params) - 模块参数
 * - C: Modifier (58 params) - 修改器参数
 * - D: Collision (30 params) - 碰撞参数
 * 
 * Total: 154 parameters per state
 * 总计：每个状态154个参数
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Core/SoulsCameraConfig.h"
#include "Camera/Data/CameraStateEnums.h"
#include "DefaultCameraParams.generated.h"

/**
 * UDefaultCameraParams
 * 
 * Provides centralized default parameter values for the camera system.
 * Uses static methods to return default configurations that can be
 * customized per category or used as-is for most camera states.
 * 
 * 为相机系统提供集中的默认参数值。
 * 使用静态方法返回默认配置，可以按分类自定义或直接用于大多数相机状态。
 * 
 * Usage 使用方法:
 * - GetDefaultConfig(): Get complete default configuration
 *                       获取完整默认配置
 * - GetDefaultConfigForCategory(): Get category-specific defaults
 *                                  获取特定分类的默认配置
 * - Individual getters for specific parameter groups
 *   各参数组的独立获取器
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UDefaultCameraParams : public UObject
{
    GENERATED_BODY()
    
public:
    UDefaultCameraParams();

    //========================================
    // Static Fallback Getters
    // 静态回退值获取器
    //========================================
    
    /**
     * Get the default base distance for camera initialization fallback.
     * 获取相机初始化回退用的默认基础距离。
     * 
     * @return Default base distance (400.0f)
     */
    FORCEINLINE static float GetDefaultBaseDistance() { return 400.0f; }
    
    /**
     * Get the default base FOV for camera initialization fallback.
     * 获取相机初始化回退用的默认基础FOV。
     * 
     * @return Default base FOV (70.0f)
     */
    FORCEINLINE static float GetDefaultBaseFOV() { return 70.0f; }

    //========================================
    // Complete Configuration Methods
    // 完整配置方法
    //========================================

    /**
     * Get the complete default camera configuration.
     * Returns a FCameraStateConfig with all default values.
     * 
     * 获取完整的默认相机配置。
     * 返回包含所有默认值的FCameraStateConfig。
     * 
     * @return Complete default camera state configuration
     *         完整的默认相机状态配置
     */
    UFUNCTION(BlueprintPure, Category = "Camera|DefaultParams")
    static FCameraStateConfig GetDefaultConfig();

    /**
     * Get default configuration customized for a specific category.
     * Applies category-specific overrides to the base defaults.
     * 
     * 获取针对特定分类自定义的默认配置。
     * 在基础默认值上应用分类特定的覆盖。
     * 
     * @param Category The camera state category
     *                 相机状态分类
     * @return Configuration with category-specific defaults applied
     *         应用了分类特定默认值的配置
     */
    UFUNCTION(BlueprintPure, Category = "Camera|DefaultParams")
    static FCameraStateConfig GetDefaultConfigForCategory(ECameraStateCategory Category);

    //========================================
    // Individual Parameter Group Getters
    // 各参数组获取器
    //========================================

    /**
     * Get default StateBase parameters (A-Group, 34 params).
     * Contains identity, distance, FOV, rotation, offset, lag, transition,
     * collision, auto-correct, flags, and hierarchy settings.
     * 
     * 获取默认StateBase参数（A组，34个参数）。
     * 包含身份、距离、FOV、旋转、偏移、延迟、过渡、
     * 碰撞、自动校正、标志和层级设置。
     * 
     * @return Default StateBase parameters
     *         默认StateBase参数
     */
    UFUNCTION(BlueprintPure, Category = "Camera|DefaultParams")
    static FStateBaseParams GetDefaultStateBaseParams();

    /**
     * Get StateBase parameters customized for a specific category.
     * Applies category-specific adjustments (e.g., combat uses different
     * distance, boss uses different FOV).
     * 
     * 获取针对特定分类自定义的StateBase参数。
     * 应用分类特定的调整（如战斗使用不同距离，Boss使用不同FOV）。
     * 
     * @param Category The camera state category
     *                 相机状态分类
     * @return StateBase parameters with category overrides
     *         带有分类覆盖的StateBase参数
     */
    UFUNCTION(BlueprintPure, Category = "Camera|DefaultParams")
    static FStateBaseParams GetDefaultStateBaseParamsForCategory(ECameraStateCategory Category);

    /**
     * Get default Module parameters (B-Group, 32 params).
     * Contains position, rotation, distance, FOV, offset, and constraint
     * module behavior settings.
     * 
     * 获取默认Module参数（B组，32个参数）。
     * 包含位置、旋转、距离、FOV、偏移和约束模块行为设置。
     * 
     * @return Default Module parameters
     *         默认Module参数
     */
    UFUNCTION(BlueprintPure, Category = "Camera|DefaultParams")
    static FModuleParams GetDefaultModuleParams();

    /**
     * Get default Modifier parameters (C-Group, 58 params).
     * Contains shake, reaction, cinematic, zoom, effect, and special
     * modifier behavior settings.
     * 
     * 获取默认Modifier参数（C组，58个参数）。
     * 包含震动、反应、电影、缩放、效果和特殊修改器行为设置。
     * 
     * @return Default Modifier parameters
     *         默认Modifier参数
     */
    UFUNCTION(BlueprintPure, Category = "Camera|DefaultParams")
    static FModifierParams GetDefaultModifierParams();

    /**
     * Get default Collision parameters (D-Group, 30 params).
     * Contains detection, response, occlusion, recovery, and special
     * collision handling settings.
     * 
     * 获取默认Collision参数（D组，30个参数）。
     * 包含检测、响应、遮挡、恢复和特殊碰撞处理设置。
     * 
     * @return Default Collision parameters
     *         默认Collision参数
     */
    UFUNCTION(BlueprintPure, Category = "Camera|DefaultParams")
    static FCollisionParams GetDefaultCollisionParams();

private:
    //========================================
    // Internal Helper Methods
    // 内部辅助方法
    //========================================

    /**
     * Apply category-specific overrides to StateBase parameters.
     * Modifies the provided parameters based on the category's requirements.
     * 
     * 为StateBase参数应用分类特定的覆盖。
     * 根据分类的需求修改提供的参数。
     * 
     * @param OutParams Parameters to modify (in/out)
     *                  要修改的参数（输入/输出）
     * @param Category The category to apply overrides for
     *                 要应用覆盖的分类
     */
    static void ApplyCategoryOverrides(FStateBaseParams& OutParams, ECameraStateCategory Category);

    /**
     * Create base default StateBase parameters.
     * Used as the foundation before category overrides are applied.
     * 
     * 创建基础默认StateBase参数。
     * 用作应用分类覆盖之前的基础。
     * 
     * @return Base StateBase parameters
     *         基础StateBase参数
     */
    static FStateBaseParams CreateBaseStateParams();

    /**
     * Create base default Module parameters.
     * 
     * 创建基础默认Module参数。
     * 
     * @return Base Module parameters
     *         基础Module参数
     */
    static FModuleParams CreateBaseModuleParams();

    /**
     * Create base default Modifier parameters.
     * 
     * 创建基础默认Modifier参数。
     * 
     * @return Base Modifier parameters
     *         基础Modifier参数
     */
    static FModifierParams CreateBaseModifierParams();

    /**
     * Create base default Collision parameters.
     * 
     * 创建基础默认Collision参数。
     * 
     * @return Base Collision parameters
     *         基础Collision参数
     */
    static FCollisionParams CreateBaseCollisionParams();
};
