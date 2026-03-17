// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Core/SoulsCameraGlobalConfig.h"

USoulsCameraGlobalConfig::USoulsCameraGlobalConfig()
{
	// GlobalParams uses its default constructor with sensible defaults
}

void USoulsCameraGlobalConfig::GetDistanceLimits(float& OutMin, float& OutMax) const
{
	OutMin = GlobalParams.Limits.GlobalMinDistance;
	OutMax = GlobalParams.Limits.GlobalMaxDistance;
}

void USoulsCameraGlobalConfig::GetFOVLimits(float& OutMin, float& OutMax) const
{
	OutMin = GlobalParams.Limits.GlobalMinFOV;
	OutMax = GlobalParams.Limits.GlobalMaxFOV;
}

void USoulsCameraGlobalConfig::GetDefaultBlendSettings(float& OutTime, ECameraBlendType& OutType) const
{
	OutTime = GlobalParams.Transition.DefaultBlendTime;
	OutType = GlobalParams.Transition.DefaultBlendType;
}

bool USoulsCameraGlobalConfig::IsDebugEnabledByDefault() const
{
	return GlobalParams.Debug.bEnableDebugByDefault;
}

float USoulsCameraGlobalConfig::GetMaxLockOnDistance() const
{
	return GlobalParams.LockOn.MaxLockOnDistance;
}

int32 USoulsCameraGlobalConfig::GetUpdateRate() const
{
	return GlobalParams.Performance.UpdateRate;
}

#if WITH_EDITOR
void USoulsCameraGlobalConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Validate distance limits
	if (GlobalParams.Limits.GlobalMinDistance >= GlobalParams.Limits.GlobalMaxDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraGlobalConfig: GlobalMinDistance should be less than GlobalMaxDistance"));
	}

	// Validate FOV limits
	if (GlobalParams.Limits.GlobalMinFOV >= GlobalParams.Limits.GlobalMaxFOV)
	{
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraGlobalConfig: GlobalMinFOV should be less than GlobalMaxFOV"));
	}

	// Validate blend time
	if (GlobalParams.Transition.DefaultBlendTime < 0.0f)
	{
		GlobalParams.Transition.DefaultBlendTime = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraGlobalConfig: DefaultBlendTime cannot be negative, clamped to 0"));
	}

	// Validate lock-on distance
	if (GlobalParams.LockOn.MaxLockOnDistance <= 0.0f)
	{
		GlobalParams.LockOn.MaxLockOnDistance = 2000.0f;
		UE_LOG(LogTemp, Warning, TEXT("USoulsCameraGlobalConfig: MaxLockOnDistance must be positive, reset to default"));
	}
}
#endif
