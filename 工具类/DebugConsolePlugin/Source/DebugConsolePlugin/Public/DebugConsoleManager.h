// DebugConsoleManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"  // 添加这行，用于 UObjectInitialized()

class FDebugConsoleOutputDevice;

class DEBUGCONSOLEPLUGIN_API FDebugConsoleManager
{
public:
    /** 获取单例实例 */
    static FDebugConsoleManager& Get();
    
    /** 初始化控制台（插件自动调用） */
    void Initialize();
    
    /** 关闭控制台（插件自动调用） */
    void Shutdown();
    
    /** 检查控制台是否已初始化 */
    bool IsInitialized() const { return bIsInitialized; }
    
    /** 直接向控制台输出文本（可选，供外部使用） */
    void PrintToConsole(const FString& Message);
    
private:
    FDebugConsoleManager() = default;
    ~FDebugConsoleManager() = default;
    
    // 禁止拷贝
    FDebugConsoleManager(const FDebugConsoleManager&) = delete;
    FDebugConsoleManager& operator=(const FDebugConsoleManager&) = delete;
    
    void AllocateConsole();
    void FreeConsoleWindow();
    void WaitForKeyPress();
    void OnEngineExit();
    
    bool bIsInitialized = false;
    bool bConsoleAllocated = false;
    
    TSharedPtr<FDebugConsoleOutputDevice> OutputDevice;
    FDelegateHandle ExitDelegateHandle;
};
