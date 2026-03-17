// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "Camera/Core/SoulsCameraConfig.h"
#include "Camera/Data/CameraStateEnums.h"
#include "CameraStage_StateMachine.generated.h"

// Forward declarations
class USoulsCameraStateMachine;

/**
 * UCameraStage_StateMachine - Stage 2: State Machine Update
 * 阶段2：状态机更新
 * 
 * Second stage in the pipeline. Handles camera state management:
 * Pipeline第二阶段，处理相机状态管理：
 * 
 * - Evaluates input context to determine desired state 评估输入上下文确定目标状态
 * - Requests state changes when needed 需要时请求状态切换
 * - Updates state transition blending 更新状态过渡混合
 * - Retrieves state configuration from DataTable 从DataTable获取状态配置
 * - Sets initial output values based on current state 基于当前状态设置初始输出值
 * 
 * Layer: DECISION (决策层)
 * Required: Yes (必需)
 * Skippable: No (不可跳过)
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_StateMachine : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_StateMachine();

    //========================================
    // ICameraStage Interface 接口实现
    //========================================

    /** Get stage index (2 for StateMachine) */
    virtual int32 GetStageIndex() const override { return 2; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("StateMachine")); }
    
    /** This stage cannot be skipped */
    virtual bool CanBeSkipped() const override { return false; }
    
    /** Execute the state machine stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

    //========================================
    // State Machine Reference 状态机引用
    //========================================

    /** 
     * Set reference to the state machine
     * @param InStateMachine State machine instance
     */
    void SetStateMachine(USoulsCameraStateMachine* InStateMachine);

    /**
     * Get the state machine reference
     * @return State machine instance, or nullptr if not set
     */
    FORCEINLINE USoulsCameraStateMachine* GetStateMachine() const { return StateMachine; }

protected:
    //========================================
    // State Evaluation 状态评估
    //========================================

    /** 
     * Evaluate current state based on input context
     * Determines if a state change is needed
     * 评估当前状态，确定是否需要状态切换
     */
    void EvaluateState(FStageExecutionContext& Context);

    /**
     * Determine the desired state based on input context
     * 基于输入上下文确定目标状态
     * @param Context Execution context
     * @return Desired state name
     */
    FName DetermineDesiredState(const FStageExecutionContext& Context) const;

    //========================================
    // State Transition 状态过渡
    //========================================

    /** 
     * Update state transition blending
     * 更新状态过渡混合
     */
    void UpdateTransition(FStageExecutionContext& Context);

    /**
     * Check if currently in a state transition
     * @return True if transitioning between states
     */
    bool IsInTransition() const;

    //========================================
    // Configuration Application 配置应用
    //========================================

    /** 
     * Apply state configuration to output
     * Sets initial output values based on state config
     * 应用状态配置到输出
     */
    void ApplyStateConfig(FStageExecutionContext& Context, const FCameraStateConfig& Config);

    /** 
     * Blend between two state configurations
     * Used during state transitions
     * 混合两个状态配置
     */
    void BlendStateConfigs(
        FStageExecutionContext& Context,
        const FCameraStateConfig& FromConfig,
        const FCameraStateConfig& ToConfig,
        float BlendAlpha
    );

    /**
     * Apply blend curve to alpha value
     * 应用混合曲线到alpha值
     * @param Alpha Raw alpha (0-1)
     * @param BlendType Type of blend curve
     * @return Curved alpha value
     */
    float ApplyBlendCurve(float Alpha, ECameraBlendType BlendType) const;

    //========================================
    // State Priority 状态优先级
    //========================================

    /**
     * Get priority for a state category
     * Higher priority states override lower priority
     * 获取状态类别的优先级
     */
    int32 GetStatePriority(ECameraStateCategory Category) const;

private:
    //========================================
    // State Machine Reference
    //========================================

    /** Reference to state machine (owned by Manager) */
    UPROPERTY()
    USoulsCameraStateMachine* StateMachine;

    //========================================
    // Cached State Data 缓存的状态数据
    //========================================

    /** Cached current state config (reduces DataTable queries) */
    FCameraStateConfig CurrentStateConfig;

    /** Cached previous state config (for blending) */
    FCameraStateConfig PreviousStateConfig;

    /** Current state name */
    FName CurrentStateName;

    /** Previous state name (before transition) */
    FName PreviousStateName;

    /** Is current state config valid */
    bool bCurrentConfigValid;

    /** Is previous state config valid */
    bool bPreviousConfigValid;

    //========================================
    // Transition State 过渡状态
    //========================================

    /** Current transition progress (0-1) */
    float TransitionAlpha;

    /** Current transition duration */
    float TransitionDuration;

    /** Elapsed transition time */
    float TransitionElapsed;

    /** Current blend type for transition */
    ECameraBlendType CurrentBlendType;

    /** Is currently transitioning */
    bool bIsTransitioning;
};
