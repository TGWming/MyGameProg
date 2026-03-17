// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModuleRegistry.h"

//========================================
// Module Includes - All 39 Camera Modules
//========================================

// Position Modules (P01-P08)
#include "Camera/Modules/CameraModule_Position.h"

// Rotation Modules (R01-R09)
#include "Camera/Modules/CameraModule_Rotation.h"
#include "Camera/Modules/CameraModule_Framing.h"

// Distance Modules (D01-D07)
#include "Camera/Modules/CameraModule_Distance.h"

// FOV Modules (F01-F06)
#include "Camera/Modules/CameraModule_FOV.h"

// Offset Modules (O01-O05)
#include "Camera/Modules/CameraModule_Offset.h"

// Constraint Modules (C01-C04)
#include "Camera/Modules/CameraModule_Constraint.h"

UCameraModuleRegistry::UCameraModuleRegistry()
	: bIsInitialized(false)
{
}

void UCameraModuleRegistry::Initialize(UObject* Outer)
{
	if (bIsInitialized)
	{
		return;
	}

	CreateAllModules(Outer);
	bIsInitialized = true;
}

bool UCameraModuleRegistry::RegisterModule(UCameraModuleBase* Module)
{
	if (!Module)
	{
		return false;
	}

	ECameraModuleType ModuleType = Module->GetModuleType();
	if (ModuleType == ECameraModuleType::ModuleNone)
	{
		return false;
	}

	// Check if already registered
	if (RegisteredModules.Contains(ModuleType))
	{
		UE_LOG(LogTemp, Warning, TEXT("Module type %d already registered"), static_cast<int32>(ModuleType));
		return false;
	}

	RegisteredModules.Add(ModuleType, Module);
	return true;
}

bool UCameraModuleRegistry::UnregisterModule(ECameraModuleType ModuleType)
{
	if (ModuleType == ECameraModuleType::ModuleNone)
	{
		return false;
	}

	if (!RegisteredModules.Contains(ModuleType))
	{
		return false;
	}

	RegisteredModules.Remove(ModuleType);
	return true;
}

UCameraModuleBase* UCameraModuleRegistry::GetModule(ECameraModuleType ModuleType) const
{
	const UCameraModuleBase* const* FoundModule = RegisteredModules.Find(ModuleType);
	return FoundModule ? const_cast<UCameraModuleBase*>(*FoundModule) : nullptr;
}

TArray<UCameraModuleBase*> UCameraModuleRegistry::GetAllModules() const
{
	TArray<UCameraModuleBase*> Result;
	RegisteredModules.GenerateValueArray(Result);
	return Result;
}

TArray<UCameraModuleBase*> UCameraModuleRegistry::GetModulesByCategory(EModuleCategory Category) const
{
	TArray<UCameraModuleBase*> Result;
	
	for (const auto& Pair : RegisteredModules)
	{
		if (Pair.Value && Pair.Value->GetModuleCategory() == Category)
		{
			Result.Add(Pair.Value);
		}
	}
	
	return Result;
}

void UCameraModuleRegistry::GetActiveModules(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, TArray<UCameraModuleBase*>& OutActiveModules)
{
	OutActiveModules.Empty();
	
	for (const auto& Pair : RegisteredModules)
	{
		UCameraModuleBase* Module = Pair.Value;
		if (Module && Module->IsEnabled() && Module->ShouldActivate(Context, StateConfig))
		{
			OutActiveModules.Add(Module);
		}
	}
	
	// Sort by priority (higher priority first)
	OutActiveModules.Sort([](const UCameraModuleBase& A, const UCameraModuleBase& B)
	{
		return A.GetDefaultPriority() > B.GetDefaultPriority();
	});
}

void UCameraModuleRegistry::UpdateModuleStates(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig)
{
	for (const auto& Pair : RegisteredModules)
	{
		UCameraModuleBase* Module = Pair.Value;
		if (!Module)
		{
			continue;
		}

		bool bShouldBeActive = Module->IsEnabled() && Module->ShouldActivate(Context, StateConfig);
		bool bIsCurrentlyActive = Module->IsActive();

		if (bShouldBeActive && !bIsCurrentlyActive)
		{
			// Activate module
			Module->OnActivate(Context);
			if (!ActiveModules.Contains(Module))
			{
				ActiveModules.Add(Module);
			}
		}
		else if (!bShouldBeActive && bIsCurrentlyActive)
		{
			// Deactivate module
			Module->OnDeactivate(Context);
			ActiveModules.Remove(Module);
		}
	}
}

int32 UCameraModuleRegistry::GetActiveModuleCount() const
{
	int32 Count = 0;
	for (const auto& Pair : RegisteredModules)
	{
		if (Pair.Value && Pair.Value->IsActive())
		{
			++Count;
		}
	}
	return Count;
}

/**
 * CreateAllModules - Instantiate and register all camera modules
 * 
 * This function creates instances of all 39 camera modules and
 * registers them with the registry. Modules are created in
 * category order for clarity.
 * 
 * Module Categories:
 * - Position (P01-P08): 8 modules - Calculate focus point
 * - Rotation (R01-R09): 9 modules - Calculate camera orientation
 * - Distance (D01-D07): 7 modules - Calculate arm length
 * - FOV (F01-F06): 6 modules - Calculate field of view
 * - Offset (O01-O05): 5 modules - Calculate position offsets
 * - Constraint (C01-C04): 4 modules - Enforce limits
 * 
 * Total: 39 modules
 * 
 * @param Outer The outer object for module creation (usually the camera manager)
 */
void UCameraModuleRegistry::CreateAllModules(UObject* Outer)
{
	// Validate outer object
	if (!Outer)
	{
		Outer = this;
	}

	//========================================
	// Position Modules (P01-P08) - 8 modules
	// Calculate camera focus/target point
	//========================================
	
	RegisterModule(NewObject<UCameraModule_P01_FollowTarget>(Outer));
	RegisterModule(NewObject<UCameraModule_P02_FollowTarget_Lagged>(Outer));
	RegisterModule(NewObject<UCameraModule_P03_FollowTarget_Predictive>(Outer));
	RegisterModule(NewObject<UCameraModule_P04_Orbit_LockOn>(Outer));
	RegisterModule(NewObject<UCameraModule_P05_Orbit_Boss>(Outer));
	RegisterModule(NewObject<UCameraModule_P06_FixedPoint>(Outer));
	RegisterModule(NewObject<UCameraModule_P07_Spline_Follow>(Outer));
	RegisterModule(NewObject<UCameraModule_P08_MidPoint_TwoTarget>(Outer));

	//========================================
	// Rotation Modules (R01-R10) - 10 modules
	// Calculate camera orientation/rotation
	//========================================
	
	RegisterModule(NewObject<UCameraModule_R01_PlayerInput_Free>(Outer));
	RegisterModule(NewObject<UCameraModule_R02_PlayerInput_Lagged>(Outer));
	RegisterModule(NewObject<UCameraModule_R03_LookAt_Target>(Outer));
	RegisterModule(NewObject<UCameraModule_R04_LookAt_Target_Soft>(Outer));
	RegisterModule(NewObject<UCameraModule_R05_LookAt_Boss>(Outer));
	RegisterModule(NewObject<UCameraModule_R06_AutoOrient_Movement>(Outer));
	RegisterModule(NewObject<UCameraModule_R07_AutoOrient_Delayed>(Outer));
	RegisterModule(NewObject<UCameraModule_R08_LookAt_Point>(Outer));
	RegisterModule(NewObject<UCameraModule_R09_Spline_Rotation>(Outer));
	RegisterModule(NewObject<UCameraModule_R10_Framing>(Outer));

	//========================================
	// Distance Modules (D01-D07) - 7 modules
	// Calculate camera arm length/distance
	//========================================
	
	RegisterModule(NewObject<UCameraModule_D01_BaseDistance>(Outer));
	RegisterModule(NewObject<UCameraModule_D02_TargetSize_Multiplier>(Outer));
	RegisterModule(NewObject<UCameraModule_D03_Speed_Offset>(Outer));
	RegisterModule(NewObject<UCameraModule_D04_Combat_Adjust>(Outer));
	RegisterModule(NewObject<UCameraModule_D05_Environment_Limit>(Outer));
	RegisterModule(NewObject<UCameraModule_D06_Proximity_Adjust>(Outer));
	RegisterModule(NewObject<UCameraModule_D07_Boss_Phase>(Outer));

	//========================================
	// FOV Modules (F01-F06) - 6 modules
	// Calculate field of view
	//========================================
	
	RegisterModule(NewObject<UCameraModule_F01_BaseFOV>(Outer));
	RegisterModule(NewObject<UCameraModule_F02_Speed_FOV>(Outer));
	RegisterModule(NewObject<UCameraModule_F03_Aim_FOV>(Outer));
	RegisterModule(NewObject<UCameraModule_F04_Combat_FOV>(Outer));
	RegisterModule(NewObject<UCameraModule_F05_Boss_FOV>(Outer));
	RegisterModule(NewObject<UCameraModule_F06_Impact_FOV>(Outer));

	//========================================
	// Offset Modules (O01-O05) - 5 modules
	// Calculate camera position offsets
	//========================================
	
	RegisterModule(NewObject<UCameraModule_O01_SocketOffset_Base>(Outer));
	RegisterModule(NewObject<UCameraModule_O02_Shoulder_Offset>(Outer));
	RegisterModule(NewObject<UCameraModule_O03_Shoulder_Switch>(Outer));
	RegisterModule(NewObject<UCameraModule_O04_Crouch_Offset>(Outer));
	RegisterModule(NewObject<UCameraModule_O05_Mount_Offset>(Outer));

	//========================================
	// Constraint Modules (C01-C04) - 4 modules
	// Enforce limits and ensure validity
	//========================================
	
	RegisterModule(NewObject<UCameraModule_C01_Pitch_Clamp>(Outer));
	RegisterModule(NewObject<UCameraModule_C02_Distance_Clamp>(Outer));
	RegisterModule(NewObject<UCameraModule_C03_FOV_Clamp>(Outer));
	RegisterModule(NewObject<UCameraModule_C04_Visibility_Ensure>(Outer));
}
