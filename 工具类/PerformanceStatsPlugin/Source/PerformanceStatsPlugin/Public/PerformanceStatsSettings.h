#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PerformanceStatsSettings.generated.h"

UCLASS(config=PerformanceStats, defaultconfig, meta=(DisplayName="Performance Stats Settings"))
class PERFORMANCESTATSPLUGIN_API UPerformanceStatsSettings : public UDeveloperSettings
{
    GENERATED_BODY()
    
public:
    UPerformanceStatsSettings();
    
    /** 是否启用性能监控 */
    UPROPERTY(config, EditAnywhere, Category="General")
    bool bEnablePerformanceStats;
    
    /** 控制台窗口标题 */
    UPROPERTY(config, EditAnywhere, Category="General")
    FString ConsoleTitle;
    
    /** 数据刷新间隔（秒） */
    UPROPERTY(config, EditAnywhere, Category="General", meta=(ClampMin="0.1", ClampMax="5.0"))
    float RefreshInterval;
    
    /** 显示前 N 个最消耗性能的项目 */
    UPROPERTY(config, EditAnywhere, Category="General", meta=(ClampMin="5", ClampMax="50"))
    int32 TopItemsCount;
    
    /** 窗口关闭时是否等待按键 */
    UPROPERTY(config, EditAnywhere, Category="General")
    bool bWaitForKeyOnExit;
    
    /** 帧时间 - 绿色最大值 (ms) */
    UPROPERTY(config, EditAnywhere, Category="Thresholds", meta=(ClampMin="1.0", ClampMax="100.0"))
    float FrameTimeGreenMax;
    
    /** 帧时间 - 黄色最大值 (ms) */
    UPROPERTY(config, EditAnywhere, Category="Thresholds", meta=(ClampMin="1.0", ClampMax="100.0"))
    float FrameTimeYellowMax;
    
    /** 单项资产消耗 - 绿色最大值 (ms) */
    UPROPERTY(config, EditAnywhere, Category="Thresholds", meta=(ClampMin="0.1", ClampMax="50.0"))
    float AssetCostGreenMax;
    
    /** 单项资产消耗 - 黄色最大值 (ms) */
    UPROPERTY(config, EditAnywhere, Category="Thresholds", meta=(ClampMin="0.1", ClampMax="50.0"))
    float AssetCostYellowMax;
    
    /** 追踪粒子系统 */
    UPROPERTY(config, EditAnywhere, Category="Categories")
    bool bTrackParticles;
    
    /** 追踪材质 */
    UPROPERTY(config, EditAnywhere, Category="Categories")
    bool bTrackMaterials;
    
    /** 追踪骨骼网格 */
    UPROPERTY(config, EditAnywhere, Category="Categories")
    bool bTrackSkeletalMeshes;
    
    /** 追踪静态网格 */
    UPROPERTY(config, EditAnywhere, Category="Categories")
    bool bTrackStaticMeshes;
    
    /** 追踪 Blueprint */
    UPROPERTY(config, EditAnywhere, Category="Categories")
    bool bTrackBlueprints;
    
    /** 追踪音频 */
    UPROPERTY(config, EditAnywhere, Category="Categories")
    bool bTrackAudio;
    
    /** 追踪光照 */
    UPROPERTY(config, EditAnywhere, Category="Categories")
    bool bTrackLights;
    
    static const UPerformanceStatsSettings* Get()
    {
        return GetDefault<UPerformanceStatsSettings>();
    }
};
