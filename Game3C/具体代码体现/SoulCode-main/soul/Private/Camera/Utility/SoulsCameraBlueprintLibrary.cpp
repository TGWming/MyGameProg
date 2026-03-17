// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Utility/SoulsCameraBlueprintLibrary.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Core/CameraModifierManager.h"
#include "Camera/Modifiers/CameraModifierBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

//========================================
// Camera Manager Access
//========================================

USoulsCameraManager* USoulsCameraBlueprintLibrary::GetSoulsCameraManager(const UObject* WorldContextObject, int32 PlayerIndex)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Get player controller for the specified player index
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerIndex > 0)
	{
		int32 CurrentIndex = 0;
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (CurrentIndex == PlayerIndex)
			{
				PlayerController = Iterator->Get();
				break;
			}
			CurrentIndex++;
		}
	}

	if (!PlayerController)
	{
		return nullptr;
	}

	// Get the controlled pawn
	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	// Find SoulsCameraManager component on the pawn
	return Pawn->FindComponentByClass<USoulsCameraManager>();
}

USoulsCameraManager* USoulsCameraBlueprintLibrary::GetSoulsCameraManagerFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	// First try to find it directly on the actor
	USoulsCameraManager* Manager = Actor->FindComponentByClass<USoulsCameraManager>();
	if (Manager)
	{
		return Manager;
	}

	// If not found, try to find it on the owner
	AActor* Owner = Actor->GetOwner();
	while (Owner)
	{
		Manager = Owner->FindComponentByClass<USoulsCameraManager>();
		if (Manager)
		{
			return Manager;
		}
		Owner = Owner->GetOwner();
	}

	// Try to get from instigator
	APawn* Instigator = Actor->GetInstigator();
	if (Instigator && Instigator != Actor)
	{
		Manager = Instigator->FindComponentByClass<USoulsCameraManager>();
		if (Manager)
		{
			return Manager;
		}
	}

	return nullptr;
}

//========================================
// State Control
//========================================

bool USoulsCameraBlueprintLibrary::RequestCameraStateChange(const UObject* WorldContextObject, FName NewStateName, bool bForce, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->RequestStateChange(NewStateName, bForce);
}

FName USoulsCameraBlueprintLibrary::GetCurrentCameraStateName(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return NAME_None;
	}

	return Manager->GetCurrentStateName();
}

ECameraStateCategory USoulsCameraBlueprintLibrary::GetCurrentCameraCategory(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return ECameraStateCategory::None;
	}

	return Manager->GetCurrentCategory();
}

bool USoulsCameraBlueprintLibrary::IsCameraInTransition(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->IsInTransition();
}

//========================================
// Target Lock
//========================================

void USoulsCameraBlueprintLibrary::SetCameraLockOnTarget(const UObject* WorldContextObject, AActor* Target, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->SetLockOnTarget(Target);
	}
}

void USoulsCameraBlueprintLibrary::ClearCameraLockOnTarget(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->ClearLockOnTarget();
	}
}

AActor* USoulsCameraBlueprintLibrary::GetCameraLockOnTarget(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return nullptr;
	}

	return Manager->GetLockOnTarget();
}

bool USoulsCameraBlueprintLibrary::HasCameraLockOnTarget(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->HasLockOnTarget();
}

//========================================
// Camera Effects
//========================================

void USoulsCameraBlueprintLibrary::TriggerCameraShakeByID(const UObject* WorldContextObject, ECameraModifierID ShakeID, float Intensity, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->TriggerCameraShake(ShakeID, Intensity);
	}
}

void USoulsCameraBlueprintLibrary::TriggerCameraHitReaction(const UObject* WorldContextObject, bool bHeavyHit, float Intensity, FVector HitDirection, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->TriggerHitReaction(bHeavyHit, Intensity, HitDirection);
	}
}

void USoulsCameraBlueprintLibrary::TriggerCameraSlowMotion(const UObject* WorldContextObject, float Duration, float TimeDilation, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->TriggerSlowMotion(Duration, TimeDilation);
	}
}

void USoulsCameraBlueprintLibrary::TriggerCameraHitStop(const UObject* WorldContextObject, float Intensity, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->TriggerHitStop(Intensity);
	}
}

void USoulsCameraBlueprintLibrary::TriggerCameraLowHealthEffect(const UObject* WorldContextObject, float HealthPercent, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->TriggerLowHealthEffect(HealthPercent);
	}
}

void USoulsCameraBlueprintLibrary::StopCameraLowHealthEffect(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->StopLowHealthEffect();
	}
}

void USoulsCameraBlueprintLibrary::TriggerCameraDeathEffect(const UObject* WorldContextObject, FVector DeathLocation, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->TriggerDeathCamera(DeathLocation);
	}
}

void USoulsCameraBlueprintLibrary::StopAllCameraModifiers(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->StopAllModifiers();
	}
}

//========================================
// Collision Query
//========================================

bool USoulsCameraBlueprintLibrary::IsCameraCollisionActive(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->IsCollisionActive();
}

bool USoulsCameraBlueprintLibrary::IsCameraInCollisionRecovery(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->IsInCollisionRecovery();
}

bool USoulsCameraBlueprintLibrary::IsCameraCharacterOccluded(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->IsCharacterOccluded();
}

bool USoulsCameraBlueprintLibrary::IsCameraTargetOccluded(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->IsTargetOccluded();
}

float USoulsCameraBlueprintLibrary::GetCameraCollisionAdjustedDistance(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return 0.0f;
	}

	return Manager->GetCollisionAdjustedDistance();
}

//========================================
// Output Query
//========================================

FSoulsCameraOutput USoulsCameraBlueprintLibrary::GetCameraCurrentOutput(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return FSoulsCameraOutput();
	}

	return Manager->GetCurrentOutput();
}

FVector USoulsCameraBlueprintLibrary::GetCameraLocation(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return FVector::ZeroVector;
	}

	FSoulsCameraOutput Output = Manager->GetCurrentOutput();
	return Output.GetCameraLocation();
}

FRotator USoulsCameraBlueprintLibrary::GetCameraRotation(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return FRotator::ZeroRotator;
	}

	FSoulsCameraOutput Output = Manager->GetCurrentOutput();
	return Output.Rotation;
}

float USoulsCameraBlueprintLibrary::GetCameraDistance(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return 0.0f;
	}

	FSoulsCameraOutput Output = Manager->GetCurrentOutput();
	return Output.Distance;
}

float USoulsCameraBlueprintLibrary::GetCameraFOV(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return 70.0f; // Default FOV
	}

	FSoulsCameraOutput Output = Manager->GetCurrentOutput();
	return Output.FOV;
}

//========================================
// Debug
//========================================

void USoulsCameraBlueprintLibrary::SetCameraDebugEnabled(const UObject* WorldContextObject, bool bEnabled, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (Manager)
	{
		Manager->SetDebugEnabled(bEnabled);
	}
}

bool USoulsCameraBlueprintLibrary::IsCameraDebugEnabled(const UObject* WorldContextObject, int32 PlayerIndex)
{
	USoulsCameraManager* Manager = GetSoulsCameraManager(WorldContextObject, PlayerIndex);
	if (!Manager)
	{
		return false;
	}

	return Manager->IsDebugEnabled();
}
