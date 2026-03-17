// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "TargetDetectionComponent.h"
#include "SoulsCameraDebugComponent.generated.h"

// Forward declarations
class USoulsCameraManager;

/**
 * FCameraDebugFlags
 * Configuration flags for controlling debug visualization
 * Each flag enables a specific category of debug information
 */
USTRUCT(BlueprintType)
struct SOUL_API FCameraDebugFlags
{
	GENERATED_BODY()

	/** Show focus point visualization (sphere at focus location) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowFocusPoint = false;

	/** Show camera path line (from focus to camera) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowCameraPath = false;

	/** Show collision detection probes (rays/spheres) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowCollisionProbes = false;

	/** Show target information (lock-on target info) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowTargetInfo = false;

	/** Show Framing Anchor debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowFramingDebug = false;

	/** Show lock-on detection cone visualization (sector/range/candidates) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowLockOnDetection = false;

	/** Show state information (current state, transitions) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	bool bShowStateInfo = false;

	/** Show active modules information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	bool bShowModuleInfo = false;

	/** Show active modifiers information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	bool bShowModifierInfo = false;

	/** Show performance statistics (frame time, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	bool bShowPerformanceStats = false;

	/** Show on-screen text debug info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bShowOnScreen = false;

	/** Constructor to enable all flags */
	static FCameraDebugFlags EnableAll()
	{
		FCameraDebugFlags Flags;
		Flags.bShowFocusPoint = true;
		Flags.bShowCameraPath = true;
		Flags.bShowCollisionProbes = true;
		Flags.bShowTargetInfo = true;
		Flags.bShowFramingDebug = true;
		Flags.bShowLockOnDetection = true;
		Flags.bShowStateInfo = true;
		Flags.bShowModuleInfo = true;
		Flags.bShowModifierInfo = true;
		Flags.bShowPerformanceStats = true;
		Flags.bShowOnScreen = true;
		return Flags;
	}

	/** Check if any visualization is enabled */
	bool HasAnyEnabled() const
	{
		return bShowFocusPoint || bShowCameraPath || bShowCollisionProbes ||
		       bShowTargetInfo || bShowFramingDebug || bShowLockOnDetection || bShowStateInfo || bShowModuleInfo ||
		       bShowModifierInfo || bShowPerformanceStats || bShowOnScreen;
	}

	/** Check if any world visualization is enabled */
	bool HasWorldVisualization() const
	{
		return bShowFocusPoint || bShowCameraPath || bShowCollisionProbes || bShowTargetInfo || bShowFramingDebug || bShowLockOnDetection;
	}

	/** Check if any screen text is enabled */
	bool HasScreenText() const
	{
		return bShowOnScreen && (bShowStateInfo || bShowModuleInfo || 
		                         bShowModifierInfo || bShowPerformanceStats);
	}
};

/**
 * USoulsCameraDebugComponent
 * 
 * Debug visualization component for the Souls-like camera system.
 * Provides real-time visual feedback for camera state, modules, modifiers,
 * collision, and performance metrics.
 * 
 * Usage:
 * - Add to any Actor that has USoulsCameraManager
 * - Configure DebugFlags to enable desired visualizations
 * - Toggle with SetDebugEnabled()
 * 
 * Features:
 * - Focus point visualization
 * - Camera path rendering
 * - Collision probe display
 * - On-screen state/module/modifier info
 * - Performance statistics
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent), DisplayName="Souls Camera Debug")
class SOUL_API USoulsCameraDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USoulsCameraDebugComponent();

	//========================================
	// UActorComponent Interface
	//========================================

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//========================================
	// Debug Control
	//========================================

	/**
	 * Enable or disable debug visualization
	 * @param bEnabled True to enable, false to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug")
	void SetDebugEnabled(bool bEnabled);

	/**
	 * Check if debug visualization is enabled
	 * @return True if debug is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	bool IsDebugEnabled() const;

	/**
	 * Set debug visualization flags
	 * @param Flags New debug flags configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera|Debug")
	void SetDebugFlags(const FCameraDebugFlags& Flags);

	/**
	 * Get current debug visualization flags
	 * @return Current debug flags
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug")
	FCameraDebugFlags GetDebugFlags() const;

	//========================================
	// Debug String Generation
	//========================================

	/**
	 * Get complete debug information string
	 * @return Formatted debug string with all enabled info
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug|Info")
	FString GetDebugString() const;

	/**
	 * Get camera state debug information
	 * @return Formatted string with state info
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug|Info")
	FString GetStateDebugString() const;

	/**
	 * Get active modules debug information
	 * @return Formatted string with module info
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug|Info")
	FString GetModulesDebugString() const;

	/**
	 * Get active modifiers debug information
	 * @return Formatted string with modifier info
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug|Info")
	FString GetModifiersDebugString() const;

	/**
	 * Get collision system debug information
	 * @return Formatted string with collision info
	 */
	UFUNCTION(BlueprintPure, Category = "Camera|Debug|Info")
	FString GetCollisionDebugString() const;

protected:
	//========================================
	// Debug Visualization Methods
	//========================================

	/**
	 * Main debug visualization entry point
	 * @param DeltaTime Frame delta time
	 */
	virtual void DrawDebugVisualization(float DeltaTime);

	/**
	 * Draw focus point visualization
	 * Renders a sphere at the current focus point location
	 */
	virtual void DrawFocusPoint();

	/**
	 * Draw camera path visualization
	 * Renders a line from focus point to camera location
	 */
	virtual void DrawCameraPath();

	/**
	 * Draw collision probe visualization
	 * Renders collision detection rays and sweep spheres
	 */
	virtual void DrawCollisionProbes();

	/**
	 * Draw target information visualization
	 * Renders lock-on target indicators and connection lines
	 */
	virtual void DrawTargetInfo();

	/** Draw Framing Anchor debug visualization */
	virtual void DrawFramingDebug();

	/** Draw lock-on detection cone, range circle, and candidate markers */
	virtual void DrawLockOnDetection();

	/**
	 * Draw on-screen text debug information
	 * Renders debug text to the screen
	 */
	virtual void DrawOnScreenDebug();

	//========================================
	// Configuration Properties
	//========================================

	/** Master enable for debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Enable")
	bool bDebugEnabled = false;

	/** Debug visualization flags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Flags")
	FCameraDebugFlags DebugFlags;

	//========================================
	// Color Configuration
	//========================================

	/** Color for focus point visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor FocusPointColor = FColor::Yellow;

	/** Color for camera path line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor CameraPathColor = FColor::Cyan;

	/** Color for collision visualization (hit/blocked) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor CollisionColor = FColor::Red;

	/** Color for target visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor TargetColor = FColor::Orange;

	/** Color for Framing weighted center visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor FramingCenterColor = FColor(50, 100, 255);

	/** Color for Framing Anchor information visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor FramingAnchorColor = FColor(0, 255, 100);

	/** Color for sector cone (inner priority zone) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor SectorConeColor = FColor(0, 255, 0, 128);

	/** Color for detection cone (outer full zone) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor DetectionConeColor = FColor(255, 255, 0, 80);

	/** Color for detection range circle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor DetectionRangeColor = FColor(255, 0, 0, 60);

	/** Color for safe/clear areas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor SafeColor = FColor::Green;

	/** Color for warning states */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor WarningColor = FColor::Yellow;

	/** Color for error states */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Colors")
	FColor ErrorColor = FColor::Red;

	//========================================
	// Visualization Size Configuration
	//========================================

	/** Size of the focus point sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Size", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float FocusPointSize = 10.0f;

	/** Thickness of debug lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Size", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float LineThickness = 2.0f;

	/** Size of target indicator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Size", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float TargetIndicatorSize = 50.0f;

	//========================================
	// Screen Text Configuration
	//========================================

	/** Screen position for debug text (X) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Screen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScreenTextPositionX = 0.02f;

	/** Screen position for debug text (Y) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Screen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScreenTextPositionY = 0.1f;

	/** Text scale for on-screen debug */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Screen", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float ScreenTextScale = 1.0f;

	//========================================
	// Camera Manager Reference
	//========================================

	/** Reference to the camera manager being debugged */
	UPROPERTY(BlueprintReadOnly, Category = "Debug|Reference")
	USoulsCameraManager* CameraManager = nullptr;

	//========================================
	// Performance Tracking
	//========================================

	/** Last frame's delta time */
	float LastDeltaTime;

	/** Average frame time (smoothed) */
	float AverageFrameTime;

	/** Peak frame time in recent history */
	float PeakFrameTime;

	/** Frame counter for averaging */
	int32 FrameCount;

	/** Time since last performance reset */
	float TimeSincePerformanceReset;

	/** Performance sample interval (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float PerformanceSampleInterval = 1.0f;

	//========================================
	// History Tracking
	//========================================

	/** History of focus points for path visualization */
	TArray<FVector> FocusPointHistory;

	/** Maximum history size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|History", meta = (ClampMin = "10", ClampMax = "500"))
	int32 MaxHistorySize = 100;

	/** Time between history samples */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|History", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float HistorySampleInterval = 0.05f;

	/** Time since last history sample */
	float TimeSinceLastHistorySample;

	//========================================
	// Internal Methods
	//========================================

	/** Find and cache the camera manager reference */
	void FindCameraManager();

	/** Update performance tracking metrics */
	void UpdatePerformanceMetrics(float DeltaTime);

	/** Update focus point history */
	void UpdateHistory(float DeltaTime);

	/** Reset performance tracking data */
	void ResetPerformanceData();

	/** Get formatted performance string */
	FString GetPerformanceString() const;
};
