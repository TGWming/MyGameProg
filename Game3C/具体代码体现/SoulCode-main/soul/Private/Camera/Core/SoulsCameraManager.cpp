// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Core/SoulsCameraStateMachine.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// Pipeline includes
#include "Camera/Pipeline/SoulsCameraPipeline.h"
#include "Camera/Pipeline/ICameraStage.h"

// Stage includes
#include "Camera/Pipeline/CameraStage_InputGather.h"
#include "Camera/Pipeline/CameraStage_StateMachine.h"
#include "Camera/Pipeline/CameraStage_ModuleCompute.h"
#include "Camera/Pipeline/CameraStage_ModifierApply.h"
#include "Camera/Pipeline/CameraStage_BlendSolve.h"
#include "Camera/Pipeline/CameraStage_CollisionResolve.h"
#include "Camera/Pipeline/CameraStage_PostProcess.h"
#include "Camera/Pipeline/CameraStage_RenderApply.h"

// Module system includes (Phase 3)
#include "Camera/Modules/CameraModuleRegistry.h"

// Modifier system includes (Phase 4)
#include "Camera/Core/CameraModifierManager.h"
#include "Camera/Modifiers/CameraModifierBase.h"

// Collision system includes (Phase 5)
#include "Camera/Collision/CameraCollisionResolver.h"
#include "Camera/Collision/CameraCollisionBase.h"

// Config system includes (Phase 6 - New Architecture)
#include "Camera/Config/CameraConfigService.h"
#include "Camera/Config/DefaultCameraParams.h"

// ★ Hot Reload 修复：添加 ConstructorHelpers
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogSoulsCamera, Log, All);

USoulsCameraManager::USoulsCameraManager()
    : GlobalConfig(nullptr)
    , StatesDataTable(nullptr)
    , bAutoFindComponents(true)
    , bAutoInitialize(true)
    , bDisableNativeCollision(false)  // Hybrid Mode C: 原生碰撞始终启用
    , OverridesDataTable(nullptr)
    , bUseNewConfigArchitecture(true)  // ★ 默认启用新架构
    , ConfigService(nullptr)
    , SpringArmComponent(nullptr)
    , CameraComponent(nullptr)
    , Pipeline(nullptr)
    , StateMachine(nullptr)
    , ModuleRegistry(nullptr)
    , ModifierManager(nullptr)
    , CollisionResolver(nullptr)
    , Debugger(nullptr)
    , Stage4_ModifierApply(nullptr)
    , LockOnTarget(nullptr)
    , bIsInitialized(false)
    , bDebugEnabled(false)
    , FrameCounter(0)
    , PreviousFocusPoint(FVector::ZeroVector)
    , PreviousRotation(FRotator::ZeroRotator)
    , PreviousDistance(400.0f)
    , PreviousFOV(70.0f)
    , bOriginalEnableCameraLag(true)
    , OriginalCameraLagSpeed(10.0f)
    , bOriginalEnableCameraRotationLag(true)
    , OriginalCameraRotationLagSpeed(10.0f)
    , OriginalCameraLagMaxDistance(0.0f)
    , bHasSavedOriginalLagSettings(false)
    // ★★★ Hot Reload 修复：注释掉初始化列表中的 TransitionDuration 赋值 ★★★
    // 让头文件中的默认成员初始化器生效，避免 Hot Reload 后参数重置
    // , LockOnRotationLagSpeed(3.0f)
    // , FOVTransitionDuration(0.8f)
    // , DistanceTransitionDuration(0.8f)
    // , SocketOffsetTransitionDuration(0.5f)
    // , TargetOffsetTransitionDuration(0.5f)
    // , PitchTransitionDuration(0.5f)
    // , bEnableTransitionDebugLog(true)
#if WITH_EDITOR
    , bShowDebugInfo(true)
    , bDebugCollision(false)
    , bDebugStateMachine(false)
    , bDebugPipeline(false)
    , bDebugModules(false)
    , bDebugAll(false)
#endif
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // ★★★ Hot Reload 修复：在构造函数中加载默认 DataTable ★★★
    // 这确保了即使 Hot Reload 清除了蓝图中的引用，也会有默认值
    
    // 默认加载 StatesDataTable
    static ConstructorHelpers::FObjectFinder<UDataTable> StatesTableFinder(
        TEXT("/Game/DataTables/CameraStates725.CameraStates725"));
    if (StatesTableFinder.Succeeded())
    {
        StatesDataTable = StatesTableFinder.Object;
    }

    // 默认加载 OverridesDataTable
    static ConstructorHelpers::FObjectFinder<UDataTable> OverridesTableFinder(
        TEXT("/Game/DataTables/DT_CameraOverrides.DT_CameraOverrides"));
    if (OverridesTableFinder.Succeeded())
    {
        OverridesDataTable = OverridesTableFinder.Object;
    }
}

void USoulsCameraManager::BeginPlay()
{
    Super::BeginPlay();

    //========================================
    // ★ 备用加载：如果 DataTable 引用丢失，从路径加载
    //========================================
    if (!StatesDataTable)
    {
        // 你的 DataTable 路径
        static const TCHAR* FallbackPath = TEXT("/Game/DataTables/CameraStates725.CameraStates725");
        
        StatesDataTable = LoadObject<UDataTable>(nullptr, FallbackPath);
        
        if (StatesDataTable)
        {
            UE_LOG(LogSoulsCamera, Warning, TEXT("★ StatesDataTable loaded from fallback path"));
        }
        else
        {
            UE_LOG(LogSoulsCamera, Error, TEXT("★ StatesDataTable fallback load FAILED! Path: %s"), FallbackPath);
        }
    }
    else
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("StatesDataTable already set: %s"), *StatesDataTable->GetName());
    }

    //========================================
    // 原有逻辑
    //========================================
    if (bAutoFindComponents)
    {
        AutoFindComponents();
    }

    if (bAutoInitialize)
    {
        InitializeCamera();
    }

    // ★ 新增：在游戏开始时预先保存原始 LAG 设置，避免首次锁定时的跳动
    if (SpringArmComponent && !bHasSavedOriginalLagSettings)
    {
        bOriginalEnableCameraLag = SpringArmComponent->bEnableCameraLag;
        OriginalCameraLagSpeed = SpringArmComponent->CameraLagSpeed;
        bOriginalEnableCameraRotationLag = SpringArmComponent->bEnableCameraRotationLag;
        OriginalCameraRotationLagSpeed = SpringArmComponent->CameraRotationLagSpeed;
        OriginalCameraLagMaxDistance = SpringArmComponent->CameraLagMaxDistance;
        bHasSavedOriginalLagSettings = true;
        
        // ★ 新增：初始化 SocketOffset 过渡值，避免首次锁定时跳动
        CurrentSmoothedSocketOffset = SpringArmComponent->SocketOffset;
        SocketOffsetTransitionTargetValue = CurrentSmoothedSocketOffset;
        
        UE_LOG(LogSoulsCamera, Log, TEXT("BeginPlay: Pre-saved original LAG settings - bEnableLag=%s, LagSpeed=%.1f, RotLagSpeed=%.1f"),
            bOriginalEnableCameraLag ? TEXT("true") : TEXT("false"),
            OriginalCameraLagSpeed,
            OriginalCameraRotationLagSpeed);
    }
    
    UE_LOG(LogTemp, Error, TEXT("BeginPlay SocketOffset: Smoothed=(%.1f,%.1f,%.1f)"), CurrentSmoothedSocketOffset.X, CurrentSmoothedSocketOffset.Y, CurrentSmoothedSocketOffset.Z);
}

void USoulsCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Cleanup will be handled by garbage collection
    bIsInitialized = false;
}

void USoulsCameraManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // ★ DEBUG: 确认 TickComponent 被调用
    // UE_LOG(LogTemp, Warning, TEXT("DEBUG TickComponent CALLED, Pipeline=%s"), 
    //     Pipeline ? TEXT("Valid") : TEXT("NULL"));

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 编辑器模式下不执行任何逻辑，防止影响编辑器预览
    if (!GetWorld() || !GetWorld()->IsGameWorld())
    {
        return;
    }

    //========================================
    // ★ Hot Reload 恢复检查
    // 如果 DataTable 丢失（Hot Reload 导致），自动重新加载并初始化
    //========================================
    if (!StatesDataTable || !bIsInitialized)
    {
        static const TCHAR* FallbackPath = TEXT("/Game/DataTables/CameraStates725.CameraStates725");
        
        if (!StatesDataTable)
        {
            StatesDataTable = LoadObject<UDataTable>(nullptr, FallbackPath);
            
            if (StatesDataTable)
            {
                UE_LOG(LogSoulsCamera, Warning, TEXT("★ [Hot Reload Recovery] StatesDataTable restored from: %s"), FallbackPath);
            }
        }
        
        // 如果 DataTable 恢复了但系统未初始化，重新初始化
        if (StatesDataTable && !bIsInitialized)
        {
            UE_LOG(LogSoulsCamera, Warning, TEXT("★ [Hot Reload Recovery] Re-initializing camera system..."));
            
            if (bAutoFindComponents)
            {
                AutoFindComponents();
            }
            
            if (bAutoInitialize)
            {
                InitializeCamera();
            }
        }
        
        // 如果仍然没有 DataTable，跳过本帧
        if (!StatesDataTable)
        {
            return;
        }
    }

    //========================================
    // Guard: Check initialization
    //========================================
    if (!bIsInitialized)
    {
        return;
    }

    //========================================
    // Guard: Skip if paused (optional)
    //========================================
    if (GetWorld() && GetWorld()->IsPaused())
    {
        return;
    }

    // ★ 修改：Edge 状态不需要锁定目标，其他状态没有锁定目标时跳过
    FName CurrentState = StateMachine ? StateMachine->GetCurrentStateName() : NAME_None;
    bool bIsEdgeState = CurrentState.ToString().StartsWith(TEXT("Edge_"));
    bool bIsInSpecialState = bIsEdgeState || LockOnTarget.IsValid() || bManualStateActive;

    // ★★★ 问题5修复：检查是否有活跃的过渡需要继续执行 ★★★
    bool bHasActiveTransition = bDistanceTransitionActive || bFOVTransitionActive || 
                                 bSocketOffsetTransitionActive || bTargetOffsetTransitionActive;

    if (!bIsInSpecialState)
    {
        // 如果刚从特殊状态退出，恢复原生设置（启动退出过渡）
        if (bWasInSpecialState)
        {
            // UE_LOG(LogTemp, Warning, TEXT("DEBUG TickComponent - Exiting special state, starting exit transitions"));
            RestoreNativeSpringArmSettings();
            bWasInSpecialState = false;
        }
        
        // ★★★ 问题5修复：如果有活跃的过渡，继续执行过渡逻辑 ★★★
        if (bHasActiveTransition)
        {
            // UE_LOG(LogTemp, Warning, TEXT("DEBUG TickComponent - Executing exit transitions: Dist=%d, FOV=%d, Socket=%d"),
            //     bDistanceTransitionActive ? 1 : 0, bFOVTransitionActive ? 1 : 0, bSocketOffsetTransitionActive ? 1 : 0);
            
            // 执行过渡逻辑
            float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
            
            // Distance 过渡
            if (bDistanceTransitionActive)
            {
                DistanceTransitionElapsedTime += DeltaTime;
                float Alpha = FMath::Clamp(DistanceTransitionElapsedTime / DistanceTransitionDuration, 0.0f, 1.0f);
                CurrentSmoothedDistance = FMath::Lerp(DistanceTransitionStartValue, DistanceTransitionTargetValue, Alpha);
                
                if (Alpha >= 1.0f)
                {
                    bDistanceTransitionActive = false;
                    CurrentSmoothedDistance = DistanceTransitionTargetValue;
                    // UE_LOG(LogTemp, Warning, TEXT("Exit Distance Transition COMPLETE: %.1f"), CurrentSmoothedDistance);
                }
                
                SpringArmComponent->TargetArmLength = CurrentSmoothedDistance;
            }
            
            // FOV 过渡
            if (bFOVTransitionActive)
            {
                FOVTransitionElapsedTime += DeltaTime;
                float Alpha = FMath::Clamp(FOVTransitionElapsedTime / FOVTransitionDuration, 0.0f, 1.0f);
                CurrentSmoothedFOV = FMath::Lerp(FOVTransitionStartValue, FOVTransitionTargetValue, Alpha);
                
                if (Alpha >= 1.0f)
                {
                    bFOVTransitionActive = false;
                    CurrentSmoothedFOV = FOVTransitionTargetValue;
                    // UE_LOG(LogTemp, Warning, TEXT("Exit FOV Transition COMPLETE: %.1f"), CurrentSmoothedFOV);
                }
                
                CameraComponent->SetFieldOfView(CurrentSmoothedFOV);
            }
            
            // SocketOffset 过渡
            if (bSocketOffsetTransitionActive)
            {
                SocketOffsetTransitionElapsedTime += DeltaTime;
                float Alpha = FMath::Clamp(SocketOffsetTransitionElapsedTime / SocketOffsetTransitionDuration, 0.0f, 1.0f);
                CurrentSmoothedSocketOffset = FMath::Lerp(SocketOffsetTransitionStartValue, SocketOffsetTransitionTargetValue, Alpha);
                
                if (Alpha >= 1.0f)
                {
                    bSocketOffsetTransitionActive = false;
                    CurrentSmoothedSocketOffset = SocketOffsetTransitionTargetValue;
                }
                
                SpringArmComponent->SocketOffset = CurrentSmoothedSocketOffset;
            }
            
            // TargetOffset 过渡
            if (bTargetOffsetTransitionActive)
            {
                TargetOffsetTransitionElapsedTime += DeltaTime;
                float Alpha = FMath::Clamp(TargetOffsetTransitionElapsedTime / TargetOffsetTransitionDuration, 0.0f, 1.0f);
                CurrentSmoothedTargetOffset = FMath::Lerp(TargetOffsetTransitionStartValue, TargetOffsetTransitionTargetValue, Alpha);
                
                if (Alpha >= 1.0f)
                {
                    bTargetOffsetTransitionActive = false;
                    CurrentSmoothedTargetOffset = TargetOffsetTransitionTargetValue;
                }
                
                SpringArmComponent->TargetOffset = CurrentSmoothedTargetOffset;
            }
        }
        
        return;
    }

    // 记录当前在特殊状态
    bWasInSpecialState = true;

    //========================================
    // Step 1: Gather Input Context
    //========================================
    GatherInputContext(DeltaTime);

    // ★★★ 跳动诊断日志 ★★★
#if WITH_EDITOR
    if (IsCollisionDebugEnabled() && SpringArmComponent)
    {
        static int32 JumpDiagCount = 0;
        static float LastArmLength = 0.0f;
        static bool bWasColliding = false;
        
        float CurrentArmLength = SpringArmComponent->TargetArmLength;
        
        // 获取 SpringArm 的实际相机位置（考虑碰撞后）
        FVector SocketLocation = SpringArmComponent->GetSocketLocation(USpringArmComponent::SocketName);
        FVector SpringArmLocation = SpringArmComponent->GetComponentLocation();
        float ActualDistance = FVector::Dist(SpringArmLocation, SocketLocation);
        
        // 检测是否发生跳动（距离突然变化）
        float DistanceChange = FMath::Abs(ActualDistance - LastArmLength);
        bool bJumpDetected = (DistanceChange > 20.0f) && (LastArmLength > 0.0f);
        
        if (bJumpDetected && JumpDiagCount < 50)
        {
            JumpDiagCount++;
            
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            UE_LOG(LogSoulsCamera, Error, TEXT("╔══════════════════════════════════════════════════════════════╗"));
            UE_LOG(LogSoulsCamera, Error, TEXT("║            ★★★ 跳动检测 #%d ★★★                          ║"), JumpDiagCount);
            UE_LOG(LogSoulsCamera, Error, TEXT("╚══════════════════════════════════════════════════════════════╝"));
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            UE_LOG(LogSoulsCamera, Error, TEXT("【距离变化】"));
            UE_LOG(LogSoulsCamera, Error, TEXT("  上一帧距离: %.1f"), LastArmLength);
            UE_LOG(LogSoulsCamera, Error, TEXT("  当前帧距离: %.1f"), ActualDistance);
            UE_LOG(LogSoulsCamera, Error, TEXT("  变化量: %.1f"), DistanceChange);
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            UE_LOG(LogSoulsCamera, Error, TEXT("【SpringArm 状态】"));
            UE_LOG(LogSoulsCamera, Error, TEXT("  TargetArmLength: %.1f"), CurrentArmLength);
            UE_LOG(LogSoulsCamera, Error, TEXT("  bDoCollisionTest: %s"), SpringArmComponent->bDoCollisionTest ? TEXT("YES") : TEXT("NO"));
            UE_LOG(LogSoulsCamera, Error, TEXT("  SpringArm 位置: (%.1f, %.1f, %.1f)"), 
                SpringArmLocation.X, SpringArmLocation.Y, SpringArmLocation.Z);
            UE_LOG(LogSoulsCamera, Error, TEXT("  Socket 位置: (%.1f, %.1f, %.1f)"), 
                SocketLocation.X, SocketLocation.Y, SocketLocation.Z);
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            UE_LOG(LogSoulsCamera, Error, TEXT("【角色状态】"));
            UE_LOG(LogSoulsCamera, Error, TEXT("  角色位置: (%.1f, %.1f, %.1f)"), 
                CurrentInputContext.CharacterLocation.X,
                CurrentInputContext.CharacterLocation.Y,
                CurrentInputContext.CharacterLocation.Z);
            UE_LOG(LogSoulsCamera, Error, TEXT("  角色速度: %.1f"), CurrentInputContext.CharacterSpeed);
            UE_LOG(LogSoulsCamera, Error, TEXT("  是否移动: %s"), CurrentInputContext.bIsMoving ? TEXT("YES") : TEXT("NO"));
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            UE_LOG(LogSoulsCamera, Error, TEXT("【Pipeline 输出】"));
            UE_LOG(LogSoulsCamera, Error, TEXT("  Output.Distance: %.1f"), CurrentOutput.Distance);
            UE_LOG(LogSoulsCamera, Error, TEXT("  Output.FocusPoint: (%.1f, %.1f, %.1f)"),
                CurrentOutput.FocusPoint.X, CurrentOutput.FocusPoint.Y, CurrentOutput.FocusPoint.Z);
            UE_LOG(LogSoulsCamera, Error, TEXT("  Output.bCollisionAdjusted: %s"), 
                CurrentOutput.bCollisionAdjusted ? TEXT("YES") : TEXT("NO"));
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            
            // 计算 FocusPoint 到 SpringArm 起点的差异
            FVector FocusToSpringArm = SpringArmLocation - CurrentOutput.FocusPoint;
            UE_LOG(LogSoulsCamera, Error, TEXT("【差异分析】"));
            UE_LOG(LogSoulsCamera, Error, TEXT("  FocusPoint 与 SpringArm 起点差异: (%.1f, %.1f, %.1f)"),
                FocusToSpringArm.X, FocusToSpringArm.Y, FocusToSpringArm.Z);
            UE_LOG(LogSoulsCamera, Error, TEXT("  差异距离: %.1f"), FocusToSpringArm.Size());
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            UE_LOG(LogSoulsCamera, Error, TEXT("══════════════════════════════════════════════════════════════════"));
        }
        
        LastArmLength = ActualDistance;
    }
#endif

    // ★★★ 简单诊断：每帧输出 ★★★
#if WITH_EDITOR
    if (IsPipelineDebugEnabled())
    {
        static int32 TickDiagCount = 0;
        if (TickDiagCount < 30)
        {
            TickDiagCount++;
            
            float SpringArmBefore = SpringArmComponent ? SpringArmComponent->TargetArmLength : -1.0f;
            
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
            UE_LOG(LogSoulsCamera, Error, TEXT("===== TICK #%d ====="), TickDiagCount);
            UE_LOG(LogSoulsCamera, Error, TEXT("[1] SpringArm BEFORE Pipeline: %.1f"), SpringArmBefore);
        }
    }
#endif

    //========================================
    // Step 2: Execute Pipeline (8 Stages)
    //========================================
    if (Pipeline)
    {
        CurrentOutput = Pipeline->Execute(DeltaTime, CurrentInputContext);


        // ★ DEBUG: 检查 Pipeline 输出
        // UE_LOG(LogTemp, Warning, TEXT("DEBUG Pipeline:  bIsValid=%d, Distance=%f"),
        //     CurrentOutput.bIsValid ? 1 : 0, CurrentOutput.Distance);
        
        // ★★★ Pipeline 执行后立即检查 ★★★
#if WITH_EDITOR
        if (IsPipelineDebugEnabled())
        {
            static int32 PipelineDiagCount = 0;
            if (PipelineDiagCount < 30)
            {
                PipelineDiagCount++;
                UE_LOG(LogSoulsCamera, Error, TEXT("[2] Pipeline Output: Distance=%.1f, bIsValid=%s"), 
                    CurrentOutput.Distance, CurrentOutput.bIsValid ? TEXT("YES") : TEXT("NO"));
            }
        }
#endif
        
        if (!CurrentOutput.bIsValid)
        {
            // ★ DEBUG: Pipeline 失败
            // UE_LOG(LogTemp, Warning, TEXT("DEBUG Pipeline FAILED - using fallback"));
            
            // Pipeline失败，使用回退参数
            static bool bWarningPrinted = false;
            if (!bWarningPrinted)
            {
                UE_LOG(LogSoulsCamera, Warning, TEXT("Pipeline invalid - Using fallback parameters"));
                bWarningPrinted = true;
            }
            ApplyFallbackParameters();
            
#if WITH_EDITOR
            if (IsPipelineDebugEnabled())
            {
                static int32 FallbackDiagCount = 0;
                if (FallbackDiagCount < 30)
                {
                    FallbackDiagCount++;
                    UE_LOG(LogSoulsCamera, Error, TEXT("[3] Used FALLBACK, SpringArm now: %.1f"), 
                        SpringArmComponent ? SpringArmComponent->TargetArmLength : -1.0f);
                }
            }
#endif
        }
        else
        {
            // ★ DEBUG: 即将调用 ApplyOutputToComponents
            // UE_LOG(LogTemp, Warning, TEXT("DEBUG About to call ApplyOutputToComponents"));
            
            // Pipeline成功，应用输出
            ApplyOutputToComponents();
            
#if WITH_EDITOR
            if (IsPipelineDebugEnabled())
            {
                static int32 ApplyDiagCount = 0;
                if (ApplyDiagCount < 30)
                {
                    ApplyDiagCount++;
                    UE_LOG(LogSoulsCamera, Error, TEXT("[3] Applied Output, SpringArm now: %.1f"), 
                        SpringArmComponent ? SpringArmComponent->TargetArmLength : -1.0f);
                }
            }
#endif
        }
    }
    else
    {
#if WITH_EDITOR
        if (IsPipelineDebugEnabled())
        {
            static int32 NullPipelineDiagCount = 0;
            if (NullPipelineDiagCount < 30)
            {
                NullPipelineDiagCount++;
                UE_LOG(LogSoulsCamera, Error, TEXT("[2] Pipeline is NULL! Using fallback"));
            }
        }
#endif
        ApplyFallbackParameters();
    }

    // ★★★ Tick 结束时检查 ★★★
#if WITH_EDITOR
    if (IsPipelineDebugEnabled())
    {
        static int32 TickEndDiagCount = 0;
        if (TickEndDiagCount < 30)
        {
            TickEndDiagCount++;
            float SpringArmAfter = SpringArmComponent ? SpringArmComponent->TargetArmLength : -1.0f;
            UE_LOG(LogSoulsCamera, Error, TEXT("[4] SpringArm AFTER everything: %.1f"), SpringArmAfter);
            UE_LOG(LogSoulsCamera, Error, TEXT("===== TICK END ====="));
            UE_LOG(LogSoulsCamera, Error, TEXT(""));
        }
    }
#endif

    //========================================
    // Step 3: Store Previous Frame Data
    //========================================
    StorePreviousFrameData();

    //========================================
    // Step 4: Increment Frame Counter
    //========================================
    FrameCounter++;

    // ========== 全面诊断日志 ==========
#if WITH_EDITOR
    if (IsPipelineDebugEnabled())
    {
        static int32 DiagFrameCount = 0;
        if (DiagFrameCount < 60) // 记录前60帧
        {
            DiagFrameCount++;
            
            if (DiagFrameCount == 1 || DiagFrameCount == 30 || DiagFrameCount == 60)
            {
                UE_LOG(LogSoulsCamera, Error, TEXT(""));
                UE_LOG(LogSoulsCamera, Error, TEXT("╔════════════════════════════════════════════════════════════════╗"));
                UE_LOG(LogSoulsCamera, Error, TEXT("║              DIAGNOSTIC REPORT - Frame %d                      ║"), DiagFrameCount);
                UE_LOG(LogSoulsCamera, Error, TEXT("╚════════════════════════════════════════════════════════════════╝"));
                
                // 1. SpringArm状态
                UE_LOG(LogSoulsCamera, Error, TEXT("【1. SpringArm Component Status】"));
                if (SpringArmComponent)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   TargetArmLength: %.2f"), SpringArmComponent->TargetArmLength);
                    UE_LOG(LogSoulsCamera, Error, TEXT("   bDoCollisionTest: %s"), SpringArmComponent->bDoCollisionTest ? TEXT("TRUE <<<") : TEXT("FALSE"));
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ProbeSize: %.2f"), SpringArmComponent->ProbeSize);
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ProbeChannel: %d"), (int32)SpringArmComponent->ProbeChannel.GetValue());
                    UE_LOG(LogSoulsCamera, Error, TEXT("   SocketOffset: (%.1f, %.1f, %.1f)"), 
                        SpringArmComponent->SocketOffset.X, SpringArmComponent->SocketOffset.Y, SpringArmComponent->SocketOffset.Z);
                    UE_LOG(LogSoulsCamera, Error, TEXT("   TargetOffset: (%.1f, %.1f, %.1f)"),
                        SpringArmComponent->TargetOffset.X, SpringArmComponent->TargetOffset.Y, SpringArmComponent->TargetOffset.Z);
                    UE_LOG(LogSoulsCamera, Error, TEXT("   bUsePawnControlRotation: %s"), SpringArmComponent->bUsePawnControlRotation ? TEXT("TRUE") : TEXT("FALSE"));
                    UE_LOG(LogSoulsCamera, Error, TEXT("   bInheritPitch/Yaw/Roll: %s/%s/%s"), 
                        SpringArmComponent->bInheritPitch ? TEXT("T") : TEXT("F"),
                        SpringArmComponent->bInheritYaw ? TEXT("T") : TEXT("F"),
                        SpringArmComponent->bInheritRoll ? TEXT("T") : TEXT("F"));
                    
                    // 实际相机位置
                    FVector SpringArmLoc = SpringArmComponent->GetComponentLocation();
                    FVector SocketLoc = SpringArmComponent->GetSocketLocation(USpringArmComponent::SocketName);
                    float ActualDistance = FVector::Dist(SpringArmLoc, SocketLoc);
                    UE_LOG(LogSoulsCamera, Error, TEXT("   SpringArm Location: (%.1f, %.1f, %.1f)"), SpringArmLoc.X, SpringArmLoc.Y, SpringArmLoc.Z);
                    UE_LOG(LogSoulsCamera, Error, TEXT("   Socket Location: (%.1f, %.1f, %.1f)"), SocketLoc.X, SocketLoc.Y, SocketLoc.Z);
                    UE_LOG(LogSoulsCamera, Error, TEXT("   Actual Distance (Socket): %.2f"), ActualDistance);
                }
                else
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   SpringArmComponent is NULL!"));
                }
                
                // 2. Camera Component状态
                UE_LOG(LogSoulsCamera, Error, TEXT("【2. Camera Component Status】"));
                if (CameraComponent)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   FieldOfView: %.2f"), CameraComponent->FieldOfView);
                    FVector CamWorldLoc = CameraComponent->GetComponentLocation();
                    UE_LOG(LogSoulsCamera, Error, TEXT("   World Location: (%.1f, %.1f, %.1f)"), CamWorldLoc.X, CamWorldLoc.Y, CamWorldLoc.Z);
                }
                
                // 3. StateMachine状态
                UE_LOG(LogSoulsCamera, Error, TEXT("【3. StateMachine Status】"));
                if (StateMachine)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   IsInitialized: %s"), StateMachine->IsInitialized() ? TEXT("YES") : TEXT("NO"));
                    UE_LOG(LogSoulsCamera, Error, TEXT("   CurrentState: %s"), *StateMachine->GetCurrentStateName().ToString());
                    
                    // 获取当前状态配置
                    FCameraStateConfig CurrentConfig;
                    if (StateMachine->GetCurrentStateConfig(CurrentConfig))
                    {
                        UE_LOG(LogSoulsCamera, Error, TEXT("   Config BaseDistance: %.2f"), CurrentConfig.StateBase.Distance.BaseDistance);
                        UE_LOG(LogSoulsCamera, Error, TEXT("   Config BaseFOV: %.2f"), CurrentConfig.StateBase.FOV.BaseFOV);
                    }
                    else
                    {
                        UE_LOG(LogSoulsCamera, Error, TEXT("   GetCurrentStateConfig FAILED!"));
                    }
                }
                
                // 4. ConfigService状态
                UE_LOG(LogSoulsCamera, Error, TEXT("【4. ConfigService Status】"));
                if (ConfigService)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   IsInitialized: %s"), ConfigService->IsInitialized() ? TEXT("YES") : TEXT("NO"));
                    
                    FCameraStateConfig TestConfig;
                    if (ConfigService->GetStateConfig(FName("Explore_Default"), TestConfig))
                    {
                        UE_LOG(LogSoulsCamera, Error, TEXT("   Explore_Default Config:"));
                        UE_LOG(LogSoulsCamera, Error, TEXT("     Distance.BaseDistance: %.2f"), TestConfig.StateBase.Distance.BaseDistance);
                        UE_LOG(LogSoulsCamera, Error, TEXT("     Distance.MinDistance: %.2f"), TestConfig.StateBase.Distance.MinDistance);
                        UE_LOG(LogSoulsCamera, Error, TEXT("     Distance.MaxDistance: %.2f"), TestConfig.StateBase.Distance.MaxDistance);
                        UE_LOG(LogSoulsCamera, Error, TEXT("     FOV.BaseFOV: %.2f"), TestConfig.StateBase.FOV.BaseFOV);
                        UE_LOG(LogSoulsCamera, Error, TEXT("     Offset.FocusOffset: (%.1f, %.1f, %.1f)"), 
                            TestConfig.StateBase.Offset.FocusOffset.X,
                            TestConfig.StateBase.Offset.FocusOffset.Y,
                            TestConfig.StateBase.Offset.FocusOffset.Z);
                        UE_LOG(LogSoulsCamera, Error, TEXT("     Offset.SocketOffset: (%.1f, %.1f, %.1f)"),
                            TestConfig.StateBase.Offset.SocketOffset.X,
                            TestConfig.StateBase.Offset.SocketOffset.Y,
                            TestConfig.StateBase.Offset.SocketOffset.Z);
                    }
                    else
                    {
                        UE_LOG(LogSoulsCamera, Error, TEXT("   GetStateConfig('Explore_Default') FAILED!"));
                    }
                }
                else
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ConfigService is NULL!"));
                }
                
                // 5. Pipeline状态
                UE_LOG(LogSoulsCamera, Error, TEXT("【5. Pipeline Status】"));
                if (Pipeline)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   Pipeline exists: YES"));
                    UE_LOG(LogSoulsCamera, Error, TEXT("   Last output bIsValid: %s"), CurrentOutput.bIsValid ? TEXT("YES") : TEXT("NO <<<"));
                }
                else
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   Pipeline is NULL!"));
                }
                
                // 6. CurrentOutput状态
                UE_LOG(LogSoulsCamera, Error, TEXT("【6. CurrentOutput Status】"));
                UE_LOG(LogSoulsCamera, Error, TEXT("   bIsValid: %s"), CurrentOutput.bIsValid ? TEXT("YES") : TEXT("NO"));
                UE_LOG(LogSoulsCamera, Error, TEXT("   Distance: %.2f"), CurrentOutput.Distance);
                UE_LOG(LogSoulsCamera, Error, TEXT("   FOV: %.2f"), CurrentOutput.FOV);
                UE_LOG(LogSoulsCamera, Error, TEXT("   FocusPoint: (%.1f, %.1f, %.1f)"), CurrentOutput.FocusPoint.X, CurrentOutput.FocusPoint.Y, CurrentOutput.FocusPoint.Z);
                UE_LOG(LogSoulsCamera, Error, TEXT("   SocketOffset: (%.1f, %.1f, %.1f)"), CurrentOutput.SocketOffset.X, CurrentOutput.SocketOffset.Y, CurrentOutput.SocketOffset.Z);
                UE_LOG(LogSoulsCamera, Error, TEXT("   CurrentStateName: %s"), *CurrentOutput.CurrentStateName.ToString());
                
                // 7. 角色位置
                UE_LOG(LogSoulsCamera, Error, TEXT("【7. Owner/Character Status】"));
                if (AActor* Owner = GetOwner())
                {
                    FVector OwnerLoc = Owner->GetActorLocation();
                    UE_LOG(LogSoulsCamera, Error, TEXT("   Owner Location: (%.1f, %.1f, %.1f)"), OwnerLoc.X, OwnerLoc.Y, OwnerLoc.Z);
                    
                    if (SpringArmComponent && CameraComponent)
                    {
                        FVector CamLoc = CameraComponent->GetComponentLocation();
                        float DistToOwner = FVector::Dist(OwnerLoc, CamLoc);
                        UE_LOG(LogSoulsCamera, Error, TEXT("   Camera-to-Owner Distance: %.2f"), DistToOwner);
                    }
                }
                
                // 8. 关键问题检测
                UE_LOG(LogSoulsCamera, Error, TEXT("【8. Issue Detection】"));
                bool bHasIssue = false;
                
                if (SpringArmComponent && SpringArmComponent->TargetArmLength < 100.0f)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ⚠️ ISSUE: TargetArmLength (%.1f) < 100! Should be 400!"), SpringArmComponent->TargetArmLength);
                    bHasIssue = true;
                }
                
                if (SpringArmComponent && SpringArmComponent->bDoCollisionTest)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ⚠️ ISSUE: bDoCollisionTest is TRUE! Collision may be pulling camera in."));
                    bHasIssue = true;
                }
                
                if (!CurrentOutput.bIsValid)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ⚠️ ISSUE: Pipeline output is INVALID! Using fallback."));
                    bHasIssue = true;
                }
                
                if (CurrentOutput.Distance < 100.0f)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ⚠️ ISSUE: CurrentOutput.Distance (%.1f) < 100!"), CurrentOutput.Distance);
                    bHasIssue = true;
                }
                
                if (!bHasIssue)
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT("   ✓ No obvious issues detected"));
                }
                
                UE_LOG(LogSoulsCamera, Error, TEXT("════════════════════════════════════════════════════════════════════"));
                UE_LOG(LogSoulsCamera, Error, TEXT(""));
            }
        }
    }
#endif
    // ========== 诊断日志结束 ==========

    //========================================
    // Debug Visualization (Optional)
    //========================================
#if WITH_EDITOR
    if (bShowDebugInfo)
    {
        DrawDebugInfo();
    }
#endif
}

void USoulsCameraManager::InitializeCamera()
{
    // ╔══════════════════════════════════════════════════════════════╗
    // ║                    调试日志 - 初始化开始                      ║
    // ╚══════════════════════════════════════════════════════════════╝
    UE_LOG(LogSoulsCamera, Warning, TEXT(" "));

    UE_LOG(LogSoulsCamera, Warning, TEXT("======================================================================"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("           SoulsCameraManager::InitializeCamera                       "));
    UE_LOG(LogSoulsCamera, Warning, TEXT("======================================================================"));

    UE_LOG(LogSoulsCamera, Warning, TEXT("Settings:"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  bUseNewConfigArchitecture: %s"), bUseNewConfigArchitecture ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  bAutoInitialize: %s"), bAutoInitialize ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  bDisableNativeCollision: %s"), bDisableNativeCollision ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("References:"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  StatesDataTable: %s"), StatesDataTable ? *StatesDataTable->GetName() : TEXT("NULL <<<"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  OverridesDataTable: %s"), OverridesDataTable ? *OverridesDataTable->GetName() : TEXT("NULL (OK)"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  SpringArmComponent: %s"), SpringArmComponent ? TEXT("Valid") : TEXT("NULL <<<"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  CameraComponent: %s"), CameraComponent ? TEXT("Valid") : TEXT("NULL <<<"));
    
    if (SpringArmComponent)
    {
        UE_LOG(LogSoulsCamera, Warning, TEXT("  SpringArm.TargetArmLength: %.1f"), SpringArmComponent->TargetArmLength);
        UE_LOG(LogSoulsCamera, Warning, TEXT("  SpringArm.SocketOffset: (%.1f, %.1f, %.1f)"), 
            SpringArmComponent->SocketOffset.X, SpringArmComponent->SocketOffset.Y, SpringArmComponent->SocketOffset.Z);
    }

    // ════════════════════════════════════════════════════════════════
    // 检查是否已初始化
    // ════════════════════════════════════════════════════════════════
    if (bIsInitialized)
    {
        UE_LOG(LogSoulsCamera, Warning, TEXT(">>> Already initialized, skipping"));
        return;
    }

    // ════════════════════════════════════════════════════════════════
    // 验证组件
    // ════════════════════════════════════════════════════════════════
    if (!SpringArmComponent)
    {
        UE_LOG(LogSoulsCamera, Error, TEXT(">>> FAILED: SpringArmComponent is NULL"));
        return;
    }

    if (!CameraComponent)
    {
        UE_LOG(LogSoulsCamera, Error, TEXT(">>> FAILED: CameraComponent is NULL"));
        return;
    }

    UE_LOG(LogSoulsCamera, Log, TEXT(">>> Components validated OK"));

    // ════════════════════════════════════════════════════════════════
    // 禁用原生 SpringArm 碰撞（如果需要）
    // ════════════════════════════════════════════════════════════════
    // ★★★ 方案C：启用 SpringArm 原生碰撞处理基础拉近 ★★★
    if (SpringArmComponent)
    {
        // 启用原生碰撞 - 让 SpringArm 处理基础的碰撞拉近
        SpringArmComponent->bDoCollisionTest = true;
        SpringArmComponent->ProbeSize = 12.0f;  // 标准探测半径
        SpringArmComponent->ProbeChannel = ECC_Camera;
        
        UE_LOG(LogSoulsCamera, Log, TEXT(">>> Enabled native SpringArm collision (Hybrid Mode C)"));
        UE_LOG(LogSoulsCamera, Log, TEXT(">>>   bDoCollisionTest = true"));
        UE_LOG(LogSoulsCamera, Log, TEXT(">>>   ProbeSize = %.1f"), SpringArmComponent->ProbeSize);
    }

    // ════════════════════════════════════════════════════════════════
    // 创建子系统
    // ════════════════════════════════════════════════════════════════
    CreateSubSystems();
    UE_LOG(LogSoulsCamera, Log, TEXT(">>> SubSystems created"));

    // ════════════════════════════════════════════════════════════════
    // 根据架构初始化状态机
    // ════════════════════════════════════════════════════════════════
    bool bStateMachineInitSuccess = false;

    if (bUseNewConfigArchitecture)
    {
        UE_LOG(LogSoulsCamera, Warning, TEXT(">>> Using NEW ConfigService architecture"));

        // 检查 DataTable
        if (!StatesDataTable)
        {
            UE_LOG(LogSoulsCamera, Error, TEXT(">>> StatesDataTable is NULL! Cannot use new architecture."));
            UE_LOG(LogSoulsCamera, Error, TEXT(">>> Please set StatesDataTable in Blueprint Details panel."));
        }
        else
        {
            UE_LOG(LogSoulsCamera, Log, TEXT(">>> StatesDataTable is valid: %s"), *StatesDataTable->GetName());
            
            // 创建 ConfigService
            if (!ConfigService)
            {
                ConfigService = NewObject<UCameraConfigService>(this);
                UE_LOG(LogSoulsCamera, Log, TEXT(">>> ConfigService created"));
            }

            if (ConfigService)
            {
                // 初始化 ConfigService
                UE_LOG(LogSoulsCamera, Log, TEXT(">>> Calling ConfigService->Initialize()..."));
                bool bConfigInitialized = ConfigService->Initialize(StatesDataTable, OverridesDataTable);
                UE_LOG(LogSoulsCamera, Warning, TEXT(">>> ConfigService->Initialize()返回: %s"), 
                    bConfigInitialized ? TEXT("TRUE") : TEXT("FALSE"));

                if (bConfigInitialized && ConfigService->IsInitialized())
                {
                    // 初始化 StateMachine
                    if (StateMachine)
                    {
                        UE_LOG(LogSoulsCamera, Log, TEXT(">>> Calling StateMachine->InitializeWithConfigService()..."));
                        StateMachine->InitializeWithConfigService(ConfigService);
                        
                        // 绑定状态切换事件，用于初始化过渡值
                        if (StateMachine)
                        {
                            StateMachine->OnStateChanged.AddDynamic(this, &USoulsCameraManager::OnCameraStateChanged);
                        }
            
                        bStateMachineInitSuccess = StateMachine->IsInitialized();
                        UE_LOG(LogSoulsCamera, Warning, TEXT(">>> StateMachine 初始化: %s"), 
                            bStateMachineInitSuccess ? TEXT("成功") : TEXT("失败"));
                        
                        if (bStateMachineInitSuccess)
                        {
                            UE_LOG(LogSoulsCamera, Warning, TEXT(">>> 当前状态: %s"), 
                                *StateMachine->GetCurrentStateName().ToString());
                        }
                    }
                    else
                    {
                        UE_LOG(LogSoulsCamera, Error, TEXT(">>> StateMachine is NULL after CreateSubSystems!"));
                    }
                }
                else
                {
                    UE_LOG(LogSoulsCamera, Error, TEXT(">>> ConfigService initialization failed"));
                }
            }
        }
    }
    else
    {
        UE_LOG(LogSoulsCamera, Warning, TEXT(">>> Using LEGACY DataTable architecture"));

        if (StateMachine && StatesDataTable)
        {
            StateMachine->Initialize(StatesDataTable);
            bStateMachineInitSuccess = StateMachine->IsInitialized();
            UE_LOG(LogSoulsCamera, Log, TEXT(">>> Legacy StateMachine initialized: %s"), 
                bStateMachineInitSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
        }
        else
        {
            UE_LOG(LogSoulsCamera, Warning, TEXT(">>> StateMachine or StatesDataTable is null"));
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 设置初始状态（如果 StateMachine 成功初始化）
    // ════════════════════════════════════════════════════════════════
    if (bStateMachineInitSuccess && StateMachine && StateMachine->IsInitialized())
    {
        if (StateMachine->GetCurrentStateName().IsNone())
        {
            StateMachine->RequestStateChange(FName("Explore_Default"), true);
            UE_LOG(LogSoulsCamera, Log, TEXT(">>> Requested initial state: Explore_Default"));
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 设置初始输出值
    // ════════════════════════════════════════════════════════════════
    if (SpringArmComponent)
    {
        CurrentOutput.Distance = SpringArmComponent->TargetArmLength;
        CurrentOutput.SocketOffset = SpringArmComponent->SocketOffset;
        CurrentOutput.TargetOffset = SpringArmComponent->TargetOffset;
        CurrentOutput.Rotation = SpringArmComponent->GetComponentRotation();

        // ★★★ 回退机制：确保有合理的默认值 ★★★
        if (CurrentOutput.Distance < 100.0f)
        {
            CurrentOutput.Distance = UDefaultCameraParams::GetDefaultBaseDistance();
            SpringArmComponent->TargetArmLength = CurrentOutput.Distance;
            UE_LOG(LogSoulsCamera, Warning, TEXT(">>> FALLBACK: Applied default distance: %.1f"), CurrentOutput.Distance);
        }
    }

    if (CameraComponent)
    {
        CurrentOutput.FOV = CameraComponent->FieldOfView;

        if (CurrentOutput.FOV < 10.0f)
        {
            CurrentOutput.FOV = UDefaultCameraParams::GetDefaultBaseFOV();
            CameraComponent->SetFieldOfView(CurrentOutput.FOV);
            UE_LOG(LogSoulsCamera, Warning, TEXT(">>> FALLBACK: Applied default FOV: %.1f"), CurrentOutput.FOV);
        }
    }

    // ★★★ 问题4修复：从 Explore_Default 配置获取 SocketOffset 并应用到 SpringArm ★★★
    // 探索状态下 Pipeline 不执行，配置中的 SocketOffset 永远不会被应用
    // 在初始化时预先设置，避免首次锁定时跳动
    FCameraStateConfig InitialConfig;
    bool bGotConfig = false;
    if (ConfigService) { bGotConfig = ConfigService->GetStateConfig(FName("Explore_Default"), InitialConfig); }
    if (bGotConfig && SpringArmComponent) {
        SpringArmComponent->SocketOffset = InitialConfig.StateBase.Offset.SocketOffset;
        CurrentSmoothedSocketOffset = InitialConfig.StateBase.Offset.SocketOffset;
        SocketOffsetTransitionTargetValue = CurrentSmoothedSocketOffset;
        UE_LOG(LogTemp, Warning, TEXT("InitCamera: Applied SocketOffset from Explore_Default: (%.1f, %.1f, %.1f)"), SpringArmComponent->SocketOffset.X, SpringArmComponent->SocketOffset.Y, SpringArmComponent->SocketOffset.Z);
    }

    // 存储初始值
    PreviousFocusPoint = CurrentOutput.FocusPoint;
    PreviousRotation = CurrentOutput.Rotation;
    PreviousDistance = CurrentOutput.Distance;
    PreviousFOV = CurrentOutput.FOV;

    // ════════════════════════════════════════════════════════════════
    // 标记初始化完成
    // ════════════════════════════════════════════════════════════════
    bIsInitialized = true;

    // ╔══════════════════════════════════════════════════════════════╗
    // ║                    调试日志 - 初始化完成                      ║
    // ╚══════════════════════════════════════════════════════════════╝
    UE_LOG(LogSoulsCamera, Warning, TEXT(" "));
    UE_LOG(LogSoulsCamera, Warning, TEXT("======================================================================"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("           InitializeCamera COMPLETE                                  "));
    UE_LOG(LogSoulsCamera, Warning, TEXT("======================================================================"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("Final Status:"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  bIsInitialized: TRUE"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  Architecture: %s"), bUseNewConfigArchitecture ? TEXT("New (ConfigService)") : TEXT("Legacy"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  StateMachine Ready: %s"), (StateMachine && StateMachine->IsInitialized()) ? TEXT("YES") : TEXT("NO <<<"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  Current State: %s"), StateMachine ? *StateMachine->GetCurrentStateName().ToString() : TEXT("N/A"));
    UE_LOG(LogSoulsCamera, Warning, TEXT("  Distance: %.1f"), CurrentOutput.Distance);
    UE_LOG(LogSoulsCamera, Warning, TEXT("  FOV: %.1f"), CurrentOutput.FOV);
    UE_LOG(LogSoulsCamera, Warning, TEXT("======================================================================"));
}

bool USoulsCameraManager::IsInitialized() const
{
    return bIsInitialized;
}

void USoulsCameraManager::SetSpringArmComponent(USpringArmComponent* InSpringArm)
{
    SpringArmComponent = InSpringArm;
}

void USoulsCameraManager::SetCameraComponent(UCameraComponent* InCamera)
{
    CameraComponent = InCamera;
}

USpringArmComponent* USoulsCameraManager::GetSpringArm() const
{
    return SpringArmComponent;
}

UCameraComponent* USoulsCameraManager::GetCamera() const
{
    return CameraComponent;
}

bool USoulsCameraManager::RequestStateChange(FName NewStateName, bool bForce)
{
    if (StateMachine)
    {
        bool bResult = StateMachine->RequestStateChange(NewStateName, bForce);
        
        if (bResult)
        {
            // Track manual state: any state other than Explore_Default is considered "manual"
            // Explore_Default means returning to native SpringArm behavior
            static const FName ExploreDefaultName = FName("Explore_Default");
            
            if (NewStateName == ExploreDefaultName)
            {
                bManualStateActive = false;
                ManualStateName = NAME_None;
            }
            else
            {
                bManualStateActive = true;
                ManualStateName = NewStateName;
            }
        }
        
        return bResult;
    }
    return false;
}

FName USoulsCameraManager::GetCurrentStateName() const
{
    if (StateMachine)
    {
        return StateMachine->GetCurrentStateName();
    }
    return NAME_None;
}

ECameraStateCategory USoulsCameraManager::GetCurrentCategory() const
{
    if (StateMachine)
    {
        return StateMachine->GetCurrentCategory();
    }
    return ECameraStateCategory::None;
}

bool USoulsCameraManager::IsInTransition() const
{
    if (StateMachine)
    {
        return StateMachine->IsInTransition();
    }
    return false;
}

void USoulsCameraManager::TriggerModifier(ECameraModifierType ModifierType, float Intensity)
{
    if (ModifierManager)
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("TriggerModifier: Type %d with intensity %.2f"), 
            static_cast<int32>(ModifierType), Intensity);
    }
}

void USoulsCameraManager::StopModifier(ECameraModifierType ModifierType)
{
    if (ModifierManager)
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("StopModifier: Type %d"), 
            static_cast<int32>(ModifierType));
    }
}

void USoulsCameraManager::StopAllModifiers()
{
    if (ModifierManager)
    {
        ModifierManager->StopAllModifiers(false);
        UE_LOG(LogSoulsCamera, Log, TEXT("StopAllModifiers called"));
    }
}

//========================================
// Modifier System Convenience Interface
//========================================

void USoulsCameraManager::TriggerCameraShake(ECameraModifierID ShakeID, float Intensity)
{
    if (ModifierManager)
    {
        ModifierManager->TriggerModifierSimple(ShakeID, Intensity);
        UE_LOG(LogSoulsCamera, Verbose, TEXT("TriggerCameraShake: %s with intensity %.2f"),
            *UEnum::GetValueAsString(ShakeID), Intensity);
    }
}

void USoulsCameraManager::TriggerHitReaction(bool bHeavyHit, float Intensity, FVector HitDirection)
{
    if (ModifierManager)
    {
        ModifierManager->TriggerHitReaction(bHeavyHit, Intensity, HitDirection);
        UE_LOG(LogSoulsCamera, Verbose, TEXT("TriggerHitReaction: %s hit, intensity %.2f"),
            bHeavyHit ? TEXT("heavy") : TEXT("light"), Intensity);
    }
}

void USoulsCameraManager::TriggerSlowMotion(float Duration, float TimeDilation)
{
    if (ModifierManager)
    {
        ModifierManager->TriggerSlowMotion(Duration, TimeDilation);
        UE_LOG(LogSoulsCamera, Verbose, TEXT("TriggerSlowMotion: duration=%.2f, dilation=%.2f"),
            Duration, TimeDilation);
    }
}

void USoulsCameraManager::TriggerHitStop(float Intensity)
{
    if (ModifierManager)
    {
        ModifierManager->TriggerHitStop(Intensity);
        UE_LOG(LogSoulsCamera, Verbose, TEXT("TriggerHitStop: intensity=%.2f"), Intensity);
    }
}

void USoulsCameraManager::TriggerLowHealthEffect(float HealthPercent)
{
    if (ModifierManager)
    {
        ModifierManager->TriggerLowHealthEffect(HealthPercent);
        UE_LOG(LogSoulsCamera, Verbose, TEXT("TriggerLowHealthEffect: health=%.0f%%"),
            HealthPercent * 100.0f);
    }
}

void USoulsCameraManager::StopLowHealthEffect()
{
    if (ModifierManager)
    {
        ModifierManager->StopModifier(ECameraModifierID::Modifier_E01_Effect_LowHealth, false);
        UE_LOG(LogSoulsCamera, Verbose, TEXT("StopLowHealthEffect"));
    }
}

void USoulsCameraManager::TriggerDeathCamera(FVector DeathLocation)
{
    if (ModifierManager)
    {
        FModifierTriggerData TriggerData;
        TriggerData.Intensity = 1.0f;
        TriggerData.SourceLocation = DeathLocation;
        
        ModifierManager->TriggerModifier(ECameraModifierID::Modifier_X03_Special_DeathCam, TriggerData);
        UE_LOG(LogSoulsCamera, Log, TEXT("TriggerDeathCamera: at (%.0f, %.0f, %.0f)"),
            DeathLocation.X, DeathLocation.Y, DeathLocation.Z);
    }
}

void USoulsCameraManager::SetLockOnTarget(AActor* Target)
{
    // ★ 首次锁定时保存原始 LAG 设置（通常由 BeginPlay 预先执行）
    if (Target && SpringArmComponent && !bHasSavedOriginalLagSettings)
    {
        bOriginalEnableCameraLag = SpringArmComponent->bEnableCameraLag;
        OriginalCameraLagSpeed = SpringArmComponent->CameraLagSpeed;
        bOriginalEnableCameraRotationLag = SpringArmComponent->bEnableCameraRotationLag;
        OriginalCameraRotationLagSpeed = SpringArmComponent->CameraRotationLagSpeed;
        OriginalCameraLagMaxDistance = SpringArmComponent->CameraLagMaxDistance;
        bHasSavedOriginalLagSettings = true;
        
        UE_LOG(LogSoulsCamera, Log, TEXT("SetLockOnTarget: Saved original LAG settings"));
    }
    
    // ★ 锁定瞬间保存当前 FOV 和 Distance，用于平滑过渡
    if (Target && CameraComponent && SpringArmComponent)
    {
        CurrentSmoothedFOV = CameraComponent->FieldOfView;
        CurrentSmoothedDistance = SpringArmComponent->TargetArmLength;
        FOVTransitionTargetValue = CurrentSmoothedFOV;
        DistanceTransitionTargetValue = CurrentSmoothedDistance;
        
        // ★ 新增：保存当前 SocketOffset，避免首次锁定时跳动
        CurrentSmoothedSocketOffset = SpringArmComponent->SocketOffset;
        SocketOffsetTransitionTargetValue = CurrentSmoothedSocketOffset;
        
        bHasInitializedTransitionValues = true;
        
        UE_LOG(LogTemp, Warning, TEXT("LockOn INIT: Saved FOV=%.1f, Distance=%.1f, SocketOffset=(%.1f, %.1f, %.1f)"), 
            CurrentSmoothedFOV, CurrentSmoothedDistance,
            CurrentSmoothedSocketOffset.X, CurrentSmoothedSocketOffset.Y, CurrentSmoothedSocketOffset.Z);
    }

    
    // ★ 每次锁定时都执行：同步 ControlRotation 和保存当前 Distance
    if (Target && SpringArmComponent)
    {
        // 同步 ControlRotation 到当前相机旋转，避免切换控制权时跳动
        APawn* OwnerPawn = Cast<APawn>(GetOwner());
        if (OwnerPawn)
        {
            APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
            if (PC)
            {
                // ★ DEBUG: 记录锁定前的状态
                FRotator BeforeControlRotation = PC->GetControlRotation();
                FRotator CurrentCameraRotation = SpringArmComponent->GetComponentRotation();
                
                UE_LOG(LogSoulsCamera, Warning, TEXT("===== LOCK ON INPUT MOMENT ====="));
                UE_LOG(LogSoulsCamera, Warning, TEXT("BEFORE: ControlRotation P=%.1f Y=%.1f"),
                    BeforeControlRotation.Pitch, BeforeControlRotation.Yaw);
                UE_LOG(LogSoulsCamera, Warning, TEXT("BEFORE: SpringArm Rotation P=%.1f Y=%.1f"),
                    CurrentCameraRotation.Pitch, CurrentCameraRotation.Yaw);
                
                // ★★★ 修复：不再用 SpringArm 旋转覆盖 ControlRotation ★★★
                // 保持当前的 ControlRotation 不变，让 Pipeline 的 R03 模块平滑过渡到锁定视角
                // 旧代码: PC->SetControlRotation(CurrentCameraRotation);
                // 新代码: 保持原有的 ControlRotation 不变，不作任何覆盖
                
                // ★ DEBUG: 记录锁定后的状态
                FRotator AfterControlRotation = PC->GetControlRotation();
                UE_LOG(LogSoulsCamera, Warning, TEXT("AFTER: ControlRotation P=%.1f Y=%.1f"),
                    AfterControlRotation.Pitch, AfterControlRotation.Yaw);
                UE_LOG(LogSoulsCamera, Warning, TEXT("DELTA: ControlRotation changed by P=%.1f Y=%.1f"),
                    FMath::Abs(AfterControlRotation.Pitch - BeforeControlRotation.Pitch),
                    FMath::Abs(AfterControlRotation.Yaw - BeforeControlRotation.Yaw));
                UE_LOG(LogSoulsCamera, Warning, TEXT("================================="));
                
                UE_LOG(LogSoulsCamera, Log, TEXT("SetLockOnTarget: Synced ControlRotation to Camera (Pitch=%.1f, Yaw=%.1f, Roll=%.1f)"),
                    CurrentCameraRotation.Pitch, CurrentCameraRotation.Yaw, CurrentCameraRotation.Roll);
                // ★ DEBUG: 记录锁定时的相机状态
                UE_LOG(LogSoulsCamera, Warning, TEXT("DEBUG LockOn: SpringArm Distance=%.1f, SocketOffset=(%.1f,%.1f,%.1f)"),
                    SpringArmComponent->TargetArmLength,
                    SpringArmComponent->SocketOffset.X,
                    SpringArmComponent->SocketOffset.Y,
                    SpringArmComponent->SocketOffset.Z);
            }
        }
        
        // 保存当前 TargetArmLength，确保锁定时 Distance 从当前值平滑过渡
        CurrentOutput.Distance = SpringArmComponent->TargetArmLength;
        
        UE_LOG(LogSoulsCamera, Log, TEXT("SetLockOnTarget: Saved current Distance = %.1f for smooth transition"),
            SpringArmComponent->TargetArmLength);
    }
    
    LockOnTarget = Target;
    
    UE_LOG(LogTemp, Error, TEXT("SetLockOnTarget: Smoothed=(%.1f,%.1f,%.1f) SpringArm=(%.1f,%.1f,%.1f)"), CurrentSmoothedSocketOffset.X, CurrentSmoothedSocketOffset.Y, CurrentSmoothedSocketOffset.Z, SpringArmComponent ? SpringArmComponent->SocketOffset.X : 0.f, SpringArmComponent ? SpringArmComponent->SocketOffset.Y : 0.f, SpringArmComponent ? SpringArmComponent->SocketOffset.Z : 0.f);
    
    if (Target)
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("Lock-on target set to %s"), *Target->GetName());
    }
    else
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("Lock-on target cleared"));
    }
}

void USoulsCameraManager::OnCameraStateChanged(FName OldState, FName NewState)
{
    // 如果状态相同，不需要初始化过渡值
    if (OldState == NewState)
    {
        return;
    }
    
    // 状态切换时保存当前相机参数，用于平滑过渡
    if (!CameraComponent || !SpringArmComponent)
    {
        return;
    }
    
    // 保存当前 FOV
    if (!bFOVTransitionActive)
    {
        CurrentSmoothedFOV = CameraComponent->FieldOfView;
        FOVTransitionTargetValue = CurrentSmoothedFOV;
    }
    
    // 保存当前 Distance
    if (!bDistanceTransitionActive)
    {
        CurrentSmoothedDistance = SpringArmComponent->TargetArmLength;
        DistanceTransitionTargetValue = CurrentSmoothedDistance;
    }
    
    // 保存当前 SocketOffset
    if (!bSocketOffsetTransitionActive)
    {
        CurrentSmoothedSocketOffset = SpringArmComponent->SocketOffset;
        SocketOffsetTransitionTargetValue = CurrentSmoothedSocketOffset;
    }
    
    // 保存当前 TargetOffset
    if (!bTargetOffsetTransitionActive)
    {
        CurrentSmoothedTargetOffset = SpringArmComponent->TargetOffset;
        TargetOffsetTransitionTargetValue = CurrentSmoothedTargetOffset;
    }
    
    // 标记已初始化
    bHasInitializedTransitionValues = true;

    // DEBUG: 追踪状态切换时的值
    UE_LOG(LogTemp, Warning, TEXT("DEBUG OnStateChanged: NewState=%s, SpringArmLength=%f, SmoothedDist=%f"),
        *NewState.ToString(),
        SpringArmComponent ? SpringArmComponent->TargetArmLength : 0.0f,
        CurrentSmoothedDistance);
    
    if (bEnableTransitionDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraStateChanged: %s -> %s | FOV=%.1f, Distance=%.1f"), 
            *OldState.ToString(), *NewState.ToString(), CurrentSmoothedFOV, CurrentSmoothedDistance);
    }
}

void USoulsCameraManager::ClearLockOnTarget()
{
    // ★ 新增：恢复原始 LAG 设置
    if (SpringArmComponent && bHasSavedOriginalLagSettings)
    {
        SpringArmComponent->bEnableCameraLag = bOriginalEnableCameraLag;
        SpringArmComponent->CameraLagSpeed = OriginalCameraLagSpeed;
        SpringArmComponent->bEnableCameraRotationLag = bOriginalEnableCameraRotationLag;
        SpringArmComponent->CameraRotationLagSpeed = OriginalCameraRotationLagSpeed;
        SpringArmComponent->CameraLagMaxDistance = OriginalCameraLagMaxDistance;
        
        // ★ 解锁时：同步 ControlRotation 到当前相机旋转，避免切换控制权时跳动
        APawn* OwnerPawn = Cast<APawn>(GetOwner());
        if (OwnerPawn)
        {
            APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
            if (PC)
            {
                // 获取当前相机旋转
                FRotator CurrentCameraRotation = SpringArmComponent->GetComponentRotation();
                // 同步到 ControlRotation
                PC->SetControlRotation(CurrentCameraRotation);
                
                UE_LOG(LogSoulsCamera, Log, TEXT("ClearLockOnTarget: Synced ControlRotation to Camera (Pitch=%.1f, Yaw=%.1f, Roll=%.1f)"),
                    CurrentCameraRotation.Pitch, CurrentCameraRotation.Yaw, CurrentCameraRotation.Roll);
            }
        }
        
        // ★ 新增：恢复 bUsePawnControlRotation，使相机跟随控制器输入
        SpringArmComponent->bUsePawnControlRotation = true;
        
        UE_LOG(LogSoulsCamera, Log, TEXT("ClearLockOnTarget: Restored LAG and bUsePawnControlRotation=true"));
    }
    
    SetLockOnTarget(nullptr);
}

//========================================
// RestoreNativeSpringArmSettings
// 恢复原生 SpringArm 设置
//========================================
void USoulsCameraManager::RestoreNativeSpringArmSettings()
{
    if (!SpringArmComponent || !CameraComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("DEBUG RestoreNativeSpringArmSettings - Components not valid"));
        return;
    }
    
    // 恢复到原生默认值（与蓝图中 SpringArm 的默认设置一致）
    const float NativeDistance = 300.0f;
    const float NativeFOV = 90.0f;
    
    // ★★★ 问题5修复：不直接赋值，而是启动平滑过渡 ★★★
    // 旧代码（直接赋值导致跳变）：
    // SpringArmComponent->TargetArmLength = NativeDistance;
    // CameraComponent->SetFieldOfView(NativeFOV);
    
    // 新代码：启动 Distance 退出过渡
    if (FMath::Abs(NativeDistance - CurrentSmoothedDistance) > 1.0f)
    {
        DistanceTransitionStartValue = CurrentSmoothedDistance;
        DistanceTransitionTargetValue = NativeDistance;
        DistanceTransitionElapsedTime = 0.0f;
        bDistanceTransitionActive = true;
        
        UE_LOG(LogTemp, Warning, TEXT("RestoreNative: Distance Transition START: %.1f -> %.1f"), 
            CurrentSmoothedDistance, NativeDistance);
    }
    
    // 新代码：启动 FOV 退出过渡
    if (FMath::Abs(NativeFOV - CurrentSmoothedFOV) > 0.1f)
    {
        FOVTransitionStartValue = CurrentSmoothedFOV;
        FOVTransitionTargetValue = NativeFOV;
        FOVTransitionElapsedTime = 0.0f;
        bFOVTransitionActive = true;
        
        UE_LOG(LogTemp, Warning, TEXT("RestoreNative: FOV Transition START: %.1f -> %.1f"), 
            CurrentSmoothedFOV, NativeFOV);
    }
    
    // ★ SocketOffset 也需要平滑过渡回默认值
    // ★★★ 修复：从 Explore_Default 配置读取正确的 SocketOffset，而不是硬编码 (0,0,0) ★★★
    FVector NativeSocketOffset = FVector(0.0f, 50.0f, 0.0f);  // 默认值改为 (0,50,0)
    FCameraStateConfig ExploreConfig;
    if (ConfigService && ConfigService->GetStateConfig(FName("Explore_Default"), ExploreConfig))
    {
        NativeSocketOffset = ExploreConfig.StateBase.Offset.SocketOffset;
    }
    if (FVector::Dist(NativeSocketOffset, CurrentSmoothedSocketOffset) > 1.0f)
    {
        SocketOffsetTransitionStartValue = CurrentSmoothedSocketOffset;
        SocketOffsetTransitionTargetValue = NativeSocketOffset;
        SocketOffsetTransitionElapsedTime = 0.0f;
        bSocketOffsetTransitionActive = true;
    }
    
    // 恢复玩家控制旋转
    SpringArmComponent->bUsePawnControlRotation = true;
    
    UE_LOG(LogTemp, Warning, TEXT("DEBUG RestoreNativeSpringArmSettings - Started exit transitions: Distance=%.1f->%.1f, FOV=%.1f->%.1f, bUsePawnControlRotation=true"), 
        CurrentSmoothedDistance, NativeDistance, CurrentSmoothedFOV, NativeFOV);
}

//========================================
// ApplyExitTransition
// 退出过渡辅助函数（问题5修复 - 第三部分）
//========================================
void USoulsCameraManager::ApplyExitTransition(float DeltaTime)
{
    // Step 10: Distance 过渡逻辑
    if (bDistanceTransitionActive)
    {
        DistanceTransitionElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(DistanceTransitionElapsedTime / DistanceTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedDistance = FMath::Lerp(DistanceTransitionStartValue, DistanceTransitionTargetValue, Alpha);
        SpringArmComponent->TargetArmLength = CurrentSmoothedDistance;
        if (Alpha >= 1.0f) { bDistanceTransitionActive = false; }
    }
    
    // Step 11: FOV 过渡逻辑
    if (bFOVTransitionActive)
    {
        FOVTransitionElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(FOVTransitionElapsedTime / FOVTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedFOV = FMath::Lerp(FOVTransitionStartValue, FOVTransitionTargetValue, Alpha);
        CameraComponent->SetFieldOfView(CurrentSmoothedFOV);
        if (Alpha >= 1.0f) { bFOVTransitionActive = false; }
    }
    
    // Step 12: SocketOffset 过渡逻辑
    if (bSocketOffsetTransitionActive)
    {
        SocketOffsetTransitionElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(SocketOffsetTransitionElapsedTime / SocketOffsetTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedSocketOffset = FMath::Lerp(SocketOffsetTransitionStartValue, SocketOffsetTransitionTargetValue, Alpha);
        SpringArmComponent->SocketOffset = CurrentSmoothedSocketOffset;
        if (Alpha >= 1.0f) { bSocketOffsetTransitionActive = false; }
    }
}

AActor* USoulsCameraManager::GetLockOnTarget() const
{
    return LockOnTarget.Get();
}

bool USoulsCameraManager::HasLockOnTarget() const
{
    return LockOnTarget.IsValid();
}

FSoulsCameraOutput USoulsCameraManager::GetCurrentOutput() const
{
    return CurrentOutput;
}

FCameraInputContext USoulsCameraManager::GetCurrentInputContext() const
{
    return CurrentInputContext;
}

//========================================
// Convenience Query Functions Implementation
//========================================

FVector USoulsCameraManager::GetCurrentCameraLocation() const
{
    return CurrentOutput.GetCameraLocation();
}

FRotator USoulsCameraManager::GetCurrentCameraRotation() const
{
    return CurrentOutput.Rotation;
}

FVector USoulsCameraManager::GetCurrentFocusPoint() const
{
    return CurrentOutput.FocusPoint;
}

float USoulsCameraManager::GetCurrentDistance() const
{
    return CurrentOutput.Distance;
}

float USoulsCameraManager::GetCurrentFOV() const
{
    return CurrentOutput.FOV;
}

FString USoulsCameraManager::GetDebugString() const
{
    FString Result;

    Result += TEXT("=== Souls Camera Debug ===\n");
    Result += FString::Printf(TEXT("State: %s\n"), *GetCurrentStateName().ToString());
    Result += FString::Printf(TEXT("Category: %s\n"), *UEnum::GetValueAsString(GetCurrentCategory()));
    Result += FString::Printf(TEXT("In Transition: %s\n"), IsInTransition() ? TEXT("Yes") : TEXT("No"));
    Result += TEXT("\n");
    
    Result += TEXT("[Transform]\n");
    Result += FString::Printf(TEXT("Focus: (%.1f, %.1f, %.1f)\n"), 
        CurrentOutput.FocusPoint.X, CurrentOutput.FocusPoint.Y, CurrentOutput.FocusPoint.Z);
    Result += FString::Printf(TEXT("Rotation: (P: %.1f, Y: %.1f, R: %.1f)\n"), 
        CurrentOutput.Rotation.Pitch, CurrentOutput.Rotation.Yaw, CurrentOutput.Rotation.Roll);
    Result += FString::Printf(TEXT("Distance: %.2f\n"), CurrentOutput.Distance);
    Result += FString::Printf(TEXT("FOV: %.2f\n"), CurrentOutput.FOV);
    Result += TEXT("\n");
    
    Result += TEXT("[Lock-On]\n");
    Result += FString::Printf(TEXT("Has Target: %s\n"), HasLockOnTarget() ? TEXT("Yes") : TEXT("No"));
    if (HasLockOnTarget())
    {
        AActor* Target = GetLockOnTarget();
        Result += FString::Printf(TEXT("Target: %s\n"), Target ? *Target->GetName() : TEXT("Invalid"));
    }
    Result += TEXT("\n");
    
    Result += TEXT("[Collision]\n");
    Result += FString::Printf(TEXT("Active: %s\n"), IsCollisionActive() ? TEXT("Yes") : TEXT("No"));
    Result += FString::Printf(TEXT("Recovery: %s\n"), IsInCollisionRecovery() ? TEXT("Yes") : TEXT("No"));
    Result += FString::Printf(TEXT("Character Occluded: %s\n"), IsCharacterOccluded() ? TEXT("Yes") : TEXT("No"));
    Result += FString::Printf(TEXT("Target Occluded: %s\n"), IsTargetOccluded() ? TEXT("Yes") : TEXT("No"));
    Result += FString::Printf(TEXT("Adjusted Distance: %.2f\n"), GetCollisionAdjustedDistance());
    Result += TEXT("\n");
    
    Result += TEXT("[System]\n");
    Result += FString::Printf(TEXT("Frame: %llu\n"), FrameCounter);
    Result += FString::Printf(TEXT("Modules: %d\n"), ModuleRegistry ? ModuleRegistry->GetModuleCount() : 0);
    Result += FString::Printf(TEXT("Active Modifiers: %d\n"), ModifierManager ? ModifierManager->GetActiveModifierCount() : 0);

    return Result;
}

void USoulsCameraManager::SetDebugEnabled(bool bEnabled)
{
    bDebugEnabled = bEnabled;
}

bool USoulsCameraManager::IsDebugEnabled() const
{
    return bDebugEnabled;
}

USoulsCameraStateMachine* USoulsCameraManager::GetStateMachine() const
{
    return StateMachine;
}

UCameraModuleRegistry* USoulsCameraManager::GetModuleRegistry() const
{
    return ModuleRegistry;
}

UCameraModifierManager* USoulsCameraManager::GetModifierManager() const
{
    return ModifierManager;
}

UCameraCollisionResolver* USoulsCameraManager::GetCollisionResolver() const
{
    return CollisionResolver;
}

//========================================
// Collision System Interface Implementation
//========================================

bool USoulsCameraManager::IsCollisionActive() const
{
    if (CollisionResolver)
    {
        return CollisionResolver->IsCollisionActive();
    }
    return false;
}

bool USoulsCameraManager::IsInCollisionRecovery() const
{
    if (CollisionResolver)
    {
        return CollisionResolver->IsInRecovery();
    }
    return false;
}

bool USoulsCameraManager::IsCharacterOccluded() const
{
    if (CollisionResolver)
    {
        const FCollisionState& State = CollisionResolver->GetCollisionState();
        return State.bCharacterOccluded;
    }
    return false;
}

bool USoulsCameraManager::IsTargetOccluded() const
{
    if (CollisionResolver)
    {
        const FCollisionState& State = CollisionResolver->GetCollisionState();
        return State.bTargetOccluded;
    }
    return false;
}

float USoulsCameraManager::GetCollisionAdjustedDistance() const
{
    if (CollisionResolver)
    {
        const FCollisionState& State = CollisionResolver->GetCollisionState();
        return State.CurrentDistance;
    }
    return 0.0f;
}

void USoulsCameraManager::SetCollisionStrategyEnabled(ECollisionStrategyID StrategyType, bool bEnabled)
{
    if (CollisionResolver)
    {
        UCameraCollisionBase* Strategy = CollisionResolver->GetStrategy(StrategyType);
        if (Strategy)
        {
            Strategy->SetEnabled(bEnabled);
            
            UE_LOG(LogSoulsCamera, Verbose, TEXT("SetCollisionStrategyEnabled: %s %s"),
                *UEnum::GetValueAsString(StrategyType),
                bEnabled ? TEXT("enabled") : TEXT("disabled"));
        }
    }
}

void USoulsCameraManager::SetCollisionDebugEnabled(bool bEnabled)
{
    if (CollisionResolver)
    {
        CollisionResolver->SetDebugEnabled(bEnabled);
        
        UE_LOG(LogSoulsCamera, Log, TEXT("SetCollisionDebugEnabled: %s"),
            bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

void USoulsCameraManager::AutoFindComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Find SpringArm
    if (!SpringArmComponent)
    {
        SpringArmComponent = Owner->FindComponentByClass<USpringArmComponent>();
        if (SpringArmComponent)
        {
            UE_LOG(LogSoulsCamera, Log, TEXT("Auto-found SpringArmComponent"));
        }
    }

    // Find Camera
    if (!CameraComponent)
    {
        CameraComponent = Owner->FindComponentByClass<UCameraComponent>();
        if (CameraComponent)
        {
            UE_LOG(LogSoulsCamera, Log, TEXT("Auto-found CameraComponent"));
        }
    }
}

void USoulsCameraManager::CreateSubSystems()
{
    UE_LOG(LogSoulsCamera, Log, TEXT("CreateSubSystems: Creating camera sub-systems..."));

    //========================================
    // Create State Machine
    //========================================
    
    StateMachine = NewObject<USoulsCameraStateMachine>(this);
    if (StateMachine)
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateSubSystems: StateMachine created"));
    }

    //========================================
    // Create Module Registry (Phase 3)
    //========================================
    
    ModuleRegistry = NewObject<UCameraModuleRegistry>(this);
    if (ModuleRegistry)
    {
        ModuleRegistry->Initialize(this);
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateSubSystems: ModuleRegistry created with %d modules"), 
            ModuleRegistry->GetModuleCount());
    }

    //========================================
    // Create Modifier Manager (Phase 4)
    //========================================

    ModifierManager = NewObject<UCameraModifierManager>(this);
    if (ModifierManager)
    {
        ModifierManager->Initialize(this);
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateSubSystems: ModifierManager created with %d modifiers"), 
            ModifierManager->GetModifierCount());
    }

    //========================================
    // Create Collision Resolver (Phase 5)
    //========================================

    CollisionResolver = NewObject<UCameraCollisionResolver>(this);
    if (CollisionResolver)
    {
        CollisionResolver->Initialize(this);
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateSubSystems: CollisionResolver created with %d strategies"), 
            CollisionResolver->GetStrategyCount());

    }

    //========================================
    // Create Pipeline
    //========================================
    
    Pipeline = NewObject<USoulsCameraPipeline>(this);
    if (Pipeline)
    {
        Pipeline->Initialize(this);
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateSubSystems: Pipeline created"));
    }

    //========================================
    // Create and Register All 8 Stages
    //========================================
    
    CreateAndRegisterStages();

    UE_LOG(LogSoulsCamera, Log, TEXT("CreateSubSystems: All sub-systems created successfully"));
    UE_LOG(LogSoulsCamera, Log, TEXT("  - StateMachine: Ready"));
    UE_LOG(LogSoulsCamera, Log, TEXT("  - ModuleRegistry: %d modules"), 
        ModuleRegistry ? ModuleRegistry->GetModuleCount() : 0);
    UE_LOG(LogSoulsCamera, Log, TEXT("  - ModifierManager: %d modifiers"), 
        ModifierManager ? ModifierManager->GetModifierCount() : 0);
    UE_LOG(LogSoulsCamera, Log, TEXT("  - CollisionResolver: %d strategies"), 
        CollisionResolver ? CollisionResolver->GetStrategyCount() : 0);
    UE_LOG(LogSoulsCamera, Log, TEXT("  - Pipeline: 8 stages"));
}

void USoulsCameraManager::GatherInputContext(float DeltaTime)
{
    // Reset context to defaults
    CurrentInputContext.Reset();
    
    //========================================
    // Timing
    //========================================
    CurrentInputContext.DeltaTime = DeltaTime;
    CurrentInputContext.GameTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    //========================================
    // Character State
    //========================================
    CurrentInputContext.CharacterLocation = Owner->GetActorLocation();
    CurrentInputContext.CharacterRotation = Owner->GetActorRotation();
    CurrentInputContext.CharacterActor = Owner;

    // Velocity and movement state from Character
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        CurrentInputContext.CharacterVelocity = Character->GetVelocity();
        CurrentInputContext.CharacterSpeed = CurrentInputContext.CharacterVelocity.Size2D();
        CurrentInputContext.bIsMoving = CurrentInputContext.CharacterSpeed > 10.0f;
        CurrentInputContext.bIsCrouching = Character->bIsCrouched;
        
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            CurrentInputContext.bIsInAir = MovementComp->IsFalling();
            CurrentInputContext.bIsSprinting = MovementComp->MaxWalkSpeed > 600.0f; // Simplified check
            CurrentInputContext.bIsSwimming = MovementComp->IsSwimming();
        }
    }

    //========================================
    // Player Input
    // ★★★ 从正确的 InputComponent 读取输入轴 ★★★
    // 输入轴绑定在 Character（Owner）上，不是 PlayerController 上
    //========================================
    
    // 方法 1：从 Owner（Character/Pawn）获取输入
    APawn* OwnerPawn = Cast<APawn>(Owner);
    if (OwnerPawn)
    {
        // 使用 GetInputAxisValue - 这会从 Pawn 的 InputComponent 读取
        float YawInput = OwnerPawn->GetInputAxisValue(TEXT("Turn"));
        float PitchInput = OwnerPawn->GetInputAxisValue(TEXT("LookUp"));
        
        // 如果还需要 TurnRate 和 LookUpRate（手柄输入）
        float TurnRate = OwnerPawn->GetInputAxisValue(TEXT("TurnRate"));
        float LookUpRate = OwnerPawn->GetInputAxisValue(TEXT("LookUpRate"));
        
        // 合并鼠标和手柄输入
        if (FMath::Abs(TurnRate) > KINDA_SMALL_NUMBER)
        {
            YawInput = TurnRate;
        }
        if (FMath::Abs(LookUpRate) > KINDA_SMALL_NUMBER)
        {
            PitchInput = LookUpRate;
        }
        
        CurrentInputContext.CameraInput = FVector2D(YawInput, PitchInput);
        CurrentInputContext.InputMagnitude = CurrentInputContext.CameraInput.Size();
    }
    
    // 方法 2（备用）：如果上面不工作，使用鼠标增量
    if (APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController()))
    {
        float MouseX, MouseY;
        PC->GetInputMouseDelta(MouseX, MouseY);
        
        // 如果鼠标有输入且之前没有从Pawn获取到输入，使用鼠标值
        if ((FMath::Abs(MouseX) > KINDA_SMALL_NUMBER || FMath::Abs(MouseY) > KINDA_SMALL_NUMBER) &&
            CurrentInputContext.InputMagnitude < KINDA_SMALL_NUMBER)
        {
            CurrentInputContext.CameraInput = FVector2D(MouseX, MouseY);
            CurrentInputContext.InputMagnitude = CurrentInputContext.CameraInput.Size();
        }
        
        // Track time since last input for auto-center features
        if (CurrentInputContext.InputMagnitude > KINDA_SMALL_NUMBER)
        {
            CurrentInputContext.TimeSinceLastInput = 0.0f;
        }
        else
        {
            // Accumulate time - Note: this persists across frames via a separate tracker
            // For now, we just reset it since we don't有 persistent tracking
            CurrentInputContext.TimeSinceLastInput = 0.0f;
        }
    }

    //========================================
    // Lock-On Target
    //========================================
    CurrentInputContext.bHasTarget = LockOnTarget.IsValid();
    
    // ★★★ 调试日志：确认锁定目标信息被正确传递 ★★★
    static int32 LockOnDebugCount = 0;
    if (LockOnTarget.IsValid() && LockOnDebugCount < 30)
    {
        LockOnDebugCount++;
        UE_LOG(LogSoulsCamera, Warning, TEXT(""));
        UE_LOG(LogSoulsCamera, Warning, TEXT("★★★ GatherInputContext - LockOn #%d ★★★"), LockOnDebugCount);
        UE_LOG(LogSoulsCamera, Warning, TEXT("  LockOnTarget: %s"), *LockOnTarget.Get()->GetName());
        UE_LOG(LogSoulsCamera, Warning, TEXT("  TargetLocation:  (%.1f, %.1f, %.1f)"),
            LockOnTarget.Get()->GetActorLocation().X,
            LockOnTarget.Get()->GetActorLocation().Y,
            LockOnTarget.Get()->GetActorLocation().Z);
        UE_LOG(LogSoulsCamera, Warning, TEXT("  bHasTarget:  TRUE"));
    }
    
    if (CurrentInputContext.bHasTarget)
    {
        AActor* Target = LockOnTarget.Get();
        CurrentInputContext.TargetActor = Target;
        CurrentInputContext.TargetLocation = Target->GetActorLocation();
        CurrentInputContext.TargetDistance = FVector::Dist(
            CurrentInputContext.CharacterLocation, 
            CurrentInputContext.TargetLocation
        );
        
        // Target size from bounds for proper framing
        FVector Origin, Extent;
        Target->GetActorBounds(false, Origin, Extent);
        CurrentInputContext.TargetSize = Extent.Size();
        
        // Check if target is boss (via tag check)
        CurrentInputContext.bTargetIsBoss = Target->Tags.Contains(FName("Boss"));
    }

    //========================================
    // Previous Frame Data
    //========================================
}

void USoulsCameraManager::StorePreviousFrameData()
{
    // Store current output for next frame interpolation and fallback
    PreviousFocusPoint = CurrentOutput.FocusPoint;
    PreviousRotation = CurrentOutput.Rotation;
    PreviousDistance = CurrentOutput.Distance;
    PreviousFOV = CurrentOutput.FOV;
}

void USoulsCameraManager::ApplyOutput(const FSoulsCameraOutput& Output)
{
    if (!SpringArmComponent || !CameraComponent)
    {
        return;
    }
    
    // Apply to SpringArm
    SpringArmComponent->TargetArmLength = Output.Distance;
    SpringArmComponent->SocketOffset = Output.SocketOffset;
    SpringArmComponent->TargetOffset = Output.TargetOffset;
    
    // Apply rotation (if we have valid data)
    if (!Output.Rotation.IsNearlyZero())
    {
        SpringArmComponent->SetWorldRotation(Output.Rotation);
    }

    // Apply to Camera
    CameraComponent->SetFieldOfView(Output.FOV);
}

void USoulsCameraManager::ApplyFallbackParameters()
{
    if (!SpringArmComponent || !CameraComponent)
    {
        return;
    }

    float TargetDistance = 400.0f;
    float TargetFOV = 70.0f;
    
    if (StateMachine && StateMachine->IsInitialized() && ConfigService)
    {
        FName CurrentState = StateMachine->GetCurrentStateName();
        if (!CurrentState.IsNone())
        {
            FCameraStateConfig StateConfig;
            if (ConfigService->GetStateConfig(CurrentState, StateConfig))
            {
                TargetDistance = StateConfig.StateBase.Distance.BaseDistance;
                TargetFOV = StateConfig.StateBase.FOV.BaseFOV;
                
                // ★添加调试日志
                static bool bWarningPrinted = false;
                if (!bWarningPrinted)
                {
                    UE_LOG(LogSoulsCamera, Warning, TEXT("Fallback parameters based on state config - State: %s, Distance: %.1f, FOV: %.1f"),
                        *CurrentState.ToString(), TargetDistance, TargetFOV);
                    bWarningPrinted = true;
                }
                
                // 安全检查
                if (TargetDistance < 50.0f || TargetDistance > 2000.0f)
                {
                    TargetDistance = 400.0f;
                }
                if (TargetFOV < 30.0f || TargetFOV > 120.0f)
                {
                    TargetFOV = 70.0f;
                }
            }
        }
    }
    
    // 平滑插值
    float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
    
    float CurrentDistance = SpringArmComponent->TargetArmLength;
    float NewDistance = FMath::FInterpTo(CurrentDistance, TargetDistance, DeltaTime, 5.0f);
    SpringArmComponent->TargetArmLength = NewDistance;
    
    float CurrentFOV = CameraComponent->FieldOfView;
    float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 5.0f);
    CameraComponent->SetFieldOfView(NewFOV);

    // 更新输出
    CurrentOutput.Distance = NewDistance;
    CurrentOutput.FOV = NewFOV;
    CurrentOutput.bIsValid = true;
}

void USoulsCameraManager::ApplyOutputToComponents()
{
    // ★ DEBUG: 确认函数被调用
    // UE_LOG(LogTemp, Warning, TEXT("DEBUG ApplyOutputToComponents CALLED"));

    // 编辑器模式下不执行，防止影响编辑器视口
    if (!GetWorld() || !GetWorld()->IsGameWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("DEBUG ApplyOutputToComponents - Editor mode, returning"));
        return;
    }

    // DEBUG: 追踪配置应用时的值
    UE_LOG(LogTemp, Warning, TEXT("DEBUG ApplyOutput:  OutputDist=%f, TargetValue=%f, SmoothedDist=%f"),
        CurrentOutput.Distance, DistanceTransitionTargetValue, CurrentSmoothedDistance);

    // ★ 修改：根据状态类型决定是否应用 Override
    FName CurrentState = StateMachine ? StateMachine->GetCurrentStateName() : NAME_None;
    bool bIsEdgeState = CurrentState.ToString().StartsWith(TEXT("Edge_"));

    // Edge 状态（远眺）：无论是否有锁定目标都应用 Override
    // 其他状态：没有锁定目标时不覆盖原生设置
    if (!bIsEdgeState && !LockOnTarget.IsValid() && !bManualStateActive)
    {
        return;
    }

    // ========== 旋转应用（锁定状态） ==========
    // ★ Step 1: 旋转插值速度变量
    float RotationInterpSpeed = 10.0f;
    
    // ★★★ 修复：锁定后相机旋转不跟随敌人的问题 ★★★
    if (LockOnTarget.IsValid() && SpringArmComponent)
    {
        // 锁定状态下，禁用 Pawn Control Rotation，使用 Pipeline 计算的旋转
        SpringArmComponent->bUsePawnControlRotation = false;
        
        // 获取目标旋转（由 R03 模块计算）
        FRotator TargetRotation = CurrentOutput.Rotation;
        
        // 获取当前 SpringArm 旋转
        FRotator CurrentRotation = SpringArmComponent->GetComponentRotation();
        
        // 平滑插值到目标旋转（避免跳变）
        // 使用可配置的 LockOnRotationLagSpeed（如果有效），否则使用局部默认值
        if (LockOnRotationLagSpeed > 0.0f)
        {
            RotationInterpSpeed = LockOnRotationLagSpeed;
        }
        
        float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
        
        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            TargetRotation,
            DeltaTime,
            RotationInterpSpeed
        );
        
        // 应用旋转到 SpringArm
        SpringArmComponent->SetWorldRotation(NewRotation);
        
        // 同步更新 ControlRotation（保持一致性，解锁时不会跳变）
        if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
        {
            PC->SetControlRotation(NewRotation);
        }
        
        // DEBUG 日志（可选，验证后可删除）
        static int32 RotationApplyDebugCount = 0;
        if (RotationApplyDebugCount < 30)
        {
            RotationApplyDebugCount++;
            UE_LOG(LogTemp, Warning, TEXT("DEBUG ApplyRotation: Target=(%.1f,%.1f), Current=(%.1f,%.1f), New=(%.1f,%.1f)"),
                TargetRotation.Pitch, TargetRotation.Yaw,
                CurrentRotation.Pitch, CurrentRotation.Yaw,
                NewRotation.Pitch, NewRotation.Yaw);
        }
    }

    // ★★★ 从当前状态配置读取并应用 LAG 参数到原生组件 ★★★
    if (ConfigService && StateMachine && StateMachine->IsInitialized())
    {
        CurrentState = StateMachine->GetCurrentStateName();
        if (!CurrentState.IsNone())
        {
            FCameraStateConfig StateConfig;
            if (ConfigService->GetStateConfig(CurrentState, StateConfig))
            {
                // 获取 LAG 参数
                float PositionLagSpeed = StateConfig.StateBase.Lag.PositionLagSpeed;
                float RotationLagSpeed = StateConfig.StateBase.Lag.RotationLagSpeed;
                float DistanceLagSpeed = StateConfig.StateBase.Lag.DistanceLagSpeed;
                float FOVLagSpeed = StateConfig.StateBase.Lag.FOVLagSpeed;
                
                // ★ 修改：锁定状态下使用原生 LAG 设置，保持与探索状态一致的手感
                bool bHasTarget = LockOnTarget.IsValid();
                if (bHasTarget && bHasSavedOriginalLagSettings)
                {
                    // 锁定状态：使用保存的原生 LAG 参数
                    SpringArmComponent->bEnableCameraLag = bOriginalEnableCameraLag;
                    SpringArmComponent->CameraLagSpeed = OriginalCameraLagSpeed;
                    
                    SpringArmComponent->bEnableCameraRotationLag = bOriginalEnableCameraRotationLag;
                    SpringArmComponent->CameraRotationLagSpeed = OriginalCameraRotationLagSpeed;
                }
                else if (PositionLagSpeed > 0.0f)
                {
                    if (!SpringArmComponent->bEnableCameraLag)
                    {
                        SpringArmComponent->bEnableCameraLag = true;
                    }
                    SpringArmComponent->CameraLagSpeed = PositionLagSpeed;
                }
                else
                {
                    SpringArmComponent->bEnableCameraLag = false;
                }
                
                // 应用旋转 LAG 到 SpringArm（仅在非锁定状态下）
                // ★ 修改：锁定状态下使用独立的 LockOnRotationLagSpeed 参数
                if (bHasTarget)
                {
                    // 锁定状态：使用专用的锁定 Rotation LAG 速度
                    if (!SpringArmComponent->bEnableCameraRotationLag)
                    {
                        SpringArmComponent->bEnableCameraRotationLag = true;
                    }
                    SpringArmComponent->CameraRotationLagSpeed = LockOnRotationLagSpeed;
                }
                else if (RotationLagSpeed > 0.0f)
                {
                    if (!SpringArmComponent->bEnableCameraRotationLag)
                    {
                        SpringArmComponent->bEnableCameraRotationLag = true;
                    }
                    SpringArmComponent->CameraRotationLagSpeed = RotationLagSpeed;
                }
                
                // 存储 Distance 和 FOV 的 LAG 速度供插值使用
                // 这些值会在后面的距离和FOV应用代码中使用
                float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
                
                // 应用线性 Distance 过渡
                if (CurrentOutput.Distance > 0.0f)
                {
                    float TargetDistance = CurrentOutput.Distance;
                    
                    // 初始化：从组件获取当前实际值（FOV 和 Distance 一起初始化）
                    if (!bHasInitializedTransitionValues)
                    {
                        // 初始化 Distance
                        CurrentSmoothedDistance = SpringArmComponent->TargetArmLength;
                        DistanceTransitionTargetValue = CurrentSmoothedDistance;
                        
                        // 初始化 FOV
                        CurrentSmoothedFOV = CameraComponent->FieldOfView;
                        FOVTransitionTargetValue = CurrentSmoothedFOV;
                        
                        UE_LOG(LogTemp, Error, TEXT("INIT: FOV=%.1f, Distance=%.1f"), CurrentSmoothedFOV, CurrentSmoothedDistance);
                        
                        bHasInitializedTransitionValues = true;
                    }
                    
                    // 检测目标值变化，开始新的过渡
                    if (FMath::Abs(TargetDistance - DistanceTransitionTargetValue) > 1.0f)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Distance Transition START: %.1f -> %.1f, Duration=%.2f"), CurrentSmoothedDistance, TargetDistance, DistanceTransitionDuration);
                        DistanceTransitionStartValue = CurrentSmoothedDistance;
                        DistanceTransitionTargetValue = TargetDistance;
                        DistanceTransitionElapsedTime = 0.0f;
                        bDistanceTransitionActive = true;
                    }
                    
                    // 线性过渡进行中
    if (bDistanceTransitionActive)
    {
        DistanceTransitionElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(DistanceTransitionElapsedTime / DistanceTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedDistance = FMath::Lerp(DistanceTransitionStartValue, DistanceTransitionTargetValue, Alpha);
        SpringArmComponent->TargetArmLength = CurrentSmoothedDistance;
        
        if (Alpha >= 1.0f) { bDistanceTransitionActive = false; }
    }
    
                    CurrentOutput.Distance = CurrentSmoothedDistance;
                }
                
                // 应用线性 FOV 过渡
                if (CurrentOutput.FOV > 0.0f)
                {
                    // 临时调试：打印每帧的 FOV 和 Distance 值
                    static int32 DebugFrameCounter = 0;
                    DebugFrameCounter++;
                    if (DebugFrameCounter <= 10 || bFOVTransitionActive || bDistanceTransitionActive)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("[Frame %d] FOV: Current=%.1f, Smoothed=%.1f, Target=%.1f, Active=%d | Distance: Current=%.1f, Smoothed=%.1f, Target=%.1f, Active=%d"),
                            DebugFrameCounter,
                            CameraComponent ? CameraComponent->FieldOfView : 0.0f,
                            CurrentSmoothedFOV,
                            CurrentOutput.FOV,
                            bFOVTransitionActive ? 1 : 0,
                            SpringArmComponent ? SpringArmComponent->TargetArmLength : 0.0f,
                            CurrentSmoothedDistance,
                            CurrentOutput.Distance,
                            bDistanceTransitionActive ? 1 : 0
                        );
                    }

                    float TargetFOV = CurrentOutput.FOV;
                    
                    // 检测目标值变化，开始新的过渡
                    if (FMath::Abs(TargetFOV - FOVTransitionTargetValue) > 0.1f)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("FOV Transition START: %.1f -> %.1f, Duration=%.2f"), CurrentSmoothedFOV, TargetFOV, FOVTransitionDuration);
                        FOVTransitionStartValue = CurrentSmoothedFOV;
                        FOVTransitionTargetValue = TargetFOV;
                        FOVTransitionElapsedTime = 0.0f;
                        bFOVTransitionActive = true;
                    }
                    
                    // 线性过渡进行中
    if (bFOVTransitionActive)
    {
        FOVTransitionElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(FOVTransitionElapsedTime / FOVTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedFOV = FMath::Lerp(FOVTransitionStartValue, FOVTransitionTargetValue, Alpha);
        CameraComponent->SetFieldOfView(CurrentSmoothedFOV);
        
        if (Alpha >= 1.0f)
        {
            bFOVTransitionActive = false;
            CurrentSmoothedFOV = FOVTransitionTargetValue;
            UE_LOG(LogTemp, Warning, TEXT("ApplyExitTransition: FOV COMPLETE: %.1f"), CurrentSmoothedFOV);
        }
    }
    
                    CurrentOutput.FOV = CurrentSmoothedFOV;
                }

            }
        }
    }

    // SocketOffset 过渡检测
    {
        static int32 SocketDbgCnt = 0;
        if (SocketDbgCnt < 60) {
            SocketDbgCnt++;
            UE_LOG(LogTemp, Error, TEXT("ApplySocketOffset[%d]: Target=(%.1f,%.1f,%.1f) Smoothed=(%.1f,%.1f,%.1f) IsZero=%d"), SocketDbgCnt, CurrentOutput.SocketOffset.X, CurrentOutput.SocketOffset.Y, CurrentOutput.SocketOffset.Z, CurrentSmoothedSocketOffset.X, CurrentSmoothedSocketOffset.Y, CurrentSmoothedSocketOffset.Z, CurrentSmoothedSocketOffset.IsZero()?1:0);
        }
        
        FVector TargetSocketOffset = CurrentOutput.SocketOffset;
        if (CurrentSmoothedSocketOffset.IsZero() && !TargetSocketOffset.IsZero())
        {
            CurrentSmoothedSocketOffset = SpringArmComponent->SocketOffset;
            SocketOffsetTransitionTargetValue = CurrentSmoothedSocketOffset;
        }
        float SocketOffsetDelta = FVector::Dist(TargetSocketOffset, SocketOffsetTransitionTargetValue);
        if (SocketOffsetDelta > 1.0f)
        {
            SocketOffsetTransitionStartValue = CurrentSmoothedSocketOffset;
            SocketOffsetTransitionTargetValue = TargetSocketOffset;
            SocketOffsetTransitionElapsedTime = 0.0f;
            bSocketOffsetTransitionActive = true;
        }
    }

    // SocketOffset 线性过渡执行
    if (bSocketOffsetTransitionActive)
    {
        float SocketOffsetDeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
        SocketOffsetTransitionElapsedTime += SocketOffsetDeltaTime;
        float Alpha = FMath::Clamp(SocketOffsetTransitionElapsedTime / SocketOffsetTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedSocketOffset = FMath::Lerp(SocketOffsetTransitionStartValue, SocketOffsetTransitionTargetValue, Alpha);
        SpringArmComponent->SocketOffset = CurrentSmoothedSocketOffset;
        
        if (Alpha >= 1.0f)
        {
            bSocketOffsetTransitionActive = false;
            CurrentSmoothedSocketOffset = SocketOffsetTransitionTargetValue;
            UE_LOG(LogTemp, Warning, TEXT("ApplyExitTransition: SocketOffset COMPLETE"));
        }
    }
    
    // 应用平滑后的 SocketOffset
    if (!CurrentSmoothedSocketOffset.IsZero() || bSocketOffsetTransitionActive)
    {
        CurrentOutput.SocketOffset = CurrentSmoothedSocketOffset;
    }

    SpringArmComponent->SocketOffset = CurrentOutput.SocketOffset;

    // TargetOffset 过渡检测
    {
        FVector TargetTargetOffset = CurrentOutput.TargetOffset;
        if (CurrentSmoothedTargetOffset.IsZero() && !TargetTargetOffset.IsZero())
        {
            CurrentSmoothedTargetOffset = SpringArmComponent->TargetOffset;
            TargetOffsetTransitionTargetValue = CurrentSmoothedTargetOffset;
        }
        float OffsetDelta = FVector::Dist(TargetTargetOffset, TargetOffsetTransitionTargetValue);
        if (OffsetDelta > 1.0f)
        {
            TargetOffsetTransitionStartValue = CurrentSmoothedTargetOffset;
            TargetOffsetTransitionTargetValue = TargetTargetOffset;
            TargetOffsetTransitionElapsedTime = 0.0f;
            bTargetOffsetTransitionActive = true;
        }
    }

    // TargetOffset 线性过渡执行
    if (bTargetOffsetTransitionActive)
    {
        float TargetOffsetDeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
        TargetOffsetTransitionElapsedTime += TargetOffsetDeltaTime;
        float Alpha = FMath::Clamp(TargetOffsetTransitionElapsedTime / TargetOffsetTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedTargetOffset = FMath::Lerp(TargetOffsetTransitionStartValue, TargetOffsetTransitionTargetValue, Alpha);
        SpringArmComponent->TargetOffset = CurrentSmoothedTargetOffset;
        
        if (Alpha >= 1.0f)
        {
            bTargetOffsetTransitionActive = false;
            CurrentSmoothedTargetOffset = TargetOffsetTransitionTargetValue;
        }
    }
    
    // 应用平滑后的 TargetOffset
    if (!CurrentSmoothedTargetOffset.IsZero() || bTargetOffsetTransitionActive)
    {
        CurrentOutput.TargetOffset = CurrentSmoothedTargetOffset;
    }

    SpringArmComponent->TargetOffset = CurrentOutput.TargetOffset;

    // Pitch 过渡检测
    {
        float TargetPitch = CurrentOutput.Rotation.Pitch;
        if (!bHasPitchInitialized)
        {
            // 从当前 SpringArm 读取实际 Pitch 值
            CurrentSmoothedPitch = SpringArmComponent->GetComponentRotation().Pitch;
            PitchTransitionTargetValue = CurrentSmoothedPitch;
            bHasPitchInitialized = true;
        }
        float PitchDelta = FMath::Abs(TargetPitch - PitchTransitionTargetValue);
        if (PitchDelta > 0.5f && bHasPitchInitialized)
        {
            PitchTransitionStartValue = CurrentSmoothedPitch;
            PitchTransitionTargetValue = TargetPitch;
            PitchTransitionElapsedTime = 0.0f;
            bPitchTransitionActive = true;
        }
    }

    // Pitch 线性过渡执行
    if (bPitchTransitionActive)
    {
        float PitchDeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
        PitchTransitionElapsedTime += PitchDeltaTime;
        float Alpha = FMath::Clamp(PitchTransitionElapsedTime / PitchTransitionDuration, 0.0f, 1.0f);
        CurrentSmoothedPitch = FMath::Lerp(PitchTransitionStartValue, PitchTransitionTargetValue, Alpha);
        
        if (Alpha >= 1.0f)
        {
            bPitchTransitionActive = false;
            CurrentSmoothedPitch = PitchTransitionTargetValue;
        }
    }
    
    // 应用平滑后的 Pitch
    if (bHasPitchInitialized)
    {
        CurrentOutput.Rotation.Pitch = CurrentSmoothedPitch;
    }

    // ========== 应用所有计算后的值到组件 ==========
    
    // 应用平滑后的 FOV
    if (CurrentSmoothedFOV > 0.0f)
    {
        CameraComponent->SetFieldOfView(CurrentSmoothedFOV);
    }
    
    // 应用平滑后的 Distance
    if (CurrentSmoothedDistance > 0.0f)
    {
        SpringArmComponent->TargetArmLength = CurrentSmoothedDistance;
    }
}


//========================================
// Pipeline & Stage Creation
//========================================

void USoulsCameraManager::CreateAndRegisterStages()
{
    if (!Pipeline)
    {
        UE_LOG(LogSoulsCamera, Error, TEXT("CreateAndRegisterStages: Pipeline is null"));
        return;
    }

    UE_LOG(LogSoulsCamera, Log, TEXT("CreateAndRegisterStages: Registering 8 stages..."));

    // Stage 1: Input Gather (INPUT LAYER)
    UCameraStage_InputGather* Stage1 = NewObject<UCameraStage_InputGather>(this);
    if (Stage1)
    {
        Pipeline->RegisterStage(1, TScriptInterface<ICameraStage>(Stage1));
    }

    // Stage 2: State Machine (DECISION LAYER)
    UCameraStage_StateMachine* Stage2 = NewObject<UCameraStage_StateMachine>(this);
    if (Stage2)
    {
        Stage2->SetStateMachine(StateMachine);
        Pipeline->RegisterStage(2, TScriptInterface<ICameraStage>(Stage2));
    }

    // Stage 3: Module Compute (COMPUTE LAYER)
    UCameraStage_ModuleCompute* Stage3 = NewObject<UCameraStage_ModuleCompute>(this);
    if (Stage3)
    {
        Stage3->SetModuleRegistry(ModuleRegistry);
        Pipeline->RegisterStage(3, TScriptInterface<ICameraStage>(Stage3));
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateAndRegisterStages: Stage 3 connected to ModuleRegistry"));
    }

    // Stage 4: Modifier Apply (COMPUTE LAYER - Skippable)
    Stage4_ModifierApply = NewObject<UCameraStage_ModifierApply>(this);
    if (Stage4_ModifierApply)
    {
        Stage4_ModifierApply->SetModifierManager(ModifierManager);
        Pipeline->RegisterStage(4, TScriptInterface<ICameraStage>(Stage4_ModifierApply));
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateAndRegisterStages: Stage 4 connected to ModifierManager (%d modifiers)"),
            ModifierManager ? ModifierManager->GetModifierCount() : 0);
    }

    // Stage 5: Blend & Solve (COMPUTE LAYER)
    UCameraStage_BlendSolve* Stage5 = NewObject<UCameraStage_BlendSolve>(this);
    if (Stage5)
    {
        Pipeline->RegisterStage(5, TScriptInterface<ICameraStage>(Stage5));
    }

    // Stage 6: Collision Resolve (SAFETY LAYER - Skippable)
    UCameraStage_CollisionResolve* Stage6 = NewObject<UCameraStage_CollisionResolve>(this);
    if (Stage6)
    {
        Stage6->SetCollisionResolver(CollisionResolver);
        Pipeline->RegisterStage(6, TScriptInterface<ICameraStage>(Stage6));
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateAndRegisterStages: Stage 6 connected to CollisionResolver (%d strategies)"),
            CollisionResolver ? CollisionResolver->GetStrategyCount() : 0);
    }

    // Stage 7: Post Process (OUTPUT LAYER - Skippable)
    UCameraStage_PostProcess* Stage7 = NewObject<UCameraStage_PostProcess>(this);
    if (Stage7)
    {
        Pipeline->RegisterStage(7, TScriptInterface<ICameraStage>(Stage7));
    }

    // Stage 8: Render Apply (OUTPUT LAYER)
    UCameraStage_RenderApply* Stage8 = NewObject<UCameraStage_RenderApply>(this);
    if (Stage8)
    {
        Pipeline->RegisterStage(8, TScriptInterface<ICameraStage>(Stage8));
    }

    // Verify all required stages are registered
    if (Pipeline->AreAllStagesRegistered())
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("CreateAndRegisterStages: All 8 stages registered successfully"));
}
    else
    {
        UE_LOG(LogSoulsCamera, Warning, TEXT("CreateAndRegisterStages: Not all required stages registered"));
    }
}

//========================================
// ConfigService Architecture Methods
//========================================

bool USoulsCameraManager::CreateConfigService()
{
    // Create ConfigService instance
    ConfigService = NewObject<UCameraConfigService>(this, UCameraConfigService::StaticClass());
    
    if (!ConfigService)
    {
        UE_LOG(LogSoulsCamera, Error, TEXT("USoulsCameraManager::CreateConfigService - Failed to create ConfigService"));
        return false;
    }
    
    // Initialize with DataTables
    bool bSuccess = ConfigService->Initialize(StatesDataTable, OverridesDataTable);
    
    if (bSuccess)
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("USoulsCameraManager::CreateConfigService - ConfigService created and initialized successfully"));
    }
    else
    {
        UE_LOG(LogSoulsCamera, Error, TEXT("USoulsCameraManager::CreateConfigService - ConfigService initialization failed"));
    }
    
    return bSuccess;
}

bool USoulsCameraManager::InitializeWithConfigService()
{
    // Validate DataTable references
    if (!StatesDataTable)
    {
        UE_LOG(LogSoulsCamera, Error, TEXT("USoulsCameraManager::InitializeWithConfigService - StatesDataTable is null"));
        return false;
    }
    
    // OverridesDataTable is optional, so we don't fail if it's null
    if (!OverridesDataTable)
    {
        UE_LOG(LogSoulsCamera, Warning, TEXT("USoulsCameraManager::InitializeWithConfigService - OverridesDataTable is null, using defaults only"));
    }
    
    // Create ConfigService
    if (!CreateConfigService())
    {
        return false;
}

    // Set architecture flag
    bUseNewConfigArchitecture = true;
    
    // Initialize StateMachine with ConfigService
    if (StateMachine)
    {
        StateMachine->InitializeWithConfigService(ConfigService);
        UE_LOG(LogSoulsCamera, Log, TEXT("USoulsCameraManager::InitializeWithConfigService - StateMachine initialized with ConfigService"));
    }
    else
    {
        UE_LOG(LogSoulsCamera, Warning, TEXT("USoulsCameraManager::InitializeWithConfigService - StateMachine is null"));
        }
    
    UE_LOG(LogSoulsCamera, Log, TEXT("USoulsCameraManager::InitializeWithConfigService - Initialization complete"));
    return true;
}

bool USoulsCameraManager::GetCameraStateConfig(FName StateName, FCameraStateConfig& OutConfig) const
{
    // New architecture: use ConfigService
    if (bUseNewConfigArchitecture && ConfigService)
    {
        return ConfigService->GetStateConfig(StateName, OutConfig);
    }
    
    // Legacy architecture: use StateMachine's method or direct DataTable lookup
    if (StateMachine)
    {
        // Try to get from StateMachine if it has the method
        FCameraStateConfig TempConfig;
        if (StateMachine->GetStateConfig(StateName, TempConfig))
        {
            OutConfig = TempConfig;
            return true;
        }
    }
    
    // Fallback: direct DataTable lookup (legacy)
    if (StatesDataTable)
    {
        const FCameraStateConfig* FoundConfig = StatesDataTable->FindRow<FCameraStateConfig>(StateName, TEXT(""));
        if (FoundConfig)
        {
            OutConfig = *FoundConfig;
            return true;
        }
    }
    
    return false;
}

bool USoulsCameraManager::DoesCameraStateExist(FName StateName) const
{
    // New architecture: use ConfigService
    if (bUseNewConfigArchitecture && ConfigService)
    {
        return ConfigService->DoesStateExist(StateName);
    }
    
    // Legacy architecture: check DataTable directly
    if (StatesDataTable)
    {
        return StatesDataTable->FindRow<FCameraStateConfig>(StateName, TEXT("")) != nullptr;
    }
    
    return false;
}

TArray<FName> USoulsCameraManager::GetAllCameraStateNames() const
{
    TArray<FName> StateNames;
    
    // New architecture: use ConfigService
    if (bUseNewConfigArchitecture && ConfigService)
    {
        return ConfigService->GetAllStateNames();
    }
    
    // Legacy architecture: get from DataTable
    if (StatesDataTable)
    {
        StateNames = StatesDataTable->GetRowNames();
    }
    
    return StateNames;
}

void USoulsCameraManager::LogConfigStatistics() const
{
    UE_LOG(LogSoulsCamera, Log, TEXT("========================================"));
    UE_LOG(LogSoulsCamera, Log, TEXT("Camera Manager Configuration Statistics"));
    UE_LOG(LogSoulsCamera, Log, TEXT("========================================"));
    
    UE_LOG(LogSoulsCamera, Log, TEXT("Architecture: %s"), 
        bUseNewConfigArchitecture ? TEXT("New (ConfigService)") : TEXT("Legacy (DataTable)"));
    
    if (bUseNewConfigArchitecture && ConfigService)
    {
        int32 StateCount = 0;
        int32 OverrideCount = 0;
        int32 CachedCount = 0;
        
        ConfigService->GetStatistics(StateCount, OverrideCount, CachedCount);
        
        UE_LOG(LogSoulsCamera, Log, TEXT("States Loaded: %d"), StateCount);
        UE_LOG(LogSoulsCamera, Log, TEXT("Overrides Loaded: %d"), OverrideCount);
        UE_LOG(LogSoulsCamera, Log, TEXT("Cached Configs: %d"), CachedCount);
    }
    else
    {
        if (StatesDataTable)
        {
            UE_LOG(LogSoulsCamera, Log, TEXT("StatesDataTable: %s (%d rows)"), 
                *StatesDataTable->GetName(), 
                StatesDataTable->GetRowNames().Num());
        }
        else
        {
            UE_LOG(LogSoulsCamera, Log, TEXT("StatesDataTable: None"));
        }
    }
    
    if (OverridesDataTable)
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("OverridesDataTable: %s (%d rows)"), 
            *OverridesDataTable->GetName(), 
            OverridesDataTable->GetRowNames().Num());
    }
    else
    {
        UE_LOG(LogSoulsCamera, Log, TEXT("OverridesDataTable: None"));
}

    UE_LOG(LogSoulsCamera, Log, TEXT("StateMachine: %s"), StateMachine ? TEXT("Valid") : TEXT("Null"));
    UE_LOG(LogSoulsCamera, Log, TEXT("ConfigService: %s"), ConfigService ? TEXT("Valid") : TEXT("Null"));
    
    UE_LOG(LogSoulsCamera, Log, TEXT("========================================"));
}

void USoulsCameraManager::ReloadConfigData()
{
    UE_LOG(LogSoulsCamera, Log, TEXT("USoulsCameraManager::ReloadConfigData - Reloading configuration data..."));
    
    if (bUseNewConfigArchitecture && ConfigService)
    {
        // Reload ConfigService data
        ConfigService->ReloadData();
        
        // Re-initialize StateMachine if needed
        if (StateMachine)
        {
            StateMachine->InitializeWithConfigService(ConfigService);
        }
        
        UE_LOG(LogSoulsCamera, Log, TEXT("USoulsCameraManager::ReloadConfigData - ConfigService data reloaded"));
    }
    else
    {
        // Legacy: just log a message (DataTable reloading would require re-importing)
        UE_LOG(LogSoulsCamera, Warning, TEXT("USoulsCameraManager::ReloadConfigData - Legacy mode, manual DataTable reimport required"));
    }
    
    // Log statistics after reload
    LogConfigStatistics();
}

//========================================
// Debug Accessors Implementation
//========================================

bool USoulsCameraManager::IsCollisionDebugEnabled() const
{
#if WITH_EDITOR
    return bDebugAll || bDebugCollision;
#else
    return false;
#endif
}

bool USoulsCameraManager::IsStateMachineDebugEnabled() const
{
#if WITH_EDITOR
    return bDebugAll || bDebugStateMachine;
#else
    return false;
#endif
}

bool USoulsCameraManager::IsPipelineDebugEnabled() const
{
#if WITH_EDITOR
    return bDebugAll || bDebugPipeline;
#else
    return false;
#endif
}

bool USoulsCameraManager::IsModuleDebugEnabled() const
{
#if WITH_EDITOR
    return bDebugAll || bDebugModules;
#else
    return false;
#endif
}

void USoulsCameraManager::SetAllDebugEnabled(bool bEnabled)
{
#if WITH_EDITOR
    bDebugAll = bEnabled;
    bDebugCollision = bEnabled;
    bDebugStateMachine = bEnabled;
    bDebugPipeline = bEnabled;
    bDebugModules = bEnabled;
    bShowDebugInfo = bEnabled;
#endif
}

//========================================
// Debug Methods
//========================================

#if WITH_EDITOR
void USoulsCameraManager::DrawDebugInfo()
{
    if (!GetWorld() || !GEngine)
    {
        return;
    }

    // Build debug text with comprehensive camera info
    FString DebugText = FString::Printf(
        TEXT("=== Souls Camera ===\n")
        TEXT("State: %s\n")
        TEXT("Distance: %.1f\n")
        TEXT("FOV: %.1f\n")
        TEXT("Collision: %s\n")
        TEXT("Lock-On: %s\n")
        TEXT("Frame: %llu"),
        *GetCurrentStateName().ToString(),
        CurrentOutput.Distance,
        CurrentOutput.FOV,
        CurrentOutput.bCollisionAdjusted ? TEXT("Yes") : TEXT("No"),
        LockOnTarget.IsValid() ? TEXT("Active") : TEXT("None"),
        FrameCounter
    );

    // Display on screen
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugText);
}
#endif

