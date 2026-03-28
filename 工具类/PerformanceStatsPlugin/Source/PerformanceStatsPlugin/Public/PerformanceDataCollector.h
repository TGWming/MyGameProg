#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"

/** 颜色等级枚举 */
enum class ECostLevel : uint8
{
    Green,
    Yellow,
    Red
};

/** 资产类型枚举 */
enum class EAssetCategory : uint8
{
    Particle,
    Material,
    SkeletalMesh,
    StaticMesh,
    Blueprint,
    Audio,
    Light,
    Other
};

/** 单个资产的性能数据 */
struct FAssetPerformanceData
{
    FString AssetName;
    FString AssetPath;
    FString ClassName;
    EAssetCategory Category;
    float CostMs;
    ECostLevel CostLevel;
    int32 InstanceCount;
    
    FAssetPerformanceData()
        : Category(EAssetCategory::Other)
        , CostMs(0.0f)
        , CostLevel(ECostLevel::Green)
        , InstanceCount(1)
    {}
};

/** 类别汇总数据 */
struct FCategoryPerformanceData
{
    EAssetCategory Category;
    float TotalCostMs;
    ECostLevel CostLevel;
    TArray<FAssetPerformanceData> Assets;
    
    FCategoryPerformanceData()
        : Category(EAssetCategory::Other)
        , TotalCostMs(0.0f)
        , CostLevel(ECostLevel::Green)
    {}
};

/** 帧性能数据 */
struct FFramePerformanceData
{
    float FrameTimeMs;
    float GameThreadTimeMs;
    float RenderThreadTimeMs;
    float GPUTimeMs;
    float FPS;
    
    ECostLevel FrameTimeCostLevel;
    ECostLevel GameThreadCostLevel;
    ECostLevel RenderThreadCostLevel;
    ECostLevel GPUCostLevel;
    
    FFramePerformanceData()
        : FrameTimeMs(0.0f)
        , GameThreadTimeMs(0.0f)
        , RenderThreadTimeMs(0.0f)
        , GPUTimeMs(0.0f)
        , FPS(0.0f)
        , FrameTimeCostLevel(ECostLevel::Green)
        , GameThreadCostLevel(ECostLevel::Green)
        , RenderThreadCostLevel(ECostLevel::Green)
        , GPUCostLevel(ECostLevel::Green)
    {}
};

/** 性能数据采集器 */
class PERFORMANCESTATSPLUGIN_API FPerformanceDataCollector
{
public:
    static FPerformanceDataCollector& Get();
    
    /** 采集所有性能数据 */
    void CollectAllData(UWorld* World);
    
    /** 获取帧性能数据 */
    const FFramePerformanceData& GetFrameData() const { return FrameData; }
    
    /** 获取 Top N 最消耗性能的资产 */
    const TArray<FAssetPerformanceData>& GetTopExpensiveAssets() const { return TopExpensiveAssets; }
    
    /** 获取按类别分组的数据 */
    const TArray<FCategoryPerformanceData>& GetCategoryData() const { return CategoryData; }
    
    /** 判断消耗等级 */
    static ECostLevel GetCostLevel(float CostMs, float GreenMax, float YellowMax);
    
    /** 获取类别名称 */
    static const TCHAR* GetCategoryName(EAssetCategory Category);
    
private:
    FPerformanceDataCollector() = default;
    
    void CollectFrameData();
    void CollectAssetData(UWorld* World);
    void CollectParticleData(UWorld* World);
    void CollectSkeletalMeshData(UWorld* World);
    void CollectStaticMeshData(UWorld* World);
    void CollectBlueprintData(UWorld* World);
    void CollectAudioData(UWorld* World);
    void CollectLightData(UWorld* World);
    
    void SortAndCategorize();
    void CalculateCostLevels();
    
    FFramePerformanceData FrameData;
    TArray<FAssetPerformanceData> AllAssets;
    TArray<FAssetPerformanceData> TopExpensiveAssets;
    TArray<FCategoryPerformanceData> CategoryData;
    
    FCriticalSection DataCriticalSection;
};

