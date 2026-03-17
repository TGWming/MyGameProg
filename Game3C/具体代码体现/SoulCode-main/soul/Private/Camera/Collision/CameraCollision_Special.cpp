// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Collision/CameraCollision_Special.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

/**
 * CameraCollision_Special.cpp
 * 
 * This file contains the implementations of the Special base class
 * and all 4 Special strategies (SP01-SP04).
 * 
 * Special strategies handle unique environmental situations that require
 * custom camera behavior beyond standard collision response.
 * 
 * Implementation Structure:
 * - SpecialBase: Common environment detection functionality
 * - SP01: TightSpace - Confined area handling
 * - SP02: LowCeiling - Low overhead handling
 * - SP03: CliffEdge - Edge/drop-off handling
 * - SP04: CornerCase - Corner geometry handling
 */


//========================================
// SpecialBase - Common Special Functionality
//========================================

UCameraCollision_SpecialBase::UCameraCollision_SpecialBase()
    : BaseProbeDistance(150.0f)
    , TransitionSpeed(5.0f)
    , ActivationThreshold(0.1f)
    , CurrentSpecialFactor(0.0f)
    , bSpecialModeActive(false)
    , TimeInSpecialMode(0.0f)
    , CachedDeltaTime(0.0f)
{
    // Special strategies are enabled by default
    bIsEnabled = true;
    bCanBlend = true;  // Special strategies can blend
}

void UCameraCollision_SpecialBase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
    
    // Cache delta time for use in Execute
    CachedDeltaTime = DeltaTime;
    
    // Update time in special mode
    if (bSpecialModeActive)
    {
        TimeInSpecialMode += DeltaTime;
    }
    else
    {
        TimeInSpecialMode = 0.0f;
    }
}

void UCameraCollision_SpecialBase::Reset()
{
    Super::Reset();
    
    CurrentSpecialFactor = 0.0f;
    bSpecialModeActive = false;
    TimeInSpecialMode = 0.0f;
    CachedDeltaTime = 0.0f;
}

int32 UCameraCollision_SpecialBase::PerformEnvironmentProbe(
    const FStageExecutionContext& Context,
    float ProbeDistance,
    TArray<FHitResult>& OutHits) const
{
    OutHits.Empty();
    
    FVector CameraLocation = GetCameraLocation(Context);
    
    // Define probe directions (6 cardinal + 8 diagonal = 14 total)
    TArray<FVector> ProbeDirections;
    
    // Cardinal directions
    ProbeDirections.Add(FVector::ForwardVector);
    ProbeDirections.Add(FVector::BackwardVector);
    ProbeDirections.Add(FVector::RightVector);
    ProbeDirections.Add(FVector::LeftVector);
    ProbeDirections.Add(FVector::UpVector);
    ProbeDirections.Add(FVector::DownVector);
    
    // Diagonal directions (horizontal)
    ProbeDirections.Add((FVector::ForwardVector + FVector::RightVector).GetSafeNormal());
    ProbeDirections.Add((FVector::ForwardVector + FVector::LeftVector).GetSafeNormal());
    ProbeDirections.Add((FVector::BackwardVector + FVector::RightVector).GetSafeNormal());
    ProbeDirections.Add((FVector::BackwardVector + FVector::LeftVector).GetSafeNormal());
    
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    int32 BlockedCount = 0;
    
    for (const FVector& Direction : ProbeDirections)
    {
        FHitResult Hit;
        if (IsDirectionBlocked(Context, Direction, ProbeDistance, Hit))
        {
            OutHits.Add(Hit);
            BlockedCount++;
        }
    }
    
    return BlockedCount;
}

bool UCameraCollision_SpecialBase::IsDirectionBlocked(
    const FStageExecutionContext& Context,
    const FVector& Direction,
    float Distance,
    FHitResult& OutHit) const
{
    FVector CameraLocation = GetCameraLocation(Context);
    FVector EndLocation = CameraLocation + Direction * Distance;
    
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    return LineTrace(CameraLocation, EndLocation, OutHit, Params);
}

float UCameraCollision_SpecialBase::GetClearanceInDirection(
    const FStageExecutionContext& Context,
    const FVector& Direction,
    float MaxDistance) const
{
    FHitResult Hit;
    if (IsDirectionBlocked(Context, Direction, MaxDistance, Hit))
    {
        return Hit.Distance;
    }
    return MaxDistance;
}

FVector UCameraCollision_SpecialBase::GetCameraLocation(const FStageExecutionContext& Context) const
{
    return Context.Output.FocusPoint - 
           Context.Output.Rotation.Vector() * Context.Output.Distance;
}

float UCameraCollision_SpecialBase::InterpolateSpecialFactor(
    float Current,
    float Target,
    float DeltaTime,
    float Speed) const
{
    return FMath::FInterpTo(Current, Target, DeltaTime, Speed);
}


//========================================
// SP01: Tight Space Implementation
//========================================

/**
 * Constructor - Configure tight space parameters
 * 
 * Tight space detection uses multi-directional probing:
 * - Checks multiple directions around camera
 * - If enough directions are blocked, considers it tight
 * - Applies FOV compensation and distance adjustment
 */
UCameraCollision_SP01_TightSpace::UCameraCollision_SP01_TightSpace()
    : TightSpaceThreshold(100.0f)
    , BlockedDirectionsThreshold(3)
    , TightSpaceFOVBonus(15.0f)
    , TightSpaceMinDistance(40.0f)
    , bInTightSpace(false)
    , TightSpaceFactor(0.0f)
    , CurrentBlockedDirections(0)
{
    Priority = 150;
    BaseProbeDistance = 100.0f;
    TransitionSpeed = 4.0f;
}

bool UCameraCollision_SP01_TightSpace::ShouldExecute(const FStageExecutionContext& Context) const
{
    // Always execute to handle smooth transitions
    return bIsEnabled;
}

bool UCameraCollision_SP01_TightSpace::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Detect tight space
    bool bCurrentlyTight = DetectTightSpace(Context);
    
    // Smooth transition
    float TargetFactor = bCurrentlyTight ? 1.0f : 0.0f;
    TightSpaceFactor = InterpolateSpecialFactor(TightSpaceFactor, TargetFactor, CachedDeltaTime, TransitionSpeed);
    
    // Update active state
    bInTightSpace = (TightSpaceFactor > ActivationThreshold);
    bSpecialModeActive = bInTightSpace;
    CurrentSpecialFactor = TightSpaceFactor;
    
    if (!bInTightSpace)
    {
        SetActive(false);
        return false;
    }
    
    SetActive(true);
    
    // Build response with tight space adjustments
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    
    // Increase FOV to maintain visibility in tight space
    OutResponse.FOVAdjustment = TightSpaceFOVBonus * TightSpaceFactor;
    
    // Allow closer minimum distance in tight space
    // This is informational - the collision system will use this
    // No direct distance adjustment here, but we signal the factor
    
    OutResponse.Weight = TightSpaceFactor;
    OutResponse.Priority = GetPriority();
    OutResponse.bIsActive = true;
    OutResponse.ActiveDuration = TimeInSpecialMode;
    
    UE_LOG(LogTemp, Verbose, TEXT("SP01 TightSpace: Factor=%.2f, BlockedDirs=%d, FOV+=%.1f"),
        TightSpaceFactor, CurrentBlockedDirections, OutResponse.FOVAdjustment);
    
    return true;
}

void UCameraCollision_SP01_TightSpace::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_SP01_TightSpace::Reset()
{
    Super::Reset();
    
    bInTightSpace = false;
    TightSpaceFactor = 0.0f;
    CurrentBlockedDirections = 0;
}

bool UCameraCollision_SP01_TightSpace::DetectTightSpace(const FStageExecutionContext& Context)
{
    FVector CameraLocation = GetCameraLocation(Context);
    
    // Define horizontal probe directions (ignore vertical for tight space)
    TArray<FVector> ProbeDirections;
    ProbeDirections.Add(FVector::ForwardVector);
    ProbeDirections.Add(FVector::BackwardVector);
    ProbeDirections.Add(FVector::RightVector);
    ProbeDirections.Add(FVector::LeftVector);
    ProbeDirections.Add((FVector::ForwardVector + FVector::RightVector).GetSafeNormal());
    ProbeDirections.Add((FVector::ForwardVector + FVector::LeftVector).GetSafeNormal());
    ProbeDirections.Add((FVector::BackwardVector + FVector::RightVector).GetSafeNormal());
    ProbeDirections.Add((FVector::BackwardVector + FVector::LeftVector).GetSafeNormal());
    
    CurrentBlockedDirections = 0;
    
    for (const FVector& Direction : ProbeDirections)
    {
        float Clearance = GetClearanceInDirection(Context, Direction, TightSpaceThreshold);
        if (Clearance < TightSpaceThreshold)
        {
            CurrentBlockedDirections++;
        }
    }
    
    // Consider tight space if enough directions are blocked
    return CurrentBlockedDirections >= BlockedDirectionsThreshold;
}


//========================================
// SP02: Low Ceiling Implementation
//========================================

/**
 * Constructor - Configure low ceiling parameters
 * 
 * Low ceiling detection checks overhead clearance:
 * - Traces upward from camera position
 * - If clearance is below threshold, activates handling
 * - Applies pitch adjustment and optional downward push
 */
UCameraCollision_SP02_LowCeiling::UCameraCollision_SP02_LowCeiling()
    : LowCeilingThreshold(150.0f)
    , MaxPitchAdjustment(20.0f)
    , bPushCameraDown(true)
    , MaxDownwardPush(50.0f)
    , CurrentOverheadClearance(1000.0f)
    , LowCeilingFactor(0.0f)
    , CurrentPitchAdjustment(0.0f)
{
    Priority = 145;
    BaseProbeDistance = 200.0f;
    TransitionSpeed = 5.0f;
}

bool UCameraCollision_SP02_LowCeiling::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_SP02_LowCeiling::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check overhead clearance
    CurrentOverheadClearance = GetClearanceInDirection(Context, FVector::UpVector, BaseProbeDistance);
    
    // Determine if ceiling is low
    bool bLowCeiling = (CurrentOverheadClearance < LowCeilingThreshold);
    
    // Calculate factor based on how low the ceiling is
    float TargetFactor = 0.0f;
    if (bLowCeiling)
    {
        // Factor increases as ceiling gets lower
        TargetFactor = 1.0f - (CurrentOverheadClearance / LowCeilingThreshold);
        TargetFactor = FMath::Clamp(TargetFactor, 0.0f, 1.0f);
    }
    
    // Smooth transition
    LowCeilingFactor = InterpolateSpecialFactor(LowCeilingFactor, TargetFactor, CachedDeltaTime, TransitionSpeed);
    
    // Update active state
    bSpecialModeActive = (LowCeilingFactor > ActivationThreshold);
    CurrentSpecialFactor = LowCeilingFactor;
    
    if (!bSpecialModeActive)
    {
        CurrentPitchAdjustment = 0.0f;
        SetActive(false);
        return false;
    }
    
    SetActive(true);
    
    // Build response with low ceiling adjustments
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    
    // Apply pitch adjustment - tilt camera down when ceiling is low
    float TargetPitch = -MaxPitchAdjustment * LowCeilingFactor;  // Negative = look down
    CurrentPitchAdjustment = FMath::FInterpTo(CurrentPitchAdjustment, TargetPitch, CachedDeltaTime, TransitionSpeed);
    OutResponse.RotationAdjustment.Pitch = CurrentPitchAdjustment;
    
    // Apply downward push if enabled
    if (bPushCameraDown)
    {
        float PushAmount = MaxDownwardPush * LowCeilingFactor;
        OutResponse.PositionAdjustment = FVector(0.0f, 0.0f, -PushAmount);
    }
    
    OutResponse.Weight = LowCeilingFactor;
    OutResponse.Priority = GetPriority();
    OutResponse.bIsActive = true;
    OutResponse.ActiveDuration = TimeInSpecialMode;
    
    UE_LOG(LogTemp, Verbose, TEXT("SP02 LowCeiling: Clearance=%.1f, Factor=%.2f, Pitch=%.1f"),
        CurrentOverheadClearance, LowCeilingFactor, CurrentPitchAdjustment);
    
    return true;
}

void UCameraCollision_SP02_LowCeiling::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_SP02_LowCeiling::Reset()
{
    Super::Reset();
    
    CurrentOverheadClearance = 1000.0f;
    LowCeilingFactor = 0.0f;
    CurrentPitchAdjustment = 0.0f;
}


//========================================
// SP03: Cliff Edge Implementation
//========================================

/**
 * Constructor - Configure cliff edge parameters
 * 
 * Cliff edge detection checks for drop-offs:
 * - Traces downward from camera and character positions
 * - Compares ground heights to detect edges
 * - Applies pullback and pitch adjustment at edges
 */
UCameraCollision_SP03_CliffEdge::UCameraCollision_SP03_CliffEdge()
    : GroundCheckDistance(500.0f)
    , CliffHeightThreshold(200.0f)
    , EdgePullbackDistance(50.0f)
    , EdgePitchAdjustment(10.0f)
    , bNearEdge(false)
    , EdgeFactor(0.0f)
    , GroundHeightAtCamera(0.0f)
    , GroundHeightAtCharacter(0.0f)
{
    Priority = 140;
    BaseProbeDistance = 300.0f;
    TransitionSpeed = 4.0f;
}

bool UCameraCollision_SP03_CliffEdge::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_SP03_CliffEdge::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Detect cliff edge
    bool bCurrentlyNearEdge = DetectCliffEdge(Context);
    
    // Smooth transition
    float TargetFactor = bCurrentlyNearEdge ? 1.0f : 0.0f;
    EdgeFactor = InterpolateSpecialFactor(EdgeFactor, TargetFactor, CachedDeltaTime, TransitionSpeed);
    
    // Update active state
    bNearEdge = (EdgeFactor > ActivationThreshold);
    bSpecialModeActive = bNearEdge;
    CurrentSpecialFactor = EdgeFactor;
    
    if (!bNearEdge)
    {
        SetActive(false);
        return false;
    }
    
    SetActive(true);
    
    // Build response with cliff edge adjustments
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    
    // Pull camera back from edge
    FVector CameraLocation = GetCameraLocation(Context);
    FVector CharacterLocation = Context.InputContext.CharacterLocation;
    FVector ToCharacter = (CharacterLocation - CameraLocation).GetSafeNormal2D();
    
    // Position adjustment: pull towards character (away from edge)
    float PullbackAmount = EdgePullbackDistance * EdgeFactor;
    OutResponse.PositionAdjustment = ToCharacter * PullbackAmount;
    
    // Slight pitch up when at edge (to see more of the drop)
    OutResponse.RotationAdjustment.Pitch = EdgePitchAdjustment * EdgeFactor;
    
    OutResponse.Weight = EdgeFactor;
    OutResponse.Priority = GetPriority();
    OutResponse.bIsActive = true;
    OutResponse.ActiveDuration = TimeInSpecialMode;
    
    UE_LOG(LogTemp, Verbose, TEXT("SP03 CliffEdge: Factor=%.2f, HeightDiff=%.1f, Pullback=%.1f"),
        EdgeFactor, GroundHeightAtCharacter - GroundHeightAtCamera, PullbackAmount);
    
    return true;
}

void UCameraCollision_SP03_CliffEdge::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_SP03_CliffEdge::Reset()
{
    Super::Reset();
    
    bNearEdge = false;
    EdgeFactor = 0.0f;
    GroundHeightAtCamera = 0.0f;
    GroundHeightAtCharacter = 0.0f;
}

bool UCameraCollision_SP03_CliffEdge::DetectCliffEdge(const FStageExecutionContext& Context)
{
    FVector CameraLocation = GetCameraLocation(Context);
    FVector CharacterLocation = Context.InputContext.CharacterLocation;
    
    // Get ground height at both positions
    GroundHeightAtCamera = GetGroundHeight(CameraLocation);
    GroundHeightAtCharacter = GetGroundHeight(CharacterLocation);
    
    // Check if camera is over a significant drop
    float HeightDifference = GroundHeightAtCharacter - GroundHeightAtCamera;
    
    // If ground under camera is much lower than ground under character, we're near an edge
    return HeightDifference > CliffHeightThreshold;
}

float UCameraCollision_SP03_CliffEdge::GetGroundHeight(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return Location.Z - 1000.0f;  // Return very low if no world
    }
    
    FVector Start = Location;
    FVector End = Location - FVector(0.0f, 0.0f, GroundCheckDistance);
    
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    FHitResult Hit;
    if (World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params))
    {
        return Hit.ImpactPoint.Z;
    }
    
    // No ground found - treat as very low (cliff)
    return Location.Z - GroundCheckDistance;
}

//========================================
// SP04: Corner Case Implementation
//========================================

/**
 * Constructor - Configure corner case parameters
 * 
 * Corner detection checks for L-shaped geometry:
 * - Probes multiple directions to find walls
 * - Detects when two walls form a corner
 * - Calculates escape direction and applies adjustment
 */
UCameraCollision_SP04_CornerCase::UCameraCollision_SP04_CornerCase()
    : CornerAngleThreshold(90.0f)
    , CornerCheckDistance(100.0f)
    , MaxCornerEscapeDistance(50.0f)
    , MaxCornerYawAdjustment(30.0f)
    , bInCorner(false)
    , CornerFactor(0.0f)
    , CornerEscapeDirection(FVector::ZeroVector)
    , WallNormal1(FVector::ZeroVector)
    , WallNormal2(FVector::ZeroVector)
{
    Priority = 155;  // Highest among special strategies
    BaseProbeDistance = 100.0f;
    TransitionSpeed = 6.0f;
}

bool UCameraCollision_SP04_CornerCase::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_SP04_CornerCase::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Detect corner
    bool bCurrentlyInCorner = DetectCorner(Context);
    
    // Smooth transition
    float TargetFactor = bCurrentlyInCorner ? 1.0f : 0.0f;
    CornerFactor = InterpolateSpecialFactor(CornerFactor, TargetFactor, CachedDeltaTime, TransitionSpeed);
    
    // Update active state
    bInCorner = (CornerFactor > ActivationThreshold);
    bSpecialModeActive = bInCorner;
    CurrentSpecialFactor = CornerFactor;
    
    if (!bInCorner)
    {
        SetActive(false);
        return false;
    }
    
    SetActive(true);
    
    // Calculate escape direction
    FVector EscapeDir = CalculateEscapeDirection();
    
    // Build response with corner adjustments
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    
    // Position adjustment: move towards escape direction
    float EscapeAmount = MaxCornerEscapeDistance * CornerFactor;
    OutResponse.PositionAdjustment = EscapeDir * EscapeAmount;
    
    // Calculate yaw adjustment to look away from corner
    // Find angle between current camera direction and escape direction
    FVector CameraForward = Context.Output.Rotation.Vector();
    CameraForward.Z = 0.0f;
    CameraForward.Normalize();
    
    FVector EscapeDirHorizontal = EscapeDir;
    EscapeDirHorizontal.Z = 0.0f;
    EscapeDirHorizontal.Normalize();
    
    float DotProduct = FVector::DotProduct(CameraForward, EscapeDirHorizontal);
    FVector CrossProduct = FVector::CrossProduct(CameraForward, EscapeDirHorizontal);
    float YawNeeded = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
    
    // Determine sign based on cross product
    if (CrossProduct.Z < 0.0f)
    {
        YawNeeded = -YawNeeded;
    }
    
    // Clamp yaw adjustment
    float ClampedYaw = FMath::Clamp(YawNeeded * CornerFactor, -MaxCornerYawAdjustment, MaxCornerYawAdjustment);
    OutResponse.RotationAdjustment.Yaw = ClampedYaw;
    
    OutResponse.Weight = CornerFactor;
    OutResponse.Priority = GetPriority();
    OutResponse.bIsActive = true;
    OutResponse.ActiveDuration = TimeInSpecialMode;
    OutResponse.bOverride = false;
    
    UE_LOG(LogTemp, Verbose, TEXT("SP04 CornerCase: Factor=%.2f, EscapeDir=(%.1f,%.1f), Yaw=%.1f"),
        CornerFactor, EscapeDir.X, EscapeDir.Y, ClampedYaw);
    
    return true;
}

void UCameraCollision_SP04_CornerCase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_SP04_CornerCase::Reset()
{
    Super::Reset();
    
    bInCorner = false;
    CornerFactor = 0.0f;
    CornerEscapeDirection = FVector::ZeroVector;
    WallNormal1 = FVector::ZeroVector;
    WallNormal2 = FVector::ZeroVector;
}

bool UCameraCollision_SP04_CornerCase::DetectCorner(const FStageExecutionContext& Context)
{
    FVector CameraLocation = GetCameraLocation(Context);
    
    // Probe in 8 horizontal directions
    TArray<FVector> ProbeDirections;
    for (int32 i = 0; i < 8; ++i)
    {
        float Angle = (360.0f / 8.0f) * i;
        float Rad = FMath::DegreesToRadians(Angle);
        ProbeDirections.Add(FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0.0f));
    }
    
    // Find walls
    TArray<FVector> WallNormals;
    TArray<float> WallDistances;
    
    for (const FVector& Direction : ProbeDirections)
    {
        FHitResult Hit;
        if (IsDirectionBlocked(Context, Direction, CornerCheckDistance, Hit))
        {
            WallNormals.Add(Hit.ImpactNormal);
            WallDistances.Add(Hit.Distance);
        }
    }
    
    // Need at least 2 walls to form a corner
    if (WallNormals.Num() < 2)
    {
        WallNormal1 = FVector::ZeroVector;
        WallNormal2 = FVector::ZeroVector;
        return false;
    }
    
    // Find two walls that form an approximate 90 degree angle
    for (int32 i = 0; i < WallNormals.Num(); ++i)
    {
        for (int32 j = i + 1; j < WallNormals.Num(); ++j)
        {
            float Dot = FVector::DotProduct(WallNormals[i], WallNormals[j]);
            float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));
            
            // Check if angle is close to 90 degrees (within threshold)
            float AngleDiff = FMath::Abs(AngleDeg - 90.0f);
            if (AngleDiff < (180.0f - CornerAngleThreshold) / 2.0f)
            {
                WallNormal1 = WallNormals[i];
                WallNormal2 = WallNormals[j];
                return true;
            }
        }
    }
    
    return false;
}

FVector UCameraCollision_SP04_CornerCase::CalculateEscapeDirection() const
{
    if (WallNormal1.IsNearlyZero() || WallNormal2.IsNearlyZero())
    {
        return FVector::ForwardVector;
    }
    
    // Escape direction is the sum of both wall normals (bisector pointing out of corner)
    FVector Escape = (WallNormal1 + WallNormal2).GetSafeNormal();
    
    // Ensure horizontal
    Escape.Z = 0.0f;
    
    if (Escape.IsNearlyZero())
    {
        // Walls are opposite - pick perpendicular to first wall
        Escape = FVector::CrossProduct(WallNormal1, FVector::UpVector).GetSafeNormal();
    }
    else
    {
        Escape.Normalize();
    }
    
    // Store for debug visualization (mutable member allows modification in const method)
    CornerEscapeDirection = Escape;
    return Escape;
}
