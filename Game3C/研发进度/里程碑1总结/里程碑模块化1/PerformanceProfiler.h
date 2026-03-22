#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceProfiler.generated.h"

/**
 * 性能数据结构体
 * 存储函数性能统计信息
 */
USTRUCT(BlueprintType)
struct SOUL_API FPerformanceData
{
	GENERATED_BODY()

	/** 函数名称 */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	FString FunctionName;

	/** 平均执行时间（毫秒） */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float AverageTime = 0.0f;

	/** 最大执行时间（毫秒） */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float MaxTime = 0.0f;

	/** 调用次数 */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 CallCount = 0;

	/** 总执行时间（毫秒） */
	float TotalTime = 0.0f;

	/** 最小执行时间（毫秒） */
	float MinTime = 0.0f;

	FPerformanceData()
	{
		FunctionName = TEXT("");
		AverageTime = 0.0f;
		MaxTime = 0.0f;
		CallCount = 0;
		TotalTime = 0.0f;
		MinTime = FLT_MAX;
	}

	FPerformanceData(const FString& InFunctionName)
	{
		FunctionName = InFunctionName;
		AverageTime = 0.0f;
		MaxTime = 0.0f;
		CallCount = 0;
		TotalTime = 0.0f;
		MinTime = FLT_MAX;
	}
};

/**
 * 性能分析器子系统
 * 用于收集和分析函数执行性能
 */
UCLASS(BlueprintType)
class SOUL_API UPerformanceProfiler : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 记录函数执行时间
	 * @param FunctionName 函数名称
	 * @param ElapsedTime 执行时间（毫秒）
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	void RecordFunctionTime(const FString& FunctionName, float ElapsedTime);

	/**
	 * 获取性能报告
	 * @return 所有函数的性能数据数组
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	TArray<FPerformanceData> GetPerformanceReport();

	/**
	 * 重置性能数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	void ResetPerformanceData();

	/**
	 * 打印性能报告到日志
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	void PrintPerformanceReport();

	/**
	 * 获取特定函数的性能数据
	 * @param FunctionName 函数名称
	 * @return 函数的性能数据，如果不存在则返回空数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	FPerformanceData GetFunctionPerformanceData(const FString& FunctionName);

	/**
	 * 检查是否启用性能监控
	 * @return 如果启用返回true
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	bool IsPerformanceMonitoringEnabled() const;

	/**
	 * 启用或禁用性能监控
	 * @param bEnabled 是否启用
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	void SetPerformanceMonitoringEnabled(bool bEnabled);

	/**
	 * 获取性能分析器实例
	 * @param WorldContext 世界上下文对象
	 * @return 性能分析器实例，如果不存在则返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler", meta = (WorldContext = "WorldContextObject"))
	static UPerformanceProfiler* GetPerformanceProfiler(const UObject* WorldContextObject);

private:
	/** 性能数据映射表 */
	UPROPERTY()
	TMap<FString, FPerformanceData> PerformanceMap;

	/** 是否启用性能监控 */
	UPROPERTY()
	bool bIsPerformanceMonitoringEnabled = true;

	/** 更新性能数据统计信息 */
	void UpdatePerformanceStatistics(FPerformanceData& Data, float ElapsedTime);

	/** 格式化时间为可读字符串 */
	FString FormatTime(float TimeInMs) const;
};

/**
 * 性能作用域结构体
 * 用于自动测量函数执行时间
 */
struct SOUL_API FSoulPerformanceScope
{
public:
	FSoulPerformanceScope(const FString& InFunctionName);
	~FSoulPerformanceScope();

private:
	FString FunctionName;
	double StartTime;
};

// 性能监控宏定义
#define SOUL_PERFORMANCE_SCOPE(FunctionName) \
	FSoulPerformanceScope PerformanceScope(FunctionName)

#define SOUL_PERFORMANCE_SCOPE_CONDITIONAL(FunctionName, Condition) \
	TOptional<FSoulPerformanceScope> PerformanceScope; \
	if (Condition) \
	{ \
		PerformanceScope.Emplace(FunctionName); \
	}