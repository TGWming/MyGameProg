// CameraModule_Framing.h
// Framing Anchor 相机模块 - 构图驱动的 Focus Point 解算

#pragma once

#include "CoreMinimal.h"
#include "Camera/Modules/CameraModuleBase.h"
#include "Camera/Config/SoulsCameraParams_Framing.h"
#include "CameraModule_Framing.generated.h"

/**
 * R10 Framing Anchor Module
 * 
 * 职责：根据 2D 屏幕构图意图，解算 3D Focus Point
 * 
 * 三层架构中的位置：
 * - 接收 Layer 1 构图参数（FFramingParams）
 * - 执行 Layer 2 Focus 解算
 * - 输出 Rotation 供 Layer 3 执行
 * 
 * 优先级：300（高于 R03/R04/R05 LookAt 模块）
 * 激活条件：状态配置 bEnableFraming = true 且有有效目标
 */
UCLASS(Blueprintable, BlueprintType)
class SOUL_API UCameraModule_R10_Framing : public UCameraModuleBase
{
    GENERATED_BODY()

public:
    UCameraModule_R10_Framing();

    //~ Begin UCameraModuleBase Interface
    virtual ECameraModuleType GetModuleType() const override { return ECameraModuleType::Module_R10_Framing; }
    virtual EModuleCategory GetModuleCategory() const override { return EModuleCategory::Rotation; }
    virtual FName GetModuleName() const override { return TEXT("R10_Framing"); }
    virtual FString GetModuleDescription() const override { return TEXT("Framing Anchor - 构图驱动的 Focus Point 解算"); }
    virtual int32 GetDefaultPriority() const override { return 300; }
    virtual EBlendPolicy GetDefaultBlendPolicy() const override { return EBlendPolicy::Override; }
    virtual bool ShouldActivate(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig) const override;
    virtual bool Compute(const FStageExecutionContext& Context, const FCameraStateConfig& StateConfig, FModuleOutput& OutOutput) override;
    //~ End UCameraModuleBase Interface

protected:
    /**
     * 计算 Focus Point
     * 将 2D 构图意图解算为 3D 世界空间点
     * 
     * @param PlayerLocation 玩家位置
     * @param TargetLocation 目标位置
     * @param FramingParams 构图参数
     * @return 解算后的 Focus Point
     */
    FVector CalculateFocusPoint(
        const FVector& PlayerLocation,
        const FVector& TargetLocation,
        const FFramingParams& FramingParams
    ) const;

    /**
     * 计算相机看向 Focus Point 的旋转
     * 
     * @param CameraLocation 相机位置
     * @param FocusPoint 焦点位置
     * @return 相机旋转
     */
    FRotator CalculateLookAtRotation(
        const FVector& CameraLocation,
        const FVector& FocusPoint
    ) const;
};
