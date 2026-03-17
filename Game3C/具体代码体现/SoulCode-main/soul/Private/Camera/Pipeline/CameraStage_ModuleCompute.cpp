// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_ModuleCompute.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "Camera/Core/SoulsCameraStateMachine.h"

// Include module system
#include "Camera/Modules/CameraModuleRegistry.h"
#include "Camera/Modules/CameraModuleBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_Module, Log, All);

//========================================
// Constructor 构造函数
//========================================

UCameraStage_ModuleCompute::UCameraStage_ModuleCompute()
    : ModuleRegistry(nullptr)
    , ModulesExecutedCount(0)
    , ModulesSkippedCount(0)
    , TotalModuleTimeMs(0.0f)
{
}

//========================================
// ICameraStage Interface 接口实现
//========================================

EStageResult UCameraStage_ModuleCompute::Execute(FStageExecutionContext& Context)
{
    // 【诊断日志】记录Stage开始时的Distance
    float DistanceAtStart = Context.Output.Distance;

    // Reset statistics
    ModulesExecutedCount = 0;
    ModulesSkippedCount = 0;
    TotalModuleTimeMs = 0.0f;

    // Clear previous module outputs
    Context.ModuleOutputs.Empty();

    // Execute modules
    const double StartTime = FPlatformTime::Seconds();
    ExecuteModules(Context);
    TotalModuleTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

    UE_LOG(LogCameraStage_Module, Verbose, TEXT("Execute: Generated %d module outputs in %.2fms"), 
        Context.ModuleOutputs.Num(), TotalModuleTimeMs);

    // 【诊断日志】在return之前输出Distance变化
    static int32 Stage3LogCount = 0;
    if (Stage3LogCount < 10)
    {
        Stage3LogCount++;
        UE_LOG(LogTemp, Error, TEXT("【Stage3 ModuleCompute】 Distance: %.1f -> %.1f"), DistanceAtStart, Context.Output.Distance);
    }

    return EStageResult::Success;
}

void UCameraStage_ModuleCompute::OnPreExecute(const FStageExecutionContext& Context)
{
    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("Stage 3 ModuleCompute: PreExecute"));
}

void UCameraStage_ModuleCompute::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("Stage 3 ModuleCompute: PostExecute - Executed: %d, Skipped: %d"),
        ModulesExecutedCount, ModulesSkippedCount);
}

//========================================
// Module Registry Reference 模块注册表引用
//========================================

void UCameraStage_ModuleCompute::SetModuleRegistry(UCameraModuleRegistry* InRegistry)
{
    ModuleRegistry = InRegistry;
    
    if (ModuleRegistry)
    {
        UE_LOG(LogCameraStage_Module, Log, TEXT("Module registry set successfully"));
    }
}

//========================================
// Module Execution 模块执行
//========================================

/**
 * ExecuteModules - Execute all active camera modules
 * 
 * This is the core function of Stage 3. It: 
 * 1. Gets the current state configuration
 * 2. Uses ModuleRegistry to get active modules
 * 3. Executes each module in priority order
 * 4. Collects all outputs into Context.ModuleOutputs
 * 
 * If ModuleRegistry is not available, falls back to basic outputs. 
 * 
 * @param Context Current execution context (will be modified)
 */
void UCameraStage_ModuleCompute::ExecuteModules(FStageExecutionContext& Context)
{
	// Clear previous module outputs
	Context.ModuleOutputs.Empty();

	// ★ 从 Context 获取 Manager 的 Debug 开关
	bool bShouldLogStage3 = false;
	if (Context.Manager)
	{
		bShouldLogStage3 = Context.Manager->IsStage3DebugEnabled();
	}

	//========================================
	// Step 1: Get current state configuration
	//========================================
	
	FCameraStateConfig StateConfig;
	
	if (Context.Manager)
	{
		USoulsCameraStateMachine* StateMachine = Context.Manager->GetStateMachine();
		if (StateMachine)
		{
			StateMachine->GetCurrentStateConfig(StateConfig);
		}
	}

	//========================================
	// Step 2: Check ModuleRegistry availability
	//========================================
	
	UCameraModuleRegistry* Registry = nullptr;
	
	if (Context.Manager)
	{
		Registry = Context.Manager->GetModuleRegistry();
	}
	
	if (!Registry)
	{
		CreateFallbackOutputs(Context, StateConfig);
		return;
	}

	//========================================
	// Step 3: Update module activation states
	//========================================
	
	Registry->UpdateModuleStates(Context, StateConfig);

	//========================================
	// Step 4: Get active modules sorted by priority
	//========================================
	
	TArray<UCameraModuleBase*> ActiveModules;
	Registry->GetActiveModules(Context, StateConfig, ActiveModules);
	
	if (ActiveModules.Num() == 0)
	{
		CreateFallbackOutputs(Context, StateConfig);
		return;
	}

	//========================================
	// Step 5: Execute each active module
	//========================================
	
	for (UCameraModuleBase* Module : ActiveModules)
	{
		if (!Module)
		{
			ModulesSkippedCount++;
			continue;
		}
		
		FModuleOutput Output;
		bool bSuccess = Module->Compute(Context, StateConfig, Output);
		
		if (bSuccess)
		{
			// ★★★ 调试日志：记录 Rotation 模块输出 ★★★
			if (bShouldLogStage3 && Output.bHasRotationOutput)
			{
				UE_LOG(LogCameraStage_Module, Warning, TEXT("★ Stage3: Rotation output from ModuleID %d: Pitch=%.1f, Yaw=%.1f"),
					static_cast<int32>(Output.ModuleID),
					Output.RotationOutput.Pitch,
					Output.RotationOutput.Yaw);
			}
			
			Context.ModuleOutputs.Add(Output);
			ModulesExecutedCount++;
		}
		else
		{
			ModulesSkippedCount++;
		}
	}

	// ★★★ 调试：检查 ModuleOutputs 中是否有 R03 的输出 ★★★
	if (bShouldLogStage3 && Context.InputContext.bHasTarget)
	{
		UE_LOG(LogTemp, Error, TEXT(""));
		UE_LOG(LogTemp, Error, TEXT("★★★ Stage3 Final Check [LOCK-ON] ★★★"));
		UE_LOG(LogTemp, Error, TEXT("  Total ModuleOutputs: %d"), Context.ModuleOutputs.Num());
		
		bool bFoundR03 = false;
		for (int32 i = 0; i < Context.ModuleOutputs.Num(); i++)
		{
			const FModuleOutput& Output = Context.ModuleOutputs[i];
			
			// 检查是否是 R03 的输出 (ModuleID=11, CategoryRotation=2, Pitch != 0)
			if (Output.bHasRotationOutput && 
			    Output.ModuleCategory == ECameraModuleCategory::CategoryRotation &&
			    !FMath::IsNearlyZero(Output.RotationOutput.Pitch, 1.0f))
			{
				bFoundR03 = true;
				UE_LOG(LogTemp, Error, TEXT("  ✓ Found R03-like output at index %d: "), i);
				UE_LOG(LogTemp, Error, TEXT("      ModuleID: %d"), static_cast<int32>(Output.ModuleID));
				UE_LOG(LogTemp, Error, TEXT("      Rotation: Pitch=%.1f, Yaw=%.1f"), 
					Output.RotationOutput.Pitch, Output.RotationOutput.Yaw);
				UE_LOG(LogTemp, Error, TEXT("      Priority: %d, Weight: %.2f"), Output.Priority, Output.Weight);
			}
			
			// 列出所有 Rotation 输出，包括模块类名
			if (Output.bHasRotationOutput)
			{
				UE_LOG(LogTemp, Error, TEXT("  [%d] Rotation:  ModuleID=%d, Pitch=%.1f, Yaw=%.1f, Priority=%d, Category=%d"),
					i, static_cast<int32>(Output.ModuleID),
					Output.RotationOutput.Pitch, Output.RotationOutput.Yaw,
					Output.Priority,
					static_cast<int32>(Output.ModuleCategory));
			}
		}
		
		if (!bFoundR03)
		{
			UE_LOG(LogTemp, Error, TEXT("  ✗ R03 output NOT FOUND in ModuleOutputs!"));
		}
	}
}

void UCameraStage_ModuleCompute::ExecuteModuleCategory(FStageExecutionContext& Context, ECameraModuleCategory Category)
{
    // Will be fully implemented in Phase 3
    // This method will execute all active modules of a specific category
    
    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("ExecuteModuleCategory: Category %d (placeholder)"), 
        static_cast<int32>(Category));
}

bool UCameraStage_ModuleCompute::ExecuteSingleModule(FStageExecutionContext& Context, UCameraModuleBase* Module)
{
    // Will be fully implemented in Phase 3
    // This method will execute a single module and collect its output
    
    if (!Module)
    {
        return false;
    }

    // Placeholder: In Phase 3, this will call Module->Compute(Context, Output)
    ModulesExecutedCount++;
    return true;
}

//========================================
// Placeholder Methods 占位方法
//========================================

void UCameraStage_ModuleCompute::ExecutePositionModules(FStageExecutionContext& Context)
{
    // Placeholder: Create basic position output from state config
    FModuleOutput PositionOutput;
    PositionOutput.ModuleCategory = ECameraModuleCategory::CategoryPosition;
    PositionOutput.Priority = 100;
    PositionOutput.BlendPolicy = EBlendPolicy::Override;
    PositionOutput.Weight = 1.0f;
    PositionOutput.PositionOffset = Context.Output.FocusPoint;
    PositionOutput.bHasPositionOutput = true;
    
    Context.ModuleOutputs.Add(PositionOutput);
    ModulesExecutedCount++;

    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("ExecutePositionModules: Placeholder output added"));
}

void UCameraStage_ModuleCompute::ExecuteRotationModules(FStageExecutionContext& Context)
{
    // Placeholder: Create basic rotation output
    // In exploration mode, use player input; in lock-on mode, look at target
    
    FModuleOutput RotationOutput;
    RotationOutput.ModuleCategory = ECameraModuleCategory::CategoryRotation;
    RotationOutput.Priority = 100;
    RotationOutput.BlendPolicy = EBlendPolicy::Override;
    RotationOutput.Weight = 1.0f;

    if (Context.InputContext.bHasTarget)
    {
        // Lock-on: Calculate rotation to look at target
        const FVector DirectionToTarget = Context.InputContext.GetDirectionToTarget();
        if (!DirectionToTarget.IsNearlyZero())
        {
            RotationOutput.RotationOutput = DirectionToTarget.Rotation();
        }
        else
        {
            RotationOutput.RotationOutput = Context.Output.Rotation;
        }
    }
    else
    {
        // Free exploration: Use current rotation (will be modified by player input)
        RotationOutput.RotationOutput = Context.Output.Rotation;
    }
    
    RotationOutput.bHasRotationOutput = true;
    Context.ModuleOutputs.Add(RotationOutput);
    ModulesExecutedCount++;

    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("ExecuteRotationModules: Placeholder output added (HasTarget: %s)"),
        Context.InputContext.bHasTarget ? TEXT("Yes") : TEXT("No"));
}

void UCameraStage_ModuleCompute::ExecuteDistanceModules(FStageExecutionContext& Context)
{
    // Placeholder: Create basic distance output from state config
    FModuleOutput DistanceOutput;
    DistanceOutput.ModuleCategory = ECameraModuleCategory::CategoryDistance;
    DistanceOutput.Priority = 100;
    DistanceOutput.BlendPolicy = EBlendPolicy::Override;
    DistanceOutput.Weight = 1.0f;
    DistanceOutput.DistanceOutput = Context.Output.Distance;
    DistanceOutput.bHasDistanceOutput = true;
    DistanceOutput.bDistanceIsMultiplier = false;
    
    Context.ModuleOutputs.Add(DistanceOutput);
    ModulesExecutedCount++;

    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("ExecuteDistanceModules: Placeholder output added (Distance: %.1f)"),
        Context.Output.Distance);
}

void UCameraStage_ModuleCompute::ExecuteFOVModules(FStageExecutionContext& Context)
{
    // Placeholder: Create basic FOV output from state config
    FModuleOutput FOVOutput;
    FOVOutput.ModuleCategory = ECameraModuleCategory::CategoryFOV;
    FOVOutput.Priority = 100;
    FOVOutput.BlendPolicy = EBlendPolicy::Override;
    FOVOutput.Weight = 1.0f;
    FOVOutput.FOVOutput = Context.Output.FOV;
    FOVOutput.bHasFOVOutput = true;
    
    Context.ModuleOutputs.Add(FOVOutput);
    ModulesExecutedCount++;

    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("ExecuteFOVModules: Placeholder output added (FOV: %.1f)"),
        Context.Output.FOV);
}

void UCameraStage_ModuleCompute::ExecuteOffsetModules(FStageExecutionContext& Context)
{
    // Placeholder: Create basic offset output from state config
    FModuleOutput OffsetOutput;
    OffsetOutput.ModuleCategory = ECameraModuleCategory::CategoryOffset;
    OffsetOutput.Priority = 100;
    OffsetOutput.BlendPolicy = EBlendPolicy::Override;
    OffsetOutput.Weight = 1.0f;
    OffsetOutput.SocketOffsetOutput = Context.Output.SocketOffset;
    OffsetOutput.bHasSocketOffsetOutput = true;
    
    Context.ModuleOutputs.Add(OffsetOutput);
    ModulesExecutedCount++;

    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("ExecuteOffsetModules: Placeholder output added"));
}

void UCameraStage_ModuleCompute::ExecuteConstraintModules(FStageExecutionContext& Context)
{
    // Placeholder: Constraint modules don't produce output directly
    // They modify other outputs in Phase 3
    // For now, just log that we would execute them
    
    UE_LOG(LogCameraStage_Module, VeryVerbose, TEXT("ExecuteConstraintModules: Placeholder (will apply constraints in Phase 3)"));
    
    // No output added - constraints modify existing values
    // ModulesExecutedCount++; // Don't count since no output produced
}

//========================================
// Fallback Outputs
//========================================

/**
 * CreateFallbackOutputs - Create basic outputs when ModuleRegistry is unavailable
 * 
 * This function provides minimal camera functionality when the module
 * system is not properly initialized. It creates basic outputs for
 * position, rotation, distance, and FOV based on state config values.
 * 
 * Use Cases:
 * - ModuleRegistry not yet initialized
 * - Error during module system setup
 * - Testing without full module system
 * 
 * @param Context Current execution context (will be modified)
 * @param StateConfig Current camera state configuration
 */
void UCameraStage_ModuleCompute::CreateFallbackOutputs(
	FStageExecutionContext& Context, 
	const FCameraStateConfig& StateConfig)
{
	//========================================
	// Position Output (P01 equivalent)
	//========================================
	
	FModuleOutput PositionOutput;
	PositionOutput.ModuleCategory = ECameraModuleCategory::CategoryPosition;
	PositionOutput.Priority = 100;
	PositionOutput.BlendPolicy = EBlendPolicy::Override;
	PositionOutput.Weight = 1.0f;
	
	// Calculate focus point: character location + configured offsets
	FVector FocusPoint = Context.InputContext.CharacterLocation;
	
	// Add focus offset from config
	FVector FocusOffset = StateConfig.StateBase.Offset.FocusOffset;
	FocusPoint += FocusOffset;
	
	// Add vertical offset
	float VerticalOffset = StateConfig.StateBase.Offset.VerticalOffset;
	FocusPoint.Z += VerticalOffset;
	
	PositionOutput.PositionOffset = FocusPoint;
	PositionOutput.bHasPositionOutput = true;
	
	Context.ModuleOutputs.Add(PositionOutput);


	//========================================
	// Rotation Output (R01 equivalent)
	//========================================
	
	FModuleOutput RotationOutput;
	RotationOutput.ModuleCategory = ECameraModuleCategory::CategoryRotation;
	RotationOutput.Priority = 100;
	RotationOutput.BlendPolicy = EBlendPolicy::Override;
	RotationOutput.Weight = 1.0f;
	
	// Use previous camera rotation to maintain continuity
	// This prevents camera from snapping to a default orientation
	RotationOutput.RotationOutput = Context.InputContext.PreviousCameraRotation;
	RotationOutput.bHasRotationOutput = true;
	
	Context.ModuleOutputs.Add(RotationOutput);


	//========================================
	// Distance Output (D01 equivalent)
	//========================================
	
	FModuleOutput DistanceOutput;
	DistanceOutput.ModuleCategory = ECameraModuleCategory::CategoryDistance;
	DistanceOutput.Priority = 100;
	DistanceOutput.BlendPolicy = EBlendPolicy::Override;
	DistanceOutput.Weight = 1.0f;
	
	// Get base distance from config
	float BaseDistance = StateConfig.StateBase.Distance.BaseDistance;
	
	// Use default if not configured
	if (BaseDistance <= 0.0f)
	{
		BaseDistance = 400.0f;  // Default: 4 meters
	}
	
	DistanceOutput.DistanceOutput = BaseDistance;
	DistanceOutput.bHasDistanceOutput = true;
	
	Context.ModuleOutputs.Add(DistanceOutput);


	//========================================
	// FOV Output (F01 equivalent)
	//========================================
	
	FModuleOutput FOVOutput;
	FOVOutput.ModuleCategory = ECameraModuleCategory::CategoryFOV;
	FOVOutput.Priority = 100;
	FOVOutput.BlendPolicy = EBlendPolicy::Override;
	FOVOutput.Weight = 1.0f;
	
	// Get base FOV from config
	float BaseFOV = StateConfig.StateBase.FOV.BaseFOV;
	
	// Use default if not configured
	if (BaseFOV <= 0.0f)
	{
		BaseFOV = 90.0f;  // Default: 90 degrees
	}
	
	FOVOutput.FOVOutput = BaseFOV;
	FOVOutput.bHasFOVOutput = true;
	
	Context.ModuleOutputs.Add(FOVOutput);


	UE_LOG(LogCameraStage_Module, Verbose, TEXT("CreateFallbackOutputs: Created 4 fallback outputs (Position, Rotation, Distance, FOV)"));
}
