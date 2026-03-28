#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PerformanceStatsFunctionLibrary.generated.h"

UCLASS()
class PERFORMANCESTATSPLUGIN_API UPerformanceStatsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** 开启性能监控窗口 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats", meta = (Keywords = "performance monitor enable open"))
	static void EnablePerformanceMonitor();
	
	/** 关闭性能监控窗口 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats", meta = (Keywords = "performance monitor disable close"))
	static void DisablePerformanceMonitor();
	
	/** 切换性能监控窗口开关 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats", meta = (Keywords = "performance monitor toggle"))
	static void TogglePerformanceMonitor();
	
	/** 检查性能监控是否启用 */
	UFUNCTION(BlueprintPure, Category = "Performance Stats")
	static bool IsPerformanceMonitorEnabled();
	
	/** 暂停数据刷新 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats", meta = (Keywords = "performance pause freeze"))
	static void PausePerformanceMonitor();
	
	/** 恢复数据刷新 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats", meta = (Keywords = "performance resume continue"))
	static void ResumePerformanceMonitor();
	
	/** 切换暂停状态 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats")
	static void TogglePausePerformanceMonitor();
	
	/** 检查是否已暂停 */
	UFUNCTION(BlueprintPure, Category = "Performance Stats")
	static bool IsPerformanceMonitorPaused();
};
