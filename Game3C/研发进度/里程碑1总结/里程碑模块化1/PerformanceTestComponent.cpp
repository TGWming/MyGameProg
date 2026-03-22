#include "PerformanceTestComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

// 导入性能监控宏
#define SOUL_PERFORMANCE_SCOPE(FunctionName) \
	FSoulPerformanceScope PerformanceScope(FunctionName)

#define SOUL_PERFORMANCE_SCOPE_CONDITIONAL(FunctionName, Condition) \
	TOptional<FSoulPerformanceScope> PerformanceScope; \
	if (Condition) \
	{ \
		PerformanceScope.Emplace(FunctionName); \
	}

UPerformanceTestComponent::UPerformanceTestComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // 每秒执行一次
}

void UPerformanceTestComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 获取性能分析器实例
	ProfilerInstance = UPerformanceProfiler::GetPerformanceProfiler(this);
	
	if (ProfilerInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceTestComponent: Successfully obtained PerformanceProfiler instance"));
		
		// 运行初始测试
		TestBasicPerformanceMonitoring();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PerformanceTestComponent: Failed to obtain PerformanceProfiler instance"));
	}
}

void UPerformanceTestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	TestCounter++;
	
	// 每10秒运行一次测试
	if (TestCounter % 10 == 0)
	{
		TestPerformanceScopeMacros();
	}
	
	// 每30秒生成一次报告
	if (TestCounter % 30 == 0)
	{
		TestPerformanceReporting();
	}
}

void UPerformanceTestComponent::TestBasicPerformanceMonitoring()
{
	if (!ProfilerInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceTestComponent: No ProfilerInstance available for basic test"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== Testing Basic Performance Monitoring ==="));

	// 测试手动记录性能数据
	ProfilerInstance->RecordFunctionTime(TEXT("TestFunction_Fast"), 0.5f);
	ProfilerInstance->RecordFunctionTime(TEXT("TestFunction_Medium"), 5.0f);
	ProfilerInstance->RecordFunctionTime(TEXT("TestFunction_Slow"), 15.0f);

	// 记录多次相同函数以测试统计功能
	for (int32 i = 0; i < 5; i++)
	{
		float RandomTime = FMath::RandRange(1.0f, 10.0f);
		ProfilerInstance->RecordFunctionTime(TEXT("TestFunction_Variable"), RandomTime);
	}

	UE_LOG(LogTemp, Warning, TEXT("Basic performance monitoring test completed"));
}

void UPerformanceTestComponent::TestPerformanceScopeMacros()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Testing Performance Scope Macros ==="));

	// 测试基本作用域宏
	{
		SOUL_PERFORMANCE_SCOPE(TEXT("ScopeTest_Basic"));
		SimulateFastFunction();
	}

	// 测试条件作用域宏
	bool bShouldProfile = true;
	{
		SOUL_PERFORMANCE_SCOPE_CONDITIONAL(TEXT("ScopeTest_Conditional"), bShouldProfile);
		SimulateMediumFunction();
	}

	// 测试条件为false的情况
	bShouldProfile = false;
	{
		SOUL_PERFORMANCE_SCOPE_CONDITIONAL(TEXT("ScopeTest_Conditional_False"), bShouldProfile);
		SimulateSlowFunction(); // 这个不应该被记录
	}

	UE_LOG(LogTemp, Warning, TEXT("Performance scope macros test completed"));
}

void UPerformanceTestComponent::TestPerformanceReporting()
{
	if (!ProfilerInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformanceTestComponent: No ProfilerInstance available for reporting test"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== Testing Performance Reporting ==="));

	// 获取性能报告
	TArray<FPerformanceData> Report = ProfilerInstance->GetPerformanceReport();
	UE_LOG(LogTemp, Warning, TEXT("Retrieved performance report with %d entries"), Report.Num());

	// 测试特定函数查询
	FPerformanceData TestData = ProfilerInstance->GetFunctionPerformanceData(TEXT("TestFunction_Variable"));
	if (TestData.CallCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestFunction_Variable: Avg=%.3fms, Max=%.3fms, Calls=%d"), 
			TestData.AverageTime, TestData.MaxTime, TestData.CallCount);
	}

	// 打印完整报告
	ProfilerInstance->PrintPerformanceReport();
	
	UE_LOG(LogTemp, Warning, TEXT("Performance reporting test completed"));
}

void UPerformanceTestComponent::RunFullPerformanceTestSuite()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Running Full Performance Test Suite ==="));

	// 重置性能数据
	if (ProfilerInstance)
	{
		ProfilerInstance->ResetPerformanceData();
	}

	// 运行所有测试
	TestBasicPerformanceMonitoring();
	TestPerformanceScopeMacros();
	TestPerformanceReporting();

	UE_LOG(LogTemp, Warning, TEXT("=== Full Performance Test Suite Completed ==="));
}

void UPerformanceTestComponent::SimulateFastFunction()
{
	SOUL_PERFORMANCE_SCOPE(TEXT("SimulateFastFunction"));
	
	// 模拟快速计算
	float Result = 0.0f;
	for (int32 i = 0; i < 100; i++)
	{
		Result += FMath::Sin(i * 0.1f);
	}
	
	// 确保编译器不优化掉这个计算
	volatile float VolatileResult = Result;
}

void UPerformanceTestComponent::SimulateSlowFunction()
{
	SOUL_PERFORMANCE_SCOPE(TEXT("SimulateSlowFunction"));
	
	// 模拟慢速计算
	float Result = 0.0f;
	for (int32 i = 0; i < 10000; i++)
	{
		Result += FMath::Sqrt(i) * FMath::Cos(i * 0.01f);
	}
	
	// 确保编译器不优化掉这个计算
	volatile float VolatileResult = Result;
}

void UPerformanceTestComponent::SimulateMediumFunction()
{
	SOUL_PERFORMANCE_SCOPE(TEXT("SimulateMediumFunction"));
	
	// 模拟中等速度计算
	float Result = 0.0f;
	for (int32 i = 1; i < 1000; i++) // 从1开始避免log(0)
	{
		Result += FMath::Pow(i, 0.5f) + FMath::Loge(i); // 使用FMath::Loge而不是Log
	}
	
	// 确保编译器不优化掉这个计算
	volatile float VolatileResult = Result;
}