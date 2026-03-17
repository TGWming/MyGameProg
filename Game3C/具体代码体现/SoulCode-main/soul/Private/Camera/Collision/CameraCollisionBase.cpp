// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Collision/CameraCollisionBase.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Collision/CameraCollisionResolver.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

/**
 * CameraCollisionBase.cpp
 * 
 * Implementation of the abstract base class for all collision strategies.
 * Provides common functionality for collision detection, tracing, and utilities.
 * 
 * This file implements:
 * - Constructor and initialization
 * - Strategy identity methods
 * - Update and reset functionality
 * - Trace helpers (Line, Sphere, Cone)
 * - Location validation
 * - Debug visualization
 */


//========================================
// Constructor
//========================================

UCameraCollisionBase::UCameraCollisionBase()
	: Priority(100)
	, BlendWeight(1.0f)
	, bCanBlend(true)
	, TraceChannel(ECC_Camera)
	, MinDistance(50.0f)
	, ProbeRadius(12.0f)
	, bIsActive(false)
	, bIsEnabled(true)
	, ActiveTime(0.0f)
{
}


//========================================
// Strategy Identity
//========================================

FName UCameraCollisionBase::GetStrategyName() const
{
	// Return class name as default strategy name
	return GetClass()->GetFName();
}

FString UCameraCollisionBase::GetStrategyDescription() const
{
	// Return formatted description with type and category
	return FString::Printf(TEXT("Collision Strategy: %s [%s]"),
		*GetStrategyName().ToString(),
		*UEnum::GetValueAsString(GetStrategyCategory()));
}


//========================================
// Strategy Execution
//========================================

bool UCameraCollisionBase::ShouldExecute(const FStageExecutionContext& Context) const
{
	// Default implementation: execute if enabled
	// Derived classes can add additional conditions
	return bIsEnabled;
}

void UCameraCollisionBase::Update(float DeltaTime, const FStageExecutionContext& Context)
{
	// Update active time if currently active
	if (bIsActive)
	{
		ActiveTime += DeltaTime;
	}
	else
	{
		// Reset active time when not active
		ActiveTime = 0.0f;
	}
}

void UCameraCollisionBase::Reset()
{
	bIsActive = false;
	ActiveTime = 0.0f;
}


//========================================
// Trace Helpers
//========================================

bool UCameraCollisionBase::LineTrace(
	const FVector& Start,
	const FVector& End,
	FHitResult& OutHit,
	const FCollisionQueryParams& Params) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraCollisionBase::LineTrace - No world available"));
		return false;
	}

	return World->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		TraceChannel,
		Params
	);
}

bool UCameraCollisionBase::SphereTrace(
	const FVector& Start,
	const FVector& End,
	float Radius,
	FHitResult& OutHit,
	const FCollisionQueryParams& Params) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraCollisionBase::SphereTrace - No world available"));
		return false;
	}

	FCollisionShape SphereShape;
	SphereShape.SetSphere(Radius);

	return World->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		SphereShape,
		Params
	);
}

int32 UCameraCollisionBase::ConeTrace(
	const FVector& Origin,
	const FVector& Direction,
	float Distance,
	float ConeAngle,
	int32 NumRays,
	TArray<FHitResult>& OutHits,
	const FCollisionQueryParams& Params) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraCollisionBase::ConeTrace - No world available"));
		return 0;
	}

	OutHits.Reset();
	OutHits.Reserve(NumRays);

	// Ensure we have at least one ray
	NumRays = FMath::Max(1, NumRays);

	// Normalize direction
	FVector NormalizedDir = Direction.GetSafeNormal();
	if (NormalizedDir.IsNearlyZero())
	{
		NormalizedDir = FVector::ForwardVector;
	}

	// Calculate perpendicular vectors for cone
	FVector Right = FVector::CrossProduct(NormalizedDir, FVector::UpVector).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Right = FVector::CrossProduct(NormalizedDir, FVector::RightVector).GetSafeNormal();
	}
	FVector Up = FVector::CrossProduct(Right, NormalizedDir).GetSafeNormal();

	int32 HitCount = 0;
	float ConeAngleRad = FMath::DegreesToRadians(ConeAngle);

	// Cast rays in cone pattern
	for (int32 i = 0; i < NumRays; ++i)
	{
		FVector RayDirection;

		if (i == 0)
		{
			// First ray is always the center
			RayDirection = NormalizedDir;
		}
		else
		{
			// Distribute remaining rays in a spiral pattern
			float t = static_cast<float>(i) / static_cast<float>(NumRays - 1);
			float AngleOffset = t * ConeAngleRad;
			float RotationAngle = t * PI * 4.0f;  // Spiral

			// Calculate offset direction
			FVector OffsetDir = Right * FMath::Cos(RotationAngle) + Up * FMath::Sin(RotationAngle);
			OffsetDir = OffsetDir.GetSafeNormal();

			// Rotate main direction by cone angle
			RayDirection = FMath::Lerp(NormalizedDir, OffsetDir, FMath::Sin(AngleOffset)).GetSafeNormal();
		}

		FVector TraceEnd = Origin + RayDirection * Distance;

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Origin, TraceEnd, TraceChannel, Params))
		{
			OutHits.Add(Hit);
			HitCount++;
		}
	}

	return HitCount;
}

bool UCameraCollisionBase::IsLocationValid(const FVector& Location, float Radius) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		// Assume valid if no world (safe default)
		return true;
	}

	FCollisionQueryParams Params = GetDefaultTraceParams();

	FCollisionShape SphereShape;
	SphereShape.SetSphere(Radius);

	// Check if location overlaps with any geometry
	TArray<FOverlapResult> Overlaps;
	bool bOverlap = World->OverlapMultiByChannel(
		Overlaps,
		Location,
		FQuat::Identity,
		TraceChannel,
		SphereShape,
		Params
	);

	// Location is valid if there are no overlaps
	return !bOverlap;
}

bool UCameraCollisionBase::FindSafeLocation(
	const FVector& DesiredLocation,
	FVector& OutSafeLocation,
	float SearchRadius) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		OutSafeLocation = DesiredLocation;
		return false;
	}

	// First check if desired location is already valid
	if (IsLocationValid(DesiredLocation, ProbeRadius))
	{
		OutSafeLocation = DesiredLocation;
		return true;
	}

	// Search in expanding spheres
	const int32 NumSearchRings = 4;
	const int32 PointsPerRing = 8;

	for (int32 Ring = 1; Ring <= NumSearchRings; ++Ring)
	{
		float RingRadius = SearchRadius * (static_cast<float>(Ring) / NumSearchRings);

		for (int32 Point = 0; Point < PointsPerRing; ++Point)
		{
			float Angle = (static_cast<float>(Point) / PointsPerRing) * 2.0f * PI;

			// Try horizontal offsets
			FVector TestLocation = DesiredLocation + FVector(
				FMath::Cos(Angle) * RingRadius,
				FMath::Sin(Angle) * RingRadius,
				0.0f
			);

			if (IsLocationValid(TestLocation, ProbeRadius))
			{
				OutSafeLocation = TestLocation;
				return true;
			}

			// Try with upward offset
			TestLocation.Z += RingRadius * 0.5f;
			if (IsLocationValid(TestLocation, ProbeRadius))
			{
				OutSafeLocation = TestLocation;
				return true;
			}
		}
	}

	// No safe location found, return original
	OutSafeLocation = DesiredLocation;
	return false;
}


//========================================
// Utility Methods
//========================================

UWorld* UCameraCollisionBase::GetWorld() const
{
	// Try cached world first
	if (CachedWorld.IsValid())
	{
		return CachedWorld.Get();
	}

	// Try to get world from owner camera manager
	if (OwnerCameraManager.IsValid())
	{
		UWorld* World = OwnerCameraManager->GetWorld();
		if (World)
		{
			const_cast<UCameraCollisionBase*>(this)->CachedWorld = World;
			return World;
		}
	}

	// Try to get world from outer chain
	UObject* Outer = GetOuter();
	while (Outer)
	{
		UWorld* World = Outer->GetWorld();
		if (World)
		{
			const_cast<UCameraCollisionBase*>(this)->CachedWorld = World;
			return World;
		}
		Outer = Outer->GetOuter();
	}

	return nullptr;
}

FCollisionQueryParams UCameraCollisionBase::GetDefaultTraceParams() const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CameraCollision), false);
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = false;

	// 尝试从 OwnerCameraManager 获取 Owner
	AActor* OwnerActor = nullptr;
	
	if (OwnerCameraManager.IsValid())
	{
		OwnerActor = OwnerCameraManager->GetOwner();
	}
	
	// ★★★ 备用方案：从 Outer 链获取 ★★★
	if (!OwnerActor)
	{
		UObject* CurrentOuter = GetOuter();
		while (CurrentOuter)
		{
			// 检查是否是 CameraCollisionResolver
			UCameraCollisionResolver* Resolver = Cast<UCameraCollisionResolver>(CurrentOuter);
			if (Resolver)
			{
				// 从 Resolver 获取 CameraManager
				USoulsCameraManager* Manager = Resolver->GetCameraManager();
				if (Manager)
				{
					OwnerActor = Manager->GetOwner();
					break;
				}
			}
			
			// 直接检查是否是 Actor
			AActor* AsActor = Cast<AActor>(CurrentOuter);
			if (AsActor)
			{
				OwnerActor = AsActor;
				break;
			}
			
			CurrentOuter = CurrentOuter->GetOuter();
		}
	}
	
	// 添加忽略的 Actor
	if (OwnerActor)
	{
		Params.AddIgnoredActor(OwnerActor);
		
		// ★ 诊断日志 ★
		static int32 IgnoreLogCount = 0;
		if (IgnoreLogCount < 3)
		{
			IgnoreLogCount++;
			UE_LOG(LogTemp, Warning, TEXT("GetDefaultTraceParams: ✓ 忽略 Actor: %s"), *OwnerActor->GetName());
		}
	}
	else
	{
		// ★ 警告：没有找到要忽略的 Actor ★
		static int32 WarnLogCount = 0;
		if (WarnLogCount < 3)
		{
			WarnLogCount++;
			UE_LOG(LogTemp, Error, TEXT("GetDefaultTraceParams: ⚠️ 未找到 OwnerActor！碰撞可能会检测到角色自己！"));
		}
	}

	return Params;
}

ECollisionChannel UCameraCollisionBase::GetCameraCollisionChannel() const
{
	return TraceChannel;
}


//========================================
// Debug
//========================================

void UCameraCollisionBase::DrawDebug(const UWorld* World, float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (!World)
	{
		return;
	}

	// Base implementation draws nothing
	// Derived classes should override to draw specific debug info
#endif
}

void UCameraCollisionBase::SetOwnerCameraManager(USoulsCameraManager* InCameraManager)
{
	OwnerCameraManager = InCameraManager;
}
