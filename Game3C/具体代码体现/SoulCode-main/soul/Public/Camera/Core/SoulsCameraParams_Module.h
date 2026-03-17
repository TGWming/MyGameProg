// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraParams_Module.generated.h"

/**
 * B-Group: Module Parameters (32 params total)
 * Controls behavior of the 39 camera calculation modules
 */

//========================================
// B1: Position Module Parameters (6 params)
//========================================
USTRUCT(BlueprintType)
struct FModuleParams_Position
{
	GENERATED_BODY()

	/** Prediction time for predictive follow (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PredictionTime;

	/** Orbit radius multiplier for lock-on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float OrbitRadiusMultiplier;

	/** Mid-point bias between player and target (0=player, 1=target) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MidPointBias;

	/** Fixed point world location (for cinematic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	FVector FixedPointLocation;

	/** Enable position smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	bool bEnablePositionSmoothing;

	/** Position smoothing strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnablePositionSmoothing"))
	float PositionSmoothingStrength;

	FModuleParams_Position()
		: PredictionTime(0.2f)
		, OrbitRadiusMultiplier(1.0f)
		, MidPointBias(0.3f)
		, FixedPointLocation(FVector::ZeroVector)
		, bEnablePositionSmoothing(true)
		, PositionSmoothingStrength(0.5f)
	{}
};

//========================================
// B2: Rotation Module Parameters (6 params)
//========================================
USTRUCT(BlueprintType)
struct FModuleParams_Rotation
{
	GENERATED_BODY()

	/** Input sensitivity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float InputSensitivity;

	/** Soft lock-on strength (0=free, 1=hard lock) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoftLockStrength;

	/** Auto-orient speed when following movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float AutoOrientSpeed;

	/** Minimum movement speed to trigger auto-orient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float AutoOrientMinSpeed;

	/** Look-at interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float LookAtInterpSpeed;

	/** Enable rotation input lag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bEnableRotationLag;

	FModuleParams_Rotation()
		: InputSensitivity(1.0f)
		, SoftLockStrength(0.7f)
		, AutoOrientSpeed(3.0f)
		, AutoOrientMinSpeed(100.0f)
		, LookAtInterpSpeed(8.0f)
		, bEnableRotationLag(true)
	{}
};

//========================================
// B3: Distance Module Parameters (6 params)
//========================================
USTRUCT(BlueprintType)
struct FModuleParams_Distance
{
	GENERATED_BODY()

	/** Target size threshold for distance adjustment (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float TargetSizeThreshold;

	/** Distance multiplier per 100cm of target size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float TargetSizeDistanceScale;

	/** Speed threshold to start distance adjustment (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float SpeedDistanceThreshold;

	/** Maximum distance added at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float SpeedDistanceMax;

	/** Combat distance offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	float CombatDistanceOffset;

	/** Boss phase distance multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float BossPhaseDistanceScale;

	FModuleParams_Distance()
		: TargetSizeThreshold(200.0f)
		, TargetSizeDistanceScale(1.2f)
		, SpeedDistanceThreshold(300.0f)
		, SpeedDistanceMax(100.0f)
		, CombatDistanceOffset(-50.0f)
		, BossPhaseDistanceScale(1.0f)
	{}
};

//========================================
// B4: FOV Module Parameters (5 params)
//========================================
USTRUCT(BlueprintType)
struct FModuleParams_FOV
{
	GENERATED_BODY()

	/** FOV increase at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float SpeedFOVIncrease;

	/** Speed threshold to start FOV increase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float SpeedFOVThreshold;

	/** Aim/focus mode FOV reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float AimFOVReduction;

	/** Combat FOV offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "-20.0", ClampMax = "20.0"))
	float CombatFOVOffset;

	/** Boss FOV offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "-20.0", ClampMax = "20.0"))
	float BossFOVOffset;

	// ===== Impact FOV Configuration =====
	
	/** FOV冲击量（度），受击时FOV变化幅度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV|Impact", 
			  meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float ImpactFOVAmount;
	
	/** FOV冲击持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV|Impact",
			  meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float ImpactDuration;

	/** Enable dynamic FOV adjustment based on speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV|Dynamic")
	bool bEnableDynamicFOV;

	/** Minimum FOV value when speeding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV|Dynamic", meta = (ClampMin = "10.0", ClampMax = "90.0", EditCondition = "bEnableDynamicFOV"))
	float MinSpeedFOV;

	/** Maximum FOV value when speeding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV|Dynamic", meta = (ClampMin = "10.0", ClampMax = "90.0", EditCondition = "bEnableDynamicFOV"))
	float MaxSpeedFOV;

	/** Speed threshold for dynamic FOV scaling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV|Dynamic", meta = (ClampMin = "0.0", ClampMax = "1000.0", EditCondition = "bEnableDynamicFOV"))
	float DynamicFOVSpeedThreshold;

	/** FOV scaling factor based on speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV|Dynamic", meta = (ClampMin = "0.1", ClampMax = "3.0", EditCondition = "bEnableDynamicFOV"))
	float DynamicFOVScale;

	FModuleParams_FOV()
		: SpeedFOVIncrease(10.0f)
		, SpeedFOVThreshold(400.0f)
		, AimFOVReduction(10.0f)
		, CombatFOVOffset(-5.0f)
		, BossFOVOffset(5.0f)
		, ImpactFOVAmount(10.0f)
		, ImpactDuration(0.15f)
		, bEnableDynamicFOV(true)
		, MinSpeedFOV(45.0f)
		, MaxSpeedFOV(90.0f)
		, DynamicFOVSpeedThreshold(600.0f)
		, DynamicFOVScale(1.5f)
	{}
};

//========================================
// B5: Offset Module Parameters (5 params)
//========================================
USTRUCT(BlueprintType)
struct FModuleParams_Offset
{
	GENERATED_BODY()

	/** Shoulder switch transition time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ShoulderSwitchTime;

	/** Crouch offset vertical adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "-100.0", ClampMax = "0.0"))
	float CrouchVerticalOffset;

	/** Mount offset vertical adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float MountVerticalOffset;

	/** Enable dynamic shoulder switching based on target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	bool bAutoShoulderSwitch;

	/** Shoulder offset interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float ShoulderInterpSpeed;

	FModuleParams_Offset()
		: ShoulderSwitchTime(0.3f)
		, CrouchVerticalOffset(-30.0f)
		, MountVerticalOffset(50.0f)
		, bAutoShoulderSwitch(false)
		, ShoulderInterpSpeed(8.0f)
	{}
};

//========================================
// B6: Constraint Module Parameters (4 params)
//========================================
USTRUCT(BlueprintType)
struct FModuleParams_Constraint
{
	GENERATED_BODY()

	/** Enable visibility ensure (keep target visible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	bool bEnsureTargetVisible;

	/** Visibility check frequency per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "1.0", ClampMax = "60.0", EditCondition = "bEnsureTargetVisible"))
	float VisibilityCheckRate;

	/** Enable soft pitch limits (gradual resistance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	bool bUseSoftPitchLimits;

	/** Soft limit resistance strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bUseSoftPitchLimits"))
	float SoftLimitStrength;

	FModuleParams_Constraint()
		: bEnsureTargetVisible(true)
		, VisibilityCheckRate(30.0f)
		, bUseSoftPitchLimits(true)
		, SoftLimitStrength(0.5f)
	{}
};

//========================================
// Complete B-Group Structure (34 params)
//========================================
/**
 * Complete module parameters (B-Group)
 * Contains all 34 parameters for module behavior configuration
 * 
 * Parameter Count:
 * - B1 Position: 6 params
 * - B2 Rotation: 6 params
 * - B3 Distance: 6 params
 * - B4 FOV: 7 params
 * - B5 Offset: 5 params
 * - B6 Constraint: 4 params
 * - Total: 34 params
 */
USTRUCT(BlueprintType)
struct FModuleParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "B1_Position")
	FModuleParams_Position Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "B2_Rotation")
	FModuleParams_Rotation Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "B3_Distance")
	FModuleParams_Distance Distance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "B4_FOV")
	FModuleParams_FOV FOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "B5_Offset")
	FModuleParams_Offset Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "B6_Constraint")
	FModuleParams_Constraint Constraint;

	FModuleParams()
	{
		// All sub-structs use their own default constructors
	}
};
