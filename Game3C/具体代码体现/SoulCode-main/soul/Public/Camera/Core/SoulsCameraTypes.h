// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SoulsCameraTypes.generated.h"

//========================================
// Forward Declarations
//========================================

// Core Camera Classes
class USoulsCameraManager;
class USoulsCameraPipeline;
class USoulsCameraStateMachine;

// Module System
class UCameraModuleBase;
class UCameraModuleRegistry;

// Modifier System
class UCameraModifierBase;
class UCameraModifierManager;

// Collision System
class UCameraCollisionResolver;

// UE4 Components
class USpringArmComponent;
class UCameraComponent;

//========================================
// Note: Enum Definitions
//========================================
// All camera system enumerations are defined in:
// Camera/Data/CameraStateEnums.h
//
// This includes:
// - ECameraStateCategory
// - ECameraStateSubCategory
// - ECameraStateReference
// - ECameraBlendType
// - ECameraModuleType
// - ECameraModifierID
// - ECollisionStrategyID
//
// This file only contains forward declarations for class types.

