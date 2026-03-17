// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICameraStage.h"
#include "SoulsCameraPipeline.generated.h"

// Forward declarations
class USoulsCameraManager;

/**
 * Pipeline execution statistics
 * Pipeline执行统计数据
 */
USTRUCT(BlueprintType)
struct SOUL_API FPipelineStats
{
	GENERATED_BODY()

	/** Total execution time in milliseconds 总执行时间(毫秒) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float TotalTimeMs;

	/** Per-stage execution times 各Stage执行时间 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	TArray<float> StageTimesMs;

	/** Number of stages executed 执行的Stage数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 StagesExecuted;

	/** Number of stages skipped 跳过的Stage数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 StagesSkipped;

	/** Frame number 帧号 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int64 FrameNumber;

	/** Default constructor */
	FPipelineStats()
		: TotalTimeMs(0.0f)
		, StagesExecuted(0)
		, StagesSkipped(0)
		, FrameNumber(0)
	{
		StageTimesMs.SetNum(8);
	}

	/** Reset stats to defaults */
	void Reset()
	{
		TotalTimeMs = 0.0f;
		for (float& Time : StageTimesMs)
		{
			Time = 0.0f;
		}
		StagesExecuted = 0;
		StagesSkipped = 0;
	}
};

/**
 * USoulsCameraPipeline
 * Pipeline管理器类
 * 
 * Manages the execution of all 8 camera pipeline stages.
 * Each frame, stages are executed in order:
 * 
 * INPUT LAYER (输入层):
 *   Stage 1: Input Gather - 收集输入和游戏状态
 * 
 * DECISION LAYER (决策层):
 *   Stage 2: State Machine - 状态机评估和切换
 * 
 * COMPUTE LAYER (计算层):
 *   Stage 3: Module Compute - 执行相机模块计算
 *   Stage 4: Modifier Apply (skippable) - 应用修改器效果(可跳过)
 *   Stage 5: Blend & Solve - 混合输出并解决冲突
 * 
 * SAFETY LAYER (安全层):
 *   Stage 6: Collision Resolve (skippable) - 碰撞解算(可跳过)
 * 
 * OUTPUT LAYER (输出层):
 *   Stage 7: Post Process (skippable) - 后处理调整(可跳过)
 *   Stage 8: Render Apply - 应用到渲染组件
 */
UCLASS(BlueprintType)
class SOUL_API USoulsCameraPipeline : public UObject
{
	GENERATED_BODY()

public:
	/** Constructor */
	USoulsCameraPipeline();

	//========================================
	// Initialization 初始化
	//========================================

	/** 
	 * Initialize pipeline with camera manager reference
	 * @param InManager Camera manager owning this pipeline
	 */
	void Initialize(USoulsCameraManager* InManager);

	/** 
	 * Check if pipeline is initialized
	 * @return True if initialized
	 */
	FORCEINLINE bool IsInitialized() const { return bIsInitialized; }

	//========================================
	// Stage Registration Stage注册
	//========================================

	/** 
	 * Register a stage at specified index (1-8)
	 * @param StageIndex Stage number (1-8)
	 * @param Stage Stage object implementing ICameraStage
	 * @return True if registration successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Pipeline")
	bool RegisterStage(int32 StageIndex, TScriptInterface<ICameraStage> Stage);

	/** 
	 * Unregister a stage
	 * @param StageIndex Stage number (1-8)
	 * @return True if unregistration successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Pipeline")
	bool UnregisterStage(int32 StageIndex);

	/** 
	 * Get stage at index
	 * @param StageIndex Stage number (1-8)
	 * @return Stage interface, or null if not registered
	 */
	TScriptInterface<ICameraStage> GetStage(int32 StageIndex) const;

	/** 
	 * Check if all required stages are registered
	 * @return True if all non-skippable stages are registered
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline")
	bool AreAllStagesRegistered() const;

	//========================================
	// Execution 执行
	//========================================

	/** 
	 * Execute the full pipeline
	 * @param DeltaTime Frame delta time
	 * @param InputContext Input context from Stage 1
	 * @return Final camera output
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Pipeline")
	FSoulsCameraOutput Execute(float DeltaTime, const FCameraInputContext& InputContext);

	/** 
	 * Get the current execution context (valid during execution)
	 * @return Current execution context
	 */
	FORCEINLINE const FStageExecutionContext& GetCurrentContext() const { return CurrentContext; }

	//========================================
	// Stage Control Stage控制
	//========================================

	/** 
	 * Enable/disable a specific stage
	 * @param StageIndex Stage number (1-8)
	 * @param bEnabled Enable flag
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Pipeline")
	void SetStageEnabled(int32 StageIndex, bool bEnabled);

	/** 
	 * Check if stage is enabled
	 * @param StageIndex Stage number (1-8)
	 * @return True if enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline")
	bool IsStageEnabled(int32 StageIndex) const;

	/** 
	 * Skip all optional stages (performance mode)
	 * @param bEnabled Performance mode flag
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Pipeline")
	void SetPerformanceMode(bool bEnabled);

	/**
	 * Check if performance mode is enabled
	 * @return True if in performance mode
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline")
	FORCEINLINE bool IsPerformanceMode() const { return bPerformanceMode; }

	//========================================
	// Statistics 统计
	//========================================

	/** 
	 * Get last frame's execution statistics
	 * @return Last frame stats
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline")
	FORCEINLINE FPipelineStats GetLastFrameStats() const { return LastFrameStats; }

	/** 
	 * Enable/disable stats collection
	 * @param bEnabled Stats collection flag
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Pipeline")
	void SetStatsEnabled(bool bEnabled) { bCollectStats = bEnabled; }

	/**
	 * Check if stats collection is enabled
	 * @return True if collecting stats
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Pipeline")
	FORCEINLINE bool IsStatsEnabled() const { return bCollectStats; }

protected:
	//========================================
	// Internal State 内部状态
	//========================================

	/** Reference to camera manager */
	UPROPERTY()
	USoulsCameraManager* CameraManager;

	/** Registered stages (index 0-7 for stages 1-8) */
	UPROPERTY()
	TArray<UObject*> RegisteredStages;

	/** Stage enabled flags (index 0-7 for stages 1-8) */
	TArray<bool> StageEnabled;

	/** Current execution context */
	FStageExecutionContext CurrentContext;

	/** Last frame statistics */
	FPipelineStats LastFrameStats;

	/** Is pipeline initialized */
	bool bIsInitialized;

	/** Collect execution statistics */
	bool bCollectStats;

	/** Performance mode (skip optional stages) */
	bool bPerformanceMode;

	/** Current frame number */
	int64 CurrentFrameNumber;

	//========================================
	// Internal Methods 内部方法
	//========================================

	/** 
	 * Execute a single stage
	 * @param StageIndex Stage number (1-8)
	 * @return Stage execution result
	 */
	EStageResult ExecuteStage(int32 StageIndex);

	/** 
	 * Check if stage at index is required (non-skippable)
	 * @param StageIndex Stage number (1-8)
	 * @return True if stage is required
	 */
	bool IsStageRequired(int32 StageIndex) const;

	/**
	 * Convert stage index to array index (1-8 -> 0-7)
	 * @param StageIndex Stage number (1-8)
	 * @return Array index (0-7), or -1 if invalid
	 */
	FORCEINLINE int32 StageIndexToArrayIndex(int32 StageIndex) const
	{
		return (StageIndex >= 1 && StageIndex <= 8) ? (StageIndex - 1) : -1;
	}
};
