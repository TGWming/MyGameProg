// CameraModule_Framing.cpp
// Framing Anchor 相机模块实现

#include "Camera/Modules/CameraModule_Framing.h"
#include "Camera/Config/SoulsCameraParams_Framing.h"

UCameraModule_R10_Framing::UCameraModule_R10_Framing()
{
    // 模块默认配置
}

bool UCameraModule_R10_Framing::ShouldActivate(
    const FStageExecutionContext& Context, 
    const FCameraStateConfig& StateConfig) const
{
    // 条件 1：必须启用 Framing
    if (!StateConfig.FramingParams.bEnableFraming)
    {
        return false;
    }

    // 条件 2：必须有有效目标
    if (!Context.InputContext.bHasTarget)
    {
        return false;
    }

    // 条件 3：目标位置必须有效
    if (Context.InputContext.TargetLocation.IsZero())
    {
        return false;
    }

    return true;
}

bool UCameraModule_R10_Framing::Compute(
    const FStageExecutionContext& Context, 
    const FCameraStateConfig& StateConfig, 
    FModuleOutput& OutOutput)
{
    // 创建基础输出
    OutOutput = CreateBaseOutput();

    // 获取构图参数
    const FFramingParams& FramingParams = StateConfig.FramingParams;

    // 获取输入数据
    const FVector PlayerLocation = Context.InputContext.CharacterLocation;
    const FVector TargetLocation = Context.InputContext.TargetLocation;

    // Step 1: 计算 Focus Point（Layer 2 解算）
    FVector FocusPoint = CalculateFocusPoint(PlayerLocation, TargetLocation, FramingParams);

    // Step 2: 获取相机位置（使用前一帧相机位置作为当前相机位置估算）
    FVector CameraLocation = Context.InputContext.PreviousCameraLocation;
    // 如果前一帧位置无效（例如游戏刚开始），fallback 到玩家位置 + 高度偏移
    if (CameraLocation.IsNearlyZero())
    {
        CameraLocation = Context.InputContext.CharacterLocation;
        CameraLocation.Z += 100.0f; // 相机默认高度偏移
    }

    // Step 3: 计算看向 Focus Point 的旋转
    FRotator LookAtRotation = CalculateLookAtRotation(CameraLocation, FocusPoint);

    // 输出结果
    // [Fix] R10 不再输出旋转，避免以 Priority=300 覆盖 R03_LookAt_Target 的锁定旋转
    // OutOutput.RotationOutput = LookAtRotation;
    OutOutput.bHasRotationOutput = false;

    // 输出 Focus Point（供调试和其他模块使用）
    OutOutput.FocusPoint = FocusPoint;
    OutOutput.bHasFocusPoint = true;

    return true;
}

FVector UCameraModule_R10_Framing::CalculateFocusPoint(
    const FVector& PlayerLocation,
    const FVector& TargetLocation,
    const FFramingParams& FramingParams
) const
{
    // === Step 1: 归一化权重 ===
    float TotalWeight = FramingParams.PlayerWeight + FramingParams.EnemyWeight;
    if (TotalWeight <= KINDA_SMALL_NUMBER)
    {
        TotalWeight = 1.0f;
    }

    float NormalizedPlayerWeight = FramingParams.PlayerWeight / TotalWeight;
    float NormalizedEnemyWeight = FramingParams.EnemyWeight / TotalWeight;

    // === Step 2: 加权中心 ===
    FVector WeightedCenter = PlayerLocation * NormalizedPlayerWeight + TargetLocation * NormalizedEnemyWeight;

    // === Step 3: Framing Anchor 约束 ===
    // Anchor 定义了"Focus Point 应该出现在屏幕的什么位置"
    // (0.5, 0.5) = 屏幕中心 → 无偏移
    // (0.5, 0.3) = 屏幕偏下 → Focus Point 上移 → 相机仰视
    // (0.7, 0.5) = 屏幕偏右 → Focus Point 左移 → 画面右侧留白
    
    // 计算 Anchor 偏离中心的程度（-1 到 1）
    float AnchorOffsetX = (FramingParams.FramingAnchorX - 0.5f) * 2.0f;
    float AnchorOffsetY = (FramingParams.FramingAnchorY - 0.5f) * 2.0f;

    // 只有当 Anchor 偏离中心时才需要计算偏移
    if (!FMath::IsNearlyZero(AnchorOffsetX) || !FMath::IsNearlyZero(AnchorOffsetY))
    {
        // 计算从玩家到目标的方向作为"前向"参考
        FVector ForwardDir = (TargetLocation - PlayerLocation).GetSafeNormal();
        
        // 如果玩家和目标重叠，使用角色前方作为参考
        if (ForwardDir.IsNearlyZero())
        {
            ForwardDir = FVector::ForwardVector;
        }

        // 计算右向量和上向量
        FVector WorldUp = FVector::UpVector;
        FVector RightDir = FVector::CrossProduct(ForwardDir, WorldUp).GetSafeNormal();
        FVector UpDir = FVector::CrossProduct(RightDir, ForwardDir).GetSafeNormal();

        // 偏移量基于玩家和目标的距离
        float Distance = FVector::Dist(PlayerLocation, TargetLocation);
        float OffsetScale = FMath::Clamp(Distance * 0.3f, 50.0f, 500.0f);

        // 应用偏移（反向：锚点偏右 → Focus Point 偏左，锚点偏下 → Focus Point 偏上）
        WeightedCenter -= RightDir * AnchorOffsetX * OffsetScale;
        WeightedCenter -= UpDir * AnchorOffsetY * OffsetScale;
    }

    // === Step 4: Vertical Bias（仰视倾向）===
    // 正值 → Focus Point 升高 → 相机仰视
    // 负值 → Focus Point 降低 → 相机俯视
    float VerticalOffset = FramingParams.VerticalBias * 200.0f;
    WeightedCenter.Z += VerticalOffset;

    return WeightedCenter;
}

FRotator UCameraModule_R10_Framing::CalculateLookAtRotation(
    const FVector& CameraLocation,
    const FVector& FocusPoint
) const
{
    // 计算从相机到 Focus Point 的方向
    FVector LookDirection = (FocusPoint - CameraLocation).GetSafeNormal();

    // 转换为旋转
    if (LookDirection.IsNearlyZero())
    {
        return FRotator::ZeroRotator;
    }

    return LookDirection.Rotation();
}
