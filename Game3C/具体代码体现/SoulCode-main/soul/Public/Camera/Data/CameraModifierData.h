// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraModifierData.generated.h"

/**
 * Camera Modifier Row Structure
 * Camera modifier data table row structure
 * Used for importing camera modifier configuration data from CSV
 */
USTRUCT(BlueprintType)
struct FCameraModifierRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** Modifier unique identifier (this field will automatically become RowName, will be empty after import) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FString ModifierID;

	/** Modifier name - program reference identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FName ModifierName;

	/** Chinese display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
	FString ChineseName;

	/** Main category (Shake, Reaction, Cinematic, Zoom, Effect, Special) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Category;

	/** Sub category (Combat, Environment, Movement, Special, Boss, Status, Aim, Time, Death) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString SubCategory;

	/** Trigger event description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FString TriggerEvent;

	/** Duration (can be numeric value, range, or descriptive text) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	FString Duration;

	/** Blend in time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BlendInTime;

	/** Blend out time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BlendOutTime;

	/** Priority (higher value means higher priority, range 0-255) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority")
	int32 Priority;

	/** Whether affects position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affects")
	bool bAffectsPosition;

	/** Whether affects rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affects")
	bool bAffectsRotation;

	/** Whether affects FOV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affects")
	bool bAffectsFOV;

	/** Whether affects post process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affects")
	bool bAffectsPostProcess;

	/** Whether is core modifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core")
	bool bIsCore;

	/** Modifier function description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description")
	FString Description;

	/** Constructor - Initialize default values */
	FCameraModifierRow()
		: ModifierID(TEXT(""))
		, ModifierName(NAME_None)
		, ChineseName(TEXT(""))
		, Category(TEXT("Shake"))
		, SubCategory(TEXT("Combat"))
		, TriggerEvent(TEXT(""))
		, Duration(TEXT("0.0"))
		, BlendInTime(0.0f)
		, BlendOutTime(0.0f)
		, Priority(100)
		, bAffectsPosition(false)
		, bAffectsRotation(false)
		, bAffectsFOV(false)
		, bAffectsPostProcess(false)
		, bIsCore(false)
		, Description(TEXT(""))
	{
	}
};
