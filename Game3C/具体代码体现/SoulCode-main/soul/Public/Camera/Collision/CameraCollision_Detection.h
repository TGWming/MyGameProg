// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraCollisionBase.h"
#include "CameraCollision_Detection.generated.h"

/**
 * Detection Strategy Declarations (D01-D04)
 * 
 * This file contains the 4 Detection strategies for the collision system.
 * Detection strategies determine how the camera detects collisions with the environment.
 * 
 * All detection strategies inherit from UCameraCollision_DetectionBase which provides
 * common functionality for collision traces.
 * 
 * Key Characteristics:
 * - Execute during the Detection Phase of collision resolution
 * - Populate FCollisionDetectionResult with collision info
 * - Different methods trade off between performance and accuracy
 * - Higher priority strategies are tried first
 * 
 * Detection Types:
 * - D01: SingleRay - Simple, fast, single ray
 * - D02: SphereSweep - Better coverage, avoids thin geometry
 * - D03: MultiRay - Multiple rays for best coverage
 * - D04: MultiSphereSweep - Multiple sphere sweeps for character-shaped detection
 */


//========================================
// Detection Base Class
//========================================

/**
 * UCameraCollision_DetectionBase
 * 
 * Abstract base class for all detection strategies.
 * Provides common functionality for collision detection traces.
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraCollision_DetectionBase : public UCameraCollisionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_DetectionBase();

    //========================================
    // Strategy Identity
    //========================================

    /** All detection strategies belong to Detection category */
    virtual ECollisionCategory GetStrategyCategory() const override { return ECollisionCategory::Detection; }

protected:
    //========================================
    // Common Detection Methods
    //========================================

    /**
     * Calculate camera trace endpoints
     * @param Context Execution context
     * @param OutStart Output start point (focus point)
     * @param OutEnd Output end point (camera position)
     */
    void CalculateTraceEndpoints(
        const FStageExecutionContext& Context,
        FVector& OutStart,
        FVector& OutEnd
    ) const;

    /**
     * Calculate safe camera position based on hit
     * @param Start Trace start point
     * @param End Trace end point
     * @param HitResult Hit result from trace
     * @param InSafetyMargin Additional margin from hit point
     * @return Safe camera position
     */
    FVector CalculateSafePosition(
        const FVector& Start,
        const FVector& End,
        const FHitResult& HitResult,
        float InSafetyMargin = 10.0f
    ) const;

    /**
     * Calculate safe distance from trace result
     * @param Context Execution context
     * @param HitResult Hit result from trace
     * @return Safe distance from focus point
     */
    float CalculateSafeDistance(
        const FStageExecutionContext& Context,
        const FHitResult& HitResult
    ) const;

    /**
     * Fill detection result from hit
     * @param Context Execution context
     * @param HitResult Hit result from trace
     * @param OutDetectionResult Output detection result to fill
     */
    void FillDetectionResult(
        const FStageExecutionContext& Context,
        const FHitResult& HitResult,
        FCollisionDetectionResult& OutDetectionResult
    ) const;

    //========================================
    // Common Configuration
    //========================================

    /** Safety margin from collision point (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Config", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float SafetyMargin;

    /** Extra distance to add to trace for early detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Config", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float TraceExtension;

    /** Draw debug traces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Debug")
    bool bDrawDebugTraces;
};


//========================================
// D01: Single Ray Detection
//========================================

/**
 * UCameraCollision_D01_SingleRay
 * 
 * D01: Simple line trace collision detection
 * 
 * Casts a single ray from focus point to desired camera position.
 * This is the fastest detection method but can miss thin geometry.
 * 
 * Use Cases:
 * - General purpose detection
 * - Performance-critical situations
 * - Open environments
 * 
 * Characteristics:
 * - Fast (single trace)
 * - May miss thin walls
 * - Priority: 100 (lowest, fallback)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_D01_SingleRay : public UCameraCollision_DetectionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_D01_SingleRay();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_D01_SingleRay; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Simple line trace collision detection"); 
    }
    
    virtual int32 GetPriority() const override { return 100; }

    //========================================
    // Execution
    //========================================

    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) override;

protected:
    //========================================
    // Configuration
    //========================================

    /** Offset from actual camera position for detection (extend past camera) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|SingleRay", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float DetectionOffset;
};


//========================================
// D02: Sphere Sweep Detection
//========================================

/**
 * UCameraCollision_D02_SphereSweep
 * 
 * D02: Sphere sweep collision detection
 * 
 * Sweeps a sphere from focus point to desired camera position.
 * Better at catching thin geometry and provides more accurate "safe" positions.
 * 
 * Use Cases:
 * - Primary detection method
 * - Environments with thin walls/pillars
 * - When accuracy is important
 * 
 * Characteristics:
 * - Medium cost (single sweep)
 * - Catches thin geometry
 * - Priority: 120 (highest, preferred)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_D02_SphereSweep : public UCameraCollision_DetectionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_D02_SphereSweep();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_D02_SphereSweep; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Sphere sweep collision detection"); 
    }
    
    virtual int32 GetPriority() const override { return 120; }

    //========================================
    // Execution
    //========================================

    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) override;

protected:
    //========================================
    // Configuration
    //========================================

    /** Sphere radius for sweep (matches UE4 SpringArm default) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|SphereSweep", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float SweepRadius;

    /** Minimum sweep radius for tight spaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|SphereSweep", meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float MinSweepRadius;

    /** Reduce radius when near obstacles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|SphereSweep")
    bool bAdaptiveRadius;
};


//========================================
// D03: Multi-Ray Detection
//========================================

/**
 * UCameraCollision_D03_MultiRay
 * 
 * D03: Multiple ray cast detection for better coverage
 * 
 * Casts multiple rays in a cone pattern around the main camera line.
 * Provides the best coverage but at higher performance cost.
 * 
 * Use Cases:
 * - Complex environments (forests, ruins)
 * - When single traces miss obstacles
 * - High-precision collision needed
 * 
 * Characteristics:
 * - Higher cost (multiple traces)
 * - Best coverage
 * - Priority: 110 (medium)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_D03_MultiRay : public UCameraCollision_DetectionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_D03_MultiRay();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_D03_MultiRay; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Multi-ray collision detection for better coverage"); 
    }
    
    virtual int32 GetPriority() const override { return 110; }

    //========================================
    // Execution
    //========================================

    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) override;

protected:
    //========================================
    // Configuration
    //========================================

    /** Number of rays to cast (including center) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiRay", meta = (ClampMin = "3", ClampMax = "16"))
    int32 NumRays;

    /** Spread angle for rays (degrees from center) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiRay", meta = (ClampMin = "5.0", ClampMax = "45.0"))
    float SpreadAngle;

    /** Weight for center ray (0.5 = equal to sides) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiRay", meta = (ClampMin = "0.3", ClampMax = "1.0"))
    float CenterRayWeight;

    /** Use closest hit or weighted average */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiRay")
    bool bUseClosestHit;
};


//========================================
// D04: Multi Sphere Sweep Detection
//========================================

/**
 * UCameraCollision_D04_MultiSphereSweep
 * 
 * D04: Multiple sphere sweep collision detection
 * 
 * Sweeps multiple spheres from focus point to camera position.
 * Combines the benefits of sphere sweep with multi-ray coverage.
 * 
 * Use Cases:
 * - Character-relative detection
 * - When vertical clearance matters
 * - Tight corridors
 * 
 * Characteristics:
 * - Medium-high cost
 * - Best for complex spaces
 * - Priority: 115 (medium-high)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_D04_MultiSphereSweep : public UCameraCollision_DetectionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_D04_MultiSphereSweep();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_D04_MultiSphereSweep; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Multi sphere sweep collision detection"); 
    }
    
    virtual int32 GetPriority() const override { return 115; }

    //========================================
    // Execution
    //========================================

    virtual bool Execute(
        const FStageExecutionContext& Context,
        const FCollisionDetectionResult& DetectionResult,
        FCollisionResponseOutput& OutResponse
    ) override;

protected:
    //========================================
    // Configuration
    //========================================

    /** Primary sphere radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiSphere", meta = (ClampMin = "5.0", ClampMax = "30.0"))
    float PrimarySphereRadius;

    /** Secondary sphere radius (for offset sweeps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiSphere", meta = (ClampMin = "2.0", ClampMax = "20.0"))
    float SecondarySphereRadius;

    /** Vertical offset for upper/lower sweeps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiSphere", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float VerticalOffset;

    /** Include upper sphere sweep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiSphere")
    bool bIncludeUpperSweep;

    /** Include lower sphere sweep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|MultiSphere")
    bool bIncludeLowerSweep;
};
