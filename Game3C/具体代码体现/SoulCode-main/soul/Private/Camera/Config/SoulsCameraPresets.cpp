// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Config/SoulsCameraPresets.h"

//========================================
// State Config Presets - 状态配置预设
//========================================

FCameraStateConfig USoulsCameraPresets::GetExplorationConfig()
{
    FCameraStateConfig Config;
    
    // Identity - 状态标识
    Config.StateBase.Identity.StateName = FName("Explore_Default");
    Config.StateBase.Identity.Category = ECameraStateCategory::FreeExploration;
    Config.StateBase.Identity.Priority = 100;

    // Distance - 标准距离
    Config.StateBase.Distance.BaseDistance = 400.0f;
    Config.StateBase.Distance.MinDistance = 150.0f;
    Config.StateBase.Distance.MaxDistance = 600.0f;

    // FOV - 标准视野
    Config.StateBase.FOV.BaseFOV = 90.0f;
    Config.StateBase.FOV.MinFOV = 70.0f;
    Config.StateBase.FOV.MaxFOV = 110.0f;

    // Rotation - 俯仰限制
    Config.StateBase.Rotation.MinPitch = -60.0f;
    Config.StateBase.Rotation.MaxPitch = 70.0f;

    // Offset - 肩部偏移
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 80.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 50.0f, 0.0f);
    Config.StateBase.Offset.ShoulderOffset = 50.0f;

    // Lag - 跟随延迟
    Config.StateBase.Lag.PositionLagSpeed = 8.0f;
    Config.StateBase.Lag.RotationLagSpeed = 15.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 5.0f;
    Config.StateBase.Lag.FOVLagSpeed = 6.0f;

    // Transition - 过渡
    Config.StateBase.Transition.BlendInTime = 0.3f;
    Config.StateBase.Transition.BlendOutTime = 0.3f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::SmoothStep;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = true;
    Config.StateBase.Collision.CollisionRadius = 12.0f;
    Config.StateBase.Collision.RecoveryDelay = 0.5f;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = true;
    Config.StateBase.AutoCorrect.AutoCenterDelay = 3.0f;

    // Collision - 默认碰撞配置
    Config.Collision = GetDefaultCollisionConfig();

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetCombatConfig()
{
    FCameraStateConfig Config;
    
    // Identity
    Config.StateBase.Identity.StateName = FName("Combat");
    Config.StateBase.Identity.Category = ECameraStateCategory::Combat;
    Config.StateBase.Identity.Priority = 200;

    // Distance - 更近的战斗距离
    Config.StateBase.Distance.BaseDistance = 350.0f;
    Config.StateBase.Distance.MinDistance = 150.0f;
    Config.StateBase.Distance.MaxDistance = 500.0f;

    // FOV - 稍宽的视野
    Config.StateBase.FOV.BaseFOV = 95.0f;
    Config.StateBase.FOV.MinFOV = 75.0f;
    Config.StateBase.FOV.MaxFOV = 105.0f;

    // Rotation
    Config.StateBase.Rotation.MinPitch = -45.0f;
    Config.StateBase.Rotation.MaxPitch = 60.0f;

    // Offset
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 90.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 60.0f, 10.0f);
    Config.StateBase.Offset.ShoulderOffset = 60.0f;

    // Lag - 更紧密的跟随
    Config.StateBase.Lag.PositionLagSpeed = 12.0f;
    Config.StateBase.Lag.RotationLagSpeed = 10.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 8.0f;
    Config.StateBase.Lag.FOVLagSpeed = 8.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.2f;
    Config.StateBase.Transition.BlendOutTime = 0.3f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::SmoothStep;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = true;
    Config.StateBase.Collision.CollisionRadius = 12.0f;
    Config.StateBase.Collision.RecoveryDelay = 0.3f;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = false;
    Config.StateBase.AutoCorrect.AutoCenterDelay = 5.0f;

    // Collision - 更快响应
    Config.Collision = GetDefaultCollisionConfig();
    Config.Collision.Response.PullInSpeed = 20.0f;
    Config.Collision.Response.PullInAcceleration = 2500.0f;

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetBossLockOnConfig()
{
    FCameraStateConfig Config;
    
    // Identity
    Config.StateBase.Identity.StateName = FName("BossLockOn");
    Config.StateBase.Identity.Category = ECameraStateCategory::Boss;
    Config.StateBase.Identity.Priority = 300;

    // Distance - 根据Boss大小动态调整
    Config.StateBase.Distance.BaseDistance = 500.0f;
    Config.StateBase.Distance.MinDistance = 300.0f;
    Config.StateBase.Distance.MaxDistance = 800.0f;

    // FOV - 宽视野看Boss
    Config.StateBase.FOV.BaseFOV = 100.0f;
    Config.StateBase.FOV.MinFOV = 85.0f;
    Config.StateBase.FOV.MaxFOV = 115.0f;

    // Rotation - 限制俯仰角
    Config.StateBase.Rotation.MinPitch = -30.0f;
    Config.StateBase.Rotation.MaxPitch = 45.0f;

    // Offset - 玩家和Boss之间
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 120.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 0.0f, 20.0f);
    Config.StateBase.Offset.VerticalOffset = 50.0f;

    // Lag - 平滑跟随
    Config.StateBase.Lag.PositionLagSpeed = 6.0f;
    Config.StateBase.Lag.RotationLagSpeed = 8.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 4.0f;
    Config.StateBase.Lag.FOVLagSpeed = 5.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.4f;
    Config.StateBase.Transition.BlendOutTime = 0.5f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::EaseInOut;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = true;
    Config.StateBase.Collision.CollisionRadius = 15.0f;
    Config.StateBase.Collision.RecoveryDelay = 0.3f;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = false;

    // Flags
    Config.StateBase.Flags.bRequiresTarget = true;

    // Collision
    Config.Collision = GetDefaultCollisionConfig();
    Config.Collision.Occlusion.bCheckTargetOcclusion = true;

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetDialogueConfig()
{
    FCameraStateConfig Config;
    
    // Identity
    Config.StateBase.Identity.StateName = FName("Dialogue");
    Config.StateBase.Identity.Category = ECameraStateCategory::NPC;
    Config.StateBase.Identity.Priority = 400;

    // Distance - 对话近距离
    Config.StateBase.Distance.BaseDistance = 200.0f;
    Config.StateBase.Distance.MinDistance = 150.0f;
    Config.StateBase.Distance.MaxDistance = 300.0f;

    // FOV - 电影感
    Config.StateBase.FOV.BaseFOV = 70.0f;
    Config.StateBase.FOV.MinFOV = 60.0f;
    Config.StateBase.FOV.MaxFOV = 80.0f;

    // Rotation - 非常有限
    Config.StateBase.Rotation.MinPitch = -15.0f;
    Config.StateBase.Rotation.MaxPitch = 15.0f;

    // Offset - 相对角色固定
    Config.StateBase.Offset.FocusOffset = FVector(-100.0f, 100.0f, 50.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 30.0f, 0.0f);
    Config.StateBase.Offset.ShoulderOffset = 30.0f;

    // Lag - 缓慢响应
    Config.StateBase.Lag.PositionLagSpeed = 3.0f;
    Config.StateBase.Lag.RotationLagSpeed = 5.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 3.0f;
    Config.StateBase.Lag.FOVLagSpeed = 4.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.5f;
    Config.StateBase.Transition.BlendOutTime = 0.4f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::EaseInOut;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = true;
    Config.StateBase.Collision.CollisionRadius = 10.0f;
    Config.StateBase.Collision.RecoveryDelay = 1.0f;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = false;

    // Flags
    Config.StateBase.Flags.bIsCinematic = true;
    Config.StateBase.Flags.bIgnoreInput = true;

    // Collision - 最小化
    Config.Collision = GetRelaxedCollisionConfig();

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetCinematicConfig()
{
    FCameraStateConfig Config;
    
    // Identity
    Config.StateBase.Identity.StateName = FName("Cinematic");
    Config.StateBase.Identity.Category = ECameraStateCategory::Cinematic;
    Config.StateBase.Identity.Priority = 500;

    // Distance - 过场动画主要由修改器覆盖
    Config.StateBase.Distance.BaseDistance = 300.0f;
    Config.StateBase.Distance.MinDistance = 100.0f;
    Config.StateBase.Distance.MaxDistance = 600.0f;

    // FOV
    Config.StateBase.FOV.BaseFOV = 75.0f;
    Config.StateBase.FOV.MinFOV = 50.0f;
    Config.StateBase.FOV.MaxFOV = 100.0f;

    // Rotation
    Config.StateBase.Rotation.MinPitch = -89.0f;
    Config.StateBase.Rotation.MaxPitch = 89.0f;

    // Offset
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 60.0f);
    Config.StateBase.Offset.SocketOffset = FVector::ZeroVector;

    // Lag
    Config.StateBase.Lag.PositionLagSpeed = 5.0f;
    Config.StateBase.Lag.RotationLagSpeed = 5.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 5.0f;
    Config.StateBase.Lag.FOVLagSpeed = 5.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.5f;
    Config.StateBase.Transition.BlendOutTime = 0.5f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::EaseInOut;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = false;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = false;

    // Flags
    Config.StateBase.Flags.bIsCinematic = true;
    Config.StateBase.Flags.bIgnoreInput = true;

    // Collision - 过场期间最小碰撞
    Config.Collision = GetRelaxedCollisionConfig();
    Config.Collision.bEnableCollision = false;

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetAimingConfig()
{
    FCameraStateConfig Config;
    
    // Identity
    Config.StateBase.Identity.StateName = FName("Aiming");
    Config.StateBase.Identity.Category = ECameraStateCategory::Combat;
    Config.StateBase.Identity.Priority = 250;

    // Distance - 近距离
    Config.StateBase.Distance.BaseDistance = 150.0f;
    Config.StateBase.Distance.MinDistance = 100.0f;
    Config.StateBase.Distance.MaxDistance = 200.0f;

    // FOV - 窄视野瞄准
    Config.StateBase.FOV.BaseFOV = 65.0f;
    Config.StateBase.FOV.MinFOV = 50.0f;
    Config.StateBase.FOV.MaxFOV = 75.0f;

    // Rotation - 完整范围
    Config.StateBase.Rotation.MinPitch = -80.0f;
    Config.StateBase.Rotation.MaxPitch = 80.0f;

    // Offset - 过肩视角，强肩部偏移
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 70.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 80.0f, 20.0f);
    Config.StateBase.Offset.ShoulderOffset = 80.0f;

    // Lag - 非常灵敏
    Config.StateBase.Lag.PositionLagSpeed = 20.0f;
    Config.StateBase.Lag.RotationLagSpeed = 25.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 15.0f;
    Config.StateBase.Lag.FOVLagSpeed = 10.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.15f;
    Config.StateBase.Transition.BlendOutTime = 0.2f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::EaseOut;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = true;
    Config.StateBase.Collision.CollisionRadius = 10.0f;
    Config.StateBase.Collision.RecoveryDelay = 0.1f;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = false;

    // Collision
    Config.Collision = GetAggressiveCollisionConfig();

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetMountedConfig()
{
    FCameraStateConfig Config;
    
    // Identity
    Config.StateBase.Identity.StateName = FName("Mounted");
    Config.StateBase.Identity.Category = ECameraStateCategory::Mount;
    Config.StateBase.Identity.Priority = 150;

    // Distance - 骑乘更远距离
    Config.StateBase.Distance.BaseDistance = 550.0f;
    Config.StateBase.Distance.MinDistance = 300.0f;
    Config.StateBase.Distance.MaxDistance = 800.0f;

    // FOV - 速度感更宽视野
    Config.StateBase.FOV.BaseFOV = 95.0f;
    Config.StateBase.FOV.MinFOV = 80.0f;
    Config.StateBase.FOV.MaxFOV = 110.0f;

    // Rotation
    Config.StateBase.Rotation.MinPitch = -50.0f;
    Config.StateBase.Rotation.MaxPitch = 60.0f;

    // Offset - 更高更远
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 150.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 40.0f, 30.0f);
    Config.StateBase.Offset.VerticalOffset = 30.0f;
    Config.StateBase.Offset.ShoulderOffset = 40.0f;

    // Lag
    Config.StateBase.Lag.PositionLagSpeed = 6.0f;
    Config.StateBase.Lag.RotationLagSpeed = 10.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 5.0f;
    Config.StateBase.Lag.FOVLagSpeed = 6.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.4f;
    Config.StateBase.Transition.BlendOutTime = 0.4f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::SmoothStep;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = true;
    Config.StateBase.Collision.CollisionRadius = 15.0f;
    Config.StateBase.Collision.RecoveryDelay = 0.5f;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = true;
    Config.StateBase.AutoCorrect.AutoCenterDelay = 4.0f;

    // Collision
    Config.Collision = GetDefaultCollisionConfig();

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetSwimmingConfig()
{
    FCameraStateConfig Config;
    
    // Identity
    Config.StateBase.Identity.StateName = FName("Swimming");
    Config.StateBase.Identity.Category = ECameraStateCategory::Swim;
    Config.StateBase.Identity.Priority = 120;

    // Distance
    Config.StateBase.Distance.BaseDistance = 350.0f;
    Config.StateBase.Distance.MinDistance = 200.0f;
    Config.StateBase.Distance.MaxDistance = 500.0f;

    // FOV - 水下稍宽
    Config.StateBase.FOV.BaseFOV = 100.0f;
    Config.StateBase.FOV.MinFOV = 85.0f;
    Config.StateBase.FOV.MaxFOV = 110.0f;

    // Rotation - 水下完全自由
    Config.StateBase.Rotation.MinPitch = -89.0f;
    Config.StateBase.Rotation.MaxPitch = 89.0f;

    // Offset
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 50.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 40.0f, 0.0f);
    Config.StateBase.Offset.ShoulderOffset = 40.0f;

    // Lag - 自由但平滑
    Config.StateBase.Lag.PositionLagSpeed = 5.0f;
    Config.StateBase.Lag.RotationLagSpeed = 8.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 4.0f;
    Config.StateBase.Lag.FOVLagSpeed = 5.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.5f;
    Config.StateBase.Transition.BlendOutTime = 0.5f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::SmoothStep;

    // State Collision
    Config.StateBase.Collision.bEnableCollision = true;
    Config.StateBase.Collision.CollisionRadius = 12.0f;
    Config.StateBase.Collision.RecoveryDelay = 0.5f;

    // AutoCorrect
    Config.StateBase.AutoCorrect.bEnableAutoCenter = true;
    Config.StateBase.AutoCorrect.AutoCenterDelay = 3.0f;

    // Collision - 水体感知
    Config.Collision = GetDefaultCollisionConfig();

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetConfigForCategory(ECameraStateCategory Category)
{
    switch (Category)
    {
        case ECameraStateCategory::FreeExploration:
        case ECameraStateCategory::Explore:
            return GetExplorationConfig();
            
        case ECameraStateCategory::Combat:
        case ECameraStateCategory::LockOn:
            return GetCombatConfig();
            
        case ECameraStateCategory::Boss:
            return GetBossLockOnConfig();
            
        case ECameraStateCategory::NPC:
            return GetDialogueConfig();
            
        case ECameraStateCategory::Cinematic:
            return GetCinematicConfig();
            
        case ECameraStateCategory::Mount:
            return GetMountedConfig();
            
        case ECameraStateCategory::Swim:
            return GetSwimmingConfig();
            
        case ECameraStateCategory::Sprint:
        case ECameraStateCategory::Climb:
        case ECameraStateCategory::Item:
        case ECameraStateCategory::RestPoint:
        case ECameraStateCategory::Death:
        case ECameraStateCategory::Environment:
        case ECameraStateCategory::Magic:
        case ECameraStateCategory::Multiplayer:
        case ECameraStateCategory::UserInterface:
        case ECameraStateCategory::Modifier:
        default:
            return GetExplorationConfig();
    }
}

//========================================
// Game Style Presets - 游戏风格预设
//========================================

FCameraStateConfig USoulsCameraPresets::GetDarkSoulsStyle()
{
    FCameraStateConfig Config;
    
    // Identity - Dark Souls风格：沉重、审慎
    Config.StateBase.Identity.StateName = FName("DarkSoulsStyle");
    Config.StateBase.Identity.Category = ECameraStateCategory::Explore;
    Config.StateBase.Identity.Priority = 100;

    // Distance - 中等距离，观察更多环境
    Config.StateBase.Distance.BaseDistance = 450.0f;
    Config.StateBase.Distance.MinDistance = 200.0f;
    Config.StateBase.Distance.MaxDistance = 650.0f;

    // FOV - 标准视野
    Config.StateBase.FOV.BaseFOV = 75.0f;
    Config.StateBase.FOV.MinFOV = 65.0f;
    Config.StateBase.FOV.MaxFOV = 90.0f;

    // Rotation - 较大俯仰范围
    Config.StateBase.Rotation.MinPitch = -55.0f;
    Config.StateBase.Rotation.MaxPitch = 65.0f;

    // Offset - 偏右肩
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 75.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 45.0f, 0.0f);
    Config.StateBase.Offset.ShoulderOffset = 45.0f;

    // Lag - 中等延迟，感觉更沉重
    Config.StateBase.Lag.PositionLagSpeed = 7.0f;
    Config.StateBase.Lag.RotationLagSpeed = 10.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 5.0f;
    Config.StateBase.Lag.FOVLagSpeed = 5.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.4f;
    Config.StateBase.Transition.BlendOutTime = 0.4f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::SmoothStep;

    // Collision
    Config.Collision = GetDefaultCollisionConfig();

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetBloodborneStyle()
{
    FCameraStateConfig Config;
    
    // Identity - Bloodborne风格：更快、更近
    Config.StateBase.Identity.StateName = FName("BloodborneStyle");
    Config.StateBase.Identity.Category = ECameraStateCategory::Combat;
    Config.StateBase.Identity.Priority = 100;

    // Distance - 更近，强调近战
    Config.StateBase.Distance.BaseDistance = 320.0f;
    Config.StateBase.Distance.MinDistance = 150.0f;
    Config.StateBase.Distance.MaxDistance = 500.0f;

    // FOV - 较宽视野，配合快速战斗
    Config.StateBase.FOV.BaseFOV = 95.0f;
    Config.StateBase.FOV.MinFOV = 80.0f;
    Config.StateBase.FOV.MaxFOV = 110.0f;

    // Rotation
    Config.StateBase.Rotation.MinPitch = -50.0f;
    Config.StateBase.Rotation.MaxPitch = 60.0f;

    // Offset - 更紧密
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 85.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 55.0f, 5.0f);
    Config.StateBase.Offset.ShoulderOffset = 55.0f;

    // Lag - 快速响应
    Config.StateBase.Lag.PositionLagSpeed = 12.0f;
    Config.StateBase.Lag.RotationLagSpeed = 14.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 10.0f;
    Config.StateBase.Lag.FOVLagSpeed = 8.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.25f;
    Config.StateBase.Transition.BlendOutTime = 0.25f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::EaseOut;

    // Collision - 快速响应
    Config.Collision = GetAggressiveCollisionConfig();

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetSekiroStyle()
{
    FCameraStateConfig Config;
    
    // Identity - Sekiro风格：非常灵敏、精确
    Config.StateBase.Identity.StateName = FName("SekiroStyle");
    Config.StateBase.Identity.Category = ECameraStateCategory::Combat;
    Config.StateBase.Identity.Priority = 100;

    // Distance - 中近距离
    Config.StateBase.Distance.BaseDistance = 350.0f;
    Config.StateBase.Distance.MinDistance = 180.0f;
    Config.StateBase.Distance.MaxDistance = 550.0f;

    // FOV - 稍窄，更精确
    Config.StateBase.FOV.BaseFOV = 85.0f;
    Config.StateBase.FOV.MinFOV = 70.0f;
    Config.StateBase.FOV.MaxFOV = 100.0f;

    // Rotation - 大范围
    Config.StateBase.Rotation.MinPitch = -60.0f;
    Config.StateBase.Rotation.MaxPitch = 70.0f;

    // Offset
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 90.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 50.0f, 10.0f);
    Config.StateBase.Offset.ShoulderOffset = 50.0f;

    // Lag - 非常灵敏
    Config.StateBase.Lag.PositionLagSpeed = 18.0f;
    Config.StateBase.Lag.RotationLagSpeed = 20.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 12.0f;
    Config.StateBase.Lag.FOVLagSpeed = 10.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.15f;
    Config.StateBase.Transition.BlendOutTime = 0.2f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::EaseOut;

    // Collision
    Config.Collision = GetAggressiveCollisionConfig();
    Config.Collision.Response.PullInSpeed = 25.0f;

    return Config;
}

FCameraStateConfig USoulsCameraPresets::GetEldenRingStyle()
{
    FCameraStateConfig Config;
    
    // Identity - Elden Ring风格：开阔视野、探索
    Config.StateBase.Identity.StateName = FName("EldenRingStyle");
    Config.StateBase.Identity.Category = ECameraStateCategory::FreeExploration;
    Config.StateBase.Identity.Priority = 100;

    // Distance - 更远，观察开阔世界
    Config.StateBase.Distance.BaseDistance = 500.0f;
    Config.StateBase.Distance.MinDistance = 250.0f;
    Config.StateBase.Distance.MaxDistance = 800.0f;

    // FOV - 宽视野
    Config.StateBase.FOV.BaseFOV = 90.0f;
    Config.StateBase.FOV.MinFOV = 75.0f;
    Config.StateBase.FOV.MaxFOV = 110.0f;

    // Rotation - 完整范围
    Config.StateBase.Rotation.MinPitch = -65.0f;
    Config.StateBase.Rotation.MaxPitch = 75.0f;

    // Offset - 标准肩部
    Config.StateBase.Offset.FocusOffset = FVector(0.0f, 0.0f, 80.0f);
    Config.StateBase.Offset.SocketOffset = FVector(0.0f, 50.0f, 0.0f);
    Config.StateBase.Offset.ShoulderOffset = 50.0f;

    // Lag - 平衡响应
    Config.StateBase.Lag.PositionLagSpeed = 10.0f;
    Config.StateBase.Lag.RotationLagSpeed = 12.0f;
    Config.StateBase.Lag.DistanceLagSpeed = 7.0f;
    Config.StateBase.Lag.FOVLagSpeed = 6.0f;

    // Transition
    Config.StateBase.Transition.BlendInTime = 0.3f;
    Config.StateBase.Transition.BlendOutTime = 0.35f;
    Config.StateBase.Transition.BlendType = ECameraBlendType::SmoothStep;

    // AutoCorrect - 骑乘时自动对齐
    Config.StateBase.AutoCorrect.bEnableAutoCenter = true;
    Config.StateBase.AutoCorrect.AutoCenterDelay = 3.5f;

    // Collision
    Config.Collision = GetDefaultCollisionConfig();

    return Config;
}

//========================================
// Module Activation Presets - 模块激活预设
//========================================

TArray<ECameraModuleType> USoulsCameraPresets::GetExplorationModules()
{
    TArray<ECameraModuleType> Modules;
    
    // Position modules
    Modules.Add(ECameraModuleType::Module_P02_FollowTarget_Lagged);
    
    // Rotation modules
    Modules.Add(ECameraModuleType::Module_R02_PlayerInput_Lagged);
    Modules.Add(ECameraModuleType::Module_R07_AutoOrient_Delayed);
    
    // Distance modules
    Modules.Add(ECameraModuleType::Module_D01_BaseDistance);
    Modules.Add(ECameraModuleType::Module_D03_Speed_Offset);
    
    // FOV modules
    Modules.Add(ECameraModuleType::Module_F01_BaseFOV);
    Modules.Add(ECameraModuleType::Module_F02_Speed_FOV);
    
    // Offset modules
    Modules.Add(ECameraModuleType::Module_O01_SocketOffset_Base);
    Modules.Add(ECameraModuleType::Module_O02_Shoulder_Offset);
    
    // Constraint modules
    Modules.Add(ECameraModuleType::Module_C01_Pitch_Clamp);
    Modules.Add(ECameraModuleType::Module_C02_Distance_Clamp);
    
    return Modules;
}

TArray<ECameraModuleType> USoulsCameraPresets::GetCombatModules()
{
    TArray<ECameraModuleType> Modules;
    
    // Position modules
    Modules.Add(ECameraModuleType::Module_P02_FollowTarget_Lagged);
    
    // Rotation modules
    Modules.Add(ECameraModuleType::Module_R02_PlayerInput_Lagged);
    Modules.Add(ECameraModuleType::Module_R04_LookAt_Target_Soft);
    
    // Distance modules
    Modules.Add(ECameraModuleType::Module_D01_BaseDistance);
    Modules.Add(ECameraModuleType::Module_D04_Combat_Adjust);
    Modules.Add(ECameraModuleType::Module_D06_Proximity_Adjust);
    
    // FOV modules
    Modules.Add(ECameraModuleType::Module_F01_BaseFOV);
    Modules.Add(ECameraModuleType::Module_F04_Combat_FOV);
    
    // Offset modules
    Modules.Add(ECameraModuleType::Module_O01_SocketOffset_Base);
    Modules.Add(ECameraModuleType::Module_O02_Shoulder_Offset);
    
    // Constraint modules
    Modules.Add(ECameraModuleType::Module_C01_Pitch_Clamp);
    Modules.Add(ECameraModuleType::Module_C02_Distance_Clamp);
    Modules.Add(ECameraModuleType::Module_C04_Visibility_Ensure);
    
    return Modules;
}

TArray<ECameraModuleType> USoulsCameraPresets::GetBossLockOnModules()
{
    TArray<ECameraModuleType> Modules;
    
    // Position modules - 中点追踪
    Modules.Add(ECameraModuleType::Module_P08_MidPoint_TwoTarget);
    Modules.Add(ECameraModuleType::Module_P05_Orbit_Boss);
    
    // Rotation modules - Boss锁定
    Modules.Add(ECameraModuleType::Module_R05_LookAt_Boss);
    
    // Distance modules
    Modules.Add(ECameraModuleType::Module_D01_BaseDistance);
    Modules.Add(ECameraModuleType::Module_D02_TargetSize_Multiplier);
    Modules.Add(ECameraModuleType::Module_D07_Boss_Phase);
    
    // FOV modules
    Modules.Add(ECameraModuleType::Module_F01_BaseFOV);
    Modules.Add(ECameraModuleType::Module_F05_Boss_FOV);
    
    // Offset modules
    Modules.Add(ECameraModuleType::Module_O01_SocketOffset_Base);
    
    // Constraint modules
    Modules.Add(ECameraModuleType::Module_C01_Pitch_Clamp);
    Modules.Add(ECameraModuleType::Module_C02_Distance_Clamp);
    Modules.Add(ECameraModuleType::Module_C04_Visibility_Ensure);
    
    return Modules;
}

//========================================
// Collision Presets - 碰撞配置预设
//========================================

FCollisionParams USoulsCameraPresets::GetDefaultCollisionConfig()
{
    FCollisionParams Params;
    
    // Enable collision
    Params.bEnableCollision = true;
    Params.bDrawDebug = false;
    
    // Detection - 使用默认球体扫描
    Params.Detection.bEnableD01_SingleRay = false;
    Params.Detection.bEnableD02_SphereSweep = true;
    Params.Detection.bEnableD03_MultiRay = false;
    Params.Detection.bEnableD04_MultiSphereSweep = false;
    Params.Detection.ProbeRadius = 12.0f;
    Params.Detection.CollisionChannel = ECC_Camera;
    Params.Detection.DetectionStartOffset = 20.0f;
    Params.Detection.DetectionRate = 60.0f;
    Params.Detection.CollisionPadding = 10.0f;
    
    // Response - 拉近响应
    Params.Response.bEnableRS01_PullIn = true;
    Params.Response.bEnableRS02_Slide = false;
    Params.Response.bEnableRS03_Orbit = false;
    Params.Response.bEnableRS04_FOVCompensate = false;
    Params.Response.bEnableRS05_InstantSnap = false;
    Params.Response.PullInSpeed = 15.0f;
    Params.Response.MinPullInDistance = 50.0f;
    Params.Response.PullInAcceleration = 2000.0f;
    Params.Response.SoftCollisionDistance = 100.0f;
    
    // Occlusion - 淡出遮挡物
    Params.Occlusion.bEnableOC01_FadeOut = true;
    Params.Occlusion.bEnableOC02_Hide = false;
    Params.Occlusion.bEnableOC03_PullInFurther = false;
    Params.Occlusion.bEnableOC04_DitherFade = false;
    Params.Occlusion.OccluderFadeSpeed = 8.0f;
    Params.Occlusion.OccluderMinOpacity = 0.2f;
    Params.Occlusion.FadeStartDistance = 150.0f;
    Params.Occlusion.FadeEndDistance = 50.0f;
    Params.Occlusion.bCheckCharacterOcclusion = true;
    Params.Occlusion.bCheckTargetOcclusion = true;
    Params.Occlusion.OcclusionCheckInterval = 0.05f;
    Params.Occlusion.MaxTrackedOccluders = 5;
    
    // Recovery - 延迟恢复
    Params.Recovery.bEnableRC01_DelayedRecovery = true;
    Params.Recovery.bEnableRC02_SmoothRecovery = false;
    Params.Recovery.bEnableRC03_StepRecovery = false;
    Params.Recovery.RecoveryDelay = 0.5f;
    Params.Recovery.RecoverySpeed = 5.0f;
    Params.Recovery.RecoveryAcceleration = 500.0f;
    Params.Recovery.bAbortRecoveryOnNewCollision = true;
    Params.Recovery.bVerifyRecoveryPath = true;
    
    // Special - 启用常见特殊情况
    Params.Special.bEnableSP01_TightSpace = true;
    Params.Special.bEnableSP02_LowCeiling = true;
    Params.Special.bEnableSP03_CliffEdge = true;
    Params.Special.bEnableSP04_CornerCase = true;
    Params.Special.TightSpaceThreshold = 250.0f;
    Params.Special.TightSpaceDistanceReduction = 100.0f;
    Params.Special.TightSpaceFOVBonus = 15.0f;
    Params.Special.LowCeilingThreshold = 250.0f;
    Params.Special.LowCeilingPitchAdjust = 15.0f;
    Params.Special.CliffDetectionDistance = 150.0f;
    Params.Special.CliffHeightThreshold = 200.0f;
    Params.Special.EdgePullbackDistance = 50.0f;
    Params.Special.CornerCheckDistance = 100.0f;
    
    return Params;
}

FCollisionParams USoulsCameraPresets::GetAggressiveCollisionConfig()
{
    FCollisionParams Params = GetDefaultCollisionConfig();
    
    // Detection - 更灵敏
    Params.Detection.ProbeRadius = 15.0f;
    Params.Detection.DetectionRate = 90.0f;
    Params.Detection.CollisionPadding = 15.0f;
    
    // Response - 快速拉近
    Params.Response.PullInSpeed = 25.0f;
    Params.Response.PullInAcceleration = 3000.0f;
    Params.Response.MinPullInDistance = 40.0f;
    Params.Response.bEnableRS04_FOVCompensate = true;
    Params.Response.PullInFOVCompensation = 10.0f;
    Params.Response.MaxFOVCompensation = 15.0f;
    
    // Occlusion - 更快淡出
    Params.Occlusion.OccluderFadeSpeed = 12.0f;
    Params.Occlusion.OcclusionCheckInterval = 0.033f;  // 30Hz
    
    // Recovery - 快速恢复
    Params.Recovery.RecoveryDelay = 0.2f;
    Params.Recovery.RecoverySpeed = 10.0f;
    Params.Recovery.RecoveryAcceleration = 1000.0f;
    
    // Special - 更积极的特殊处理
    Params.Special.TightSpaceDistanceReduction = 150.0f;
    Params.Special.TightSpaceFOVBonus = 20.0f;
    
    return Params;
}

FCollisionParams USoulsCameraPresets::GetRelaxedCollisionConfig()
{
    FCollisionParams Params = GetDefaultCollisionConfig();
    
    // Detection - 较少检测
    Params.Detection.ProbeRadius = 10.0f;
    Params.Detection.DetectionRate = 30.0f;
    Params.Detection.CollisionPadding = 5.0f;
    
    // Response - 缓慢拉近
    Params.Response.PullInSpeed = 8.0f;
    Params.Response.PullInAcceleration = 1000.0f;
    Params.Response.MinPullInDistance = 80.0f;
    Params.Response.SoftCollisionDistance = 150.0f;
    
    // Occlusion - 缓慢淡出
    Params.Occlusion.OccluderFadeSpeed = 4.0f;
    Params.Occlusion.OcclusionCheckInterval = 0.1f;
    
    // Recovery - 缓慢恢复
    Params.Recovery.RecoveryDelay = 1.0f;
    Params.Recovery.RecoverySpeed = 3.0f;
    Params.Recovery.RecoveryAcceleration = 300.0f;
    
    // Special - 减少特殊处理
    Params.Special.bEnableSP03_CliffEdge = false;
    Params.Special.bEnableSP04_CornerCase = false;
    Params.Special.TightSpaceDistanceReduction = 50.0f;
    Params.Special.TightSpaceFOVBonus = 10.0f;
    
    return Params;
}
