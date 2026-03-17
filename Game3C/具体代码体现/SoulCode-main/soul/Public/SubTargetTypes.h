// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SubTargetTypes.generated.h"

// ==================== 子锁点身体部位标签枚举 ====================

/**
 * ESubTargetTag
 * 
 * 多锁点锁定系统的身体部位标签。
 * 用于标识子锁点所代表的身体区域或特殊部位。
 */
UENUM(BlueprintType)
enum class ESubTargetTag : uint8
{
	None       UMETA(DisplayName = "None"),
	Head       UMETA(DisplayName = "Head"),
	Chest      UMETA(DisplayName = "Chest"),
	Torso      UMETA(DisplayName = "Torso"),
	LeftArm    UMETA(DisplayName = "LeftArm"),
	RightArm   UMETA(DisplayName = "RightArm"),
	LeftLeg    UMETA(DisplayName = "LeftLeg"),
	RightLeg   UMETA(DisplayName = "RightLeg"),
	Tail       UMETA(DisplayName = "Tail"),
	Wing       UMETA(DisplayName = "Wing"),
	Core       UMETA(DisplayName = "Core"),
	WeakPoint  UMETA(DisplayName = "WeakPoint"),
	Custom     UMETA(DisplayName = "Custom")
};

// ==================== 子锁点定义结构体 ====================

/**
 * FSubTargetDefinition
 * 
 * 描述单个子锁点的完整信息，包括挂载 Socket、部位标签、
 * 优先级、启用状态以及锁定区域的包围体参数。
 * 这是多锁点锁定系统的基础数据单元。
 */
USTRUCT(BlueprintType)
struct SOUL_API FSubTargetDefinition
{
	GENERATED_BODY()

	// ==================== 构造函数 ====================

	/** 默认构造函数，使用初始化列表设置所有字段默认值 */
	FSubTargetDefinition()
		: SocketName(NAME_None)
		, Tag(ESubTargetTag::Chest)
		, Priority(0)
		, bIsDefault(false)
		, bIsEnabled(true)
		, BoundExtent(30.0f, 30.0f, 30.0f)
		, Offset(FVector::ZeroVector)
	{
	}

	// ==================== 基本属性 ====================

	/** 挂载的 Socket 名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget")
	FName SocketName;

	/** 身体部位标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget")
	ESubTargetTag Tag;

	/** 优先级，Fallback 时使用（值越高越优先） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget", meta = (ClampMin = "0", ClampMax = "100"))
	int32 Priority;

	/** 是否为默认锁定点 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget")
	bool bIsDefault;

	/** 运行时启用状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget")
	bool bIsEnabled;

	// ==================== 包围体参数 ====================

	/** 锁定区域包围体半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget|Bounds")
	FVector BoundExtent;

	/** 相对 Socket 的额外偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubTarget|Bounds")
	FVector Offset;

	// ==================== 辅助函数 ====================

	/** 判断此子锁点定义是否有效（SocketName 不为空） */
	FORCEINLINE bool IsValid() const
	{
		return SocketName != NAME_None;
	}

	/** 判断此子锁点是否可用（有效且已启用） */
	FORCEINLINE bool IsAvailable() const
	{
		return IsValid() && bIsEnabled;
	}
};
