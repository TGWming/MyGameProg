// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Pipeline/CameraStage_InputGather.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "SubTargetManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraStage_Input, Log, All);

//========================================
// Constructor 构造函数
//========================================

UCameraStage_InputGather::UCameraStage_InputGather()
    : MovingSpeedThreshold(10.0f)
    , InputInactiveThreshold(0.5f)
    , InputDeadzone(0.1f)
    , TimeSinceLastInput(0.0f)
    , PreviousInput(FVector2D::ZeroVector)
    , PreviousCameraLocation(FVector::ZeroVector)
    , PreviousCameraRotation(FRotator::ZeroRotator)
    , bFirstFrame(true)
{
}

//========================================
// ICameraStage Interface 接口实现
//========================================

EStageResult UCameraStage_InputGather::Execute(FStageExecutionContext& Context)
{
    if (!Context.Manager)
    {
        UE_LOG(LogCameraStage_Input, Warning, TEXT("Execute: No manager reference"));
        return EStageResult::Failed;
    }

    // Store previous frame data first (before gathering new data)
    StorePreviousFrameData(Context);

    // Gather all input data in order
    GatherPlayerInput(Context);
    GatherCharacterState(Context);
    GatherTargetInfo(Context);
    GatherEnvironmentState(Context);

    // Set timing info
    Context.InputContext.DeltaTime = Context.DeltaTime;
    
    if (UWorld* World = Context.Manager->GetWorld())
    {
        Context.InputContext.GameTime = World->GetTimeSeconds();
    }

    // Clear first frame flag
    bFirstFrame = false;

    return EStageResult::Success;
}

void UCameraStage_InputGather::OnPreExecute(const FStageExecutionContext& Context)
{
    // Debug logging if needed
    UE_LOG(LogCameraStage_Input, VeryVerbose, TEXT("Stage 1 InputGather: PreExecute"));
}

void UCameraStage_InputGather::OnPostExecute(const FStageExecutionContext& Context, EStageResult Result)
{
    // Debug logging if needed
    UE_LOG(LogCameraStage_Input, VeryVerbose, TEXT("Stage 1 InputGather: PostExecute - Result: %d"), 
        static_cast<int32>(Result));
}

//========================================
// Input Gathering Methods 输入收集方法
//========================================

void UCameraStage_InputGather::GatherPlayerInput(FStageExecutionContext& Context)
{
    APlayerController* PC = GetPlayerController(Context);
    if (!PC)
    {
        Context.InputContext.CameraInput = FVector2D::ZeroVector;
        Context.InputContext.InputMagnitude = 0.0f;
        TimeSinceLastInput += Context.DeltaTime;
        Context.InputContext.TimeSinceLastInput = TimeSinceLastInput;
        return;
    }

    // ★★★ 使用鼠标增量代替输入轴（避免未绑定警告）★★★
    // 原来的 GetInputAxisValue("Turn") 等会在轴未绑定时产生警告
    float MouseX = 0.0f;
    float MouseY = 0.0f;
    PC->GetInputMouseDelta(MouseX, MouseY);
    
    // 鼠标增量作为主要输入
    float YawInput = MouseX;
    float PitchInput = -MouseY;  // 注意：鼠标Y轴通常需要反转
    
    // ★ 如果鼠标没有输入，尝试从手柄获取（使用安全方式）★
    if (FMath::IsNearlyZero(YawInput) && FMath::IsNearlyZero(PitchInput))
    {
        // 检查输入组件是否存在
        if (PC->InputComponent)
        {
            // 使用 GetInputAnalogKeyState 代替 GetInputAxisValue
            // 这不会产生"轴未绑定"的警告
            YawInput = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);
            PitchInput = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightY);
        }
    }

    // Apply deadzone
    FVector2D RawInput(YawInput, PitchInput);
    const float RawMagnitude = RawInput.Size();
    
    if (RawMagnitude < InputDeadzone)
    {
        Context.InputContext.CameraInput = FVector2D::ZeroVector;
        Context.InputContext.InputMagnitude = 0.0f;
    }
    else
    {
        // Remap input to remove deadzone
        const float RemappedMagnitude = (RawMagnitude - InputDeadzone) / (1.0f - InputDeadzone);
        Context.InputContext.CameraInput = RawInput.GetSafeNormal() * RemappedMagnitude;
        Context.InputContext.InputMagnitude = RemappedMagnitude;
    }

    // Track time since last input
    if (Context.InputContext.InputMagnitude > KINDA_SMALL_NUMBER)
    {
        TimeSinceLastInput = 0.0f;
        PreviousInput = Context.InputContext.CameraInput;
    }
    else
    {
        TimeSinceLastInput += Context.DeltaTime;
    }

    Context.InputContext.TimeSinceLastInput = TimeSinceLastInput;
}

void UCameraStage_InputGather::GatherCharacterState(FStageExecutionContext& Context)
{
    ACharacter* Character = GetControlledCharacter(Context);
    
    if (!Character)
    {
        // Try to get owner actor if not a character
        AActor* Owner = Context.Manager ? Context.Manager->GetOwner() : nullptr;
        if (Owner)
        {
            Context.InputContext.CharacterLocation = Owner->GetActorLocation();
            Context.InputContext.CharacterRotation = Owner->GetActorRotation();
            Context.InputContext.CharacterVelocity = Owner->GetVelocity();
            Context.InputContext.CharacterSpeed = Context.InputContext.CharacterVelocity.Size2D();
            Context.InputContext.bIsMoving = Context.InputContext.CharacterSpeed > MovingSpeedThreshold;
        }
        return;
    }

    // Basic transform
    Context.InputContext.CharacterLocation = Character->GetActorLocation();
    Context.InputContext.CharacterRotation = Character->GetActorRotation();

    // Velocity
    Context.InputContext.CharacterVelocity = Character->GetVelocity();
    Context.InputContext.CharacterSpeed = Context.InputContext.CharacterVelocity.Size2D();
    
    // Movement state flags
    Context.InputContext.bIsMoving = Context.InputContext.CharacterSpeed > MovingSpeedThreshold;
    Context.InputContext.bIsCrouching = Character->bIsCrouched;

    // Movement component data
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp)
    {
        Context.InputContext.bIsInAir = MovementComp->IsFalling();
        
        // Sprint detection: speed exceeds walk speed threshold
        const float SprintSpeedThreshold = MovementComp->MaxWalkSpeed * 1.2f;
        Context.InputContext.bIsSprinting = Context.InputContext.CharacterSpeed > SprintSpeedThreshold;
    }
    else
    {
        Context.InputContext.bIsInAir = false;
        Context.InputContext.bIsSprinting = false;
    }

    // Health ratio (try to get from common interface or component)
    // Default to full health if not available
    Context.InputContext.HealthRatio = 1.0f;
    
    // TODO: Implement health detection via interface
    // if (IHealthInterface* HealthInterface = Cast<IHealthInterface>(Character))
    // {
    //     Context.InputContext.HealthRatio = HealthInterface->GetHealthRatio();
    // }
}

void UCameraStage_InputGather::GatherTargetInfo(FStageExecutionContext& Context)
{
    // Get target from manager (lock-on target)
    AActor* Target = nullptr;
    
    if (Context.Manager)
    {
        // Try to get lock-on target from manager
        // Note: GetLockOnTarget() should be implemented in SoulsCameraManager
        Target = Context.Manager->GetLockOnTarget();
    }
    
    Context.InputContext.SetTargetActor(Target);

    if (Target)
    {
        // ==================== 优先从 SubTargetManager 获取锁定位置 ====================
        bool bUsedSubTarget = false;
        ACharacter* Character = GetControlledCharacter(Context);
        if (Character)
        {
            USubTargetManager* SubTargetMgr = Character->FindComponentByClass<USubTargetManager>();
            if (SubTargetMgr && SubTargetMgr->HasValidSubTarget())
            {
                Context.InputContext.TargetLocation = SubTargetMgr->GetCurrentLockPosition();
                bUsedSubTarget = true;
            }
        }

        // Fallback：如果没有 SubTargetManager 或没有有效子锁点，使用 Actor Root
        if (!bUsedSubTarget)
        {
            Context.InputContext.TargetLocation = Target->GetActorLocation();
        }

        Context.InputContext.TargetDistance = FVector::Dist(
            Context.InputContext.CharacterLocation,
            Context.InputContext.TargetLocation
        );

        // Calculate target size from bounds
        FVector Origin, BoxExtent;
        Target->GetActorBounds(false, Origin, BoxExtent);
        Context.InputContext.TargetSize = BoxExtent.Size();

        // Check if boss (via tag or interface)
        Context.InputContext.bTargetIsBoss = Target->Tags.Contains(FName(TEXT("Boss")));
        
        // Combat state
        Context.InputContext.bIsInCombat = true;
    }
    else
    {
        Context.InputContext.TargetLocation = FVector::ZeroVector;
        Context.InputContext.TargetDistance = 0.0f;
        Context.InputContext.TargetSize = 100.0f;
        Context.InputContext.bTargetIsBoss = false;
        
        // No target doesn't necessarily mean no combat
        // Combat state could be set by other means (e.g., recent damage, enemy proximity)
        // For now, default to false when no target
        Context.InputContext.bIsInCombat = false;
    }
}

void UCameraStage_InputGather::GatherEnvironmentState(FStageExecutionContext& Context)
{
    // Environment detection will be fully implemented in Phase 5 (Collision)
    // For now, set safe defaults
    
    Context.InputContext.bInTightSpace = false;
    Context.InputContext.bUnderLowCeiling = false;
    Context.InputContext.bNearCliffEdge = false;

    // TODO Phase 5: Implement environment traces
    // - Check for walls on sides (tight space detection)
    // - Check for ceiling above (low ceiling detection)
    // - Check for ground below/ahead (cliff edge detection)
    
    UE_LOG(LogCameraStage_Input, VeryVerbose, TEXT("GatherEnvironmentState: Using defaults (Phase 5 TODO)"));
}

void UCameraStage_InputGather::StorePreviousFrameData(FStageExecutionContext& Context)
{
    if (bFirstFrame)
    {
        // First frame - initialize with current values
        if (Context.Manager)
        {
            AActor* Owner = Context.Manager->GetOwner();
            if (Owner)
            {
                PreviousCameraLocation = Owner->GetActorLocation();
                PreviousCameraRotation = Owner->GetActorRotation();
            }
        }
    }

    // Set previous frame data in context
    Context.InputContext.PreviousCameraLocation = PreviousCameraLocation;
    Context.InputContext.PreviousCameraRotation = PreviousCameraRotation;

    // Store current for next frame (will be updated after render apply)
    // For now, use output from last frame if available
    PreviousCameraLocation = Context.Output.GetCameraLocation();
    PreviousCameraRotation = Context.Output.Rotation;
}

//========================================
// Helper Methods 辅助方法
//========================================

ACharacter* UCameraStage_InputGather::GetControlledCharacter(const FStageExecutionContext& Context) const
{
    if (!Context.Manager)
    {
        return nullptr;
    }

    AActor* Owner = Context.Manager->GetOwner();
    return Cast<ACharacter>(Owner);
}

APlayerController* UCameraStage_InputGather::GetPlayerController(const FStageExecutionContext& Context) const
{
    ACharacter* Character = GetControlledCharacter(Context);
    if (Character)
    {
        return Cast<APlayerController>(Character->GetController());
    }

    // Fallback: try to get from owner directly if it's a pawn
    if (Context.Manager)
    {
        AActor* Owner = Context.Manager->GetOwner();
        if (APawn* Pawn = Cast<APawn>(Owner))
        {
            return Cast<APlayerController>(Pawn->GetController());
        }
    }

    return nullptr;
}
