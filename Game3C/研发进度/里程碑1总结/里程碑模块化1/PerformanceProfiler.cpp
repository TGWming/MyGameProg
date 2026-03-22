#include "PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

// 前向声明
class USoulDebugSettings;

// UPerformanceProfiler实现

void UPerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("PerformanceProfiler: Subsystem initialized"));
	
	// 暂时硬编码启用性能监控，避免循环依赖
	bIsPerformanceMonitoringEnabled = true;
	
	// 初始化性能数据映射表
	PerformanceMap.Reset();
	
	UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: Performance monitoring %s"), 
		bIsPerformanceMonitoringEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UPerformanceProfiler::Deinitialize()
{
	if (bIsPerformanceMonitoringEnabled && PerformanceMap.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: Shutting down with %d recorded functions"), PerformanceMap.Num());
		PrintPerformanceReport();
	}
	
	PerformanceMap.Reset();
	Super::Deinitialize();
}

void UPerformanceProfiler::RecordFunctionTime(const FString& FunctionName, float ElapsedTime)
{
	if (!bIsPerformanceMonitoringEnabled)
	{
		return;
	}

	if (FunctionName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: Cannot record performance for empty function name"));
		return;
	}

	// 查找或创建性能数据
	if (!PerformanceMap.Contains(FunctionName))
	{
		PerformanceMap.Add(FunctionName, FPerformanceData(FunctionName));
	}

	FPerformanceData& Data = PerformanceMap[FunctionName];
	UpdatePerformanceStatistics(Data, ElapsedTime);

	// 详细调试日志
	UE_LOG(LogTemp, VeryVerbose, TEXT("PerformanceProfiler: Recorded %s - Time: %.3fms (Avg: %.3fms, Max: %.3fms, Calls: %d)"), 
		*FunctionName, ElapsedTime, Data.AverageTime, Data.MaxTime, Data.CallCount);
}

TArray<FPerformanceData> UPerformanceProfiler::GetPerformanceReport()
{
	TArray<FPerformanceData> Report;
	
	for (const auto& Pair : PerformanceMap)
	{
		Report.Add(Pair.Value);
	}
	
	// 按平均执行时间降序排序
	Report.Sort([](const FPerformanceData& A, const FPerformanceData& B)
	{
		return A.AverageTime > B.AverageTime;
	});
	
	return Report;
}

void UPerformanceProfiler::ResetPerformanceData()
{
	int32 PreviousCount = PerformanceMap.Num();
	PerformanceMap.Reset();
	
	UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: Reset performance data (%d functions cleared)"), PreviousCount);
}

void UPerformanceProfiler::PrintPerformanceReport()
{
	if (PerformanceMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: No performance data to report"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT(""));
	UE_LOG(LogTemp, Warning, TEXT("=== SOUL PERFORMANCE REPORT ==="));
	UE_LOG(LogTemp, Warning, TEXT("Generated at: %s"), *FDateTime::Now().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Total functions tracked: %d"), PerformanceMap.Num());
	UE_LOG(LogTemp, Warning, TEXT(""));

	// 获取排序后的报告
	TArray<FPerformanceData> SortedReport = GetPerformanceReport();

	// 打印表头
	UE_LOG(LogTemp, Warning, TEXT("%-40s | %10s | %10s | %10s | %8s"), 
		TEXT("Function Name"), TEXT("Avg (ms)"), TEXT("Max (ms)"), TEXT("Min (ms)"), TEXT("Calls"));
	UE_LOG(LogTemp, Warning, TEXT("%-40s-|-%10s-|-%10s-|-%10s-|-%8s"), 
		TEXT("----------------------------------------"), 
		TEXT("----------"), TEXT("----------"), TEXT("----------"), TEXT("--------"));

	// 打印每个函数的性能数据
	for (const FPerformanceData& Data : SortedReport)
	{
		FString FunctionDisplayName = Data.FunctionName;
		if (FunctionDisplayName.Len() > 40)
		{
			FunctionDisplayName = FunctionDisplayName.Left(37) + TEXT("...");
		}

		// 计算最小时间显示值
		float MinTimeDisplay = (Data.MinTime == FLT_MAX) ? 0.0f : Data.MinTime;

		UE_LOG(LogTemp, Warning, TEXT("%-40s | %10s | %10s | %10s | %8d"), 
			*FunctionDisplayName,
			*FormatTime(Data.AverageTime),
			*FormatTime(Data.MaxTime),
			*FormatTime(MinTimeDisplay),
			Data.CallCount);
	}

	UE_LOG(LogTemp, Warning, TEXT(""));

	// 统计信息
	float TotalTime = 0.0f;
	int32 TotalCalls = 0;
	float MaxAvgTime = 0.0f;
	FString SlowestFunction;

	for (const FPerformanceData& Data : SortedReport)
	{
		TotalTime += Data.TotalTime;
		TotalCalls += Data.CallCount;
		
		if (Data.AverageTime > MaxAvgTime)
		{
			MaxAvgTime = Data.AverageTime;
			SlowestFunction = Data.FunctionName;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SUMMARY:"));
	UE_LOG(LogTemp, Warning, TEXT("- Total execution time: %s"), *FormatTime(TotalTime));
	UE_LOG(LogTemp, Warning, TEXT("- Total function calls: %d"), TotalCalls);
	UE_LOG(LogTemp, Warning, TEXT("- Slowest function: %s (avg: %s)"), *SlowestFunction, *FormatTime(MaxAvgTime));
	UE_LOG(LogTemp, Warning, TEXT("=============================="));
	UE_LOG(LogTemp, Warning, TEXT(""));
}

FPerformanceData UPerformanceProfiler::GetFunctionPerformanceData(const FString& FunctionName)
{
	if (PerformanceMap.Contains(FunctionName))
	{
		return PerformanceMap[FunctionName];
	}
	
	// 返回空的性能数据
	return FPerformanceData();
}

bool UPerformanceProfiler::IsPerformanceMonitoringEnabled() const
{
	return bIsPerformanceMonitoringEnabled;
}

void UPerformanceProfiler::SetPerformanceMonitoringEnabled(bool bEnabled)
{
	bool bPreviousState = bIsPerformanceMonitoringEnabled;
	bIsPerformanceMonitoringEnabled = bEnabled;
	
	UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: Performance monitoring %s -> %s"), 
		bPreviousState ? TEXT("ENABLED") : TEXT("DISABLED"),
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
	
	if (!bEnabled && PerformanceMap.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("PerformanceProfiler: Performance monitoring disabled, current data preserved"));
	}
}

UPerformanceProfiler* UPerformanceProfiler::GetPerformanceProfiler(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: Invalid WorldContextObject"));
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: Could not get World from context object"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceProfiler: No GameInstance found"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPerformanceProfiler>();
}

void UPerformanceProfiler::UpdatePerformanceStatistics(FPerformanceData& Data, float ElapsedTime)
{
	// 更新调用次数
	Data.CallCount++;
	
	// 更新总时间
	Data.TotalTime += ElapsedTime;
	
	// 更新平均时间
	Data.AverageTime = Data.TotalTime / static_cast<float>(Data.CallCount);
	
	// 更新最大时间
	if (ElapsedTime > Data.MaxTime)
	{
		Data.MaxTime = ElapsedTime;
	}
	
	// 更新最小时间
	if (ElapsedTime < Data.MinTime)
	{
		Data.MinTime = ElapsedTime;
	}
}

FString UPerformanceProfiler::FormatTime(float TimeInMs) const
{
	if (TimeInMs < 0.001f)
	{
		return FString::Printf(TEXT("%.3f?s"), TimeInMs * 1000.0f);
	}
	else if (TimeInMs < 1.0f)
	{
		return FString::Printf(TEXT("%.3fms"), TimeInMs);
	}
	else if (TimeInMs < 1000.0f)
	{
		return FString::Printf(TEXT("%.2fms"), TimeInMs);
	}
	else
	{
		return FString::Printf(TEXT("%.2fs"), TimeInMs / 1000.0f);
	}
}

// FSoulPerformanceScope实现

FSoulPerformanceScope::FSoulPerformanceScope(const FString& InFunctionName)
	: FunctionName(InFunctionName)
	, StartTime(0.0)
{
	// 记录开始时间（以秒为单位）
	StartTime = FPlatformTime::Seconds();
}

FSoulPerformanceScope::~FSoulPerformanceScope()
{
	// 计算执行时间（转换为毫秒）
	double EndTime = FPlatformTime::Seconds();
	float ElapsedTimeMs = static_cast<float>((EndTime - StartTime) * 1000.0);
	
	// 获取性能分析器实例并记录时间
	if (GEngine && GEngine->GetCurrentPlayWorld())
	{
		UPerformanceProfiler* Profiler = UPerformanceProfiler::GetPerformanceProfiler(GEngine->GetCurrentPlayWorld());
		if (Profiler)
		{
			Profiler->RecordFunctionTime(FunctionName, ElapsedTimeMs);
		}
	}
}