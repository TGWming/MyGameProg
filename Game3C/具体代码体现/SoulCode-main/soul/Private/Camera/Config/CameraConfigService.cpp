// Copyright (c) 2024. All Rights Reserved.

/**
 * CameraConfigService.cpp
 * 
 * Implementation of the central configuration service for Souls-like camera system.
 * 魂类游戏相机系统中央配置服务的实现。
 */

#include "Camera/Config/CameraConfigService.h"
#include "Camera/Config/DefaultCameraParams.h"
#include "Camera/Data/CameraParamOverrideData.h"
#include "Camera/Data/CameraStateData.h"
#include "Engine/DataTable.h"

//========================================
// Constructor
//========================================

UCameraConfigService::UCameraConfigService()
    : StatesDataTable(nullptr)
    , OverridesDataTable(nullptr)
    , bIsInitialized(false)
{
}

//========================================
// Initialization
//========================================

bool UCameraConfigService::Initialize(UDataTable* InStatesDataTable, UDataTable* InOverridesDataTable)
{
    UE_LOG(LogTemp, Warning, TEXT("=== ConfigService::Initialize START ==="));
    UE_LOG(LogTemp, Warning, TEXT("  InStatesDataTable: %s"), InStatesDataTable ? *InStatesDataTable->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  InOverridesDataTable: %s"), InOverridesDataTable ? *InOverridesDataTable->GetName() : TEXT("NULL"));

    // Save DataTable references
    StatesDataTable = InStatesDataTable;
    OverridesDataTable = InOverridesDataTable;

    // Validate states DataTable
    if (!StatesDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("ConfigService::Initialize - StatesDataTable is required but null"));
        bIsInitialized = false;
        return false;
    }

    // 打印DataTable的行结构
    if (StatesDataTable->GetRowStruct())
    {
        UE_LOG(LogTemp, Warning, TEXT("  StatesDataTable RowStruct: %s"), *StatesDataTable->GetRowStruct()->GetName());
        
        // 检查是否是正确的结构
        bool bIsFCameraStateRow = StatesDataTable->GetRowStruct()->IsChildOf(FCameraStateRow::StaticStruct());
        UE_LOG(LogTemp, Warning, TEXT("  Is FCameraStateRow: %s"), bIsFCameraStateRow ? TEXT("YES") : TEXT("NO"));
        
        if (!bIsFCameraStateRow)
        {
            UE_LOG(LogTemp, Error, TEXT("  ERROR: DataTable is NOT using FCameraStateRow structure!"));
            UE_LOG(LogTemp, Error, TEXT("  Expected: FCameraStateRow, Got: %s"), *StatesDataTable->GetRowStruct()->GetName());
        }
    }

    // Load state definitions
    LoadStateDefinitions();

    // Load overrides (optional) - 只有当 OverridesDataTable 存在且不是同一个表时才加载
    if (OverridesDataTable && OverridesDataTable != StatesDataTable)
    {
        LoadOverrides();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("ConfigService::Initialize - Skipping overrides (none provided or same as states table)"));
    }

    // Mark as initialized
    bIsInitialized = true;

    UE_LOG(LogTemp, Warning, TEXT("=== ConfigService::Initialize END ==="));
    UE_LOG(LogTemp, Warning, TEXT("  States loaded: %d"), StateCategories.Num());
    UE_LOG(LogTemp, Warning, TEXT("  bIsInitialized: TRUE"));
    return true;
}

bool UCameraConfigService::IsInitialized() const
{
    return bIsInitialized;
}

void UCameraConfigService::ReloadData()
{
    UE_LOG(LogTemp, Log, TEXT("CameraConfigService::ReloadData - Reloading all data..."));

    // Clear all loaded data
    StateCategories.Empty();
    StatePriorities.Empty();
    AllOverrides.Empty();
    ConfigCache.Empty();

    // Reload data
    LoadStateDefinitions();
    LoadOverrides();

    UE_LOG(LogTemp, Log, TEXT("CameraConfigService::ReloadData - Reloaded %d states and %d states with overrides"),
        StateCategories.Num(), AllOverrides.Num());
}

//========================================
// Configuration Query
//========================================

bool UCameraConfigService::GetStateConfig(FName StateName, FCameraStateConfig& OutConfig) const
{
    // Check initialization
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraConfigService::GetStateConfig - Service not initialized"));
        return false;
    }

    // Find state category
    const ECameraStateCategory* CategoryPtr = StateCategories.Find(StateName);
    if (!CategoryPtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraConfigService::GetStateConfig - State '%s' not found"), *StateName.ToString());
        return false;
    }

    // Build complete configuration
    BuildCompleteConfig(StateName, *CategoryPtr, OutConfig);

    return true;
}

const FCameraStateConfig* UCameraConfigService::GetCachedStateConfig(FName StateName)
{
    // Check cache first
    FCameraStateConfig* CachedConfig = ConfigCache.Find(StateName);
    if (CachedConfig)
    {
        return CachedConfig;
    }

    // Not in cache, build and cache it
    FCameraStateConfig NewConfig;
    if (GetStateConfig(StateName, NewConfig))
    {
        // Add to cache
        ConfigCache.Add(StateName, NewConfig);
        return ConfigCache.Find(StateName);
    }

    // State not found
    return nullptr;
}

bool UCameraConfigService::DoesStateExist(FName StateName) const
{
    return StateCategories.Contains(StateName);
}

TArray<FName> UCameraConfigService::GetAllStateNames() const
{
    TArray<FName> StateNames;
    StateCategories.GetKeys(StateNames);
    return StateNames;
}

TArray<FName> UCameraConfigService::GetStateNamesInCategory(ECameraStateCategory Category) const
{
    TArray<FName> StateNames;
    
    for (const auto& Pair : StateCategories)
    {
        if (Pair.Value == Category)
        {
            StateNames.Add(Pair.Key);
        }
    }
    
    return StateNames;
}

//========================================
// State Metadata Query
//========================================

ECameraStateCategory UCameraConfigService::GetStateCategory(FName StateName) const
{
    const ECameraStateCategory* CategoryPtr = StateCategories.Find(StateName);
    if (CategoryPtr)
    {
        return *CategoryPtr;
    }
    
    // Default value if not found
    return ECameraStateCategory::FreeExploration;
}

int32 UCameraConfigService::GetStatePriority(FName StateName) const
{
    const int32* PriorityPtr = StatePriorities.Find(StateName);
    if (PriorityPtr)
    {
        return *PriorityPtr;
    }
    
    // Default priority if not found
    return 100;
}

//========================================
// Override Query
//========================================

bool UCameraConfigService::HasOverrides(FName StateName) const
{
    return AllOverrides.Contains(StateName);
}

int32 UCameraConfigService::GetOverrideCount(FName StateName) const
{
    const FStateParamOverrides* Overrides = AllOverrides.Find(StateName);
    if (Overrides)
    {
        return Overrides->GetOverrideCount();
    }
    
    return 0;
}

//========================================
// Cache Management
//========================================

void UCameraConfigService::ClearCache()
{
    int32 ClearedCount = ConfigCache.Num();
    ConfigCache.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("CameraConfigService::ClearCache - Cleared %d cached configurations"), ClearedCount);
}

void UCameraConfigService::PreCacheStates(const TArray<FName>& StateNames)
{
    int32 CachedCount = 0;
    
    for (const FName& StateName : StateNames)
    {
        if (GetCachedStateConfig(StateName) != nullptr)
        {
            CachedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CameraConfigService::PreCacheStates - Pre-cached %d/%d states"), 
        CachedCount, StateNames.Num());
}

//========================================
// Debug
//========================================

void UCameraConfigService::GetStatistics(int32& OutStateCount, int32& OutOverrideCount, int32& OutCachedCount) const
{
    OutStateCount = StateCategories.Num();
    
    // Count total overrides
    OutOverrideCount = 0;
    for (const auto& Pair : AllOverrides)
    {
        OutOverrideCount += Pair.Value.GetOverrideCount();
    }
    
    OutCachedCount = ConfigCache.Num();
}

void UCameraConfigService::LogAllData() const
{
    UE_LOG(LogTemp, Log, TEXT("========================================"));
    UE_LOG(LogTemp, Log, TEXT("CameraConfigService Data Dump"));
    UE_LOG(LogTemp, Log, TEXT("========================================"));
    
    // Log state definitions
    UE_LOG(LogTemp, Log, TEXT("--- State Definitions (%d total) ---"), StateCategories.Num());
    for (const auto& Pair : StateCategories)
    {
        const int32* PriorityPtr = StatePriorities.Find(Pair.Key);
        int32 Priority = PriorityPtr ? *PriorityPtr : 0;
        
        UE_LOG(LogTemp, Log, TEXT("  State: %s | Category: %d | Priority: %d"),
            *Pair.Key.ToString(), static_cast<int32>(Pair.Value), Priority);
    }
    
    // Log overrides
    UE_LOG(LogTemp, Log, TEXT("--- Overrides (%d states with overrides) ---"), AllOverrides.Num());
    for (const auto& Pair : AllOverrides)
    {
        UE_LOG(LogTemp, Log, TEXT("  State: %s | Override Count: %d"),
            *Pair.Key.ToString(), Pair.Value.GetOverrideCount());
        
        for (const FCameraParamOverrideRow& Override : Pair.Value.Overrides)
        {
            UE_LOG(LogTemp, Log, TEXT("    - Path: %s | Type: %d | Value: %s"),
                *Override.ParamPath, static_cast<int32>(Override.ValueType), *Override.ToString());
        }
    }
    
    // Log cache status
    UE_LOG(LogTemp, Log, TEXT("--- Cache Status ---"));
    UE_LOG(LogTemp, Log, TEXT("  Cached Configurations: %d"), ConfigCache.Num());
    
    UE_LOG(LogTemp, Log, TEXT("========================================"));
}

//========================================
// Internal Methods - Data Loading
//========================================

void UCameraConfigService::LoadStateDefinitions()
{
    StateCategories.Empty();
    StatePriorities.Empty();

    UE_LOG(LogTemp, Warning, TEXT("=== LoadStateDefinitions START ==="));

    if (!StatesDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadStateDefinitions - StatesDataTable is NULL"));
        return;
    }

    TArray<FName> RowNames = StatesDataTable->GetRowNames();
    UE_LOG(LogTemp, Warning, TEXT("LoadStateDefinitions - Total rows: %d"), RowNames.Num());

    int32 SuccessCount = 0;
    int32 FailCount = 0;

    for (const FName& RowName : RowNames)
    {
        const FCameraStateRow* Row = StatesDataTable->FindRow<FCameraStateRow>(RowName, TEXT("LoadStateDefinitions"));

        if (Row)
        {
            FName StateName = Row->StateName;
            if (StateName.IsNone())
            {
                StateName = RowName;
            }

            ECameraStateCategory Category = Row->Category;
            
            StateCategories.Add(StateName, Category);
            StatePriorities.Add(StateName, Row->Priority);

            SuccessCount++;

            // ★ 打印前5个状态的详细分类信息 ★
            if (SuccessCount <= 5)
            {
                UE_LOG(LogTemp, Warning, TEXT("  [%d] State: %s | Category: %d | Priority: %d"),
                    SuccessCount, *StateName.ToString(), static_cast<int32>(Category), Row->Priority);
            }
        }
        else
        {
            FailCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("=== LoadStateDefinitions END ==="));
    UE_LOG(LogTemp, Warning, TEXT("  Success: %d, Failed: %d"), SuccessCount, FailCount);
    
    // ★ 专门验证 Explore_Default 的分类 ★
    if (const ECameraStateCategory* Cat = StateCategories.Find(FName("Explore_Default")))
    {
        UE_LOG(LogTemp, Warning, TEXT("  >>> Explore_Default Category Value: %d"), static_cast<int32>(*Cat));
    }
}

void UCameraConfigService::LoadOverrides()
{
    AllOverrides.Empty();

    if (!OverridesDataTable)
    {
        UE_LOG(LogTemp, Log, TEXT("CameraConfigService::LoadOverrides - No overrides DataTable provided (this is OK)"));
        return;
    }

    // Use Helper function to load overrides
    AllOverrides = UCameraParamOverrideHelpers::LoadOverridesFromDataTable(OverridesDataTable);

    // Count total overrides for logging
    int32 TotalOverrides = 0;
    for (const auto& Pair : AllOverrides)
    {
        TotalOverrides += Pair.Value.GetOverrideCount();
    }

    UE_LOG(LogTemp, Log, TEXT("CameraConfigService::LoadOverrides - Loaded %d states with %d total overrides"),
        AllOverrides.Num(), TotalOverrides);
}

//========================================
// Internal Methods - Config Building
//========================================

void UCameraConfigService::BuildCompleteConfig(FName StateName, ECameraStateCategory Category, FCameraStateConfig& OutConfig) const
{
    // ★ Debug logging for Override flow ★
    // UE_LOG(LogTemp, Warning, TEXT("BuildCompleteConfig - Looking for overrides for state: %s"), *StateName.ToString());
    // UE_LOG(LogTemp, Warning, TEXT("BuildCompleteConfig - AllOverrides count: %d"), AllOverrides.Num());
    
    // 1. Get default config for this category
    OutConfig = UDefaultCameraParams::GetDefaultConfigForCategory(Category);
    
    // UE_LOG(LogTemp, Warning, TEXT("BuildCompleteConfig - Before override: Distance=%f, FOV=%f"), 
    //     OutConfig.StateBase.Distance.BaseDistance, OutConfig.StateBase.FOV.BaseFOV);

    // 2. Set state identity information
    OutConfig.StateBase.Identity.StateName = StateName;
    OutConfig.StateBase.Identity.Category = Category;

    // Set priority from loaded data
    const int32* PriorityPtr = StatePriorities.Find(StateName);
    if (PriorityPtr)
    {
        OutConfig.StateBase.Identity.Priority = *PriorityPtr;
    }

    // 3. Find overrides for this state
    const FStateParamOverrides* Overrides = AllOverrides.Find(StateName);
    
    // ★ Debug logging for Override lookup result ★
    // if (Overrides)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("BuildCompleteConfig - Found %d overrides for %s"), Overrides->GetOverrideCount(), *StateName.ToString());
    // }
    // else
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("BuildCompleteConfig - NO overrides found for %s"), *StateName.ToString());
    // }

    // 4. Apply overrides if any exist
    if (Overrides && Overrides->GetOverrideCount() > 0)
    {
        ApplyOverridesToConfig(*Overrides, OutConfig);
        // UE_LOG(LogTemp, Warning, TEXT("BuildCompleteConfig - After override: Distance=%f, FOV=%f"), 
        //     OutConfig.StateBase.Distance.BaseDistance, OutConfig.StateBase.FOV.BaseFOV);
    }

    // 诊断日志：确认 Build 完成后的 FramingParams
    // 只在 FramingParams 发生变化时输出日志
    {
        static FString LastFramingLog;
        FString CurrentFramingLog = FString::Printf(TEXT("ConfigService::BuildCompleteConfig for '%s' - Framing: bEnable=%s, AnchorX=%.2f, AnchorY=%.2f, PlayerW=%.2f, EnemyW=%.2f"),
            *StateName.ToString(),
            OutConfig.FramingParams.bEnableFraming ? TEXT("true") : TEXT("false"),
            OutConfig.FramingParams.FramingAnchorX,
            OutConfig.FramingParams.FramingAnchorY,
            OutConfig.FramingParams.PlayerWeight,
            OutConfig.FramingParams.EnemyWeight);
        if (CurrentFramingLog != LastFramingLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s"), *CurrentFramingLog);
            LastFramingLog = CurrentFramingLog;
        }
    }
}

void UCameraConfigService::ApplyOverridesToConfig(const FStateParamOverrides& Overrides, FCameraStateConfig& OutConfig) const
{
    for (const FCameraParamOverrideRow& Override : Overrides.Overrides)
    {
        if (Override.IsValid())
        {
            ApplySingleOverride(Override, OutConfig);
        }
    }
}

void UCameraConfigService::ApplySingleOverride(const FCameraParamOverrideRow& Override, FCameraStateConfig& OutConfig) const
{
    // ★ Debug logging for each override application ★
    // UE_LOG(LogTemp, Warning, TEXT("ApplySingleOverride - Applying: %s = %f (Type: %d)"), 
    //     *Override.ParamPath, Override.FloatValue, (int32)Override.ValueType);
    
    const FString& Path = Override.ParamPath;

    // Parse path into components
    TArray<FString> Components = UCameraParamOverrideHelpers::ParseParamPath(Path);

    if (Components.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraConfigService::ApplySingleOverride - Invalid param path: %s"), *Path);
        return;
    }

    // Get top-level category
    const FString& Category = Components[0];

    //========================================
    // StateBase Parameters
    //========================================
    if (Category == TEXT("StateBase"))
    {
        if (Components.Num() >= 3)
        {
            const FString& Group = Components[1];
            const FString& Param = Components[2];

            // Distance group
            if (Group == TEXT("Distance"))
            {
                if (Param == TEXT("BaseDistance") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Distance.BaseDistance = Override.FloatValue;
                }
                else if (Param == TEXT("MinDistance") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Distance.MinDistance = Override.FloatValue;
                }
                else if (Param == TEXT("MaxDistance") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Distance.MaxDistance = Override.FloatValue;
                }
            }
            // FOV group
            else if (Group == TEXT("FOV"))
            {
                if (Param == TEXT("BaseFOV") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.FOV.BaseFOV = Override.FloatValue;
                }
                else if (Param == TEXT("MinFOV") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.FOV.MinFOV = Override.FloatValue;
                }
                else if (Param == TEXT("MaxFOV") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.FOV.MaxFOV = Override.FloatValue;
                }
            }
            // Rotation group
            else if (Group == TEXT("Rotation"))
            {
                if (Param == TEXT("MinPitch") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Rotation.MinPitch = Override.FloatValue;
                }
                else if (Param == TEXT("MaxPitch") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Rotation.MaxPitch = Override.FloatValue;
                }
            }
            // Offset group
            else if (Group == TEXT("Offset"))
            {
                if (Param == TEXT("FocusOffset") && Override.ValueType == EParamValueType::Vector)
                {
                    OutConfig.StateBase.Offset.FocusOffset = Override.VectorValue;
                }
                else if (Param == TEXT("SocketOffset") && Override.ValueType == EParamValueType::Vector)
                {
                    OutConfig.StateBase.Offset.SocketOffset = Override.VectorValue;
                }
                else if (Param == TEXT("VerticalOffset") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Offset.VerticalOffset = Override.FloatValue;
                }
                else if (Param == TEXT("ShoulderOffset") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Offset.ShoulderOffset = Override.FloatValue;
                }
            }
            // Lag group
            else if (Group == TEXT("Lag"))
            {
                if (Param == TEXT("PositionLagSpeed") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Lag.PositionLagSpeed = Override.FloatValue;
                }
                else if (Param == TEXT("RotationLagSpeed") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Lag.RotationLagSpeed = Override.FloatValue;
                }
                else if (Param == TEXT("DistanceLagSpeed") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Lag.DistanceLagSpeed = Override.FloatValue;
                }
                else if (Param == TEXT("FOVLagSpeed") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Lag.FOVLagSpeed = Override.FloatValue;
                }
            }
            // Transition group
            else if (Group == TEXT("Transition"))
            {
                if (Param == TEXT("BlendInTime") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Transition.BlendInTime = Override.FloatValue;
                }
                else if (Param == TEXT("BlendOutTime") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Transition.BlendOutTime = Override.FloatValue;
                }
                else if (Param == TEXT("BlendType") && Override.ValueType == EParamValueType::BlendType)
                {
                    OutConfig.StateBase.Transition.BlendType = Override.BlendTypeValue;
                }
            }
            // Collision group
            else if (Group == TEXT("Collision"))
            {
                if (Param == TEXT("bEnableCollision") && Override.ValueType == EParamValueType::Bool)
                {
                    OutConfig.StateBase.Collision.bEnableCollision = Override.BoolValue;
                }
                else if (Param == TEXT("CollisionRadius") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Collision.CollisionRadius = Override.FloatValue;
                }
                else if (Param == TEXT("RecoveryDelay") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.StateBase.Collision.RecoveryDelay = Override.FloatValue;
                }
            }
            // Flags group
            else if (Group == TEXT("Flags"))
            {
                if (Param == TEXT("bRequiresTarget") && Override.ValueType == EParamValueType::Bool)
                {
                    OutConfig.StateBase.Flags.bRequiresTarget = Override.BoolValue;
                }
                else if (Param == TEXT("bIsCinematic") && Override.ValueType == EParamValueType::Bool)
                {
                    OutConfig.StateBase.Flags.bIsCinematic = Override.BoolValue;
                }
                else if (Param == TEXT("bIgnoreInput") && Override.ValueType == EParamValueType::Bool)
                {
                    OutConfig.StateBase.Flags.bIgnoreInput = Override.BoolValue;
                }
            }
            // Identity group
            else if (Group == TEXT("Identity"))
            {
                if (Param == TEXT("Priority") && Override.ValueType == EParamValueType::Int)
                {
                    OutConfig.StateBase.Identity.Priority = Override.IntValue;
                }
            }
        }
    }
    //========================================
    // Module Parameters
    //========================================
    else if (Category == TEXT("Module"))
    {
        if (Components.Num() >= 3)
        {
            const FString& Group = Components[1];
            const FString& Param = Components[2];

            if (Group == TEXT("Position"))
            {
                if (Param == TEXT("PredictionTime") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.Module.Position.PredictionTime = Override.FloatValue;
                }
            }
            else if (Group == TEXT("Rotation"))
            {
                if (Param == TEXT("InputSensitivity") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.Module.Rotation.InputSensitivity = Override.FloatValue;
                }
                else if (Param == TEXT("SoftLockStrength") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.Module.Rotation.SoftLockStrength = Override.FloatValue;
                }
            }
        }
    }
    //========================================
    // Modifier Parameters
    //========================================
    else if (Category == TEXT("Modifier"))
    {
        if (Components.Num() >= 3)
        {
            const FString& Group = Components[1];
            const FString& Param = Components[2];

            if (Group == TEXT("Shake"))
            {
                if (Param == TEXT("LightHitAmplitude") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.Modifier.Shake.LightHitAmplitude = Override.FloatValue;
                }
                else if (Param == TEXT("HeavyHitAmplitude") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.Modifier.Shake.HeavyHitAmplitude = Override.FloatValue;
                }
            }
        }
    }
    //========================================
    // Collision Parameters
    //========================================
    else if (Category == TEXT("Collision"))
    {
        if (Components.Num() >= 3)
        {
            const FString& Group = Components[1];
            const FString& Param = Components[2];

            if (Group == TEXT("Detection"))
            {
                if (Param == TEXT("ProbeRadius") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.Collision.Detection.ProbeRadius = Override.FloatValue;
                }
            }
            else if (Group == TEXT("Response"))
            {
                if (Param == TEXT("PullInSpeed") && Override.ValueType == EParamValueType::Float)
                {
                    OutConfig.Collision.Response.PullInSpeed = Override.FloatValue;
                }
            }
        }
    }
    //========================================
    // FramingParams Parameters
    //========================================
    else if (Category == TEXT("FramingParams"))
    {
        // FramingParams 只有两级路径：FramingParams.ParamName
        // Components[1] 直接是参数名
        FString ParamName = Components.Num() > 1 ? Components[1] : TEXT("");

        // [Disabled] BuildCompleteConfig 的变化检测日志已包含最终参数值，逐条 override 日志无额外信息量
        // UE_LOG(LogTemp, Warning, TEXT("ConfigService: Applying FramingParams override - ParamName='%s', FloatValue=%.2f, BoolValue=%s"),
        //     *ParamName, Override.FloatValue, Override.BoolValue ? TEXT("true") : TEXT("false"));

        if (ParamName == TEXT("bEnableFraming"))
        {
            if (Override.ValueType == EParamValueType::Bool)
            {
                OutConfig.FramingParams.bEnableFraming = Override.BoolValue;
            }
        }
        else if (ParamName == TEXT("FramingAnchorX"))
        {
            if (Override.ValueType == EParamValueType::Float)
            {
                OutConfig.FramingParams.FramingAnchorX = Override.FloatValue;
            }
        }
        else if (ParamName == TEXT("FramingAnchorY"))
        {
            if (Override.ValueType == EParamValueType::Float)
            {
                OutConfig.FramingParams.FramingAnchorY = Override.FloatValue;
            }
        }
        else if (ParamName == TEXT("PlayerWeight"))
        {
            if (Override.ValueType == EParamValueType::Float)
            {
                OutConfig.FramingParams.PlayerWeight = Override.FloatValue;
            }
        }
        else if (ParamName == TEXT("EnemyWeight"))
        {
            if (Override.ValueType == EParamValueType::Float)
            {
                OutConfig.FramingParams.EnemyWeight = Override.FloatValue;
            }
        }
        else if (ParamName == TEXT("VerticalBias"))
        {
            if (Override.ValueType == EParamValueType::Float)
            {
                OutConfig.FramingParams.VerticalBias = Override.FloatValue;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ApplySingleOverride: Unknown FramingParams param '%s'"), *ParamName);
        }
    }

    // Uncomment for verbose logging of applied overrides
    // UE_LOG(LogTemp, Verbose, TEXT("CameraConfigService::ApplySingleOverride - Applied override %s"), *Path);
}
