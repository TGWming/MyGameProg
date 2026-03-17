// SoulsCameraTestActor.cpp
// Test Actor for validating the Souls-like Camera System

#include "Camera/Test/SoulsCameraTestActor.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Modules/CameraModuleRegistry.h"
#include "Camera/Core/CameraModifierManager.h"
#include "Camera/Collision/CameraCollisionResolver.h"
#include "Camera/Utility/SoulsCameraBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"

ASoulsCameraTestActor::ASoulsCameraTestActor()
    : bAutoFindCameraManager(true)
    , bValidateOnBeginPlay(true)
    , bShowDebugDisplay(false)
    , CameraManager(nullptr)
    , bTestCombatMode(false)
    , bValidationReportDirty(true)
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASoulsCameraTestActor::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoFindCameraManager)
    {
        CameraManager = FindCameraManager();

        if (CameraManager)
        {
            UE_LOG(LogTemp, Log, TEXT("SoulsCameraTestActor: Found camera manager"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SoulsCameraTestActor: No camera manager found"));
        }
    }

    if (bValidateOnBeginPlay && CameraManager)
    {
        ValidateSystemInitialization();
    }
}

void ASoulsCameraTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bShowDebugDisplay && CameraManager && GEngine)
    {
        FString DebugStr = GetCameraDebugString();
        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, DebugStr);
    }
}

USoulsCameraManager* ASoulsCameraTestActor::FindCameraManager()
{
    // First try blueprint library method
    USoulsCameraManager* Manager = USoulsCameraBlueprintLibrary::GetSoulsCameraManager(this, 0);
    if (Manager)
    {
        return Manager;
    }

    // Try to get from player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter)
    {
        Manager = PlayerCharacter->FindComponentByClass<USoulsCameraManager>();
        if (Manager)
        {
            return Manager;
        }
    }

    return nullptr;
}

// ============================================================================
// Test Functions
// ============================================================================

void ASoulsCameraTestActor::RunAllTests()
{
    TestResults.Empty();
    bValidationReportDirty = true;

    UE_LOG(LogTemp, Log, TEXT("========================================"));
    UE_LOG(LogTemp, Log, TEXT("Running All Camera System Tests"));
    UE_LOG(LogTemp, Log, TEXT("========================================"));

    // Validation tests
    ValidateSystemInitialization();
    ValidateModuleCount();
    ValidateModifierCount();
    ValidateCollisionCount();

    // Functional tests
    TestStateTransitions();
    TestModifiers();
    TestTargetLock();

    UE_LOG(LogTemp, Log, TEXT("========================================"));
    UE_LOG(LogTemp, Log, TEXT("Test Results: %d tests completed"), TestResults.Num());
    UE_LOG(LogTemp, Log, TEXT("========================================"));

    BuildValidationReport();
}

void ASoulsCameraTestActor::TestStateTransitions()
{
    if (!CameraManager)
    {
        LogTestResult(TEXT("StateTransitions"), false, TEXT("No camera manager"));
        return;
    }

    // Test state transitions using FName
    TArray<FName> StatesToTest = {
        FName("Exploration"),
        FName("Combat"),
        FName("Exploration")
    };

    bool bAllPassed = true;
    for (const FName& StateName : StatesToTest)
    {
        bool bSuccess = CameraManager->RequestStateChange(StateName, false);
        if (!bSuccess)
        {
            bAllPassed = false;
            UE_LOG(LogTemp, Warning, TEXT("Failed to transition to state: %s"), *StateName.ToString());
        }
    }

    LogTestResult(TEXT("StateTransitions"), bAllPassed, 
        bAllPassed ? TEXT("All transitions successful") : TEXT("Some transitions failed"));
}

void ASoulsCameraTestActor::TestModifiers()
{
    if (!CameraManager)
    {
        LogTestResult(TEXT("Modifiers"), false, TEXT("No camera manager"));
        return;
    }

    UCameraModifierManager* ModifierManager = CameraManager->GetModifierManager();
    if (!ModifierManager)
    {
        LogTestResult(TEXT("Modifiers"), false, TEXT("No modifier manager"));
        return;
    }

    // Test triggering modifier using ECameraModifierID
    bool bTriggered = ModifierManager->TriggerModifierSimple(
        ECameraModifierID::Modifier_S01_Shake_Hit_Light, 1.0f);
    
    // Check if modifier is active
    bool bIsActive = ModifierManager->IsModifierActive(
        ECameraModifierID::Modifier_S01_Shake_Hit_Light);

    LogTestResult(TEXT("Modifiers"), bTriggered, 
        FString::Printf(TEXT("Triggered: %s, Active: %s"), 
            bTriggered ? TEXT("Yes") : TEXT("No"),
            bIsActive ? TEXT("Yes") : TEXT("No")));

    // Stop the modifier
    ModifierManager->StopModifier(ECameraModifierID::Modifier_S01_Shake_Hit_Light, true);
}

void ASoulsCameraTestActor::TestCollision()
{
    if (!CameraManager)
    {
        LogTestResult(TEXT("Collision"), false, TEXT("No camera manager"));
        return;
    }

    bool bCollisionActive = CameraManager->IsCollisionActive();
    
    LogTestResult(TEXT("Collision"), true, 
        FString::Printf(TEXT("Collision system check - Current state: %s"), 
            bCollisionActive ? TEXT("In Collision") : TEXT("Clear")));
}

void ASoulsCameraTestActor::TestTargetLock()
{
    if (!CameraManager)
    {
        LogTestResult(TEXT("TargetLock"), false, TEXT("No camera manager"));
        return;
    }

    // Set this actor as target
    CameraManager->SetLockOnTarget(this);
    bool bLocked = CameraManager->HasLockOnTarget();
    AActor* Target = CameraManager->GetLockOnTarget();
    bool bCorrectTarget = (Target == this);

    // Clear lock
    CameraManager->ClearLockOnTarget();
    bool bCleared = !CameraManager->HasLockOnTarget();

    bool bSuccess = bLocked && bCorrectTarget && bCleared;
    LogTestResult(TEXT("TargetLock"), bSuccess,
        FString::Printf(TEXT("Locked: %s, CorrectTarget: %s, Cleared: %s"), 
            bLocked ? TEXT("Yes") : TEXT("No"),
            bCorrectTarget ? TEXT("Yes") : TEXT("No"),
            bCleared ? TEXT("Yes") : TEXT("No")));
}

// ============================================================================
// Effect Trigger Functions
// ============================================================================

void ASoulsCameraTestActor::TriggerShakeTest(bool bHeavy)
{
    if (!CameraManager) return;
    
    ECameraModifierID ShakeID = bHeavy ? 
        ECameraModifierID::Modifier_S02_Shake_Hit_Heavy : 
        ECameraModifierID::Modifier_S01_Shake_Hit_Light;
    
    CameraManager->TriggerCameraShake(ShakeID, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Triggered %s shake"), bHeavy ? TEXT("heavy") : TEXT("light"));
}

void ASoulsCameraTestActor::TriggerSlowMotionTest(float Duration)
{
    if (!CameraManager) return;
    
    CameraManager->TriggerSlowMotion(Duration, 0.3f);
    UE_LOG(LogTemp, Log, TEXT("Triggered slow motion for %.2f seconds"), Duration);
}

void ASoulsCameraTestActor::TriggerHitStopTest()
{
    if (!CameraManager) return;
    
    CameraManager->TriggerHitStop(1.0f);
    UE_LOG(LogTemp, Log, TEXT("Triggered hit stop"));
}

void ASoulsCameraTestActor::TriggerHitReactionTest(bool bHeavyHit)
{
    if (!CameraManager) return;
    
    CameraManager->TriggerHitReaction(bHeavyHit, 1.0f, FVector::ForwardVector);
    UE_LOG(LogTemp, Log, TEXT("Triggered %s hit reaction"), bHeavyHit ? TEXT("heavy") : TEXT("light"));
}

void ASoulsCameraTestActor::TriggerLowHealthTest(float HealthPercent)
{
    if (!CameraManager) return;
    
    CameraManager->TriggerLowHealthEffect(HealthPercent);
    UE_LOG(LogTemp, Log, TEXT("Triggered low health effect at %.0f%% health"), HealthPercent * 100.0f);
}

// ============================================================================
// State Control Functions
// ============================================================================

void ASoulsCameraTestActor::ForceStateChange(FName NewStateName)
{
    if (!CameraManager) return;
    
    bool bSuccess = CameraManager->RequestStateChange(NewStateName, true);
    UE_LOG(LogTemp, Log, TEXT("Force state change to %s: %s"), 
        *NewStateName.ToString(), 
        bSuccess ? TEXT("Success") : TEXT("Failed"));
}

void ASoulsCameraTestActor::SetExplorationState()
{
    ForceStateChange(FName("Exploration"));
}

void ASoulsCameraTestActor::SetCombatState()
{
    ForceStateChange(FName("Combat"));
}

void ASoulsCameraTestActor::ToggleCombatMode()
{
    bTestCombatMode = !bTestCombatMode;
    ForceStateChange(bTestCombatMode ? FName("Combat") : FName("Exploration"));
}

void ASoulsCameraTestActor::SetAsLockOnTarget()
{
    if (!CameraManager) return;
    
    CameraManager->SetLockOnTarget(this);
    UE_LOG(LogTemp, Log, TEXT("Set this actor as lock-on target"));
}

void ASoulsCameraTestActor::ClearLockOn()
{
    if (!CameraManager) return;
    
    CameraManager->ClearLockOnTarget();
    UE_LOG(LogTemp, Log, TEXT("Cleared lock-on"));
}

// ============================================================================
// Validation Functions
// ============================================================================

bool ASoulsCameraTestActor::ValidateSystemInitialization()
{
    if (!CameraManager)
    {
        CameraManager = FindCameraManager();
    }

    bool bValid = (CameraManager != nullptr) && CameraManager->IsInitialized();
    LogTestResult(TEXT("SystemInit"), bValid, 
        bValid ? TEXT("Camera system initialized") : TEXT("Camera system NOT initialized"));
    
    bValidationReportDirty = true;
    return bValid;
}

bool ASoulsCameraTestActor::ValidateModuleCount()
{
    if (!CameraManager)
    {
        LogTestResult(TEXT("ModuleCount"), false, TEXT("No camera manager"));
        return false;
    }

    UCameraModuleRegistry* Registry = CameraManager->GetModuleRegistry();
    if (!Registry)
    {
        LogTestResult(TEXT("ModuleCount"), false, TEXT("No module registry"));
        return false;
    }

    int32 ModuleCount = Registry->GetModuleCount();
    bool bValid = (ModuleCount == 39);

    LogTestResult(TEXT("ModuleCount"), bValid,
        FString::Printf(TEXT("Expected 39, found %d"), ModuleCount));

    bValidationReportDirty = true;
    return bValid;
}

bool ASoulsCameraTestActor::ValidateModifierCount()
{
    if (!CameraManager)
    {
        LogTestResult(TEXT("ModifierCount"), false, TEXT("No camera manager"));
        return false;
    }

    UCameraModifierManager* ModifierManager = CameraManager->GetModifierManager();
    if (!ModifierManager)
    {
        LogTestResult(TEXT("ModifierCount"), false, TEXT("No modifier manager"));
        return false;
    }

    int32 ModifierCount = ModifierManager->GetModifierCount();
    bool bValid = (ModifierCount == 26);

    LogTestResult(TEXT("ModifierCount"), bValid,
        FString::Printf(TEXT("Expected 26, found %d"), ModifierCount));

    bValidationReportDirty = true;
    return bValid;
}

bool ASoulsCameraTestActor::ValidateCollisionCount()
{
    if (!CameraManager)
    {
        LogTestResult(TEXT("CollisionCount"), false, TEXT("No camera manager"));
        return false;
    }

    UCameraCollisionResolver* CollisionResolver = CameraManager->GetCollisionResolver();
    if (!CollisionResolver)
    {
        LogTestResult(TEXT("CollisionCount"), false, TEXT("No collision resolver"));
        return false;
    }

    int32 StrategyCount = CollisionResolver->GetStrategyCount();
    bool bValid = (StrategyCount == 20);

    LogTestResult(TEXT("CollisionCount"), bValid,
        FString::Printf(TEXT("Expected 20, found %d"), StrategyCount));

    bValidationReportDirty = true;
    return bValid;
}

// ============================================================================
// Report and Debug Functions
// ============================================================================

void ASoulsCameraTestActor::LogTestResult(const FString& TestName, bool bSuccess, const FString& Details)
{
    FString Result = FString::Printf(TEXT("[%s] %s: %s"),
        bSuccess ? TEXT("PASS") : TEXT("FAIL"),
        *TestName,
        *Details);

    TestResults.Add(Result);
    UE_LOG(LogTemp, Log, TEXT("%s"), *Result);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, bSuccess ? FColor::Green : FColor::Red, Result);
    }
}

void ASoulsCameraTestActor::BuildValidationReport()
{
    CachedValidationReport = TEXT("=== Camera System Validation Report ===\n\n");

    for (const FString& Result : TestResults)
    {
        CachedValidationReport += Result + TEXT("\n");
    }

    CachedValidationReport += TEXT("\n=== End Report ===");
    bValidationReportDirty = false;
}

FString ASoulsCameraTestActor::GetValidationReport() const
{
    if (bValidationReportDirty || CachedValidationReport.IsEmpty())
    {
        // Return temporary report with current test results
        FString TempReport = TEXT("=== Camera System Validation Report ===\n\n");
        for (const FString& Result : TestResults)
        {
            TempReport += Result + TEXT("\n");
        }
        TempReport += TEXT("\n=== End Report ===");
        return TempReport;
    }
    return CachedValidationReport;
}

void ASoulsCameraTestActor::PrintValidationReport()
{
    FString Report = GetValidationReport();
    UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, Report);
    }
}

void ASoulsCameraTestActor::SetDebugDisplayEnabled(bool bEnabled)
{
    bShowDebugDisplay = bEnabled;
}

FString ASoulsCameraTestActor::GetCameraDebugString() const
{
    if (!CameraManager)
    {
        return TEXT("No Camera Manager");
    }
    
    return CameraManager->GetDebugString();
}
