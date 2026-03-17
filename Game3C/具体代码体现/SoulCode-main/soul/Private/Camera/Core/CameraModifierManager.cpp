// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Core/CameraModifierManager.h"
#include "Camera/Core/SoulsCameraManager.h"

// Shake Modifiers (Phase 4.2)
#include "Camera/Modifiers/CameraModifier_Shake.h"

// Reaction Modifiers (Phase 4.3)
#include "Camera/Modifiers/CameraModifier_Reaction.h"

// Cinematic Modifiers (Phase 4.4)
#include "Camera/Modifiers/CameraModifier_Cinematic.h"

// Zoom Modifiers (Phase 4.5)
#include "Camera/Modifiers/CameraModifier_Zoom.h"

// Effect Modifiers (Phase 4.5)
#include "Camera/Modifiers/CameraModifier_Effect.h"

// Special Modifiers (Phase 4.6)
#include "Camera/Modifiers/CameraModifier_Special.h"

UCameraModifierManager::UCameraModifierManager()
	: CameraManager(nullptr)
	, bIsInitialized(false)
{
}

//========================================
// Initialization
//========================================

void UCameraModifierManager::Initialize(USoulsCameraManager* InCameraManager)
{
	if (bIsInitialized)
	{
		return;
	}

	CameraManager = InCameraManager;
	
	// Create all modifier instances
	CreateAllModifiers(this);
	
	bIsInitialized = true;
}

//========================================
// Modifier Registration
//========================================

bool UCameraModifierManager::RegisterModifier(UCameraModifierBase* Modifier)
{
	if (!Modifier)
	{
		return false;
	}

	ECameraModifierID ModifierID = Modifier->GetModifierID();
	if (ModifierID == ECameraModifierID::None)
	{
		return false;
	}

	// Check if already registered
	if (RegisteredModifiers.Contains(ModifierID))
	{
		return false;
	}

	RegisteredModifiers.Add(ModifierID, Modifier);
	return true;
}

bool UCameraModifierManager::UnregisterModifier(ECameraModifierID ModifierID)
{
	if (ModifierID == ECameraModifierID::None)
	{
		return false;
	}

	return RegisteredModifiers.Remove(ModifierID) > 0;
}

UCameraModifierBase* UCameraModifierManager::GetModifier(ECameraModifierID ModifierID) const
{
	UCameraModifierBase* const* Found = RegisteredModifiers.Find(ModifierID);
	return Found ? *Found : nullptr;
}

TArray<UCameraModifierBase*> UCameraModifierManager::GetAllModifiers() const
{
	TArray<UCameraModifierBase*> Result;
	RegisteredModifiers.GenerateValueArray(Result);
	return Result;
}

TArray<UCameraModifierBase*> UCameraModifierManager::GetModifiersInCategory(EModifierCategory Category) const
{
	TArray<UCameraModifierBase*> Result;
	for (const auto& Pair : RegisteredModifiers)
	{
		if (Pair.Value && Pair.Value->GetModifierCategory() == Category)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

//========================================
// Modifier Triggering
//========================================

bool UCameraModifierManager::TriggerModifier(ECameraModifierID ModifierID, const FModifierTriggerData& TriggerData)
{
	UCameraModifierBase* Modifier = GetModifier(ModifierID);
	if (!Modifier)
	{
		return false;
	}

	if (!CanTriggerModifier(Modifier))
	{
		return false;
	}

	Modifier->Trigger(TriggerData);
	return true;
}

bool UCameraModifierManager::TriggerModifierSimple(ECameraModifierID ModifierID, float Intensity)
{
	return TriggerModifier(ModifierID, FModifierTriggerData::MakeSimple(Intensity));
}

bool UCameraModifierManager::TriggerModifierWithDuration(ECameraModifierID ModifierID, float Intensity, float Duration)
{
	return TriggerModifier(ModifierID, FModifierTriggerData::MakeWithDuration(Intensity, Duration));
}

void UCameraModifierManager::StopModifier(ECameraModifierID ModifierID, bool bImmediate)
{
	UCameraModifierBase* Modifier = GetModifier(ModifierID);
	if (Modifier)
	{
		Modifier->Stop(bImmediate);
	}
}

void UCameraModifierManager::StopAllModifiers(bool bImmediate)
{
	for (auto& Pair : RegisteredModifiers)
	{
		if (Pair.Value)
		{
			Pair.Value->Stop(bImmediate);
		}
	}
}

void UCameraModifierManager::StopModifiersInCategory(EModifierCategory Category, bool bImmediate)
{
	for (auto& Pair : RegisteredModifiers)
	{
		if (Pair.Value && Pair.Value->GetModifierCategory() == Category)
		{
			Pair.Value->Stop(bImmediate);
		}
	}
}

//========================================
// Update & Query
//========================================

void UCameraModifierManager::UpdateModifiers(float DeltaTime, const FStageExecutionContext& Context)
{
	// Update all registered modifiers
	for (auto& Pair : RegisteredModifiers)
	{
		if (Pair.Value)
		{
			Pair.Value->Update(DeltaTime, Context);
		}
	}

	// Refresh the active modifiers cache
	RefreshActiveModifiersList();
}

void UCameraModifierManager::GetActiveModifiers(TArray<UCameraModifierBase*>& OutActiveModifiers) const
{
	OutActiveModifiers = ActiveModifiersCache;
}

void UCameraModifierManager::GetActiveModifierOutputs(const FStageExecutionContext& Context, TArray<FModifierOutput>& OutOutputs)
{
	OutOutputs.Reset();
	OutOutputs.Reserve(ActiveModifiersCache.Num());

	for (UCameraModifierBase* Modifier : ActiveModifiersCache)
	{
		if (Modifier)
		{
			FModifierOutput Output;
			if (Modifier->GetOutput(Context, Output))
			{
				OutOutputs.Add(Output);
			}
		}
	}
}

bool UCameraModifierManager::HasActiveModifiers() const
{
	return ActiveModifiersCache.Num() > 0;
}

bool UCameraModifierManager::IsModifierActive(ECameraModifierID ModifierID) const
{
	UCameraModifierBase* Modifier = GetModifier(ModifierID);
	return Modifier && Modifier->IsActive();
}

int32 UCameraModifierManager::GetActiveModifierCount() const
{
	return ActiveModifiersCache.Num();
}

//========================================
// Convenience Trigger Functions
//========================================

void UCameraModifierManager::TriggerShake(ECameraModifierID ShakeID, float Intensity)
{
	TriggerModifierSimple(ShakeID, Intensity);
}

void UCameraModifierManager::TriggerHitReaction(bool bHeavyHit, float Intensity, FVector HitDirection)
{
	ECameraModifierID ModifierID = bHeavyHit 
		? ECameraModifierID::Modifier_S02_Shake_Hit_Heavy 
		: ECameraModifierID::Modifier_S01_Shake_Hit_Light;
	
	TriggerModifier(ModifierID, FModifierTriggerData::MakeDirectional(Intensity, FVector::ZeroVector, HitDirection));
}

void UCameraModifierManager::TriggerParryEffect(bool bPerfectParry)
{
	ECameraModifierID ModifierID = bPerfectParry 
		? ECameraModifierID::Modifier_R02_Reaction_PerfectParry 
		: ECameraModifierID::Modifier_R01_Reaction_Parry;
	
	TriggerModifierSimple(ModifierID, 1.0f);
}

void UCameraModifierManager::TriggerStaggerEffect(float Intensity)
{
	TriggerModifierSimple(ECameraModifierID::Modifier_R03_Reaction_Stagger, Intensity);
}

void UCameraModifierManager::TriggerKnockbackEffect(float Intensity, FVector Direction)
{
	FModifierTriggerData TriggerData = FModifierTriggerData::MakeDirectional(Intensity, FVector::ZeroVector, Direction);
	TriggerModifier(ECameraModifierID::Modifier_R04_Reaction_Knockback, TriggerData);
}

void UCameraModifierManager::TriggerKnockdownEffect(float Intensity)
{
	TriggerModifierSimple(ECameraModifierID::Modifier_R05_Reaction_Knockdown, Intensity);
}

void UCameraModifierManager::TriggerGuardBreakEffect(float Intensity, AActor* Attacker)
{
	FModifierTriggerData TriggerData = FModifierTriggerData::MakeSimple(Intensity);
	TriggerData.SourceActor = Attacker;
	TriggerModifier(ECameraModifierID::Modifier_R06_Reaction_GuardBreak, TriggerData);
}

void UCameraModifierManager::TriggerExecutionCamera(AActor* Victim, ECameraModifierID ExecutionID)
{
	FModifierTriggerData TriggerData = FModifierTriggerData::MakeSimple(1.0f);
	TriggerData.TargetActor = Victim;
	
	TriggerModifier(ExecutionID, TriggerData);
}

void UCameraModifierManager::TriggerSlowMotion(float Duration, float TimeDilation)
{
	FModifierTriggerData TriggerData = FModifierTriggerData::MakeWithDuration(1.0f, Duration);
	TriggerData.CustomFloat = TimeDilation;
	
	TriggerModifier(ECameraModifierID::Modifier_X01_Special_SlowMotion, TriggerData);
}

void UCameraModifierManager::TriggerHitStop(float Duration)
{
	TriggerModifierWithDuration(ECameraModifierID::Modifier_X02_Special_HitStop, 1.0f, Duration);
}

void UCameraModifierManager::TriggerLowHealthEffect(float HealthPercent)
{
	FModifierTriggerData TriggerData = FModifierTriggerData::MakeSimple(1.0f - HealthPercent);
	TriggerModifier(ECameraModifierID::Modifier_E01_Effect_LowHealth, TriggerData);
}

void UCameraModifierManager::TriggerBossIntro(AActor* Boss, float Duration)
{
	FModifierTriggerData TriggerData = FModifierTriggerData::MakeWithDuration(1.0f, Duration);
	TriggerData.TargetActor = Boss;
	
	TriggerModifier(ECameraModifierID::Modifier_C04_Cinematic_BossIntro, TriggerData);
}

//========================================
// Internal Methods
//========================================

void UCameraModifierManager::CreateAllModifiers(UObject* Outer)
{
	UE_LOG(LogTemp, Log, TEXT("CameraModifierManager: Creating all 26 modifiers..."));

	//========================================
	// Shake Modifiers (S01-S05) - 5 modifiers
	//========================================
	
	RegisterModifier(NewObject<UCameraModifier_S01_Shake_Hit_Light>(Outer));
	RegisterModifier(NewObject<UCameraModifier_S02_Shake_Hit_Heavy>(Outer));
	RegisterModifier(NewObject<UCameraModifier_S03_Shake_Attack_Hit>(Outer));
	RegisterModifier(NewObject<UCameraModifier_S04_Shake_Environment>(Outer));
	RegisterModifier(NewObject<UCameraModifier_S05_Shake_Landing>(Outer));

	//========================================
	// Reaction Modifiers (R01-R06) - 6 modifiers
	//========================================
	
	RegisterModifier(NewObject<UCameraModifier_R01_Reaction_Parry>(Outer));
	RegisterModifier(NewObject<UCameraModifier_R02_Reaction_PerfectParry>(Outer));
	RegisterModifier(NewObject<UCameraModifier_R03_Reaction_Stagger>(Outer));
	RegisterModifier(NewObject<UCameraModifier_R04_Reaction_Knockback>(Outer));
	RegisterModifier(NewObject<UCameraModifier_R05_Reaction_Knockdown>(Outer));
	RegisterModifier(NewObject<UCameraModifier_R06_Reaction_GuardBreak>(Outer));

	//========================================
	// Cinematic Modifiers (C01-C05) - 5 modifiers
	//========================================
	
	RegisterModifier(NewObject<UCameraModifier_C01_Cinematic_Execution>(Outer));
	RegisterModifier(NewObject<UCameraModifier_C02_Cinematic_Backstab>(Outer));
	RegisterModifier(NewObject<UCameraModifier_C03_Cinematic_Riposte>(Outer));
	RegisterModifier(NewObject<UCameraModifier_C04_Cinematic_BossIntro>(Outer));
	RegisterModifier(NewObject<UCameraModifier_C05_Cinematic_BossPhase>(Outer));

	//========================================
	// Zoom Modifiers (Z01-Z04) - 4 modifiers
	//========================================
	
	RegisterModifier(NewObject<UCameraModifier_Z01_Zoom_AttackImpact>(Outer));
	RegisterModifier(NewObject<UCameraModifier_Z02_Zoom_ChargeRelease>(Outer));
	RegisterModifier(NewObject<UCameraModifier_Z03_Zoom_SkillActivate>(Outer));
	RegisterModifier(NewObject<UCameraModifier_Z04_Zoom_CriticalHit>(Outer));

	//========================================
	// Effect Modifiers (E01-E03) - 3 modifiers
	//========================================
	
	RegisterModifier(NewObject<UCameraModifier_E01_Effect_LowHealth>(Outer));
	RegisterModifier(NewObject<UCameraModifier_E02_Effect_StatusAilment>(Outer));
	RegisterModifier(NewObject<UCameraModifier_E03_Effect_FocusMode>(Outer));

	//========================================
	// Special Modifiers (X01-X03) - 3 modifiers
	//========================================
	
	RegisterModifier(NewObject<UCameraModifier_X01_Special_SlowMotion>(Outer));
	RegisterModifier(NewObject<UCameraModifier_X02_Special_HitStop>(Outer));
	RegisterModifier(NewObject<UCameraModifier_X03_Special_DeathCam>(Outer));

	UE_LOG(LogTemp, Log, TEXT("CameraModifierManager: Successfully created all %d modifiers!"), 
		RegisteredModifiers.Num());
	
	// Verify all 26 modifiers registered
	if (RegisteredModifiers.Num() == 26)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraModifierManager: All 26 modifiers registered successfully!"));
		UE_LOG(LogTemp, Log, TEXT("  - Shake (S01-S05): 5"));
		UE_LOG(LogTemp, Log, TEXT("  - Reaction (R01-R06): 6"));
		UE_LOG(LogTemp, Log, TEXT("  - Cinematic (C01-C05): 5"));
		UE_LOG(LogTemp, Log, TEXT("  - Zoom (Z01-Z04): 4"));
		UE_LOG(LogTemp, Log, TEXT("  - Effect (E01-E03): 3"));
		UE_LOG(LogTemp, Log, TEXT("  - Special (X01-X03): 3"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraModifierManager: Expected 26 modifiers, got %d"), 
			RegisteredModifiers.Num());
	}
}

void UCameraModifierManager::RefreshActiveModifiersList()
{
	ActiveModifiersCache.Reset();
	
	for (const auto& Pair : RegisteredModifiers)
	{
		if (Pair.Value && Pair.Value->IsActive())
		{
			ActiveModifiersCache.Add(Pair.Value);
		}
	}

	// Sort by priority
	SortActiveModifiersByPriority();
}

void UCameraModifierManager::SortActiveModifiersByPriority()
{
	ActiveModifiersCache.Sort([](const UCameraModifierBase& A, const UCameraModifierBase& B)
	{
		return A.GetPriority() < B.GetPriority();
	});
}

bool UCameraModifierManager::CanTriggerModifier(UCameraModifierBase* Modifier) const
{
	if (!Modifier)
	{
		return false;
	}

	// If modifier can stack, always allow
	if (Modifier->CanStack())
	{
		return true;
	}

	// If modifier is already active and cannot be interrupted, deny
	if (Modifier->IsActive() && !Modifier->CanBeInterrupted())
	{
		return false;
	}

	return true;
}
