// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Modules/CameraModuleBase.h"
#include "Camera/Core/SoulsCameraManager.h"

UCameraModuleBase::UCameraModuleBase()
	: bIsActive(false)
	, bIsEnabled(true)
	, TimeSinceActivation(0.0f)
{
}

//========================================
// Module Identity
//========================================

// Default implementation returns the class name
FName UCameraModuleBase::GetModuleName() const
{
	return GetClass()->GetFName();
}

// Default implementation returns generic description
FString UCameraModuleBase::GetModuleDescription() const
{
	return FString::Printf(TEXT("Module: %s"), *GetModuleName().ToString());
}

//========================================
// Activation
//========================================

// Default implementation: activate if enabled
// Derived classes should override with specific activation conditions
bool UCameraModuleBase::ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const
{
	return bIsEnabled;
}

//========================================
// Lifecycle
//========================================

// Called when module becomes active
void UCameraModuleBase::OnActivate(const FStageExecutionContext& Context)
{
	bIsActive = true;
	TimeSinceActivation = 0.0f;
	UE_LOG(LogTemp, Verbose, TEXT("Module %s activated"), *GetModuleName().ToString());
}

// Called when module becomes inactive
void UCameraModuleBase::OnDeactivate(const FStageExecutionContext& Context)
{
	bIsActive = false;
	UE_LOG(LogTemp, Verbose, TEXT("Module %s deactivated"), *GetModuleName().ToString());
}

// Called each frame while active, updates activation timer
void UCameraModuleBase::OnUpdate(float DeltaTime, const FStageExecutionContext& Context)
{
	if (bIsActive)
	{
		TimeSinceActivation += DeltaTime;
	}
}

//========================================
// Helper Methods
//========================================

// Create pre-filled output with module identity
FModuleOutput UCameraModuleBase::CreateBaseOutput() const
{
	FModuleOutput Output;
	
	Output.Priority = GetDefaultPriority();
	Output.BlendPolicy = GetDefaultBlendPolicy();
	Output.Weight = 1.0f;
	
	// Map EModuleCategory to ECameraModuleCategory
	switch (GetModuleCategory())
	{
	case EModuleCategory::Position:
		Output.ModuleCategory = ECameraModuleCategory::CategoryPosition;
		break;
	case EModuleCategory::Rotation:
		Output.ModuleCategory = ECameraModuleCategory::CategoryRotation;
		break;
	case EModuleCategory::Distance:
		Output.ModuleCategory = ECameraModuleCategory::CategoryDistance;
		break;
	case EModuleCategory::FOV:
		Output.ModuleCategory = ECameraModuleCategory::CategoryFOV;
		break;
	case EModuleCategory::Offset:
		Output.ModuleCategory = ECameraModuleCategory::CategoryOffset;
		break;
	case EModuleCategory::Constraint:
		Output.ModuleCategory = ECameraModuleCategory::CategoryConstraint;
		break;
	default:
		Output.ModuleCategory = ECameraModuleCategory::CategoryNone;
		break;
	}
	
	return Output;
}

// Calculate frame-rate independent interpolation alpha using exponential decay
float UCameraModuleBase::GetInterpAlpha(float DeltaTime, float LagSpeed) const
{
	if (LagSpeed <= 0.0f)
	{
		return 1.0f; // Instant, no interpolation
	}
	
	return FMath::Clamp(1.0f - FMath::Exp(-LagSpeed * DeltaTime), 0.0f, 1.0f);
}

// Safely get owner actor from manager
AActor* UCameraModuleBase::GetOwnerActor(const FStageExecutionContext& Context) const
{
	if (Context.Manager)
	{
		return Context.Manager->GetOwner();
	}
	return nullptr;
}

// Safely get target actor from input context
AActor* UCameraModuleBase::GetTargetActor(const FStageExecutionContext& Context) const
{
	return Context.InputContext.GetTargetActor();
}
