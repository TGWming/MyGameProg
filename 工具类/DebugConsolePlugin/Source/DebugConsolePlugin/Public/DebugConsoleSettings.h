// DebugConsoleSettings.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DebugConsoleSettings.generated.h"

UCLASS(config=DebugConsole, defaultconfig, meta=(DisplayName="Debug Console Settings"))
class DEBUGCONSOLEPLUGIN_API UDebugConsoleSettings : public UDeveloperSettings
{
    GENERATED_BODY()
    
public:
    UDebugConsoleSettings();
    
    /** 是否启用调试控制台 */
    UPROPERTY(config, EditAnywhere, Category="General")
    bool bEnableDebugConsole;
    
    /** 游戏退出后是否等待按键 */
    UPROPERTY(config, EditAnywhere, Category="General")
    bool bWaitForKeyOnExit;
    
    /** 控制台窗口标题 */
    UPROPERTY(config, EditAnywhere, Category="Appearance")
    FString ConsoleTitle;
    
    /** 是否显示时间戳 */
    UPROPERTY(config, EditAnywhere, Category="Output")
    bool bShowTimestamp;
    
    /** 是否显示日志类别 */
    UPROPERTY(config, EditAnywhere, Category="Output")
    bool bShowCategory;
    
    /** 是否显示日志级别 */
    UPROPERTY(config, EditAnywhere, Category="Output")
    bool bShowVerbosity;
    
    /** 要过滤的日志类别（留空则显示全部） */
    UPROPERTY(config, EditAnywhere, Category="Filter")
    TArray<FName> CategoryFilter;
    
    /** 最低日志级别（0=Fatal, 1=Error, 2=Warning, 3=Display, 4=Log, 5=Verbose, 6=VeryVerbose） */
    UPROPERTY(config, EditAnywhere, Category="Filter", meta=(ClampMin="0", ClampMax="6"))
    int32 MinVerbosity;
    
    /** 是否监控资源加载事件 */
    UPROPERTY(config, EditAnywhere, Category="Event Monitor")
    bool bMonitorAssetLoading;
    
    /** 是否监控Actor生成/销毁 */
    UPROPERTY(config, EditAnywhere, Category="Event Monitor")
    bool bMonitorActorSpawnDestroy;
    
    /** 是否监控对象属性修改 */
    UPROPERTY(config, EditAnywhere, Category="Event Monitor")
    bool bMonitorPropertyChanges;
    
    /** 是否监控世界/关卡事件 */
    UPROPERTY(config, EditAnywhere, Category="Event Monitor")
    bool bMonitorWorldEvents;
    
    /** 是否监控游戏流程事件 */
    UPROPERTY(config, EditAnywhere, Category="Event Monitor")
    bool bMonitorGameFlowEvents;
    
    /** 是否监控编辑器选择事件（仅编辑器有效） */
    UPROPERTY(config, EditAnywhere, Category="Event Monitor")
    bool bMonitorEditorSelection;
    
    // 获取设置实例
    static const UDebugConsoleSettings* Get()
    {
        return GetDefault<UDebugConsoleSettings>();
    }
    
    // 获取最低日志级别
    ELogVerbosity::Type GetMinVerbosityAsLogVerbosity() const
    {
        return static_cast<ELogVerbosity::Type>(FMath::Clamp(MinVerbosity, 0, 6));
    }
};
