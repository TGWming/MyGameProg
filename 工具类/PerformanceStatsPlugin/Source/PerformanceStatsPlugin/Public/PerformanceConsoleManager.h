#pragma once

#include "CoreMinimal.h"
#include "PerformanceDataCollector.h"

#if PLATFORM_WINDOWS
#include "Windows/MinWindows.h"
#endif

// 控制台性能数据管理器
class PERFORMANCESTATSPLUGIN_API FPerformanceConsoleManager
{
public:
    // 单例访问
    static FPerformanceConsoleManager& Get();

    // 初始化
    void Initialize();
    // 清理
    void Shutdown();

    // 控制方法
    void SetEnabled(bool bInEnabled);
    bool IsEnabled() const;
    void SetPaused(bool bInPaused);
    bool IsPaused() const;

    // Tick更新
    void Tick(float DeltaTime);
    
    // 分配控制台窗口
    void AllocateConsole();
    // 释放控制台窗口
    void FreeConsoleWindow();
    
    // 处理键盘输入
    void ProcessKeyboardInput();

private:
// 刷新控制台显示
void RefreshDisplay();
// 打印帧数据
void PrintFrameData(const FFramePerformanceData& Data);
    
// 打印 Top N 资产
void PrintTopExpensiveAssets(const TArray<FAssetPerformanceData>& Assets);
    
    // 打印类别数据
    void PrintCategoryData(const TArray<FCategoryPerformanceData>& Categories);
    
    // 获取消耗等级指示符
    const char* GetCostIndicator(ECostLevel Level);
    
    // 设置控制台文本颜色
    void SetConsoleColor(ECostLevel Level);
    void ResetConsoleColor();

private:
float UpdateInterval;        // 刷新间隔
float TimeSinceLastUpdate;   // 距离上次刷新的时间
bool bEnabled;               // 是否启用
bool bPaused;                // 是否暂停
bool bConsoleAllocated;      // 控制台是否已分配

#if PLATFORM_WINDOWS
    HANDLE ConsoleHandle;        // Windows控制台句柄
    WORD DefaultColor;           // 默认颜色
#endif

    FDelegateHandle TickDelegateHandle;  // Tick委托句柄

private:
    FPerformanceConsoleManager();
    ~FPerformanceConsoleManager();
    FPerformanceConsoleManager(const FPerformanceConsoleManager&) = delete;
    FPerformanceConsoleManager& operator=(const FPerformanceConsoleManager&) = delete;
};