// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Collision/CameraCollision_Response.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

/**
 * CameraCollision_Response.cpp
 * 
 * This file contains the implementations of the Response base class
 * and all 5 Response strategies (RS01-RS05).
 * 
 * Response strategies determine how the camera adjusts when collisions
 * are detected. They read FCollisionDetectionResult and produce
 * FCollisionResponseOutput with camera adjustments.
 * 
 * Implementation Structure:
 * - ResponseBase: Common response functionality
 * - RS01: PullIn - Simple distance reduction
 * - RS02: Slide - Lateral movement along surface
 * - RS03: Orbit - Rotation to find clear view
 * - RS04: FOVCompensate - FOV adjustment to maintain framing
 * - RS05: InstantSnap - Immediate snap to safe position
 */


//========================================
// ResponseBase - Common Response Functionality
//========================================

UCameraCollision_ResponseBase::UCameraCollision_ResponseBase()
    : BaseInterpSpeed(10.0f)
    , ResponseBlendTime(0.2f)
    , bOverrideOthers(false)
    , ResponseTime(0.0f)
    , bResponseActive(false)
    , CachedDeltaTime(0.0f)
{
    // Response strategies can blend by default
    bCanBlend = true;
    bIsEnabled = true;
}

void UCameraCollision_ResponseBase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
    
    // Cache delta time for interpolation in Execute
    CachedDeltaTime = DeltaTime;
    
    // Update response time if active
    if (bResponseActive)
    {
        ResponseTime += DeltaTime;
    }
    else
    {
        ResponseTime = 0.0f;
    }
}

void UCameraCollision_ResponseBase::Reset()
{
    Super::Reset();
    
    ResponseTime = 0.0f;
    bResponseActive = false;
    CachedDeltaTime = 0.0f;
}

float UCameraCollision_ResponseBase::InterpolateValue(float Current, float Target, float DeltaTime, float Speed) const
{
    if (FMath::IsNearlyEqual(Current, Target, 0.01f))
    {
        return Target;
    }
    
    return FMath::FInterpTo(Current, Target, DeltaTime, Speed);
}

FVector UCameraCollision_ResponseBase::InterpolateVector(const FVector& Current, const FVector& Target, float DeltaTime, float Speed) const
{
    if (Current.Equals(Target, 0.1f))
    {
        return Target;
    }
    
    return FMath::VInterpTo(Current, Target, DeltaTime, Speed);
}

FRotator UCameraCollision_ResponseBase::InterpolateRotator(const FRotator& Current, const FRotator& Target, float DeltaTime, float Speed) const
{
    if (Current.Equals(Target, 0.1f))
    {
        return Target;
    }
    
    return FMath::RInterpTo(Current, Target, DeltaTime, Speed);
}

bool UCameraCollision_ResponseBase::IsResponseNeeded(const FCollisionDetectionResult& DetectionResult) const
{
    // Response is needed if collision was detected and adjustment is significant
    return DetectionResult.bCollisionDetected && DetectionResult.AdjustmentRatio > 0.01f;
}

void UCameraCollision_ResponseBase::InitializeResponse(FCollisionResponseOutput& OutResponse) const
{
    OutResponse.Reset();
    OutResponse.StrategyType = GetStrategyType();
    OutResponse.Category = GetStrategyCategory();
    OutResponse.Priority = GetPriority();
    OutResponse.Weight = BlendWeight;
    OutResponse.bOverride = bOverrideOthers;
    OutResponse.BlendTime = ResponseBlendTime;
    OutResponse.bIsActive = true;
}


//========================================
// RS01: Pull In Implementation
//========================================

/**
 * Constructor - Configure pull-in response parameters
 * 
 * Pull-in is the simplest and most common collision response:
 * - Reduces camera distance to avoid obstacle
 * - Maintains camera angle
 * - Simple and reliable
 */
UCameraCollision_RS01_PullIn::UCameraCollision_RS01_PullIn()
    : PullInSpeed(300.0f)
    , MinPullInTime(0.1f)
    , bUseOvershoot(false)
    , OvershootAmount(0.05f)
    , CurrentDistanceAdjustment(0.0f)
    , TargetDistanceAdjustment(0.0f)
    , bIsAdjusting(false)
{
    // Default priority for basic pull-in
    Priority = 100;
    BaseInterpSpeed = 15.0f;
    
    // Acceleration mode defaults (新增)
    bUseAcceleration = true;
    PullInAcceleration = 500.0f;
    MaxPullInSpeed = 500.0f;
    CurrentPullSpeed = 0.0f;
}

bool UCameraCollision_RS01_PullIn::ShouldExecute(const FStageExecutionContext& Context) const
{
    // Execute when enabled
    return bIsEnabled;
}

bool UCameraCollision_RS01_PullIn::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // ★★★ 方案C：基础拉近由 SpringArm 原生碰撞处理 ★★★
    // RS01 PullIn 现在只用于记录状态，不实际调整距离
    // 距离调整由 UE4 SpringArm 的 bDoCollisionTest 处理
    
    static bool bLoggedOnce = false;
    if (!bLoggedOnce)
    {
        UE_LOG(LogTemp, Log, TEXT("RS01_PullIn: Hybrid Mode C - Distance adjustment handled by native SpringArm"));
        bLoggedOnce = true;
    }
    
    // 仍然跟踪碰撞状态，但不产生距离调整
    if (!IsResponseNeeded(DetectionResult))
    {
        bResponseActive = false;
        bIsAdjusting = false;
        CurrentPullSpeed = 0.0f;
        SetActive(false);
        return false;
    }
    
    // 标记为活跃状态（用于调试和状态跟踪）
    bResponseActive = true;
    bIsAdjusting = true;
    SetActive(true);
    
    // 不产生距离调整 - 让 SpringArm 处理
    // 但仍然返回一个空响应以表示我们在跟踪碰撞
    InitializeResponse(OutResponse);
    OutResponse.DistanceAdjustment = 0.0f;  // ★ 关键：不调整距离
    OutResponse.ActiveDuration = ResponseTime;
    
    // 记录但不应用
    float DesiredDistance = DetectionResult.DesiredDistance;
    float SafeDistance = DetectionResult.SafeDistance;
    
#if WITH_EDITOR
    // 从 Outer 链获取 CameraManager 检查调试开关
    bool bShouldLogResponse = false;
    UObject* CurrentOuter = GetOuter();
    while (CurrentOuter)
    {
        if (USoulsCameraManager* Manager = Cast<USoulsCameraManager>(CurrentOuter))
        {
            bShouldLogResponse = Manager->IsCollisionDebugEnabled();
            break;
        }
        CurrentOuter = CurrentOuter->GetOuter();
    }
    
    if (bShouldLogResponse)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PullIn响应] DesiredDist: %.1f, SafeDist: %.1f (SpringArm handles adjustment)"),
            DesiredDistance, SafeDistance);
    }
#endif
    
    return true;  // 返回 true 表示我们处理了，但 DistanceAdjustment = 0
}

void UCameraCollision_RS01_PullIn::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
    
    // Additional update logic if needed
}

void UCameraCollision_RS01_PullIn::Reset()
{
    Super::Reset();
    
    CurrentDistanceAdjustment = 0.0f;
    TargetDistanceAdjustment = 0.0f;
    bIsAdjusting = false;
    CurrentPullSpeed = 0.0f;
}

void UCameraCollision_RS01_PullIn::ResetPullState()
{
    CurrentPullSpeed = 0.0f;
}


//========================================
// RS02: Slide Implementation
//========================================

/**
 * Constructor - Configure slide response parameters
 * 
 * Slide response moves camera laterally to avoid obstacles:
 * - Maintains distance from target
 * - Uses collision normal to determine slide direction
 * - Validates new position to avoid secondary collisions
 */
UCameraCollision_RS02_Slide::UCameraCollision_RS02_Slide()
    : MaxSlideDistance(80.0f)
    , SlideSpeed(200.0f)
    , bPreferHorizontalSlide(true)
    , bVerifySlideDestination(true)
    , CurrentSlideOffset(FVector::ZeroVector)
    , TargetSlideOffset(FVector::ZeroVector)
    , SlideDirection(FVector::ZeroVector)
{
    Priority = 110;
    BaseInterpSpeed = 12.0f;
}

bool UCameraCollision_RS02_Slide::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_RS02_Slide::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if response is needed
    if (!IsResponseNeeded(DetectionResult))
    {
        // No collision - smoothly return slide to zero
        if (!CurrentSlideOffset.IsNearlyZero(0.5f))
        {
            CurrentSlideOffset = InterpolateVector(
                CurrentSlideOffset,
                FVector::ZeroVector,
                CachedDeltaTime,
                BaseInterpSpeed * 0.7f  // Slower return for stability
            );
            
            // Still output the returning slide
            InitializeResponse(OutResponse);
            OutResponse.PositionAdjustment = CurrentSlideOffset;
            OutResponse.ActiveDuration = ResponseTime;
            
            bResponseActive = true;
            SetActive(true);
            return true;
        }
        
        bResponseActive = false;
        SetActive(false);
        return false;
    }

    bResponseActive = true;
    SetActive(true);

    // Get collision info
    FVector CollisionNormal = DetectionResult.CollisionNormal;
    FVector CameraDirection = -Context.Output.Rotation.Vector();
    
    // Calculate slide direction (perpendicular to collision normal)
    SlideDirection = CalculateSlideDirection(CollisionNormal, CameraDirection);
    
    // If prefer horizontal, flatten the slide direction
    if (bPreferHorizontalSlide)
    {
        SlideDirection.Z *= 0.3f;  // Reduce vertical component
        SlideDirection.Normalize();
        
        if (SlideDirection.IsNearlyZero())
        {
            // Fallback to pure horizontal
            FVector RightVector = FVector::CrossProduct(FVector::UpVector, CameraDirection).GetSafeNormal();
            SlideDirection = RightVector;
        }
    }
    
    // Calculate target slide distance based on adjustment ratio
    float AdjustmentNeeded = DetectionResult.DesiredDistance - DetectionResult.SafeDistance;
    float SlideAmount = FMath::Clamp(AdjustmentNeeded * 0.6f, 0.0f, MaxSlideDistance);
    
    // Determine slide direction (try to maintain consistency)
    FVector NewTargetSlide = SlideDirection * SlideAmount;
    
    // If we're already sliding, prefer to continue in same direction
    if (!CurrentSlideOffset.IsNearlyZero())
    {
        FVector CurrentDir = CurrentSlideOffset.GetSafeNormal();
        FVector NewDir = NewTargetSlide.GetSafeNormal();
        float DotResult = FVector::DotProduct(CurrentDir, NewDir);
        
        if (DotResult < 0.0f)
        {
            // Would reverse direction - check if current direction still works
            FVector ContinueSlide = CurrentSlideOffset.GetSafeNormal() * SlideAmount;
            if (IsSlideValid(Context, DetectionResult, ContinueSlide))
            {
                NewTargetSlide = ContinueSlide;
            }
        }
    }
    
    // Verify slide destination if enabled
    if (bVerifySlideDestination)
    {
        if (!IsSlideValid(Context, DetectionResult, NewTargetSlide))
        {
            // Try opposite direction
            NewTargetSlide = -NewTargetSlide;
            
            if (!IsSlideValid(Context, DetectionResult, NewTargetSlide))
            {
                // Both blocked - reduce slide amount
                NewTargetSlide = SlideDirection * (SlideAmount * 0.3f);
            }
        }
    }
    
    TargetSlideOffset = NewTargetSlide;
    
    // Calculate interpolation speed based on urgency
    float Urgency = DetectionResult.AdjustmentRatio;
    float EffectiveSpeed = BaseInterpSpeed * (1.0f + Urgency);
    
    // Interpolate current slide towards target
    CurrentSlideOffset = InterpolateVector(
        CurrentSlideOffset,
        TargetSlideOffset,
        CachedDeltaTime,
        EffectiveSpeed
    );

    // Build response output
    InitializeResponse(OutResponse);
    OutResponse.PositionAdjustment = CurrentSlideOffset;
    OutResponse.ActiveDuration = ResponseTime;

    UE_LOG(LogTemp, Verbose, TEXT("RS02 Slide: Offset=(%.1f, %.1f, %.1f), target=(%.1f, %.1f, %.1f)"),
        CurrentSlideOffset.X, CurrentSlideOffset.Y, CurrentSlideOffset.Z,
        TargetSlideOffset.X, TargetSlideOffset.Y, TargetSlideOffset.Z);

    return true;
}

void UCameraCollision_RS02_Slide::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_RS02_Slide::Reset()
{
    Super::Reset();
    CurrentSlideOffset = FVector::ZeroVector;
    TargetSlideOffset = FVector::ZeroVector;
    SlideDirection = FVector::ZeroVector;
}

/**
 * Calculate optimal slide direction from collision
 */
FVector UCameraCollision_RS02_Slide::CalculateSlideDirection(
    const FVector& CollisionNormal,
    const FVector& CameraDirection) const
{
    // Project camera direction onto collision plane
    FVector SlideDir = FVector::VectorPlaneProject(CameraDirection, CollisionNormal);
    
    if (SlideDir.IsNearlyZero())
    {
        // Camera direction is perpendicular to surface - use arbitrary direction
        FVector Right = FVector::CrossProduct(CollisionNormal, FVector::UpVector);
        if (Right.IsNearlyZero())
        {
            Right = FVector::CrossProduct(CollisionNormal, FVector::ForwardVector);
        }
        SlideDir = Right.GetSafeNormal();
    }
    else
    {
        SlideDir.Normalize();
    }
    
    return SlideDir;
}

/**
 * Verify that slide position doesn't cause new collision
 */
bool UCameraCollision_RS02_Slide::IsSlideValid(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    const FVector& SlideOffset) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return true;  // Assume valid if can't check
    }
    
    FVector FocusPoint = Context.Output.FocusPoint;
    FVector TestPosition = DetectionResult.SafeLocation + SlideOffset;
    
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    FHitResult Hit;
    bool bBlocked = World->LineTraceSingleByChannel(
        Hit,
        FocusPoint,
        TestPosition,
        TraceChannel,
        Params
    );
    
    return !bBlocked;
}


/**
 * Constructor - Configure orbit response parameters
 * 
 * Orbit response adjusts camera angle to find clear view:
 * - Tests multiple angles to find best rotation
 * - Prefers yaw over pitch for natural feel
 * - Limited rotation to prevent disorientation
 */
UCameraCollision_RS03_Orbit::UCameraCollision_RS03_Orbit()
    : MaxOrbitAngle(20.0f)
    , OrbitSpeed(90.0f)
    , NumTestAngles(8)
    , bPreferYawOrbit(true)
    , MaxPitchAdjustment(10.0f)
    , CurrentOrbitAdjustment(FRotator::ZeroRotator)
    , TargetOrbitAdjustment(FRotator::ZeroRotator)
    , BestFoundOrbit(FRotator::ZeroRotator)
{
    Priority = 105;
    BaseInterpSpeed = 8.0f;  // Slower for rotation to feel natural
}

bool UCameraCollision_RS03_Orbit::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_RS03_Orbit::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if response is needed
    if (!IsResponseNeeded(DetectionResult))
    {
        // No collision - smoothly return rotation to zero
        if (!CurrentOrbitAdjustment.IsNearlyZero(0.5f))
        {
            CurrentOrbitAdjustment = InterpolateRotator(
                CurrentOrbitAdjustment,
                FRotator::ZeroRotator,
                CachedDeltaTime,
                BaseInterpSpeed * 0.5f  // Slow return to avoid snap
            );
            
            // Still output the returning rotation
            InitializeResponse(OutResponse);
            OutResponse.RotationAdjustment = CurrentOrbitAdjustment;
            OutResponse.ActiveDuration = ResponseTime;
            
            bResponseActive = true;
            SetActive(true);
            return true;
        }
        
        bResponseActive = false;
        SetActive(false);
        return false;
    }

    bResponseActive = true;
    SetActive(true);

    // Find best rotation to avoid collision
    BestFoundOrbit = FindBestOrbit(Context, DetectionResult);
    
    // If no good rotation found, don't apply rotation response
    if (BestFoundOrbit.IsNearlyZero())
    {
        // Still interpolate current rotation back towards zero
        TargetOrbitAdjustment = FRotator::ZeroRotator;
    }
    else
    {
        TargetOrbitAdjustment = BestFoundOrbit;
    }
    
    // Calculate interpolation speed based on urgency
    float Urgency = DetectionResult.AdjustmentRatio;
    float EffectiveSpeed = BaseInterpSpeed * (1.0f + Urgency * 0.5f);
    
    // Interpolate current rotation towards target
    CurrentOrbitAdjustment = InterpolateRotator(
        CurrentOrbitAdjustment,
        TargetOrbitAdjustment,
        CachedDeltaTime,
        EffectiveSpeed
    );

    // Build response output
    InitializeResponse(OutResponse);
    OutResponse.RotationAdjustment = CurrentOrbitAdjustment;
    OutResponse.ActiveDuration = ResponseTime;

    UE_LOG(LogTemp, Verbose, TEXT("RS03 Orbit: Adjustment=(P:%.1f, Y:%.1f, R:%.1f)"),
        CurrentOrbitAdjustment.Pitch,
        CurrentOrbitAdjustment.Yaw,
        CurrentOrbitAdjustment.Roll);

    return true;
}

void UCameraCollision_RS03_Orbit::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_RS03_Orbit::Reset()
{
    Super::Reset();
    CurrentOrbitAdjustment = FRotator::ZeroRotator;
    TargetOrbitAdjustment = FRotator::ZeroRotator;
    BestFoundOrbit = FRotator::ZeroRotator;
}

FRotator UCameraCollision_RS03_Orbit::FindBestOrbit(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult)
{
    FRotator BestRotation = FRotator::ZeroRotator;
    float BestScore = 0.0f;
    
    // Test angles in both directions
    for (int32 i = 1; i <= NumTestAngles; ++i)
    {
        float Fraction = static_cast<float>(i) / static_cast<float>(NumTestAngles);
        float TestAngle = MaxOrbitAngle * Fraction;
        
        // Test positive and negative yaw
        TArray<FRotator> TestRotations;
        
        if (bPreferYawOrbit)
        {
            // Primarily yaw, with optional pitch
            TestRotations.Add(FRotator(0.0f, TestAngle, 0.0f));
            TestRotations.Add(FRotator(0.0f, -TestAngle, 0.0f));
            
            // Also test pitch at smaller angles
            float PitchAngle = FMath::Min(TestAngle, MaxPitchAdjustment);
            TestRotations.Add(FRotator(PitchAngle, 0.0f, 0.0f));
            TestRotations.Add(FRotator(-PitchAngle, 0.0f, 0.0f));
            
            // Combined
            TestRotations.Add(FRotator(PitchAngle * 0.5f, TestAngle * 0.7f, 0.0f));
            TestRotations.Add(FRotator(PitchAngle * 0.5f, -TestAngle * 0.7f, 0.0f));
        }
        else
        {
            // Equal yaw and pitch testing
            TestRotations.Add(FRotator(0.0f, TestAngle, 0.0f));
            TestRotations.Add(FRotator(0.0f, -TestAngle, 0.0f));
            TestRotations.Add(FRotator(TestAngle, 0.0f, 0.0f));
            TestRotations.Add(FRotator(-TestAngle, 0.0f, 0.0f));
        }
        
        for (const FRotator& TestRot : TestRotations)
        {
            if (TestOrbit(Context, TestRot))
            {
                // Score based on smaller rotation being better
                float RotationMagnitude = FMath::Abs(TestRot.Yaw) + FMath::Abs(TestRot.Pitch) * 1.5f;
                float Score = MaxOrbitAngle * 2.0f - RotationMagnitude;
                
                if (Score > BestScore)
                {
                    BestScore = Score;
                    BestRotation = TestRot;
                }
            }
        }
        
        // If found a good rotation at this angle, use it (prefer smaller angles)
        if (BestScore > 0.0f)
        {
            break;
        }
    }
    
    return BestRotation;
}

bool UCameraCollision_RS03_Orbit::TestOrbit(
    const FStageExecutionContext& Context,
    const FRotator& TestOrbitRotation) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return true;  // Assume valid if can't check
    }
    
    // Apply test rotation to camera direction
    FRotator AdjustedRotation = Context.Output.Rotation + TestOrbitRotation;
    FVector AdjustedDirection = -AdjustedRotation.Vector();
    
    // Calculate test camera position
    FVector FocusPoint = Context.Output.FocusPoint;
    FVector TestPosition = FocusPoint + AdjustedDirection * Context.Output.Distance;
    
    // Trace to check if path is clear
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    FHitResult Hit;
    bool bBlocked = World->LineTraceSingleByChannel(
        Hit,
        FocusPoint,
        TestPosition,
        TraceChannel,
        Params
    );
    
    // Clear if no hit, or hit is far enough
    if (!bBlocked)
    {
        return true;
    }
    
    // Consider it valid if we can get at least 80% of desired distance
    return Hit.Distance >= Context.Output.Distance * 0.8f;
}


/**
 * Constructor - Configure FOV compensation parameters
 * 
 * FOV compensation works with PullIn to maintain visual framing:
 * - When camera is pulled closer, character appears larger
 * - Increasing FOV compensates by widening the view
 * - Creates a more natural feel during collision handling
 */
UCameraCollision_RS04_FOVCompensate::UCameraCollision_RS04_FOVCompensate()
    : MaxFOVIncrease(20.0f)
    , FOVPerDistanceUnit(0.1f)
    , FOVAdjustSpeed(10.0f)
    , MinDistanceReductionThreshold(30.0f)
    , CurrentFOVAdjustment(0.0f)
    , TargetFOVAdjustment(0.0f)
    , OriginalDistance(0.0f)
{
    Priority = 95;  // Lower priority, applied after distance adjustments
    BaseInterpSpeed = 10.0f;
    bCanBlend = true;
}

bool UCameraCollision_RS04_FOVCompensate::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_RS04_FOVCompensate::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if response is needed
    if (!IsResponseNeeded(DetectionResult))
    {
        // No collision - smoothly return FOV to normal
        if (FMath::Abs(CurrentFOVAdjustment) > 0.1f)
        {
            CurrentFOVAdjustment = InterpolateValue(
                CurrentFOVAdjustment,
                0.0f,
                CachedDeltaTime,
                BaseInterpSpeed * 0.5f  // Return slower for smooth transition
            );
            
            // Still output the returning adjustment
            InitializeResponse(OutResponse);
            OutResponse.FOVAdjustment = CurrentFOVAdjustment;
            OutResponse.ActiveDuration = ResponseTime;
            
            bResponseActive = true;
            SetActive(true);
            return true;
        }
        
        // Reset original distance tracking
        OriginalDistance = 0.0f;
        bResponseActive = false;
        SetActive(false);
        return false;
    }

    bResponseActive = true;
    SetActive(true);

    // Track original distance if just started colliding
    if (OriginalDistance <= 0.0f)
    {
        OriginalDistance = DetectionResult.DesiredDistance;
    }

    // Calculate distance reduction
    float DistanceReduction = OriginalDistance - DetectionResult.SafeDistance;
    
    // Only compensate if reduction exceeds threshold
    if (DistanceReduction < MinDistanceReductionThreshold)
    {
        TargetFOVAdjustment = 0.0f;
    }
    else
    {
        // Calculate FOV increase based on distance reduction
        float ExcessReduction = DistanceReduction - MinDistanceReductionThreshold;
        TargetFOVAdjustment = ExcessReduction * FOVPerDistanceUnit;
        
        // Clamp to maximum
        TargetFOVAdjustment = FMath::Min(TargetFOVAdjustment, MaxFOVIncrease);
    }
    
    // Adjust speed based on urgency
    float Urgency = DetectionResult.AdjustmentRatio;
    float EffectiveSpeed = FOVAdjustSpeed * (1.0f + Urgency);
    
    // Interpolate current FOV towards target
    CurrentFOVAdjustment = InterpolateValue(
        CurrentFOVAdjustment,
        TargetFOVAdjustment,
        CachedDeltaTime,
        EffectiveSpeed
    );

    // Build response output
    InitializeResponse(OutResponse);
    OutResponse.FOVAdjustment = CurrentFOVAdjustment;
    OutResponse.ActiveDuration = ResponseTime;

    UE_LOG(LogTemp, Verbose, TEXT("RS04 FOVCompensate: FOV+=%.1f (target=%.1f, distReduction=%.1f)"),
        CurrentFOVAdjustment, TargetFOVAdjustment, DistanceReduction);

    return true;
}

void UCameraCollision_RS04_FOVCompensate::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_RS04_FOVCompensate::Reset()
{
    Super::Reset();
    CurrentFOVAdjustment = 0.0f;
    TargetFOVAdjustment = 0.0f;
    OriginalDistance = 0.0f;
}


/**
 * Constructor - Configure instant snap parameters
 * 
 * Instant snap is the emergency response: 
 * - Immediately snaps camera to safe position
 * - No smoothing for severe collisions
 * - Highest priority response
 */
UCameraCollision_RS05_InstantSnap::UCameraCollision_RS05_InstantSnap()
    : SnapThreshold(0.7f)
    , bMicroSmooth(true)
    , MicroSmoothFrames(2)
    , SafetyPadding(5.0f)
    , bSnapTriggered(false)
    , MicroSmoothRemaining(0)
    , MicroSmoothTarget(0.0f)
{
    Priority = 120;  // Highest priority - emergency response
    BaseInterpSpeed = 50.0f;  // Very fast when needed
    bOverrideOthers = true;  // Override other responses when triggered
}

bool UCameraCollision_RS05_InstantSnap::ShouldExecute(const FStageExecutionContext& Context) const
{
    return bIsEnabled;
}

bool UCameraCollision_RS05_InstantSnap::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Check if collision is severe enough for snap
    if (!DetectionResult.bCollisionDetected || DetectionResult.AdjustmentRatio < SnapThreshold)
    {
        // Not severe enough - reset state
        bSnapTriggered = false;
        MicroSmoothRemaining = 0;
        bResponseActive = false;
        SetActive(false);
        return false;
    }

    bResponseActive = true;
    SetActive(true);

    // Calculate snap distance
    float DesiredDistance = DetectionResult.DesiredDistance;
    float SafeDistance = DetectionResult.SafeDistance - SafetyPadding;
    SafeDistance = FMath::Max(SafeDistance, MinDistance);  // Don't go below minimum
    
    float SnapAdjustment = -(DesiredDistance - SafeDistance);

    // Apply micro-smooth if enabled
    if (bMicroSmooth && !bSnapTriggered)
    {
        // First frame of snap - start micro smooth
        bSnapTriggered = true;
        MicroSmoothRemaining = MicroSmoothFrames;
        MicroSmoothTarget = SnapAdjustment;
    }

    float FinalAdjustment;
    
    if (bMicroSmooth && MicroSmoothRemaining > 0)
    {
        // Micro-smooth in progress
        float Progress = 1.0f - (static_cast<float>(MicroSmoothRemaining) / static_cast<float>(MicroSmoothFrames));
        FinalAdjustment = FMath::Lerp(0.0f, MicroSmoothTarget, Progress);
        MicroSmoothRemaining--;
    }
    else
    {
        // Immediate snap
        FinalAdjustment = SnapAdjustment;
    }

    // Build response output
    InitializeResponse(OutResponse);
    OutResponse.DistanceAdjustment = FinalAdjustment;
    OutResponse.bOverride = true;  // Override other responses
    OutResponse.ActiveDuration = ResponseTime;

    UE_LOG(LogTemp, Verbose, TEXT("RS05 InstantSnap: Adjustment=%.1f (severity=%.2f, threshold=%.2f)"),
        FinalAdjustment, DetectionResult.AdjustmentRatio, SnapThreshold);

    return true;
}

void UCameraCollision_RS05_InstantSnap::Update(float DeltaTime, const FStageExecutionContext& Context)
{
    Super::Update(DeltaTime, Context);
}

void UCameraCollision_RS05_InstantSnap::Reset()
{
    Super::Reset();
    bSnapTriggered = false;
    MicroSmoothRemaining = 0;
    MicroSmoothTarget = 0.0f;
}
