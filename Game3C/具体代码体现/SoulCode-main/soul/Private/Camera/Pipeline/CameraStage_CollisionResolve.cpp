// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_CollisionResolve.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Collision/CameraCollisionResolver.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_Collision, Log, All);

//========================================
// Constructor
//========================================

UCameraStage_CollisionResolve::UCameraStage_CollisionResolve()
    : CollisionResolver(nullptr)
    , bCollisionActive(false)
    , bWasCollisionActive(false)
    , TimeSinceCollisionEnd(0.0f)
    , CurrentCollisionRatio(0.0f)
    , TargetCollisionRatio(0.0f)
    , OriginalDistance(0.0f)
    , TracesPerformed(0)
    , TotalCollisionTimeMs(0.0f)
    , DefaultProbeRadius(12.0f)
    , DefaultRecoveryDelay(0.3f)
    , DefaultRecoverySpeed(500.0f)
{
}

//========================================
// ICameraStage Interface
//========================================

bool UCameraStage_CollisionResolve::ShouldExecute(const FStageExecutionContext& Context) const
{
    // Skip if in cinematic mode (cinematics usually handle their own collision)
    // TODO Phase 5: Check Context.Output.bCinematicOverride
    
    UE_LOG(LogCameraStage_Collision, VeryVerbose, TEXT("ShouldExecute: true"));
    return true;
}

EStageResult UCameraStage_CollisionResolve::Execute(FStageExecutionContext& Context)
{
    // ★★★ 混合模式 C ★★★
    // 基础碰撞拉近：由 UE4 SpringArm 原生 bDoCollisionTest 处理
    // 高级功能：由此 Stage 的 CollisionResolver 处理
    //   - RS02 Slide: 沿墙面滑动
    //   - RS03 Orbit: 绕过障碍物旋转
    //   - RS04 FOV: 拉近时 FOV 补偿
    //   - OC01-04: 角色/目标遮挡处理
    //   - SP01-04: 特殊场景（悬崖、狭窄空间、低天花板等）
    //
    // RS01 PullIn 已改为被动模式，不产生距离调整

    // ★★★ 诊断日志开始 ★★★
    static int32 Stage6DiagCount = 0;
    bool bShouldLog = (Stage6DiagCount < 10);
    
    float DistanceAtStart = Context.Output.Distance;
    
    if (bShouldLog)
    {
        Stage6DiagCount++;
        UE_LOG(LogTemp, Error, TEXT(""));
        UE_LOG(LogTemp, Error, TEXT("╔══════════════════════════════════════════════════════════════╗"));
        UE_LOG(LogTemp, Error, TEXT("║  Stage6 CollisionResolve 诊断 #%d                            ║"), Stage6DiagCount);
        UE_LOG(LogTemp, Error, TEXT("╚══════════════════════════════════════════════════════════════╝"));
        UE_LOG(LogTemp, Error, TEXT("【输入】"));
        UE_LOG(LogTemp, Error, TEXT("   Context.Output.Distance: %.1f"), DistanceAtStart);
        UE_LOG(LogTemp, Error, TEXT("   CollisionResolver: %s"), CollisionResolver ? TEXT("存在") : TEXT("NULL"));
        
        if (CollisionResolver)
        {
            UE_LOG(LogTemp, Error, TEXT("   CollisionResolver->IsInitialized(): %s"), 
                CollisionResolver->IsInitialized() ? TEXT("YES") : TEXT("NO"));
            UE_LOG(LogTemp, Error, TEXT("   CollisionResolver->IsCollisionActive(): %s"), 
                CollisionResolver->IsCollisionActive() ? TEXT("YES") : TEXT("NO"));
        }
    }
    // ★★★ 诊断日志结束 ★★★

    if (!Context.Manager)
    {
        UE_LOG(LogCameraStage_Collision, Warning, TEXT("Execute: No manager reference"));
        return EStageResult::Failed;
    }

    // Reset statistics
    TracesPerformed = 0;
    const double StartTime = FPlatformTime::Seconds();

    // Store original values before collision adjustment
    OriginalDistance = Context.Output.Distance;
    Context.Output.PreCollisionDistance = OriginalDistance;
    Context.Output.bCollisionAdjusted = false;
    Context.Output.CollisionCompressionRatio = 1.0f;

    // Remember previous collision state
    bWasCollisionActive = bCollisionActive;

    //========================================
    // Use CollisionResolver if available (Phase 5)
    //========================================
    if (CollisionResolver && CollisionResolver->IsInitialized())
    {
        if (bShouldLog)
        {
            UE_LOG(LogTemp, Error, TEXT("【路径】使用 CollisionResolver 分支"));
            UE_LOG(LogTemp, Error, TEXT("   调用 ResolveCollisions 前 Distance: %.1f"), Context.Output.Distance);
        }
        
        // Let the resolver handle all collision logic
        CollisionResolver->ResolveCollisions(Context);
        
        if (bShouldLog)
        {
            UE_LOG(LogTemp, Error, TEXT("   调用 ResolveCollisions 后 Distance: %.1f"), Context.Output.Distance);
        }
        
        // Update resolver state
        CollisionResolver->Update(Context.DeltaTime, Context);
        
        // Update local state from resolver
        bCollisionActive = CollisionResolver->IsCollisionActive();
        
        TotalCollisionTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);
        
        UE_LOG(LogCameraStage_Collision, Verbose, TEXT("Execute (Resolver): Collision=%s, Time=%.2fms"),
            bCollisionActive ? TEXT("Active") : TEXT("None"), TotalCollisionTimeMs);
        
        // ★★★ 最终诊断日志 ★★★
        if (bShouldLog)
        {
            float DistanceAtEnd = Context.Output.Distance;
            UE_LOG(LogTemp, Error, TEXT("【输出 - Resolver分支】"));
            UE_LOG(LogTemp, Error, TEXT("   Context.Output.Distance: %.1f"), DistanceAtEnd);
            UE_LOG(LogTemp, Error, TEXT("   变化: %.1f -> %.1f (差异: %.1f)"), 
                DistanceAtStart, DistanceAtEnd, DistanceAtEnd - DistanceAtStart);
            UE_LOG(LogTemp, Error, TEXT("   bCollisionAdjusted: %s"), Context.Output.bCollisionAdjusted ? TEXT("YES") : TEXT("NO"));
            
            if (FMath::Abs(DistanceAtStart - DistanceAtEnd) > 1.0f)
            {
                UE_LOG(LogTemp, Error, TEXT("   ⚠️ 距离被修改了！差异: %.1f"), DistanceAtEnd - DistanceAtStart);
            }
            UE_LOG(LogTemp, Error, TEXT("════════════════════════════════════════════════════════════════"));
        }
        
        return EStageResult::Success;
    }

    //========================================
    // Fallback: Basic collision detection
    //========================================
    
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【路径】使用 Fallback 基本碰撞检测分支"));
    }
    
    // Perform collision detection
    FHitResult HitResult;
    const bool bCollisionDetected = DetectCollision(Context, HitResult);

    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("   碰撞检测结果: %s"), bCollisionDetected ? TEXT("有碰撞") : TEXT("无碰撞"));
    }

    if (bCollisionDetected)
    {
        if (bShouldLog)
        {
            UE_LOG(LogTemp, Error, TEXT("   >>> 检测到碰撞，调用 ApplyCollisionResponse"));
            UE_LOG(LogTemp, Error, TEXT("   >>> 碰撞前 Distance: %.1f"), Context.Output.Distance);
        }
        
        // Collision detected - apply response
        ApplyCollisionResponse(Context, HitResult);
        bCollisionActive = true;
        TimeSinceCollisionEnd = 0.0f;
        
        if (bShouldLog)
        {
            UE_LOG(LogTemp, Error, TEXT("   >>> 碰撞响应后 Distance: %.1f"), Context.Output.Distance);
        }
    }
    else
    {
        // No collision - handle recovery
        if (bCollisionActive)
        {
            if (bShouldLog)
            {
                UE_LOG(LogTemp, Error, TEXT("   >>> 无碰撞但之前有，调用 UpdateCollisionRecovery"));
                UE_LOG(LogTemp, Error, TEXT("   >>> 恢复前 Distance: %.1f"), Context.Output.Distance);
            }
            
            UpdateCollisionRecovery(Context);
            
            if (bShouldLog)
            {
                UE_LOG(LogTemp, Error, TEXT("   >>> 恢复后 Distance: %.1f"), Context.Output.Distance);
            }
        }
    }

    TotalCollisionTimeMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

    UE_LOG(LogCameraStage_Collision, Verbose, TEXT("Execute (Fallback): %d traces, Collision=%s, Time=%.2fms"),
        TracesPerformed, bCollisionActive ? TEXT("Active") : TEXT("None"), TotalCollisionTimeMs);

    // ★★★ 最终诊断日志 ★★★
    if (bShouldLog)
    {
        float DistanceAtEnd = Context.Output.Distance;
        UE_LOG(LogTemp, Error, TEXT("【输出 - Fallback分支】"));
        UE_LOG(LogTemp, Error, TEXT("   Context.Output.Distance: %.1f"), DistanceAtEnd);
        UE_LOG(LogTemp, Error, TEXT("   变化: %.1f -> %.1f (差异: %.1f)"), 
            DistanceAtStart, DistanceAtEnd, DistanceAtEnd - DistanceAtStart);
        UE_LOG(LogTemp, Error, TEXT("   bCollisionAdjusted: %s"), Context.Output.bCollisionAdjusted ? TEXT("YES") : TEXT("NO"));
        
        if (FMath::Abs(DistanceAtStart - DistanceAtEnd) > 1.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("   ⚠️ 距离被修改了！差异: %.1f"), DistanceAtEnd - DistanceAtStart);
        }
        UE_LOG(LogTemp, Error, TEXT("════════════════════════════════════════════════════════════════"));
    }

    return EStageResult::Success;
}

void UCameraStage_CollisionResolve::OnPreExecute(const FStageExecutionContext& Context)
{
    UE_LOG(LogCameraStage_Collision, VeryVerbose, TEXT("Stage 6 CollisionResolve: PreExecute - Distance=%.1f"),
        Context.Output.Distance);
}

void UCameraStage_CollisionResolve::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    UE_LOG(LogCameraStage_Collision, VeryVerbose, TEXT("Stage 6 CollisionResolve: PostExecute - Distance=%.1f, Adjusted=%s"),
        Context.Output.Distance, Context.Output.bCollisionAdjusted ? TEXT("Yes") : TEXT("No"));
}

//========================================
// Collision Resolver Reference
//========================================

void UCameraStage_CollisionResolve::SetCollisionResolver(UCameraCollisionResolver* InResolver)
{
    CollisionResolver = InResolver;
    
    if (CollisionResolver)
    {
        UE_LOG(LogCameraStage_Collision, Log, TEXT("SetCollisionResolver: Connected (%d strategies)"),
            CollisionResolver->GetStrategyCount());
    }
    else
    {
        UE_LOG(LogCameraStage_Collision, Warning, TEXT("SetCollisionResolver: Resolver is null, using fallback detection"));
    }
}

//========================================
// Collision Detection
//========================================

bool UCameraStage_CollisionResolve::DetectCollision(FStageExecutionContext& Context, FHitResult& OutHitResult)
{
    // Phase 5: Will use CollisionResolver with configurable strategies
    // For now, perform basic sphere sweep collision
    
    ECollisionStrategyID DetectionStrategy = GetCurrentDetectionStrategy(Context);
    
    switch (DetectionStrategy)
    {
        case ECollisionStrategyID::Collision_D01_SingleRay:
            return DetectCollision_SingleRay(Context, OutHitResult);
            
        case ECollisionStrategyID::Collision_D02_SphereSweep:
        default:
            return DetectCollision_SphereSweep(Context, OutHitResult);
    }
}

bool UCameraStage_CollisionResolve::DetectCollision_SingleRay(const FStageExecutionContext& Context, FHitResult& OutHitResult)
{
    UWorld* World = Context.Manager->GetWorld();
    if (!World)
    {
        return false;
    }

    // Calculate desired camera position
    const FVector FocusPoint = Context.Output.FocusPoint;
    const FVector CameraDirection = Context.Output.Rotation.Vector() * -1.0f;
    const float DesiredDistance = Context.Output.Distance;
    const FVector DesiredCameraPos = FocusPoint + CameraDirection * DesiredDistance;

    // Setup query params
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CameraCollision), false);
    QueryParams.AddIgnoredActor(Context.Manager->GetOwner());

    // Perform line trace
    const bool bHit = World->LineTraceSingleByChannel(
        OutHitResult,
        FocusPoint,
        DesiredCameraPos,
        ECC_Camera,
        QueryParams
    );

    TracesPerformed++;

    return bHit && OutHitResult.bBlockingHit;
}

bool UCameraStage_CollisionResolve::DetectCollision_SphereSweep(const FStageExecutionContext& Context, FHitResult& OutHitResult)
{
    UWorld* World = Context.Manager->GetWorld();
    if (!World)
    {
        return false;
    }

    // Calculate desired camera position
    const FVector FocusPoint = Context.Output.FocusPoint;
    const FVector CameraDirection = Context.Output.Rotation.Vector() * -1.0f;
    const float DesiredDistance = Context.Output.Distance;
    const FVector DesiredCameraPos = FocusPoint + CameraDirection * DesiredDistance;

    // Setup query params
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CameraCollision), false);
    QueryParams.AddIgnoredActor(Context.Manager->GetOwner());

    // Perform sphere sweep
    const bool bHit = World->SweepSingleByChannel(
        OutHitResult,
        FocusPoint,
        DesiredCameraPos,
        FQuat::Identity,
        ECC_Camera,
        FCollisionShape::MakeSphere(DefaultProbeRadius),
        QueryParams
    );

    TracesPerformed++;

    return bHit && OutHitResult.bBlockingHit;
}

//========================================
// Collision Response
//========================================

void UCameraStage_CollisionResolve::ApplyCollisionResponse(FStageExecutionContext& Context, const FHitResult& HitResult)
{
    ECollisionStrategyID ResponseStrategy = GetCurrentResponseStrategy(Context);
    
    switch (ResponseStrategy)
    {
        case ECollisionStrategyID::Collision_RS04_FOVCompensate:
            ApplyResponse_PullIn(Context, HitResult);
            // Also apply FOV compensation
            ApplyResponse_FOVCompensate(Context, OriginalDistance - Context.Output.Distance);
            break;
            
        case ECollisionStrategyID::Collision_RS01_PullIn:
        default:
            ApplyResponse_PullIn(Context, HitResult);
            break;
    }
}

void UCameraStage_CollisionResolve::ApplyResponse_PullIn(FStageExecutionContext& Context, const FHitResult& HitResult)
{
    // Calculate the focus point to hit location distance
    const FVector FocusPoint = Context.Output.FocusPoint;
    float HitDistance = (HitResult.Location - FocusPoint).Size();
    
    // Apply safety margin (probe radius)
    const float MinDistance = 50.0f; // Minimum allowed distance
    float AdjustedDistance = FMath::Max(HitDistance - DefaultProbeRadius, MinDistance);

    // Calculate compression ratio
    const float CompressionRatio = (OriginalDistance > 0.0f) ? (AdjustedDistance / OriginalDistance) : 1.0f;

    // Update collision ratio for smooth transitions
    TargetCollisionRatio = 1.0f - CompressionRatio;
    CurrentCollisionRatio = FMath::FInterpTo(CurrentCollisionRatio, TargetCollisionRatio, Context.DeltaTime, 10.0f);

    // Apply adjusted distance
    Context.Output.Distance = AdjustedDistance;
    Context.Output.bCollisionAdjusted = true;
    Context.Output.CollisionCompressionRatio = CompressionRatio;

    UE_LOG(LogCameraStage_Collision, Verbose, TEXT("ApplyResponse_PullIn: Distance %.1f -> %.1f (ratio: %.2f)"),
        OriginalDistance, AdjustedDistance, CompressionRatio);
}

void UCameraStage_CollisionResolve::ApplyResponse_FOVCompensate(FStageExecutionContext& Context, float DistanceReduction)
{
    // Increase FOV to compensate for reduced distance
    // This helps maintain a similar framing when camera is pushed in
    
    const float MaxFOVCompensation = 15.0f; // Maximum FOV increase (degrees)
    const float CompensationFactor = FMath::Clamp(DistanceReduction / OriginalDistance, 0.0f, 1.0f);
    const float FOVIncrease = MaxFOVCompensation * CompensationFactor;

    Context.Output.FOV += FOVIncrease;

    UE_LOG(LogCameraStage_Collision, VeryVerbose, TEXT("ApplyResponse_FOVCompensate: FOV increased by %.1f degrees"),
        FOVIncrease);
}

//========================================
// Collision Recovery
//========================================

void UCameraStage_CollisionResolve::UpdateCollisionRecovery(FStageExecutionContext& Context)
{
    ECollisionStrategyID RecoveryStrategy = GetCurrentRecoveryStrategy(Context);
    
    switch (RecoveryStrategy)
    {
        case ECollisionStrategyID::Collision_RC02_SmoothRecovery:
        default:
            ApplyRecovery_Smooth(Context);
            break;
    }
}

void UCameraStage_CollisionResolve::ApplyRecovery_Smooth(FStageExecutionContext& Context)
{
    // Wait for recovery delay
    TimeSinceCollisionEnd += Context.DeltaTime;
    
    if (TimeSinceCollisionEnd < DefaultRecoveryDelay)
    {
        // Still in delay period, maintain current collision adjustment
        return;
    }

    // Gradually return to original distance
    TargetCollisionRatio = 0.0f;
    CurrentCollisionRatio = FMath::FInterpTo(CurrentCollisionRatio, TargetCollisionRatio, Context.DeltaTime, 5.0f);

    // Check if recovery is complete
    if (CurrentCollisionRatio < 0.01f)
    {
        CurrentCollisionRatio = 0.0f;
        bCollisionActive = false;
        UE_LOG(LogCameraStage_Collision, Verbose, TEXT("ApplyRecovery_Smooth: Recovery complete"));
    }
}

//========================================
// Occlusion Handling
//========================================

void UCameraStage_CollisionResolve::HandleCharacterOcclusion(FStageExecutionContext& Context, const FHitResult& HitResult)
{
    // Phase 5: Will implement character fade/hide when camera is too close
    // For now, just log
    
    UE_LOG(LogCameraStage_Collision, VeryVerbose, TEXT("HandleCharacterOcclusion: Placeholder (Phase 5)"));
}

//========================================
// Placeholder Methods
//========================================

ECollisionStrategyID UCameraStage_CollisionResolve::GetCurrentDetectionStrategy(const FStageExecutionContext& Context) const
{
    // Phase 5: Will get from state config via CollisionResolver
    // For now, return default sphere sweep
    return ECollisionStrategyID::Collision_D02_SphereSweep;
}

ECollisionStrategyID UCameraStage_CollisionResolve::GetCurrentResponseStrategy(const FStageExecutionContext& Context) const
{
    // Phase 5: Will get from state config via CollisionResolver
    // For now, return default pull-in
    return ECollisionStrategyID::Collision_RS01_PullIn;
}

ECollisionStrategyID UCameraStage_CollisionResolve::GetCurrentRecoveryStrategy(const FStageExecutionContext& Context) const
{
    // Phase 5: Will get from state config via CollisionResolver
    // For now, return default smooth recovery
    return ECollisionStrategyID::Collision_RC02_SmoothRecovery;
}
