#include "PerformanceStatsSettings.h"

UPerformanceStatsSettings::UPerformanceStatsSettings()
{
    CategoryName = TEXT("Plugins");
    
    bEnablePerformanceStats = true;
    ConsoleTitle = TEXT("Performance Stats Monitor");
    RefreshInterval = 0.5f;
    TopItemsCount = 10;
    bWaitForKeyOnExit = true;
    
    FrameTimeGreenMax = 16.67f;
    FrameTimeYellowMax = 33.33f;
    AssetCostGreenMax = 2.0f;
    AssetCostYellowMax = 5.0f;
    
    bTrackParticles = true;
    bTrackMaterials = true;
    bTrackSkeletalMeshes = true;
    bTrackStaticMeshes = true;
    bTrackBlueprints = true;
    bTrackAudio = true;
    bTrackLights = true;
}
