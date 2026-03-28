// GameEventMonitor.cpp
#include "GameEventMonitor.h"
#include "DebugConsoleSettings.h"
#include "DebugConsoleManager.h"
#include "DebugConsoleColors.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectIterator.h"
#include "HAL/Platform.h"
#include "Misc/CoreDelegates.h"
#include "UObject/UObjectGlobals.h"
#include "IPlatformFilePak.h"

#if WITH_EDITOR
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#endif

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

FGameEventMonitor& FGameEventMonitor::Get()
{
	static FGameEventMonitor Instance;
	return Instance;
}

void FGameEventMonitor::Initialize()
{
#if WITH_DEBUG_CONSOLE && !UE_BUILD_SHIPPING
	if (bIsInitialized)
	{
		return;
	}

	const UDebugConsoleSettings* Settings = UDebugConsoleSettings::Get();
	if (!Settings || !Settings->bEnableDebugConsole)
	{
		return;
	}

	// ========== 注册资源加载事件 ==========
	if (Settings->bMonitorAssetLoading)
	{
		AsyncLoadPackageHandle = FCoreDelegates::OnAsyncLoadPackage.AddRaw(this, &FGameEventMonitor::OnAsyncLoadPackage);
		SyncLoadPackageHandle = FCoreDelegates::OnSyncLoadPackage.AddRaw(this, &FGameEventMonitor::OnSyncLoadPackage);
		
		// Pak文件挂载事件 (UE4.27使用OnPakFileMounted2)
		if (FCoreDelegates::OnPakFileMounted2.IsBound() || true)
		{
			PakFileMountedHandle = FCoreDelegates::OnPakFileMounted2.AddRaw(this, &FGameEventMonitor::OnPakFileMounted);
		}
	}

	// ========== 注册Actor/对象事件 ==========
	if (Settings->bMonitorActorSpawnDestroy)
	{
		// Actor生成事件需要在World创建后绑定，这里先绑定世界事件
		PostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FGameEventMonitor::OnPostWorldInitialization);
	}

	// ========== 注册世界/关卡事件 ==========
	if (Settings->bMonitorWorldEvents)
	{
		if (!PostWorldInitializationHandle.IsValid())
		{
			PostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FGameEventMonitor::OnPostWorldInitialization);
		}
		WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(this, &FGameEventMonitor::OnWorldCleanup);
		LevelAddedToWorldHandle = FWorldDelegates::LevelAddedToWorld.AddRaw(this, &FGameEventMonitor::OnLevelAddedToWorld);
		LevelRemovedFromWorldHandle = FWorldDelegates::LevelRemovedFromWorld.AddRaw(this, &FGameEventMonitor::OnLevelRemovedFromWorld);
	}

	// ========== 注册游戏流程事件 ==========
	if (Settings->bMonitorGameFlowEvents)
	{
		// GameMode事件通过FGameModeEvents访问
		// 注意：这些委托可能在某些UE版本中不存在，需要检查
		GameStateClassChangedHandle = FCoreDelegates::GameStateClassChanged.AddRaw(this, &FGameEventMonitor::OnGameStateClassChanged);
	}

	// ========== 注册编辑器事件 ==========
#if WITH_EDITOR
	if (Settings->bMonitorEditorSelection)
	{
		if (GEditor)
		{
			BeginPIEHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FGameEventMonitor::OnBeginPIE);
			EndPIEHandle = FEditorDelegates::EndPIE.AddRaw(this, &FGameEventMonitor::OnEndPIE);
		}
	}
#endif

	// ========== 注册系统事件 ==========
	PostGarbageCollectHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddRaw(this, &FGameEventMonitor::OnPostGarbageCollect);
	MemoryTrimHandle = FCoreDelegates::GetMemoryTrimDelegate().AddRaw(this, &FGameEventMonitor::OnMemoryTrim);
	BeginFrameHandle = FCoreDelegates::OnBeginFrame.AddRaw(this, &FGameEventMonitor::OnBeginFrame);

	bIsInitialized = true;

	PrintMonitorMessage(DebugConsoleColors::Highlight, TEXT("MONITOR"), TEXT("Game Event Monitor Initialized"));
#endif
}

void FGameEventMonitor::Shutdown()
{
#if WITH_DEBUG_CONSOLE && !UE_BUILD_SHIPPING
	if (!bIsInitialized)
	{
		return;
	}

	// 注销所有委托
	FCoreDelegates::OnAsyncLoadPackage.Remove(AsyncLoadPackageHandle);
	FCoreDelegates::OnSyncLoadPackage.Remove(SyncLoadPackageHandle);
	FCoreDelegates::OnPakFileMounted2.Remove(PakFileMountedHandle);
	FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationHandle);
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
	FWorldDelegates::LevelAddedToWorld.Remove(LevelAddedToWorldHandle);
	FWorldDelegates::LevelRemovedFromWorld.Remove(LevelRemovedFromWorldHandle);
	FCoreDelegates::GameStateClassChanged.Remove(GameStateClassChangedHandle);
	FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGarbageCollectHandle);
	FCoreDelegates::GetMemoryTrimDelegate().Remove(MemoryTrimHandle);
	FCoreDelegates::OnBeginFrame.Remove(BeginFrameHandle);

#if WITH_EDITOR
	FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);
#endif

	bIsInitialized = false;
#endif
}

// ========== 资源加载事件处理器实现 ==========

void FGameEventMonitor::OnAsyncLoadPackage(const FString& PackageName)
{
	PrintMonitorMessage(DebugConsoleColors::Loading, TEXT("ASYNC_LOAD"), FString::Printf(TEXT("Package: %s"), *PackageName));
}

void FGameEventMonitor::OnSyncLoadPackage(const FString& PackageName)
{
	PrintMonitorMessage(DebugConsoleColors::Loading, TEXT("SYNC_LOAD"), FString::Printf(TEXT("Package: %s"), *PackageName));
}

void FGameEventMonitor::OnPakFileMounted(const IPakFile& PakFile)
{
	FString PakFilename = PakFile.PakGetPakFilename();
	PrintMonitorMessage(DebugConsoleColors::Loading, TEXT("PAK_MOUNT"), 
		FString::Printf(TEXT("Pak: %s"), *PakFilename));
}

// ========== Actor/对象事件处理器实现 ==========

void FGameEventMonitor::OnActorSpawned(AActor* Actor)
{
	if (Actor)
	{
		FString ActorInfo = GetActorInfoString(Actor);
		PrintMonitorMessage(DebugConsoleColors::Spawn, TEXT("SPAWN"), ActorInfo);
	}
}

void FGameEventMonitor::OnActorDestroyed(AActor* Actor)
{
	if (Actor)
	{
		FString ActorName = Actor->GetName();
		uint32 UniqueID = Actor->GetUniqueID();
		PrintMonitorMessage(DebugConsoleColors::Destroy, TEXT("DESTROY"), 
			FString::Printf(TEXT("%s | ID: %u"), *ActorName, UniqueID));
	}
}

// ========== 世界/关卡事件处理器实现 ==========

void FGameEventMonitor::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (World)
	{
		FString MapName = World->GetMapName();
		PrintMonitorMessage(DebugConsoleColors::World, TEXT("WORLD_INIT"), 
			FString::Printf(TEXT("Map: %s"), *MapName));

		// 注册Actor生成事件
		const UDebugConsoleSettings* Settings = UDebugConsoleSettings::Get();
		if (Settings && Settings->bMonitorActorSpawnDestroy)
		{
			ActorSpawnedHandle = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &FGameEventMonitor::OnActorSpawned));
		}
	}
}

void FGameEventMonitor::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (World)
	{
		FString MapName = World->GetMapName();
		PrintMonitorMessage(DebugConsoleColors::World, TEXT("WORLD_CLEANUP"), 
			FString::Printf(TEXT("Map: %s | SessionEnded: %s"), *MapName, bSessionEnded ? TEXT("Yes") : TEXT("No")));

		// 注销Actor生成事件
		if (ActorSpawnedHandle.IsValid())
		{
			World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
			ActorSpawnedHandle.Reset();
		}
	}
}

void FGameEventMonitor::OnLevelAddedToWorld(ULevel* Level, UWorld* World)
{
	if (Level)
	{
		FString LevelName = Level->GetOuter() ? Level->GetOuter()->GetName() : TEXT("Unknown");
		PrintMonitorMessage(DebugConsoleColors::World, TEXT("LEVEL_LOADED"), 
			FString::Printf(TEXT("Level: %s"), *LevelName));
	}
}

void FGameEventMonitor::OnLevelRemovedFromWorld(ULevel* Level, UWorld* World)
{
	if (Level)
	{
		FString LevelName = Level->GetOuter() ? Level->GetOuter()->GetName() : TEXT("Unknown");
		PrintMonitorMessage(DebugConsoleColors::World, TEXT("LEVEL_UNLOADED"), 
			FString::Printf(TEXT("Level: %s"), *LevelName));
	}
}

// ========== 游戏流程事件处理器实现 ==========

void FGameEventMonitor::OnGameModeInitialized(AGameModeBase* GameMode)
{
	if (GameMode)
	{
		FString GameModeClass = GameMode->GetClass()->GetName();
		PrintMonitorMessage(DebugConsoleColors::GameFlow, TEXT("GAMEMODE_INIT"), 
			FString::Printf(TEXT("GameMode: %s"), *GameModeClass));
	}
}

void FGameEventMonitor::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (NewPlayer)
	{
		FString PlayerName = NewPlayer->GetName();
		PrintMonitorMessage(DebugConsoleColors::GameFlow, TEXT("PLAYER_LOGIN"), 
			FString::Printf(TEXT("PlayerController: %s"), *PlayerName));
	}
}

void FGameEventMonitor::OnGameStateClassChanged(const FString& StateName)
{
	PrintMonitorMessage(DebugConsoleColors::GameFlow, TEXT("GAMESTATE_CHANGE"), 
		FString::Printf(TEXT("State: %s"), *StateName));
}

// ========== 编辑器事件处理器实现 ==========

#if WITH_EDITOR
void FGameEventMonitor::OnEditorSelectionChanged(UObject* NewSelection)
{
	if (NewSelection)
	{
		AActor* SelectedActor = Cast<AActor>(NewSelection);
		if (SelectedActor)
		{
			FString ActorInfo = GetActorInfoString(SelectedActor);
			PrintMonitorMessage(DebugConsoleColors::Editor, TEXT("SELECT"), ActorInfo);
		}
	}
}

void FGameEventMonitor::OnBeginPIE(bool bIsSimulating)
{
	PrintMonitorMessage(DebugConsoleColors::Editor, TEXT("PIE_BEGIN"), 
		bIsSimulating ? TEXT("Simulating") : TEXT("Playing"));
}

void FGameEventMonitor::OnEndPIE(bool bIsSimulating)
{
	PrintMonitorMessage(DebugConsoleColors::Editor, TEXT("PIE_END"), 
		bIsSimulating ? TEXT("Simulating") : TEXT("Playing"));
}
#endif

// ========== 系统事件处理器实现 ==========

void FGameEventMonitor::OnPostGarbageCollect()
{
	// 可以记录GC耗时，但需要在PreGC时记录开始时间
	PrintMonitorMessage(DebugConsoleColors::System, TEXT("GC"), TEXT("Garbage Collection Completed"));
}

void FGameEventMonitor::OnMemoryTrim()
{
	PrintMonitorMessage(DebugConsoleColors::System, TEXT("MEMORY_TRIM"), TEXT("Memory Trim Requested"));
}

void FGameEventMonitor::OnBeginFrame()
{
	// 帧事件会非常频繁，默认不输出每一帧，可以根据需要修改
	// 例如每100帧输出一次
	FrameCounter++;
	if (FrameCounter % 100 == 0)
	{
		// PrintMonitorMessage(DebugConsoleColors::System, TEXT("FRAME"), 
		// 	FString::Printf(TEXT("Frame: %llu"), FrameCounter));
	}
}

// ========== 辅助方法实现 ==========

void FGameEventMonitor::PrintMonitorMessage(WORD Color, const FString& Category, const FString& Message)
{
#if WITH_DEBUG_CONSOLE && PLATFORM_WINDOWS
	if (!FDebugConsoleManager::Get().IsInitialized())
	{
		return;
	}

	// 设置颜色
	HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole != INVALID_HANDLE_VALUE)
	{
		::SetConsoleTextAttribute(hConsole, Color);
	}

	// 输出消息
	FString TimeStamp = GetTimeStamp();
	FString FullMessage = FString::Printf(TEXT("[%s] [%s] %s"), *TimeStamp, *Category, *Message);
	FDebugConsoleManager::Get().PrintToConsole(FullMessage);

	// 恢复默认颜色
	if (hConsole != INVALID_HANDLE_VALUE)
	{
		::SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}
#endif
}

FString FGameEventMonitor::GetTimeStamp() const
{
	FDateTime Now = FDateTime::Now();
	return FString::Printf(TEXT("%02d:%02d:%02d.%03d"), 
		Now.GetHour(), Now.GetMinute(), Now.GetSecond(), Now.GetMillisecond());
}

FString FGameEventMonitor::GetActorInfoString(AActor* Actor) const
{
	if (!Actor)
	{
		return TEXT("Invalid Actor");
	}

	FString ActorName = Actor->GetName();
	FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : TEXT("Unknown");
	uint32 UniqueID = Actor->GetUniqueID();
	FVector Location = Actor->GetActorLocation();

	return FString::Printf(TEXT("%s | Class: %s | ID: %u | Loc: (%.1f, %.1f, %.1f)"), 
		*ActorName, *ClassName, UniqueID, Location.X, Location.Y, Location.Z);
}

