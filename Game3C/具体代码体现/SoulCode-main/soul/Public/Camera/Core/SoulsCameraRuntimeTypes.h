// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/Color.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraRuntimeTypes.generated.h"

/**
 * Runtime data structures for the camera pipeline
 * These structures are used to pass data between pipeline stages
 * 
 * Structure Hierarchy:
 * - FCameraInputContext:  Pipeline input (Stage 1 output)
 * - FSoulsCameraOutput: Pipeline final output
 * - FModuleOutput/FModifierOutput: Intermediate results
 * - FCollisionOutput: Collision stage output (Stage 6)
 */

//========================================
// Runtime Enums for Pipeline
//========================================

/**
 * Camera output types for module results
 * Used to categorize what aspect of camera a module output affects
 */
UENUM(BlueprintType)
enum class ECameraOutputType : uint8
{
	OutputNone = 0              UMETA(DisplayName = "None"),
	OutputPosition              UMETA(DisplayName = "Position"),
	OutputRotation              UMETA(DisplayName = "Rotation"),
	OutputDistance              UMETA(DisplayName = "Distance"),
	OutputFOV                   UMETA(DisplayName = "FOV"),
	OutputOffset                UMETA(DisplayName = "Offset"),
	OutputConstraint            UMETA(DisplayName = "Constraint"),
	OutputLockOn                UMETA(DisplayName = "Lock On"),
	OutputCinematic             UMETA(DisplayName = "Cinematic"),
	OutputCustom                UMETA(DisplayName = "Custom"),
	
	OutputMAX                   UMETA(Hidden)
};

/**
 * Camera Module Category Enum
 * Used to identify the category of camera module producing output
 */
UENUM(BlueprintType)
enum class ECameraModuleCategory : uint8
{
	CategoryNone        UMETA(DisplayName = "None"),
	CategoryPosition    UMETA(DisplayName = "Position"),
	CategoryRotation    UMETA(DisplayName = "Rotation"),
	CategoryDistance    UMETA(DisplayName = "Distance"),
	CategoryFOV         UMETA(DisplayName = "FOV"),
	CategoryOffset      UMETA(DisplayName = "Offset"),
	CategoryConstraint  UMETA(DisplayName = "Constraint"),
	CategoryLockOn      UMETA(DisplayName = "LockOn"),
	CategoryFollow      UMETA(DisplayName = "Follow"),
	CategoryOrbit       UMETA(DisplayName = "Orbit"),
	CategoryCinematic   UMETA(DisplayName = "Cinematic")
};

/**
 * Blend Policy Enum
 * Defines how module outputs should be combined
 */
UENUM(BlueprintType)
enum class EBlendPolicy : uint8
{
	Override        UMETA(DisplayName = "Override"),
	Additive        UMETA(DisplayName = "Additive"),
	Multiplicative  UMETA(DisplayName = "Multiplicative"),
	Minimum         UMETA(DisplayName = "Minimum"),
	Maximum         UMETA(DisplayName = "Maximum"),
	Blend           UMETA(DisplayName = "Blend")
};

/**
 * Camera Modifier Type Enum
 * Used to identify the type of camera modifier producing effects
 */
UENUM(BlueprintType)
enum class ECameraModifierType : uint8
{
	None            UMETA(DisplayName = "None"),
	Shake           UMETA(DisplayName = "Shake"),
	Reaction        UMETA(DisplayName = "Reaction"),
	Cinematic       UMETA(DisplayName = "Cinematic"),
	Zoom            UMETA(DisplayName = "Zoom"),
	Effect          UMETA(DisplayName = "Effect"),
	Override        UMETA(DisplayName = "Override"),
	TimeDilation    UMETA(DisplayName = "TimeDilation"),
	PostProcess     UMETA(DisplayName = "PostProcess"),
	Special         UMETA(DisplayName = "Special")
};

/**
 * EModifierState
 * Lifecycle state for camera modifiers
 */
UENUM(BlueprintType)
enum class EModifierState : uint8
{
	Inactive        UMETA(DisplayName = "Inactive"),
	BlendingIn      UMETA(DisplayName = "Blending In"),
	Active          UMETA(DisplayName = "Active"),
	BlendingOut     UMETA(DisplayName = "Blending Out"),
	Paused          UMETA(DisplayName = "Paused")
};

//========================================
// FCameraInputContext - Pipeline Input (Stage 1 Output)
//========================================
/**
 * FCameraInputContext
 * Input data gathered at the start of each frame
 * Used by Stage 1 (InputGather) and passed to all subsequent stages
 */
USTRUCT(BlueprintType)
struct SOUL_API FCameraInputContext
{
	GENERATED_BODY()

	//========================================
	// Timing
	//========================================
	
	/** Delta time for this frame */
	UPROPERTY(BlueprintReadWrite, Category = "Timing")
	float DeltaTime;

	/** Total game time */
	UPROPERTY(BlueprintReadWrite, Category = "Timing")
	float GameTime;

	//========================================
	// Character Info
	//========================================
	
	/** Character world location */
	UPROPERTY(BlueprintReadWrite, Category = "Character")
	FVector CharacterLocation;

	/** Character world rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Character")
	FRotator CharacterRotation;

	/** Character velocity */
	UPROPERTY(BlueprintReadWrite, Category = "Character")
	FVector CharacterVelocity;

	/** Character speed (2D, ignoring Z) */
	UPROPERTY(BlueprintReadWrite, Category = "Character")
	float CharacterSpeed;

	/** Reference to character actor */
	TWeakObjectPtr<AActor> CharacterActor;

	//========================================
	// Movement State
	//========================================
	
	/** Is character moving */
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsMoving;

	/** Is character sprinting */
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsSprinting;

	/** Is character crouching */
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsCrouching;

	/** Is character in air (jumping/falling) */
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsInAir;

	/** Is character climbing */
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsClimbing;

	/** Is character swimming */
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsSwimming;

	/** Is character mounted */
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	bool bIsMounted;

	//========================================
	// Combat State
	//========================================
	
	/** Is in combat mode */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsInCombat;

	/** Is blocking/guarding */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsBlocking;

	/** Is attacking */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsAttacking;

	/** Is aiming (ranged/magic) */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsAiming;

	/** Health ratio (0-1) */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float HealthRatio;

	//========================================
	// Target Info (Lock-On)
	//========================================
	
	/** Has a lock-on target */
	UPROPERTY(BlueprintReadWrite, Category = "Target")
	bool bHasTarget;

	/** Target world location */
	UPROPERTY(BlueprintReadWrite, Category = "Target")
	FVector TargetLocation;

	/** Distance to target */
	UPROPERTY(BlueprintReadWrite, Category = "Target")
	float TargetDistance;

	/** Target size for framing */
	UPROPERTY(BlueprintReadWrite, Category = "Target")
	float TargetSize;

	/** Is target a boss */
	UPROPERTY(BlueprintReadWrite, Category = "Target")
	bool bTargetIsBoss;

	/** Reference to target actor */
	TWeakObjectPtr<AActor> TargetActor;

	//========================================
	// Player Input
	//========================================
	
	/** Raw camera input from player (Yaw, Pitch) */
	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FVector2D CameraInput;

	/** Camera input magnitude */
	UPROPERTY(BlueprintReadWrite, Category = "Input")
	float InputMagnitude;

	/** Time since last input */
	UPROPERTY(BlueprintReadWrite, Category = "Input")
	float TimeSinceLastInput;

	//========================================
	// Previous Frame Data
	//========================================
	
	/** Previous frame camera rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Previous")
	FRotator PreviousCameraRotation;

	/** Previous frame camera location */
	UPROPERTY(BlueprintReadWrite, Category = "Previous")
	FVector PreviousCameraLocation;

	//========================================
	// Environment
	//========================================
	
	/** Is in tight space */
	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	bool bInTightSpace;

	/** Is under low ceiling */
	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	bool bUnderLowCeiling;

	/** Is near cliff edge */
	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	bool bNearCliffEdge;

	/** Water surface height (for underwater detection) */
	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float WaterSurfaceHeight;

	/** Is camera underwater */
	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	bool bIsUnderwater;

	//========================================
	// Constructor & Methods
	//========================================

	FCameraInputContext()
		: DeltaTime(0.0f)
		, GameTime(0.0f)
		, CharacterLocation(FVector::ZeroVector)
		, CharacterRotation(FRotator::ZeroRotator)
		, CharacterVelocity(FVector::ZeroVector)
		, CharacterSpeed(0.0f)
		, bIsMoving(false)
		, bIsSprinting(false)
		, bIsCrouching(false)
		, bIsInAir(false)
		, bIsClimbing(false)
		, bIsSwimming(false)
		, bIsMounted(false)
		, bIsInCombat(false)
		, bIsBlocking(false)
		, bIsAttacking(false)
		, bIsAiming(false)
		, HealthRatio(1.0f)
		, bHasTarget(false)
		, TargetLocation(FVector::ZeroVector)
		, TargetDistance(0.0f)
		, TargetSize(100.0f)
		, bTargetIsBoss(false)
		, CameraInput(FVector2D::ZeroVector)
		, InputMagnitude(0.0f)
		, TimeSinceLastInput(0.0f)
		, PreviousCameraRotation(FRotator::ZeroRotator)
		, PreviousCameraLocation(FVector::ZeroVector)
		, bInTightSpace(false)
		, bUnderLowCeiling(false)
		, bNearCliffEdge(false)
		, WaterSurfaceHeight(-99999.0f)
		, bIsUnderwater(false)
	{}

	/** Reset to default values */
	void Reset()
	{
		*this = FCameraInputContext();
	}

	/** Check if target is valid */
	FORCEINLINE bool HasValidTarget() const
	{
		return bHasTarget && TargetActor.IsValid();
	}

	/** Get target actor safely */
	FORCEINLINE AActor* GetTargetActor() const
	{
		return TargetActor.Get();
	}

	/** Set target actor */
	void SetTargetActor(AActor* InTargetActor)
	{
		TargetActor = InTargetActor;
		bHasTarget = (InTargetActor != nullptr);
	}

	/** Get character actor safely */
	FORCEINLINE AActor* GetCharacterActor() const
	{
		return CharacterActor.Get();
	}

	/** Set character actor */
	void SetCharacterActor(AActor* InCharacterActor)
	{
		CharacterActor = InCharacterActor;
	}

	/** Get direction from character to target */
	FORCEINLINE FVector GetDirectionToTarget() const
	{
		if (!bHasTarget)
		{
			return FVector::ZeroVector;
		}
		FVector Direction = TargetLocation - CharacterLocation;
		Direction.Normalize();
		return Direction;
	}

	/** Check if player has recent input */
	FORCEINLINE bool HasRecentInput(float Threshold = 0.5f) const
	{
		return TimeSinceLastInput < Threshold;
	}
};

//========================================
// FCollisionOutput - Stage 6 Output
//========================================
/**
 * FCollisionOutput
 * Output from Stage 6 (CollisionResolve)
 * Contains all camera adjustments from collision handling
 */
USTRUCT(BlueprintType)
struct SOUL_API FCollisionOutput
{
	GENERATED_BODY()

	//========================================
	// Camera Adjustments
	//========================================
	
	/** Position adjustment from collision */
	UPROPERTY(BlueprintReadOnly, Category = "Collision")
	FVector PositionAdjustment;

	/** Distance adjustment (negative = closer) */
	UPROPERTY(BlueprintReadOnly, Category = "Collision")
	float DistanceAdjustment;

	/** Rotation adjustment */
	UPROPERTY(BlueprintReadOnly, Category = "Collision")
	FRotator RotationAdjustment;

	/** FOV adjustment */
	UPROPERTY(BlueprintReadOnly, Category = "Collision")
	float FOVAdjustment;

	//========================================
	// Collision State
	//========================================
	
	/** Is collision currently active */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
	bool bCollisionActive;

	/** Is in recovery mode */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
	bool bIsRecovering;

	/** Current safe distance */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
	float SafeDistance;

	/** Desired distance (before collision) */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
	float DesiredDistance;

	//========================================
	// Occlusion State
	//========================================
	
	/** Is character occluded from camera view */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|Occlusion")
	bool bCharacterOccluded;

	/** Is target occluded from camera view */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|Occlusion")
	bool bTargetOccluded;

	/** Character fade alpha (1 = visible, 0 = invisible) */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|Occlusion")
	float CharacterFadeAlpha;

	//========================================
	// Special Environment State
	//========================================
	
	/** Is in tight space */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|Special")
	bool bInTightSpace;

	/** Tight space factor (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|Special")
	float TightSpaceFactor;

	/** Is near cliff edge */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|Special")
	bool bNearCliffEdge;

	/** Is in corner */
	UPROPERTY(BlueprintReadOnly, Category = "Collision|Special")
	bool bInCorner;

	//========================================
	// Constructor
	//========================================

	FCollisionOutput()
		: PositionAdjustment(FVector::ZeroVector)
		, DistanceAdjustment(0.0f)
		, RotationAdjustment(FRotator::ZeroRotator)
		, FOVAdjustment(0.0f)
		, bCollisionActive(false)
		, bIsRecovering(false)
		, SafeDistance(0.0f)
		, DesiredDistance(0.0f)
		, bCharacterOccluded(false)
		, bTargetOccluded(false)
		, CharacterFadeAlpha(1.0f)
		, bInTightSpace(false)
		, TightSpaceFactor(0.0f)
		, bNearCliffEdge(false)
		, bInCorner(false)
	{}

	/** Reset to default values */
	void Reset()
	{
		*this = FCollisionOutput();
	}

	/** Check if any adjustment is active */
	bool HasAdjustments() const
	{
		return !PositionAdjustment.IsNearlyZero() ||
		       !FMath::IsNearlyZero(DistanceAdjustment) ||
		       !RotationAdjustment.IsNearlyZero() ||
		       !FMath::IsNearlyZero(FOVAdjustment);
	}
};

//========================================
// FSoulsCameraOutput - Pipeline Output (Stage 5-8)
//========================================
/**
 * FSoulsCameraOutput
 * Final camera output after all 8 pipeline stages
 * Applied to SpringArm and Camera components
 */
USTRUCT(BlueprintType)
struct SOUL_API FSoulsCameraOutput
{
	GENERATED_BODY()

	//========================================
	// Core Transform
	//========================================
	
	/** Focus point (where camera looks at) */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Core")
	FVector FocusPoint;

	/** Camera rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Core")
	FRotator Rotation;

	/** Camera distance from focus point */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Core")
	float Distance;

	//========================================
	// Field of View
	//========================================
	
	/** Field of view */
	UPROPERTY(BlueprintReadWrite, Category = "Output|FOV")
	float FOV;

	//========================================
	// Offsets
	//========================================
	
	/** Socket offset (local space offset at camera end) */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Offset")
	FVector SocketOffset;

	/** Target offset (offset applied to focus point) */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Offset")
	FVector TargetOffset;

	//========================================
	// Post Process
	//========================================
	
	/** Vignette intensity (0-1) */
	UPROPERTY(BlueprintReadWrite, Category = "Output|PostProcess")
	float VignetteIntensity;

	/** Color tint */
	UPROPERTY(BlueprintReadWrite, Category = "Output|PostProcess")
	FLinearColor ColorTint;

	/** Motion blur amount */
	UPROPERTY(BlueprintReadWrite, Category = "Output|PostProcess")
	float MotionBlurAmount;

	//========================================
	// DOF Settings (Depth of Field)
	//========================================

	/** DOF focal distance */
	UPROPERTY(BlueprintReadWrite, Category = "Output|PostProcess")
	float DOFFocalDistance;

	/** DOF focal region */
	UPROPERTY(BlueprintReadWrite, Category = "Output|PostProcess")
	float DOFFocalRegion;

	/** DOF near blur size */
	UPROPERTY(BlueprintReadWrite, Category = "Output|PostProcess")
	float DOFNearBlurSize;

	/** DOF far blur size */
	UPROPERTY(BlueprintReadWrite, Category = "Output|PostProcess")
	float DOFFarBlurSize;

	//========================================
	// Time Control
	//========================================

	/** Time dilation (1 = normal) */
	UPROPERTY(BlueprintReadWrite, Category = "Output|TimeControl")
	float TimeDilation;

	//========================================
	// State Info
	//========================================
	
	/** Current camera state name */
	UPROPERTY(BlueprintReadWrite, Category = "Output|State")
	FName CurrentStateName;

	/** State blend alpha (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Output|State")
	float StateBlendAlpha;

	/** Is in state transition */
	UPROPERTY(BlueprintReadOnly, Category = "Output|State")
	bool bIsInTransition;

	/** Is output valid */
	UPROPERTY(BlueprintReadWrite, Category = "Output|State")
	bool bIsValid;

	//========================================
	// Collision Info
	//========================================
	
	/** Was adjusted by collision */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Collision")
	bool bCollisionAdjusted;

	/** Distance before collision adjustment */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Collision")
	float PreCollisionDistance;

	/** Collision compression ratio (1 = no compression) */
	UPROPERTY(BlueprintReadWrite, Category = "Output|Collision")
	float CollisionCompressionRatio;

	//========================================
	// Constructor & Methods
	//========================================

	FSoulsCameraOutput()
		: FocusPoint(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Distance(400.0f)
		, FOV(70.0f)
		, SocketOffset(FVector::ZeroVector)
		, TargetOffset(FVector::ZeroVector)
		, VignetteIntensity(0.0f)
		, ColorTint(FLinearColor::White)
		, MotionBlurAmount(0.5f)
		, DOFFocalDistance(0.0f)
		, DOFFocalRegion(0.0f)
		, DOFNearBlurSize(0.0f)
		, DOFFarBlurSize(0.0f)
		, TimeDilation(1.0f)
		, CurrentStateName(NAME_None)
		, StateBlendAlpha(1.0f)
		, bIsInTransition(false)
		, bIsValid(false)
		, bCollisionAdjusted(false)
		, PreCollisionDistance(400.0f)
		, CollisionCompressionRatio(1.0f)
	{}

	/** Reset to default values */
	void Reset()
	{
		*this = FSoulsCameraOutput();
	}

	/** Calculate camera world location */
	FORCEINLINE FVector GetCameraLocation() const
	{
		FVector Direction = Rotation.Vector() * -1.0f;
		return FocusPoint + Direction * Distance + SocketOffset;
	}

	/** Get look direction */
	FORCEINLINE FVector GetLookDirection() const
	{
		return Rotation.Vector();
	}

	/** Check if camera is compressed by collision */
	FORCEINLINE bool IsCompressed(float Threshold = 0.9f) const
	{
		return bCollisionAdjusted && CollisionCompressionRatio < Threshold;
	}

	/** Lerp between two camera outputs */
	static FSoulsCameraOutput Lerp(const FSoulsCameraOutput& A, const FSoulsCameraOutput& B, float Alpha)
	{
		FSoulsCameraOutput Result;
		
		Result.FocusPoint = FMath::Lerp(A.FocusPoint, B.FocusPoint, Alpha);
		Result.Rotation = FMath::Lerp(A.Rotation, B.Rotation, Alpha);
		Result.Distance = FMath::Lerp(A.Distance, B.Distance, Alpha);
		Result.FOV = FMath::Lerp(A.FOV, B.FOV, Alpha);
		Result.SocketOffset = FMath::Lerp(A.SocketOffset, B.SocketOffset, Alpha);
		Result.TargetOffset = FMath::Lerp(A.TargetOffset, B.TargetOffset, Alpha);
		Result.VignetteIntensity = FMath::Lerp(A.VignetteIntensity, B.VignetteIntensity, Alpha);
		Result.ColorTint = FMath::Lerp(A.ColorTint, B.ColorTint, Alpha);
		Result.MotionBlurAmount = FMath::Lerp(A.MotionBlurAmount, B.MotionBlurAmount, Alpha);
		Result.DOFFocalDistance = FMath::Lerp(A.DOFFocalDistance, B.DOFFocalDistance, Alpha);
		Result.DOFFocalRegion = FMath::Lerp(A.DOFFocalRegion, B.DOFFocalRegion, Alpha);
		Result.DOFNearBlurSize = FMath::Lerp(A.DOFNearBlurSize, B.DOFNearBlurSize, Alpha);
		Result.DOFFarBlurSize = FMath::Lerp(A.DOFFarBlurSize, B.DOFFarBlurSize, Alpha);
		Result.TimeDilation = FMath::Lerp(A.TimeDilation, B.TimeDilation, Alpha);
		Result.CurrentStateName = B.CurrentStateName;
		Result.StateBlendAlpha = Alpha;
		Result.bIsInTransition = (Alpha > 0.0f && Alpha < 1.0f);
		Result.bIsValid = B.bIsValid;
		Result.bCollisionAdjusted = B.bCollisionAdjusted;
		Result.PreCollisionDistance = B.PreCollisionDistance;
		Result.CollisionCompressionRatio = B.CollisionCompressionRatio;
		
		return Result;
	}
};

//========================================
// FModuleOutput - Single Module Output (Stage 3)
//========================================
/**
 * FModuleOutput
 * Output from a single camera module (Stage 3)
 */
USTRUCT(BlueprintType)
struct SOUL_API FModuleOutput
{
	GENERATED_BODY()

	//========================================
	// Module Identification
	//========================================

	/** Which module produced this output (by ID) */
	UPROPERTY(BlueprintReadOnly, Category = "Module")
	ECameraModuleID ModuleID;

	/** Module category */
	UPROPERTY(BlueprintReadOnly, Category = "Module")
	ECameraModuleCategory ModuleCategory;

	/** Module priority for sorting */
	UPROPERTY(BlueprintReadOnly, Category = "Module")
	int32 Priority;

	/** Blend policy for combining with other outputs */
	UPROPERTY(BlueprintReadOnly, Category = "Module")
	EBlendPolicy BlendPolicy;

	/** Output weight for blending */
	UPROPERTY(BlueprintReadOnly, Category = "Module")
	float Weight;

	//========================================
	// Output Values
	//========================================

	/** Position contribution */
	UPROPERTY(BlueprintReadWrite, Category = "Output")
	FVector PositionOffset;

	/** Has position output */
	UPROPERTY(BlueprintReadOnly, Category = "Output")
	bool bHasPositionOutput;

	/** Rotation contribution */
	UPROPERTY(BlueprintReadWrite, Category = "Output")
	FRotator RotationOutput;

	/** Has rotation output */
	UPROPERTY(BlueprintReadOnly, Category = "Output")
	bool bHasRotationOutput;

	/** Distance contribution */
	UPROPERTY(BlueprintReadWrite, Category = "Output")
	float DistanceOutput;

	/** Has distance output */
	UPROPERTY(BlueprintReadOnly, Category = "Output")
	bool bHasDistanceOutput;

	/** Is distance a multiplier (vs additive) */
	UPROPERTY(BlueprintReadOnly, Category = "Output")
	bool bDistanceIsMultiplier;

	/** FOV contribution */
	UPROPERTY(BlueprintReadWrite, Category = "Output")
	float FOVOutput;

	/** Has FOV output */
	UPROPERTY(BlueprintReadOnly, Category = "Output")
	bool bHasFOVOutput;

	/** Socket offset contribution */
	UPROPERTY(BlueprintReadWrite, Category = "Output")
	FVector SocketOffsetOutput;

	/** Has socket offset output */
	UPROPERTY(BlueprintReadOnly, Category = "Output")
	bool bHasSocketOffsetOutput;

	/** 解算后的 3D 焦点位置（由 Framing Module 输出） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
	FVector FocusPoint = FVector::ZeroVector;

	/** FocusPoint 是否有效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
	bool bHasFocusPoint = false;

	//========================================
	// Constructor
	//========================================

	FModuleOutput()
		: ModuleID(ECameraModuleID::None)
		, ModuleCategory(ECameraModuleCategory::CategoryNone)
		, Priority(0)
		, BlendPolicy(EBlendPolicy::Blend)
		, Weight(1.0f)
		, PositionOffset(FVector::ZeroVector)
		, bHasPositionOutput(false)
		, RotationOutput(FRotator::ZeroRotator)
		, bHasRotationOutput(false)
		, DistanceOutput(0.0f)
		, bHasDistanceOutput(false)
		, bDistanceIsMultiplier(false)
		, FOVOutput(0.0f)
		, bHasFOVOutput(false)
		, SocketOffsetOutput(FVector::ZeroVector)
		, bHasSocketOffsetOutput(false)
		, FocusPoint(FVector::ZeroVector)
		, bHasFocusPoint(false)
	{}

	//========================================
	// Factory Methods
	//========================================

	static FModuleOutput MakePosition(ECameraModuleCategory Category, const FVector& Position, int32 InPriority = 100, EBlendPolicy Policy = EBlendPolicy::Blend)
	{
		FModuleOutput Output;
		Output.ModuleCategory = Category;
		Output.Priority = InPriority;
		Output.BlendPolicy = Policy;
		Output.PositionOffset = Position;
		Output.bHasPositionOutput = true;
		return Output;
	}

	static FModuleOutput MakeRotation(ECameraModuleCategory Category, const FRotator& InRotation, int32 InPriority = 100, EBlendPolicy Policy = EBlendPolicy::Blend)
	{
		FModuleOutput Output;
		Output.ModuleCategory = Category;
		Output.Priority = InPriority;
		Output.BlendPolicy = Policy;
		Output.RotationOutput = InRotation;
		Output.bHasRotationOutput = true;
		return Output;
	}

	static FModuleOutput MakeDistance(ECameraModuleCategory Category, float InDistance, bool bIsMultiplier = false, int32 InPriority = 100, EBlendPolicy Policy = EBlendPolicy::Blend)
	{
		FModuleOutput Output;
		Output.ModuleCategory = Category;
		Output.Priority = InPriority;
		Output.BlendPolicy = Policy;
		Output.DistanceOutput = InDistance;
		Output.bHasDistanceOutput = true;
		Output.bDistanceIsMultiplier = bIsMultiplier;
		return Output;
	}

	static FModuleOutput MakeFOV(ECameraModuleCategory Category, float InFOV, int32 InPriority = 100, EBlendPolicy Policy = EBlendPolicy::Additive)
	{
		FModuleOutput Output;
		Output.ModuleCategory = Category;
		Output.Priority = InPriority;
		Output.BlendPolicy = Policy;
		Output.FOVOutput = InFOV;
		Output.bHasFOVOutput = true;
		return Output;
	}
};

//========================================
// FModifierOutput - Single Modifier Output (Stage 4)
//========================================
/**
 * FModifierOutput
 * Output from a single camera modifier (Stage 4)
 */
USTRUCT(BlueprintType)
struct SOUL_API FModifierOutput
{
	GENERATED_BODY()

	//========================================
	// Modifier Identification
	//========================================

	/** Which modifier produced this output (by ID) */
	UPROPERTY(BlueprintReadOnly, Category = "Modifier")
	ECameraModifierID ModifierID;

	/** Modifier type category */
	UPROPERTY(BlueprintReadOnly, Category = "Modifier")
	ECameraModifierType ModifierType;

	/** Current modifier state */
	UPROPERTY(BlueprintReadOnly, Category = "Modifier")
	EModifierState State;

	/** Output weight for blending */
	UPROPERTY(BlueprintReadOnly, Category = "Modifier")
	float CurrentWeight;

	/** Time remaining for timed modifiers */
	UPROPERTY(BlueprintReadOnly, Category = "Modifier")
	float TimeRemaining;

	//========================================
	// Effect Values
	//========================================

	/** Position offset */
	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	FVector PositionEffect;

	/** Rotation offset */
	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	FRotator RotationEffect;

	/** Distance offset */
	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	float DistanceEffect;

	/** FOV offset */
	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	float FOVEffect;

	//========================================
	// Post Process Effects
	//========================================

	/** Vignette intensity (0-1) */
	UPROPERTY(BlueprintReadWrite, Category = "PostProcess")
	float VignetteEffect;

	/** Color tint */
	UPROPERTY(BlueprintReadWrite, Category = "PostProcess")
	FLinearColor ColorTintEffect;

	/** Saturation multiplier */
	UPROPERTY(BlueprintReadWrite, Category = "PostProcess")
	float SaturationEffect;

	//========================================
	// Special Effects
	//========================================

	/** Time dilation (1 = normal) */
	UPROPERTY(BlueprintReadWrite, Category = "Special")
	float TimeDilationEffect;

	/** Should override camera completely */
	UPROPERTY(BlueprintReadOnly, Category = "Special")
	bool bOverrideCamera;

	/** Override transform (if bOverrideCamera) */
	UPROPERTY(BlueprintReadWrite, Category = "Special")
	FTransform OverrideTransform;

	//========================================
	// Constructor
	//========================================

	FModifierOutput()
		: ModifierID(ECameraModifierID::None)
		, ModifierType(ECameraModifierType::None)
		, State(EModifierState::Inactive)
		, CurrentWeight(0.0f)
		, TimeRemaining(0.0f)
		, PositionEffect(FVector::ZeroVector)
		, RotationEffect(FRotator::ZeroRotator)
		, DistanceEffect(0.0f)
		, FOVEffect(0.0f)
		, VignetteEffect(0.0f)
		, ColorTintEffect(FLinearColor::White)
		, SaturationEffect(1.0f)
		, TimeDilationEffect(1.0f)
		, bOverrideCamera(false)
		, OverrideTransform(FTransform::Identity)
	{}

	/** Check if modifier is active */
	FORCEINLINE bool IsActive() const
	{
		return State != EModifierState::Inactive && CurrentWeight > 0.0f;
	}

	/** Check if this is an override modifier */
	FORCEINLINE bool IsOverride() const
	{
		return bOverrideCamera && IsActive();
	}
};