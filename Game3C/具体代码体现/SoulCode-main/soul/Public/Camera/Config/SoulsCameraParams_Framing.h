// SoulsCameraParams_Framing.h
// Framing Anchor 系统参数定义

#pragma once

#include "CoreMinimal.h"
#include "SoulsCameraParams_Framing.generated.h"

/**
 * Framing 参数结构体
 * 定义 2D 屏幕空间的构图意图
 */
USTRUCT(BlueprintType)
struct SOUL_API FFramingParams
{
    GENERATED_BODY()

    /** 是否启用 Framing Module */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing")
    bool bEnableFraming = false;

    /** 屏幕锚点 X (0=左, 0.5=中, 1=右) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FramingAnchorX = 0.5f;

    /** 屏幕锚点 Y (0=下, 0.5=中, 1=上) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FramingAnchorY = 0.5f;

    /** 玩家权重 (0-1)，影响 Focus Point 计算 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing|Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PlayerWeight = 0.5f;

    /** 敌人权重 (0-1)，影响 Focus Point 计算 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing|Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EnemyWeight = 0.5f;

    /** 仰视倾向 (正值=仰视, 负值=俯视)，影响 Focus Point 高度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing|Bias", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float VerticalBias = 0.0f;

    /** 构造函数 */
    FFramingParams()
        : bEnableFraming(false)
        , FramingAnchorX(0.5f)
        , FramingAnchorY(0.5f)
        , PlayerWeight(0.5f)
        , EnemyWeight(0.5f)
        , VerticalBias(0.0f)
    {
    }
};
