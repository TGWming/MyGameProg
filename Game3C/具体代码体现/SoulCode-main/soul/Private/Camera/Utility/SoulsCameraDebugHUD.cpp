// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Utility/SoulsCameraDebugHUD.h"
#include "Camera/Core/SoulsCameraManager.h"
#include "Camera/Core/SoulsCameraRuntimeTypes.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

ASoulsCameraDebugHUD::ASoulsCameraDebugHUD()
	: bShowFramingOverlay(true)
	, bShowThirdsGrid(true)
	, bShowActorProjections(true)
	, bShowAnchorTarget(true)
	, bShowFramingInfo(true)
	, CachedCameraManager(nullptr)
{
}

USoulsCameraManager* ASoulsCameraDebugHUD::GetCameraManager()
{
	if (CachedCameraManager)
	{
		return CachedCameraManager;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return nullptr;
	}

	CachedCameraManager = PC->GetPawn()->FindComponentByClass<USoulsCameraManager>();
	return CachedCameraManager;
}

void ASoulsCameraDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bShowFramingOverlay || !Canvas)
	{
		return;
	}

	if (bShowThirdsGrid)       DrawThirdsGrid();
	if (bShowActorProjections) DrawActorProjections();
	if (bShowAnchorTarget)     DrawAnchorTarget();
	if (bShowFramingInfo)      DrawFramingInfo();
}

// ============================================================
// 三分线网格
// ============================================================
void ASoulsCameraDebugHUD::DrawThirdsGrid()
{
	float W = Canvas->SizeX;
	float H = Canvas->SizeY;
	FLinearColor GridColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.25f); // 白色半透明

	// 垂直三分线（1/3 和 2/3）
	float X1 = W / 3.0f;
	float X2 = W * 2.0f / 3.0f;
	DrawLine(X1, 0, X1, H, GridColor);
	DrawLine(X2, 0, X2, H, GridColor);

	// 水平三分线（1/3 和 2/3）
	float Y1 = H / 3.0f;
	float Y2 = H * 2.0f / 3.0f;
	DrawLine(0, Y1, W, Y1, GridColor);
	DrawLine(0, Y2, W, Y2, GridColor);

	// 中心点（小十字）
	float CenterX = W * 0.5f;
	float CenterY = H * 0.5f;
	FLinearColor CenterColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.4f);
	DrawLine(CenterX - 10, CenterY, CenterX + 10, CenterY, CenterColor);
	DrawLine(CenterX, CenterY - 10, CenterX, CenterY + 10, CenterColor);
}

// ============================================================
// 玩家和敌人的屏幕投影位置
// ============================================================
void ASoulsCameraDebugHUD::DrawActorProjections()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}

	float W = Canvas->SizeX;
	float H = Canvas->SizeY;

	// === 玩家投影 ===
	ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
	if (PlayerChar)
	{
		FVector2D PlayerScreen;
		if (PC->ProjectWorldLocationToScreen(PlayerChar->GetActorLocation(), PlayerScreen, true))
		{
			// 绿色十字 = 玩家
			DrawCrosshair(PlayerScreen, FLinearColor::Green, 12.0f, 2.0f);
			DrawLabel(PlayerScreen + FVector2D(15, -8),
				FString::Printf(TEXT("P (%.0f%%, %.0f%%)"),
					PlayerScreen.X / W * 100.0f,
					PlayerScreen.Y / H * 100.0f),
				FLinearColor::Green);
		}
	}

	// === 敌人投影 ===
	USoulsCameraManager* CamMgr = GetCameraManager();
	if (CamMgr)
	{
		AActor* Target = CamMgr->GetLockOnTarget();
		if (Target)
		{
			FVector2D TargetScreen;
			if (PC->ProjectWorldLocationToScreen(Target->GetActorLocation(), TargetScreen, true))
			{
				// 红色十字 = 敌人
				DrawCrosshair(TargetScreen, FLinearColor::Red, 12.0f, 2.0f);
				DrawLabel(TargetScreen + FVector2D(15, -8),
					FString::Printf(TEXT("E (%.0f%%, %.0f%%)"),
						TargetScreen.X / W * 100.0f,
						TargetScreen.Y / H * 100.0f),
					FLinearColor::Red);
			}
		}
	}

	// === Focus Point 投影 ===
	if (CamMgr)
	{
		FSoulsCameraOutput Output = CamMgr->GetCurrentOutput();
		FVector2D FocusScreen;
		if (PC->ProjectWorldLocationToScreen(Output.FocusPoint, FocusScreen, true))
		{
			// 黄色十字 = Focus Point
			DrawCrosshair(FocusScreen, FLinearColor::Yellow, 8.0f, 1.5f);
			DrawLabel(FocusScreen + FVector2D(15, -8),
				FString::Printf(TEXT("F (%.0f%%, %.0f%%)"),
					FocusScreen.X / W * 100.0f,
					FocusScreen.Y / H * 100.0f),
				FLinearColor::Yellow);
		}
	}
}

// ============================================================
// Anchor 目标位置（构图目标点）
// ============================================================
void ASoulsCameraDebugHUD::DrawAnchorTarget()
{
	float W = Canvas->SizeX;
	float H = Canvas->SizeY;

	// Anchor 位置（从测试参数读取，后续从配置读取）
	// 目前用临时值：AnchorX=0.5, AnchorY=0.2
	float AnchorX = 0.5f;
	float AnchorY = 0.2f;

	// [TODO] 后续从 CameraManager 获取实际的 FramingParams
	// USoulsCameraManager* CamMgr = GetCameraManager();
	// if (CamMgr) { AnchorX = CamMgr->GetCurrentFramingParams().FramingAnchorX; ... }

	FVector2D AnchorScreen(W * AnchorX, H * AnchorY);

	// 青色大十字 = Anchor 目标位置
	DrawCrosshair(AnchorScreen, FLinearColor(0.0f, 1.0f, 1.0f), 20.0f, 2.5f);

	// 在 Anchor 周围画一个小矩形框（构图容差区域）
	float BoxHalfW = W * 0.05f;  // 屏幕 5% 宽度
	float BoxHalfH = H * 0.05f;  // 屏幕 5% 高度
	FLinearColor BoxColor = FLinearColor(0.0f, 1.0f, 1.0f, 0.4f);
	DrawLine(AnchorScreen.X - BoxHalfW, AnchorScreen.Y - BoxHalfH, AnchorScreen.X + BoxHalfW, AnchorScreen.Y - BoxHalfH, BoxColor); // 上
	DrawLine(AnchorScreen.X + BoxHalfW, AnchorScreen.Y - BoxHalfH, AnchorScreen.X + BoxHalfW, AnchorScreen.Y + BoxHalfH, BoxColor); // 右
	DrawLine(AnchorScreen.X + BoxHalfW, AnchorScreen.Y + BoxHalfH, AnchorScreen.X - BoxHalfW, AnchorScreen.Y + BoxHalfH, BoxColor); // 下
	DrawLine(AnchorScreen.X - BoxHalfW, AnchorScreen.Y + BoxHalfH, AnchorScreen.X - BoxHalfW, AnchorScreen.Y - BoxHalfH, BoxColor); // 左

	DrawLabel(AnchorScreen + FVector2D(25, -8),
		FString::Printf(TEXT("Anchor (%.0f%%, %.0f%%)"), AnchorX * 100.0f, AnchorY * 100.0f),
		FLinearColor(0.0f, 1.0f, 1.0f));
}

// ============================================================
// Framing 参数信息文字
// ============================================================
void ASoulsCameraDebugHUD::DrawFramingInfo()
{
	float W = Canvas->SizeX;
	float H = Canvas->SizeY;

	// 左下角显示参数信息
	float StartX = 20.0f;
	float StartY = H - 140.0f;
	float LineHeight = 18.0f;
	FLinearColor TextColor = FLinearColor(0.8f, 0.8f, 0.8f, 0.9f);

	// [TEMP] 临时参数值，后续从 CameraManager 获取
	FString Lines[] = {
		TEXT("=== FRAMING DEBUG ==="),
		TEXT("AnchorX: 0.50  AnchorY: 0.20"),
		TEXT("PlayerW: 0.20  EnemyW: 0.80"),
		TEXT("VertBias: 0.30"),
		TEXT("Status: ACTIVE (TEMP)")
	};

	for (int32 i = 0; i < 5; i++)
	{
		// 简单的文字绘制（使用默认字体）
		Canvas->SetDrawColor(FColor(200, 200, 200, 230));

		FFontRenderInfo FontInfo;
		FontInfo.bClipText = true;

		Canvas->DrawText(
			GEngine->GetSmallFont(),
			Lines[i],
			StartX,
			StartY + i * LineHeight,
			1.2f, 1.2f,
			FontInfo
		);
	}
}

// ============================================================
// 辅助函数：十字标记
// ============================================================
void ASoulsCameraDebugHUD::DrawCrosshair(FVector2D ScreenPos, FLinearColor Color, float Size, float Thickness)
{
	// 水平线
	DrawLine(ScreenPos.X - Size, ScreenPos.Y, ScreenPos.X + Size, ScreenPos.Y, Color);
	// 垂直线
	DrawLine(ScreenPos.X, ScreenPos.Y - Size, ScreenPos.X, ScreenPos.Y + Size, Color);

	// 如果需要更粗的线，画多条偏移线
	if (Thickness > 1.0f)
	{
		for (float Offset = 1.0f; Offset < Thickness; Offset += 1.0f)
		{
			DrawLine(ScreenPos.X - Size, ScreenPos.Y + Offset, ScreenPos.X + Size, ScreenPos.Y + Offset, Color);
			DrawLine(ScreenPos.X + Offset, ScreenPos.Y - Size, ScreenPos.X + Offset, ScreenPos.Y + Size, Color);
		}
	}
}

// ============================================================
// 辅助函数：带标签的文字
// ============================================================
void ASoulsCameraDebugHUD::DrawLabel(FVector2D ScreenPos, const FString& Text, FLinearColor Color)
{
	if (!Canvas || !GEngine)
	{
		return;
	}

	Canvas->SetDrawColor(Color.ToFColor(true));

	FFontRenderInfo FontInfo;
	FontInfo.bClipText = true;

	Canvas->DrawText(
		GEngine->GetTinyFont(),
		Text,
		ScreenPos.X,
		ScreenPos.Y,
		1.0f, 1.0f,
		FontInfo
	);
}
