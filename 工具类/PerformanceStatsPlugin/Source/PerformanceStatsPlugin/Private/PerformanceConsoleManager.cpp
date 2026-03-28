#include "PerformanceConsoleManager.h"
#include "PerformanceStatsSettings.h"
#include "CoreGlobals.h"
#include "Misc/CoreDelegates.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <conio.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

FPerformanceConsoleManager& FPerformanceConsoleManager::Get()
{
    static FPerformanceConsoleManager Instance;
    return Instance;
}

FPerformanceConsoleManager::FPerformanceConsoleManager()
    : UpdateInterval(1.0f)    // 每秒刷新一次
    , TimeSinceLastUpdate(0.0f)
    , bEnabled(true)
    , bPaused(false)
    , bConsoleAllocated(false)
#if PLATFORM_WINDOWS
    , ConsoleHandle(nullptr)
    , DefaultColor(0)
#endif
{
}

FPerformanceConsoleManager::~FPerformanceConsoleManager()
{
}

void FPerformanceConsoleManager::Initialize()
{
#if WITH_PERFORMANCE_STATS && PLATFORM_WINDOWS
    UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Initialize() called"));
    UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: WITH_PERFORMANCE_STATS is enabled"));
    
    const UPerformanceStatsSettings* Settings = UPerformanceStatsSettings::Get();
    if (!Settings || !Settings->bEnablePerformanceStats)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Performance stats disabled in settings"));
        return;
    }
    
    // 分配控制台窗口
    AllocateConsole();
    
    if (!bConsoleAllocated)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Console allocation failed, monitoring disabled"));
        return;
    }
    
    // 注册Tick回调 - 使用Lambda包装，因为OnEndFrame的签名是void()
    TickDelegateHandle = FCoreDelegates::OnEndFrame.AddLambda([this]()
    {
        this->Tick(FApp::GetDeltaTime());
    });
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Initialized successfully"));
#endif
}

void FPerformanceConsoleManager::Shutdown()
{
    // 注销Tick回调
    if (TickDelegateHandle.IsValid())
    {
        FCoreDelegates::OnEndFrame.Remove(TickDelegateHandle);
        TickDelegateHandle.Reset();
    }

#if PLATFORM_WINDOWS
    // 恢复默认颜色
    ResetConsoleColor();
    // 释放控制台窗口
    FreeConsoleWindow();
#endif
}

void FPerformanceConsoleManager::Tick(float DeltaTime)
{
#if WITH_PERFORMANCE_STATS && PLATFORM_WINDOWS
    if (!bEnabled || !bConsoleAllocated)
    {
        return;
    }
    
    if (!UObjectInitialized())
    {
        return;
    }
    
    const UPerformanceStatsSettings* Settings = UPerformanceStatsSettings::Get();
    if (!Settings)
    {
        return;
    }
    
    // 处理键盘输入
    ProcessKeyboardInput();
    
    TimeSinceLastUpdate += FApp::GetDeltaTime();
    
    if (TimeSinceLastUpdate >= Settings->RefreshInterval)
    {
        TimeSinceLastUpdate = 0.0f;
        
        if (!bPaused)
        {
            // 获取当前 World
            UWorld* World = nullptr;
            if (GEngine)
            {
                for (const FWorldContext& Context : GEngine->GetWorldContexts())
                {
                    if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
                    {
                        World = Context.World();
                        break;
                    }
                }
            }
            
            if (World)
            {
                FPerformanceDataCollector::Get().CollectAllData(World);
            }
            
            // 只有未暂停时才刷新显示，避免暂停时窗口闪烁
            RefreshDisplay();
        }
    }
#endif
}

void FPerformanceConsoleManager::RefreshDisplay()
{
#if WITH_PERFORMANCE_STATS && PLATFORM_WINDOWS
    if (ConsoleHandle == nullptr || ConsoleHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }
    
    // 移动光标到顶部
    COORD coord = {0, 0};
    SetConsoleCursorPosition(ConsoleHandle, coord);
    
    const FPerformanceDataCollector& Collector = FPerformanceDataCollector::Get();
    
    // 打印标题
    ResetConsoleColor();
    printf("================================================================\n");
    printf("  PERFORMANCE STATS MONITOR              %s\n", 
        bPaused ? "[PAUSED]" : "        ");
    printf("================================================================\n");
    printf("  Time: %s      [P] Pause/Resume\n", 
        TCHAR_TO_UTF8(*FDateTime::Now().ToString(TEXT("%H:%M:%S"))));
    printf("================================================================\n\n");
    
    // 打印帧数据
    PrintFrameData(Collector.GetFrameData());
    
    // 打印 Top N
    printf("  ============ TOP EXPENSIVE ITEMS ============\n\n");
    PrintTopExpensiveAssets(Collector.GetTopExpensiveAssets());
    
    // 打印类别数据
    printf("  ============ BY CATEGORY ============\n\n");
    PrintCategoryData(Collector.GetCategoryData());
    
    // 清除多余的行
    for (int32 i = 0; i < 5; ++i)
    {
        printf("                                                                \n");
    }
#endif
}

void FPerformanceConsoleManager::PrintFrameData(const FFramePerformanceData& Data)
{
#if WITH_PERFORMANCE_STATS && PLATFORM_WINDOWS
    printf("  FRAME STATS\n");
    printf("  ----------------------------------------------------------------\n");
    
    SetConsoleColor(Data.FrameTimeCostLevel);
    printf("  Frame Time:   %7.2f ms  %s    FPS: %.1f\n", 
        Data.FrameTimeMs, GetCostIndicator(Data.FrameTimeCostLevel), Data.FPS);
    ResetConsoleColor();
    
    SetConsoleColor(Data.GameThreadCostLevel);
    printf("  Game Thread:  %7.2f ms  %s\n", 
        Data.GameThreadTimeMs, GetCostIndicator(Data.GameThreadCostLevel));
    ResetConsoleColor();
    
    SetConsoleColor(Data.RenderThreadCostLevel);
    printf("  Render Thread:%7.2f ms  %s\n", 
        Data.RenderThreadTimeMs, GetCostIndicator(Data.RenderThreadCostLevel));
    ResetConsoleColor();
    
    SetConsoleColor(Data.GPUCostLevel);
    printf("  GPU Time:     %7.2f ms  %s\n", 
        Data.GPUTimeMs, GetCostIndicator(Data.GPUCostLevel));
    ResetConsoleColor();
    
    printf("\n");
#endif
}

const char* FPerformanceConsoleManager::GetCostIndicator(ECostLevel Level)
{
    switch (Level)
    {
        case ECostLevel::Green:  return "[OK]";
        case ECostLevel::Yellow: return "[! ]";
        case ECostLevel::Red:    return "[!!]";
        default:                 return "[??]";
    }
}

void FPerformanceConsoleManager::SetConsoleColor(ECostLevel Level)
{
#if PLATFORM_WINDOWS
    if (ConsoleHandle == nullptr || ConsoleHandle == INVALID_HANDLE_VALUE)
        return;

    WORD Color = DefaultColor;
    switch (Level)
    {
    case ECostLevel::Green:
        Color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
    case ECostLevel::Yellow:
        Color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
    case ECostLevel::Red:
        Color = FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
    }
    SetConsoleTextAttribute(ConsoleHandle, Color);
#endif
}

void FPerformanceConsoleManager::ResetConsoleColor()
{
#if PLATFORM_WINDOWS
    if (ConsoleHandle != nullptr && ConsoleHandle != INVALID_HANDLE_VALUE)
    {
        SetConsoleTextAttribute(ConsoleHandle, DefaultColor);
    }
#endif
}

void FPerformanceConsoleManager::SetEnabled(bool bInEnabled)
{
    bEnabled = bInEnabled;
}

bool FPerformanceConsoleManager::IsEnabled() const
{
    return bEnabled;
}

void FPerformanceConsoleManager::SetPaused(bool bInPaused)
{
    bPaused = bInPaused;
}

bool FPerformanceConsoleManager::IsPaused() const
{
    return bPaused;
}

void FPerformanceConsoleManager::PrintTopExpensiveAssets(const TArray<FAssetPerformanceData>& Assets)
{
#if WITH_PERFORMANCE_STATS && PLATFORM_WINDOWS
    printf("  # | Type       | Asset Name                    | Cost    | Cnt\n");
    printf("  --+------------+-------------------------------+---------+-----\n");
    
    int32 Index = 1;
    for (const FAssetPerformanceData& Asset : Assets)
    {
        SetConsoleColor(Asset.CostLevel);
        
        FString ShortName = Asset.AssetName.Left(28);
        
        printf("  %2d| %-10s | %-29s | %6.2fms| %3d\n",
            Index++,
            TCHAR_TO_UTF8(FPerformanceDataCollector::GetCategoryName(Asset.Category)),
            TCHAR_TO_UTF8(*ShortName),
            Asset.CostMs,
            Asset.InstanceCount);
        
        ResetConsoleColor();
    }
    
    // 显示第一个资产的完整路径
    if (Assets.Num() > 0)
    {
        printf("\n  Top Asset Path:\n");
        printf("  %s\n", TCHAR_TO_UTF8(*Assets[0].AssetPath.Left(60)));
    }
    
    printf("\n");
#endif
}

void FPerformanceConsoleManager::PrintCategoryData(const TArray<FCategoryPerformanceData>& Categories)
{
#if WITH_PERFORMANCE_STATS && PLATFORM_WINDOWS
    for (const FCategoryPerformanceData& Cat : Categories)
    {
        SetConsoleColor(Cat.CostLevel);
        printf("  [%s] Total: %.2fms  %s\n", 
            TCHAR_TO_UTF8(FPerformanceDataCollector::GetCategoryName(Cat.Category)),
            Cat.TotalCostMs,
            GetCostIndicator(Cat.CostLevel));
        ResetConsoleColor();
        
        // 显示该类别下前 3 个资产
        int32 Count = FMath::Min(3, Cat.Assets.Num());
        for (int32 i = 0; i < Count; ++i)
        {
            const FAssetPerformanceData& Asset = Cat.Assets[i];
            SetConsoleColor(Asset.CostLevel);
            printf("    |- %-30s  %6.2fms  %s\n",
                TCHAR_TO_UTF8(*Asset.AssetName.Left(30)),
                Asset.CostMs,
                GetCostIndicator(Asset.CostLevel));
            ResetConsoleColor();
        }
        
        // 显示路径
        if (Cat.Assets.Num() > 0)
        {
            printf("       Path: %s\n", TCHAR_TO_UTF8(*Cat.Assets[0].AssetPath.Left(50)));
        }
        printf("\n");
    }
#endif
}

void FPerformanceConsoleManager::AllocateConsole()
{
#if PLATFORM_WINDOWS
    if (bConsoleAllocated)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Console already allocated, reusing existing"));
        return;
    }
    
    // 检查是否已经有其他插件创建了控制台（如 DebugConsolePlugin）
    ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    bool bConsoleCreated = false;
    bool bConsoleExistedBefore = false;
    
    // 如果已经有控制台句柄，检查标题来判断是否为 DebugConsole
    if (ConsoleHandle != INVALID_HANDLE_VALUE && ConsoleHandle != nullptr)
    {
        TCHAR ExistingTitle[256];
        if (::GetConsoleTitleW(ExistingTitle, 256) > 0)
        {
            FString TitleStr(ExistingTitle);
            if (TitleStr.Contains(TEXT("Debug Console")))
            {
                UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Detected DebugConsolePlugin window!"));
                UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Cannot share console with DebugConsole - outputs will conflict!"));
                UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Please disable one of the console plugins."));
                UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Initialization aborted to avoid conflicts."));
                return;
            }
            
            // 如果是其他控制台，警告但允许使用
            if (TitleStr != TEXT("Performance Stats Monitor"))
            {
                bConsoleExistedBefore = true;
                UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Existing console detected: %s"), *TitleStr);
                UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Will attempt to take over, but conflicts may occur!"));
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Reusing existing console handle"));
        bConsoleCreated = true;
    }
    else
    {
        // 在编辑器中先尝试附加到父进程控制台，失败后再分配新控制台
        
        // 尝试附加到父进程控制台（PIE模式下）
        if (::AttachConsole(ATTACH_PARENT_PROCESS))
        {
            UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Attached to parent console"));
            bConsoleCreated = true;
        }
        // 如果附加失败，尝试分配新的控制台窗口
        else if (::AllocConsole())
        {
            UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Allocated new console window"));
            bConsoleCreated = true;
        }
        else
        {
            DWORD ErrorCode = GetLastError();
            // 错误代码 5 表示访问被拒绝，通常是因为控制台已经存在
            if (ErrorCode == ERROR_ACCESS_DENIED)
            {
                UE_LOG(LogTemp, Warning, TEXT("PerformanceConsoleManager: Console already exists (Error 5), attempting to get existing handle"));
                // 尝试重新获取句柄
                ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
                if (ConsoleHandle != INVALID_HANDLE_VALUE && ConsoleHandle != nullptr)
                {
                    bConsoleCreated = true;
                    bConsoleExistedBefore = true;
                }
            }
            
            if (!bConsoleCreated)
            {
                UE_LOG(LogTemp, Error, TEXT("PerformanceConsoleManager: Failed to allocate or attach console. Error code: %d"), ErrorCode);
                return;
            }
        }
    }
    
    if (bConsoleCreated)
    {
        const UPerformanceStatsSettings* Settings = UPerformanceStatsSettings::Get();
        
        // 设置控制台标题（使用Unicode版本）
        ::SetConsoleTitleW(Settings ? *Settings->ConsoleTitle : TEXT("Performance Stats Monitor"));
        
        // 重定向标准输出
        FILE* fp = nullptr;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        
        // 获取控制台句柄（如果之前没有获取）
        if (ConsoleHandle == nullptr || ConsoleHandle == INVALID_HANDLE_VALUE)
        {
            ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        }
        
        if (ConsoleHandle != INVALID_HANDLE_VALUE && ConsoleHandle != nullptr)
        {
            // 获取默认颜色
            CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
            if (GetConsoleScreenBufferInfo(ConsoleHandle, &ConsoleInfo))
            {
                DefaultColor = ConsoleInfo.wAttributes;
            }
            
            // 设置控制台缓冲区大小，防止滚动
            COORD bufferSize;
            bufferSize.X = 80;
            bufferSize.Y = 50;
            SetConsoleScreenBufferSize(ConsoleHandle, bufferSize);
            
            // 隐藏光标
            CONSOLE_CURSOR_INFO cursorInfo;
            cursorInfo.dwSize = 1;
            cursorInfo.bVisible = 0;
            SetConsoleCursorInfo(ConsoleHandle, &cursorInfo);
            
            bConsoleAllocated = true;
            UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Console setup completed successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PerformanceConsoleManager: Failed to get console handle"));
        }
    }
#endif
}

void FPerformanceConsoleManager::FreeConsoleWindow()
{
#if PLATFORM_WINDOWS
    if (bConsoleAllocated)
    {
        // 不调用 FreeConsole，保持控制台窗口存活，以便下次PIE会话可以重用
        // ::FreeConsole();
        
        // 只是标记为未分配，但保留句柄
        // ConsoleHandle = nullptr;  // 保留句柄，下次可以重用
        bConsoleAllocated = false;
        UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: Console marked as freed (window kept alive for reuse)"));
    }
#endif
}

void FPerformanceConsoleManager::ProcessKeyboardInput()
{
#if PLATFORM_WINDOWS
    // 检查是否有键盘输入（非阻塞）
    if (_kbhit())
    {
        int ch = _getch();
        
        // P 或 p 键切换暂停状态
        if (ch == 'P' || ch == 'p')
        {
            bPaused = !bPaused;
            UE_LOG(LogTemp, Log, TEXT("PerformanceConsoleManager: %s"), 
                bPaused ? TEXT("Paused") : TEXT("Resumed"));
        }
    }
#endif
}