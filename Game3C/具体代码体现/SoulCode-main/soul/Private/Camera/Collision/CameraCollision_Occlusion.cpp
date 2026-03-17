// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Collision/CameraCollision_Occlusion.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"

/**
 * CameraCollision_Occlusion.cpp
 * 
 * This file contains the implementations of the Occlusion base class
 * and all 4 Occlusion strategies (OC01-OC04).
 * 
 * Occlusion strategies handle situations where the player character
 * or lock-on target is blocked from camera view by environmental objects.
 * 
 * Implementation Structure:
 * - OcclusionBase: Common occlusion functionality
 * - OC01: FadeOut - Fade occluding objects to transparency
 * - OC02: Hide - Completely hide occluding objects
 * - OC03: PullInFurther - Pull camera closer to see past occlusion
 * - OC04: DitherFade - Use dither pattern for fade effect
 */


//========================================
// OcclusionBase - Common Occlusion Functionality
//========================================

UCameraCollision_OcclusionBase::UCameraCollision_OcclusionBase()
    : OcclusionTraceChannel(ECC_Visibility)
    , OcclusionDelay(0.1f)
    , FadeSpeed(8.0f)
    , CharacterHeightOffset(100.0f)
    , OcclusionTime(0.0f)
    , CachedDeltaTime(0.0f)
{
    // Occlusion strategies are enabled by default
    bIsEnabled = true;
    bCanBlend = false;  // Occlusion strategies typically don't blend
}

void UCameraCollision_OcclusionBase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
    
    // Cache delta time for use in Execute
    CachedDeltaTime = DeltaTime;
    
    // Clean up stale actor references
    CleanupStaleActors();
}

void UCameraCollision_OcclusionBase::Reset()
{
    Super::Reset();
    
    // Clear all tracked actors
    CurrentOccludingActors.Empty();
    PreviousOccludingActors.Empty();
    OcclusionTime = 0.0f;
    CachedDeltaTime = 0.0f;
}

int32 UCameraCollision_OcclusionBase::PerformOcclusionTrace(
    const FStageExecutionContext& Context,
    const FVector& TargetLocation,
    TArray<FHitResult>& OutHits) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    FVector CameraLocation = GetCameraLocation(Context);
    
    // Setup trace params
    FCollisionQueryParams Params;
    Params.bTraceComplex = false;
    Params.bReturnPhysicalMaterial = false;
    
    // Ignore character - get owner from camera manager
    if (OwnerCameraManager.IsValid())
    {
        if (AActor* Owner = OwnerCameraManager->GetOwner())
        {
            Params.AddIgnoredActor(Owner);
        }
    }
    
    // Ignore target
    if (Context.InputContext.TargetActor.IsValid())
    {
        Params.AddIgnoredActor(Context.InputContext.TargetActor.Get());
    }
    
    // Perform multi-trace to get all occluding objects
    bool bHit = World->LineTraceMultiByChannel(
        OutHits,
        CameraLocation,
        TargetLocation,
        OcclusionTraceChannel,
        Params
    );
    
    return OutHits.Num();
}

bool UCameraCollision_OcclusionBase::ShouldHandleActor(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Don't handle pawns/characters
    if (Actor->IsA(APawn::StaticClass()))
    {
        return false;
    }
    
    // Check if actor has any primitive components that can be faded
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    return PrimitiveComponents.Num() > 0;
}

void UCameraCollision_OcclusionBase::AddOccludingActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    TWeakObjectPtr<AActor> WeakActor = Actor;
    
    // Check if already tracked
    if (!CurrentOccludingActors.Contains(WeakActor))
    {
        CurrentOccludingActors.Add(WeakActor);
        
        // Remove from previous list if present
        PreviousOccludingActors.Remove(WeakActor);
    }
}

void UCameraCollision_OcclusionBase::RemoveOccludingActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    TWeakObjectPtr<AActor> WeakActor = Actor;
    
    // Move to previous list for fade out
    if (CurrentOccludingActors.Remove(WeakActor) > 0)
    {
        if (!PreviousOccludingActors.Contains(WeakActor))
        {
            PreviousOccludingActors.Add(WeakActor);
        }
    }
}

void UCameraCollision_OcclusionBase::CleanupStaleActors()
{
    // Remove invalid weak pointers from current list
    CurrentOccludingActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });
    
    // Remove invalid weak pointers from previous list
    PreviousOccludingActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });
}

FVector UCameraCollision_OcclusionBase::GetCameraLocation(const FStageExecutionContext& Context) const
{
    return Context.Output.FocusPoint - 
           Context.Output.Rotation.Vector() * Context.Output.Distance;
}

FVector UCameraCollision_OcclusionBase::GetCharacterLocation(const FStageExecutionContext& Context) const
{
    FVector CharLocation = Context.InputContext.CharacterLocation;
    CharLocation.Z += CharacterHeightOffset;
    return CharLocation;
}


//========================================
// OC01: Fade Out Implementation
//========================================

/**
 * Constructor - Configure fade out parameters
 * 
 * Fade out is the default occlusion handling method:
 * - Smoothly fades occluding objects to transparency
 * - Maintains environmental awareness
 * - Works with materials that support opacity
 */
UCameraCollision_OC01_FadeOut::UCameraCollision_OC01_FadeOut()
    : TargetFadeOpacity(0.3f)
    , FadeStartDistance(200.0f)
    , FadeEndDistance(50.0f)
    , bUseDistanceBasedFade(true)
    , CurrentFadeAlpha(1.0f)
    , TargetFadeAlpha(1.0f)
{
    // Default priority for fade out
    Priority = 100;
    FadeSpeed = 8.0f;
}

bool UCameraCollision_OC01_FadeOut::ShouldExecute(const FStageExecutionContext& Context) const
{
    // Always execute to handle fade in/out transitions
    return bIsEnabled;
}

bool UCameraCollision_OC01_FadeOut::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if character is occluded
    bool bIsOccluded = DetectionResult.bCharacterOccluded;
    
    // Store new actors for comparison
    TArray<TWeakObjectPtr<AActor>> NewOccludingActors;
    
    if (bIsOccluded)
    {
        // Update occlusion time
        OcclusionTime += CachedDeltaTime;
        
        // Only process after delay
        if (OcclusionTime >= OcclusionDelay)
        {
            // Perform our own trace to get occluding actors
            FVector CharacterLocation = GetCharacterLocation(Context);
            TArray<FHitResult> Hits;
            int32 HitCount = PerformOcclusionTrace(Context, CharacterLocation, Hits);
            
            // Process hits
            for (const FHitResult& Hit : Hits)
            {
                AActor* HitActor = Hit.GetActor();
                if (HitActor && ShouldHandleActor(HitActor))
                {
                    NewOccludingActors.Add(HitActor);
                    AddOccludingActor(HitActor);
                }
            }
        }
        
        // Calculate target fade based on detection result or distance
        if (bUseDistanceBasedFade)
        {
            float CurrentDistance = DetectionResult.bCollisionDetected ? 
                DetectionResult.SafeDistance : Context.Output.Distance;
            
            if (CurrentDistance >= FadeStartDistance)
            {
                TargetFadeAlpha = 1.0f;
            }
            else if (CurrentDistance <= FadeEndDistance)
            {
                TargetFadeAlpha = TargetFadeOpacity;
            }
            else
            {
                float FadeRange = FadeStartDistance - FadeEndDistance;
                float DistanceInRange = CurrentDistance - FadeEndDistance;
                float FadeRatio = DistanceInRange / FadeRange;
                TargetFadeAlpha = FMath::Lerp(TargetFadeOpacity, 1.0f, FadeRatio);
            }
        }
        else
        {
            // Simple binary fade when occluded
            TargetFadeAlpha = TargetFadeOpacity;
        }
    }
    else
    {
        // Not occluded - fade back to full opacity
        OcclusionTime = 0.0f;
        TargetFadeAlpha = 1.0f;
    }
    
    // Find actors that are no longer occluding
    for (const TWeakObjectPtr<AActor>& WeakActor : CurrentOccludingActors)
    {
        if (WeakActor.IsValid() && !NewOccludingActors.Contains(WeakActor))
        {
            // Move to previous for fade out
            if (!PreviousOccludingActors.Contains(WeakActor))
            {
                PreviousOccludingActors.Add(WeakActor);
            }
        }
    }
    
    // Update current list
    if (bIsOccluded && OcclusionTime >= OcclusionDelay)
    {
        CurrentOccludingActors = NewOccludingActors;
    }
    else if (!bIsOccluded)
    {
        // Move all current to previous for fade out
        for (const TWeakObjectPtr<AActor>& WeakActor : CurrentOccludingActors)
        {
            if (WeakActor.IsValid() && !PreviousOccludingActors.Contains(WeakActor))
            {
                PreviousOccludingActors.Add(WeakActor);
            }
        }
        CurrentOccludingActors.Empty();
    }
    
    // Interpolate fade alpha
    CurrentFadeAlpha = FMath::FInterpTo(CurrentFadeAlpha, TargetFadeAlpha, CachedDeltaTime, FadeSpeed);
    
    // Apply fade to current occluding actors
    for (const TWeakObjectPtr<AActor>& WeakActor : CurrentOccludingActors)
    {
        if (WeakActor.IsValid())
        {
            ApplyFadeToActor(WeakActor.Get(), CurrentFadeAlpha);
        }
    }
    
    // Fade out previous actors (restore to full opacity)
    TArray<TWeakObjectPtr<AActor>> ActorsToRemove;
    for (const TWeakObjectPtr<AActor>& WeakActor : PreviousOccludingActors)
    {
        if (WeakActor.IsValid())
        {
            // Get current opacity and fade towards 1.0
            // For simplicity, just restore immediately
            // In production, you'd track per-actor opacity
            RestoreActor(WeakActor.Get());
            ActorsToRemove.Add(WeakActor);
        }
    }
    
    // Remove fully restored actors
    for (const TWeakObjectPtr<AActor>& WeakActor : ActorsToRemove)
    {
        PreviousOccludingActors.Remove(WeakActor);
    }
    
    // Update active state
    SetActive(CurrentOccludingActors.Num() > 0 || PreviousOccludingActors.Num() > 0);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("OC01 FadeOut: Occluded=%d, Actors=%d, Alpha=%.2f"),
        bIsOccluded ? 1 : 0, CurrentOccludingActors.Num(), CurrentFadeAlpha);
    
    // Occlusion strategies don't produce camera adjustments
    return false;
}

void UCameraCollision_OC01_FadeOut::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_OC01_FadeOut::Reset()
{
    Super::Reset();
    
    // Restore all actors before clearing
    for (const TWeakObjectPtr<AActor>& WeakActor : CurrentOccludingActors)
    {
        if (WeakActor.IsValid())
        {
            RestoreActor(WeakActor.Get());
        }
    }
    
    for (const TWeakObjectPtr<AActor>& WeakActor : PreviousOccludingActors)
    {
        if (WeakActor.IsValid())
        {
            RestoreActor(WeakActor.Get());
        }
    }
    
    CurrentFadeAlpha = 1.0f;
    TargetFadeAlpha = 1.0f;
}

TArray<AActor*> UCameraCollision_OC01_FadeOut::GetFadedActors() const
{
    TArray<AActor*> Result;
    
    for (const TWeakObjectPtr<AActor>& WeakActor : CurrentOccludingActors)
    {
        if (WeakActor.IsValid())
        {
            Result.Add(WeakActor.Get());
        }
    }
    
    return Result;
}

void UCameraCollision_OC01_FadeOut::ApplyFadeToActor(AActor* Actor, float Alpha)
{
    if (!Actor)
    {
        return;
    }
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        int32 NumMaterials = Component->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            UMaterialInterface* Material = Component->GetMaterial(i);
            if (!Material)
            {
                continue;
            }
            
            // Try to get or create dynamic material instance
            UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material);
            if (!DynMaterial)
            {
                // Create dynamic instance
                DynMaterial = Component->CreateDynamicMaterialInstance(i, Material);
            }
            
            if (DynMaterial)
            {
                // Set opacity parameter (material must support this)
                DynMaterial->SetScalarParameterValue(FName("Opacity"), Alpha);
                
                // Also try common parameter names
                DynMaterial->SetScalarParameterValue(FName("OpacityMultiplier"), Alpha);
                DynMaterial->SetScalarParameterValue(FName("FadeAlpha"), Alpha);
            }
        }
    }
}

void UCameraCollision_OC01_FadeOut::RestoreActor(AActor* Actor)
{
    // Restore to full opacity
    ApplyFadeToActor(Actor, 1.0f);
}


//========================================
// OC02: Hide Implementation
//========================================

/**
 * Constructor - Configure hide parameters
 * 
 * Hide is a simple but effective occlusion handling method:
 * - Completely hides occluding objects
 * - Fast performance
 * - May cause visual popping if not handled carefully
 */
UCameraCollision_OC02_Hide::UCameraCollision_OC02_Hide()
    : HideDelay(0.1f)
    , ShowDelay(0.05f)
    , bUseActorVisibility(true)
{
    Priority = 105;
    OcclusionDelay = 0.1f;
}

bool UCameraCollision_OC02_Hide::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_OC02_Hide::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if character is occluded
    bool bIsOccluded = DetectionResult.bCharacterOccluded;
    
    // Track new occluding actors
    TArray<TWeakObjectPtr<AActor>> NewOccludingActors;
    
    if (bIsOccluded)
    {
        // Update occlusion time
        OcclusionTime += CachedDeltaTime;
        
        // Only process after delay to prevent flickering
        if (OcclusionTime >= OcclusionDelay)
        {
            // Perform trace to get occluding actors
            FVector CharacterLocation = GetCharacterLocation(Context);
            TArray<FHitResult> Hits;
            int32 HitCount = PerformOcclusionTrace(Context, CharacterLocation, Hits);
            
            // Process hits
            for (const FHitResult& Hit : Hits)
            {
                AActor* HitActor = Hit.GetActor();
                if (HitActor && ShouldHandleActor(HitActor))
                {
                    TWeakObjectPtr<AActor> WeakActor = HitActor;
                    NewOccludingActors.Add(WeakActor);
                    
                    // Check if this is a new actor to hide
                    if (!HiddenActors.Contains(WeakActor))
                    {
                        // Check actor-specific delay
                        float* OcclusionTimePtr = ActorOcclusionTimes.Find(WeakActor);
                        if (!OcclusionTimePtr)
                        {
                            // Start tracking this actor
                            ActorOcclusionTimes.Add(WeakActor, 0.0f);
                        }
                        else
                        {
                            *OcclusionTimePtr += CachedDeltaTime;
                            
                            // Hide after delay
                            if (*OcclusionTimePtr >= HideDelay)
                            {
                                HideActor(HitActor);
                                HiddenActors.AddUnique(WeakActor);
                            }
                        }
                    }
                    
                    // Update current occluding actors list
                    AddOccludingActor(HitActor);
                }
            }
        }
    }
    else
    {
        // Not occluded - reset occlusion time
        OcclusionTime = 0.0f;
    }
    
    // Find actors that are no longer occluding and should be shown
    TArray<TWeakObjectPtr<AActor>> ActorsToShow;
    
    for (const TWeakObjectPtr<AActor>& WeakActor : HiddenActors)
    {
        if (!WeakActor.IsValid())
        {
            ActorsToShow.Add(WeakActor);
            continue;
        }
        
        // Check if still in new occluding list
        if (!NewOccludingActors.Contains(WeakActor))
        {
            // Actor is no longer occluding
            float* OcclusionTimePtr = ActorOcclusionTimes.Find(WeakActor);
            if (OcclusionTimePtr)
            {
                *OcclusionTimePtr -= CachedDeltaTime;
                
                // Show after delay expires
                if (*OcclusionTimePtr <= -ShowDelay)
                {
                    ShowActor(WeakActor.Get());
                    ActorsToShow.Add(WeakActor);
                    ActorOcclusionTimes.Remove(WeakActor);
                }
            }
            else
            {
                // No tracking info, show immediately
                ShowActor(WeakActor.Get());
                ActorsToShow.Add(WeakActor);
            }
        }
    }
    
    // Remove shown actors from hidden list
    for (const TWeakObjectPtr<AActor>& WeakActor : ActorsToShow)
    {
        HiddenActors.Remove(WeakActor);
        RemoveOccludingActor(WeakActor.IsValid() ? WeakActor.Get() : nullptr);
    }
    
    // Update active state
    SetActive(HiddenActors.Num() > 0);
    
    UE_LOG(LogTemp, Verbose, TEXT("OC02 Hide: Occluded=%d, Hidden=%d"),
        bIsOccluded ? 1 : 0, HiddenActors.Num());
    
    // Hide strategy doesn't produce camera adjustments
    return false;
}

void UCameraCollision_OC02_Hide::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
    
    // Update occlusion times for all tracked actors
    TArray<TWeakObjectPtr<AActor>> StaleActors;
    
    for (auto& Pair : ActorOcclusionTimes)
    {
        if (!Pair.Key.IsValid())
        {
            StaleActors.Add(Pair.Key);
        }
    }
    
    // Remove stale entries
    for (const TWeakObjectPtr<AActor>& WeakActor : StaleActors)
    {
        ActorOcclusionTimes.Remove(WeakActor);
    }
}

void UCameraCollision_OC02_Hide::Reset()
{
    Super::Reset();
    
    // Show all hidden actors
    for (const TWeakObjectPtr<AActor>& WeakActor : HiddenActors)
    {
        if (WeakActor.IsValid())
        {
            ShowActor(WeakActor.Get());
        }
    }
    
    HiddenActors.Empty();
    ActorOcclusionTimes.Empty();
}

TArray<AActor*> UCameraCollision_OC02_Hide::GetHiddenActors() const
{
    TArray<AActor*> Result;
    
    for (const TWeakObjectPtr<AActor>& WeakActor : HiddenActors)
    {
        if (WeakActor.IsValid())
        {
            Result.Add(WeakActor.Get());
        }
    }
    
    return Result;
}

void UCameraCollision_OC02_Hide::HideActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (bUseActorVisibility)
    {
        Actor->SetActorHiddenInGame(true);
    }
    else
    {
        // Alternative: Use render custom depth for outline/silhouette
        TArray<UPrimitiveComponent*> Components;
        Actor->GetComponents<UPrimitiveComponent>(Components);
        
        for (UPrimitiveComponent* Component : Components)
        {
            if (Component)
            {
                Component->SetRenderCustomDepth(true);
                Component->SetCustomDepthStencilValue(1);
            }
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("OC02 Hide: Hiding actor %s"), *Actor->GetName());
}

void UCameraCollision_OC02_Hide::ShowActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (bUseActorVisibility)
    {
        Actor->SetActorHiddenInGame(false);
    }
    else
    {
        // Restore custom depth settings
        TArray<UPrimitiveComponent*> Components;
        Actor->GetComponents<UPrimitiveComponent>(Components);
        
        for (UPrimitiveComponent* Component : Components)
        {
            if (Component)
            {
                Component->SetRenderCustomDepth(false);
            }
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("OC02 Hide: Showing actor %s"), *Actor->GetName());
}


//========================================
// OC03: Pull In Further Implementation
//========================================

/**
 * Constructor - Configure pull-in parameters
 * 
 * Pull-in further is used when material-based solutions don't work:
 * - Pulls camera even closer than collision response
 * - Ensures character is visible without modifying materials
 * - Works with any object type
 */
UCameraCollision_OC03_PullInFurther::UCameraCollision_OC03_PullInFurther()
    : AdditionalPullInDistance(50.0f)
    , MaxTotalPullIn(200.0f)
    , PullInSpeed(10.0f)
    , MinCharacterDistance(30.0f)
    , CurrentPullIn(0.0f)
    , TargetPullIn(0.0f)
{
    Priority = 110;
    OcclusionDelay = 0.15f;
}

bool UCameraCollision_OC03_PullInFurther::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_OC03_PullInFurther::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if character is occluded
    bool bIsOccluded = DetectionResult.bCharacterOccluded;
    
    if (bIsOccluded)
    {
        // Update occlusion time
        OcclusionTime += CachedDeltaTime;
        
        // Only apply pull-in after delay
        if (OcclusionTime >= OcclusionDelay)
        {
            // Calculate how much additional pull-in is needed
            // Start with base additional distance
            float DesiredPullIn = AdditionalPullInDistance;
            
            // Increase pull-in over time if still occluded
            float TimeMultiplier = FMath::Clamp((OcclusionTime - OcclusionDelay) / 0.5f, 0.0f, 1.0f);
            DesiredPullIn *= (1.0f + TimeMultiplier * 0.5f);
            
            // Check current camera distance
            float CurrentDistance = DetectionResult.bCollisionDetected ? 
                DetectionResult.SafeDistance : Context.Output.Distance;
            
            // Ensure we don't pull in closer than minimum
            float MaxPullIn = FMath::Max(0.0f, CurrentDistance - MinCharacterDistance);
            MaxPullIn = FMath::Min(MaxPullIn, MaxTotalPullIn);
            
            TargetPullIn = FMath::Min(DesiredPullIn, MaxPullIn);
        }
        else
        {
            // During delay, don't change target
            TargetPullIn = CurrentPullIn;
        }
    }
    else
    {
        // Not occluded - return to normal
        OcclusionTime = 0.0f;
        TargetPullIn = 0.0f;
    }
    
    // Interpolate current pull-in towards target
    float InterpSpeed = bIsOccluded ? PullInSpeed : (PullInSpeed * 0.5f);  // Return slower
    CurrentPullIn = FMath::FInterpTo(CurrentPullIn, TargetPullIn, CachedDeltaTime, InterpSpeed);
    
    // Only output if we have significant pull-in
    if (FMath::Abs(CurrentPullIn) > 0.1f)
    {
        OutResponse.Reset();
        OutResponse.StrategyType = GetStrategyType();
        OutResponse.Category = GetStrategyCategory();
        OutResponse.DistanceAdjustment = -CurrentPullIn;  // Negative = closer
        OutResponse.Weight = BlendWeight;
        OutResponse.Priority = GetPriority();
        OutResponse.bIsActive = true;
        OutResponse.ActiveDuration = OcclusionTime;
        
        SetActive(true);
        
        UE_LOG(LogTemp, Verbose, TEXT("OC03 PullInFurther: Occluded=%d, PullIn=%.1f (target=%.1f)"),
            bIsOccluded ? 1 : 0, CurrentPullIn, TargetPullIn);
        
        return true;
    }
    
    SetActive(false);
    return false;
}

void UCameraCollision_OC03_PullInFurther::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_OC03_PullInFurther::Reset()
{
    Super::Reset();
    CurrentPullIn = 0.0f;
    TargetPullIn = 0.0f;
}


//========================================
// OC04: Dither Fade Implementation
//========================================

/**
 * Constructor - Configure dither fade parameters
 * 
 * Dither fade provides a stylized transparency effect:
 * - Uses stipple/dither pattern instead of alpha blending
 * - Avoids transparency sorting issues
 * - Works well with deferred rendering
 */
UCameraCollision_OC04_DitherFade::UCameraCollision_OC04_DitherFade()
    : TargetDitherAmount(0.7f)
    , DitherFadeSpeed(10.0f)
    , DitherPatternScale(1.0f)
    , bScreenSpaceDither(true)
    , DitherParameterName(TEXT("DitherAmount"))
    , CurrentDitherAmount(0.0f)
{
    Priority = 120;  // Highest priority among occlusion strategies
    OcclusionDelay = 0.05f;  // Shorter delay for responsive dither
    FadeSpeed = 10.0f;
}

bool UCameraCollision_OC04_DitherFade::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_OC04_DitherFade::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if character is occluded
    bool bIsOccluded = DetectionResult.bCharacterOccluded;
    
    // Track new occluding actors this frame
    TArray<TWeakObjectPtr<AActor>> NewOccludingActors;
    float TargetDither = 0.0f;
    
    if (bIsOccluded)
    {
        // Update occlusion time
        OcclusionTime += CachedDeltaTime;
        
        // Only process after delay
        if (OcclusionTime >= OcclusionDelay)
        {
            TargetDither = TargetDitherAmount;
            
            // Perform trace to get occluding actors
            FVector CharacterLocation = GetCharacterLocation(Context);
            TArray<FHitResult> Hits;
            int32 HitCount = PerformOcclusionTrace(Context, CharacterLocation, Hits);
            
            // Process hits
            for (const FHitResult& Hit : Hits)
            {
                AActor* HitActor = Hit.GetActor();
                if (HitActor && ShouldHandleActor(HitActor))
                {
                    TWeakObjectPtr<AActor> WeakActor = HitActor;
                    NewOccludingActors.Add(WeakActor);
                    
                    // Add to current occluding actors
                    AddOccludingActor(HitActor);
                    
                    // Initialize dither amount if new
                    if (!ActorDitherAmounts.Contains(WeakActor))
                    {
                        ActorDitherAmounts.Add(WeakActor, 0.0f);
                    }
                }
            }
        }
    }
    else
    {
        // Not occluded - reset
        OcclusionTime = 0.0f;
        TargetDither = 0.0f;
    }
    
    // Interpolate global dither amount
    CurrentDitherAmount = FMath::FInterpTo(CurrentDitherAmount, TargetDither, CachedDeltaTime, DitherFadeSpeed);
    
    // Update dither for each tracked actor
    TArray<TWeakObjectPtr<AActor>> ActorsToRemove;
    
    for (auto& Pair : ActorDitherAmounts)
    {
        TWeakObjectPtr<AActor> WeakActor = Pair.Key;
        
        if (!WeakActor.IsValid())
        {
            ActorsToRemove.Add(WeakActor);
            continue;
        }
        
        float& CurrentActorDither = Pair.Value;
        float ActorTargetDither = 0.0f;
        
        // Check if this actor is still occluding
        if (NewOccludingActors.Contains(WeakActor))
        {
            ActorTargetDither = TargetDitherAmount;
        }
        
        // Interpolate this actor's dither
        CurrentActorDither = FMath::FInterpTo(CurrentActorDither, ActorTargetDither, CachedDeltaTime, DitherFadeSpeed);
        
        // Apply dither to actor
        ApplyDitherToActor(WeakActor.Get(), CurrentActorDither);
        
        // Remove if fully faded out
        if (CurrentActorDither < 0.01f && ActorTargetDither < 0.01f)
        {
            RemoveDitherFromActor(WeakActor.Get());
            ActorsToRemove.Add(WeakActor);
            RemoveOccludingActor(WeakActor.Get());
        }
    }
    
    // Clean up removed actors
    for (const TWeakObjectPtr<AActor>& WeakActor : ActorsToRemove)
    {
        ActorDitherAmounts.Remove(WeakActor);
    }
    
    // Update current occluding actors list
    CurrentOccludingActors = NewOccludingActors;
    
    // Update active state
    SetActive(ActorDitherAmounts.Num() > 0);
    
    UE_LOG(LogTemp, Verbose, TEXT("OC04 DitherFade: Occluded=%d, Actors=%d, GlobalDither=%.2f"),
        bIsOccluded ? 1 : 0, ActorDitherAmounts.Num(), CurrentDitherAmount);
    
    // Dither strategy doesn't produce camera adjustments
    return false;
}

void UCameraCollision_OC04_DitherFade::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
    
    // Clean up stale actor references from dither map
    TArray<TWeakObjectPtr<AActor>> StaleActors;
    
    for (const auto& Pair : ActorDitherAmounts)
    {
        if (!Pair.Key.IsValid())
        {
            StaleActors.Add(Pair.Key);
        }
    }
    
    for (const TWeakObjectPtr<AActor>& WeakActor : StaleActors)
    {
        ActorDitherAmounts.Remove(WeakActor);
    }
}

void UCameraCollision_OC04_DitherFade::Reset()
{
    Super::Reset();
    
    // Remove dither from all tracked actors
    for (const auto& Pair : ActorDitherAmounts)
    {
        if (Pair.Key.IsValid())
        {
            RemoveDitherFromActor(Pair.Key.Get());
        }
    }
    
    CurrentDitherAmount = 0.0f;
    ActorDitherAmounts.Empty();
}

void UCameraCollision_OC04_DitherFade::ApplyDitherToActor(AActor* Actor, float DitherAmount)
{
    if (!Actor)
    {
        return;
    }
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        int32 NumMaterials = Component->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            UMaterialInterface* Material = Component->GetMaterial(i);
            if (!Material)
            {
                continue;
            }
            
            // Try to get or create dynamic material instance
            UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material);
            if (!DynMaterial)
            {
                DynMaterial = Component->CreateDynamicMaterialInstance(i, Material);
            }
            
            if (DynMaterial)
            {
                // Set dither amount parameter
                DynMaterial->SetScalarParameterValue(DitherParameterName, DitherAmount);
                
                // Also try common parameter names
                DynMaterial->SetScalarParameterValue(FName("DitherOpacity"), DitherAmount);
                DynMaterial->SetScalarParameterValue(FName("ScreenDoor"), DitherAmount);
                
                // Set pattern scale if using screen space dither
                if (bScreenSpaceDither)
                {
                    DynMaterial->SetScalarParameterValue(FName("DitherScale"), DitherPatternScale);
                    DynMaterial->SetScalarParameterValue(FName("PatternScale"), DitherPatternScale);
                }
            }
        }
    }
}

void UCameraCollision_OC04_DitherFade::RemoveDitherFromActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Reset dither to 0 (fully visible)
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        int32 NumMaterials = Component->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            UMaterialInterface* Material = Component->GetMaterial(i);
            UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material);
            
            if (DynMaterial)
            {
                // Reset dither parameters
                DynMaterial->SetScalarParameterValue(DitherParameterName, 0.0f);
                DynMaterial->SetScalarParameterValue(FName("DitherOpacity"), 0.0f);
                DynMaterial->SetScalarParameterValue(FName("ScreenDoor"), 0.0f);
            }
        }
    }
}

UMaterialInstanceDynamic* UCameraCollision_OC04_DitherFade::GetDitherMaterial(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }
    
    // Find first dynamic material instance on this actor
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        int32 NumMaterials = Component->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            UMaterialInterface* Material = Component->GetMaterial(i);
            UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material);
            
            if (DynMaterial)
            {
                return DynMaterial;
            }
        }
    }
    
    return nullptr;
}
