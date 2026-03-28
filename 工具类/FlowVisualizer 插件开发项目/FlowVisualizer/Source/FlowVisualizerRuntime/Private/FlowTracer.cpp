// Copyright FlowVisualizer Plugin. All Rights Reserved.

#include "FlowTracer.h"
#include "HAL/PlatformTime.h"

FFlowTracer& FFlowTracer::Get()
{
	static FFlowTracer Instance;
	return Instance;
}

FFlowTracer::FFlowTracer()
	: BufferHead(0)
	, BufferCapacity(1024)
	, EventCount(0)
{
	EventBuffer.SetNum(BufferCapacity);
}

FFlowTracer::~FFlowTracer()
{
}

void FFlowTracer::Signal(FName NodeID, const FString& Data)
{
	FFlowEvent NewEvent;
	NewEvent.Timestamp = FPlatformTime::Seconds();
	NewEvent.FrameNumber = GFrameNumber;
	NewEvent.NodeID = NodeID;
	NewEvent.DataPayload = Data;
	NewEvent.Scope = GetCurrentScope();

	// Write into ring buffer under lock
	{
		FScopeLock Lock(&BufferLock);
		EventBuffer[BufferHead] = NewEvent;
		BufferHead = (BufferHead + 1) % BufferCapacity;
		if (EventCount < BufferCapacity)
		{
			++EventCount;
		}
	}

	// Broadcast OUTSIDE the lock to prevent deadlock if a listener calls back
	OnFlowEvent.Broadcast(NewEvent);
}

TArray<FFlowEvent> FFlowTracer::GetRecentEvents(int32 Count) const
{
	TArray<FFlowEvent> Result;

	FScopeLock Lock(&BufferLock);

	// Clamp to the number of events actually stored
	const int32 Available = FMath::Min(EventCount, BufferCapacity);
	const int32 NumToReturn = FMath::Clamp(Count, 0, Available);

	if (NumToReturn <= 0)
	{
		return Result;
	}

	Result.Reserve(NumToReturn);

	// The oldest of the requested events starts at this offset from BufferHead
	// BufferHead points to the *next* write slot, so the most recent event is at (BufferHead - 1).
	// We want the last NumToReturn events, oldest first.
	const int32 StartIndex = (BufferHead - NumToReturn + BufferCapacity) % BufferCapacity;

	for (int32 i = 0; i < NumToReturn; ++i)
	{
		const int32 Index = (StartIndex + i) % BufferCapacity;
		Result.Add(EventBuffer[Index]);
	}

	return Result;
}

void FFlowTracer::ClearBuffer()
{
	FScopeLock Lock(&BufferLock);

	for (int32 i = 0; i < BufferCapacity; ++i)
	{
		EventBuffer[i] = FFlowEvent();
	}
	BufferHead = 0;
	EventCount = 0;
}

int32 FFlowTracer::GetBufferUsed() const
{
	FScopeLock Lock(&BufferLock);
	return FMath::Min(EventCount, BufferCapacity);
}

int32 FFlowTracer::GetBufferCapacity() const
{
	FScopeLock Lock(&BufferLock);
	return BufferCapacity;
}

void FFlowTracer::PushScope(const FString& ScopeName)
{
	FScopeLock Lock(&BufferLock);
	ScopeStack.Add(ScopeName);
}

void FFlowTracer::PopScope(const FString& ScopeName)
{
	FScopeLock Lock(&BufferLock);
	if (ScopeStack.Num() > 0 && ScopeStack.Last() == ScopeName)
	{
		ScopeStack.Pop();
	}
}

FString FFlowTracer::GetCurrentScope() const
{
	FScopeLock Lock(&BufferLock);
	return FString::Join(ScopeStack, TEXT("/"));
}
