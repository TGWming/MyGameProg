// Copyright FlowVisualizer Plugin. All Rights Reserved.
// PA.3 — FFMODAutoCapture: header for PIE lifecycle bindings.

#pragma once

#include "CoreMinimal.h"

#if WITH_FMOD_STUDIO
#include "fmod_studio.hpp"
#endif

/**
 * FFMODAutoCapture — Singleton that automatically discovers and monitors all live
 * FMOD event instances during PIE sessions.
 *
 * Lifecycle:
 *   Initialize() — called once from EditorModule::StartupModule(); registers PIE delegates.
 *   Shutdown()    — called once from EditorModule::ShutdownModule(); unregisters everything.
 *
 * During PIE the class:
 *   - Installs a FMOD System POSTUPDATE callback that scans for new EventInstances
 *     and forwards them to FFlowFMODBridge for per-event callback monitoring.
 *   - Polls global FMOD parameters every 200 ms and emits FLOW_SIGNAL on change.
 */
class FFMODAutoCapture
{
public:
	/** Meyer's singleton accessor. */
	static FFMODAutoCapture& Get();

	/** Register PIE begin/end delegates. Call once from StartupModule(). */
	void Initialize();

	/** Unregister delegates and stop capture if active. Call once from ShutdownModule(). */
	void Shutdown();

private:
	FFMODAutoCapture() = default;
	~FFMODAutoCapture() = default;

	// Non-copyable, non-movable
	FFMODAutoCapture(const FFMODAutoCapture&) = delete;
	FFMODAutoCapture& operator=(const FFMODAutoCapture&) = delete;

	/** PIE callbacks */
	void OnBeginPIE(bool bIsSimulating);
	void OnEndPIE(bool bIsSimulating);

	/** Start / stop the FMOD capture session. */
	void StartCapture();
	void StopCapture();

#if WITH_FMOD_STUDIO
	/** FMOD System-level callback (POSTUPDATE) — scans for new instances. */
	static FMOD_RESULT F_CALLBACK FMODSystemCallback(
		FMOD_STUDIO_SYSTEM* system,
		FMOD_STUDIO_SYSTEM_CALLBACK_TYPE type,
		void* commanddata,
		void* userdata);

	/** Periodic polling of global FMOD parameters. */
	void PollGlobalParameters();

	FMOD::Studio::System* CachedStudioSystem = nullptr;
	FTimerHandle PollTimerHandle;
	TMap<FString, float> LastParameterSnapshot;
#endif

	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
	bool bIsCapturing = false;
};
