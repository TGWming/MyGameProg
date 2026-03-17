// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "Camera/Core/SoulsCameraConfig.h"
#include "CameraStage_ModuleCompute.generated.h"

// Forward declarations
class UCameraModuleRegistry;
class UCameraModuleBase;

/**
 * UCameraStage_ModuleCompute - Stage 3: Module Compute
 * 阶段3：模块计算
 * 
 * Third stage in the pipeline. Executes active camera modules:
 * Pipeline第三阶段，执行激活的相机模块：
 * 
 * - Position modules (P01-P08): Calculate focus point 计算焦点位置
 * - Rotation modules (R01-R09): Calculate camera rotation 计算相机旋转
 * - Distance modules (D01-D07): Calculate arm length 计算臂长距离
 * - FOV modules (F01-F06): Calculate field of view 计算视野角度
 * - Offset modules (O01-O05): Calculate socket/target offsets 计算偏移量
 * - Constraint modules (C01-C04): Apply limits and constraints 应用限制约束
 * 
 * Each module produces FModuleOutput which is collected for blending in Stage 5.
 * 每个模块产生FModuleOutput，收集后在Stage 5中混合。
 * 
 * Layer:  COMPUTE (计算层)
 * Required:  Yes (必需)
 * Skippable: No (不可跳过)
 * 
 * Note: This is a skeleton implementation.  Full module system will be implemented in Phase 3.
 * 注意：这是骨架实现，完整模块系统将在Phase 3实现。
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_ModuleCompute : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_ModuleCompute();

    //========================================
    // ICameraStage Interface 接口实现
    //========================================

    /** Get stage index (3 for ModuleCompute) */
    virtual int32 GetStageIndex() const override { return 3; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("ModuleCompute")); }
    
    /** This stage cannot be skipped */
    virtual bool CanBeSkipped() const override { return false; }
    
    /** Execute the module compute stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

    //========================================
    // Module Registry Reference 模块注册表引用
    //========================================

    /** 
     * Set reference to module registry
     * @param InRegistry Module registry instance
     */
    void SetModuleRegistry(UCameraModuleRegistry* InRegistry);

    /**
     * Get the module registry reference
     * @return Module registry instance, or nullptr if not set
     */
    FORCEINLINE UCameraModuleRegistry* GetModuleRegistry() const { return ModuleRegistry; }

protected:
    //========================================
    // Module Execution 模块执行
    //========================================

    /** 
     * Execute all active modules and collect outputs
     * 执行所有激活的模块并收集输出
     */
    void ExecuteModules(FStageExecutionContext& Context);

    /** 
     * Execute modules of a specific category
     * 执行特定分类的模块
     * @param Context Execution context
     * @param Category Module category to execute
     */
    void ExecuteModuleCategory(FStageExecutionContext& Context, ECameraModuleCategory Category);

    /**
     * Execute a single module
     * 执行单个模块
     * @param Context Execution context
     * @param Module Module to execute
     * @return True if module executed successfully
     */
    bool ExecuteSingleModule(FStageExecutionContext& Context, UCameraModuleBase* Module);

    //========================================
    // Placeholder Methods (Phase 3) 占位方法
    //========================================

    /**
     * Placeholder:  Execute position modules
     * 占位：执行位置模块
     */
    void ExecutePositionModules(FStageExecutionContext& Context);

    /**
     * Placeholder:  Execute rotation modules
     * 占位：执行旋转模块
     */
    void ExecuteRotationModules(FStageExecutionContext& Context);

    /**
     * Placeholder:  Execute distance modules
     * 占位：执行距离模块
     */
    void ExecuteDistanceModules(FStageExecutionContext& Context);

    /**
     * Placeholder:  Execute FOV modules
     * 占位：执行FOV模块
     */
    void ExecuteFOVModules(FStageExecutionContext& Context);

    /**
     * Placeholder:  Execute offset modules
     * 占位：执行偏移模块
     */
    void ExecuteOffsetModules(FStageExecutionContext& Context);

    /**
     * Placeholder:  Execute constraint modules
     * 占位：执行约束模块
     */
    void ExecuteConstraintModules(FStageExecutionContext& Context);

	//========================================
	// Fallback Outputs
	//========================================

	/**
	 * Create fallback outputs when ModuleRegistry is not available
	 * 
	 * This provides basic camera functionality using state config values
	 * when the module system is not properly initialized. 
	 * 
	 * Creates outputs for:
	 * - Position (follow character)
	 * - Rotation (maintain previous)
	 * - Distance (base distance)
	 * - FOV (base FOV)
	 * 
	 * @param Context Current execution context (will be modified)
	 * @param StateConfig Current camera state configuration
	 */
	void CreateFallbackOutputs(FStageExecutionContext& Context, const FCameraStateConfig& StateConfig);

private:
    //========================================
    // Module Registry 模块注册表
    //========================================

    /** 
     * Reference to module registry (will be set in Phase 3) 
     * Note: Not a UPROPERTY because UCameraModuleRegistry is not yet defined
     * 注意：非UPROPERTY，因为UCameraModuleRegistry尚未定义
     */
    UCameraModuleRegistry* ModuleRegistry;

    //========================================
    // Execution Statistics 执行统计
    //========================================

    /** Number of modules executed this frame */
    int32 ModulesExecutedCount;

    /** Number of modules skipped this frame */
    int32 ModulesSkippedCount;

    /** Total module execution time (ms) for profiling */
    float TotalModuleTimeMs;

    // ★ Debug LOG 开关已移至 SoulsCameraManager 统一管理
    // 使用 Context.Manager->IsStage3DebugEnabled() 访问
};
