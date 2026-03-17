// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "CameraStage_InputGather.generated.h"

// Forward declarations
class ACharacter;
class APlayerController;

/**
 * UCameraStage_InputGather - Stage 1: Input Gather
 * 阶段1：输入收集
 * 
 * First stage in the pipeline. Collects all input data needed for camera calculations:
 * Pipeline第一阶段，收集相机计算所需的所有输入数据：
 * 
 * - Player controller input (camera rotation) 玩家控制器输入
 * - Character state (position, velocity, movement) 角色状态
 * - Target information (lock-on target) 目标信息
 * - Environment state (tight spaces, low ceiling) 环境状态
 * - Previous frame data 上一帧数据
 * 
 * Layer: INPUT (输入层)
 * Required: Yes (必需)
 * Skippable: No (不可跳过)
 */
UCLASS(BlueprintType)
class SOUL_API UCameraStage_InputGather : public UObject, public ICameraStage
{
    GENERATED_BODY()

public:
    /** Constructor */
    UCameraStage_InputGather();

    //========================================
    // ICameraStage Interface 接口实现
    //========================================

    /** Get stage index (1 for InputGather) */
    virtual int32 GetStageIndex() const override { return 1; }
    
    /** Get stage name for debugging */
    virtual FName GetStageName() const override { return FName(TEXT("InputGather")); }
    
    /** This stage cannot be skipped */
    virtual bool CanBeSkipped() const override { return false; }
    
    /** Execute the input gathering stage */
    virtual EStageResult Execute(FStageExecutionContext& Context) override;

    /** Pre-execute hook for debugging */
    virtual void OnPreExecute(const FStageExecutionContext& Context) override;

    /** Post-execute hook for debugging */
    virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) override;

protected:
    //========================================
    // Input Gathering Methods 输入收集方法
    //========================================

    /** 
     * Gather player controller input (camera rotation)
     * 收集玩家控制器输入（相机旋转）
     */
    void GatherPlayerInput(FStageExecutionContext& Context);

    /** 
     * Gather character state (position, velocity, movement flags)
     * 收集角色状态（位置、速度、移动标志）
     */
    void GatherCharacterState(FStageExecutionContext& Context);

    /** 
     * Gather target information (lock-on target)
     * 收集目标信息（锁定目标）
     */
    void GatherTargetInfo(FStageExecutionContext& Context);

    /** 
     * Gather environment state (tight spaces, low ceiling, cliff edge)
     * 收集环境状态（狭窄空间、低天花板、悬崖边缘）
     */
    void GatherEnvironmentState(FStageExecutionContext& Context);

    /** 
     * Store previous frame data for smoothing
     * 存储上一帧数据用于平滑
     */
    void StorePreviousFrameData(FStageExecutionContext& Context);

    //========================================
    // Helper Methods 辅助方法
    //========================================

    /**
     * Get the controlled character from manager
     * 从管理器获取被控制的角色
     */
    ACharacter* GetControlledCharacter(const FStageExecutionContext& Context) const;

    /**
     * Get the player controller from manager
     * 从管理器获取玩家控制器
     */
    APlayerController* GetPlayerController(const FStageExecutionContext& Context) const;

    //========================================
    // Configuration 配置
    //========================================

    /** 
     * Minimum speed to consider character as moving (cm/s)
     * 判定角色移动的最小速度阈值（厘米/秒）
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MovingSpeedThreshold;

    /** 
     * Time threshold to consider input as inactive (seconds)
     * 判定输入为非活跃的时间阈值（秒）
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float InputInactiveThreshold;

    /**
     * Minimum input magnitude to register as input
     * 注册为输入的最小输入幅度
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float InputDeadzone;

private:
    //========================================
    // Runtime State 运行时状态
    //========================================

    /** Accumulated time since last valid input */
    float TimeSinceLastInput;

    /** Previous frame's input for comparison */
    FVector2D PreviousInput;

    /** Previous frame's camera location */
    FVector PreviousCameraLocation;

    /** Previous frame's camera rotation */
    FRotator PreviousCameraRotation;

    /** Is this the first frame of execution */
    bool bFirstFrame;
};
