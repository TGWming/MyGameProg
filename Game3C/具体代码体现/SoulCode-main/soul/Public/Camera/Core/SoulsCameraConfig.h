// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraParams_StateBase.h"
#include "SoulsCameraParams_Module.h"
#include "SoulsCameraParams_Modifier.h"
#include "SoulsCameraParams_Collision.h"
#include "Camera/Config/SoulsCameraParams_Framing.h"
#include "SoulsCameraConfig.generated.h"

/**
 * FCameraStateConfig - Main configuration structure for camera states
 * 
 * This structure defines a single camera state with all its parameters.
 * It inherits from FTableRowBase to be used in UE4 DataTables.
 * 
 * Parameter Groups:
 * - A: StateBase (34 params) - Basic state settings
 * - B: Module (32 params) - Module behavior settings
 * - C: Modifier (58 params) - Modifier behavior settings
 * - D: Collision (30 params) - Collision handling settings
 * 
 * Total: 154 parameters per state
 * Expected rows: 725 states
 */
USTRUCT(BlueprintType)
struct FCameraStateConfig : public FTableRowBase
{
	GENERATED_BODY()

	//========================================
	// A-Group: State Base Parameters (34 params)
	//========================================
	/**
	 * Basic state parameters including identity, distance, FOV,
	 * rotation limits, offsets, lag, transitions, collision, and flags
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A_StateBase")
	FStateBaseParams StateBase;

	//========================================
	// B-Group: Module Parameters (32 params)
	//========================================
	/**
	 * Parameters controlling the behavior of the 39 camera modules
	 * including position, rotation, distance, FOV, offset, and constraints
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "B_Module")
	FModuleParams Module;

	//========================================
	// C-Group: Modifier Parameters (58 params)
	//========================================
	/**
	 * Parameters controlling the behavior of the 26 camera modifiers
	 * including shake, reaction, cinematic, zoom, effect, and special
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C_Modifier")
	FModifierParams Modifier;

	//========================================
	// D-Group: Collision Parameters (30 params)
	//========================================
	/**
	 * Parameters controlling the 20 collision strategies
	 * including detection, response, occlusion, recovery, and special cases
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D_Collision")
	FCollisionParams Collision;

	/** Framing 构图参数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing")
	FFramingParams FramingParams;

	/** Default constructor */
	FCameraStateConfig()
	{
		// All sub-structs use their own default constructors
	}

	/**
	 * Get the state name from StateBase.Identity
	 * @return The unique name of this state
	 */
	FORCEINLINE FName GetStateName() const
	{
		return StateBase.Identity.StateName;
	}

	/**
	 * Get the state priority
	 * @return Priority value (higher = more important)
	 */
	FORCEINLINE int32 GetPriority() const
	{
		return StateBase.Identity.Priority;
	}

	/**
	 * Get the state category
	 * @return The category this state belongs to
	 */
	FORCEINLINE ECameraStateCategory GetCategory() const
	{
		return StateBase.Identity.Category;
	}

	/**
	 * Check if this state requires a locked target
	 * @return True if target is required
	 */
	FORCEINLINE bool RequiresTarget() const
	{
		return StateBase.Flags.bRequiresTarget;
	}

	/**
	 * Check if this is a cinematic state
	 * @return True if cinematic
	 */
	FORCEINLINE bool IsCinematic() const
	{
		return StateBase.Flags.bIsCinematic;
	}
};
