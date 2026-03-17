// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnConfig.h"
#include "Blueprint/UserWidget.h"
#include "UIManagerComponent.generated.h"

class USubTargetManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnWidgetShown, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockOnWidgetHidden);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSocketProjectionUpdated, AActor*, Target, FVector2D, ScreenPosition);

/**
 * UI manager component for lock-on system
 * Handles widget display and screen projection
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UUIManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== Properties ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> LockOnWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	EUIDisplayMode CurrentUIDisplayMode = EUIDisplayMode::ScreenSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FHybridProjectionSettings HybridProjectionSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bEnableSizeAdaptiveUI = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FSizeUIConfig SizeUIConfig;

	// ★ Debug LOG 开关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugSocketLookup = false;

	// ========== State ==========

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UUserWidget* LockOnWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AActor* CurrentLockOnTarget;

	// ========== Delegates ==========

	UPROPERTY(BlueprintAssignable, Category = "Lock-On Events")
	FOnLockOnWidgetShown OnLockOnWidgetShown;

	UPROPERTY(BlueprintAssignable, Category = "Lock-On Events")
	FOnLockOnWidgetHidden OnLockOnWidgetHidden;

	UPROPERTY(BlueprintAssignable, Category = "Lock-On Events")
	FOnSocketProjectionUpdated OnSocketProjectionUpdated;

	// ========== Public Functions ==========

	/**
	 * Show lock-on widget for target
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void ShowLockOnWidget(AActor* Target);

	/**
	 * Hide lock-on widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void HideLockOnWidget();

	/**
	 * Update lock-on widget for target change
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void UpdateLockOnWidget(AActor* NewTarget, AActor* OldTarget);

	/**
	 * Get target projection location in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	FVector GetTargetProjectionLocation(AActor* Target) const;

	/**
	 * Project world location to screen space
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	FVector2D ProjectToScreen(FVector WorldLocation) const;

	/**
	 * Update projection widget position
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void UpdateProjectionWidget(AActor* Target);

	/**
	 * Check if target is valid for UI display
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	bool IsValidTargetForUI(AActor* Target) const;

	/**
	 * Debug: List all sockets on target's mesh
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On|Debug")
	void DebugListTargetSockets(AActor* Target) const;

private:
	// ========== Private State ==========

	UPROPERTY()
	AActor* PreviousLockOnTarget;

	UPROPERTY()
	TArray<AActor*> TargetsWithActiveWidgets;

	float LastUIUpdateTime = 0.0f;

	const float UI_UPDATE_INTERVAL = 0.033f; // ~30 FPS for UI updates

	// ========== Private Functions ==========

	/**
	 * Get player controller
	 */
	APlayerController* GetPlayerController() const;

	/**
	 * Try to get socket location from target
	 */
	bool TryGetSocketLocation(AActor* Target, FName SocketName, FVector& OutLocation) const;

	/**
	 * Apply size-based UI settings
	 */
	void ApplySizeBasedUISettings(AActor* Target, EEnemySizeCategory SizeCategory);
};
