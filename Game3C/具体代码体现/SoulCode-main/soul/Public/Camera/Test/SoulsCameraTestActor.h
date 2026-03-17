// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/Data/CameraStateEnums.h"
#include "SoulsCameraTestActor.generated.h"

// Forward declarations
class USoulsCameraManager;
class UCameraModifierManager;
class UCameraModuleRegistry;
class UCameraCollisionResolver;

/**
 * ASoulsCameraTestActor
 * 
 * Actor for testing camera system functionality.
 * Place in level and use to trigger various camera tests.
 * 
 * Features:
 * - State transition testing
 * - Modifier effect testing
 * - Collision system testing
 * - Lock-on target testing
 * - System validation
 */
UCLASS(BlueprintType, Blueprintable)
class SOUL_API ASoulsCameraTestActor : public AActor
{
    GENERATED_BODY()

public:
    ASoulsCameraTestActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    //========================================
    // Test Controls - 测试控制
    //========================================

    /** Run all tests - 运行所有测试 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test")
    void RunAllTests();

    /** Test state transitions - 测试状态转换 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test")
    void TestStateTransitions();

    /** Test modifiers - 测试修改器 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test")
    void TestModifiers();

    /** Test collision - 测试碰撞 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test")
    void TestCollision();

    /** Test target lock - 测试目标锁定 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test")
    void TestTargetLock();

    //========================================
    // Effect Triggers - 效果触发（手动测试用）
    //========================================

    /** Trigger shake test - 触发震动测试 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Effects")
    void TriggerShakeTest(bool bHeavy = false);

    /** Trigger slow motion test - 触发慢动作测试 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Effects")
    void TriggerSlowMotionTest(float Duration = 1.0f);

    /** Trigger hit stop test - 触发顿帧测试 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Effects")
    void TriggerHitStopTest();

    /** Trigger hit reaction test - 触发受击反应测试 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Effects")
    void TriggerHitReactionTest(bool bHeavyHit = false);

    /** Trigger low health effect test - 触发低血量效果测试 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Effects")
    void TriggerLowHealthTest(float HealthPercent = 0.2f);

    //========================================
    // State Triggers - 状态触发
    //========================================

    /** Force state change by name - 按名称强制切换状态 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|State")
    void ForceStateChange(FName NewStateName);

    /** Request state change to Exploration - 切换到探索状态 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|State")
    void SetExplorationState();

    /** Request state change to Combat - 切换到战斗状态 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|State")
    void SetCombatState();

    /** Toggle combat mode - 切换战斗模式 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|State")
    void ToggleCombatMode();

    /** Set this actor as lock-on target - 将此Actor设为锁定目标 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|State")
    void SetAsLockOnTarget();

    /** Clear lock-on - 清除锁定 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|State")
    void ClearLockOn();

    //========================================
    // Validation - 验证
    //========================================

    /** Validate system initialization - 验证系统初始化 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Validate")
    bool ValidateSystemInitialization();

    /** Validate module count (expected: 39) - 验证模块数量 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Validate")
    bool ValidateModuleCount();

    /** Validate modifier count (expected: 26) - 验证修改器数量 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Validate")
    bool ValidateModifierCount();

    /** Validate collision strategy count (expected: 20) - 验证碰撞策略数量 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Validate")
    bool ValidateCollisionCount();

    /** Get validation report - 获取验证报告 */
    UFUNCTION(BlueprintPure, Category = "Camera Test|Validate")
    FString GetValidationReport() const;

    /** Print validation report to log - 打印验证报告到日志 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Validate")
    void PrintValidationReport();

    //========================================
    // Debug Display - 调试显示
    //========================================

    /** Enable/disable on-screen debug display - 启用/禁用屏幕调试显示 */
    UFUNCTION(BlueprintCallable, Category = "Camera Test|Debug")
    void SetDebugDisplayEnabled(bool bEnabled);

    /** Get current camera debug string - 获取当前相机调试信息 */
    UFUNCTION(BlueprintPure, Category = "Camera Test|Debug")
    FString GetCameraDebugString() const;

protected:
    //========================================
    // Internal Methods - 内部方法
    //========================================

    /** Find camera manager in world - 在世界中查找相机管理器 */
    USoulsCameraManager* FindCameraManager();

    /** Log test result - 记录测试结果 */
    void LogTestResult(const FString& TestName, bool bSuccess, const FString& Details = TEXT(""));

    /** Build validation report - 构建验证报告 */
    void BuildValidationReport();

    //========================================
    // Configuration - 配置
    //========================================

    /** Whether to auto-find camera manager on begin play */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Test|Config")
    bool bAutoFindCameraManager;

    /** Whether to run validation on begin play */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Test|Config")
    bool bValidateOnBeginPlay;

    /** Whether to show debug display */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Test|Config")
    bool bShowDebugDisplay;

    //========================================
    // References - 引用
    //========================================

    /** Reference to found camera manager */
    UPROPERTY(BlueprintReadOnly, Category = "Camera Test|Reference")
    USoulsCameraManager* CameraManager;

    //========================================
    // State - 状态
    //========================================

    /** Test results - 测试结果 */
    TArray<FString> TestResults;

    /** Validation report cache - 验证报告缓存 */
    FString CachedValidationReport;

    /** Is currently in combat test mode */
    bool bTestCombatMode;

    /** Is validation report dirty */
    bool bValidationReportDirty;
};
