// Copyright FlowVisualizer Plugin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlowPipelineTypes.h"
#include "HAL/CriticalSection.h"

/** Delegate broadcast each time a new FFlowEvent is recorded. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlowEvent, const FFlowEvent&);

/**
 * FFlowTracer — Singleton runtime event collector.
 *
 * Game code calls FFlowTracer::Get().Signal() to emit events.
 * Editor panels bind to OnFlowEvent to visualize them in real-time.
 *
 * Internally uses a TArray-based ring buffer protected by FCriticalSection.
 */
class FLOWVISUALIZERRUNTIME_API FFlowTracer
{
public:
	/**
	 * Returns the singleton instance (Meyer's Singleton).
	 */
	static FFlowTracer& Get();

	/**
	 * Record a new flow event.
	 *
	 * @param NodeID   Identifier of the node that triggered the event.
	 * @param Data     Optional free-form payload string.
	 *
	 * The event is written into the ring buffer under lock.
	 * OnFlowEvent is broadcast *outside* the lock to avoid deadlocks.
	 */
	void Signal(FName NodeID, const FString& Data = TEXT(""));

	/** 进入一个 Scope 层级 */
	void PushScope(const FString& ScopeName);

	/** 退出当前最内层 Scope */
	void PopScope(const FString& ScopeName);

	/** 获取当前完整 Scope 路径，如 "BP_Player/CombatLogic" */
	FString GetCurrentScope() const;

	/**
	 * Retrieve the most recent events in chronological order (oldest first).
	 *
	 * @param Count  Maximum number of events to return.
	 * @return Array of events ordered from oldest to newest.
	 */
	TArray<FFlowEvent> GetRecentEvents(int32 Count) const;

	/**
	 * Clear the entire event buffer and reset the write head.
	 */
	void ClearBuffer();

	/**
	 * Returns the number of events currently stored in the ring buffer.
	 * Thread-safe (acquires BufferLock).
	 */
	int32 GetBufferUsed() const;

	/**
	 * Returns the maximum capacity of the ring buffer.
	 * Thread-safe (acquires BufferLock).
	 */
	int32 GetBufferCapacity() const;

	/** Delegate broadcast after each Signal() call (outside the buffer lock). */
	FOnFlowEvent OnFlowEvent;

private:
	FFlowTracer();
	~FFlowTracer();

	// Non-copyable, non-movable
	FFlowTracer(const FFlowTracer&) = delete;
	FFlowTracer& operator=(const FFlowTracer&) = delete;
	FFlowTracer(FFlowTracer&&) = delete;
	FFlowTracer& operator=(FFlowTracer&&) = delete;

	/** Ring buffer storage. Always kept at BufferCapacity elements. */
	TArray<FFlowEvent> EventBuffer;

	/** Current write position in the ring buffer. */
	int32 BufferHead;

	/** Maximum number of events stored in the ring buffer. */
	int32 BufferCapacity;

	/** Total number of events ever written (used to determine fill level). */
	int32 EventCount;

	/** Thread-safety lock for buffer reads and writes. */
	mutable FCriticalSection BufferLock;

	/** 当前活跃的 Scope 栈，如 ["BP_Player", "CombatLogic"] */
	TArray<FString> ScopeStack;
};
