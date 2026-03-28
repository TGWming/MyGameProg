// DebugConsoleManager.cpp
#include "DebugConsoleManager.h"
#include "DebugConsoleSettings.h"
#include "DebugConsoleOutputDevice.h"
#include "GameEventMonitor.h"

#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

FDebugConsoleManager& FDebugConsoleManager::Get()
{
    static FDebugConsoleManager Instance;
    return Instance;
}

void FDebugConsoleManager::Initialize()
{
#if WITH_DEBUG_CONSOLE && !UE_BUILD_SHIPPING
    if (bIsInitialized)
    {
        return;
    }
    
    // 安全检查：确保 UObject 系统已初始化
    if (!UObjectInitialized())
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugConsolePlugin: UObject system not initialized yet, skipping initialization"));
        return;
    }
    
    const UDebugConsoleSettings* Settings = UDebugConsoleSettings::Get();
    if (!Settings || !Settings->bEnableDebugConsole)
    {
        return;
    }
    
    AllocateConsole();
    
    if (bConsoleAllocated)
    {
        // 创建并注册输出设备
        OutputDevice = MakeShared<FDebugConsoleOutputDevice>();
        GLog->AddOutputDevice(OutputDevice.Get());
        
        // 注册引擎退出回调
        ExitDelegateHandle = FCoreDelegates::OnExit.AddRaw(this, &FDebugConsoleManager::OnEngineExit);
        
        bIsInitialized = true;
        
        // 输出初始化信息
        PrintToConsole(TEXT(""));
        PrintToConsole(TEXT("========================================================"));
        PrintToConsole(TEXT("           DEBUG CONSOLE INITIALIZED                    "));
        PrintToConsole(TEXT("   This window will remain open after game exits        "));
        PrintToConsole(TEXT("========================================================"));
        PrintToConsole(TEXT(""));
        
        // 初始化游戏事件监控器
        FGameEventMonitor::Get().Initialize();
    }
#endif
}

void FDebugConsoleManager::Shutdown()
{
#if WITH_DEBUG_CONSOLE && !UE_BUILD_SHIPPING
    if (!bIsInitialized)
    {
        return;
    }
    
    // 关闭游戏事件监控器
    FGameEventMonitor::Get().Shutdown();
    
    // 移除输出设备
    if (OutputDevice.IsValid())
    {
        GLog->RemoveOutputDevice(OutputDevice.Get());
        OutputDevice.Reset();
    }
    
    // 移除退出回调
    if (ExitDelegateHandle.IsValid())
    {
        FCoreDelegates::OnExit.Remove(ExitDelegateHandle);
    }
    
    bIsInitialized = false;
#endif
}

void FDebugConsoleManager::AllocateConsole()
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    if (bConsoleAllocated)
    {
        return;
    }
    
    if (::AllocConsole())
    {
        const UDebugConsoleSettings* Settings = UDebugConsoleSettings::Get();
        
        // 设置控制台标题
        ::SetConsoleTitle(*Settings->ConsoleTitle);
        
        // 重定向标准输出
        FILE* fp = nullptr;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
        
        // 设置控制台代码页为UTF-8
        ::SetConsoleOutputCP(CP_UTF8);
        ::SetConsoleCP(CP_UTF8);
        
        // 可选：调整控制台窗口大小
        HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
        SMALL_RECT windowSize = {0, 0, 119, 30};
        ::SetConsoleWindowInfo(hConsole, 1, &windowSize);
        
        // 设置控制台颜色（可选）
        ::SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        
        bConsoleAllocated = true;
    }
#endif
}

void FDebugConsoleManager::OnEngineExit()
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    const UDebugConsoleSettings* Settings = UDebugConsoleSettings::Get();
    
    if (bConsoleAllocated && Settings && Settings->bWaitForKeyOnExit)
    {
        WaitForKeyPress();
    }
    
    FreeConsoleWindow();
#endif
}

void FDebugConsoleManager::WaitForKeyPress()
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    
    // 恢复默认颜色
    ::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    printf("\n");
    printf("================================================================\n");
    
    // 设置高亮颜色
    ::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("  GAME HAS EXITED\n");
    
    ::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    printf("================================================================\n");
    printf("\n");
    printf("  Press any key to close this window...\n");
    printf("\n");
    
    // 等待按键
    _getch();
#endif
}

void FDebugConsoleManager::FreeConsoleWindow()
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    if (bConsoleAllocated)
    {
        ::FreeConsole();
        bConsoleAllocated = false;
    }
#endif
}

void FDebugConsoleManager::PrintToConsole(const FString& Message)
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
    if (bConsoleAllocated)
    {
        printf("%s\n", TCHAR_TO_UTF8(*Message));
    }
#endif
}
