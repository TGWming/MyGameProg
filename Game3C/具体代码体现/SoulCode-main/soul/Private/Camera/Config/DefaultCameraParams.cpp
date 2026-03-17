// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Config/DefaultCameraParams.h"

UDefaultCameraParams::UDefaultCameraParams()
{
    // Empty constructor - all defaults are defined in the static Get* methods
}

//========================================
// Complete Configuration Methods
// 完整配置方法
//========================================

FCameraStateConfig UDefaultCameraParams::GetDefaultConfig()
{
    FCameraStateConfig Config;
    
    // Apply base defaults for each parameter group
    Config.StateBase = CreateBaseStateParams();
    Config.Module = CreateBaseModuleParams();
    Config.Modifier = CreateBaseModifierParams();
    Config.Collision = CreateBaseCollisionParams();
    
    return Config;
}

FCameraStateConfig UDefaultCameraParams::GetDefaultConfigForCategory(ECameraStateCategory Category)
{
    FCameraStateConfig Config = GetDefaultConfig();
    
    // Apply category-specific overrides to StateBase
    ApplyCategoryOverrides(Config.StateBase, Category);
    
    return Config;
}

//========================================
// Individual Parameter Group Getters
// 各参数组获取器
//========================================

FStateBaseParams UDefaultCameraParams::GetDefaultStateBaseParams()
{
    return CreateBaseStateParams();
}

FStateBaseParams UDefaultCameraParams::GetDefaultStateBaseParamsForCategory(ECameraStateCategory Category)
{
    FStateBaseParams Params = CreateBaseStateParams();
    ApplyCategoryOverrides(Params, Category);
    return Params;
}

FModuleParams UDefaultCameraParams::GetDefaultModuleParams()
{
    return CreateBaseModuleParams();
}

FModifierParams UDefaultCameraParams::GetDefaultModifierParams()
{
    return CreateBaseModifierParams();
}

FCollisionParams UDefaultCameraParams::GetDefaultCollisionParams()
{
    return CreateBaseCollisionParams();
}

//========================================
// Internal Helper Methods
// 内部辅助方法
//========================================

void UDefaultCameraParams::ApplyCategoryOverrides(FStateBaseParams& OutParams, ECameraStateCategory Category)
{
    // Set identity category
    OutParams.Identity.Category = Category;
    
    switch (Category)
    {
        case ECameraStateCategory::FreeExploration:
        case ECameraStateCategory::Explore:
            OutParams.Distance.BaseDistance = 300.0f;
            OutParams.Distance.MinDistance = 150.0f;
            OutParams.Distance.MaxDistance = 600.0f;
            OutParams.FOV.BaseFOV = 90.0f;
            OutParams.FOV.MinFOV = 70.0f;
            OutParams.FOV.MaxFOV = 100.0f;
            OutParams.Rotation.MinPitch = -60.0f;
            OutParams.Rotation.MaxPitch = 70.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 80.0f);
            OutParams.Offset.SocketOffset = FVector(0.0f, 50.0f, 0.0f);
            OutParams.Offset.ShoulderOffset = 50.0f;
            OutParams.Lag.PositionLagSpeed = 6.0f;
            OutParams.Lag.RotationLagSpeed = 6.0f;
            OutParams.Lag.DistanceLagSpeed = 0.5f;
            OutParams.Lag.FOVLagSpeed = 6.0f;
            OutParams.Transition.BlendInTime = 0.3f;
            OutParams.Transition.BlendOutTime = 0.3f;
            OutParams.AutoCorrect.bEnableAutoCenter = true;
            OutParams.AutoCorrect.AutoCenterDelay = 3.0f;
            break;
            
        case ECameraStateCategory::Combat:
            OutParams.Distance.BaseDistance = 300.0f;
            OutParams.Distance.MinDistance = 200.0f;
            OutParams.Distance.MaxDistance = 500.0f;
            OutParams.FOV.BaseFOV = 90.0f;
            OutParams.FOV.MinFOV = 55.0f;
            OutParams.FOV.MaxFOV = 90.0f;
            OutParams.Rotation.MinPitch = -45.0f;
            OutParams.Rotation.MaxPitch = 50.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 90.0f);
            OutParams.Offset.SocketOffset = FVector(0.0f, 60.0f, 10.0f);
            OutParams.Offset.ShoulderOffset = 60.0f;
            OutParams.Lag.PositionLagSpeed = 12.0f;
            OutParams.Lag.RotationLagSpeed = 15.0f;
            OutParams.Lag.DistanceLagSpeed = 10.0f;
            OutParams.Lag.FOVLagSpeed = 8.0f;
            OutParams.Transition.BlendInTime = 0.25f;
            OutParams.Transition.BlendOutTime = 0.3f;
            OutParams.AutoCorrect.bEnableAutoCenter = false;
            break;
            
        case ECameraStateCategory::LockOn:
            OutParams.Distance.BaseDistance = 300.0f;
            OutParams.Distance.MinDistance = 200.0f;
            OutParams.Distance.MaxDistance = 300.0f;
            OutParams.FOV.BaseFOV = 90.0f;
            OutParams.FOV.MinFOV = 55.0f;
            OutParams.FOV.MaxFOV = 90.0f;
            OutParams.Rotation.MinPitch = -60.0f;
            OutParams.Rotation.MaxPitch = 60.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 90.0f);
            OutParams.Offset.SocketOffset = FVector(0.0f, 50.0f, 20.0f);
            OutParams.Offset.TargetOffset = FVector(0.0f, 0.0f, 50.0f);
            OutParams.Offset.ShoulderOffset = 50.0f;
            OutParams.Lag.PositionLagSpeed = 15.0f;
            OutParams.Lag.RotationLagSpeed = 20.0f;
            OutParams.Lag.DistanceLagSpeed = 12.0f;
            OutParams.Lag.FOVLagSpeed = 10.0f;
            OutParams.Transition.BlendInTime = 0.2f;
            OutParams.Transition.BlendOutTime = 0.3f;
            OutParams.AutoCorrect.bEnableAutoCenter = false;
            OutParams.Flags.bRequiresTarget = true;
            break;
            
        case ECameraStateCategory::Boss:
            OutParams.Distance.BaseDistance = 500.0f;
            OutParams.Distance.MinDistance = 350.0f;
            OutParams.Distance.MaxDistance = 800.0f;
            OutParams.FOV.BaseFOV = 75.0f;
            OutParams.FOV.MinFOV = 65.0f;
            OutParams.FOV.MaxFOV = 100.0f;
            OutParams.Rotation.MinPitch = -30.0f;
            OutParams.Rotation.MaxPitch = 45.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 120.0f);
            OutParams.Offset.SocketOffset = FVector(0.0f, 70.0f, 30.0f);
            OutParams.Offset.ShoulderOffset = 70.0f;
            OutParams.Lag.PositionLagSpeed = 12.0f;
            OutParams.Lag.RotationLagSpeed = 15.0f;
            OutParams.Lag.DistanceLagSpeed = 10.0f;
            OutParams.Lag.FOVLagSpeed = 8.0f;
            OutParams.Transition.BlendInTime = 0.5f;
            OutParams.Transition.BlendOutTime = 0.4f;
            OutParams.AutoCorrect.bEnableAutoCenter = false;
            OutParams.Flags.bRequiresTarget = true;
            break;
            
        case ECameraStateCategory::Mount:
            OutParams.Distance.BaseDistance = 600.0f;
            OutParams.Distance.MinDistance = 400.0f;
            OutParams.Distance.MaxDistance = 800.0f;
            OutParams.FOV.BaseFOV = 75.0f;
            OutParams.FOV.MinFOV = 65.0f;
            OutParams.FOV.MaxFOV = 95.0f;
            OutParams.Rotation.MinPitch = -50.0f;
            OutParams.Rotation.MaxPitch = 60.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 150.0f);
            OutParams.Offset.SocketOffset = FVector(0.0f, 60.0f, 20.0f);
            OutParams.Offset.VerticalOffset = 30.0f;
            OutParams.Offset.ShoulderOffset = 60.0f;
            OutParams.Lag.PositionLagSpeed = 8.0f;
            OutParams.Lag.RotationLagSpeed = 8.0f;
            OutParams.Lag.DistanceLagSpeed = 6.0f;
            OutParams.Lag.FOVLagSpeed = 5.0f;
            OutParams.Transition.BlendInTime = 0.5f;
            OutParams.Transition.BlendOutTime = 0.4f;
            OutParams.AutoCorrect.bEnableAutoCenter = true;
            OutParams.AutoCorrect.AutoCenterDelay = 2.0f;
            break;
            
        case ECameraStateCategory::Sprint:
            OutParams.Distance.BaseDistance = 450.0f;
            OutParams.Distance.MinDistance = 300.0f;
            OutParams.Distance.MaxDistance = 600.0f;
            OutParams.FOV.BaseFOV = 75.0f;
            OutParams.FOV.MinFOV = 70.0f;
            OutParams.FOV.MaxFOV = 90.0f;
            OutParams.Rotation.MinPitch = -50.0f;
            OutParams.Rotation.MaxPitch = 60.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 80.0f);
            OutParams.Offset.SocketOffset = FVector(0.0f, 50.0f, 10.0f);
            OutParams.Offset.ShoulderOffset = 50.0f;
            OutParams.Lag.PositionLagSpeed = 12.0f;
            OutParams.Lag.RotationLagSpeed = 12.0f;
            OutParams.Lag.DistanceLagSpeed = 10.0f;
            OutParams.Lag.FOVLagSpeed = 8.0f;
            OutParams.Transition.BlendInTime = 0.2f;
            OutParams.Transition.BlendOutTime = 0.25f;
            OutParams.AutoCorrect.bEnableAutoCenter = true;
            OutParams.AutoCorrect.AutoCenterDelay = 1.5f;
            break;
            
        case ECameraStateCategory::Cinematic:
            OutParams.Distance.BaseDistance = 300.0f;
            OutParams.Distance.MinDistance = 100.0f;
            OutParams.Distance.MaxDistance = 500.0f;
            OutParams.FOV.BaseFOV = 55.0f;
            OutParams.FOV.MinFOV = 35.0f;
            OutParams.FOV.MaxFOV = 75.0f;
            OutParams.Rotation.MinPitch = -89.0f;
            OutParams.Rotation.MaxPitch = 89.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 60.0f);
            OutParams.Offset.SocketOffset = FVector::ZeroVector;
            OutParams.Offset.ShoulderOffset = 0.0f;
            OutParams.Lag.PositionLagSpeed = 5.0f;
            OutParams.Lag.RotationLagSpeed = 5.0f;
            OutParams.Lag.DistanceLagSpeed = 4.0f;
            OutParams.Lag.FOVLagSpeed = 3.0f;
            OutParams.Transition.BlendInTime = 0.8f;
            OutParams.Transition.BlendOutTime = 0.6f;
            OutParams.Collision.bEnableCollision = false;
            OutParams.AutoCorrect.bEnableAutoCenter = false;
            OutParams.Flags.bIsCinematic = true;
            OutParams.Flags.bIgnoreInput = true;
            break;
            
        case ECameraStateCategory::Death:
            OutParams.Distance.BaseDistance = 500.0f;
            OutParams.Distance.MinDistance = 400.0f;
            OutParams.Distance.MaxDistance = 700.0f;
            OutParams.FOV.BaseFOV = 70.0f;
            OutParams.FOV.MinFOV = 60.0f;
            OutParams.FOV.MaxFOV = 80.0f;
            OutParams.Rotation.MinPitch = -30.0f;
            OutParams.Rotation.MaxPitch = 30.0f;
            OutParams.Offset.FocusOffset = FVector(0.0f, 0.0f, 40.0f);
            OutParams.Offset.SocketOffset = FVector::ZeroVector;
            OutParams.Offset.ShoulderOffset = 0.0f;
            OutParams.Lag.PositionLagSpeed = 3.0f;
            OutParams.Lag.RotationLagSpeed = 3.0f;
            OutParams.Lag.DistanceLagSpeed = 2.0f;
            OutParams.Lag.FOVLagSpeed = 2.0f;
            OutParams.Transition.BlendInTime = 1.0f;
            OutParams.Transition.BlendOutTime = 0.5f;
            OutParams.AutoCorrect.bEnableAutoCenter = false;
            OutParams.Flags.bIgnoreInput = true;
            break;
            
        default:
            OutParams.Distance.BaseDistance = 400.0f;
            OutParams.Distance.MinDistance = 150.0f;
            OutParams.Distance.MaxDistance = 800.0f;
            OutParams.FOV.BaseFOV = 70.0f;
            OutParams.FOV.MinFOV = 50.0f;
            OutParams.FOV.MaxFOV = 100.0f;
            break;
    }
}

FStateBaseParams UDefaultCameraParams::CreateBaseStateParams()
{
    FStateBaseParams Params;
    
    // A1: Identity 身份
    Params.Identity.StateName = NAME_None;
    Params.Identity.Priority = 100;
    Params.Identity.Category = ECameraStateCategory::FreeExploration;
    
    // A2: Distance 距离
    Params.Distance.BaseDistance = 400.0f;
    Params.Distance.MinDistance = 150.0f;
    Params.Distance.MaxDistance = 800.0f;
    
    // A3: FOV 视场角
    Params.FOV.BaseFOV = 70.0f;
    Params.FOV.MinFOV = 50.0f;
    Params.FOV.MaxFOV = 100.0f;
    
    // A4: Rotation 旋转
    Params.Rotation.MinPitch = -60.0f;
    Params.Rotation.MaxPitch = 60.0f;
    
    // A5: Offset 偏移
    Params.Offset.FocusOffset = FVector(0.0f, 0.0f, 60.0f);
    Params.Offset.SocketOffset = FVector::ZeroVector;
    Params.Offset.TargetOffset = FVector::ZeroVector;
    Params.Offset.VerticalOffset = 0.0f;
    Params.Offset.ShoulderOffset = 0.0f;
    Params.Offset.ForwardOffset = 0.0f;
    
    // A6: Lag 延迟
    Params.Lag.PositionLagSpeed = 10.0f;
    Params.Lag.RotationLagSpeed = 10.0f;
    Params.Lag.DistanceLagSpeed = 8.0f;
    Params.Lag.FOVLagSpeed = 6.0f;
    
    // A7: Transition 过渡
    Params.Transition.BlendInTime = 0.3f;
    Params.Transition.BlendOutTime = 0.3f;
    Params.Transition.BlendType = ECameraBlendType::SmoothStep;
    
    // A8: Collision 碰撞
    Params.Collision.bEnableCollision = true;
    Params.Collision.CollisionRadius = 12.0f;
    Params.Collision.RecoveryDelay = 0.5f;
    
    // A9: AutoCorrect 自动校正
    Params.AutoCorrect.bEnableAutoCenter = true;
    Params.AutoCorrect.AutoCenterDelay = 3.0f;
    
    // A10: Flags 标志
    Params.Flags.bRequiresTarget = false;
    Params.Flags.bIsCinematic = false;
    Params.Flags.bIgnoreInput = false;
    
    // A11: Hierarchy 层级
    Params.Hierarchy.ParentState = NAME_None;
    Params.Hierarchy.bOverrideParent = false;
    
    return Params;
}

FModuleParams UDefaultCameraParams::CreateBaseModuleParams()
{
    // Return default module params using constructor defaults
    // Module参数使用构造函数默认值
    return FModuleParams();
}

FModifierParams UDefaultCameraParams::CreateBaseModifierParams()
{
    // Return default modifier params using constructor defaults
    // Modifier参数使用构造函数默认值
    return FModifierParams();
}

FCollisionParams UDefaultCameraParams::CreateBaseCollisionParams()
{
    // Return default collision params using constructor defaults
    // Collision参数使用构造函数默认值
    return FCollisionParams();
}
