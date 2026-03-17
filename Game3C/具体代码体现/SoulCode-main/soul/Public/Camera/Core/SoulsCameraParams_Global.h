// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraParams_Global.generated.h"

/**
 * E-Group: Global Parameters (22 params total)
 * System-wide settings that apply to all camera states
 */

//========================================
// E1: Global Limits (4 params)
//========================================
USTRUCT(BlueprintType)
struct FGlobalParams_Limits
{
	GENERATED_BODY()

	/** Absolute minimum camera distance allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "20.0", ClampMax = "200.0"))
	float GlobalMinDistance;

	/** Absolute maximum camera distance allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "500.0", ClampMax = "3000.0"))
	float GlobalMaxDistance;

	/** Absolute minimum FOV allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "20.0", ClampMax = "60.0"))
	float GlobalMinFOV;

	/** Absolute maximum FOV allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "80.0", ClampMax = "140.0"))
	float GlobalMaxFOV;

	FGlobalParams_Limits()
		: GlobalMinDistance(50.0f)
		, GlobalMaxDistance(1500.0f)
		, GlobalMinFOV(40.0f)
		, GlobalMaxFOV(120.0f)
	{}
};

//========================================
// E2: Global Smoothing (4 params)
//========================================
USTRUCT(BlueprintType)
struct FGlobalParams_Smoothing
{
	GENERATED_BODY()

	/** Enable global position smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
	bool bEnableGlobalSmoothing;

	/** Global smoothing strength (0=none, 1=max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableGlobalSmoothing"))
	float GlobalSmoothingStrength;

	/** Maximum smoothing time window (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.01", ClampMax = "1.0", EditCondition = "bEnableGlobalSmoothing"))
	float SmoothingTimeWindow;

	/** Enable adaptive smoothing based on movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
	bool bAdaptiveSmoothing;

	FGlobalParams_Smoothing()
		: bEnableGlobalSmoothing(true)
		, GlobalSmoothingStrength(0.5f)
		, SmoothingTimeWindow(0.1f)
		, bAdaptiveSmoothing(true)
	{}
};

//========================================
// E3: Global Transition (3 params)
//========================================
USTRUCT(BlueprintType)
struct FGlobalParams_Transition
{
	GENERATED_BODY()

	/** Default blend time for state transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float DefaultBlendTime;

	/** Default blend type for transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	ECameraBlendType DefaultBlendType;

	/** Allow interrupting transitions with new state changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bAllowTransitionInterrupt;

	FGlobalParams_Transition()
		: DefaultBlendTime(0.3f)
		, DefaultBlendType(ECameraBlendType::SmoothStep)
		, bAllowTransitionInterrupt(true)
	{}
};

//========================================
// E4: Debug Settings (3 params)
//========================================
USTRUCT(BlueprintType)
struct FGlobalParams_Debug
{
	GENERATED_BODY()

	/** Enable debug visualization by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableDebugByDefault;

	/** Show collision probes in debug view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowCollisionProbes;

	/** Log state changes to output log */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bLogStateChanges;

	FGlobalParams_Debug()
		: bEnableDebugByDefault(false)
		, bShowCollisionProbes(false)
		, bLogStateChanges(false)
	{}
};

//========================================
// E5: Lock-On Settings (5 params)
//========================================
USTRUCT(BlueprintType)
struct FGlobalParams_LockOn
{
	GENERATED_BODY()

	/** Maximum distance to acquire lock-on target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
	float MaxLockOnDistance;

	/** Angle tolerance for lock-on acquisition (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (ClampMin = "10.0", ClampMax = "90.0"))
	float LockOnAngleTolerance;

	/** Sticky lock-on strength (resistance to breaking lock) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LockOnStickiness;

	/** Time to switch between targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float TargetSwitchTime;

	/** Auto-break lock when target too far */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	bool bAutoBreakLockOnDistance;

	FGlobalParams_LockOn()
		: MaxLockOnDistance(2000.0f)
		, LockOnAngleTolerance(45.0f)
		, LockOnStickiness(0.7f)
		, TargetSwitchTime(0.25f)
		, bAutoBreakLockOnDistance(true)
	{}
};

//========================================
// E6: Performance Settings (3 params)
//========================================
USTRUCT(BlueprintType)
struct FGlobalParams_Performance
{
	GENERATED_BODY()

	/** Camera update rate (times per second, 0 = every frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "120"))
	int32 UpdateRate;

	/** Enable LOD for collision detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableCollisionLOD;

	/** Skip modifier updates when no modifiers active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bSkipInactiveModifierUpdates;

	FGlobalParams_Performance()
		: UpdateRate(0)
		, bEnableCollisionLOD(true)
		, bSkipInactiveModifierUpdates(true)
	{}
};

//========================================
// Complete E-Group Structure (22 params)
//========================================
/**
 * Complete global parameters (E-Group)
 * Contains all 22 system-wide configuration parameters
 */
USTRUCT(BlueprintType)
struct FGlobalParams
{
	GENERATED_BODY()

	/** E1: Global limits (4 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "E1_Limits")
	FGlobalParams_Limits Limits;

	/** E2: Smoothing settings (4 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "E2_Smoothing")
	FGlobalParams_Smoothing Smoothing;

	/** E3: Transition settings (3 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "E3_Transition")
	FGlobalParams_Transition Transition;

	/** E4: Debug settings (3 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "E4_Debug")
	FGlobalParams_Debug Debug;

	/** E5: Lock-on settings (5 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "E5_LockOn")
	FGlobalParams_LockOn LockOn;

	/** E6: Performance settings (3 params) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "E6_Performance")
	FGlobalParams_Performance Performance;

	FGlobalParams()
	{
		// All sub-structs use their own default constructors
	}
};
