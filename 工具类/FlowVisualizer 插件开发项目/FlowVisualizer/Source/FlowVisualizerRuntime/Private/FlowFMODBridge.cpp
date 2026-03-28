// Copyright FlowVisualizer Plugin. All Rights Reserved.

#include "FlowFMODBridge.h"
#include "FlowTracerMacros.h"
#include "Async/Async.h"

// ---------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------

FFlowFMODBridge& FFlowFMODBridge::Get()
{
	static FFlowFMODBridge Instance;
	return Instance;
}

FFlowFMODBridge::FFlowFMODBridge()
{
}

FFlowFMODBridge::~FFlowFMODBridge()
{
}

// ---------------------------------------------------------------------
// Monitoring API (WITH_FMOD_STUDIO=1 only; =0 is inline no-op in header)
// ---------------------------------------------------------------------

#if WITH_FMOD_STUDIO

/** Helper: extract the FMOD event path string from a raw FMOD_STUDIO_EVENTINSTANCE handle. */
static FString GetEventPath(FMOD_STUDIO_EVENTINSTANCE* Event)
{
	FMOD::Studio::EventInstance* Instance = reinterpret_cast<FMOD::Studio::EventInstance*>(Event);
	FMOD::Studio::EventDescription* Desc = nullptr;
	if (Instance->getDescription(&Desc) == FMOD_OK && Desc)
	{
		char PathBuf[256];
		int Retrieved = 0;
		if (Desc->getPath(PathBuf, sizeof(PathBuf), &Retrieved) == FMOD_OK)
		{
			return FString(UTF8_TO_TCHAR(PathBuf));
		}
	}
	return FString(TEXT("unknown"));
}

void FFlowFMODBridge::MonitorEventInstance(FMOD::Studio::EventInstance* Instance, const FString& Context)
{
	if (!Instance)
	{
		return;
	}

	FScopeLock Lock(&InstanceLock);

	if (MonitoredInstances.Contains(Instance))
	{
		return; // Already monitored — idempotent
	}

	FMonitoredInstance Entry;
	Entry.Context = Context;
	MonitoredInstances.Add(Instance, Entry);

	Instance->setCallback(EventCallback, FMOD_STUDIO_EVENT_CALLBACK_ALL);
}

void FFlowFMODBridge::StopMonitoringInstance(FMOD::Studio::EventInstance* Instance)
{
	if (!Instance)
	{
		return;
	}

	FScopeLock Lock(&InstanceLock);
	MonitoredInstances.Remove(Instance);
}

FMOD_RESULT F_CALLBACK FFlowFMODBridge::EventCallback(
	FMOD_STUDIO_EVENT_CALLBACK_TYPE Type,
	FMOD_STUDIO_EVENTINSTANCE* Event,
	void* Parameters)
{
	FMOD::Studio::EventInstance* Instance = reinterpret_cast<FMOD::Studio::EventInstance*>(Event);

	// Extract event path and context while FMOD pointers are still valid
	FString EventPath = GetEventPath(Event);
	FString Context;
	{
		FScopeLock Lock(&Get().InstanceLock);
		if (const FMonitoredInstance* Found = Get().MonitoredInstances.Find(Instance))
		{
			Context = Found->Context;
		}
	}

	if (Type == FMOD_STUDIO_EVENT_CALLBACK_CREATED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_Created", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_STARTED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_Started", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_STOPPED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_Stopped", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_START_FAILED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_StartFailed", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_REAL_TO_VIRTUAL)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_Virtualized", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER)
	{
		const auto* Props = static_cast<const FMOD_STUDIO_TIMELINE_MARKER_PROPERTIES*>(Parameters);
		FString MarkerName = UTF8_TO_TCHAR(Props->name);
		int32 Position = Props->position;

		AsyncTask(ENamedThreads::GameThread, [EventPath, Context, MarkerName, Position]()
		{
			FLOW_SIGNAL("FMOD_Marker", FString::Printf(TEXT("Event=%s Marker=%s Pos=%d Context=%s"), *EventPath, *MarkerName, Position, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT)
	{
		const auto* Props = static_cast<const FMOD_STUDIO_TIMELINE_BEAT_PROPERTIES*>(Parameters);
		int32 Bar = Props->bar;
		int32 Beat = Props->beat;
		float Tempo = Props->tempo;

		AsyncTask(ENamedThreads::GameThread, [EventPath, Context, Bar, Beat, Tempo]()
		{
			FLOW_SIGNAL("FMOD_Beat", FString::Printf(TEXT("Event=%s Bar=%d Beat=%d Tempo=%.1f Context=%s"), *EventPath, Bar, Beat, Tempo, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_VIRTUAL_TO_REAL)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_Restored", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_SoundPlay", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_STOPPED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_SoundStop", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_RESTARTED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_Restarted", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});
	}
	else if (Type == FMOD_STUDIO_EVENT_CALLBACK_DESTROYED)
	{
		AsyncTask(ENamedThreads::GameThread, [EventPath, Context]()
		{
			FLOW_SIGNAL("FMOD_Destroyed", FString::Printf(TEXT("Event=%s Context=%s"), *EventPath, *Context));
		});

		// Clean up TMap entry — instance is about to become invalid
		FScopeLock Lock(&Get().InstanceLock);
		Get().MonitoredInstances.Remove(Instance);
	}

	return FMOD_OK;
}

#endif // WITH_FMOD_STUDIO

// ---------------------------------------------------------------------
// Manual signal helpers (always available)
// ---------------------------------------------------------------------

void FFlowFMODBridge::SignalBankLoad(const FString& BankName)
{
	FLOW_SIGNAL_FMT("FMOD_BankLoad", "Bank=%s Result=OK", *BankName);
}

void FFlowFMODBridge::SignalBankUnload(const FString& BankName)
{
	FLOW_SIGNAL_FMT("FMOD_BankUnload", "Bank=%s", *BankName);
}

void FFlowFMODBridge::SignalGlobalParameter(const FString& ParamName, float Value)
{
	FLOW_SIGNAL_FMT("FMOD_GlobalParam", "Param=%s Value=%.4f", *ParamName, Value);
}

// ---------------------------------------------------------------------
// Statistics
// ---------------------------------------------------------------------

int32 FFlowFMODBridge::GetActiveMonitoredCount() const
{
#if WITH_FMOD_STUDIO
	FScopeLock Lock(&InstanceLock);
	return MonitoredInstances.Num();
#else
	return 0;
#endif
}
