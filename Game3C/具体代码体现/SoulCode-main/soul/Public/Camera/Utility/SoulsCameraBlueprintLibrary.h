// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "SoulsCameraBlueprintLibrary.generated.h"

// Forward declarations
class USoulsCameraManager;

/**
 * USoulsCameraBlueprintLibrary
 * 
 * Blueprint function library for the Souls-like camera system.
 * Provides easy access to camera functionality from Blueprints.
 * 
 * Categories:
 * - Camera Manager Access: Get camera manager instances
 * - State Control: Request state changes, query state info
 * - Target Lock: Lock-on target management
 * - Camera Effects: Trigger camera modifiers (shake, hit reaction, etc.)
 * - Collision Query: Query collision state
 * - Output Query: Get current camera output values
 * - Debug: Debug visualization control
 */
UCLASS()
class SOUL_API USoulsCameraBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//========================================
	// Camera Manager Access
	//========================================

	/**
	 * Get the SoulsCameraManager for a specific player
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Camera manager instance, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Access", meta = (WorldContext = "WorldContextObject"))
	static USoulsCameraManager* GetSoulsCameraManager(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get the SoulsCameraManager from an actor (finds owner's camera manager)
	 * @param Actor Actor to search from
	 * @return Camera manager instance, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Access")
	static USoulsCameraManager* GetSoulsCameraManagerFromActor(AActor* Actor);

	//========================================
	// State Control
	//========================================

	/**
	 * Request a camera state change
	 * @param WorldContextObject World context
	 * @param NewStateName Name of the state to transition to
	 * @param bForce Force the transition even if conditions aren't met
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if state change was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|State", meta = (WorldContext = "WorldContextObject"))
	static bool RequestCameraStateChange(const UObject* WorldContextObject, FName NewStateName, bool bForce = false, int32 PlayerIndex = 0);

	/**
	 * Get the current camera state name
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Current state name
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|State", meta = (WorldContext = "WorldContextObject"))
	static FName GetCurrentCameraStateName(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get the current camera state category
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Current state category
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|State", meta = (WorldContext = "WorldContextObject"))
	static ECameraStateCategory GetCurrentCameraCategory(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Check if camera is in transition between states
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if in transition
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|State", meta = (WorldContext = "WorldContextObject"))
	static bool IsCameraInTransition(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	//========================================
	// Target Lock
	//========================================

	/**
	 * Set the camera lock-on target
	 * @param WorldContextObject World context
	 * @param Target Target actor to lock on to
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Target", meta = (WorldContext = "WorldContextObject"))
	static void SetCameraLockOnTarget(const UObject* WorldContextObject, AActor* Target, int32 PlayerIndex = 0);

	/**
	 * Clear the camera lock-on target
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Target", meta = (WorldContext = "WorldContextObject"))
	static void ClearCameraLockOnTarget(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get the current lock-on target
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Current lock-on target, or nullptr if none
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Target", meta = (WorldContext = "WorldContextObject"))
	static AActor* GetCameraLockOnTarget(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Check if camera has a lock-on target
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if has lock-on target
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Target", meta = (WorldContext = "WorldContextObject"))
	static bool HasCameraLockOnTarget(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	//========================================
	// Camera Effects
	//========================================

	/**
	 * Trigger a camera shake effect by modifier ID
	 * @param WorldContextObject World context
	 * @param ShakeID Shake modifier ID (S01-S05)
	 * @param Intensity Shake intensity (default 1.0)
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void TriggerCameraShakeByID(const UObject* WorldContextObject, ECameraModifierID ShakeID, float Intensity = 1.0f, int32 PlayerIndex = 0);

	/**
	 * Trigger a hit reaction camera effect
	 * @param WorldContextObject World context
	 * @param bHeavyHit True for heavy hit, false for light hit
	 * @param Intensity Reaction intensity (default 1.0)
	 * @param HitDirection Direction of the hit
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void TriggerCameraHitReaction(const UObject* WorldContextObject, bool bHeavyHit, float Intensity = 1.0f, FVector HitDirection = FVector::ZeroVector, int32 PlayerIndex = 0);

	/**
	 * Trigger slow motion effect
	 * @param WorldContextObject World context
	 * @param Duration Duration in seconds
	 * @param TimeDilation Time scale (0.3 = 30% speed)
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void TriggerCameraSlowMotion(const UObject* WorldContextObject, float Duration = 0.8f, float TimeDilation = 0.35f, int32 PlayerIndex = 0);

	/**
	 * Trigger hit stop (freeze frame) effect
	 * @param WorldContextObject World context
	 * @param Intensity Hit intensity (affects freeze duration)
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void TriggerCameraHitStop(const UObject* WorldContextObject, float Intensity = 1.0f, int32 PlayerIndex = 0);

	/**
	 * Trigger low health visual effect
	 * @param WorldContextObject World context
	 * @param HealthPercent Current health percentage (0-1)
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void TriggerCameraLowHealthEffect(const UObject* WorldContextObject, float HealthPercent, int32 PlayerIndex = 0);

	/**
	 * Stop low health effect (when health is restored)
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void StopCameraLowHealthEffect(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Trigger death camera effect
	 * @param WorldContextObject World context
	 * @param DeathLocation Location where player died
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void TriggerCameraDeathEffect(const UObject* WorldContextObject, FVector DeathLocation, int32 PlayerIndex = 0);

	/**
	 * Stop all active camera modifiers
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Effects", meta = (WorldContext = "WorldContextObject"))
	static void StopAllCameraModifiers(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	//========================================
	// Collision Query
	//========================================

	/**
	 * Check if camera collision is currently active
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if collision is active
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Collision", meta = (WorldContext = "WorldContextObject"))
	static bool IsCameraCollisionActive(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Check if camera is in collision recovery mode
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if in recovery
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Collision", meta = (WorldContext = "WorldContextObject"))
	static bool IsCameraInCollisionRecovery(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Check if character is occluded from camera view
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if character is occluded
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Collision", meta = (WorldContext = "WorldContextObject"))
	static bool IsCameraCharacterOccluded(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Check if lock-on target is occluded from camera view
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if target is occluded
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Collision", meta = (WorldContext = "WorldContextObject"))
	static bool IsCameraTargetOccluded(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get current collision-adjusted camera distance
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Current safe distance (adjusted for collision)
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Collision", meta = (WorldContext = "WorldContextObject"))
	static float GetCameraCollisionAdjustedDistance(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	//========================================
	// Output Query
	//========================================

	/**
	 * Get current camera output struct
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Current camera output
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Output", meta = (WorldContext = "WorldContextObject"))
	static FSoulsCameraOutput GetCameraCurrentOutput(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get current camera world location
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Camera world location
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Output", meta = (WorldContext = "WorldContextObject"))
	static FVector GetCameraLocation(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get current camera rotation
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Camera rotation
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Output", meta = (WorldContext = "WorldContextObject"))
	static FRotator GetCameraRotation(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get current camera distance from focus point
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Camera distance
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Output", meta = (WorldContext = "WorldContextObject"))
	static float GetCameraDistance(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Get current camera field of view
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return Camera FOV
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Output", meta = (WorldContext = "WorldContextObject"))
	static float GetCameraFOV(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	//========================================
	// Debug
	//========================================

	/**
	 * Enable or disable camera debug visualization
	 * @param WorldContextObject World context
	 * @param bEnabled Whether to enable debug visualization
	 * @param PlayerIndex Player index (0 for single player)
	 */
	UFUNCTION(BlueprintCallable, Category = "Souls Camera|Debug", meta = (WorldContext = "WorldContextObject"))
	static void SetCameraDebugEnabled(const UObject* WorldContextObject, bool bEnabled, int32 PlayerIndex = 0);

	/**
	 * Check if camera debug visualization is enabled
	 * @param WorldContextObject World context
	 * @param PlayerIndex Player index (0 for single player)
	 * @return True if debug is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Souls Camera|Debug", meta = (WorldContext = "WorldContextObject"))
	static bool IsCameraDebugEnabled(const UObject* WorldContextObject, int32 PlayerIndex = 0);
};
