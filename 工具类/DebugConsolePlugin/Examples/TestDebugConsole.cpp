// TestDebugConsole.h
// 可选的测试文件 - 用于验证 DebugConsolePlugin 功能
// 将此文件放在游戏项目的 Source 目录中

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestDebugConsole.generated.h"

/**
 * 测试调试控制台插件的示例 Actor
 * 使用方法：
 * 1. 将此文件添加到项目中
 * 2. 在关卡中放置此 Actor
 * 3. 运行游戏，观察独立控制台窗口的输出
 */
UCLASS()
class ATestDebugConsole : public AActor
{
    GENERATED_BODY()
    
public:    
    ATestDebugConsole();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

private:
    /** 测试计数器 */
    int32 TickCounter = 0;
    
    /** 测试所有日志级别 */
    void TestAllLogLevels();
    
    /** 测试不同类别 */
    void TestDifferentCategories();
    
    /** 测试格式化输出 */
    void TestFormattedOutput();
};

// TestDebugConsole.cpp
#include "TestDebugConsole.h"

// 定义自定义日志类别
DEFINE_LOG_CATEGORY_STATIC(LogTestConsole, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogGameplay, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogAI, Log, All);

ATestDebugConsole::ATestDebugConsole()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATestDebugConsole::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Display, TEXT("═══════════════════════════════════════════"));
    UE_LOG(LogTemp, Display, TEXT("  Debug Console Plugin Test Started"));
    UE_LOG(LogTemp, Display, TEXT("═══════════════════════════════════════════"));
    
    // 测试所有日志级别
    TestAllLogLevels();
    
    // 测试不同类别
    TestDifferentCategories();
    
    // 测试格式化输出
    TestFormattedOutput();
}

void ATestDebugConsole::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TickCounter++;
    
    // 每 60 帧（约1秒）输出一次
    if (TickCounter % 60 == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Tick Counter: %d | Delta Time: %.4f"), TickCounter, DeltaTime);
    }
    
    // 每 300 帧（约5秒）输出一次警告
    if (TickCounter % 300 == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("This is a periodic warning (every 5 seconds)"));
    }
}

void ATestDebugConsole::TestAllLogLevels()
{
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("--- Testing All Log Levels ---"));
    
    UE_LOG(LogTemp, Log, TEXT("This is a LOG message"));
    UE_LOG(LogTemp, Display, TEXT("This is a DISPLAY message"));
    UE_LOG(LogTemp, Warning, TEXT("This is a WARNING message"));
    UE_LOG(LogTemp, Error, TEXT("This is an ERROR message"));
    UE_LOG(LogTemp, Verbose, TEXT("This is a VERBOSE message"));
    UE_LOG(LogTemp, VeryVerbose, TEXT("This is a VERY VERBOSE message"));
    
    // 注意：Fatal 会导致程序崩溃，所以不在这里测试
    // UE_LOG(LogTemp, Fatal, TEXT("This is a FATAL message"));
}

void ATestDebugConsole::TestDifferentCategories()
{
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("--- Testing Different Categories ---"));
    
    UE_LOG(LogTemp, Log, TEXT("Message from LogTemp category"));
    UE_LOG(LogTestConsole, Log, TEXT("Message from LogTestConsole category"));
    UE_LOG(LogGameplay, Log, TEXT("Message from LogGameplay category"));
    UE_LOG(LogAI, Warning, TEXT("AI Warning: Path not found"));
    UE_LOG(LogTemp, Error, TEXT("Simulated error in LogTemp"));
}

void ATestDebugConsole::TestFormattedOutput()
{
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("--- Testing Formatted Output ---"));
    
    // 数值格式化
    int32 PlayerScore = 12345;
    float PlayerHealth = 87.5f;
    UE_LOG(LogTemp, Log, TEXT("Player Score: %d, Health: %.1f%%"), PlayerScore, PlayerHealth);
    
    // 字符串格式化
    FString PlayerName = TEXT("TestPlayer");
    UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *PlayerName);
    
    // Vector 格式化
    FVector PlayerLocation = FVector(100.0f, 200.0f, 50.0f);
    UE_LOG(LogTemp, Log, TEXT("Player Location: %s"), *PlayerLocation.ToString());
    
    // 多种类型混合
    UE_LOG(LogTemp, Log, TEXT("Complex: Name=%s, Pos=%s, HP=%.1f, Score=%d"), 
        *PlayerName, 
        *PlayerLocation.ToString(), 
        PlayerHealth, 
        PlayerScore);
    
    // Unicode 字符测试
    UE_LOG(LogTemp, Display, TEXT("Unicode Test: 你好世界 🎮 Testing 测试"));
    
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("═══════════════════════════════════════════"));
    UE_LOG(LogTemp, Display, TEXT("  All Tests Completed!"));
    UE_LOG(LogTemp, Display, TEXT("  Check the Debug Console window for output"));
    UE_LOG(LogTemp, Display, TEXT("═══════════════════════════════════════════"));
}
