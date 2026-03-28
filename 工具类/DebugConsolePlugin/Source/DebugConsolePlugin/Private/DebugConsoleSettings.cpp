// DebugConsoleSettings.cpp
#include "DebugConsoleSettings.h"

UDebugConsoleSettings::UDebugConsoleSettings()
{
    CategoryName = TEXT("Plugins");
    
    // 设置默认值
    bEnableDebugConsole = true;
    bWaitForKeyOnExit = true;
    ConsoleTitle = TEXT("Game Debug Console");
    bShowTimestamp = true;
    bShowCategory = true;
    bShowVerbosity = true;
    MinVerbosity = 4; // 4 = Log level
    
    // 事件监控默认值
    bMonitorAssetLoading = true;
    bMonitorActorSpawnDestroy = true;
    bMonitorPropertyChanges = false;  // 默认关闭，因为可能很频繁
    bMonitorWorldEvents = true;
    bMonitorGameFlowEvents = true;
    bMonitorEditorSelection = true;
}
