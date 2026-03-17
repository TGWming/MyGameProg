// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraParams_StateBase.generated.h"

//========================================
// Identity Parameters - Sub-Step 1.2.1
//========================================

/**
 * State identity parameters
 * Basic identification and priority settings for camera states
 */
USTRUCT(BlueprintType)
struct FStateIdentityParams
{
	GENERATED_BODY()

	/** Unique name identifier for this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName StateName;

	/** Priority level (higher = more important, 0-255) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ClampMin = "0", ClampMax = "255"))
	int32 Priority;

	/** Category this state belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	ECameraStateCategory Category;

	FStateIdentityParams()
		: StateName(NAME_None)
		, Priority(100)
		, Category(ECameraStateCategory::FreeExploration)
	{}
};

//========================================
// Distance Parameters - Sub-Step 1.2.2
//========================================

/**
 * State distance parameters
 * Controls camera arm length for this state
 */
USTRUCT(BlueprintType)
struct FStateDistanceParams
{
	GENERATED_BODY()

	/** Base camera distance in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
	float BaseDistance;

	/** Minimum allowed distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
	float MinDistance;

	/** Maximum allowed distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
	float MaxDistance;

	FStateDistanceParams()
		: BaseDistance(400.0f)
		, MinDistance(150.0f)
		, MaxDistance(800.0f)
	{}
};

//========================================
// FOV Parameters - Sub-Step 1.2.3
//========================================

/**
 * State FOV parameters
 * Controls field of view settings for this state
 */
USTRUCT(BlueprintType)
struct FStateFOVParams
{
	GENERATED_BODY()

	/** Base field of view in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "30.0", ClampMax = "120.0"))
	float BaseFOV;

	/** Minimum allowed FOV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "30.0", ClampMax = "120.0"))
	float MinFOV;

	/** Maximum allowed FOV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "30.0", ClampMax = "120.0"))
	float MaxFOV;

	FStateFOVParams()
		: BaseFOV(70.0f)
		, MinFOV(50.0f)
		, MaxFOV(100.0f)
	{}
};

//========================================
// Rotation Parameters - Sub-Step 1.2.4
//========================================

/**
 * State rotation parameters
 * Controls pitch angle limits for this state
 */
USTRUCT(BlueprintType)
struct FStateRotationParams
{
	GENERATED_BODY()

	/** Minimum pitch angle (looking up limit, negative value) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
	float MinPitch;

	/** Maximum pitch angle (looking down limit, positive value) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float MaxPitch;

	FStateRotationParams()
		: MinPitch(-60.0f)
		, MaxPitch(60.0f)
	{}
};

//========================================
// Offset Parameters - Sub-Step 1.2.5
//========================================

/**
 * State offset parameters
 * Controls camera position offsets for this state
 */
USTRUCT(BlueprintType)
struct FStateOffsetParams
{
	GENERATED_BODY()

	/** Offset applied to the focus/pivot point (world space relative to character) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector FocusOffset;

	/** Socket offset (camera position relative to arm end) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector SocketOffset;

	/** Target offset (arm pivot point offset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector TargetOffset;

	/** Vertical offset for character height adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	float VerticalOffset;

	/** Horizontal shoulder offset (positive = right, negative = left) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	float ShoulderOffset;

	/** Forward/backward offset (positive = forward, negative = backward) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (ClampMin = "-200.0", ClampMax = "200.0"))
	float ForwardOffset;

	FStateOffsetParams()
		: FocusOffset(FVector(0.0f, 0.0f, 60.0f))
		, SocketOffset(FVector::ZeroVector)
		, TargetOffset(FVector::ZeroVector)
		, VerticalOffset(0.0f)
		, ShoulderOffset(50.0f)
		, ForwardOffset(0.0f)
	{}
};

//========================================
// Lag Parameters - Sub-Step 1.2.6
//========================================

/**
 * State lag/smoothing parameters
 * Controls camera interpolation speeds for smooth movement
 */
USTRUCT(BlueprintType)
struct FStateLagParams
{
	GENERATED_BODY()

	/** Position interpolation speed (higher = faster response) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float PositionLagSpeed;

	/** Rotation interpolation speed (higher = faster response) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float RotationLagSpeed;

	/** Distance interpolation speed (higher = faster response) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float DistanceLagSpeed;

	/** FOV interpolation speed (higher = faster response) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float FOVLagSpeed;

	FStateLagParams()
		: PositionLagSpeed(10.0f)
		, RotationLagSpeed(10.0f)
		, DistanceLagSpeed(8.0f)
		, FOVLagSpeed(6.0f)
	{}
};

//========================================
// Transition Parameters - Sub-Step 1.2.7
//========================================

/**
 * State transition parameters
 * Controls how camera blends when entering/exiting this state
 */
USTRUCT(BlueprintType)
struct FStateTransitionParams
{
	GENERATED_BODY()

	/** Time to blend into this state (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float BlendInTime;

	/** Time to blend out of this state (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float BlendOutTime;

	/** Blend curve type for transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	ECameraBlendType BlendType;

	FStateTransitionParams()
		: BlendInTime(0.3f)
		, BlendOutTime(0.3f)
		, BlendType(ECameraBlendType::SmoothStep)
	{}
};

//========================================
// Collision Parameters - Sub-Step 1.2.8
//========================================

/**
 * State collision parameters
 * Controls collision behavior specific to this state
 */
USTRUCT(BlueprintType)
struct FStateCollisionParams
{
	GENERATED_BODY()

	/** Whether collision detection is enabled for this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bEnableCollision;

	/** Collision probe radius in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "5.0", ClampMax = "100.0", EditCondition = "bEnableCollision"))
	float CollisionRadius;

	/** Recovery delay after collision ends (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "0.0", ClampMax = "3.0", EditCondition = "bEnableCollision"))
	float RecoveryDelay;

	FStateCollisionParams()
		: bEnableCollision(true)
		, CollisionRadius(12.0f)
		, RecoveryDelay(0.5f)
	{}
};

/**
 * State auto-correction parameters
 * Controls automatic camera adjustments for this state
 */
USTRUCT(BlueprintType)
struct FStateAutoCorrectParams
{
	GENERATED_BODY()

	/** Enable auto-centering when no input */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoCorrect")
	bool bEnableAutoCenter;

	/** Delay before auto-centering starts (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoCorrect", meta = (ClampMin = "0.0", ClampMax = "10.0", EditCondition = "bEnableAutoCenter"))
	float AutoCenterDelay;

	FStateAutoCorrectParams()
		: bEnableAutoCenter(true)
		, AutoCenterDelay(3.0f)
	{}
};

//========================================
// Flag and Hierarchy Parameters - Sub-Step 1.2.9
//========================================

/**
 * State flag parameters
 * Boolean flags that define state characteristics
 */
USTRUCT(BlueprintType)
struct FStateFlagParams
{
	GENERATED_BODY()

	/** This state requires a locked target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bRequiresTarget;

	/** This state is a cinematic/cutscene state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bIsCinematic;

	/** Player input is ignored in this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
	bool bIgnoreInput;

	FStateFlagParams()
		: bRequiresTarget(false)
		, bIsCinematic(false)
		, bIgnoreInput(false)
	{}
};

/**
 * State hierarchy parameters
 * Defines parent-child relationships between states
 */
USTRUCT(BlueprintType)
struct FStateHierarchyParams
{
	GENERATED_BODY()

	/** Parent state to inherit parameters from (NAME_None = no parent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hierarchy")
	FName ParentState;

	/** If true, non-default values override parent values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hierarchy")
	bool bOverrideParent;

	FStateHierarchyParams()
		: ParentState(NAME_None)
		, bOverrideParent(false)
	{}
};

//========================================
// State Base Params (Combined) - Sub-Step 1.2.10
//========================================

/**
 * Complete state base parameters (A-Group)
 * Contains all 34 parameters organized in 11 sub-groups
 * This is the foundation for each camera state configuration
 */
USTRUCT(BlueprintType)
struct FStateBaseParams
{
	GENERATED_BODY()

	//========================================
	// Sub-Group A1: Identity (3 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A1_Identity")
	FStateIdentityParams Identity;

	//========================================
	// Sub-Group A2: Distance (3 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A2_Distance")
	FStateDistanceParams Distance;

	//========================================
	// Sub-Group A3: FOV (3 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A3_FOV")
	FStateFOVParams FOV;

	//========================================
	// Sub-Group A4: Rotation (2 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A4_Rotation")
	FStateRotationParams Rotation;

	//========================================
	// Sub-Group A5: Offset (6 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A5_Offset")
	FStateOffsetParams Offset;

	//========================================
	// Sub-Group A6: Lag (4 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A6_Lag")
	FStateLagParams Lag;

	//========================================
	// Sub-Group A7: Transition (3 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A7_Transition")
	FStateTransitionParams Transition;

	//========================================
	// Sub-Group A8: Collision (3 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A8_Collision")
	FStateCollisionParams Collision;

	//========================================
	// Sub-Group A9: AutoCorrect (2 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A9_AutoCorrect")
	FStateAutoCorrectParams AutoCorrect;

	//========================================
	// Sub-Group A10: Flags (3 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A10_Flags")
	FStateFlagParams Flags;

	//========================================
	// Sub-Group A11: Hierarchy (2 params)
	//========================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "A11_Hierarchy")
	FStateHierarchyParams Hierarchy;

	FStateBaseParams()
	{
		// All sub-structs use their own default constructors
	}
};
