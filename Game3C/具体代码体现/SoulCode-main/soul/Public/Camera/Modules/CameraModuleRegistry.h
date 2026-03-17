// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "Camera/Pipeline/ICameraStage.h"
#include "Camera/Core/SoulsCameraConfig.h"
#include "CameraModuleBase.h"
#include "CameraModuleRegistry.generated.h"

/**
 * UCameraModuleRegistry
 * 
 * Central registry for managing all 39 camera modules in the system.
 * Responsible for module registration, lookup, activation state management,
 * and providing active modules to the pipeline Stage 3 (ModuleCompute).
 * 
 * Key Responsibilities:
 * - Register and unregister camera modules by type
 * - Query modules by type or category
 * - Determine which modules should be active based on context
 * - Update module activation states each frame
 * - Provide active modules to CameraStage_ModuleCompute
 * 
 * Module Categories (39 modules total):
 * - Position (P01-P08): 8 modules for camera position calculation
 * - Rotation (R01-R09): 9 modules for camera rotation calculation
 * - Distance (D01-D07): 7 modules for camera distance calculation
 * - FOV (F01-F06): 6 modules for field of view calculation
 * - Offset (O01-O05): 5 modules for camera offset calculation
 * - Constraint (C01-C04): 4 modules for camera constraints
 */
UCLASS(BlueprintType)
class SOUL_API UCameraModuleRegistry : public UObject
{
	GENERATED_BODY()

public:
	//========================================
	// Initialization
	//========================================

	/** Default constructor */
	UCameraModuleRegistry();

	/**
	 * Initialize and create all 39 camera modules
	 * @param Outer The outer object for module creation
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModuleRegistry")
	void Initialize(UObject* Outer);

	/**
	 * Check if registry is initialized
	 * @return True if registry has been initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleRegistry")
	FORCEINLINE bool IsInitialized() const { return bIsInitialized; }

	//========================================
	// Module Registration
	//========================================

	/**
	 * Register a module instance
	 * @param Module The module to register
	 * @return True if registration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModuleRegistry")
	bool RegisterModule(UCameraModuleBase* Module);

	/**
	 * Unregister a module by type
	 * @param ModuleType The type of module to unregister
	 * @return True if unregistration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModuleRegistry")
	bool UnregisterModule(ECameraModuleType ModuleType);

	/**
	 * Get module by type
	 * @param ModuleType The type of module to retrieve
	 * @return The module instance, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleRegistry")
	UCameraModuleBase* GetModule(ECameraModuleType ModuleType) const;

	/**
	 * Get all registered modules
	 * @return Array of all registered module instances
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleRegistry")
	TArray<UCameraModuleBase*> GetAllModules() const;

	/**
	 * Get modules by category (Position/Rotation/Distance/FOV/Offset/Constraint)
	 * @param Category The category to filter by
	 * @return Array of modules in the specified category
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleRegistry")
	TArray<UCameraModuleBase*> GetModulesByCategory(EModuleCategory Category) const;

	//========================================
	// Active Module Query
	//========================================

	/**
	 * Get all modules that should be active for current context
	 * @param Context The current stage execution context
	 * @param StateConfig The current camera state configuration
	 * @param OutActiveModules Output array of active modules
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModuleRegistry")
	void GetActiveModules(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, TArray<UCameraModuleBase*>& OutActiveModules);

	/**
	 * Update module activation states based on context
	 * @param Context The current stage execution context
	 * @param StateConfig The current camera state configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|ModuleRegistry")
	void UpdateModuleStates(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig);

	//========================================
	// Module Count
	//========================================

	/**
	 * Get total number of registered modules
	 * @return Number of registered modules
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleRegistry")
	FORCEINLINE int32 GetModuleCount() const { return RegisteredModules.Num(); }

	/**
	 * Get number of currently active modules
	 * @return Number of active modules
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|ModuleRegistry")
	int32 GetActiveModuleCount() const;

protected:
	//========================================
	// Internal Methods
	//========================================

	/**
	 * Create all 39 module instances
	 * @param Outer The outer object for module creation
	 */
	void CreateAllModules(UObject* Outer);

	/**
	 * Create a single module of specified type
	 * @tparam T The module class type to create
	 * @param Outer The outer object for module creation
	 * @param ModuleType The type identifier for the module
	 * @return The created module instance
	 */
	template<typename T>
	T* CreateModule(UObject* Outer, ECameraModuleType ModuleType);

private:
	//========================================
	// State
	//========================================

	/** Map of registered modules by type */
	UPROPERTY()
	TMap<ECameraModuleType, UCameraModuleBase*> RegisteredModules;

	/** Currently active modules (updated each frame) */
	UPROPERTY()
	TArray<UCameraModuleBase*> ActiveModules;

	/** Is registry initialized */
	bool bIsInitialized;
};

//========================================
// Template Implementation
//========================================

template<typename T>
T* UCameraModuleRegistry::CreateModule(UObject* Outer, ECameraModuleType ModuleType)
{
	static_assert(TIsDerivedFrom<T, UCameraModuleBase>::IsDerived, "T must derive from UCameraModuleBase");
	
	T* NewModule = NewObject<T>(Outer);
	if (NewModule)
	{
		RegisterModule(NewModule);
	}
	return NewModule;
}
