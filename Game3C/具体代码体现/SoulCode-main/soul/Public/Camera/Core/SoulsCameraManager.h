// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraConfig.h"
#include "SoulsCameraRuntimeTypes.h"
#include "Camera/Config/CameraConfigService.h"
#include "SoulsCameraManager.generated.h"

// Forward declarations - UE4 Engine classes
class USpringArmComponent;
class UCameraComponent;
class USoulsCameraGlobalConfig;

// Forward declarations - Sub-system classes
class USoulsCameraPipeline;
class USoulsCameraStateMachine;
class UCameraModuleRegistry;
class UCameraModifierManager;
class UCameraCollisionResolver;
class USoulsCameraDebugger;

// Forward declarations - Stage classes
class UCameraStage_ModifierApply;

// Forward declarations - Config classes
class UCameraConfigService;
class UDefaultCameraParams;

/**
 * USoulsCameraManager
 * 
 * Main controller for the Souls-like camera system.
 * Attach this component to your player character to enable the camera system.
 * 
 * Features:
 * - 725 configurable camera states via DataTable
 * - 39 modular camera calculation modules
 * - 26 event-triggered camera modifiers
 * - 20 collision handling strategies
 * - State-based camera transitions with blending
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent), Config=Game)
class SOUL_API USoulsCameraManager : public UActorComponent
{
	GENERATED_BODY()

public:
	USoulsCameraManager();

	//========================================
	// UActorComponent Interface
	//========================================
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//========================================
	// Initialization / 初始化
	//========================================
	
	/** Manual initialization (called automatically in BeginPlay if AutoInitialize is true) */
	UFUNCTION(BlueprintCallable, Category = "Camera|Setup")
	void InitializeCamera();

	/** Check if camera system is initialized */
	UFUNCTION(BlueprintPure, Category = "Camera|Setup")
	bool IsInitialized() const;

	/**
	 * Initialize camera system with new ConfigService architecture
	 * 使用新的ConfigService架构初始化相机系统
	 * 
	 * Call this after setting StatesDataTable and OverridesDataTable. 
	 * 在设置StatesDataTable和Overrides数据表后调用此方法。
	 * 
	 * @return True if initialization successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Init")
	bool InitializeWithConfigService();

	/**
	 * Create and setup the ConfigService
	 * 创建并设置ConfigService
	 * 
	 * @return True if ConfigService was created successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Init")
	bool CreateConfigService();

	/**
	 * Check if using new config architecture
	 * 检查是否使用新配置架构
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Config")
	bool IsUsingNewConfigArchitecture() const { return bUseNewConfigArchitecture && ConfigService != nullptr; }

	//========================================
	// Component References
	//========================================
	
	/** Set the SpringArm component to control */
	UFUNCTION(BlueprintCallable, Category = "Camera|Setup")
	void SetSpringArmComponent(USpringArmComponent* InSpringArm);

	/** Set the Camera component to control */
	UFUNCTION(BlueprintCallable, Category = "Camera|Setup")
	void SetCameraComponent(UCameraComponent* InCamera);

	/** Get the controlled SpringArm */
	UFUNCTION(BlueprintPure, Category = "Camera|Setup")
	USpringArmComponent* GetSpringArm() const;

	/** Get the controlled Camera */
	UFUNCTION(BlueprintPure, Category = "Camera|Setup")
	UCameraComponent* GetCamera() const;

	//========================================
	// State Management
	//========================================
	
	/** Request a camera state change */
	UFUNCTION(BlueprintCallable, Category = "Camera|State")
	bool RequestStateChange(FName NewStateName, bool bForce = false);

	/** Check if a manual (non-automatic) camera state is currently active */
	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsManualStateActive() const { return bManualStateActive; }

	/** Get current state name */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	FName GetCurrentStateName() const;

	/** Get current state category */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	ECameraStateCategory GetCurrentCategory() const;

	/** Check if in transition */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	bool IsInTransition() const;

	//========================================
	// Modifier Triggers (Event Interface)
	//========================================
	
	/** Trigger a camera modifier by type */
	UFUNCTION(BlueprintCallable, Category = "Camera|Modifier")
	void TriggerModifier(ECameraModifierType ModifierType, float Intensity = 1.0f);

	/** Stop a specific modifier */
	UFUNCTION(BlueprintCallable, Category = "Camera|Modifier")
	void StopModifier(ECameraModifierType ModifierType);

	/** Stop all active modifiers */
	UFUNCTION(BlueprintCallable, Category = "Camera|Modifier")
	void StopAllModifiers();

	//========================================
	// Modifier System Convenience Interface
	//========================================

	/**
	 * Trigger a camera shake effect
	 * @param ShakeID The shake modifier to trigger (S01-S05)
	 * @param Intensity Shake intensity (default 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Effects")
	void TriggerCameraShake(ECameraModifierID ShakeID, float Intensity = 1.0f);

	/**
	 * Trigger a hit reaction effect
	 * @param bHeavyHit True for heavy hit, false for light hit
	 * @param Intensity Reaction intensity (default 1.0)
	 * @param HitDirection Direction of the hit (optional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Effects")
	void TriggerHitReaction(bool bHeavyHit, float Intensity = 1.0f, FVector HitDirection = FVector::ZeroVector);

	/**
	 * Trigger slow motion effect
	 * @param Duration Duration in seconds
	 * @param TimeDilation Time scale (0.3 = 30% speed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Effects")
	void TriggerSlowMotion(float Duration = 0.8f, float TimeDilation = 0.35f);

	/**
	 * Trigger hit stop (freeze frame) effect
	 * @param Intensity Hit intensity (affects freeze duration)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Effects")
	void TriggerHitStop(float Intensity = 1.0f);

	/**
	 * Trigger low health visual effect
	 * @param HealthPercent Current health percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Effects")
	void TriggerLowHealthEffect(float HealthPercent);

	/**
	 * Stop low health effect (when health is restored)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Effects")
	void StopLowHealthEffect();

	/**
	 * Trigger death camera
	 * @param DeathLocation Location where player died
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Effects")
	void TriggerDeathCamera(FVector DeathLocation);

	//========================================
	// Target Management (Lock-On)
	//========================================
	
	/** Set the current lock-on target */
	UFUNCTION(BlueprintCallable, Category = "Camera|Target")
	void SetLockOnTarget(AActor* Target);

	/**
	 * 状态切换时的初始化回调
	 * 保存当前相机参数用于平滑过渡
	 * 
	 * @param OldState 旧状态名称
	 * @param NewState 新状态名称
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|State")
	void OnCameraStateChanged(FName OldState, FName NewState);

	/** Clear the lock-on target */
	UFUNCTION(BlueprintCallable, Category = "Camera|Target")
	void ClearLockOnTarget();

	/** 
	 * 恢复原生 SpringArm 设置
	 * 当从特殊状态（Edge、LockOn）退回到普通探索状态时调用
	 * Restore native SpringArm settings when exiting special states
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Target")
	void RestoreNativeSpringArmSettings();

	/** Get current lock-on target */
	UFUNCTION(BlueprintPure, Category = "Camera|Target")
	AActor* GetLockOnTarget() const;

	/** Check if has lock-on target */
	UFUNCTION(BlueprintPure, Category = "Camera|Target")
	bool HasLockOnTarget() const;

	//========================================
	// Output Access
	//========================================
	
	/** Get current camera output */
	UFUNCTION(BlueprintPure, Category = "Camera|Output")
	FSoulsCameraOutput GetCurrentOutput() const;

	/** Get current input context */
	UFUNCTION(BlueprintPure, Category = "Camera|Output")
	FCameraInputContext GetCurrentInputContext() const;

	/** 
	 * Get current frame counter 
	 * Note: C++ only, uint64 not supported by Blueprint
	 */
	uint64 GetFrameCounter() const { return FrameCounter; }

	//========================================
	// Convenience Query Functions
	// These functions provide convenient access to CurrentOutput data
	//========================================

	/** Get current camera world location */
	UFUNCTION(BlueprintPure, Category = "Camera|Query")
	FVector GetCurrentCameraLocation() const;

	/** Get current camera rotation */
	UFUNCTION(BlueprintPure, Category = "Camera|Query")
	FRotator GetCurrentCameraRotation() const;

	/** Get current focus point (where camera looks at) */
	UFUNCTION(BlueprintPure, Category = "Camera|Query")
	FVector GetCurrentFocusPoint() const;

	/** Get current camera distance from focus point */
	UFUNCTION(BlueprintPure, Category = "Camera|Query")
	float GetCurrentDistance() const;

	/** Get current field of view */
	UFUNCTION(BlueprintPure, Category = "Camera|Query")
	float GetCurrentFOV() const;

	/** Get formatted debug information string */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	FString GetDebugString() const;

	//========================================
	// Configuration Access / 配置访问
	//========================================

	/**
	 * Get ConfigService reference
	 * 获取ConfigService引用
	 * 
	 * @return ConfigService instance, or nullptr if not using new architecture
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Config")
	UCameraConfigService* GetConfigService() const { return ConfigService; }

	/**
	 * Get camera state configuration by name
	 * 根据名称获取相机状态配置
	 * 
	 * Works with both legacy and new architecture. 
	 * 同时支持旧架构和新架构。
	 * 
	 * @param StateName Name of the camera state
	 * @param OutConfig Output configuration
	 * @return True if state was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Config")
	bool GetCameraStateConfig(FName StateName, FCameraStateConfig& OutConfig) const;

	/**
	 * Check if a camera state exists
	 * 检查相机状态是否存在
	 * 
	 * @param StateName Name of the state to check
	 * @return True if state exists
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Config")
	bool DoesCameraStateExist(FName StateName) const;

	/**
	 * Get all available camera state names
	 * 获取所有可用的相机状态名称
	 * 
	 * @return Array of state names
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Config")
	TArray<FName> GetAllCameraStateNames() const;

	//========================================
	// Debug / 调试
	//========================================
	
	/** Enable/disable debug visualization */
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug")
void SetDebugEnabled(bool bEnabled);

	/** Check if debug is enabled */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	bool IsDebugEnabled() const;

	//========================================
	// Debug Accessors / 调试访问器
	//========================================

	/** Check if collision debug is enabled */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	bool IsCollisionDebugEnabled() const;

	/** Check if state machine debug is enabled */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	bool IsStateMachineDebugEnabled() const;

	/** Check if pipeline debug is enabled */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	bool IsPipelineDebugEnabled() const;

	/** Check if module debug is enabled */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	bool IsModuleDebugEnabled() const;

	/** Set all debug flags at once */
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug")
	void SetAllDebugEnabled(bool bEnabled);

	// ★ 扩展 Debug LOG 开关（运行时可用）
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug")
	bool IsR03DebugEnabled() const { return bDebugR03Module; }
	
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug")
	bool IsBlendSolveDebugEnabled() const { return bDebugBlendSolve; }
	
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug")
	bool IsStage3DebugEnabled() const { return bDebugStage3ModuleCompute; }

	/**
	 * Log configuration statistics
	 * 记录配置统计信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug", meta = (DevelopmentOnly))
	void LogConfigStatistics() const;

	/**
	 * Reload all configuration data
	 * 重新加载所有配置数据
	 * 
	 * Useful for hot-reloading during development. 
	 * 在开发期间用于热重载很有用。
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug", meta = (DevelopmentOnly))
	void ReloadConfigData();

	//========================================
	// Sub-System Access
	//========================================
	
	/** Get the state machine (returns nullptr until sub-systems are implemented) */
	USoulsCameraStateMachine* GetStateMachine() const;

	/** 
	 * Get the module registry
	 * @return Module registry instance, or nullptr if not yet created
	 */
	UCameraModuleRegistry* GetModuleRegistry() const;

	/** 
	 * Get the modifier manager
	 * @return Modifier manager instance, or nullptr if not yet created
	 */
	UCameraModifierManager* GetModifierManager() const;

	/**
	 * Get the collision resolver
	 * @return Collision resolver instance, or nullptr if not yet created
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Collision")
	UCameraCollisionResolver* GetCollisionResolver() const;

	//========================================
	// Collision System Interface
	//========================================

	/**
	 * Check if collision is currently active (camera pulled in due to obstacle)
	 * @return True if camera is in collision state
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Collision")
	bool IsCollisionActive() const;

	/**
	 * Check if camera is in recovery mode (returning to normal after collision)
	 * @return True if recovering from collision
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Collision")
	bool IsInCollisionRecovery() const;

	/**
	 * Check if character is currently occluded from camera view
	 * @return True if character is occluded
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Collision")
	bool IsCharacterOccluded() const;

	/**
	 * Check if lock-on target is currently occluded from camera view
	 * @return True if target is occluded
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Collision")
	bool IsTargetOccluded() const;

	/**
	 * Get current collision-adjusted camera distance
	 * @return Current safe distance (adjusted for collision)
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Collision")
	float GetCollisionAdjustedDistance() const;

	/**
	 * Enable or disable a specific collision strategy
	 * @param StrategyType Strategy to enable/disable
	 * @param bEnabled Whether to enable or disable
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
	void SetCollisionStrategyEnabled(ECollisionStrategyID StrategyType, bool bEnabled);

	/**
	 * Enable or disable collision debug visualization
	 * @param bEnabled Whether to enable debug drawing
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Collision|Debug")
	void SetCollisionDebugEnabled(bool bEnabled);

protected:
	//========================================
	// Configuration (Editable)
	//========================================
	
	/** Global camera configuration asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	USoulsCameraGlobalConfig* GlobalConfig;

	/** Camera states DataTable (Legacy - used when bUseNewConfigArchitecture is false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	UDataTable* StatesDataTable;

	/** Auto-find SpringArm and Camera components on owner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bAutoFindComponents;

	/** Auto-initialize on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bAutoInitialize;

	/** 
	 * [DEPRECATED - Hybrid Mode C] 
	 * Native SpringArm collision is now always enabled for basic pull-in. 
	 * Custom CollisionResolver handles advanced features (Slide, Orbit, FOV, Occlusion, Special).
	 * 
	 * [已弃用 - 混合模式C]
	 * 原生 SpringArm 碰撞现在总是启用以处理基础拉近。
	 * 自定义 CollisionResolver 处理高级功能（滑动、绕行、FOV、遮挡、特殊场景）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (DeprecatedProperty))
	bool bDisableNativeCollision;

	//========================================
	// DataTable References / 数据表引用
	//========================================

	/**
	 * DataTable containing parameter overrides (sparse)
	 * 包含参数覆盖的DataTable（稀疏)
	 * 
	 * Row struct: FCameraParamOverrideRow
	 * Asset path example: /Game/Data/Camera/DT_CameraOverrides
	 * 
	 * Note: This is optional. If null, only default values will be used.
	 * 注意：这是可选的。如果为空，将只使用默认值。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Config")
	UDataTable* OverridesDataTable;

	/**
	 * Whether to use the new ConfigService architecture
	 * 是否使用新的ConfigService架构
	 * 
	 * If true:  Uses ConfigService with DefaultParams + Overrides
	 * If false: Uses legacy single DataTable approach
	 * 
	 * 如果为true：使用ConfigService配合默认参数+覆盖
	 * 如果为false：使用旧的单一DataTable方式
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Config")
	bool bUseNewConfigArchitecture;

	//========================================
	// Services / 服务
	//========================================

	/**
	 * Configuration service instance (new architecture)
	 * 配置服务实例（新架构）
	 * 
	 * Created and initialized when bUseNewConfigArchitecture is true. 
	 * 当bUseNewConfigArchitecture为true时创建并初始化。
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Config")
	UCameraConfigService* ConfigService;

	//========================================
	// Component References
	//========================================
	
	/** Controlled SpringArm component */
	UPROPERTY()
	USpringArmComponent* SpringArmComponent;

	/** Controlled Camera component */
	UPROPERTY()
	UCameraComponent* CameraComponent;

	//========================================
	// Sub-Systems (UPROPERTY managed for proper GC)
	//========================================
	
	/** Camera pipeline (executes all stages) */
	UPROPERTY()
	USoulsCameraPipeline* Pipeline;

	/** State machine (manages 725 states) */
	UPROPERTY()
	USoulsCameraStateMachine* StateMachine;

	/** Module registry (39 modules) */
	UPROPERTY()
	UCameraModuleRegistry* ModuleRegistry;

	/** Modifier manager (26 modifiers) */
	UPROPERTY()
	UCameraModifierManager* ModifierManager;

	/** Collision resolver (20 strategies) */
	UPROPERTY()
	UCameraCollisionResolver* CollisionResolver;

	/** Debugger (optional) */
	UPROPERTY()
	USoulsCameraDebugger* Debugger;

	/** Cached reference to Stage 4 for modifier connection */
	UPROPERTY()
	UCameraStage_ModifierApply* Stage4_ModifierApply;

	//========================================
	// Runtime State
	//========================================
	
	/** Current input context */
	FCameraInputContext CurrentInputContext;

	/** Current camera output */
	FSoulsCameraOutput CurrentOutput;

	/** Current lock-on target */
	TWeakObjectPtr<AActor> LockOnTarget;

	/** Is camera system initialized */
	UPROPERTY()
	bool bIsInitialized;

	/** Is debug visualization enabled */
	UPROPERTY()
	bool bDebugEnabled;

	//========================================
	// Frame Tracking
	//========================================
	
	/** Frame counter for debugging and timing */
	UPROPERTY()
	uint64 FrameCounter;

	//========================================
	// Previous Frame Data (for interpolation)
	//========================================
	
	/** Previous frame focus point */
	FVector PreviousFocusPoint;

	/** Previous frame rotation */
	FRotator PreviousRotation;

	/** Previous frame distance */
	float PreviousDistance;

	/** Previous frame FOV */
	float PreviousFOV;

	//========================================
	// Original SpringArm LAG Settings (for unlock restoration)
	// ★ 原始 SpringArm LAG 设置（用于解锁后恢复）
	//========================================
	
	UPROPERTY()
	bool bOriginalEnableCameraLag;
	
	UPROPERTY()
	float OriginalCameraLagSpeed;
	
	UPROPERTY()
	bool bOriginalEnableCameraRotationLag;
	
	UPROPERTY()
	float OriginalCameraRotationLagSpeed;
	
	UPROPERTY()
	float OriginalCameraLagMaxDistance;
	
	UPROPERTY()
	bool bHasSavedOriginalLagSettings;

	/** ★ 锁定状态专用的 Rotation LAG 速度，独立于探索状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|LockOn", meta = (ClampMin = "0.1", ClampMax = "20.0"))
	float LockOnRotationLagSpeed = 3.0f;

	// ========== Linear Transition Settings 线性过渡设置 ==========
	
	/** FOV 过渡时长（秒），数值越大过渡越慢 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Linear Transition", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float FOVTransitionDuration = 0.8f;
	
	/** Distance 过渡时长（秒），数值越大过渡越慢 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Linear Transition", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float DistanceTransitionDuration = 0.8f;

	/** SocketOffset 过渡时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Linear Transition", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float SocketOffsetTransitionDuration = 0.5f;
	
	/** TargetOffset 过渡时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Linear Transition", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float TargetOffsetTransitionDuration = 0.5f;
	
	/** Pitch 过渡时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Linear Transition", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float PitchTransitionDuration = 0.5f;
	
	/** 是否启用过渡调试日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Linear Transition")
	bool bEnableTransitionDebugLog = true;

	// ★ 扩展 Debug LOG 开关（运行时可用）
	// 用于统一管理相机系统各模块的调试日志
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Logs")
	bool bDebugR03Module = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Logs")
	bool bDebugBlendSolve = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Logs")
	bool bDebugStage3ModuleCompute = false;

	// ★ 运行时状态机 Debug 开关（蓝图可访问）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Logs")
	bool bDebugStateMachineRuntime = false;

	//========================================
	// Internal Initialization / 内部初始化
	//========================================

	/**
	 * Initialize using legacy DataTable approach
	 * 使用旧的DataTable方式初始化
	 */
	void InitializeLegacy();

	/**
	 * Initialize using new ConfigService approach
	 * 使用新的ConfigService方式初始化
	 */
	void InitializeNewArchitecture();

	//========================================
	// Internal Methods
	//========================================
	
	/** Find SpringArm and Camera on owner actor */
	void AutoFindComponents();

	/** Create and initialize all sub-systems */
	void CreateSubSystems();

	/** Gather input context for this frame */
	void GatherInputContext(float DeltaTime);

	/** Apply output to SpringArm and Camera */
	void ApplyOutput(const FSoulsCameraOutput& Output);

	/** Store current frame data for next frame interpolation */
	void StorePreviousFrameData();

	/** Apply fallback camera parameters when Pipeline fails */
	void ApplyFallbackParameters();

	/** Apply Pipeline output to SpringArm and Camera components */
	void ApplyOutputToComponents();

	/**
	 * 退出过渡辅助函数（问题5修复）
	 * 在离开特殊状态（Edge/LockOn）后执行平滑过渡
	 * Apply exit transition helper function (Issue 5 fix)
	 * Executes smooth transition after leaving special states
	 * 
	 * @param DeltaTime Frame delta time
	 */
	void ApplyExitTransition(float DeltaTime);

	/** 
	 * 安全地从正确的 InputComponent 读取输入轴值
	 * Safely read input axis value from the correct InputComponent
	 * 
	 * @param AxisName Name of the input axis to read
	 * @return The axis value, or 0 if not found
	 */
	float GetInputAxisValueSafe(const FName& AxisName) const;

	//========================================
	// Pipeline & Stage Creation
	//========================================

	/** 
	 * Create and register all pipeline stages
	 */
	void CreateAndRegisterStages();

	//========================================
	// Debug Methods
	//========================================

	//========================================
	// Debug Settings / 调试设置
	// 所有调试功能仅在编辑器中可用，打包后不占用内存
	//========================================

	/** Show debug info on screen (visual overlay) */
	UPROPERTY(EditAnywhere, Category = "Debug|Visual")
	bool bShowDebugInfo = true;

	/** Enable collision system debug logs */
	UPROPERTY(EditAnywhere, Category = "Debug|Logs")
	bool bDebugCollision;
	
	/** Enable state machine debug logs */
	UPROPERTY(EditAnywhere, Category = "Debug|Logs")
	bool bDebugStateMachine;
	
	/** Enable pipeline execution debug logs */
	UPROPERTY(EditAnywhere, Category = "Debug|Logs")
	bool bDebugPipeline;
	
	/** Enable module computation debug logs */
	UPROPERTY(EditAnywhere, Category = "Debug|Logs")
	bool bDebugModules;
	
	/** Enable all debug logs at once */
	UPROPERTY(EditAnywhere, Category = "Debug|Logs")
	bool bDebugAll;

#if WITH_EDITOR
	/** Draw debug visualization */
	void DrawDebugInfo();
#endif

private:
	// ========== Linear Transition Runtime 线性过渡运行时数据 ==========
	
	// FOV 过渡数据
	float FOVTransitionStartValue = 0.0f;      // 过渡起始 FOV
	float FOVTransitionTargetValue = 0.0f;     // 过渡目标 FOV
	float FOVTransitionElapsedTime = 0.0f;     // 已过渡时间
	bool bFOVTransitionActive = false;         // 是否正在过渡
	
	// Distance 过渡数据
	float DistanceTransitionStartValue = 0.0f;   // 过渡起始 Distance
	float DistanceTransitionTargetValue = 0.0f;  // 过渡目标 Distance
	float DistanceTransitionElapsedTime = 0.0f;  // 已过渡时间
	bool bDistanceTransitionActive = false;      // 是否正在过渡
	
	// SocketOffset 过渡数据
	FVector SocketOffsetTransitionStartValue = FVector::ZeroVector;
	FVector SocketOffsetTransitionTargetValue = FVector::ZeroVector;
	float SocketOffsetTransitionElapsedTime = 0.0f;
	bool bSocketOffsetTransitionActive = false;
	FVector CurrentSmoothedSocketOffset = FVector::ZeroVector;
	
	// TargetOffset 过渡数据
	FVector TargetOffsetTransitionStartValue = FVector::ZeroVector;
	FVector TargetOffsetTransitionTargetValue = FVector::ZeroVector;
	float TargetOffsetTransitionElapsedTime = 0.0f;
	bool bTargetOffsetTransitionActive = false;
	FVector CurrentSmoothedTargetOffset = FVector::ZeroVector;
	
	// Pitch 过渡数据
	float PitchTransitionStartValue = 0.0f;
	float PitchTransitionTargetValue = 0.0f;
	float PitchTransitionElapsedTime = 0.0f;
	bool bPitchTransitionActive = false;
	float CurrentSmoothedPitch = 0.0f;
	bool bHasPitchInitialized = false;
	
	// 当前平滑值
	float CurrentSmoothedFOV = 0.0f;
	float CurrentSmoothedDistance = 0.0f;
	bool bHasInitializedTransitionValues = false;

	// ========== Special State Tracking 特殊状态追踪 ==========
	
	/** 记录上一帧是否在特殊状态（Edge 或 LockOn），用于检测退出时恢复原生设置 */
	bool bWasInSpecialState = false;

	/** Whether a manual state was requested via RequestStateChange from Blueprint/external code */
	UPROPERTY()
	bool bManualStateActive = false;

	/** The name of the manually requested state */
	UPROPERTY()
	FName ManualStateName = NAME_None;
};
