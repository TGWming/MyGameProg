#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPerformanceStatsPluginModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    static inline FPerformanceStatsPluginModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FPerformanceStatsPluginModule>("PerformanceStatsPlugin");
    }
    
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("PerformanceStatsPlugin");
    }
};
