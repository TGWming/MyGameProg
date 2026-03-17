// Fill out your copyright notice in the Description page of Project Settings.

#include "LockOnConfigComponent.h"

// Sets default values for this component's properties
ULockOnConfigComponent::ULockOnConfigComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void ULockOnConfigComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Validate configuration on begin play
	if (bOverrideOffset && !IsConfigValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LockOnConfigComponent on %s has invalid configuration!"), 
			*GetOwner()->GetName());
	}
}

bool ULockOnConfigComponent::IsConfigValid() const
{
	if (!GetOwner())
		return false;

	// If override is enabled and prefer socket is true, socket name must not be None
	if (bOverrideOffset && bPreferSocket)
	{
		return !CustomSocketName.IsNone();
	}

	return true;
}

FVector ULockOnConfigComponent::GetEffectiveOffset() const
{
	return bOverrideOffset ? CustomOffset : FVector::ZeroVector;
}

FName ULockOnConfigComponent::GetEffectiveSocketName() const
{
	if (bOverrideOffset && bPreferSocket && !CustomSocketName.IsNone())
	{
		return CustomSocketName;
	}
	return NAME_None;
}
