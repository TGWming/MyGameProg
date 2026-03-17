// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SoulsCameraDebugHUD.generated.h"

class USoulsCameraManager;

UCLASS()
class SOUL_API ASoulsCameraDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASoulsCameraDebugHUD();

	virtual void DrawHUD() override;

	/** 主开关 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing Debug")
	bool bShowFramingOverlay;

	/** 是否显示三分线 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing Debug")
	bool bShowThirdsGrid;

	/** 是否显示角色/敌人投影位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing Debug")
	bool bShowActorProjections;

	/** 是否显示 Anchor 目标位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing Debug")
	bool bShowAnchorTarget;

	/** 是否显示参数文字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing Debug")
	bool bShowFramingInfo;

protected:
	/** 绘制三分线网格 */
	void DrawThirdsGrid();

	/** 绘制玩家和敌人的屏幕投影位置 */
	void DrawActorProjections();

	/** 绘制 Anchor 目标位置（十字标记） */
	void DrawAnchorTarget();

	/** 绘制 Framing 参数信息文字 */
	void DrawFramingInfo();

	/** 绘制十字标记辅助函数 */
	void DrawCrosshair(FVector2D ScreenPos, FLinearColor Color, float Size, float Thickness);

	/** 绘制带边框的标签辅助函数 */
	void DrawLabel(FVector2D ScreenPos, const FString& Text, FLinearColor Color);

	/** 获取 CameraManager 引用 */
	USoulsCameraManager* GetCameraManager();

private:
	UPROPERTY()
	USoulsCameraManager* CachedCameraManager;
};
