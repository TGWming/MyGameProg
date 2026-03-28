// Copyright FlowVisualizer Plugin. All Rights Reserved.
// PA.2 — FFMODAutoCapture: auto-hooks FMOD global callbacks during PIE.

#pragma once

#include "CoreMinimal.h"

#if !WITH_FMOD_STUDIO

/**
 * FFMODAutoCapture — Stub when FMOD is unavailable.
 * All methods are inline no-ops so that other files can safely include this header.
 */
class FFMODAutoCapture
{
public:
	static FFMODAutoCapture& Get() { static FFMODAutoCapture Inst; return Inst; }
	void Initialize() {}
	void Shutdown() {}

private:
	FFMODAutoCapture() = default;
	~FFMODAutoCapture() = default;
	FFMODAutoCapture(const FFMODAutoCapture&) = delete;
	FFMODAutoCapture& operator=(const FFMODAutoCapture&) = delete;
	FFMODAutoCapture(FFMODAutoCapture&&) = delete;
	FFMODAutoCapture& operator=(FFMODAutoCapture&&) = delete;
};

#else // WITH_FMOD_STUDIO == 1

#include "fmod_studio.hpp"
#include "TimerManager.h"

// Forward declarations
namespace FMOD { namespace Studio { class System; } }

/**
 * FFMODAutoCapture — Singleton that automatically captures FMOD events during PIE.
 *
 * On PIE start it registers a global FMOD System callback to intercept all
 * EventInstance create/destroy/start/stop events and forwards them through
 * FlowFMODBridge / FLOW_SIGNAL. It also periodically polls global parameters
 * and emits signals for value changes.
 */
class FFMODAutoCapture
{
public:
	// -----------------------------------------------------------------
	// Lifecycle
	// -----------------------------------------------------------------

	/** Returns the singleton instance (Meyer's Singleton). */
	static FFMODAutoCapture& Get();

	/** Register PIE lifecycle callbacks (BeginPIE / EndPIE). */
	void Initialize();

	/** Unregister callbacks and release all resources. */
	void Shutdown();

	// -----------------------------------------------------------------
	// PIE callbacks
	// -----------------------------------------------------------------

	/** Called when PIE starts. Triggers StartCapture(). */
	void OnBeginPIE(bool bIsSimulating);

	/** Called when PIE ends. Triggers StopCapture(). */
	void OnEndPIE(bool bIsSimulating);

	// -----------------------------------------------------------------
	// Capture control
	// -----------------------------------------------------------------

	/** Acquire FMOD Studio System, register global System callback, start parameter polling timer. */
	void StartCapture();

	/** Unregister global callback, stop polling timer, clean up monitored instances. */
	void StopCapture();

	// -----------------------------------------------------------------
	// FMOD global callback (static, C-linkage compatible)
	// -----------------------------------------------------------------

	/** Global System callback registered with FMOD Studio System. */
	static FMOD_RESULT F_CALLBACK FMODSystemCallback(
		FMOD_STUDIO_SYSTEM* system,
		FMOD_STUDIO_SYSTEM_CALLBACK_TYPE type,
		void* commanddata,
		void* userdata);

	// -----------------------------------------------------------------
	// Parameter polling
	// -----------------------------------------------------------------

	/** Read all global parameters and emit FLOW_SIGNAL for changed values. */
	void PollGlobalParameters();

private:
	FFMODAutoCapture() = default;
	~FFMODAutoCapture() = default;
	FFMODAutoCapture(const FFMODAutoCapture&) = delete;
	FFMODAutoCapture& operator=(const FFMODAutoCapture&) = delete;
	FFMODAutoCapture(FFMODAutoCapture&&) = delete;
	FFMODAutoCapture& operator=(FFMODAutoCapture&&) = delete;

	// -----------------------------------------------------------------
	// Private members
	// -----------------------------------------------------------------

	/** Timer handle for periodic global parameter polling. */
	FTimerHandle PollTimerHandle;

	/** Snapshot of last-known global parameter values for change detection. */
	TMap<FString, float> LastParameterSnapshot;

	/** Delegate handle for FEditorDelegates::BeginPIE. */
	FDelegateHandle BeginPIEHandle;

	/** Delegate handle for FEditorDelegates::EndPIE. */
	FDelegateHandle EndPIEHandle;

	/** True while actively capturing FMOD events. */
	bool bIsCapturing = false;

	/** Cached pointer to the FMOD Studio System used during capture. */
	FMOD::Studio::System* CachedStudioSystem = nullptr;
};

#endif // WITH_FMOD_STUDIO
