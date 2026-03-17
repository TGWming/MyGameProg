// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraStateEnums.generated.h"

/**
 * Camera state main categories
 * 相机状态主分类
 * 
 * Used to classify and organize the 725 camera states
 * 用于分类和组织725个相机状态
 * 
 * Maps to: camera_states_full_725.csv "Category" column
 * 对应CSV: camera_states_full_725.csv 的 "Category" 列
 */
UENUM(BlueprintType)
enum class ECameraStateCategory : uint8
{
    /** No category / 无分类 */
    None = 0                    UMETA(DisplayName = "None"),
    
    //========================================
    // Primary Categories (from CSV)
    // 主要分类（来自CSV）
    //========================================
    
    /** Free exploration states / 自由探索状态 */
    FreeExploration             UMETA(DisplayName = "Free Exploration"),
    
    /** Combat states / 战斗状态 */
    Combat                      UMETA(DisplayName = "Combat"),
    
    /** Boss encounter states / Boss战状态 */
    Boss                        UMETA(DisplayName = "Boss"),
    
    /** Environment interaction states / 环境交互状态 */
    Environment                 UMETA(DisplayName = "Environment"),
    
    /** Item interaction states / 物品交互状态 */
    Item                        UMETA(DisplayName = "Item"),
    
    /** NPC interaction states / NPC交互状态 */
    NPC                         UMETA(DisplayName = "NPC"),
    
    /** Rest point states (bonfire, checkpoint) / 休息点状态（篝火、检查点）*/
    RestPoint                   UMETA(DisplayName = "Rest Point"),
    
    /** Death and respawn states / 死亡和重生状态 */
    Death                       UMETA(DisplayName = "Death"),
    
    /** Cinematic states / 过场动画状态 */
    Cinematic                   UMETA(DisplayName = "Cinematic"),
    
    /** Magic and skill states / 魔法和技能状态 */
    Magic                       UMETA(DisplayName = "Magic"),
    
    /** Multiplayer states / 多人游戏状态 */
    Multiplayer                 UMETA(DisplayName = "Multiplayer"),
    
    /** UI related states / UI相关状态 */
    UI                          UMETA(DisplayName = "UI"),
    
    /** Modifier states (camera effects) / 修饰器状态（相机效果）*/
    Modifier                    UMETA(DisplayName = "Modifier"),
    
    //========================================
    // Legacy/Extended Categories (for backward compatibility)
    // 遗留/扩展分类（用于向后兼容）
    //========================================
    
    /** Basic exploration state / 基础探索状态 */
    Explore                     UMETA(DisplayName = "Explore"),
    
    /** Lock-on targeting state / 锁定瞄准状态 */
    LockOn                      UMETA(DisplayName = "Lock On"),
    
    /** Sprint movement state / 冲刺移动状态 */
    Sprint                      UMETA(DisplayName = "Sprint"),
    
    /** Mount riding state / 骑乘状态 */
    Mount                       UMETA(DisplayName = "Mount"),
    
    /** Swimming state / 游泳状态 */
    Swim                        UMETA(DisplayName = "Swim"),
    
    /** Climbing state / 攀爬状态 */
    Climb                       UMETA(DisplayName = "Climb"),
    
    /** User interface state (alias for UI) / 用户界面状态（UI的别名）*/
    UserInterface               UMETA(DisplayName = "User Interface"),
    
    MAX                         UMETA(Hidden)
};

/**
 * Camera State Sub-Category Enumeration
 */
UENUM(BlueprintType)
enum class ECameraStateSubCategory : uint8
{
    None UMETA(DisplayName = "None"),
    
    // FreeExploration sub-categories
    BasicMovement UMETA(DisplayName = "Basic Movement"),
    Idle UMETA(DisplayName = "Idle"),
    Alignment UMETA(DisplayName = "Alignment"),
    Shoulder UMETA(DisplayName = "Shoulder"),
    Collision UMETA(DisplayName = "Collision"),
    Movement UMETA(DisplayName = "Movement"),
    Climbing UMETA(DisplayName = "Climbing"),
    Mount UMETA(DisplayName = "Mount"),
    Edge UMETA(DisplayName = "Edge"),
    Stealth UMETA(DisplayName = "Stealth"),
    AreaTransition UMETA(DisplayName = "Area Transition"),
    
    // Combat sub-categories
    BasicCombat UMETA(DisplayName = "Basic Combat"),
    Threat UMETA(DisplayName = "Threat"),
    LockOn UMETA(DisplayName = "Lock On"),
    Attack UMETA(DisplayName = "Attack"),
    Defend UMETA(DisplayName = "Defend"),
    Dodge UMETA(DisplayName = "Dodge"),
    Hit UMETA(DisplayName = "Hit"),
    Status UMETA(DisplayName = "Status"),
    Special UMETA(DisplayName = "Special"),
    Boss UMETA(DisplayName = "Boss"),
    Ranged UMETA(DisplayName = "Ranged"),
    CombatMultiplayer UMETA(DisplayName = "Combat Multiplayer"),
    DeadAngle UMETA(DisplayName = "Dead Angle"),
    Summon UMETA(DisplayName = "Summon"),
    
    // Environment sub-categories
    Door UMETA(DisplayName = "Door"),
    Destructible UMETA(DisplayName = "Destructible"),
    Mechanism UMETA(DisplayName = "Mechanism"),
    Trap UMETA(DisplayName = "Trap"),
    Elevator UMETA(DisplayName = "Elevator"),
    Light UMETA(DisplayName = "Light"),
    POI UMETA(DisplayName = "POI"),
    Puzzle UMETA(DisplayName = "Puzzle"),
    Hazard UMETA(DisplayName = "Hazard"),
    Altar UMETA(DisplayName = "Altar"),
    
    // Item sub-categories
    Pickup UMETA(DisplayName = "Pickup"),
    Bloodstain UMETA(DisplayName = "Bloodstain"),
    Message UMETA(DisplayName = "Message"),
    Use UMETA(DisplayName = "Use"),
    Equip UMETA(DisplayName = "Equip"),
    Upgrade UMETA(DisplayName = "Upgrade"),
    Craft UMETA(DisplayName = "Craft"),
    Map UMETA(DisplayName = "Map"),
    
    // NPC sub-categories
    Dialogue UMETA(DisplayName = "Dialogue"),
    Service UMETA(DisplayName = "Service"),
    NPCSpecial UMETA(DisplayName = "NPC Special"),
    
    // RestPoint sub-categories
    Bonfire UMETA(DisplayName = "Bonfire"),
    Checkpoint UMETA(DisplayName = "Checkpoint"),
    Hub UMETA(DisplayName = "Hub"),
    
    // Death sub-categories
    DeathProcess UMETA(DisplayName = "Death Process"),
    DeathCamera UMETA(DisplayName = "Death Camera"),
    Respawn UMETA(DisplayName = "Respawn"),
    Revive UMETA(DisplayName = "Revive"),
    Ghost UMETA(DisplayName = "Ghost"),
    
    // Cinematic sub-categories
    CutsceneType UMETA(DisplayName = "Cutscene Type"),
    Reveal UMETA(DisplayName = "Reveal"),
    Character UMETA(DisplayName = "Character"),
    WorldEvent UMETA(DisplayName = "World Event"),
    Flashback UMETA(DisplayName = "Flashback"),
    CinematicTransition UMETA(DisplayName = "Cinematic Transition"),
    
    // Magic sub-categories
    SpellCast UMETA(DisplayName = "Spell Cast"),
    MagicSummon UMETA(DisplayName = "Magic Summon"),
    Skill UMETA(DisplayName = "Skill"),
    
    // Multiplayer sub-categories
    MultiplayerSummon UMETA(DisplayName = "Multiplayer Summon"),
    PvP UMETA(DisplayName = "PvP"),
    Coop UMETA(DisplayName = "Coop"),
    
    // UserInterface sub-categories
    Menu UMETA(DisplayName = "Menu"),
    PhotoMode UMETA(DisplayName = "Photo Mode"),
    
    // Modifier sub-categories
    Core UMETA(DisplayName = "Core")
};

/**
 * Camera State Reference Game Enumeration
 */
UENUM(BlueprintType)
enum class ECameraStateReference : uint8
{
    None UMETA(DisplayName = "None"),
    All UMETA(DisplayName = "All"),
    EldenRing UMETA(DisplayName = "Elden Ring"),
    Bloodborne UMETA(DisplayName = "Bloodborne"),
    DarkSouls UMETA(DisplayName = "Dark Souls"),
    DarkSouls3 UMETA(DisplayName = "Dark Souls 3"),
    Sekiro UMETA(DisplayName = "Sekiro"),
    LiesOfP UMETA(DisplayName = "Lies of P"),
    Some UMETA(DisplayName = "Some"),
    Few UMETA(DisplayName = "Few")
};

/**
 * Camera Blend Type Enumeration
 * Defines interpolation curve types for camera transitions
 */
UENUM(BlueprintType)
enum class ECameraBlendType : uint8
{
    Cut = 0                     UMETA(DisplayName = "Cut"),
    Linear                      UMETA(DisplayName = "Linear"),
    SmoothStep                  UMETA(DisplayName = "Smooth Step"),
    EaseIn                      UMETA(DisplayName = "Ease In"),
    EaseOut                     UMETA(DisplayName = "Ease Out"),
    EaseInOut                   UMETA(DisplayName = "Ease In Out"),
    Cubic                       UMETA(DisplayName = "Cubic"),
    Spring                      UMETA(DisplayName = "Spring"),
    Custom                      UMETA(DisplayName = "Custom Curve"),
    Exponential                 UMETA(DisplayName = "Exponential"),
    Circular                    UMETA(DisplayName = "Circular"),
    Instant                     UMETA(DisplayName = "Instant"),
    MAX                         UMETA(Hidden)
};

/**
 * Camera module types - 39 modules total (Legacy naming for compatibility)
 * Each module handles a specific aspect of camera calculation
 * Note: This is the legacy enum, use ECameraModuleID for new code
 */
UENUM(BlueprintType)
enum class ECameraModuleType : uint8
{
    ModuleNone = 0                      UMETA(DisplayName = "None"),
    
    // Position Modules (P01-P08)
    Module_P01_FollowTarget             UMETA(DisplayName = "P01 Follow Target"),
    Module_P02_FollowTarget_Lagged      UMETA(DisplayName = "P02 Follow Target Lagged"),
    Module_P03_FollowTarget_Predictive  UMETA(DisplayName = "P03 Follow Target Predictive"),
    Module_P04_Orbit_LockOn             UMETA(DisplayName = "P04 Orbit Lock On"),
    Module_P05_Orbit_Boss               UMETA(DisplayName = "P05 Orbit Boss"),
    Module_P06_FixedPoint               UMETA(DisplayName = "P06 Fixed Point"),
    Module_P07_Spline_Follow            UMETA(DisplayName = "P07 Spline Follow"),
    Module_P08_MidPoint_TwoTarget       UMETA(DisplayName = "P08 MidPoint Two Target"),
    
    // Rotation Modules (R01-R09)
    Module_R01_PlayerInput_Free         UMETA(DisplayName = "R01 Player Input Free"),
    Module_R02_PlayerInput_Lagged       UMETA(DisplayName = "R02 Player Input Lagged"),
    Module_R03_LookAt_Target            UMETA(DisplayName = "R03 LookAt Target"),
    Module_R04_LookAt_Target_Soft       UMETA(DisplayName = "R04 LookAt Target Soft"),
    Module_R05_LookAt_Boss              UMETA(DisplayName = "R05 LookAt Boss"),
    Module_R06_AutoOrient_Movement      UMETA(DisplayName = "R06 Auto Orient Movement"),
    Module_R07_AutoOrient_Delayed       UMETA(DisplayName = "R07 Auto Orient Delayed"),
    Module_R08_LookAt_Point             UMETA(DisplayName = "R08 LookAt Point"),
    Module_R09_Spline_Rotation          UMETA(DisplayName = "R09 Spline Rotation"),
    Module_R10_Framing                   UMETA(DisplayName = "R10 Framing"),
    
    // Distance Modules (D01-D07)
    Module_D01_BaseDistance             UMETA(DisplayName = "D01 Base Distance"),
    Module_D02_TargetSize_Multiplier    UMETA(DisplayName = "D02 Target Size Multiplier"),
    Module_D03_Speed_Offset             UMETA(DisplayName = "D03 Speed Offset"),
    Module_D04_Combat_Adjust            UMETA(DisplayName = "D04 Combat Adjust"),
    Module_D05_Environment_Limit        UMETA(DisplayName = "D05 Environment Limit"),
    Module_D06_Proximity_Adjust         UMETA(DisplayName = "D06 Proximity Adjust"),
    Module_D07_Boss_Phase               UMETA(DisplayName = "D07 Boss Phase"),
    
    // FOV Modules (F01-F06)
    Module_F01_BaseFOV                  UMETA(DisplayName = "F01 Base FOV"),
    Module_F02_Speed_FOV                UMETA(DisplayName = "F02 Speed FOV"),
    Module_F03_Aim_FOV                  UMETA(DisplayName = "F03 Aim FOV"),
    Module_F04_Combat_FOV               UMETA(DisplayName = "F04 Combat FOV"),
    Module_F05_Boss_FOV                 UMETA(DisplayName = "F05 Boss FOV"),
    Module_F06_Impact_FOV               UMETA(DisplayName = "F06 Impact FOV"),
    
    // Offset Modules (O01-O05)
    Module_O01_SocketOffset_Base        UMETA(DisplayName = "O01 Socket Offset Base"),
    Module_O02_Shoulder_Offset          UMETA(DisplayName = "O02 Shoulder Offset"),
    Module_O03_Shoulder_Switch          UMETA(DisplayName = "O03 Shoulder Switch"),
    Module_O04_Crouch_Offset            UMETA(DisplayName = "O04 Crouch Offset"),
    Module_O05_Mount_Offset             UMETA(DisplayName = "O05 Mount Offset"),
    
    // Constraint Modules (C01-C04)
    Module_C01_Pitch_Clamp              UMETA(DisplayName = "C01 Pitch Clamp"),
    Module_C02_Distance_Clamp           UMETA(DisplayName = "C02 Distance Clamp"),
    Module_C03_FOV_Clamp                UMETA(DisplayName = "C03 FOV Clamp"),
    Module_C04_Visibility_Ensure        UMETA(DisplayName = "C04 Visibility Ensure"),
    
    ModuleMAX                           UMETA(Hidden)
};

/**
 * Camera module ID types - 39 modules total (New naming convention)
 * Each module handles a specific aspect of camera calculation
 * 
 * Categories:
 * - Position (P01-P08): 8 modules - Calculate focus point
 * - Rotation (R01-R09): 9 modules - Calculate camera orientation
 * - Distance (D01-D07): 7 modules - Calculate arm length
 * - FOV (F01-F06): 6 modules - Calculate field of view
 * - Offset (O01-O05): 5 modules - Calculate position offsets
 * - Constraint (C01-C04): 4 modules - Enforce limits
 * 
 * Total: 39 modules
 */
UENUM(BlueprintType)
enum class ECameraModuleID : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    
    //========================================
    // Position Modules (P01-P08) - 8 modules
    //========================================
    Module_P01_Position_FollowTarget UMETA(DisplayName = "P01: Follow Target"),
    Module_P02_Position_FollowLagged UMETA(DisplayName = "P02: Follow Target Lagged"),
    Module_P03_Position_FollowPredictive UMETA(DisplayName = "P03: Follow Target Predictive"),
    Module_P04_Position_OrbitLockOn UMETA(DisplayName = "P04: Orbit Lock-On"),
    Module_P05_Position_OrbitBoss UMETA(DisplayName = "P05: Orbit Boss"),
    Module_P06_Position_FixedPoint UMETA(DisplayName = "P06: Fixed Point"),
    Module_P07_Position_SplineFollow UMETA(DisplayName = "P07: Spline Follow"),
    Module_P08_Position_MidPointTwoTarget UMETA(DisplayName = "P08: Mid-Point Two Target"),
    
    //========================================
    // Rotation Modules (R01-R09) - 9 modules
    //========================================
    Module_R01_Rotation_PlayerInputFree UMETA(DisplayName = "R01: Player Input Free"),
    Module_R02_Rotation_PlayerInputLagged UMETA(DisplayName = "R02: Player Input Lagged"),
    Module_R03_Rotation_LookAtTarget UMETA(DisplayName = "R03: Look At Target"),
    Module_R04_Rotation_LookAtTargetSoft UMETA(DisplayName = "R04: Look At Target Soft"),
    Module_R05_Rotation_LookAtBoss UMETA(DisplayName = "R05: Look At Boss"),
    Module_R06_Rotation_AutoOrientMovement UMETA(DisplayName = "R06: Auto Orient Movement"),
    Module_R07_Rotation_AutoOrientDelayed UMETA(DisplayName = "R07: Auto Orient Delayed"),
    Module_R08_Rotation_LookAtPoint UMETA(DisplayName = "R08: Look At Point"),
    Module_R09_Rotation_SplineRotation UMETA(DisplayName = "R09: Spline Rotation"),
    Module_R10_Rotation_Framing UMETA(DisplayName = "R10: Framing"),
    
    //========================================
    // Distance Modules (D01-D07) - 7 modules
    //========================================
    Module_D01_Distance_Base UMETA(DisplayName = "D01: Base Distance"),
    Module_D02_Distance_TargetSizeMultiplier UMETA(DisplayName = "D02: Target Size Multiplier"),
    Module_D03_Distance_SpeedOffset UMETA(DisplayName = "D03: Speed Offset"),
    Module_D04_Distance_CombatAdjust UMETA(DisplayName = "D04: Combat Adjust"),
    Module_D05_Distance_EnvironmentLimit UMETA(DisplayName = "D05: Environment Limit"),
    Module_D06_Distance_ProximityAdjust UMETA(DisplayName = "D06: Proximity Adjust"),
    Module_D07_Distance_BossPhase UMETA(DisplayName = "D07: Boss Phase"),
    
    //========================================
    // FOV Modules (F01-F06) - 6 modules
    //========================================
    Module_F01_FOV_Base UMETA(DisplayName = "F01: Base FOV"),
    Module_F02_FOV_Speed UMETA(DisplayName = "F02: Speed FOV"),
    Module_F03_FOV_Aim UMETA(DisplayName = "F03: Aim FOV"),
    Module_F04_FOV_Combat UMETA(DisplayName = "F04: Combat FOV"),
    Module_F05_FOV_Boss UMETA(DisplayName = "F05: Boss FOV"),
    Module_F06_FOV_Impact UMETA(DisplayName = "F06: Impact FOV"),
    
    //========================================
    // Offset Modules (O01-O05) - 5 modules
    //========================================
    Module_O01_Offset_SocketBase UMETA(DisplayName = "O01: Socket Offset Base"),
    Module_O02_Offset_Shoulder UMETA(DisplayName = "O02: Shoulder Offset"),
    Module_O03_Offset_ShoulderSwitch UMETA(DisplayName = "O03: Shoulder Switch"),
    Module_O04_Offset_Crouch UMETA(DisplayName = "O04: Crouch Offset"),
    Module_O05_Offset_Mount UMETA(DisplayName = "O05: Mount Offset"),
    
    //========================================
    // Constraint Modules (C01-C04) - 4 modules
    //========================================
    Module_C01_Constraint_PitchClamp UMETA(DisplayName = "C01: Pitch Clamp"),
    Module_C02_Constraint_DistanceClamp UMETA(DisplayName = "C02: Distance Clamp"),
    Module_C03_Constraint_FOVClamp UMETA(DisplayName = "C03: FOV Clamp"),
    Module_C04_Constraint_VisibilityEnsure UMETA(DisplayName = "C04: Visibility Ensure"),
    
    MAX UMETA(Hidden)
};

/**
 * Camera modifier ID types - 26 modifiers total
 * Modifiers are event-triggered temporary effects
 * 
 * Categories:
 * - Shake (S01-S05): 5 modifiers - Camera shake effects
 * - Reaction (R01-R06): 6 modifiers - Hit reaction effects
 * - Cinematic (C01-C05): 5 modifiers - Cinematic camera effects
 * - Zoom (Z01-Z04): 4 modifiers - Zoom effects
 * - Effect (E01-E03): 3 modifiers - Visual effects
 * - Special (X01-X03): 3 modifiers - Special effects
 * 
 * Total: 26 modifiers
 */
UENUM(BlueprintType)
enum class ECameraModifierID : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    
    //========================================
    // Shake Modifiers (S01-S05) - 5 modifiers
    //========================================
    Modifier_S01_Shake_Hit_Light UMETA(DisplayName = "S01: Hit Shake Light"),
    Modifier_S02_Shake_Hit_Heavy UMETA(DisplayName = "S02: Hit Shake Heavy"),
    Modifier_S03_Shake_Attack_Hit UMETA(DisplayName = "S03: Attack Shake"),
    Modifier_S04_Shake_Environment UMETA(DisplayName = "S04: Environment Shake"),
    Modifier_S05_Shake_Landing UMETA(DisplayName = "S05: Land Shake"),
    
    //========================================
    // Reaction Modifiers (R01-R06) - 6 modifiers
    //========================================
    Modifier_R01_Reaction_Parry UMETA(DisplayName = "R01: Parry React"),
    Modifier_R02_Reaction_PerfectParry UMETA(DisplayName = "R02: Perfect Parry React"),
    Modifier_R03_Reaction_Stagger UMETA(DisplayName = "R03: Stagger React"),
    Modifier_R04_Reaction_Knockback UMETA(DisplayName = "R04: Knockback React"),
    Modifier_R05_Reaction_Knockdown UMETA(DisplayName = "R05: Knockdown React"),
    Modifier_R06_Reaction_GuardBreak UMETA(DisplayName = "R06: Guard Break React"),
    
    //========================================
    // Cinematic Modifiers (C01-C05) - 5 modifiers
    //========================================
    Modifier_C01_Cinematic_Execution UMETA(DisplayName = "C01: Execution"),
    Modifier_C02_Cinematic_Backstab UMETA(DisplayName = "C02: Backstab"),
    Modifier_C03_Cinematic_Riposte UMETA(DisplayName = "C03: Riposte"),
    Modifier_C04_Cinematic_BossIntro UMETA(DisplayName = "C04: Boss Intro"),
    Modifier_C05_Cinematic_BossPhase UMETA(DisplayName = "C05: Boss Phase Change"),
    
    //========================================
    // Zoom Modifiers (Z01-Z04) - 4 modifiers
    //========================================
    Modifier_Z01_Zoom_AttackImpact UMETA(DisplayName = "Z01: Attack Impact Zoom"),
    Modifier_Z02_Zoom_ChargeRelease UMETA(DisplayName = "Z02: Charge Release Zoom"),
    Modifier_Z03_Zoom_SkillActivate UMETA(DisplayName = "Z03: Skill Activate Zoom"),
    Modifier_Z04_Zoom_CriticalHit UMETA(DisplayName = "Z04: Critical Hit Zoom"),
    
    //========================================
    // Effect Modifiers (E01-E03) - 3 modifiers
    //========================================
    Modifier_E01_Effect_LowHealth UMETA(DisplayName = "E01: Low Health Effect"),
    Modifier_E02_Effect_StatusAilment UMETA(DisplayName = "E02: Status Ailment Effect"),
    Modifier_E03_Effect_FocusMode UMETA(DisplayName = "E03: Focus Mode Effect"),
    
    //========================================
    // Special Modifiers (X01-X03) - 3 modifiers
    //========================================
    Modifier_X01_Special_SlowMotion UMETA(DisplayName = "X01: Slow Motion"),
    Modifier_X02_Special_HitStop UMETA(DisplayName = "X02: Hit Stop"),
    Modifier_X03_Special_DeathCam UMETA(DisplayName = "X03: Death Cam"),
    
    MAX UMETA(Hidden)
};

/**
 * Collision Category Enumeration
 * Defines the functional category of collision strategies
 */
UENUM(BlueprintType)
enum class ECollisionCategory : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    Detection UMETA(DisplayName = "Detection"),
    Response UMETA(DisplayName = "Response"),
    Occlusion UMETA(DisplayName = "Occlusion"),
    Recovery UMETA(DisplayName = "Recovery"),
    Special UMETA(DisplayName = "Special")
};

/**
 * Collision strategy ID types - 20 strategies total
 * Handles camera collision detection, response, occlusion and recovery
 * 
 * Categories:
 * - Detection (D01-D04): 4 strategies - Detect collisions
 * - Response (RS01-RS05): 5 strategies - Respond to collisions
 * - Occlusion (OC01-OC04): 4 strategies - Handle occlusion
 * - Recovery (RC01-RC03): 3 strategies - Recover from collision
 * - Special (SP01-SP04): 4 strategies - Special cases
 * 
 * Total: 20 strategies
 */
UENUM(BlueprintType)
enum class ECollisionStrategyID : uint8
{
    CollisionNone = 0 UMETA(DisplayName = "None"),
    
    //========================================
    // Detection Strategies (D01-D04) - 4 strategies
    //========================================
    Collision_D01_SingleRay UMETA(DisplayName = "D01: Single Ray"),
    Collision_D02_SphereSweep UMETA(DisplayName = "D02: Sphere Sweep"),
    Collision_D03_MultiRay UMETA(DisplayName = "D03: Multi Ray"),
    Collision_D04_MultiSphereSweep UMETA(DisplayName = "D04: Multi Sphere Sweep"),
    
    //========================================
    // Response Strategies (RS01-RS05) - 5 strategies
    //========================================
    Collision_RS01_PullIn UMETA(DisplayName = "RS01: Pull In"),
    Collision_RS02_Slide UMETA(DisplayName = "RS02: Slide"),
    Collision_RS03_Orbit UMETA(DisplayName = "RS03: Orbit"),
    Collision_RS04_FOVCompensate UMETA(DisplayName = "RS04: FOV Compensate"),
    Collision_RS05_InstantSnap UMETA(DisplayName = "RS05: Instant Snap"),
    
    //========================================
    // Occlusion Strategies (OC01-OC04) - 4 strategies
    //========================================
    Collision_OC01_FadeOut UMETA(DisplayName = "OC01: Fade Out"),
    Collision_OC02_Hide UMETA(DisplayName = "OC02: Hide"),
    Collision_OC03_PullInFurther UMETA(DisplayName = "OC03: Pull In Further"),
    Collision_OC04_DitherFade UMETA(DisplayName = "OC04: Dither Fade"),
    
    //========================================
    // Recovery Strategies (RC01-RC03) - 3 strategies
    //========================================
    Collision_RC01_DelayedRecovery UMETA(DisplayName = "RC01: Delayed Recovery"),
    Collision_RC02_SmoothRecovery UMETA(DisplayName = "RC02: Smooth Recovery"),
    Collision_RC03_StepRecovery UMETA(DisplayName = "RC03: Step Recovery"),
    
    //========================================
    // Special Strategies (SP01-SP04) - 4 strategies
    //========================================
    Collision_SP01_TightSpace UMETA(DisplayName = "SP01: Tight Space"),
    Collision_SP02_LowCeiling UMETA(DisplayName = "SP02: Low Ceiling"),
    Collision_SP03_CliffEdge UMETA(DisplayName = "SP03: Cliff Edge"),
    Collision_SP04_CornerCase UMETA(DisplayName = "SP04: Corner Case"),
    
    MAX UMETA(Hidden)
};
