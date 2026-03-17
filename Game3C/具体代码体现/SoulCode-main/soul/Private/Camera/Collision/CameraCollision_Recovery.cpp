// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Collision/CameraCollision_Recovery.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

/**
 * CameraCollision_Recovery.cpp
 * 
 * This file contains the implementations of the Recovery base class
 * and all 3 Recovery strategies (RC01-RC03).
 * 
 * Recovery strategies handle returning the camera to its normal position
 * after a collision has ended. They ensure smooth, natural transitions
 * back to the desired camera distance.
 * 
 * Implementation Structure:
 * - RecoveryBase: Common recovery functionality
 * - RC01: DelayedRecovery - Wait before starting recovery
 * - RC02: SmoothRecovery - Smooth interpolation recovery
 * - RC03: StepRecovery - Step-based discrete recovery
 */


//========================================
// RecoveryBase - Common Recovery Functionality
//========================================

UCameraCollision_RecoveryBase::UCameraCollision_RecoveryBase()
    : BaseRecoverySpeed(5.0f)
    , MinRecoveryTime(0.1f)
    , CompletionTolerance(1.0f)
    , bIsRecovering(false)
    , RecoveryProgress(0.0f)
    , RecoveryTime(0.0f)
    , RecoveryStartDistance(0.0f)
    , RecoveryTargetDistance(0.0f)
    , CurrentRecoveryDistance(0.0f)
    , bWasInCollision(false)
    , CachedDeltaTime(0.0f)
{
    // Recovery strategies are enabled by default
    bIsEnabled = true;
    bCanBlend = false;  // Recovery strategies typically don't blend
}

void UCameraCollision_RecoveryBase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
    
    // Cache delta time for use in Execute
    CachedDeltaTime = DeltaTime;
}

void UCameraCollision_RecoveryBase::Reset()
{
    Super::Reset();
    
    bIsRecovering = false;
    RecoveryProgress = 0.0f;
    RecoveryTime = 0.0f;
    RecoveryStartDistance = 0.0f;
    RecoveryTargetDistance = 0.0f;
    CurrentRecoveryDistance = 0.0f;
    bWasInCollision = false;
    CachedDeltaTime = 0.0f;
}

void UCameraCollision_RecoveryBase::BeginRecovery(float StartDistance, float TargetDistance)
{
    bIsRecovering = true;
    RecoveryProgress = 0.0f;
    RecoveryTime = 0.0f;
    RecoveryStartDistance = StartDistance;
    RecoveryTargetDistance = TargetDistance;
    CurrentRecoveryDistance = StartDistance;
    
    UE_LOG(LogTemp, Verbose, TEXT("RecoveryBase: Beginning recovery from %.1f to %.1f"),
        StartDistance, TargetDistance);
}

void UCameraCollision_RecoveryBase::EndRecovery()
{
    bIsRecovering = false;
    RecoveryProgress = 1.0f;
    bWasInCollision = false;
    
    UE_LOG(LogTemp, Verbose, TEXT("RecoveryBase: Recovery complete"));
}

float UCameraCollision_RecoveryBase::UpdateRecoveryProgress(float DeltaTime, float Speed)
{
    if (!bIsRecovering)
    {
        return CurrentRecoveryDistance;
    }
    
    RecoveryTime += DeltaTime;
    
    // Calculate progress based on time and speed
    float TotalDistance = FMath::Abs(RecoveryTargetDistance - RecoveryStartDistance);
    if (TotalDistance > 0.0f)
    {
        float DistanceRecovered = FMath::Abs(CurrentRecoveryDistance - RecoveryStartDistance);
        RecoveryProgress = FMath::Clamp(DistanceRecovered / TotalDistance, 0.0f, 1.0f);
    }
    else
    {
        RecoveryProgress = 1.0f;
    }
    
    return CurrentRecoveryDistance;
}

bool UCameraCollision_RecoveryBase::ShouldBeginRecovery(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult) const
{
    // Begin recovery when:
    // 1. Was in collision before
    // 2. No longer in collision
    // 3. Current distance is less than desired distance
    
    if (DetectionResult.bCollisionDetected)
    {
        // Still in collision - no recovery yet
        return false;
    }
    
    if (!bWasInCollision && !bIsRecovering)
    {
        // Wasn't in collision, nothing to recover from
        return false;
    }
    
    // Check if distance needs recovery
    float CurrentDistance = Context.Output.Distance;
    float DesiredDistance = DetectionResult.DesiredDistance;
    
    if (DesiredDistance <= 0.0f)
    {
        // No valid desired distance
        return false;
    }
    
    float DistanceDelta = DesiredDistance - CurrentDistance;
    return DistanceDelta > CompletionTolerance;
}

float UCameraCollision_RecoveryBase::CalculateDistanceAdjustment(
    float CurrentDistance,
    float TargetDistance,
    float DeltaTime,
    float Speed) const
{
    float DistanceDelta = TargetDistance - CurrentDistance;
    
    if (FMath::Abs(DistanceDelta) <= CompletionTolerance)
    {
        // Close enough - snap to target
        return DistanceDelta;
    }
    
    // Calculate adjustment based on speed
    float MaxAdjustment = Speed * DeltaTime;
    float Adjustment = FMath::Sign(DistanceDelta) * FMath::Min(MaxAdjustment, FMath::Abs(DistanceDelta));
    
    return Adjustment;
}


//========================================
// RC01: Delayed Recovery Implementation
//========================================

/**
 * Constructor - Configure delayed recovery parameters
 * 
 * Delayed recovery waits before starting:
 * - Prevents camera jitter from rapid collision enter/exit
 * - Configurable delay time
 * - Uses smooth recovery once delay completes
 */
UCameraCollision_RC01_DelayedRecovery::UCameraCollision_RC01_DelayedRecovery()
    : RecoveryDelay(0.2f)
    , DelayedRecoverySpeed(8.0f)
    , bResetDelayOnCollision(true)
    , TimeSinceCollisionEnd(0.0f)
    , bDelayComplete(false)
{
    Priority = 100;
    BaseRecoverySpeed = 8.0f;
}

bool UCameraCollision_RC01_DelayedRecovery::ShouldExecute(const FStageExecutionContext& Context) const
{
    // Execute when recovering or was in collision
    return bIsEnabled && (bIsRecovering || bWasInCollision);
}

bool UCameraCollision_RC01_DelayedRecovery::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    bool bCurrentlyInCollision = DetectionResult.bCollisionDetected;
    
    // Track collision state
    if (bCurrentlyInCollision)
    {
        // In collision - reset recovery state
        bWasInCollision = true;
        TimeSinceCollisionEnd = 0.0f;
        bDelayComplete = false;
        CurrentRecoveryDistance = Context.Output.Distance;
        
        if (bIsRecovering && bResetDelayOnCollision)
        {
            // Re-entered collision during recovery - reset
            bIsRecovering = false;
            RecoveryProgress = 0.0f;
        }
        
        SetActive(false);
        return false;
    }
    
    // Not in collision
    if (!bWasInCollision && !bIsRecovering)
    {
        // Wasn't in collision, nothing to do
        SetActive(false);
        return false;
    }
    
    // Update time since collision ended
    TimeSinceCollisionEnd += CachedDeltaTime;
    
    // Check if delay has passed
    if (!bDelayComplete)
    {
        if (TimeSinceCollisionEnd < RecoveryDelay)
        {
            // Still waiting for delay
            SetActive(true);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("RC01 DelayedRecovery: Waiting... %.2f/%.2f"),
                TimeSinceCollisionEnd, RecoveryDelay);
            
            return false;
        }
        
        // Delay complete - begin recovery
        bDelayComplete = true;
        float CurrentDistance = Context.Output.Distance;
        float TargetDistance = DetectionResult.DesiredDistance > 0.0f ? 
            DetectionResult.DesiredDistance : CurrentDistance + 100.0f;
        
        BeginRecovery(CurrentDistance, TargetDistance);
    }
    
    // Perform recovery
    if (!bIsRecovering)
    {
        SetActive(false);
        return false;
    }
    
    SetActive(true);
    
    // Calculate target distance
    float CurrentDistance = Context.Output.Distance;
    float TargetDistance = RecoveryTargetDistance;
    float DistanceDelta = TargetDistance - CurrentDistance;
    
    // Check if recovery is complete
    if (FMath::Abs(DistanceDelta) <= CompletionTolerance)
    {
        EndRecovery();
        bWasInCollision = false;
        TimeSinceCollisionEnd = 0.0f;
        bDelayComplete = false;
        SetActive(false);
        return false;
    }
    
    // Calculate recovery amount
    float RecoveryAmount = CalculateDistanceAdjustment(
        CurrentDistance, TargetDistance, CachedDeltaTime, DelayedRecoverySpeed);
    
    // Verify recovery path is clear
    FVector FocusPoint = Context.Output.FocusPoint;
    FVector CameraDirection = -Context.Output.Rotation.Vector();
    FVector TargetCameraPos = FocusPoint + CameraDirection * (CurrentDistance + RecoveryAmount);
    
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    FHitResult Hit;
    bool bBlocked = LineTrace(FocusPoint, TargetCameraPos, Hit, Params);
    
    if (bBlocked && Hit.Distance < CurrentDistance + RecoveryAmount)
    {
        // Path blocked - limit recovery
        float SafeRecoveryAmount = FMath::Max(0.0f, Hit.Distance - CurrentDistance - ProbeRadius);
        RecoveryAmount = SafeRecoveryAmount;
        
        if (RecoveryAmount <= 0.0f)
        {
            // Still blocked - wait
            UE_LOG(LogTemp, VeryVerbose, TEXT("RC01 DelayedRecovery: Path blocked, waiting"));
            return false;
        }
    }
    
    // Update current recovery distance
    CurrentRecoveryDistance = CurrentDistance + RecoveryAmount;
    UpdateRecoveryProgress(CachedDeltaTime, DelayedRecoverySpeed);
    
    // Build response
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    OutResponse.DistanceAdjustment = RecoveryAmount;  // Positive = moving away (recovering)
    OutResponse.Weight = BlendWeight;
    OutResponse.Priority = GetPriority();
    OutResponse.bIsActive = true;
    OutResponse.ActiveDuration = RecoveryTime;
    
    UE_LOG(LogTemp, Verbose, TEXT("RC01 DelayedRecovery: Recovering +%.2f (progress: %.0f%%)"),
        RecoveryAmount, RecoveryProgress * 100.0f);
    
    return true;
}

void UCameraCollision_RC01_DelayedRecovery::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_RC01_DelayedRecovery::Reset()
{
    Super::Reset();
    
    TimeSinceCollisionEnd = 0.0f;
    bDelayComplete = false;
}


//========================================
// RC02: Smooth Recovery Implementation
//========================================

/**
 * Constructor - Configure smooth recovery parameters
 * 
 * Smooth recovery uses continuous interpolation: 
 * - Natural feeling recovery
 * - Optional ease-out for deceleration at end
 * - Most commonly used recovery type
 */
UCameraCollision_RC02_SmoothRecovery::UCameraCollision_RC02_SmoothRecovery()
    : SmoothRecoverySpeed(10.0f)
    , bUseEaseOut(true)
    , EaseOutExponent(2.0f)
    , InterpAlpha(0.0f)
{
    Priority = 105;
    BaseRecoverySpeed = 10.0f;
}

bool UCameraCollision_RC02_SmoothRecovery::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled && (bIsRecovering || bWasInCollision);
}

bool UCameraCollision_RC02_SmoothRecovery::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    bool bCurrentlyInCollision = DetectionResult.bCollisionDetected;
    
    // Track collision state
    if (bCurrentlyInCollision)
    {
        // In collision - track state but don't recover
        bWasInCollision = true;
        CurrentRecoveryDistance = Context.Output.Distance;
        InterpAlpha = 0.0f;
        
        if (bIsRecovering)
        {
            // Collision during recovery - reset
            bIsRecovering = false;
            RecoveryProgress = 0.0f;
        }
        
        SetActive(false);
        return false;
    }
    
    // Not in collision
    if (!bWasInCollision && !bIsRecovering)
    {
        SetActive(false);
        return false;
    }
    
    // Begin recovery if not already
    if (!bIsRecovering)
    {
        float CurrentDistance = Context.Output.Distance;
        float TargetDistance = DetectionResult.DesiredDistance > 0.0f ? 
            DetectionResult.DesiredDistance : CurrentDistance + 100.0f;
        
        if (FMath::Abs(TargetDistance - CurrentDistance) > CompletionTolerance)
        {
            BeginRecovery(CurrentDistance, TargetDistance);
            InterpAlpha = 0.0f;
        }
        else
        {
            // Already at target
            bWasInCollision = false;
            SetActive(false);
            return false;
        }
    }
    
    SetActive(true);
    
    // Calculate interpolation
    float CurrentDistance = Context.Output.Distance;
    float TargetDistance = RecoveryTargetDistance;
    float DistanceDelta = TargetDistance - CurrentDistance;
    
    // Check if recovery is complete
    if (FMath::Abs(DistanceDelta) <= CompletionTolerance)
    {
        EndRecovery();
        bWasInCollision = false;
        InterpAlpha = 0.0f;
        SetActive(false);
        return false;
    }
    
    // Update interpolation alpha
    InterpAlpha += CachedDeltaTime * SmoothRecoverySpeed / 
        FMath::Max(1.0f, FMath::Abs(RecoveryTargetDistance - RecoveryStartDistance));
    InterpAlpha = FMath::Clamp(InterpAlpha, 0.0f, 1.0f);
    
    // Apply ease-out if enabled
    float EffectiveAlpha = InterpAlpha;
    if (bUseEaseOut)
    {
        // Ease-out: 1 - (1 - t)^exponent
        EffectiveAlpha = 1.0f - FMath::Pow(1.0f - InterpAlpha, EaseOutExponent);
    }
    
    // Calculate target position for this frame
    float InterpolatedDistance = FMath::Lerp(RecoveryStartDistance, RecoveryTargetDistance, EffectiveAlpha);
    float RecoveryAmount = InterpolatedDistance - CurrentDistance;
    
    // Ensure we don't overshoot
    RecoveryAmount = FMath::Min(RecoveryAmount, FMath::Abs(DistanceDelta));
    
    // Verify recovery path is clear
    FVector FocusPoint = Context.Output.FocusPoint;
    FVector CameraDirection = -Context.Output.Rotation.Vector();
    FVector TargetCameraPos = FocusPoint + CameraDirection * (CurrentDistance + RecoveryAmount);
    
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    FHitResult Hit;
    bool bBlocked = LineTrace(FocusPoint, TargetCameraPos, Hit, Params);
    
    if (bBlocked && Hit.Distance < CurrentDistance + RecoveryAmount)
    {
        // Path blocked - limit recovery
        float SafeRecoveryAmount = FMath::Max(0.0f, Hit.Distance - CurrentDistance - ProbeRadius);
        RecoveryAmount = SafeRecoveryAmount;
        
        if (RecoveryAmount <= 0.0f)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("RC02 SmoothRecovery: Path blocked, waiting"));
            return false;
        }
    }
    
    // Update state
    CurrentRecoveryDistance = CurrentDistance + RecoveryAmount;
    UpdateRecoveryProgress(CachedDeltaTime, SmoothRecoverySpeed);
    
    // Build response
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    OutResponse.DistanceAdjustment = RecoveryAmount;
    OutResponse.Weight = BlendWeight;
    OutResponse.Priority = GetPriority();
    OutResponse.bIsActive = true;
    OutResponse.ActiveDuration = RecoveryTime;
    
    UE_LOG(LogTemp, Verbose, TEXT("RC02 SmoothRecovery: Recovering +%.2f (alpha: %.2f, progress: %.0f%%)"),
        RecoveryAmount, EffectiveAlpha, RecoveryProgress * 100.0f);
    
    return true;
}

void UCameraCollision_RC02_SmoothRecovery::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_RC02_SmoothRecovery::Reset()
{
    Super::Reset();
    InterpAlpha = 0.0f;
}


//========================================
// RC03: Step Recovery Implementation
//========================================

/**
 * Constructor - Configure step recovery parameters
 * 
 * Step recovery moves in discrete steps:
 * - Predictable, controlled recovery
 * - Good for stylized camera behavior
 * - Optional smoothing within each step
 */
UCameraCollision_RC03_StepRecovery::UCameraCollision_RC03_StepRecovery()
    : StepDistance(20.0f)
    , StepInterval(0.1f)
    , bSmoothWithinStep(true)
    , MaxSteps(0)
    , CurrentStep(0)
    , TotalSteps(0)
    , TimeSinceLastStep(0.0f)
    , StepStartDistance(0.0f)
    , StepTargetDistance(0.0f)
{
    Priority = 110;
    BaseRecoverySpeed = 15.0f;
}

bool UCameraCollision_RC03_StepRecovery::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled && (bIsRecovering || bWasInCollision);
}

bool UCameraCollision_RC03_StepRecovery::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    bool bCurrentlyInCollision = DetectionResult.bCollisionDetected;
    
    // Track collision state
    if (bCurrentlyInCollision)
    {
        // In collision - reset recovery
        bWasInCollision = true;
        CurrentRecoveryDistance = Context.Output.Distance;
        
        if (bIsRecovering)
        {
            bIsRecovering = false;
            CurrentStep = 0;
            TotalSteps = 0;
            TimeSinceLastStep = 0.0f;
        }
        
        SetActive(false);
        return false;
    }
    
    // Not in collision
    if (!bWasInCollision && !bIsRecovering)
    {
        SetActive(false);
        return false;
    }
    
    // Begin recovery if not already
    if (!bIsRecovering)
    {
        float CurrentDistance = Context.Output.Distance;
        float TargetDistance = DetectionResult.DesiredDistance > 0.0f ? 
            DetectionResult.DesiredDistance : CurrentDistance + 100.0f;
        
        float TotalDistance = FMath::Abs(TargetDistance - CurrentDistance);
        
        if (TotalDistance > CompletionTolerance)
        {
            BeginRecovery(CurrentDistance, TargetDistance);
            
            // Calculate total steps needed
            TotalSteps = FMath::CeilToInt(TotalDistance / StepDistance);
            if (MaxSteps > 0)
            {
                TotalSteps = FMath::Min(TotalSteps, MaxSteps);
            }
            
            CurrentStep = 0;
            TimeSinceLastStep = StepInterval; // Start first step immediately
            StepStartDistance = CurrentDistance;
            StepTargetDistance = CurrentDistance + StepDistance;
            StepTargetDistance = FMath::Min(StepTargetDistance, TargetDistance);
            
            UE_LOG(LogTemp, Verbose, TEXT("RC03 StepRecovery: Starting recovery, %d steps planned"),
                TotalSteps);
        }
        else
        {
            bWasInCollision = false;
            SetActive(false);
            return false;
        }
    }
    
    SetActive(true);
    
    // Update step timer
    TimeSinceLastStep += CachedDeltaTime;
    
    float CurrentDistance = Context.Output.Distance;
    float FinalTargetDistance = RecoveryTargetDistance;
    
    // Check if all steps complete
    if (CurrentStep >= TotalSteps || 
        FMath::Abs(FinalTargetDistance - CurrentDistance) <= CompletionTolerance)
    {
        EndRecovery();
        bWasInCollision = false;
        CurrentStep = 0;
        TotalSteps = 0;
        TimeSinceLastStep = 0.0f;
        SetActive(false);
        return false;
    }
    
    // Check if it's time for next step
    if (TimeSinceLastStep >= StepInterval)
    {
        // Start new step
        TimeSinceLastStep = 0.0f;
        CurrentStep++;
        StepStartDistance = CurrentDistance;
        StepTargetDistance = CurrentDistance + StepDistance;
        StepTargetDistance = FMath::Min(StepTargetDistance, FinalTargetDistance);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("RC03 StepRecovery: Starting step %d/%d"),
            CurrentStep, TotalSteps);
    }
    
    // Calculate recovery amount for this frame
    float RecoveryAmount = 0.0f;
    
    if (bSmoothWithinStep)
    {
        // Smooth interpolation within the step
        float StepProgress = TimeSinceLastStep / StepInterval;
        StepProgress = FMath::Clamp(StepProgress, 0.0f, 1.0f);
        
        float StepInterpolatedDistance = FMath::Lerp(StepStartDistance, StepTargetDistance, StepProgress);
        RecoveryAmount = StepInterpolatedDistance - CurrentDistance;
    }
    else
    {
        // Instant step at the start of each interval
        if (TimeSinceLastStep < CachedDeltaTime * 1.5f)
        {
            RecoveryAmount = StepTargetDistance - CurrentDistance;
        }
    }
    
    // Clamp recovery amount
    RecoveryAmount = FMath::Max(0.0f, RecoveryAmount);
    RecoveryAmount = FMath::Min(RecoveryAmount, FMath::Abs(FinalTargetDistance - CurrentDistance));
    
    if (RecoveryAmount <= 0.0f)
    {
        return false;
    }
    
    // Verify recovery path is clear
    FVector FocusPoint = Context.Output.FocusPoint;
    FVector CameraDirection = -Context.Output.Rotation.Vector();
    FVector TargetCameraPos = FocusPoint + CameraDirection * (CurrentDistance + RecoveryAmount);
    
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    FHitResult Hit;
    bool bBlocked = LineTrace(FocusPoint, TargetCameraPos, Hit, Params);
    
    if (bBlocked && Hit.Distance < CurrentDistance + RecoveryAmount)
    {
        // Path blocked - limit recovery
        float SafeRecoveryAmount = FMath::Max(0.0f, Hit.Distance - CurrentDistance - ProbeRadius);
        RecoveryAmount = SafeRecoveryAmount;
        
        if (RecoveryAmount <= 0.0f)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("RC03 StepRecovery: Path blocked at step %d"), CurrentStep);
            return false;
        }
    }
    
    // Update state
    CurrentRecoveryDistance = CurrentDistance + RecoveryAmount;
    RecoveryProgress = static_cast<float>(CurrentStep) / static_cast<float>(FMath::Max(1, TotalSteps));
    RecoveryTime += CachedDeltaTime;
    
    // Build response
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    OutResponse.DistanceAdjustment = RecoveryAmount;
    OutResponse.Weight = BlendWeight;
    OutResponse.Priority = GetPriority();
    OutResponse.bIsActive = true;
    OutResponse.ActiveDuration = RecoveryTime;
    
    UE_LOG(LogTemp, Verbose, TEXT("RC03 StepRecovery: Step %d/%d, recovering +%.2f"),
        CurrentStep, TotalSteps, RecoveryAmount);
    
    return true;
}

void UCameraCollision_RC03_StepRecovery::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_RC03_StepRecovery::Reset()
{
    Super::Reset();
    
    CurrentStep = 0;
    TotalSteps = 0;
    TimeSinceLastStep = 0.0f;
    StepStartDistance = 0.0f;
    StepTargetDistance = 0.0f;
}
