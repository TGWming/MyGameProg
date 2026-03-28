#pragma once

#include "Modules/ModuleManager.h"

class FFlowVisualizerRuntimeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
