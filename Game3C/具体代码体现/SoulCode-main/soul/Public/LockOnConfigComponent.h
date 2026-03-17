// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnConfigComponent.generated.h"

/**
 * Lock-on configuration component
 * Attach to enemy actors to customize lock-on behavior
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API ULockOnConfigComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULockOnConfigComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// ========== Properties ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On Config")
	bool bOverrideOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On Config", meta = (EditCondition = "bOverrideOffset"))
	FVector CustomOffset = FVector(0.0f, 0.0f, -50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On Config", meta = (EditCondition = "bOverrideOffset"))
	bool bPreferSocket = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock-On Config", meta = (EditCondition = "bOverrideOffset && bPreferSocket"))
	FName CustomSocketName = FName("LockOnSocket");

	// ========== Public Functions ==========

	/**
	 * Check if configuration is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	bool IsConfigValid() const;

	/**
	 * Get effective offset (returns CustomOffset if override is enabled, else ZeroVector)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	FVector GetEffectiveOffset() const;

	/**
	 * Get effective socket name (returns CustomSocketName if valid, else NAME_None)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	FName GetEffectiveSocketName() const;
};
