#include "PerformanceStatsPluginModule.h"
#include "PerformanceConsoleManager.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FPerformanceStatsPluginModule"

void FPerformanceStatsPluginModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddLambda([]()
	{
		FPerformanceConsoleManager::Get().Initialize();
		UE_LOG(LogTemp, Log, TEXT("PerformanceStatsPlugin: Module started (delayed init)"));
	});
}

void FPerformanceStatsPluginModule::ShutdownModule()
{
	FPerformanceConsoleManager::Get().Shutdown();
	UE_LOG(LogTemp, Log, TEXT("PerformanceStatsPlugin: Module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPerformanceStatsPluginModule, PerformanceStatsPlugin)
