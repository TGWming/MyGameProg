#include "PerformanceStatsFunctionLibrary.h"
#include "PerformanceConsoleManager.h"

void UPerformanceStatsFunctionLibrary::EnablePerformanceMonitor()
{
#if WITH_PERFORMANCE_STATS
	FPerformanceConsoleManager::Get().SetEnabled(true);
#endif
}

void UPerformanceStatsFunctionLibrary::DisablePerformanceMonitor()
{
#if WITH_PERFORMANCE_STATS
	FPerformanceConsoleManager::Get().SetEnabled(false);
#endif
}

void UPerformanceStatsFunctionLibrary::TogglePerformanceMonitor()
{
#if WITH_PERFORMANCE_STATS
	FPerformanceConsoleManager& Manager = FPerformanceConsoleManager::Get();
	Manager.SetEnabled(!Manager.IsEnabled());
#endif
}

bool UPerformanceStatsFunctionLibrary::IsPerformanceMonitorEnabled()
{
#if WITH_PERFORMANCE_STATS
	return FPerformanceConsoleManager::Get().IsEnabled();
#else
	return false;
#endif
}

void UPerformanceStatsFunctionLibrary::PausePerformanceMonitor()
{
#if WITH_PERFORMANCE_STATS
	FPerformanceConsoleManager::Get().SetPaused(true);
#endif
}

void UPerformanceStatsFunctionLibrary::ResumePerformanceMonitor()
{
#if WITH_PERFORMANCE_STATS
	FPerformanceConsoleManager::Get().SetPaused(false);
#endif
}

void UPerformanceStatsFunctionLibrary::TogglePausePerformanceMonitor()
{
#if WITH_PERFORMANCE_STATS
	FPerformanceConsoleManager& Manager = FPerformanceConsoleManager::Get();
	Manager.SetPaused(!Manager.IsPaused());
#endif
}

bool UPerformanceStatsFunctionLibrary::IsPerformanceMonitorPaused()
{
#if WITH_PERFORMANCE_STATS
	return FPerformanceConsoleManager::Get().IsPaused();
#else
	return false;
#endif
}
