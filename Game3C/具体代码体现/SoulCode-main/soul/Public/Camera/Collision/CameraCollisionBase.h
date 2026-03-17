// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Pipeline/ICameraStage.h"
#include "CameraCollisionBase.generated.h"

// Forward declarations
struct FCameraStateConfig;
class USoulsCameraManager;

// ECollisionCategory and ECollisionStrategyID are now defined in CameraStateEnums.h

/**
 * Collision Detection Result
 * 
 * Contains all information gathered during collision detection phase.
 * Used by Response strategies to determine how to adjust the camera.
 */
USTRUCT(BlueprintType)
struct SOUL_API FCollisionDetectionResult
{
    GENERATED_BODY()

    //========================================
    // Basic Collision Info
    //========================================

    /** Did any collision occur */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    bool bCollisionDetected;

    /** Primary collision hit result */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FHitResult HitResult;

    /** All collision hits (for multi-ray detection) */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    TArray<FHitResult> AllHits;

    //========================================
    // Safe Position Info
    //========================================

    /** Safe distance from target (where camera should be) */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float SafeDistance;

    /** Original desired distance */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float DesiredDistance;

    /** Safe location for camera */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector SafeLocation;

    /** Original desired location */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector DesiredLocation;

    //========================================
    // Collision Geometry Info
    //========================================

    /** Primary collision normal */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector CollisionNormal;

    /** Distance from camera to collision point */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float CollisionDistance;

    /** How much the camera needs to adjust (0-1, 1 = full adjustment needed) */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float AdjustmentRatio;

    //========================================
    // Occlusion Info
    //========================================

    /** Is player character occluded from camera view */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Occlusion")
    bool bCharacterOccluded;

    /** Is lock-on target occluded from camera view */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Occlusion")
    bool bTargetOccluded;

    /** Actors blocking the view (not Blueprint accessible due to TWeakObjectPtr) */
    TArray<TWeakObjectPtr<AActor>> OccludingActors;

    //========================================
    // Environment Info
    //========================================

    /** Is camera in a tight/confined space */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Environment")
    bool bInTightSpace;

    /** Is camera underwater */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Environment")
    bool bUnderwater;

    /** Is camera transitioning between indoor/outdoor */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Environment")
    bool bEnvironmentTransition;

    //========================================
    // Timing Info
    //========================================

    /** How long has collision been active */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Timing")
    float CollisionDuration;

    /** Time since last collision */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Timing")
    float TimeSinceLastCollision;

    //========================================
    // Constructor & Methods
    //========================================

    FCollisionDetectionResult()
        : bCollisionDetected(false)
        , SafeDistance(0.0f)
        , DesiredDistance(0.0f)
        , SafeLocation(FVector::ZeroVector)
        , DesiredLocation(FVector::ZeroVector)
        , CollisionNormal(FVector::ZeroVector)
        , CollisionDistance(0.0f)
        , AdjustmentRatio(0.0f)
        , bCharacterOccluded(false)
        , bTargetOccluded(false)
        , bInTightSpace(false)
        , bUnderwater(false)
        , bEnvironmentTransition(false)
        , CollisionDuration(0.0f)
        , TimeSinceLastCollision(0.0f)
    {}

    /** Reset all values to default */
    void Reset()
    {
        bCollisionDetected = false;
        HitResult = FHitResult();
        AllHits.Reset();
        SafeDistance = 0.0f;
        DesiredDistance = 0.0f;
        SafeLocation = FVector::ZeroVector;
        DesiredLocation = FVector::ZeroVector;
        CollisionNormal = FVector::ZeroVector;
        CollisionDistance = 0.0f;
        AdjustmentRatio = 0.0f;
        bCharacterOccluded = false;
        bTargetOccluded = false;
        OccludingActors.Reset();
        bInTightSpace = false;
        bUnderwater = false;
        bEnvironmentTransition = false;
        CollisionDuration = 0.0f;
        TimeSinceLastCollision = 0.0f;
    }

    /** Check if any occlusion is active */
    bool HasOcclusion() const
    {
        return bCharacterOccluded || bTargetOccluded;
    }

    /** Check if in special environment */
    bool IsInSpecialEnvironment() const
    {
        return bInTightSpace || bUnderwater || bEnvironmentTransition;
    }
};

/**
 * Collision Response Output
 * 
 * Contains the camera adjustments calculated by a collision strategy.
 * Multiple responses can be blended together based on weight and priority.
 */
USTRUCT(BlueprintType)
struct SOUL_API FCollisionResponseOutput
{
    GENERATED_BODY()

    //========================================
    // Strategy Info
    //========================================

    /** Strategy that produced this response */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    ECollisionStrategyID StrategyType;

    /** Strategy category */
    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    ECollisionCategory Category;

    //========================================
    // Camera Adjustments
    //========================================

    /** Position offset to apply */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Adjustment")
    FVector PositionAdjustment;

    /** Distance adjustment (negative = closer) */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Adjustment")
    float DistanceAdjustment;

    /** Rotation adjustment */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Adjustment")
    FRotator RotationAdjustment;

    /** FOV adjustment */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Adjustment")
    float FOVAdjustment;

    //========================================
    // Blending Info
    //========================================

    /** Response weight for blending (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Blending")
    float Weight;

    /** Priority for conflict resolution (higher = more important) */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Blending")
    int32 Priority;

    /** Should this response override others completely */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Blending")
    bool bOverride;

    /** Blend time for this response */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|Blending")
    float BlendTime;

    //========================================
    // State Info
    //========================================

    /** Is this response currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
    bool bIsActive;

    /** How long this response has been active */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
    float ActiveDuration;

    //========================================
    // Constructor & Methods
    //========================================

    FCollisionResponseOutput()
        : StrategyType(ECollisionStrategyID::CollisionNone)
        , Category(ECollisionCategory::Detection)
        , PositionAdjustment(FVector::ZeroVector)
        , DistanceAdjustment(0.0f)
        , RotationAdjustment(FRotator::ZeroRotator)
        , FOVAdjustment(0.0f)
        , Weight(1.0f)
        , Priority(100)
        , bOverride(false)
        , BlendTime(0.2f)
        , bIsActive(false)
        , ActiveDuration(0.0f)
    {}

    /** Reset all values to default */
    void Reset()
    {
        StrategyType = ECollisionStrategyID::CollisionNone;
        Category = ECollisionCategory::Detection;
        PositionAdjustment = FVector::ZeroVector;
        DistanceAdjustment = 0.0f;
        RotationAdjustment = FRotator::ZeroRotator;
        FOVAdjustment = 0.0f;
        Weight = 1.0f;
        Priority = 100;
        bOverride = false;
        BlendTime = 0.2f;
        bIsActive = false;
        ActiveDuration = 0.0f;
    }

    /** Check if this response has any actual adjustments */
    bool HasAdjustments() const
    {
        return !PositionAdjustment.IsNearlyZero() ||
               !FMath::IsNearlyZero(DistanceAdjustment) ||
               !RotationAdjustment.IsNearlyZero() ||
               !FMath::IsNearlyZero(FOVAdjustment);
    }
};

/**
 * UCameraCollisionBase
 * 
 * Abstract base class for all 20 collision strategies.
 * 
 * Collision strategies handle different aspects of camera-environment interaction:
 * - Detection (D01-D04): How to detect collisions
 * - Response (RS01-RS05): How to respond to collisions
 * - Occlusion (OC01-OC04): Handle character/target being blocked
 * - Recovery (RC01-RC03): Return to normal after collision ends
 * - Special (SP01-SP04): Unique scenarios (tight spaces, low ceiling, etc.)
 * 
 * Each strategy:
 * - Has a unique ECollisionStrategyID
 * - Belongs to one ECollisionCategory
 * - Can check if it should execute
 * - Produces FCollisionResponseOutput
 * 
 * The CollisionResolver manages all strategies and combines their outputs.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SOUL_API UCameraCollisionBase : public UObject
{
    GENERATED_BODY()

public:
    UCameraCollisionBase();

    //========================================
    // Strategy Identity
    //========================================

    /**
     * Get the collision strategy type (unique identifier)
     * Must be overridden by derived classes
     * @return The strategy's ECollisionStrategyID
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual ECollisionStrategyID GetStrategyType() const PURE_VIRTUAL(UCameraCollisionBase::GetStrategyType, return ECollisionStrategyID::CollisionNone;);

    /**
     * Get the strategy category
     * @return The strategy's category (Detection, Response, etc.)
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual ECollisionCategory GetStrategyCategory() const PURE_VIRTUAL(UCameraCollisionBase::GetStrategyCategory, return ECollisionCategory::Detection;);

    /**
     * Get human-readable strategy name
     * @return Strategy name for debugging
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual FName GetStrategyName() const;

    /**
     * Get strategy description
     * @return Human-readable description
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual FString GetStrategyDescription() const;

    //========================================
    // Strategy Execution
    //========================================

    /**
     * Check if this strategy should execute given current context
     * @param Context Current pipeline execution context
     * @return True if strategy should execute
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual bool ShouldExecute(const FStageExecutionContext& Context) const;

    /**
     * Execute the collision strategy
     * @param Context Current pipeline execution context
     * @param DetectionResult Collision detection results (from Detection strategies)
     * @param OutResponse Output response with camera adjustments
     * @return True if strategy produced a valid response
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) PURE_VIRTUAL(UCameraCollisionBase::Execute, return false;);

    /**
     * Update strategy state each frame
     * Called even when strategy is not actively executing
     * @param DeltaTime Time since last frame
     * @param Context Current pipeline execution context
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    virtual void Update(float DeltaTime, const FStageExecutionContext& Context);

    /**
     * Reset strategy to initial state
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    virtual void Reset();

    //========================================
    // Priority & Blending
    //========================================

    /**
     * Get strategy priority (higher = executes first, wins conflicts)
     * @return Priority value (default 100)
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual int32 GetPriority() const { return Priority; }

    /**
     * Get blend weight for this strategy
     * @return Weight value (0-1)
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual float GetBlendWeight() const { return BlendWeight; }

    /**
     * Check if this strategy can blend with others
     * @return True if blendable
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    virtual bool CanBlend() const { return bCanBlend; }

    //========================================
    // State Management
    //========================================

    /** Is strategy currently active (producing output) */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    bool IsActive() const { return bIsActive; }

    /** Set strategy active state */
    void SetActive(bool bNewActive) { bIsActive = bNewActive; }

    /** Is strategy enabled (can execute) */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    bool IsEnabled() const { return bIsEnabled; }

    /** Set strategy enabled state */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    void SetEnabled(bool bNewEnabled) { bIsEnabled = bNewEnabled; }

    /** Get time this strategy has been active */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    float GetActiveTime() const { return ActiveTime; }

    //========================================
    // Debug
    //========================================

    /** Get debug color for visualization */
    virtual FColor GetDebugColor() const { return FColor::Yellow; }

    /** Draw debug visualization */
    virtual void DrawDebug(const UWorld* World, float Duration = 0.0f) const;

    /** Set the owner camera manager reference */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    void SetOwnerCameraManager(USoulsCameraManager* InCameraManager);

protected:
    //========================================
    // Trace Helpers
    //========================================

    /**
     * Perform a line trace
     * @param Start Trace start location
     * @param End Trace end location
     * @param OutHit Hit result output
     * @param Params Trace parameters
     * @return True if hit something
     */
    bool LineTrace(
        const FVector& Start,
        const FVector& End,
        FHitResult& OutHit,
        const FCollisionQueryParams& Params
    ) const;

    /**
     * Perform a sphere sweep
     * @param Start Trace start location
     * @param End Trace end location
     * @param Radius Sphere radius
     * @param OutHit Hit result output
     * @param Params Trace parameters
     * @return True if hit something
     */
    bool SphereTrace(
        const FVector& Start,
        const FVector& End,
        float Radius,
        FHitResult& OutHit,
        const FCollisionQueryParams& Params
    ) const;

    /**
     * Perform multiple line traces in a cone pattern
     * @param Origin Trace origin
     * @param Direction Primary trace direction
     * @param Distance Trace distance
     * @param ConeAngle Cone half-angle in degrees
     * @param NumRays Number of rays to cast
     * @param OutHits Array of hit results
     * @param Params Trace parameters
     * @return Number of hits
     */
    int32 ConeTrace(
        const FVector& Origin,
        const FVector& Direction,
        float Distance,
        float ConeAngle,
        int32 NumRays,
        TArray<FHitResult>& OutHits,
        const FCollisionQueryParams& Params
    ) const;

    /**
     * Check if a location is valid (not inside geometry)
     * @param Location Location to check
     * @param Radius Check radius
     * @return True if location is valid
     */
    bool IsLocationValid(const FVector& Location, float Radius = 10.0f) const;

    /**
     * Find a safe location near the desired location
     * @param DesiredLocation Where we want to be
     * @param OutSafeLocation Output safe location
     * @param SearchRadius How far to search
     * @return True if safe location found
     */
    bool FindSafeLocation(
        const FVector& DesiredLocation,
        FVector& OutSafeLocation,
        float SearchRadius = 100.0f
    ) const;

    //========================================
    // Utility Methods
    //========================================

    /** Get the world for traces */
    UWorld* GetWorld() const;

    /** Get default collision query params */
    FCollisionQueryParams GetDefaultTraceParams() const;

    /** Get the camera collision channel */
    ECollisionChannel GetCameraCollisionChannel() const;

    //========================================
    // Configuration
    //========================================

    /** Strategy priority (higher = more important) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Config")
    int32 Priority;

    /** Blend weight for this strategy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendWeight;

    /** Can this strategy blend with others */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Config")
    bool bCanBlend;

    /** Collision trace channel to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Config")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    /** Minimum distance from target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Config", meta = (ClampMin = "10.0"))
    float MinDistance;

    /** Probe radius for sphere traces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision|Config", meta = (ClampMin = "1.0"))
    float ProbeRadius;

    //========================================
    // Runtime State
    //========================================

    /** Is strategy currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
    bool bIsActive;

    /** Is strategy enabled */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
    bool bIsEnabled;

    /** Time strategy has been active */
    UPROPERTY(BlueprintReadOnly, Category = "Collision|State")
    float ActiveTime;

    /** Cached world reference */
    UPROPERTY()
    TWeakObjectPtr<UWorld> CachedWorld;

    /** Owner camera manager */
    UPROPERTY()
    TWeakObjectPtr<USoulsCameraManager> OwnerCameraManager;
};
