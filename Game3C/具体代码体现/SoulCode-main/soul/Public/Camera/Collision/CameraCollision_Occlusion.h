// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraCollisionBase.h"
#include "CameraCollision_Occlusion.generated.h"

/**
 * Occlusion Strategy Declarations (OC01-OC04)
 * 
 * This file contains the 4 Occlusion strategies for the collision system.
 * Occlusion strategies handle situations where the player character or
 * lock-on target is blocked from view by environmental objects.
 * 
 * All occlusion strategies inherit from UCameraCollision_OcclusionBase which
 * provides common functionality for visibility checking and actor management.
 * 
 * Key Characteristics:
 * - Execute during the Occlusion Phase of collision resolution
 * - Read occlusion flags from FCollisionDetectionResult
 * - May directly manipulate actor visibility/materials
 * - Track occluding actors for fade in/out transitions
 * 
 * Occlusion Types:
 * - OC01: FadeOut - Gradually fade occluding objects
 * - OC02: Hide - Completely hide occluding objects
 * - OC03: PullInFurther - Pull camera even closer to see past occlusion
 * - OC04: DitherFade - Use dither pattern for fade effect
 */

// Forward declarations
class UMaterialInterface;
class UMaterialInstanceDynamic;


//========================================
// Occlusion Base Class
//========================================

/**
 * UCameraCollision_OcclusionBase
 * 
 * Abstract base class for all occlusion strategies.
 * Provides common functionality for managing occluding actors.
 */
UCLASS(Abstract, BlueprintType)
class SOUL_API UCameraCollision_OcclusionBase : public UCameraCollisionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_OcclusionBase();

    //========================================
    // Strategy Identity
    //========================================

    /** All occlusion strategies belong to Occlusion category */
    virtual ECollisionCategory GetStrategyCategory() const override { return ECollisionCategory::Occlusion; }

    //========================================
    // Lifecycle
    //========================================

    /** Update occlusion state each frame */
    virtual void Update(float DeltaTime, const FStageExecutionContext& Context) override;

    /** Reset occlusion to initial state */
    virtual void Reset() override;

protected:
    //========================================
    // Common Occlusion Methods
    //========================================

    /**
     * Perform occlusion trace between camera and target
     * @param Context Execution context
     * @param TargetLocation Location to check visibility to
     * @param OutHits Array of hit results
     * @return Number of occluding hits
     */
    int32 PerformOcclusionTrace(
        const FStageExecutionContext& Context,
        const FVector& TargetLocation,
        TArray<FHitResult>& OutHits
    ) const;

    /**
     * Check if an actor should be considered for occlusion handling
     * @param Actor Actor to check
     * @return True if actor can be made transparent/hidden
     */
    bool ShouldHandleActor(AActor* Actor) const;

    /**
     * Add actor to tracked occluding actors
     * @param Actor Actor to track
     */
    void AddOccludingActor(AActor* Actor);

    /**
     * Remove actor from tracked occluding actors
     * @param Actor Actor to remove
     */
    void RemoveOccludingActor(AActor* Actor);

    /**
     * Clean up stale actor references
     */
    void CleanupStaleActors();

    /**
     * Get camera location from context
     * @param Context Execution context
     * @return Camera world location
     */
    FVector GetCameraLocation(const FStageExecutionContext& Context) const;

    /**
     * Get character location from context (with height offset)
     * @param Context Execution context
     * @return Character torso location
     */
    FVector GetCharacterLocation(const FStageExecutionContext& Context) const;

    //========================================
    // Common Configuration
    //========================================

    /** Occlusion trace channel */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Config")
    TEnumAsByte<ECollisionChannel> OcclusionTraceChannel;

    /** Minimum time before occlusion handling kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OcclusionDelay;

    /** Fade interpolation speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Config", meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float FadeSpeed;

    /** Height offset for character visibility check */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Config", meta = (ClampMin = "0.0", ClampMax = "200.0"))
    float CharacterHeightOffset;

    //========================================
    // Runtime State
    //========================================

    /** Currently tracked occluding actors */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> CurrentOccludingActors;

    /** Previously tracked actors (for fade out) */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> PreviousOccludingActors;

    /** Time character has been occluded */
    float OcclusionTime;

    /** Cached delta time */
    float CachedDeltaTime;
};


//========================================
// OC01: Fade Out Occlusion
//========================================

/**
 * UCameraCollision_OC01_FadeOut
 * 
 * OC01: Gradually fade occluding objects to transparency
 * 
 * When objects block the view of the character, this strategy
 * smoothly fades them to a semi-transparent state.
 * 
 * Use Cases:
 * - Default occlusion handling
 * - Walls and large objects
 * - When maintaining environmental awareness is important
 * 
 * Characteristics:
 * - Smooth visual transition
 * - Preserves object visibility (not fully hidden)
 * - Priority: 100 (default)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_OC01_FadeOut : public UCameraCollision_OcclusionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_OC01_FadeOut();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_OC01_FadeOut; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Gradually fade occluding objects to transparency"); 
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

    //========================================
    // Public Query
    //========================================

    /** Get current fade alpha for occluding objects */
    UFUNCTION(BlueprintPure, Category = "Occlusion")
    float GetCurrentFadeAlpha() const { return CurrentFadeAlpha; }

    /** Get list of currently faded actors */
    UFUNCTION(BlueprintPure, Category = "Occlusion")
    TArray<AActor*> GetFadedActors() const;

protected:
    //========================================
    // Configuration
    //========================================

    /** Target opacity for faded objects (0 = invisible, 1 = opaque) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|FadeOut", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TargetFadeOpacity;

    /** Minimum distance before fade starts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|FadeOut", meta = (ClampMin = "0.0"))
    float FadeStartDistance;

    /** Distance at which fade is complete */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|FadeOut", meta = (ClampMin = "0.0"))
    float FadeEndDistance;

    /** Use distance-based fade gradient */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|FadeOut")
    bool bUseDistanceBasedFade;

    //========================================
    // Runtime State
    //========================================

    /** Current fade alpha (0 = invisible, 1 = visible) */
    float CurrentFadeAlpha;

    /** Target fade alpha */
    float TargetFadeAlpha;

    //========================================
    // Helper Methods
    //========================================

    /** Apply fade to an actor */
    void ApplyFadeToActor(AActor* Actor, float Alpha);

    /** Restore actor to full opacity */
    void RestoreActor(AActor* Actor);
};


//========================================
// OC02: Hide Occlusion
//========================================

/**
 * UCameraCollision_OC02_Hide
 * 
 * OC02: Completely hide occluding objects
 * 
 * When objects block the view, this strategy completely hides them.
 * Simpler than fade but more abrupt visually.
 * 
 * Use Cases:
 * - Performance-critical situations
 * - When fade doesn't work well
 * - Small objects
 * 
 * Characteristics:
 * - Instant visibility change
 * - Simple implementation
 * - Priority: 105 (medium)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_OC02_Hide : public UCameraCollision_OcclusionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_OC02_Hide();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_OC02_Hide; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Completely hide occluding objects"); 
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

    //========================================
    // Public Query
    //========================================

    /** Get list of currently hidden actors */
    UFUNCTION(BlueprintPure, Category = "Occlusion")
    TArray<AActor*> GetHiddenActors() const;

protected:
    //========================================
    // Configuration
    //========================================

    /** Delay before hiding (prevents flickering) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Hide", meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float HideDelay;

    /** Delay before showing again after occlusion ends */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Hide", meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float ShowDelay;

    /** Use actor visibility or render custom depth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Hide")
    bool bUseActorVisibility;

    //========================================
    // Runtime State
    //========================================

    /** Actors currently hidden */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> HiddenActors;

    /** Time since occlusion started for each actor */
    TMap<TWeakObjectPtr<AActor>, float> ActorOcclusionTimes;

    //========================================
    // Helper Methods
    //========================================

    /** Hide an actor */
    void HideActor(AActor* Actor);

    /** Show a previously hidden actor */
    void ShowActor(AActor* Actor);
};


//========================================
// OC03: Pull In Further Occlusion
//========================================

/**
 * UCameraCollision_OC03_PullInFurther
 * 
 * OC03: Pull camera even closer to see past occlusion
 * 
 * When character is occluded, this strategy pulls the camera
 * even closer than normal collision response to ensure visibility.
 * 
 * Use Cases:
 * - When transparency isn't possible
 * - Complex geometry
 * - When object hiding isn't desirable
 * 
 * Characteristics:
 * - Changes camera distance
 * - Works with any object
 * - Priority: 110 (higher)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_OC03_PullInFurther : public UCameraCollision_OcclusionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_OC03_PullInFurther();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_OC03_PullInFurther; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Pull camera closer to see past occlusion"); 
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

    /** Additional pull-in distance when occluded */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|PullIn", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float AdditionalPullInDistance;

    /** Maximum total pull-in (including collision response) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|PullIn", meta = (ClampMin = "50.0", ClampMax = "400.0"))
    float MaxTotalPullIn;

    /** Pull-in interpolation speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|PullIn", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float PullInSpeed;

    /** Minimum distance to maintain from character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|PullIn", meta = (ClampMin = "20.0", ClampMax = "100.0"))
    float MinCharacterDistance;

    //========================================
    // Runtime State
    //========================================

    /** Current additional pull-in amount */
    float CurrentPullIn;

    /** Target pull-in amount */
    float TargetPullIn;
};


//========================================
// OC04: Dither Fade Occlusion
//========================================

/**
 * UCameraCollision_OC04_DitherFade
 * 
 * OC04: Use dither pattern for fade effect
 * 
 * Uses a dither/stipple pattern to fade occluding objects.
 * Provides a visually distinct and often more pleasing fade effect.
 * 
 * Use Cases:
 * - Stylized games
 * - When smooth alpha blending causes issues
 * - Retro or artistic effect
 * 
 * Characteristics:
 * - Dither-based transparency
 * - No sorting issues
 * - Priority: 120 (highest)
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API UCameraCollision_OC04_DitherFade : public UCameraCollision_OcclusionBase
{
    GENERATED_BODY()

public:
    UCameraCollision_OC04_DitherFade();

    //========================================
    // Strategy Identity
    //========================================

    virtual ECollisionStrategyID GetStrategyType() const override 
    { 
        return ECollisionStrategyID::Collision_OC04_DitherFade; 
    }
    
    virtual FString GetStrategyDescription() const override 
    { 
        return TEXT("Use dither pattern for fade effect"); 
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

    //========================================
    // Public Query
    //========================================

    /** Get current dither amount */
    UFUNCTION(BlueprintPure, Category = "Occlusion")
    float GetCurrentDitherAmount() const { return CurrentDitherAmount; }

protected:
    //========================================
    // Configuration
    //========================================

    /** Target dither amount (0 = fully visible, 1 = fully dithered) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Dither", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TargetDitherAmount;

    /** Dither fade speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Dither", meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float DitherFadeSpeed;

    /** Dither pattern scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Dither", meta = (ClampMin = "0.5", ClampMax = "4.0"))
    float DitherPatternScale;

    /** Use screen-space dithering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Dither")
    bool bScreenSpaceDither;

    /** Material parameter name for dither control */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occlusion|Dither")
    FName DitherParameterName;

    //========================================
    // Runtime State
    //========================================

    /** Current dither amount */
    float CurrentDitherAmount;

    /** Per-actor dither amounts */
    TMap<TWeakObjectPtr<AActor>, float> ActorDitherAmounts;

    //========================================
    // Helper Methods
    //========================================

    /** Apply dither to an actor */
    void ApplyDitherToActor(AActor* Actor, float DitherAmount);

    /** Remove dither from an actor */
    void RemoveDitherFromActor(AActor* Actor);

    /** Get or create dynamic material instance for dithering */
    UMaterialInstanceDynamic* GetDitherMaterial(AActor* Actor);
};
