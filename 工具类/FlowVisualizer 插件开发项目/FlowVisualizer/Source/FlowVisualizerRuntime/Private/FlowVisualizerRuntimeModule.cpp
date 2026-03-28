#include "FlowVisualizerRuntimeModule.h"

void FFlowVisualizerRuntimeModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FlowVisualizerRuntime module loaded"));
}

void FFlowVisualizerRuntimeModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("FlowVisualizerRuntime module unloaded"));
}

IMPLEMENT_MODULE(FFlowVisualizerRuntimeModule, FlowVisualizerRuntime)
