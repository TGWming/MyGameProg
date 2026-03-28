// Copyright FlowVisualizer Plugin. All Rights Reserved.
// PA.3 — FFMODAutoCapture: PIE lifecycle bindings (skeleton implementation).

#include "FMODAutoCapture.h"

#if WITH_FMOD_STUDIO

#include "Editor.h" // FEditorDelegates
#include "FMODStudioModule.h"
#include "fmod_studio.hpp"
#include "FlowFMODBridge.h"
#include "Async/Async.h"
#include "FlowTracerMacros.h"

/** File-local set tracking instances already forwarded to FFlowFMODBridge.
 *  Accessed only from the FMOD thread (inside POSTUPDATE callback) — no lock needed. */
static TSet<FMOD::Studio::EventInstance*> GKnownInstances;

// -----------------------------------------------------------------
// Singleton
// -----------------------------------------------------------------

FFMODAutoCapture& FFMODAutoCapture::Get()
{
	static FFMODAutoCapture Instance;
	return Instance;
}

// -----------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------

void FFMODAutoCapture::Initialize()
{
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FFMODAutoCapture::OnBeginPIE);
	EndPIEHandle   = FEditorDelegates::EndPIE.AddRaw(this, &FFMODAutoCapture::OnEndPIE);
}

void FFMODAutoCapture::Shutdown()
{
	FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);

	if (bIsCapturing)
	{
		StopCapture();
	}
}

// -----------------------------------------------------------------
// PIE callbacks
// -----------------------------------------------------------------

void FFMODAutoCapture::OnBeginPIE(bool bIsSimulating)
{
	StartCapture();
}

void FFMODAutoCapture::OnEndPIE(bool bIsSimulating)
{
	StopCapture();
}

// -----------------------------------------------------------------
// Capture control
// -----------------------------------------------------------------

void FFMODAutoCapture::StartCapture()
{
	if (bIsCapturing)
	{
		return;
	}

	// Acquire FMOD Studio System for Runtime context (PIE)
	if (!IFMODStudioModule::IsAvailable())
	{
		UE_LOG(LogTemp, Warning, TEXT("[FlowVis] IFMODStudioModule not available — FMOD auto-capture aborted."));
		return;
	}

	CachedStudioSystem = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	if (!CachedStudioSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FlowVis] FMOD Studio System is null — FMOD auto-capture aborted."));
		return;
	}

	// Clear tracking set from any previous session
	GKnownInstances.Empty();

	// Register global System callback — use POSTUPDATE to scan for new instances
	// (this FMOD version does not expose EVENT_CREATED/DESTROYED at the system level)
	CachedStudioSystem->setCallback(
		&FFMODAutoCapture::FMODSystemCallback,
		FMOD_STUDIO_SYSTEM_CALLBACK_POSTUPDATE
	);

	bIsCapturing = true;

	// Start parameter polling timer (every 200ms)
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimer(
			PollTimerHandle,
			FTimerDelegate::CreateRaw(this, &FFMODAutoCapture::PollGlobalParameters),
			0.2f,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("[FlowVis] FMOD Auto-Capture started"));
}

void FFMODAutoCapture::StopCapture()
{
	if (!bIsCapturing)
	{
		return;
	}

	if (CachedStudioSystem)
	{
		CachedStudioSystem->setCallback(nullptr, 0);
	}

	if (GEditor)
	{
		GEditor->GetTimerManager()->ClearTimer(PollTimerHandle);
	}

	GKnownInstances.Empty();
	LastParameterSnapshot.Empty();
	CachedStudioSystem = nullptr;
	bIsCapturing = false;

	UE_LOG(LogTemp, Log, TEXT("[FlowVis] FMOD Auto-Capture stopped"));
}

// -----------------------------------------------------------------
// FMOD global callback
// -----------------------------------------------------------------

FMOD_RESULT F_CALLBACK FFMODAutoCapture::FMODSystemCallback(
	FMOD_STUDIO_SYSTEM* system,
	FMOD_STUDIO_SYSTEM_CALLBACK_TYPE type,
	void* commanddata,
	void* userdata)
{
	if (type != FMOD_STUDIO_SYSTEM_CALLBACK_POSTUPDATE)
	{
		return FMOD_OK;
	}

	// Enumerate all loaded banks → event descriptions → live instances.
	// Any instance not yet in GKnownInstances is newly created.
	FMOD::Studio::System* StudioSystem = reinterpret_cast<FMOD::Studio::System*>(system);

	int BankCount = 0;
	StudioSystem->getBankCount(&BankCount);
	if (BankCount <= 0)
	{
		return FMOD_OK;
	}

	TArray<FMOD::Studio::Bank*> Banks;
	Banks.SetNumUninitialized(BankCount);
	int BankRetrieved = 0;
	if (StudioSystem->getBankList(Banks.GetData(), BankCount, &BankRetrieved) != FMOD_OK)
	{
		return FMOD_OK;
	}

	for (int32 b = 0; b < BankRetrieved; ++b)
	{
		int EventCount = 0;
		Banks[b]->getEventCount(&EventCount);
		if (EventCount <= 0)
		{
			continue;
		}

		TArray<FMOD::Studio::EventDescription*> Descriptions;
		Descriptions.SetNumUninitialized(EventCount);
		int DescRetrieved = 0;
		if (Banks[b]->getEventList(Descriptions.GetData(), EventCount, &DescRetrieved) != FMOD_OK)
		{
			continue;
		}

		for (int32 d = 0; d < DescRetrieved; ++d)
		{
			int InstCount = 0;
			Descriptions[d]->getInstanceCount(&InstCount);
			if (InstCount <= 0)
			{
				continue;
			}

			TArray<FMOD::Studio::EventInstance*> Instances;
			Instances.SetNumUninitialized(InstCount);
			int InstRetrieved = 0;
			if (Descriptions[d]->getInstanceList(Instances.GetData(), InstCount, &InstRetrieved) != FMOD_OK)
			{
				continue;
			}

			for (int32 i = 0; i < InstRetrieved; ++i)
			{
				FMOD::Studio::EventInstance* Inst = Instances[i];
				if (GKnownInstances.Contains(Inst))
				{
					continue; // Already monitored
				}

				GKnownInstances.Add(Inst);

				// Extract event path while FMOD pointers are valid
				FString EventPath = TEXT("unknown");
				char PathBuf[256];
				int PathRetrieved = 0;
				if (Descriptions[d]->getPath(PathBuf, sizeof(PathBuf), &PathRetrieved) == FMOD_OK)
				{
					EventPath = FString(UTF8_TO_TCHAR(PathBuf));
				}

				// Dispatch to GameThread — capture EventPath by value (no raw FMOD pointer in lambda closure)
				AsyncTask(ENamedThreads::GameThread, [Inst, EventPath]()
				{
					FFlowFMODBridge::Get().MonitorEventInstance(Inst, EventPath);
				});
			}
		}
	}

	return FMOD_OK;
}

// -----------------------------------------------------------------
// Parameter polling
// -----------------------------------------------------------------

void FFMODAutoCapture::PollGlobalParameters()
{
	if (!CachedStudioSystem || !bIsCapturing)
	{
		return;
	}

	int Count = 0;
	if (CachedStudioSystem->getParameterDescriptionCount(&Count) != FMOD_OK || Count <= 0)
	{
		return;
	}

	TArray<FMOD_STUDIO_PARAMETER_DESCRIPTION> Descriptions;
	Descriptions.SetNumUninitialized(Count);
	int Retrieved = 0;
	if (CachedStudioSystem->getParameterDescriptionList(Descriptions.GetData(), Count, &Retrieved) != FMOD_OK)
	{
		return;
	}

	const bool bIsFirstPoll = LastParameterSnapshot.Num() == 0;

	for (int i = 0; i < Retrieved; ++i)
	{
		const FMOD_STUDIO_PARAMETER_DESCRIPTION& Desc = Descriptions[i];

		// Only track global, game-controlled parameters
		if (!(Desc.flags & FMOD_STUDIO_PARAMETER_GLOBAL))
		{
			continue;
		}

		float CurrentValue = 0.0f;
		if (CachedStudioSystem->getParameterByID(Desc.id, &CurrentValue) != FMOD_OK)
		{
			continue;
		}

		FString ParamName = FString(UTF8_TO_TCHAR(Desc.name));

		if (bIsFirstPoll)
		{
			// First poll — populate snapshot without emitting signals
			LastParameterSnapshot.Add(ParamName, CurrentValue);
		}
		else
		{
			float* PreviousValue = LastParameterSnapshot.Find(ParamName);
			if (!PreviousValue || FMath::Abs(CurrentValue - *PreviousValue) > 0.001f)
			{
				FLOW_SIGNAL_FMT("FMOD_GlobalParam", "Param=%s Val=%.3f", *ParamName, CurrentValue);
				LastParameterSnapshot.Add(ParamName, CurrentValue);
			}
		}
	}
}

#endif // WITH_FMOD_STUDIO
