// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraCollisionHandleData.generated.h"

/**
 * 相机碰撞处理策略数据行结构体
 * 用于从CSV导入相机碰撞处理策略配置数据
 * 继承自FTableRowBase以支持数据表导入
 */
USTRUCT(BlueprintType)
struct FCameraCollisionHandleRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 策略唯一标识符，格式如"CompleteCollisionHandle_D01" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FString StrategyID;

	/** 策略名称，作为程序引用标识符（如 SingleRay, PullIn） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName StrategyName;

	/** 中文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FString ChineseName;

	/** 主分类（Detection, Response, Occlusion, Recovery, Special） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Category;

	/** 类型（Trace, Sweep, Position, FOV, Material, Visibility, Timing, Interpolation, Environment） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	FString Type;

	/** 策略功能描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	FString Description;

	/** 优先级（数值越大优先级越高，范围0-200） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	int32 Priority;

	/** 是否为核心策略 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	bool bIsCore;

	/** 输入参数列表，用斜杠分隔（如 "FocusPoint/DesiredPos/ProbeRadius"） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString InputParams;

	/** 输出参数列表，用斜杠分隔（如 "bHit/HitPoint/HitNormal"） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString OutputParams;

	/** 触发条件描述（如 "检测到碰撞时", "遮挡物支持透明化"） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	FString Condition;

	// 构造函数
	FCameraCollisionHandleRow()
		: Priority(100)
		, bIsCore(false)
	{
	}
};
