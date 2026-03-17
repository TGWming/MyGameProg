// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraConfig.h"
#include "Camera/Core/SoulsCameraParams_Collision.h"
#include "SoulsCameraPresets.generated.h"

/**
 * USoulsCameraPresets
 * 
 * Static factory class for camera configuration presets.
 * Provides ready-to-use configurations for common Souls-like scenarios.
 * 
 * Usage:
 * - State Presets: Complete camera state configurations
 * - Style Presets: Game-specific camera styles (Dark Souls, Bloodborne, etc.)
 * - Module Presets: Recommended module combinations for each scenario
 * - Collision Presets: Collision handling configurations
 */
UCLASS(BlueprintType)
class SOUL_API USoulsCameraPresets : public UObject
{
	GENERATED_BODY()

public:
	//========================================
	// State Config Presets - 状态配置预设
	//========================================

	/** Get default Exploration state config - 探索状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetExplorationConfig();

	/** Get default Combat state config - 战斗状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetCombatConfig();

	/** Get default Boss Lock-On state config - Boss锁定状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetBossLockOnConfig();

	/** Get default Dialogue state config - 对话状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetDialogueConfig();

	/** Get default Cinematic state config - 过场动画状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetCinematicConfig();

	/** Get default Aiming state config - 瞄准状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetAimingConfig();

	/** Get default Mounted state config - 骑乘状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetMountedConfig();

	/** Get default Swimming state config - 游泳状态 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetSwimmingConfig();

	/** Get config for a specific state category - 按分类获取配置 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|State")
	static FCameraStateConfig GetConfigForCategory(ECameraStateCategory Category);

	//========================================
	// Game Style Presets - 游戏风格预设
	//========================================

	/** Dark Souls style - 黑暗之魂风格（沉重、审慎） */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Style")
	static FCameraStateConfig GetDarkSoulsStyle();

	/** Bloodborne style - 血源诅咒风格（更快、更近） */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Style")
	static FCameraStateConfig GetBloodborneStyle();

	/** Sekiro style - 只狼风格（非常灵敏） */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Style")
	static FCameraStateConfig GetSekiroStyle();

	/** Elden Ring style - 艾尔登法环风格（开阔视野） */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Style")
	static FCameraStateConfig GetEldenRingStyle();

	//========================================
	// Module Activation Presets - 模块激活预设
	//========================================

	/** Get recommended modules for Exploration - 探索模块组合 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Module")
	static TArray<ECameraModuleType> GetExplorationModules();

	/** Get recommended modules for Combat - 战斗模块组合 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Module")
	static TArray<ECameraModuleType> GetCombatModules();

	/** Get recommended modules for Boss Lock-On - Boss锁定模块组合 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Module")
	static TArray<ECameraModuleType> GetBossLockOnModules();

	//========================================
	// Collision Presets - 碰撞配置预设
	//========================================

	/** Get default collision config - 默认碰撞配置 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Collision")
	static FCollisionParams GetDefaultCollisionConfig();

	/** Get aggressive collision config (fast response) - 激进碰撞配置 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Collision")
	static FCollisionParams GetAggressiveCollisionConfig();

	/** Get relaxed collision config (smooth response) - 宽松碰撞配置 */
	UFUNCTION(BlueprintPure, Category = "Camera Presets|Collision")
	static FCollisionParams GetRelaxedCollisionConfig();
};
