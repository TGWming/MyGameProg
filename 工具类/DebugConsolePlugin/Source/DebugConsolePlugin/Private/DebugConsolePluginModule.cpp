// DebugConsolePluginModule.cpp
#include "DebugConsolePluginModule.h"
#include "DebugConsoleManager.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FDebugConsolePluginModule"

void FDebugConsolePluginModule::StartupModule()
{
    // 不要在这里直接初始化！
    // 使用延迟初始化，等待引擎完全启动
    FCoreDelegates::OnPostEngineInit.AddLambda([]()
    {
        FDebugConsoleManager::Get().Initialize();
        UE_LOG(LogTemp, Log, TEXT("DebugConsolePlugin: Module started (delayed init)"));
    });
}

void FDebugConsolePluginModule::ShutdownModule()
{
    FDebugConsoleManager::Get().Shutdown();
    UE_LOG(LogTemp, Log, TEXT("DebugConsolePlugin: Module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDebugConsolePluginModule, DebugConsolePlugin)
