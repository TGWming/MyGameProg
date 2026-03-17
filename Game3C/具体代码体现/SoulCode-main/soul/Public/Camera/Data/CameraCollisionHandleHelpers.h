// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraCollisionHandleData.h"
#include "CameraCollisionHandleEnums.h"
#include "CameraCollisionHandleHelpers.generated.h"

/**
 * 相机碰撞处理辅助函数库
 * 提供数据解析、分类检查、枚举转换等静态函数
 */
UCLASS()
class SOUL_API UCameraCollisionHandleHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================
	// 数组解析函数
	// ========================================

	/** 解析输入参数字符串为数组 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static TArray<FString> GetInputParamsArray(const FCameraCollisionHandleRow& Row);

	/** 解析输出参数字符串为数组 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static TArray<FString> GetOutputParamsArray(const FCameraCollisionHandleRow& Row);

	// ========================================
	// 分类检查函数
	// ========================================

	/** 检查是否为检测类策略 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static bool IsDetectionStrategy(const FCameraCollisionHandleRow& Row);

	/** 检查是否为响应类策略 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static bool IsResponseStrategy(const FCameraCollisionHandleRow& Row);

	/** 检查是否为遮挡处理类策略 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static bool IsOcclusionStrategy(const FCameraCollisionHandleRow& Row);

	/** 检查是否为恢复类策略 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static bool IsRecoveryStrategy(const FCameraCollisionHandleRow& Row);

	/** 检查是否为特殊情况处理类策略 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static bool IsSpecialStrategy(const FCameraCollisionHandleRow& Row);

	// ========================================
	// 枚举转换函数
	// ========================================

	/** 字符串转分类枚举 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static ECollisionHandleCategory StringToCategory(const FString& CategoryString);

	/** 分类枚举转字符串 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static FString CategoryToString(ECollisionHandleCategory Category);

	/** 字符串转类型枚举 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static ECollisionHandleType StringToType(const FString& TypeString);

	/** 类型枚举转字符串 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static FString TypeToString(ECollisionHandleType Type);

	// ========================================
	// 中文显示名称
	// ========================================

	/** 获取分类中文显示名称 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static FString GetCategoryDisplayName(ECollisionHandleCategory Category);

	/** 获取类型中文显示名称 */
	UFUNCTION(BlueprintPure, Category = "Camera|CollisionHandle")
	static FString GetTypeDisplayName(ECollisionHandleType Type);
};
