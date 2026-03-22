#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceProfiler.h"
#include "PerformanceTestComponent.generated.h"

/**
 * 性能测试组件
 * 用于测试和演示性能分析器功能
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UPerformanceTestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerformanceTestComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 测试基本性能监控 */
	UFUNCTION(BlueprintCallable, Category = "Performance Test")
	void TestBasicPerformanceMonitoring();

	/** 测试性能作用域宏 */
	UFUNCTION(BlueprintCallable, Category = "Performance Test")
	void TestPerformanceScopeMacros();

	/** 测试性能报告生成 */
	UFUNCTION(BlueprintCallable, Category = "Performance Test")
	void TestPerformanceReporting();

	/** 运行完整的性能测试套件 */
	UFUNCTION(BlueprintCallable, Category = "Performance Test")
	void RunFullPerformanceTestSuite();

private:
	/** 模拟快速函数 */
	void SimulateFastFunction();

	/** 模拟慢速函数 */
	void SimulateSlowFunction();

	/** 模拟中等速度函数 */
	void SimulateMediumFunction();

	/** 性能分析器引用 */
	UPROPERTY()
	UPerformanceProfiler* ProfilerInstance = nullptr;

	/** 测试计数器 */
	int32 TestCounter = 0;
};