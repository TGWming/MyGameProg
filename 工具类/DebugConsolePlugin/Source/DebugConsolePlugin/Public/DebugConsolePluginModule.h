// DebugConsolePluginModule.h
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDebugConsolePluginModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    static inline FDebugConsolePluginModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FDebugConsolePluginModule>("DebugConsolePlugin");
    }
    
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("DebugConsolePlugin");
    }
};
