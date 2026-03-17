// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/Data/CameraStateEnums.h"
#include "CameraCollisionBase.h"
#include "CameraCollisionResolver.generated.h"

// Forward declarations
class USoulsCameraManager;
struct FStageExecutionContext;

/**
 * Collision Resolver Configuration
 * 
 * Settings for the collision resolution system.
 */
USTRUCT(BlueprintType)
struct SOUL_API FCollisionResolverConfig
{
    GENERATED_BODY()

    //========================================
    // Detection Settings
    //========================================

    /** Use sphere trace instead of line trace for primary detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    bool bUseSphereTrace;

    /** Sphere trace radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "1.0", ClampMax = "50.0", EditCondition = "bUseSphereTrace"))
    float SphereTraceRadius;

    /** Number of rays for multi-ray detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "1", ClampMax = "16"))
    int32 MultiRayCount;

    /** Cone angle for multi-ray detection (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "5.0", ClampMax = "45.0"))
    float MultiRayConeAngle;

    //========================================
    // Response Settings
    //========================================

    /** Minimum distance from target (never closer than this) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (ClampMin = "10.0"))
    float MinDistanceFromTarget;

    /** How quickly to pull camera in when collision detected */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float PullInSpeed;

    /** Allow camera to slide along surfaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    bool bAllowSliding;

    /** Allow camera to rotate to avoid obstacles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    bool bAllowRotation;

    //========================================
    // Recovery Settings
    //========================================

    /** How quickly to return to normal after collision ends */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery", meta = (ClampMin = "0.5", ClampMax = "20.0"))
    float RecoverySpeed;

    /** Delay before starting recovery (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RecoveryDelay;

    /** Smoothly interpolate during recovery */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bSmoothRecovery;

    //========================================
    // Occlusion Settings
    //========================================

    /** Enable character occlusion detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion")
    bool bCheckCharacterOcclusion;

    /** Enable target occlusion detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion")
    bool bCheckTargetOcclusion;

    /** Occlusion check frequency (seconds between checks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion", meta = (ClampMin = "0.016", ClampMax = "0.5"))
    float OcclusionCheckInterval;

    //========================================
    // Debug Settings
    //========================================

    /** Draw debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug;

    /** Debug draw duration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "0.0", EditCondition = "bDrawDebug"))
    float DebugDrawDuration;

    //========================================
    // Constructor
    //========================================

    FCollisionResolverConfig()
        : bUseSphereTrace(true)
        , SphereTraceRadius(12.0f)
        , MultiRayCount(5)
        , MultiRayConeAngle(15.0f)
        , MinDistanceFromTarget(50.0f)
        , PullInSpeed(15.0f)
        , bAllowSliding(true)
        , bAllowRotation(false)
        , RecoverySpeed(5.0f)
        , RecoveryDelay(0.1f)
        , bSmoothRecovery(true)
        , bCheckCharacterOcclusion(true)
        , bCheckTargetOcclusion(true)
        , OcclusionCheckInterval(0.1f)
        , bDrawDebug(false)
        , DebugDrawDuration(0.0f)
    {}
};

/**
 * Collision State
 * 
 * Tracks the current collision state of the camera.
 */
USTRUCT(BlueprintType)
struct SOUL_API FCollisionState
{
    GENERATED_BODY()

    /** Is collision currently active */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsColliding;

    /** Is in recovery mode */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsRecovering;

    /** Current collision distance (adjusted) */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentDistance;

    /** Desired distance (before collision adjustment) */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DesiredDistance;

    /** Time spent in current collision */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CollisionTime;

    /** Time since collision ended */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeSinceCollision;

    /** Recovery progress (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float RecoveryProgress;

    /** Is character currently occluded */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bCharacterOccluded;

    /** Is target currently occluded */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bTargetOccluded;

    FCollisionState()
        : bIsColliding(false)
        , bIsRecovering(false)
        , CurrentDistance(0.0f)
        , DesiredDistance(0.0f)
        , CollisionTime(0.0f)
        , TimeSinceCollision(0.0f)
        , RecoveryProgress(1.0f)
        , bCharacterOccluded(false)
        , bTargetOccluded(false)
    {}

    void Reset()
    {
        bIsColliding = false;
        bIsRecovering = false;
        CurrentDistance = 0.0f;
        DesiredDistance = 0.0f;
        CollisionTime = 0.0f;
        TimeSinceCollision = 0.0f;
        RecoveryProgress = 1.0f;
        bCharacterOccluded = false;
        bTargetOccluded = false;
    }
};

/**
 * UCameraCollisionResolver
 * 
 * Manages all 20 camera collision strategies and resolves collisions.
 * 
 * This class is the central hub for collision handling:
 * - Registers and manages all collision strategies
 * - Executes strategies in the correct order (Detection -> Response -> Occlusion -> Recovery -> Special)
 * - Blends multiple collision responses
 * - Tracks collision state and recovery progress
 * - Provides final collision-adjusted camera output
 * 
 * The resolver is owned by SoulsCameraManager and used by Stage 6 (CollisionResolve).
 * 
 * Strategy Categories (20 total):
 * - Detection (Det01-Det04): How to detect collisions
 * - Response (Res01-Res04): How to respond to collisions
 * - Occlusion (Occ01-Occ04): Handle view blocking
 * - Recovery (Rec01-Rec04): Return to normal after collision
 * - Special (Spc01-Spc04): Unique collision scenarios
 */
UCLASS(BlueprintType)
class SOUL_API UCameraCollisionResolver : public UObject
{
    GENERATED_BODY()

public:
    UCameraCollisionResolver();

    //========================================
    // Initialization
    //========================================

    /**
     * Initialize the resolver and create all strategies
     * @param InCameraManager Owner camera manager
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    void Initialize(USoulsCameraManager* InCameraManager);

    /**
     * Check if resolver is initialized
     * @return True if initialized
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    bool IsInitialized() const { return bIsInitialized; }

    /**
     * Shutdown and cleanup
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    void Shutdown();

    //========================================
    // Strategy Registration
    //========================================

    /**
     * Register a collision strategy
     * @param Strategy Strategy to register
     * @return True if registered successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    bool RegisterStrategy(UCameraCollisionBase* Strategy);

    /**
     * Unregister a strategy by type
     * @param StrategyType Type of strategy to unregister
     * @return True if unregistered successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    bool UnregisterStrategy(ECollisionStrategyID StrategyType);

    /**
     * Get strategy by type
     * @param StrategyType Type of strategy to get
     * @return Strategy instance or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    UCameraCollisionBase* GetStrategy(ECollisionStrategyID StrategyType) const;

    /**
     * Get all registered strategies
     * @return Array of all strategies
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    TArray<UCameraCollisionBase*> GetAllStrategies() const;

    /**
     * Get strategies by category
     * @param Category Category to filter by
     * @return Array of strategies in category
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    TArray<UCameraCollisionBase*> GetStrategiesByCategory(ECollisionCategory Category) const;

    /**
     * Get total number of registered strategies
     * @return Strategy count
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    int32 GetStrategyCount() const { return RegisteredStrategies.Num(); }

    //========================================
    // Collision Resolution (Main Entry Point)
    //========================================

    /**
     * Main collision resolution function - called by Stage 6
     * Executes all collision strategies and applies adjustments
     * @param Context Pipeline execution context
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    void ResolveCollisions(FStageExecutionContext& Context);

    /**
     * Update collision state (called every frame)
     * @param DeltaTime Time since last frame
     * @param Context Pipeline execution context
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    void Update(float DeltaTime, const FStageExecutionContext& Context);

    //========================================
    // Detection
    //========================================

    /**
     * Perform collision detection
     * @param Context Pipeline execution context
     * @return Detection result
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    FCollisionDetectionResult PerformDetection(const FStageExecutionContext& Context);

    /**
     * Check if character is occluded from camera
     * @param Context Pipeline execution context
     * @return True if character is occluded
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    bool IsCharacterOccluded(const FStageExecutionContext& Context) const;

    /**
     * Check if lock-on target is occluded from camera
     * @param Context Pipeline execution context
     * @return True if target is occluded
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    bool IsTargetOccluded(const FStageExecutionContext& Context) const;

    //========================================
    // State Query
    //========================================

    /**
     * Is collision currently active (camera adjusted)
     * @return True if colliding
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    bool IsCollisionActive() const { return CollisionState.bIsColliding; }

    /**
     * Is in recovery mode (returning to normal)
     * @return True if recovering
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    bool IsInRecovery() const { return CollisionState.bIsRecovering; }

    /**
     * Get current collision state
     * @return Collision state struct
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    const FCollisionState& GetCollisionState() const { return CollisionState; }

    /**
     * Get current detection result
     * @return Last detection result
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    const FCollisionDetectionResult& GetCurrentDetectionResult() const { return CurrentDetectionResult; }

    /**
     * Get time since last collision
     * @return Time in seconds
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    float GetTimeSinceCollision() const { return CollisionState.TimeSinceCollision; }

    /**
     * Get recovery progress (0 = just started, 1 = complete)
     * @return Recovery progress
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    float GetRecoveryProgress() const { return CollisionState.RecoveryProgress; }

    //========================================
    // Configuration
    //========================================

    /**
     * Get resolver configuration
     * @return Config struct
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    const FCollisionResolverConfig& GetConfig() const { return Config; }

    /**
     * Set resolver configuration
     * @param NewConfig New config to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
    void SetConfig(const FCollisionResolverConfig& NewConfig) { Config = NewConfig; }

    //========================================
    // Debug
    //========================================

    /**
     * Enable/disable debug visualization
     * @param bEnable True to enable debug drawing
     */
    UFUNCTION(BlueprintCallable, Category = "Camera|Collision|Debug")
    void SetDebugEnabled(bool bEnable) { Config.bDrawDebug = bEnable; }

    /**
     * Get the camera manager
     * @return Owner camera manager
     */
    UFUNCTION(BlueprintPure, Category = "Camera|Collision")
    USoulsCameraManager* GetCameraManager() const { return CameraManager.Get(); }

    /**
     * Draw debug visualization
     * @param World World to draw in
     */
    void DrawDebug(const UWorld* World) const;

protected:
    //========================================
    // Strategy Creation
    //========================================

    /**
     * Create all 20 collision strategy instances
     * @param Outer Outer object for strategy creation
     */
    void CreateAllStrategies(UObject* Outer);

    //========================================
    // Phase Execution
    //========================================

    /**
     * Execute detection strategies (Det01-Det04)
     * @param Context Pipeline execution context
     * @param OutResult Output detection result
     */
    void ExecuteDetectionStrategies(const FStageExecutionContext& Context, FCollisionDetectionResult& OutResult);

    /**
     * Execute response strategies (Res01-Res04)
     * @param Context Pipeline execution context
     * @param DetectionResult Detection results to respond to
     * @param OutResponses Array of response outputs
     */
    void ExecuteResponseStrategies(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        TArray<FCollisionResponseOutput>& OutResponses
    );

    /**
     * Execute occlusion strategies (Occ01-Occ04)
     * @param Context Pipeline execution context
     * @param DetectionResult Detection results
     */
    void ExecuteOcclusionStrategies(
        const FStageExecutionContext& Context,
        FCollisionDetectionResult& DetectionResult
    );

    /**
     * Execute recovery strategies (Rec01-Rec04)
     * @param Context Pipeline execution context
     * @param DeltaTime Time since last frame
     */
    void ExecuteRecoveryStrategies(
        FStageExecutionContext& Context,
        float DeltaTime
    );

    /**
     * Execute special strategies (Spc01-Spc04)
     * @param Context Pipeline execution context
     * @param DetectionResult Detection results
     * @param OutResponses Additional response outputs
     */
    void ExecuteSpecialStrategies(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        TArray<FCollisionResponseOutput>& OutResponses
    );

    //========================================
    // Response Processing
    //========================================

    /**
     * Blend multiple collision responses into one
     * @param Responses Array of responses to blend
     * @param OutBlended Output blended response
     */
    void BlendResponses(
        const TArray<FCollisionResponseOutput>& Responses,
        FCollisionResponseOutput& OutBlended
    );

    /**
     * Apply collision response to context output
     * @param Context Pipeline execution context
     * @param Response Response to apply
     */
    void ApplyResponse(
        FStageExecutionContext& Context,
        const FCollisionResponseOutput& Response
    );

    //========================================
    // State Management
    //========================================

    /**
     * Update collision state based on detection result
     * @param DeltaTime Time since last frame
     * @param DetectionResult Current detection result
     */
    void UpdateCollisionState(float DeltaTime, const FCollisionDetectionResult& DetectionResult);

    /**
     * Start recovery process
     */
    void BeginRecovery();

    /**
     * Update recovery progress
     * @param DeltaTime Time since last frame
     */
    void UpdateRecovery(float DeltaTime);

    //========================================
    // Members
    //========================================

    /** Owner camera manager */
    UPROPERTY()
    TWeakObjectPtr<USoulsCameraManager> CameraManager;

    /** Map of registered strategies by type */
    UPROPERTY()
    TMap<ECollisionStrategyID, UCameraCollisionBase*> RegisteredStrategies;

    /** Resolver configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FCollisionResolverConfig Config;

    /** Current collision state */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FCollisionState CollisionState;

    /** Current detection result */
    FCollisionDetectionResult CurrentDetectionResult;

    /** Previous detection result (for comparison) */
    FCollisionDetectionResult PreviousDetectionResult;

    /** Pre-collision distance (for recovery) */
    float PreCollisionDistance;

    /** Pre-collision position (for recovery) */
    FVector PreCollisionPosition;

    /** Time since last occlusion check */
    float TimeSinceOcclusionCheck;

    /** Is resolver initialized */
    bool bIsInitialized;
};
