// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraCollisionBase.h"
#include "CameraCollision_Response.generated.h"

/**
 * Response Strategy Declarations (RS01-RS05)
 * 
 * This file contains the 5 Response strategies for the collision system.
 * Response strategies determine how the camera adjusts when a collision is detected.
 * 
 * All response strategies inherit from UCameraCollision_ResponseBase which provides
 * common functionality for camera adjustment calculations.
 * 
 * Key Characteristics:
 * - Execute during the Response Phase of collision resolution
 * - Read FCollisionDetectionResult from Detection phase
 * - Produce FCollisionResponseOutput with camera adjustments
 * - Support smooth interpolation for natural camera movement
 * - Can be blended together for combined effects
 * 
 * Response Types:
 * - RS01: PullIn - Simply reduce camera distance (most common)
 * - RS02: Slide - Move camera laterally along surface
 * - RS03: Orbit - Rotate camera around target to avoid collision
 * - RS04: FOVCompensate - Increase FOV when pulling in to maintain framing
 * - RS05: InstantSnap - Immediately snap to safe position (no smoothing)
 */


//========================================
// Response Base Class
//========================================

/**
 * UCameraCollision_ResponseBase
 * 
 * Abstract base class for all response strategies.
 * Provides common functionality for camera adjustment calculations.
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraCollision_ResponseBase : public UCameraCollisionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_ResponseBase();

    //========================================
    // Strategy Identity
    //========================================

    /** All response strategies belong to Response category */
    virtual ECollisionCategory GetStrategyCategory() const override { return ECollisionCategory::Response; }

    //========================================
    // Lifecycle
    //========================================

    /** Update response state each frame */
    virtual void Update(float DeltaTime, const FStageExecutionContext& Context) override;

    /** Reset response to initial state */
    virtual void Reset() override;

protected:
    //========================================
    // Common Response Methods
    //========================================

    /**
     * Interpolate current value towards target
     * @param Current Current value
     * @param Target Target value
     * @param DeltaTime Time step
     * @param Speed Interpolation speed
     * @return Interpolated value
     */
    float InterpolateValue(float Current, float Target, float DeltaTime, float Speed) const;

    /**
     * Interpolate current vector towards target
     * @param Current Current vector
     * @param Target Target vector
     * @param DeltaTime Time step
     * @param Speed Interpolation speed
     * @return Interpolated vector
     */
    FVector InterpolateVector(const FVector& Current, const FVector& Target, float DeltaTime, float Speed) const;

    /**
     * Interpolate current rotator towards target
     * @param Current Current rotator
     * @param Target Target rotator
     * @param DeltaTime Time step
     * @param Speed Interpolation speed
     * @return Interpolated rotator
     */
    FRotator InterpolateRotator(const FRotator& Current, const FRotator& Target, float DeltaTime, float Speed) const;

    /**
     * Check if response is needed based on detection result
     * @param DetectionResult Result from detection phase
     * @return True if response should be applied
     */
    bool IsResponseNeeded(const FCollisionDetectionResult& DetectionResult) const;

    /**
     * Fill basic response output with this strategy's info
     * @param OutResponse Output to fill
     */
    void InitializeResponse(FCollisionResponseOutput& OutResponse) const;

    //========================================
    // Common Configuration
    //========================================

    /** Base interpolation speed for smooth adjustments */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Config", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float BaseInterpSpeed;

    /** Blend time for this response */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ResponseBlendTime;

    /** Should this response override others */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Config")
    bool bOverrideOthers;

    //========================================
    // Runtime State
    //========================================

    /** Time since response started */
    UPROPERTY(BlueprintReadOnly, Category = "Response|State")
    float ResponseTime;

    /** Is response currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Response|State")
    bool bResponseActive;

    /** Cached delta time for interpolation */
    float CachedDeltaTime;
};


//========================================
// RS01: Pull In Response
//========================================

/**
 * UCameraCollision_RS01_PullIn
 * 
 * RS01: Simply pull camera closer to target
 * 
 * The most common and straightforward collision response.
 * Reduces camera distance to avoid the obstacle.
 * 
 * Use Cases:
 * - Default collision response
 * - Simple environments
 * - When maintaining view angle is important
 * 
 * Characteristics:
 * - Simple and reliable
 * - May change framing (character appears larger)
 * - Priority: 100 (default)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RS01_PullIn : public UCameraCollision_ResponseBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RS01_PullIn();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RS01_PullIn; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Pull camera closer to avoid collision"); 
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

    /** 重置拉近状态（碰撞结束时调用） */
    void ResetPullState();

protected:
    //========================================
    // Configuration
    //========================================

    /** How quickly to pull camera in (units per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float PullInSpeed;

    /** Minimum time to complete pull-in (prevents instant snapping) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn", meta = (ClampMin = "0.05", ClampMax = "0.5"))
    float MinPullInTime;

    /** Add slight overshoot then settle (more dynamic feel) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn")
    bool bUseOvershoot;

    /** Overshoot amount (percentage of adjustment) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn", meta = (ClampMin = "0.0", ClampMax = "0.2", EditCondition = "bUseOvershoot"))
    float OvershootAmount;

    //========================================
    // Acceleration Mode Configuration (新增)
    //========================================

    /** 是否启用加速度模式，启用后拉近有加速感 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn|Acceleration")
    bool bUseAcceleration = true;

    /** 拉近加速度（cm/s²），值越大加速越快 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn|Acceleration", 
              meta = (ClampMin = "0.0", ClampMax = "2000.0", EditCondition = "bUseAcceleration"))
    float PullInAcceleration = 500.0f;

    /** 最大拉近速度（cm/s），加速度模式下的速度上限 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|PullIn|Acceleration", 
              meta = (ClampMin = "100.0", ClampMax = "1000.0", EditCondition = "bUseAcceleration"))
    float MaxPullInSpeed = 500.0f;

    /** 当前拉近速度（运行时状态，加速度模式使用） */
    float CurrentPullSpeed = 0.0f;

    //========================================
    // Runtime State
    //========================================

    /** Current interpolated distance adjustment */
    float CurrentDistanceAdjustment;

    /** Target distance adjustment */
    float TargetDistanceAdjustment;

    /** Is currently adjusting */
    bool bIsAdjusting;
};


//========================================
// RS02: Slide Response
//========================================

/**
 * UCameraCollision_RS02_Slide
 * 
 * RS02: Slide camera along collision surface
 * 
 * Moves camera laterally to avoid obstacle while trying to maintain distance.
 * Uses collision normal to determine slide direction.
 * 
 * Use Cases:
 * - Walls and flat surfaces
 * - When maintaining distance is important
 * - Corners and edges
 * 
 * Characteristics:
 * - Maintains original framing better
 * - May change view angle slightly
 * - Priority: 110 (higher than PullIn)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RS02_Slide : public UCameraCollision_ResponseBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RS02_Slide();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RS02_Slide; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Slide camera along collision surface"); 
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

    /** Maximum distance to slide from original position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Slide", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float MaxSlideDistance;

    /** How quickly to slide (units per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Slide", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float SlideSpeed;

    /** Prefer horizontal sliding over vertical */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Slide")
    bool bPreferHorizontalSlide;

    /** Verify slide destination is valid (no new collisions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Slide")
    bool bVerifySlideDestination;

    //========================================
    // Runtime State
    //========================================

    /** Current slide offset from original position */
    FVector CurrentSlideOffset;

    /** Target slide offset */
    FVector TargetSlideOffset;

    /** Direction of current slide */
    FVector SlideDirection;

    //========================================
    // Helper Methods
    //========================================

    /**
     * Calculate optimal slide direction from collision normal
     * @param CollisionNormal Normal of collision surface
     * @param CameraDirection Current camera direction
     * @return Slide direction vector
     */
    FVector CalculateSlideDirection(
        const FVector& CollisionNormal,
        const FVector& CameraDirection
    ) const;

    /**
     * Verify that slide position doesn't cause new collision
     * @param Context Execution context
     * @param DetectionResult Detection results
     * @param SlideOffset Proposed slide offset
     * @return True if slide position is valid
     */
    bool IsSlideValid(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        const FVector& SlideOffset
    ) const;
};


//========================================
// RS03: Orbit Response
//========================================

/**
 * UCameraCollision_RS03_Orbit
 * 
 * RS03: Orbit camera around target to avoid collision
 * 
 * Rotates camera around the focus point to find a clear view.
 * Maintains camera distance while changing viewing angle.
 * 
 * Use Cases:
 * - When pull-in or slide aren't suitable
 * - Complex geometry
 * - When slight angle change is acceptable
 * 
 * Characteristics:
 * - Changes view angle
 * - May feel disorienting if overused
 * - Priority: 105 (medium)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RS03_Orbit : public UCameraCollision_ResponseBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RS03_Orbit();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RS03_Orbit; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Orbit camera around target to avoid collision"); 
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

    /** Maximum orbit angle adjustment (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Orbit", meta = (ClampMin = "5.0", ClampMax = "45.0"))
    float MaxOrbitAngle;

    /** How quickly to orbit (degrees per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Orbit", meta = (ClampMin = "30.0", ClampMax = "180.0"))
    float OrbitSpeed;

    /** Number of angles to test when finding best orbit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Orbit", meta = (ClampMin = "4", ClampMax = "16"))
    int32 NumTestAngles;

    /** Prefer yaw rotation over pitch */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Orbit")
    bool bPreferYawOrbit;

    /** Maximum pitch adjustment (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|Orbit", meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float MaxPitchAdjustment;

    //========================================
    // Runtime State
    //========================================

    /** Current orbit rotation adjustment */
    FRotator CurrentOrbitAdjustment;

    /** Target orbit rotation adjustment */
    FRotator TargetOrbitAdjustment;

    /** Best orbit angle found during search */
    FRotator BestFoundOrbit;

    //========================================
    // Helper Methods
    //========================================

    /**
     * Find best orbit angle to avoid collision
     * Tests multiple angles and returns the one with clearest view
     * @param Context Execution context
     * @param DetectionResult Detection results
     * @return Best orbit adjustment found
     */
    FRotator FindBestOrbit(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult
    );

    /**
     * Test if a specific orbit provides clear view
     * @param Context Execution context
     * @param TestOrbit Orbit angle to test
     * @return True if orbit provides clear view
     */
    bool TestOrbit(
        const FStageExecutionContext& Context,
        const FRotator& TestOrbit
    ) const;
};


//========================================
// RS04: FOV Compensate Response
//========================================

/**
 * UCameraCollision_RS04_FOVCompensate
 * 
 * RS04: Increase FOV when pulling in to maintain framing
 * 
 * When camera is pulled in due to collision, increases FOV to compensate.
 * This helps maintain a similar sense of space and character framing.
 * 
 * Use Cases:
 * - Combined with PullIn response
 * - When maintaining framing is important
 * - Tight spaces where character appears too large
 * 
 * Characteristics:
 * - Works best combined with PullIn
 * - Maintains framing
 * - Priority: 95 (applied after PullIn)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RS04_FOVCompensate : public UCameraCollision_ResponseBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RS04_FOVCompensate();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RS04_FOVCompensate; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Increase FOV to compensate for pull-in"); 
    }
    
    virtual int32 GetPriority() const override { return 95; }

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

    /** Maximum FOV increase (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|FOVCompensate", meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float MaxFOVIncrease;

    /** FOV change per unit distance reduction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|FOVCompensate", meta = (ClampMin = "0.01", ClampMax = "0.5"))
    float FOVPerDistanceUnit;

    /** How quickly to adjust FOV */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|FOVCompensate", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float FOVAdjustSpeed;

    /** Minimum distance reduction to trigger FOV compensation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|FOVCompensate", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float MinDistanceReductionThreshold;

    //========================================
    // Runtime State
    //========================================

    /** Current FOV adjustment */
    float CurrentFOVAdjustment;

    /** Target FOV adjustment */
    float TargetFOVAdjustment;

    /** Original distance before collision */
    float OriginalDistance;
};


//========================================
// RS05: Instant Snap Response
//========================================

/**
 * UCameraCollision_RS05_InstantSnap
 * 
 * RS05: Immediately snap to safe position (no smoothing)
 * 
 * Instantly moves camera to safe position without interpolation.
 * Used when smoothing would look worse than a cut.
 * 
 * Use Cases:
 * - Severe collisions
 * - When smoothing looks bad
 * - Emergency fallback
 * 
 * Characteristics:
 * - Instant response
 * - May appear jarring
 * - Priority: 120 (highest, emergency)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_RS05_InstantSnap : public UCameraCollision_ResponseBase
{
    GENERATED_BODY()

public:
    UCameraCollision_RS05_InstantSnap();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_RS05_InstantSnap; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Instantly snap to safe position"); 
    }
    
    virtual int32 GetPriority() const override { return 120; }

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

    /** Minimum collision severity to trigger snap (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|InstantSnap", meta = (ClampMin = "0.3", ClampMax = "1.0"))
    float SnapThreshold;

    /** Apply tiny smoothing even for snap (1 frame) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|InstantSnap")
    bool bMicroSmooth;

    /** Micro smooth duration (frames worth) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|InstantSnap", meta = (ClampMin = "1", ClampMax = "5", EditCondition = "bMicroSmooth"))
    int32 MicroSmoothFrames;

    /** Add slight padding beyond safe distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response|InstantSnap", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float SafetyPadding;

    //========================================
    // Runtime State
    //========================================

    /** Was snap triggered last frame */
    bool bSnapTriggered;

    /** Frames remaining in micro smooth */
    int32 MicroSmoothRemaining;

    /** Target position for micro smooth */
    float MicroSmoothTarget;
};

