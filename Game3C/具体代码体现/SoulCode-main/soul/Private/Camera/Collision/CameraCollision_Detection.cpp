// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Collision/CameraCollision_Detection.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

/**
 * CameraCollision_Detection.cpp
 * 
 * This file contains the implementations of the Detection base class
 * and all 4 Detection strategies (D01-D04).
 * 
 * Detection strategies are responsible for detecting collisions between
 * the camera and the environment. They populate FCollisionDetectionResult
 * with information about what was hit.
 * 
 * Implementation Structure:
 * - DetectionBase: Common detection functionality
 * - D01: SingleRay - Single ray detection
 * - D02: SphereSweep - Sphere sweep detection
 * - D03: MultiRay - Multiple ray detection
 * - D04: MultiSphereSweep - Multiple sphere sweep detection
 */


//========================================
// DetectionBase - Common Detection Functionality
//========================================

UCameraCollision_DetectionBase::UCameraCollision_DetectionBase()
    : SafetyMargin(10.0f)
    , TraceExtension(5.0f)
    , bDrawDebugTraces(false)
{
    // Detection strategies are enabled by default
    bIsEnabled = true;
    bCanBlend = false;  // Detection strategies don't blend
}

void UCameraCollision_DetectionBase::CalculateTraceEndpoints(
    const FStageExecutionContext& Context,
    FVector& OutStart,
    FVector& OutEnd) const
{
    // Start point is the focus point (where camera looks at)
    OutStart = Context.Output.FocusPoint;
    
    // Calculate camera direction (away from focus point)
    FVector CameraDirection = -Context.Output.Rotation.Vector();
    
    // End point is desired camera position (plus extension for early detection)
    float TraceDistance = Context.Output.Distance + TraceExtension;
    OutEnd = OutStart + CameraDirection * TraceDistance;
    
    // ★★★ 诊断：检查起点是否在角色内部 ★★★
    static int32 EndpointDiagCount = 0;
    if (EndpointDiagCount < 5)
    {
        EndpointDiagCount++;
        float StartToCharacter = (OutStart - Context.InputContext.CharacterLocation).Size();
        UE_LOG(LogTemp, Warning, TEXT("【CalculateTraceEndpoints #%d】"), EndpointDiagCount);
        UE_LOG(LogTemp, Warning, TEXT("  CharacterLocation: (%.1f, %.1f, %.1f)"), 
            Context.InputContext.CharacterLocation.X, 
            Context.InputContext.CharacterLocation.Y, 
            Context.InputContext.CharacterLocation.Z);
        UE_LOG(LogTemp, Warning, TEXT("  FocusPoint(OutStart): (%.1f, %.1f, %.1f)"), 
            OutStart.X, OutStart.Y, OutStart.Z);
        UE_LOG(LogTemp, Warning, TEXT("  StartToCharacter 距离: %.1f"), StartToCharacter);
        UE_LOG(LogTemp, Warning, TEXT("  CameraDirection: (%.2f, %.2f, %.2f)"), 
            CameraDirection.X, CameraDirection.Y, CameraDirection.Z);
        
        if (StartToCharacter < 50.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("  ⚠️ FocusPoint 可能在角色 Capsule 内部！"));
        }
    }
}

FVector UCameraCollision_DetectionBase::CalculateSafePosition(
    const FVector& Start,
    const FVector& End,
    const FHitResult& HitResult,
    float InSafetyMargin) const
{
    if (!HitResult.bBlockingHit)
    {
        return End;
    }
    
    // Direction from start to end
    FVector Direction = (End - Start).GetSafeNormal();
    
    // Safe position is at hit point minus safety margin
    float SafeDistance = FMath::Max(0.0f, HitResult.Distance - InSafetyMargin);
    
    return Start + Direction * SafeDistance;
}

float UCameraCollision_DetectionBase::CalculateSafeDistance(
    const FStageExecutionContext& Context,
    const FHitResult& HitResult) const
{
    if (!HitResult.bBlockingHit)
    {
        return Context.Output.Distance;
    }
    
    // Safe distance is hit distance minus safety margin, clamped to minimum
    float SafeDistance = FMath::Max(
        MinDistance,
        HitResult.Distance - SafetyMargin
    );
    
    return SafeDistance;
}

void UCameraCollision_DetectionBase::FillDetectionResult(
    const FStageExecutionContext& Context,
    const FHitResult& HitResult,
    FCollisionDetectionResult& OutDetectionResult) const
{
    if (!HitResult.bBlockingHit)
    {
        return;
    }
    
    OutDetectionResult.bCollisionDetected = true;
    OutDetectionResult.HitResult = HitResult;
    OutDetectionResult.CollisionNormal = HitResult.ImpactNormal;
    OutDetectionResult.CollisionDistance = HitResult.Distance;
    
    // Calculate safe distance
    OutDetectionResult.SafeDistance = CalculateSafeDistance(Context, HitResult);
    
    // Calculate safe location
    FVector Start, End;
    CalculateTraceEndpoints(Context, Start, End);
    OutDetectionResult.SafeLocation = CalculateSafePosition(Start, End, HitResult, SafetyMargin);
    
    // Store desired values for comparison
    OutDetectionResult.DesiredDistance = Context.Output.Distance;
    OutDetectionResult.DesiredLocation = End;
    
    // Calculate adjustment ratio (0 = no adjustment, 1 = full adjustment needed)
    if (Context.Output.Distance > MinDistance)
    {
        float AdjustmentNeeded = Context.Output.Distance - OutDetectionResult.SafeDistance;
        OutDetectionResult.AdjustmentRatio = FMath::Clamp(
            AdjustmentNeeded / (Context.Output.Distance - MinDistance),
            0.0f,
            1.0f
        );
    }
    else
    {
        OutDetectionResult.AdjustmentRatio = 0.0f;
    }
}


//========================================
// D01: Single Ray Detection Implementation
//========================================

/**
 * Constructor - Configure single ray detection parameters
 * 
 * Single ray is the simplest detection method:
 * - Single ray from focus point to camera
 * - Fast but may miss thin geometry
 * - Used as fallback or in simple environments
 */
UCameraCollision_D01_SingleRay::UCameraCollision_D01_SingleRay()
    : DetectionOffset(10.0f)
{
    // Lower priority - used as fallback
    Priority = 100;
    ProbeRadius = 0.0f;  // Line trace has no radius
}

bool UCameraCollision_D01_SingleRay::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Calculate trace endpoints
    FVector TraceStart, TraceEnd;
    CalculateTraceEndpoints(Context, TraceStart, TraceEnd);
    
    // Extend trace slightly past desired position for early detection
    FVector Direction = (TraceEnd - TraceStart).GetSafeNormal();
    TraceEnd += Direction * DetectionOffset;
    
    // Setup trace parameters
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    // Perform line trace
    FHitResult Hit;
    bool bHit = LineTrace(TraceStart, TraceEnd, Hit, Params);
    
    // Debug visualization
#if ENABLE_DRAW_DEBUG
    if (bDrawDebugTraces)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FColor TraceColor = bHit ? FColor::Red : FColor::Green;
            DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, false, 0.0f, 0, 1.0f);
            
            if (bHit)
            {
                DrawDebugSphere(World, Hit.ImpactPoint, 5.0f, 8, FColor::Red, false, 0.0f, 0, 1.0f);
            }
        }
    }
#endif
    
    if (bHit)
    {
        // Detection strategies update the detection result directly
        // We need to cast away const because detection strategies modify the result
        FCollisionDetectionResult& MutableResult = const_cast<FCollisionDetectionResult&>(DetectionResult);
        
        // Fill the detection result with hit information
        FillDetectionResult(Context, Hit, MutableResult);
        
        UE_LOG(LogTemp, Verbose, TEXT("D01 SingleRay: Hit at %.1f, safe=%.1f, actor=%s"),
            Hit.Distance,
            MutableResult.SafeDistance,
            Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"));
        
        // Mark strategy as active
        SetActive(true);
        return true;
    }
    
    SetActive(false);
    return false;
}


//========================================
// D02: Sphere Sweep Implementation
//========================================

/**
 * Constructor - Configure sphere sweep detection parameters
 * 
 * Sphere sweep is the preferred detection method:
 * - Catches thin geometry that line traces miss
 * - Provides accurate "safe" positions
 * - Matches UE4 SpringArm's default behavior
 */
UCameraCollision_D02_SphereSweep::UCameraCollision_D02_SphereSweep()
    : SweepRadius(12.0f)
    , MinSweepRadius(4.0f)
    , bAdaptiveRadius(true)
{
    // Highest priority - preferred detection method
    Priority = 120;
    ProbeRadius = 12.0f;
    SafetyMargin = 5.0f;
}

bool UCameraCollision_D02_SphereSweep::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // ★★★ 详细诊断日志 ★★★
    bool bShouldLog = false;
    
#if WITH_EDITOR
    // 从 Outer 链获取 CameraManager
    UObject* CurrentOuter = GetOuter();
    while (CurrentOuter)
    {
        if (USoulsCameraManager* Manager = Cast<USoulsCameraManager>(CurrentOuter))
        {
            bShouldLog = Manager->IsCollisionDebugEnabled();
            break;
        }
        CurrentOuter = CurrentOuter->GetOuter();
    }
#endif

    // 限制输出次数
    static int32 D02DiagCount = 0;
    if (bShouldLog && D02DiagCount < 50)
    {
        D02DiagCount++;
    }
    else
    {
        bShouldLog = false;
    }
    
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT(""));
        UE_LOG(LogTemp, Error, TEXT("┌──────────────────────────────────────────────────┐"));
        UE_LOG(LogTemp, Error, TEXT("│ D02_SphereSweep 诊断 #%d                          │"), D02DiagCount);
        UE_LOG(LogTemp, Error, TEXT("└──────────────────────────────────────────────────┘"));
    }
    
    // Calculate trace endpoints
    FVector TraceStart, TraceEnd;
    CalculateTraceEndpoints(Context, TraceStart, TraceEnd);
    
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【射线参数】"));
        UE_LOG(LogTemp, Error, TEXT("  FocusPoint: (%.1f, %.1f, %.1f)"), 
            Context.Output.FocusPoint.X, Context.Output.FocusPoint.Y, Context.Output.FocusPoint.Z);
        UE_LOG(LogTemp, Error, TEXT("  TraceStart: (%.1f, %.1f, %.1f)"), 
            TraceStart.X, TraceStart.Y, TraceStart.Z);
        UE_LOG(LogTemp, Error, TEXT("  TraceEnd: (%.1f, %.1f, %.1f)"), 
            TraceEnd.X, TraceEnd.Y, TraceEnd.Z);
        UE_LOG(LogTemp, Error, TEXT("  Rotation: (P=%.1f, Y=%.1f, R=%.1f)"),
            Context.Output.Rotation.Pitch, Context.Output.Rotation.Yaw, Context.Output.Rotation.Roll);
        UE_LOG(LogTemp, Error, TEXT("  Distance: %.1f"), Context.Output.Distance);
        UE_LOG(LogTemp, Error, TEXT("  TraceExtension: %.1f"), TraceExtension);
        
        // 计算射线长度和方向
        float TraceLength = (TraceEnd - TraceStart).Size();
        FVector TraceDir = (TraceEnd - TraceStart).GetSafeNormal();
        UE_LOG(LogTemp, Error, TEXT("  TraceLength: %.1f"), TraceLength);
        UE_LOG(LogTemp, Error, TEXT("  TraceDirection: (%.2f, %.2f, %.2f)"), 
            TraceDir.X, TraceDir.Y, TraceDir.Z);
    }
    
    // Determine sweep radius
    float CurrentRadius = SweepRadius;
    
    // Adaptive radius: reduce radius when close to obstacles
    if (bAdaptiveRadius && DetectionResult.bCollisionDetected)
    {
        // If previous frame had collision, try smaller radius
        float DistanceRatio = DetectionResult.SafeDistance / FMath::Max(1.0f, Context.Output.Distance);
        CurrentRadius = FMath::Lerp(MinSweepRadius, SweepRadius, DistanceRatio);
    }
    
    CurrentRadius = FMath::Max(CurrentRadius, MinSweepRadius);
    
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【Sweep 参数】"));
        UE_LOG(LogTemp, Error, TEXT("  SweepRadius: %.1f"), CurrentRadius);
        UE_LOG(LogTemp, Error, TEXT("  MinSweepRadius: %.1f"), MinSweepRadius);
        UE_LOG(LogTemp, Error, TEXT("  bAdaptiveRadius: %s"), bAdaptiveRadius ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Error, TEXT("  TraceChannel: %d"), (int32)TraceChannel.GetValue());
    }
    
    // Setup trace parameters
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【忽略的 Actor】"));
        // UE4.27 中直接检查是否有 IgnoreActor 设置
        UE_LOG(LogTemp, Error, TEXT("  (检查 GetDefaultTraceParams 中设置了哪些忽略)"));
    }
    
    // Perform sphere sweep
    FHitResult Hit;
    bool bHit = SphereTrace(TraceStart, TraceEnd, CurrentRadius, Hit, Params);
    
    // ★★★ 关键：碰撞结果诊断 ★★★
    if (bShouldLog)
    {
        UE_LOG(LogTemp, Error, TEXT("【碰撞结果】"));
        UE_LOG(LogTemp, Error, TEXT("  bHit: %s"), bHit ? TEXT("YES ⚠️") : TEXT("NO ✓"));
        
        if (bHit)
        {
            UE_LOG(LogTemp, Error, TEXT("  ★★★ 检测到碰撞！★★★"));
            UE_LOG(LogTemp, Error, TEXT("  HitDistance: %.1f"), Hit.Distance);
            UE_LOG(LogTemp, Error, TEXT("  ImpactPoint: (%.1f, %.1f, %.1f)"), 
                Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);
            UE_LOG(LogTemp, Error, TEXT("  ImpactNormal: (%.2f, %.2f, %.2f)"), 
                Hit.ImpactNormal.X, Hit.ImpactNormal.Y, Hit.ImpactNormal.Z);
            
            // ★ 最重要：碰撞到了什么物体 ★
            if (Hit.GetActor())
            {
                UE_LOG(LogTemp, Error, TEXT("  HitActor: %s"), *Hit.GetActor()->GetName());
                UE_LOG(LogTemp, Error, TEXT("  HitActor Class: %s"), *Hit.GetActor()->GetClass()->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("  HitActor: NULL (可能是BSP/地形)"));
            }
            
            if (Hit.GetComponent())
            {
                UE_LOG(LogTemp, Error, TEXT("  HitComponent: %s"), *Hit.GetComponent()->GetName());
                UE_LOG(LogTemp, Error, TEXT("  HitComponent Class: %s"), *Hit.GetComponent()->GetClass()->GetName());
            }
            
            UE_LOG(LogTemp, Error, TEXT("  PhysMaterial: %s"), 
                Hit.PhysMaterial.IsValid() ? *Hit.PhysMaterial.Get()->GetName() : TEXT("None"));
            UE_LOG(LogTemp, Error, TEXT("  BoneName: %s"), *Hit.BoneName.ToString());
            
            // 计算碰撞点到角色的距离
            float DistToCharacter = (Hit.ImpactPoint - Context.InputContext.CharacterLocation).Size();
            UE_LOG(LogTemp, Error, TEXT("  碰撞点到角色距离: %.1f"), DistToCharacter);
            
            // 检查是否可能碰到了自己
            if (DistToCharacter < 100.0f)
            {
                UE_LOG(LogTemp, Error, TEXT("  ⚠️ 警告：碰撞点非常接近角色！可能碰到了自己！"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  ✓ 未检测到碰撞"));
        }
        UE_LOG(LogTemp, Error, TEXT("──────────────────────────────────────────────────"));
    }
    
    // ★★★ 持续碰撞诊断（Hybrid Mode C - 使用调试开关）★★★
#if WITH_EDITOR
    bool bObstacleLog = false;
    UObject* OuterForObstacle = GetOuter();
    while (OuterForObstacle)
    {
        if (USoulsCameraManager* Manager = Cast<USoulsCameraManager>(OuterForObstacle))
        {
            bObstacleLog = Manager->IsCollisionDebugEnabled();
            break;
        }
        OuterForObstacle = OuterForObstacle->GetOuter();
    }
    
    if (bObstacleLog && bHit)
    {
        // 只在碰到非角色时输出
        FString HitActorName = Hit.GetActor() ? Hit.GetActor()->GetName() : TEXT("BSP/Landscape");
        
        // 检查是否碰到了角色自己
        bool bHitSelf = false;
        if (Hit.GetActor())
        {
            FString ActorName = Hit.GetActor()->GetName();
            if (ActorName.Contains(TEXT("Character")) || ActorName.Contains(TEXT("Player")))
            {
                bHitSelf = true;
            }
        }
        
        if (!bHitSelf)
        {
            UE_LOG(LogTemp, Warning, TEXT("[碰撞检测] 碰到障碍物: %s, 距离: %.1f, SafeDist: %.1f"),
                *HitActorName,
                Hit.Distance,
                FMath::Max(MinDistance, Hit.Distance - (CurrentRadius + SafetyMargin)));
        }
    }
#endif
    
    // Debug visualization
#if ENABLE_DRAW_DEBUG
    if (bDrawDebugTraces)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FColor TraceColor = bHit ? FColor::Red : FColor::Green;
            
            // Draw capsule representing the sweep
            FVector CapsuleCenter = (TraceStart + TraceEnd) * 0.5f;
            float CapsuleHalfHeight = (TraceEnd - TraceStart).Size() * 0.5f;
            FVector TraceDirection = (TraceEnd - TraceStart).GetSafeNormal();
            FQuat CapsuleRotation = FQuat::FindBetweenNormals(FVector::UpVector, TraceDirection);
            
            DrawDebugCapsule(World, 
                CapsuleCenter,
                CapsuleHalfHeight,
                CurrentRadius,
                CapsuleRotation,
                TraceColor, false, 0.0f, 0, 1.0f);
            
            if (bHit)
            {
                DrawDebugSphere(World, Hit.ImpactPoint, CurrentRadius, 12, FColor::Red, false, 0.0f, 0, 1.0f);
                DrawDebugDirectionalArrow(World, Hit.ImpactPoint, 
                    Hit.ImpactPoint + Hit.ImpactNormal * 30.0f, 10.0f, FColor::Blue, false, 0.0f, 0, 1.0f);
            }
        }
    }
#endif
    
    if (bHit)
    {
        // Modify detection result
        FCollisionDetectionResult& MutableResult = const_cast<FCollisionDetectionResult&>(DetectionResult);
        
        // Fill basic hit info
        MutableResult.bCollisionDetected = true;
        MutableResult.HitResult = Hit;
        MutableResult.CollisionNormal = Hit.ImpactNormal;
        MutableResult.CollisionDistance = Hit.Distance;
        
        // Calculate safe distance (account for sphere radius)
        float SafeOffset = CurrentRadius + SafetyMargin;
        MutableResult.SafeDistance = FMath::Max(MinDistance, Hit.Distance - SafeOffset);
        
        // Calculate safe location
        FVector Direction = (TraceEnd - TraceStart).GetSafeNormal();
        MutableResult.SafeLocation = TraceStart + Direction * MutableResult.SafeDistance;
        
        // Store desired values
        MutableResult.DesiredDistance = Context.Output.Distance;
        MutableResult.DesiredLocation = TraceEnd;
        
        // Calculate adjustment ratio
        float TotalAdjustableDistance = Context.Output.Distance - MinDistance;
        if (TotalAdjustableDistance > 0.0f)
        {
            float AdjustmentNeeded = Context.Output.Distance - MutableResult.SafeDistance;
            MutableResult.AdjustmentRatio = FMath::Clamp(AdjustmentNeeded / TotalAdjustableDistance, 0.0f, 1.0f);
        }
        
        UE_LOG(LogTemp, Verbose, TEXT("D02 SphereSweep: Hit at %.1f (radius=%.1f), safe=%.1f, ratio=%.2f"),
            Hit.Distance, CurrentRadius, MutableResult.SafeDistance, MutableResult.AdjustmentRatio);
        
        SetActive(true);
        return true;
    }
    
    SetActive(false);
    return false;
}


UCameraCollision_D03_MultiRay::UCameraCollision_D03_MultiRay()
    : NumRays(5)
    , SpreadAngle(15.0f)
    , CenterRayWeight(0.6f)
    , bUseClosestHit(true)
{
    // Medium priority
    Priority = 110;
    SafetyMargin = 10.0f;
}

bool UCameraCollision_D03_MultiRay::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Calculate trace endpoints
    FVector TraceStart, TraceEnd;
    CalculateTraceEndpoints(Context, TraceStart, TraceEnd);
    
    FVector MainDirection = (TraceEnd - TraceStart).GetSafeNormal();
    float TraceDistance = (TraceEnd - TraceStart).Size();
    
    // Setup trace parameters
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    // Perform multi-ray trace using base class helper
    TArray<FHitResult> Hits;
    int32 HitCount = ConeTrace(TraceStart, MainDirection, TraceDistance, SpreadAngle, NumRays, Hits, Params);
    
    // Debug visualization
#if ENABLE_DRAW_DEBUG
    if (bDrawDebugTraces)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // Draw all rays in cone pattern
            for (int32 i = 0; i < NumRays; ++i)
            {
                // Calculate ray direction based on cone distribution
                float AngleFraction = (NumRays > 1) ? (static_cast<float>(i) / (NumRays - 1)) : 0.5f;
                float CurrentAngle = (AngleFraction - 0.5f) * SpreadAngle * 2.0f;
                
                FVector RayDirection = MainDirection.RotateAngleAxis(CurrentAngle, FVector::UpVector);
                FVector RayEnd = TraceStart + RayDirection * TraceDistance;
                
                bool bRayHit = i < Hits.Num() && Hits[i].bBlockingHit;
                FColor RayColor = bRayHit ? FColor::Red : FColor::Green;
                DrawDebugLine(World, TraceStart, RayEnd, RayColor, false, 0.0f, 0, 0.5f);
            }
            
            // Draw hit points
            for (const FHitResult& Hit : Hits)
            {
                if (Hit.bBlockingHit)
                {
                    DrawDebugSphere(World, Hit.ImpactPoint, 4.0f, 6, FColor::Red, false, 0.0f, 0, 1.0f);
                }
            }
        }
    }
#endif
    
    if (HitCount == 0)
    {
        SetActive(false);
        return false;
    }
    
    // Process hits
    FHitResult BestHit;
    
    if (bUseClosestHit)
    {
        // Find closest hit
        float ClosestDistance = FLT_MAX;
        for (const FHitResult& Hit : Hits)
        {
            if (Hit.bBlockingHit && Hit.Distance < ClosestDistance)
            {
                ClosestDistance = Hit.Distance;
                BestHit = Hit;
            }
        }
    }
    else
    {
        // Weighted average of all hits
        float TotalWeight = 0.0f;
        float WeightedDistance = 0.0f;
        FVector WeightedNormal = FVector::ZeroVector;
        
        for (int32 i = 0; i < Hits.Num(); ++i)
        {
            const FHitResult& Hit = Hits[i];
            if (!Hit.bBlockingHit)
            {
                continue;
            }
            
            // Center ray gets more weight
            float Weight = (i == 0) ? CenterRayWeight : (1.0f - CenterRayWeight) / FMath::Max(1, NumRays - 1);
            TotalWeight += Weight;
            WeightedDistance += Hit.Distance * Weight;
            WeightedNormal += Hit.ImpactNormal * Weight;
        }
        
        if (TotalWeight > 0.0f)
        {
            // Use first hit as base, modify distance
            BestHit = Hits[0];
            BestHit.Distance = WeightedDistance / TotalWeight;
            BestHit.ImpactNormal = (WeightedNormal / TotalWeight).GetSafeNormal();
        }
    }
    
    if (!BestHit.bBlockingHit)
    {
        SetActive(false);
        return false;
    }
    
    // Modify detection result
    FCollisionDetectionResult& MutableResult = const_cast<FCollisionDetectionResult&>(DetectionResult);
    
    // Store all hits for potential use by other strategies
    MutableResult.AllHits = Hits;
    
    // Fill detection result using best hit
    FillDetectionResult(Context, BestHit, MutableResult);
    
    UE_LOG(LogTemp, Verbose, TEXT("D03 MultiRay: %d/%d rays hit, best=%.1f, safe=%.1f"),
        HitCount, NumRays, BestHit.Distance, MutableResult.SafeDistance);
    
    SetActive(true);
    return true;
}


UCameraCollision_D04_MultiSphereSweep::UCameraCollision_D04_MultiSphereSweep()
    : PrimarySphereRadius(12.0f)
    , SecondarySphereRadius(8.0f)
    , VerticalOffset(30.0f)
    , bIncludeUpperSweep(true)
    , bIncludeLowerSweep(true)
{
    // Medium-high priority
    Priority = 115;
    SafetyMargin = 10.0f;
}

bool UCameraCollision_D04_MultiSphereSweep::Execute(
    const FStageExecutionContext& Context,
    const FCollisionDetectionResult& DetectionResult,
    FCollisionResponseOutput& OutResponse)
{
    // Calculate trace endpoints
    FVector TraceStart, TraceEnd;
    CalculateTraceEndpoints(Context, TraceStart, TraceEnd);
    
    FVector TraceDirection = (TraceEnd - TraceStart).GetSafeNormal();
    float TraceDistance = (TraceEnd - TraceStart).Size();
    
    // Setup collision parameters
    FCollisionQueryParams Params = GetDefaultTraceParams();
    
    // Get world
    UWorld* World = GetWorld();
    if (!World)
    {
        SetActive(false);
        return false;
    }
    
    // Track all hits and find closest
    TArray<FHitResult> AllHits;
    FHitResult ClosestHit;
    float ClosestDistance = FLT_MAX;
    bool bAnyHit = false;
    
    // Primary sphere sweep (center)
    FHitResult PrimaryHit;
    bool bPrimaryHit = SphereTrace(TraceStart, TraceEnd, PrimarySphereRadius, PrimaryHit, Params);
    
    if (bPrimaryHit)
    {
        AllHits.Add(PrimaryHit);
        if (PrimaryHit.Distance < ClosestDistance)
        {
            ClosestDistance = PrimaryHit.Distance;
            ClosestHit = PrimaryHit;
            bAnyHit = true;
        }
    }
    
    // Upper sphere sweep (detect low ceilings)
    if (bIncludeUpperSweep)
    {
        FVector UpperStart = TraceStart + FVector::UpVector * VerticalOffset;
        FVector UpperEnd = TraceEnd + FVector::UpVector * VerticalOffset;
        
        FHitResult UpperHit;
        bool bUpperHit = SphereTrace(UpperStart, UpperEnd, SecondarySphereRadius, UpperHit, Params);
        
        if (bUpperHit)
        {
            AllHits.Add(UpperHit);
            if (UpperHit.Distance < ClosestDistance)
            {
                ClosestDistance = UpperHit.Distance;
                ClosestHit = UpperHit;
                bAnyHit = true;
            }
        }
        
#if ENABLE_DRAW_DEBUG
        if (bDrawDebugTraces)
        {
            FColor UpperColor = bUpperHit ? FColor::Red : FColor::Cyan;
            DrawDebugLine(World, UpperStart, UpperEnd, UpperColor, false, 0.0f, 0, 0.5f);
            DrawDebugSphere(World, UpperStart, SecondarySphereRadius, 6, UpperColor, false, 0.0f, 0, 0.5f);
            DrawDebugSphere(World, UpperEnd, SecondarySphereRadius, 6, UpperColor, false, 0.0f, 0, 0.5f);
        }
#endif
    }
    
    // Lower sphere sweep (detect obstacles below)
    if (bIncludeLowerSweep)
    {
        FVector LowerStart = TraceStart - FVector::UpVector * VerticalOffset;
        FVector LowerEnd = TraceEnd - FVector::UpVector * VerticalOffset;
        
        FHitResult LowerHit;
        bool bLowerHit = SphereTrace(LowerStart, LowerEnd, SecondarySphereRadius, LowerHit, Params);
        
        if (bLowerHit)
        {
            AllHits.Add(LowerHit);
            if (LowerHit.Distance < ClosestDistance)
            {
                ClosestDistance = LowerHit.Distance;
                ClosestHit = LowerHit;
                bAnyHit = true;
            }
        }
        
#if ENABLE_DRAW_DEBUG
        if (bDrawDebugTraces)
        {
            FColor LowerColor = bLowerHit ? FColor::Red : FColor::Magenta;
            DrawDebugLine(World, LowerStart, LowerEnd, LowerColor, false, 0.0f, 0, 0.5f);
            DrawDebugSphere(World, LowerStart, SecondarySphereRadius, 6, LowerColor, false, 0.0f, 0, 0.5f);
            DrawDebugSphere(World, LowerEnd, SecondarySphereRadius, 6, LowerColor, false, 0.0f, 0, 0.5f);
        }
#endif
    }
    
    // Debug visualization for primary sweep
#if ENABLE_DRAW_DEBUG
    if (bDrawDebugTraces)
    {
        FColor PrimaryColor = bPrimaryHit ? FColor::Red : FColor::Green;
        
        // Draw primary sweep as capsule representation
        FVector CapsuleCenter = (TraceStart + TraceEnd) * 0.5f;
        float CapsuleHalfHeight = TraceDistance * 0.5f;
        FQuat CapsuleRotation = FQuat::FindBetweenNormals(FVector::UpVector, TraceDirection);
        
        DrawDebugCapsule(World, CapsuleCenter, CapsuleHalfHeight, PrimarySphereRadius,
            CapsuleRotation, PrimaryColor, false, 0.0f, 0, 1.0f);
        
        if (bAnyHit)
        {
            // Draw closest hit point
            DrawDebugSphere(World, ClosestHit.ImpactPoint, 8.0f, 8, FColor::Red, false, 0.0f, 0, 2.0f);
            
            // Draw impact normal
            DrawDebugDirectionalArrow(World, ClosestHit.ImpactPoint,
                ClosestHit.ImpactPoint + ClosestHit.ImpactNormal * 40.0f,
                15.0f, FColor::Blue, false, 0.0f, 0, 1.0f);
        }
    }
#endif
    
    if (!bAnyHit)
    {
        SetActive(false);
        return false;
    }
    
    // Modify detection result
    FCollisionDetectionResult& MutableResult = const_cast<FCollisionDetectionResult&>(DetectionResult);
    
    // Store all hits for potential use by other strategies
    MutableResult.AllHits = AllHits;
    
    // Fill basic hit info using closest hit
    MutableResult.bCollisionDetected = true;
    MutableResult.HitResult = ClosestHit;
    MutableResult.CollisionNormal = ClosestHit.ImpactNormal;
    MutableResult.CollisionDistance = ClosestHit.Distance;
    
    // Calculate safe distance (account for sphere radius and vertical offset)
    float EffectiveRadius = FMath::Max(PrimarySphereRadius, SecondarySphereRadius);
    float TotalSafetyOffset = EffectiveRadius + SafetyMargin;
    MutableResult.SafeDistance = FMath::Max(MinDistance, ClosestHit.Distance - TotalSafetyOffset);
    
    // Calculate safe location
    MutableResult.SafeLocation = TraceStart + TraceDirection * MutableResult.SafeDistance;
    
    // Store desired values
    MutableResult.DesiredDistance = Context.Output.Distance;
    MutableResult.DesiredLocation = TraceEnd;
    
    // Calculate adjustment ratio
    float TotalAdjustableDistance = Context.Output.Distance - MinDistance;
    if (TotalAdjustableDistance > 0.0f)
    {
        float AdjustmentNeeded = Context.Output.Distance - MutableResult.SafeDistance;
        MutableResult.AdjustmentRatio = FMath::Clamp(AdjustmentNeeded / TotalAdjustableDistance, 0.0f, 1.0f);
    }
    
    // Check for tight space (significant compression needed)
    if (MutableResult.AdjustmentRatio > 0.7f)
    {
        MutableResult.bInTightSpace = true;
    }
    
    // Check for upper/lower collision indicating confined space
    // When upper or lower sweeps hit closer than primary, mark as tight space
    if ((bIncludeUpperSweep || bIncludeLowerSweep) && AllHits.Num() > 1)
    {
        for (const FHitResult& Hit : AllHits)
        {
            if (&Hit != &ClosestHit && Hit.bBlockingHit)
            {
                // Secondary hit indicates confined vertical space
                if (Hit.Distance < ClosestHit.Distance * 0.95f)
                {
                    MutableResult.bInTightSpace = true;
                    break;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Verbose, TEXT("D04 MultiSphereSweep: %d hits, closest=%.1f, safe=%.1f, tight=%d"),
        AllHits.Num(), ClosestHit.Distance, MutableResult.SafeDistance, 
        MutableResult.bInTightSpace ? 1 : 0);
    
    SetActive(true);
    return true;
}
