// Copyright FlowVisualizer Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

#if WITH_FMOD_STUDIO
#include "fmod_studio.hpp"
#endif

// Forward declarations for FMOD types used in the full-feature branch
#if WITH_FMOD_STUDIO
namespace FMOD { namespace Studio { class EventInstance; } }
#endif

/**
 * FFlowFMODBridge — Singleton bridge for automatic FMOD audio event collection.
 *
 * When WITH_FMOD_STUDIO=1, this class hooks into FMOD EventInstance callbacks
 * to automatically feed audio events into FFlowTracer.
 * When WITH_FMOD_STUDIO=0, all monitoring APIs compile to no-ops.
 */
class FLOWVISUALIZERRUNTIME_API FFlowFMODBridge
{
public:
	/** Returns the singleton instance (Meyer's Singleton). */
	static FFlowFMODBridge& Get();

	// -----------------------------------------------------------------
	// Monitoring API — conditional on FMOD availability
	// -----------------------------------------------------------------

#if WITH_FMOD_STUDIO
	/**
	 * Register an FMOD EventInstance for automatic monitoring.
	 *
	 * @param Instance  Pointer to a live FMOD::Studio::EventInstance.
	 * @param Context   Optional human-readable context string (e.g. "Footstep").
	 */
	void MonitorEventInstance(FMOD::Studio::EventInstance* Instance, const FString& Context = TEXT(""));

	/**
	 * Stop monitoring a previously registered EventInstance.
	 *
	 * @param Instance  The same pointer passed to MonitorEventInstance.
	 */
	void StopMonitoringInstance(FMOD::Studio::EventInstance* Instance);
#else
	/** No-op stub when FMOD is not available. */
	void MonitorEventInstance(void* Instance, const FString& Context = TEXT("")) {}

	/** No-op stub when FMOD is not available. */
	void StopMonitoringInstance(void* Instance) {}
#endif

	// -----------------------------------------------------------------
	// Manual signal helpers — always available regardless of FMOD
	// -----------------------------------------------------------------

	/** Signal that an FMOD bank has been loaded. */
	void SignalBankLoad(const FString& BankName);

	/** Signal that an FMOD bank has been unloaded. */
	void SignalBankUnload(const FString& BankName);

	/** Signal that an FMOD global parameter has been changed. */
	void SignalGlobalParameter(const FString& ParamName, float Value);

	// -----------------------------------------------------------------
	// Statistics
	// -----------------------------------------------------------------

	/** Returns the number of currently active monitored event instances. */
	int32 GetActiveMonitoredCount() const;

private:
	FFlowFMODBridge();
	~FFlowFMODBridge();

	// Non-copyable, non-movable (singleton)
	FFlowFMODBridge(const FFlowFMODBridge&) = delete;
	FFlowFMODBridge& operator=(const FFlowFMODBridge&) = delete;
	FFlowFMODBridge(FFlowFMODBridge&&) = delete;
	FFlowFMODBridge& operator=(FFlowFMODBridge&&) = delete;

#if WITH_FMOD_STUDIO
	/** FMOD callback dispatched when a monitored event changes state. */
	static FMOD_RESULT F_CALLBACK EventCallback(
		FMOD_STUDIO_EVENT_CALLBACK_TYPE Type,
		FMOD_STUDIO_EVENTINSTANCE* Event,
		void* Parameters);

	/** Per-instance monitoring data. */
	struct FMonitoredInstance
	{
		FString Context;
	};

	/** Map from raw FMOD EventInstance pointer to monitoring metadata. */
	TMap<FMOD::Studio::EventInstance*, FMonitoredInstance> MonitoredInstances;

	/** Thread-safety lock for MonitoredInstances access. */
	mutable FCriticalSection InstanceLock;
#endif
};
