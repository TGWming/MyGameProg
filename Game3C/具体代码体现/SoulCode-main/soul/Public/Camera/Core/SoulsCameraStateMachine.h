// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Config/CameraConfigService.h"
#include "SoulsCameraConfig.h"
#include "SoulsCameraRuntimeTypes.h"
#include "SoulsCameraStateMachine.generated.h"

// Forward Declarations
class UCameraConfigService;

/**
 * USoulsCameraStateMachine
 * 
 * Manages camera state transitions and configuration lookup.
 * Handles the 725 camera states defined in the DataTable.
 * 
 * Key Responsibilities:
 * - State initialization from DataTable
 * - State transition management with blending
 * - Configuration lookup and caching
 * - State evaluation based on input context
 * - Event broadcasting for state changes
 */
UCLASS(BlueprintType)
class SOUL_API USoulsCameraStateMachine : public UObject
{
	GENERATED_BODY()

public:
	USoulsCameraStateMachine();

	//========================================
	// Initialization
	//========================================
	
	/** 
	 * Initialize with DataTable reference (legacy architecture)
	 * 使用DataTable引用初始化（旧架构）
	 * 
	 * @param InStatesDataTable DataTable containing FCameraStateConfig rows
	 * @deprecated Use InitializeWithConfigService instead for new architecture
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|StateMachine")
	void Initialize(UDataTable* InStatesDataTable);

	/**
	 * Initialize with ConfigService (new architecture)
	 * 使用ConfigService初始化（新架构）
	 * 
	 * @param InConfigService The configuration service instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|StateMachine")
	void InitializeWithConfigService(UCameraConfigService* InConfigService);

	/** 
	 * Check if initialized and ready
	 * @return True if state machine is properly initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	bool IsInitialized() const;

	//========================================
	// State Management
	//========================================
	
	/** 
	 * Request a state change
	 * @param NewStateName Name of the state to transition to
	 * @param bForce If true, skip transition validation checks
	 * @return True if state change was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|StateMachine")
	bool RequestStateChange(FName NewStateName, bool bForce = false);

	/** 
	 * Get current state name
	 * @return Name of the currently active state
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	FName GetCurrentStateName() const;

	/** 
	 * Get previous state name
	 * @return Name of the previous state (for blending purposes)
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	FName GetPreviousStateName() const;

	/** 
	 * Get current state configuration
	 * @param OutConfig Configuration struct to fill
	 * @return True if current state config was found
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	bool GetCurrentStateConfig(FCameraStateConfig& OutConfig) const;

	/** 
	 * Get configuration for any state by name
	 * @param StateName Name of the state to look up
	 * @param OutConfig Configuration struct to fill
	 * @return True if state config was found
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	bool GetStateConfig(FName StateName, FCameraStateConfig& OutConfig) const;

	/** 
	 * Check if a state exists
	 * @param StateName Name of the state to check
	 * @return True if state exists in the DataTable
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	bool DoesStateExist(FName StateName) const;

	//========================================
	// State Query
	//========================================
	
	/** 
	 * Get all states in a category
	 * @param Category The category to filter by
	 * @return Array of state names in the specified category
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	TArray<FName> GetStatesInCategory(ECameraStateCategory Category) const;

	/** 
	 * Get current state category
	 * @return Category of the current state
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	ECameraStateCategory GetCurrentCategory() const;

	/** 
	 * Check if currently in specified category
	 * @param Category Category to check against
	 * @return True if current state is in the specified category
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	bool IsInCategory(ECameraStateCategory Category) const;

	//========================================
	// Transition
	//========================================
	
	/** 
	 * Update transition (called each frame)
	 * @param DeltaTime Frame delta time in seconds
	 */
	void UpdateTransition(float DeltaTime);

	/** 
	 * Get current blend alpha (0=previous state, 1=current state)
	 * @return Blend alpha value between 0 and 1
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	float GetBlendAlpha() const;

	/** 
	 * Check if currently transitioning between states
	 * @return True if a transition is in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|StateMachine")
	bool IsInTransition() const;

	/** 
	 * Get blended config between previous and current state
	 * @param OutConfig Blended configuration struct to fill
	 * @return True if blended config could be calculated
	 */
	bool GetBlendedConfig(FCameraStateConfig& OutConfig) const;

	//========================================
	// Evaluation
	//========================================
	
	/** 
	 * Evaluate and potentially change state based on input context
	 * @param Context Current camera input context for evaluation
	 */
	void EvaluateState(const FCameraInputContext& Context);

	//========================================
	// Events (Delegates)
	//========================================
	
	/** Called when state changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCameraStateChanged, FName, OldState, FName, NewState);
	
	UPROPERTY(BlueprintAssignable, Category = "Camera|StateMachine")
	FOnCameraStateChanged OnStateChanged;

	/** Called when transition completes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraTransitionComplete, FName, NewState);
	
	UPROPERTY(BlueprintAssignable, Category = "Camera|StateMachine")
	FOnCameraTransitionComplete OnTransitionComplete;

protected:
	//========================================
	// Internal State
	//========================================
	
	/** Reference to states DataTable (legacy architecture) */
	UPROPERTY()
	UDataTable* StatesDataTable;

	/** Reference to config service (new architecture) */
	UPROPERTY()
	UCameraConfigService* ConfigService;

	/** Flag indicating whether using new ConfigService or legacy DataTable */
	UPROPERTY()
	bool bUseConfigService;

	/** Current state name */
	UPROPERTY()
	FName CurrentStateName;

	/** Previous state name (for blending) */
	UPROPERTY()
	FName PreviousStateName;

	/** Current blend alpha */
	UPROPERTY()
	float BlendAlpha;

	/** Time remaining in transition */
	UPROPERTY()
	float TransitionTimeRemaining;

	/** Total transition time */
	UPROPERTY()
	float TransitionTotalTime;

	/** Current blend type */
	UPROPERTY()
	ECameraBlendType CurrentBlendType;

	/** Is initialized */
	UPROPERTY()
	bool bIsInitialized;

	/** Cached state configs for quick lookup */
	TMap<FName, FCameraStateConfig> CachedConfigs;

	/** Cached category mappings for quick category queries */
	TMap<ECameraStateCategory, TArray<FName>> CategoryToStates;

	//========================================
	// Internal Methods
	//========================================
	
	/** Cache all state configs from DataTable */
	void CacheStateConfigs();

	/** 
	 * Calculate blend alpha based on blend type
	 * @param LinearAlpha Linear interpolation value (0-1)
	 * @param BlendType Type of blend curve to apply
	 * @return Curved blend alpha value
	 */
	float CalculateBlendAlpha(float LinearAlpha, ECameraBlendType BlendType) const;

	/** 
	 * Apply blend function to alpha value
	 * @param Alpha Linear alpha value (0-1)
	 * @param BlendType Type of blend curve to apply
	 * @return Curved alpha value
	 */
	static float ApplyBlendCurve(float Alpha, ECameraBlendType BlendType);

	/**
	 * Validate if a state transition is allowed
	 * @param FromState Current state name
	 * @param ToState Target state name
	 * @param Context Current input context
	 * @return True if transition is valid
	 */
	bool ValidateTransition(FName FromState, FName ToState, const FCameraInputContext& Context) const;

	/**
	 * Begin transition to a new state
	 * @param NewStateName Target state name
	 */
	void BeginTransition(FName NewStateName);

	/**
	 * Complete the current transition
	 */
	void CompleteTransition();

	/**
	 * Get state config using ConfigService
	 * 使用ConfigService获取状态配置
	 * 
	 * @param StateName Name of the state
	 * @param OutConfig Output configuration
	 * @return True if config was found
	 */
	bool GetStateConfigFromService(FName StateName, FCameraStateConfig& OutConfig) const;

	/**
	 * Check if state exists using ConfigService
	 * 使用ConfigService检查状态是否存在
	 */
	bool DoesStateExistInService(FName StateName) const;
};
