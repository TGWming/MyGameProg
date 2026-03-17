// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "Camera/Core/SoulsCameraConfig.h"
#include "Camera/Pipeline/ICameraStage.h"
#include "CameraModuleBase.generated.h"

/**
 * Module Category Enumeration
 * 
 * Categorizes the 39 camera modules into 6 functional groups:
 * - Position:   P01-P08, Focus point calculation
 * - Rotation:   R01-R09, Camera rotation
 * - Distance:   D01-D07, Arm length
 * - FOV:        F01-F06, Field of view
 * - Offset:     O01-O05, Socket/target offsets
 * - Constraint: C01-C04, Limits and constraints
 */
UENUM(BlueprintType)
enum class EModuleCategory : uint8
{
	Position    UMETA(DisplayName = "Position"),    // P01-P08: Focus point calculation
	Rotation    UMETA(DisplayName = "Rotation"),    // R01-R09: Camera rotation
	Distance    UMETA(DisplayName = "Distance"),    // D01-D07: Arm length
	FOV         UMETA(DisplayName = "FOV"),         // F01-F06: Field of view
	Offset      UMETA(DisplayName = "Offset"),      // O01-O05: Socket/target offsets
	Constraint  UMETA(DisplayName = "Constraint")   // C01-C04: Limits and constraints
};

/**
 * UCameraModuleBase - Abstract base class for all camera calculation modules
 * 
 * The camera system uses 39 modular modules to compute camera parameters.
 * Each module is responsible for calculating a specific aspect of the camera:
 * - Position modules (P01-P08): Calculate focus point / follow position
 * - Rotation modules (R01-R09): Calculate camera rotation and orientation
 * - Distance modules (D01-D07): Calculate spring arm length / distance
 * - FOV modules (F01-F06): Calculate field of view adjustments
 * - Offset modules (O01-O05): Calculate socket and target offsets
 * - Constraint modules (C01-C04): Apply limits and constraints to values
 * 
 * Pipeline Integration:
 * - Stage 2 (State Machine): Determines active camera state
 * - Stage 3 (Module Compute): Executes all active modules via this base class
 * - Stage 5 (Blend & Solve): Combines module outputs using blend policies
 * 
 * Lifecycle:
 * 1. ShouldActivate() is called to determine if module should run
 * 2. OnActivate() is called when module becomes active
 * 3. Compute() is called each frame while active
 * 4. OnDeactivate() is called when module becomes inactive
 * 
 * Usage:
 * Derive from this class and implement GetModuleType(), GetModuleCategory(),
 * and Compute() to create a new camera module.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SOUL_API UCameraModuleBase : public UObject
{
	GENERATED_BODY()

public:
	/** Default constructor */
	UCameraModuleBase();

	//========================================
	// Module Identity
	//========================================

	/**
	 * Get the specific module type identifier
	 * Must be overridden by derived classes
	 * @return The ECameraModuleType enum value for this module
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	virtual ECameraModuleType GetModuleType() const PURE_VIRTUAL(UCameraModuleBase::GetModuleType, return ECameraModuleType::ModuleNone;);

	/**
	 * Get the category this module belongs to
	 * Must be overridden by derived classes
	 * @return The EModuleCategory enum value (Position, Rotation, Distance, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	virtual EModuleCategory GetModuleCategory() const PURE_VIRTUAL(UCameraModuleBase::GetModuleCategory, return EModuleCategory::Position;);

	/**
	 * Get the unique name of this module
	 * Override to provide a custom name
	 * @return The module name as FName
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	virtual FName GetModuleName() const;

	/**
	 * Get a human-readable description of this module
	 * Override to provide detailed description
	 * @return Description string
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	virtual FString GetModuleDescription() const;

	//========================================
	// Module Priority & Blending
	//========================================

	/**
	 * Get the default priority for this module
	 * Higher priority modules execute first and can override lower priority ones
	 * Note: Can be overridden by state config
	 * @return Default priority value (default: 100)
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	virtual int32 GetDefaultPriority() const { return 100; }

	/**
	 * Get the default blend policy for this module's output
	 * Determines how this module's output combines with others
	 * Note: Can be overridden by state config
	 * @return Default blend policy (default: Blend)
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	virtual EBlendPolicy GetDefaultBlendPolicy() const { return EBlendPolicy::Blend; }

	//========================================
	// Activation
	//========================================

	/**
	 * Determine if this module should activate for the current context
	 * Override to implement custom activation logic
	 * @param Context The current pipeline execution context
	 * @param StateConfig The current camera state configuration
	 * @return True if module should be active
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const;

	/**
	 * Check if this module is currently active
	 * @return True if module is active
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	bool IsActive() const { return bIsActive; }

	/**
	 * Enable or disable this module
	 * Disabled modules will not be considered for activation
	 * @param bEnabled Whether to enable the module
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Module")
	void SetEnabled(bool bEnabled) { bIsEnabled = bEnabled; }

	/**
	 * Check if this module is enabled
	 * @return True if module is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Module")
	bool IsEnabled() const { return bIsEnabled; }

	//========================================
	// Computation
	//========================================

	/**
	 * Main computation method - calculates this module's camera output
	 * Must be overridden by derived classes
	 * 
	 * This method is called each frame while the module is active.
	 * Implementations should:
	 * 1. Read input from Context.InputContext
	 * 2. Read configuration from StateConfig
	 * 3. Perform calculations
	 * 4. Write results to OutOutput
	 * 
	 * @param Context The current pipeline execution context containing input data
	 * @param StateConfig The current camera state configuration with module parameters
	 * @param OutOutput Output structure to fill with computed values
	 * @return True if computation succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Module")
	virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) PURE_VIRTUAL(UCameraModuleBase::Compute, return false;);

	//========================================
	// Lifecycle
	//========================================

	/**
	 * Called when this module becomes active
	 * Override to implement initialization logic
	 * @param Context The current pipeline execution context
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Module")
	virtual void OnActivate(const FStageExecutionContext& Context);

	/**
	 * Called when this module becomes inactive
	 * Override to implement cleanup logic
	 * @param Context The current pipeline execution context
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Module")
	virtual void OnDeactivate(const FStageExecutionContext& Context);

	/**
	 * Called each frame while module is active (before Compute)
	 * Override to implement per-frame update logic
	 * @param DeltaTime Time since last frame
	 * @param Context The current pipeline execution context
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Module")
	virtual void OnUpdate(float DeltaTime, const FStageExecutionContext& Context);

protected:
	//========================================
	// Helper Methods for Derived Classes
	//========================================

	/**
	 * Create a base output structure with default values
	 * Convenience method for derived classes
	 * @return A pre-initialized FModuleOutput structure
	 */
	FModuleOutput CreateBaseOutput() const;

	/**
	 * Calculate interpolation alpha for smooth transitions
	 * @param DeltaTime Time since last frame
	 * @param LagSpeed The lag/interpolation speed (higher = faster)
	 * @return Alpha value for FMath::Lerp (0.0 - 1.0)
	 */
	float GetInterpAlpha(float DeltaTime, float LagSpeed) const;

	/**
	 * Get the owner actor from execution context
	 * @param Context The pipeline execution context
	 * @return The owner actor (usually the player character), or nullptr
	 */
	AActor* GetOwnerActor(const FStageExecutionContext& Context) const;

	/**
	 * Get the target actor from execution context
	 * @param Context The pipeline execution context
	 * @return The current target actor (e.g., lock-on target), or nullptr
	 */
	AActor* GetTargetActor(const FStageExecutionContext& Context) const;

	//========================================
	// State
	//========================================

	/** Whether this module is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsActive;

	/** Whether this module is enabled (can be activated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsEnabled;

	/** Time elapsed since this module was activated */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float TimeSinceActivation;
};
