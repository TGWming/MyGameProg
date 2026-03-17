// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "SoulsCameraParams_Global.h"
#include "SoulsCameraGlobalConfig.generated.h"

/**
 * USoulsCameraGlobalConfig
 * 
 * Data asset containing global camera system settings.
 * Create instances of this in the editor via:
 * Content Browser -> Add New -> Miscellaneous -> Data Asset -> SoulsCameraGlobalConfig
 * 
 * This contains system-wide settings that apply across all camera states,
 * including limits, smoothing, transitions, debug, lock-on, and performance settings.
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API USoulsCameraGlobalConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	USoulsCameraGlobalConfig();

	//========================================
	// Global Parameters (E-Group, 22 params)
	//========================================

	/** All global camera system parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
	FGlobalParams GlobalParams;

	//========================================
	// DataTable Reference
	//========================================

	/** Reference to the camera states DataTable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data References")
	TSoftObjectPtr<UDataTable> StatesDataTable;

	//========================================
	// Helper Functions
	//========================================

	/** Get global distance limits */
	UFUNCTION(BlueprintPure, Category = "Camera|Global")
	void GetDistanceLimits(float& OutMin, float& OutMax) const;

	/** Get global FOV limits */
	UFUNCTION(BlueprintPure, Category = "Camera|Global")
	void GetFOVLimits(float& OutMin, float& OutMax) const;

	/** Get default blend settings */
	UFUNCTION(BlueprintPure, Category = "Camera|Global")
	void GetDefaultBlendSettings(float& OutTime, ECameraBlendType& OutType) const;

	/** Check if debug is enabled by default */
	UFUNCTION(BlueprintPure, Category = "Camera|Global")
	bool IsDebugEnabledByDefault() const;

	/** Get lock-on settings */
	UFUNCTION(BlueprintPure, Category = "Camera|Global")
	float GetMaxLockOnDistance() const;

	/** Get camera update rate (0 = every frame) */
	UFUNCTION(BlueprintPure, Category = "Camera|Global")
	int32 GetUpdateRate() const;

	//========================================
	// Validation
	//========================================

#if WITH_EDITOR
	/** Validate configuration in editor */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
