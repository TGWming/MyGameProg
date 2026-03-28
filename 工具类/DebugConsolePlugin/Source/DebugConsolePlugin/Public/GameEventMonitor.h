// GameEventMonitor.h
#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"

class AActor;
class UWorld;
class ULevel;
class UObject;

/**
 * 游戏事件监控器
 * 监控引擎各类事件并输出到调试控制台
 */
class DEBUGCONSOLEPLUGIN_API FGameEventMonitor
{
public:
	/** 获取单例实例 */
	static FGameEventMonitor& Get();

	/** 初始化监控器，注册所有委托 */
	void Initialize();

	/** 关闭监控器，注销所有委托 */
	void Shutdown();

	/** 检查是否已初始化 */
	bool IsInitialized() const { return bIsInitialized; }

private:
	FGameEventMonitor() = default;
	~FGameEventMonitor() = default;

	// 禁止拷贝
	FGameEventMonitor(const FGameEventMonitor&) = delete;
	FGameEventMonitor& operator=(const FGameEventMonitor&) = delete;

	// ========== 资源加载事件处理器 ==========
	void OnAsyncLoadPackage(const FString& PackageName);
	void OnSyncLoadPackage(const FString& PackageName);
	void OnAssetLoaded(UObject* Asset);
	void OnPakFileMounted(const class IPakFile& PakFile);

	// ========== Actor/对象事件处理器 ==========
	void OnActorSpawned(AActor* Actor);
	void OnActorDestroyed(AActor* Actor);
	void OnObjectPropertyChanged(UObject* Object, struct FPropertyChangedEvent& PropertyChangedEvent);

	// ========== 世界/关卡事件处理器 ==========
	void OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	void OnLevelAddedToWorld(ULevel* Level, UWorld* World);
	void OnLevelRemovedFromWorld(ULevel* Level, UWorld* World);

	// ========== 游戏流程事件处理器 ==========
	void OnGameModeInitialized(class AGameModeBase* GameMode);
	void OnGameModePostLogin(class AGameModeBase* GameMode, class APlayerController* NewPlayer);
	void OnGameStateClassChanged(const FString& StateName);

	// ========== 编辑器事件处理器 ==========
#if WITH_EDITOR
	void OnEditorSelectionChanged(UObject* NewSelection);
	void OnBeginPIE(bool bIsSimulating);
	void OnEndPIE(bool bIsSimulating);
#endif

	// ========== 系统事件处理器 ==========
	void OnPostGarbageCollect();
	void OnMemoryTrim();
	void OnBeginFrame();

	// ========== 辅助方法 ==========
	void PrintMonitorMessage(WORD Color, const FString& Category, const FString& Message);
	FString GetTimeStamp() const;
	FString GetActorInfoString(AActor* Actor) const;

	// 委托句柄
	FDelegateHandle AsyncLoadPackageHandle;
	FDelegateHandle SyncLoadPackageHandle;
	FDelegateHandle AssetLoadedHandle;
	FDelegateHandle PakFileMountedHandle;
	FDelegateHandle ActorSpawnedHandle;
	FDelegateHandle ActorDestroyedHandle;
	FDelegateHandle ObjectPropertyChangedHandle;
	FDelegateHandle PostWorldInitializationHandle;
	FDelegateHandle WorldCleanupHandle;
	FDelegateHandle LevelAddedToWorldHandle;
	FDelegateHandle LevelRemovedFromWorldHandle;
	FDelegateHandle GameModeInitializedHandle;
	FDelegateHandle GameModePostLoginHandle;
	FDelegateHandle GameStateClassChangedHandle;
	FDelegateHandle PostGarbageCollectHandle;
	FDelegateHandle MemoryTrimHandle;
	FDelegateHandle BeginFrameHandle;

#if WITH_EDITOR
	FDelegateHandle EditorSelectionChangedHandle;
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
#endif

	bool bIsInitialized = false;

	// 帧计数器（用于BeginFrame事件）
	uint64 FrameCounter = 0;
};
