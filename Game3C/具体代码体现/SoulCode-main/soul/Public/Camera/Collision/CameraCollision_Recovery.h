// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraCollisionBase.h"
#include "CameraCollision_Recovery.generated.h"

/**
 * Recovery Strategy Declarations (RC01-RC03)
 * 
 * This file contains the 3 Recovery strategies for the collision system.
 * Recovery strategies handle how the camera returns to its normal position
 * after a collision has ended.
 * 
 * All recovery strategies inherit from UCameraCollision_RecoveryBase which
 * provides common functionality for tracking recovery state and progress.
 * 
 * Key Characteristics:
 * - Execute during the Recovery Phase of collision resolution
 * - Only active when transitioning from collision to no-collision
 * - Produce FCollisionResponseOutput with distance adjustments
 * - Track recovery progress and timing
 * 
 * Recovery Types:
 * - RC01: DelayedRecovery - Wait before starting recovery (prevents jitter)
 * - RC02: SmoothRecovery - Smooth interpolation back to target distance
 * - RC03: StepRecovery - Step-based recovery for precise control
 */


//========================================
// Recovery Base Class
//========================================

/**
 * UCameraCollision_RecoveryBase
 * 
 * Abstract base class for all recovery strategies.
 * Provides common functionality for managing recovery state and progress.
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraCollision_RecoveryBase : public UCameraCollisionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RecoveryBase();

    //========================================
    // Strategy Identity
    //========================================

    /** All recovery strategies belong to Recovery category */
    virtual ECollisionCategory GetStrategyCategory() const override { return ECollisionCategory::Recovery; }

    //========================================
    // Lifecycle
    //========================================

    /** Update recovery state each frame */
    virtual void Update(float DeltaTime, const FStageExecutionContext& Context) override;

    /** Reset recovery to initial state */
    virtual void Reset() override;

    //========================================
    // Recovery State Query
    //========================================

    /** Is recovery currently in progress */
    UFUNCTION(BlueprintPure, Category = "Recovery")
    bool IsRecovering() const { return bIsRecovering; }

    /** Get current recovery progress (0 = just started, 1 = complete) */
    UFUNCTION(BlueprintPure, Category = "Recovery")
    float GetRecoveryProgress() const { return RecoveryProgress; }

    /** Get time since recovery started */
    UFUNCTION(BlueprintPure, Category = "Recovery")
    float GetRecoveryTime() const { return RecoveryTime; }

protected:
    //========================================
    // Common Recovery Methods
    //========================================

    /**
     * Begin recovery process
     * @param StartDistance Distance at start of recovery
     * @param TargetDistance Target distance to recover to
     */
    void BeginRecovery(float StartDistance, float TargetDistance);

    /**
     * End recovery process
     */
    void EndRecovery();

    /**
     * Update recovery progress
     * @param DeltaTime Time since last frame
     * @param Speed Recovery speed
     * @return New current distance
     */
    float UpdateRecoveryProgress(float DeltaTime, float Speed);

    /**
     * Check if recovery should start
     * @param Context Execution context
     * @param DetectionResult Detection results
     * @return True if recovery should begin
     */
    bool ShouldBeginRecovery(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult
    ) const;

    /**
     * Calculate distance adjustment for this frame
     * @param CurrentDistance Current camera distance
     * @param TargetDistance Target camera distance
     * @param DeltaTime Time since last frame
     * @param Speed Recovery speed
     * @return Distance adjustment (positive = further away)
     */
    float CalculateDistanceAdjustment(
        float CurrentDistance,
        float TargetDistance,
        float DeltaTime,
        float Speed
    ) const;

    //========================================
    // Common Configuration
    //========================================

    /** Base recovery speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Config", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float BaseRecoverySpeed;

    /** Minimum recovery time (prevents instant snap) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinRecoveryTime;

    /** Distance tolerance for considering recovery complete */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Config", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CompletionTolerance;

    //========================================
    // Runtime State
    //========================================

    /** Is recovery currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Recovery|State")
    bool bIsRecovering;

    /** Current recovery progress (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "Recovery|State")
    float RecoveryProgress;

    /** Time since recovery started */
    UPROPERTY(BlueprintReadOnly, Category = "Recovery|State")
    float RecoveryTime;

    /** Distance at start of recovery */
    float RecoveryStartDistance;

    /** Target distance to recover to */
    float RecoveryTargetDistance;

    /** Current distance during recovery */
    float CurrentRecoveryDistance;

    /** Was in collision previous frame */
    bool bWasInCollision;

    /** Cached delta time */
    float CachedDeltaTime;
};


//========================================
// RC01: Delayed Recovery
//========================================

/**
 * UCameraCollision_RC01_DelayedRecovery
 * 
 * RC01: Wait before starting recovery
 * 
 * Adds a delay after collision ends before beginning recovery.
 * This prevents camera jitter when rapidly entering/exiting collisions.
 * 
 * Use Cases:
 * - Complex environments with many obstacles
 * - When player is moving near walls
 * - Preventing visual jitter
 * 
 * Characteristics:
 * - Waits before recovery
 * - Smooth once started
 * - Priority: 100 (default)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RC01_DelayedRecovery : public UCameraCollision_RecoveryBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RC01_DelayedRecovery();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RC01_DelayedRecovery; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Delayed recovery to prevent jitter"); 
    }
    
    virtual int32 GetPriority() const override { return 100; }

    //========================================
    // Execution
    //========================================

    virtual bool ShouldExecute(const FStageExecutionContext& Context) const override;

    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) override;

    virtual void Update(float DeltaTime, const FStageExecutionContext& Context) override;
    virtual void Reset() override;

protected:
    //========================================
    // Configuration
    //========================================

    /** Delay before starting recovery (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Delayed", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RecoveryDelay;

    /** Speed once recovery begins */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Delayed", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float DelayedRecoverySpeed;

    /** Reset delay if collision happens during delay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Delayed")
    bool bResetDelayOnCollision;

    //========================================
    // Runtime State
    //========================================

    /** Time since collision ended */
    float TimeSinceCollisionEnd;

    /** Has delay been satisfied */
    bool bDelayComplete;
};


//========================================
// RC02: Smooth Recovery
//========================================

/**
 * UCameraCollision_RC02_SmoothRecovery
 * 
 * RC02: Smooth interpolation back to target distance
 * 
 * Uses smooth interpolation for a natural feeling recovery.
 * The most commonly used recovery strategy.
 * 
 * Use Cases:
 * - Default recovery behavior
 * - General gameplay
 * - When natural feel is important
 * 
 * Characteristics:
 * - Smooth interpolation
 * - Consistent speed
 * - Priority: 105 (medium)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RC02_SmoothRecovery : public UCameraCollision_RecoveryBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RC02_SmoothRecovery();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RC02_SmoothRecovery; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Smooth interpolated recovery"); 
    }
    
    virtual int32 GetPriority() const override { return 105; }

    //========================================
    // Execution
    //========================================

    virtual bool ShouldExecute(const FStageExecutionContext& Context) const override;

    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) override;

    virtual void Update(float DeltaTime, const FStageExecutionContext& Context) override;
    virtual void Reset() override;

protected:
    //========================================
    // Configuration
    //========================================

    /** Interpolation speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Smooth", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float SmoothRecoverySpeed;

    /** Use ease-out curve for more natural feel */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Smooth")
    bool bUseEaseOut;

    /** Ease-out exponent (higher = more pronounced ease) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Smooth", meta = (ClampMin = "1.0", ClampMax = "5.0", EditCondition = "bUseEaseOut"))
    float EaseOutExponent;

    //========================================
    // Runtime State
    //========================================

    /** Interpolation alpha (0-1) */
    float InterpAlpha;
};


//========================================
// RC03: Step Recovery
//========================================

/**
 * UCameraCollision_RC03_StepRecovery
 * 
 * RC03: Step-based recovery for precise control
 * 
 * Recovers in discrete steps rather than continuous interpolation.
 * Useful for stylized games or when precise control is needed.
 * 
 * Use Cases:
 * - Stylized camera behavior
 * - When discrete movement is desired
 * - Precise distance control
 * 
 * Characteristics:
 * - Discrete steps
 * - Predictable timing
 * - Priority: 110 (highest)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RC03_StepRecovery : public UCameraCollision_RecoveryBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RC03_StepRecovery();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RC03_StepRecovery; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Step-based recovery for precise control"); 
    }
    
    virtual int32 GetPriority() const override { return 110; }

    //========================================
    // Execution
    //========================================

    virtual bool ShouldExecute(const FStageExecutionContext& Context) const override;

    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) override;

    virtual void Update(float DeltaTime, const FStageExecutionContext& Context) override;
    virtual void Reset() override;

protected:
    //========================================
    // Configuration
    //========================================

    /** Distance per step */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Step", meta = (ClampMin = "5.0", ClampMax = "50.0"))
    float StepDistance;

    /** Time between steps (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Step", meta = (ClampMin = "0.05", ClampMax = "0.5"))
    float StepInterval;

    /** Smooth within each step */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Step")
    bool bSmoothWithinStep;

    /** Number of steps to complete recovery (0 = auto-calculate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery|Step", meta = (ClampMin = "0", ClampMax = "20"))
    int32 MaxSteps;

    //========================================
    // Runtime State
    //========================================

    /** Current step index */
    int32 CurrentStep;

    /** Total steps for this recovery */
    int32 TotalSteps;

    /** Time since last step */
    float TimeSinceLastStep;

    /** Distance at current step start */
    float StepStartDistance;

    /** Target distance for current step */
    float StepTargetDistance;
};
