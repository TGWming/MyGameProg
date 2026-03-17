// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LockOnConfig.generated.h"

/**
 * Enemy size classification for adaptive UI and targeting
 */
UENUM(BlueprintType)
enum class EEnemySizeCategory : uint8
{
	Small UMETA(DisplayName = "Small"),
	Medium UMETA(DisplayName = "Medium"),
	Large UMETA(DisplayName = "Large")
};

/**
 * UI display mode for lock-on widget
 */
UENUM(BlueprintType)
enum class EUIDisplayMode : uint8
{
	Traditional3D UMETA(DisplayName = "Traditional 3D"),
	SocketProjection UMETA(DisplayName = "Socket Projection"),
	ScreenSpace UMETA(DisplayName = "Screen Space"),
	SizeAdaptive UMETA(DisplayName = "Size Adaptive")
};

/**
 * Projection mode for lock-on position
 */
UENUM(BlueprintType)
enum class EProjectionMode : uint8
{
	ActorCenter UMETA(DisplayName = "Actor Center"),
	BoundsCenter UMETA(DisplayName = "Bounds Center"),
	CustomOffset UMETA(DisplayName = "Custom Offset"),
	Hybrid UMETA(DisplayName = "Hybrid")
};

/**
 * Method used to determine lock position
 */
UENUM(BlueprintType)
enum class ELockMethod : uint8
{
	None UMETA(DisplayName = "None"),
	Socket UMETA(DisplayName = "Socket"),
	Capsule UMETA(DisplayName = "Capsule"),
	Root UMETA(DisplayName = "Root")
};

/**
 * Hybrid projection settings
 */
USTRUCT(BlueprintType)
struct FHybridProjectionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On")
	EProjectionMode ProjectionMode = EProjectionMode::Hybrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On")
	FVector CustomOffset = FVector(0.0f, 0.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BoundsOffsetRatio = 0.5f;
};

/**
 * Lock-on system settings
 */
USTRUCT(BlueprintType)
struct FLockOnSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float LockOnRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float LockOnAngle = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float SectorLockAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float EdgeDetectionAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float ExtendedLockRangeMultiplier = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switching", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float TargetSwitchCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float TargetSearchInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.5", ClampMax = "0.95"))
	float ThumbstickThreshold = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float RaycastHeightOffset = 50.0f;
};

/**
 * Socket projection settings for lock-on targeting
 */
USTRUCT(BlueprintType)
struct FSocketProjectionSettings
{
	GENERATED_BODY()

	/** Primary socket name to use for targeting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FName TargetSocketName = FName(TEXT("spine_02"));

	/** Additional offset applied to the socket location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FVector SocketOffset = FVector::ZeroVector;

	/** Whether to try alternative sockets if primary is not found */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bEnableSocketFallback = true;

	/** List of socket names to search through as fallbacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	TArray<FName> SocketSearchNames;
};

/**
 * Size-based UI configuration
 */
USTRUCT(BlueprintType)
struct FSizeUIConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float SmallEnemyUIScale = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float MediumEnemyUIScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float LargeEnemyUIScale = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FLinearColor SmallEnemyColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FLinearColor MediumEnemyColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FLinearColor LargeEnemyColor = FLinearColor::Red;
};
