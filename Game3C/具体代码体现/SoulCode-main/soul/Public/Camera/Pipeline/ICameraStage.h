// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "ICameraStage.generated.h"

// Forward declarations
class USoulsCameraManager;

/**
 * Pipeline stage execution context
 * Contains all data needed for stage execution
 * Stage间传递的执行上下文
 */
USTRUCT(BlueprintType)
struct SOUL_API FStageExecutionContext
{
	GENERATED_BODY()

	//========================================
	// Timing
	//========================================
	
	/** Delta time for this frame */
	UPROPERTY(BlueprintReadOnly, Category = "Context|Timing")
	float DeltaTime;

	/** Total game time */
	UPROPERTY(BlueprintReadOnly, Category = "Context|Timing")
	float GameTime;

	//========================================
	// References
	//========================================
	
	/** Reference to camera manager */
	UPROPERTY(BlueprintReadOnly, Category = "Context|References")
	USoulsCameraManager* Manager;

	//========================================
	// Input Data
	//========================================
	
	/** Input context from game (Stage 1 output) */
	UPROPERTY(BlueprintReadWrite, Category = "Context|Input")
	FCameraInputContext InputContext;

	//========================================
	// Accumulated Output
	//========================================
	
	/** Current camera output being built through stages */
	UPROPERTY(BlueprintReadWrite, Category = "Context|Output")
	FSoulsCameraOutput Output;

	//========================================
	// Stage-Specific Outputs
	//========================================
	
	/** Module outputs from Stage 3 */
	TArray<FModuleOutput> ModuleOutputs;

	/** Modifier outputs from Stage 4 */
	TArray<FModifierOutput> ModifierOutputs;

	/** Collision output from Stage 6 */
	UPROPERTY(BlueprintReadOnly, Category = "Context|Stages")
	FCollisionOutput CollisionOutput;

	//========================================
	// Pipeline Control
	//========================================

	/** Should skip remaining stages */
	UPROPERTY(BlueprintReadOnly, Category = "Context|Control")
	bool bShouldAbort;

	/** Stage that requested abort (1-8, -1 = none) */
	UPROPERTY(BlueprintReadOnly, Category = "Context|Control")
	int32 AbortRequestedByStage;

	//========================================
	// Override Modifier State (Stage 4 -> Stage 5)
	//========================================

	/** True if an override modifier (cinematic/death cam) is active */
	UPROPERTY(BlueprintReadOnly, Category = "Context|Override")
	bool bHasOverrideModifier;

	/** Override transform when bHasOverrideModifier is true */
	UPROPERTY(BlueprintReadOnly, Category = "Context|Override")
	FTransform OverrideTransform;

	//========================================
	// State Tracking
	//========================================
	
	/** Current executing stage index (1-8) */
	UPROPERTY(BlueprintReadOnly, Category = "Context|State")
	int32 CurrentStageIndex;

	/** Is this the first frame after initialization */
	UPROPERTY(BlueprintReadOnly, Category = "Context|State")
	bool bIsFirstFrame;

	/** Frame counter (for debugging) - not exposed to Blueprint due to uint64 */
	uint64 FrameCounter;

	/** Did any stage fail this frame */
	UPROPERTY(BlueprintReadOnly, Category = "Context|State")
	bool bHasStageError;

	//========================================
	// Constructor & Methods
	//========================================

	FStageExecutionContext()
		: DeltaTime(0.0f)
		, GameTime(0.0f)
		, Manager(nullptr)
		, bShouldAbort(false)
		, AbortRequestedByStage(-1)
		, bHasOverrideModifier(false)
		, OverrideTransform(FTransform::Identity)
		, CurrentStageIndex(0)
		, bIsFirstFrame(true)
		, FrameCounter(0)
		, bHasStageError(false)
	{}

	/** Reset context for new frame */
	void Reset()
	{
		InputContext.Reset();
		Output = FSoulsCameraOutput();
		ModuleOutputs.Empty();
		ModifierOutputs.Empty();
		CollisionOutput.Reset();
		bShouldAbort = false;
		AbortRequestedByStage = -1;
		bHasOverrideModifier = false;
		OverrideTransform = FTransform::Identity;
		CurrentStageIndex = 0;
		bHasStageError = false;
	}

	/** Increment frame counter */
	void IncrementFrame()
	{
		FrameCounter++;
		bIsFirstFrame = false;
	}

	/** Set current stage */
	void SetCurrentStage(int32 StageIndex)
	{
		CurrentStageIndex = StageIndex;
	}

	/** Mark stage error */
	void SetStageError(int32 StageIndex)
	{
		bHasStageError = true;
		// Optionally record which stage had error
	}
};

/**
 * Stage execution result
 * Stage执行结果枚举
 */
UENUM(BlueprintType)
enum class EStageResult : uint8
{
	/** Stage executed successfully 成功执行 */
	Success         UMETA(DisplayName = "Success"),
	
	/** Stage was skipped (optional stage) 跳过执行 */
	Skipped         UMETA(DisplayName = "Skipped"),
	
	/** Stage failed but pipeline can continue 失败但继续 */
	Failed          UMETA(DisplayName = "Failed"),
	
	/** Stage requests pipeline abort 请求中止 */
	Abort           UMETA(DisplayName = "Abort")
};

/**
 * ICameraStage - Interface for all pipeline stages
 * Pipeline Stage接口基类
 * 
 * 8个Stage顺序执行：
 * Stage 1: Input Gather - Collect player input and game state
 * Stage 2: State Machine - Evaluate and update camera state
 * Stage 3: Module Compute - Execute active camera modules
 * Stage 4: Modifier Apply - Apply active camera modifiers (skippable)
 * Stage 5: Blend & Solve - Blend outputs and resolve conflicts
 * Stage 6: Collision Resolve - Handle camera collision (skippable)
 * Stage 7: Post Process - Apply final adjustments (skippable)
 * Stage 8: Render Apply - Apply output to components
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UCameraStage : public UInterface
{
	GENERATED_BODY()
};

class SOUL_API ICameraStage
{
	GENERATED_BODY()

public:
	/**
	 * Get the stage index (1-8)
	 * @return Stage number (1=Input Gather, 8=Render Apply)
	 */
	virtual int32 GetStageIndex() const = 0;

	/**
	 * Get the stage name for debugging
	 * @return Human-readable stage name
	 */
	virtual FName GetStageName() const = 0;

	/**
	 * Check if this stage can be skipped
	 * @return True if stage is optional (e.g., Modifier Apply, Collision Resolve, Post Process)
	 */
	virtual bool CanBeSkipped() const { return false; }

	/**
	 * Check if this stage should execute this frame
	 * @param Context Execution context
	 * @return True if stage should execute
	 */
	virtual bool ShouldExecute(const FStageExecutionContext& Context) const { return true; }

	/**
	 * Execute the stage
	 * @param Context Execution context (modified in place)
	 * @return Execution result
	 */
	virtual EStageResult Execute(FStageExecutionContext& Context) = 0;

	/**
	 * Called when stage is about to execute (for debugging/profiling)
	 * @param Context Execution context
	 */
	virtual void OnPreExecute(const FStageExecutionContext& Context) {}

	/**
	 * Called after stage execution (for debugging/profiling)
	 * @param Context Execution context
	 * @param Result Execution result
	 */
	virtual void OnPostExecute(const FStageExecutionContext& Context, EStageResult Result) {}

};
