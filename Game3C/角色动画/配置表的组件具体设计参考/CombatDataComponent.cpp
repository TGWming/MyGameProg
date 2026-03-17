#include "CombatDataComponent.h"
#include "GameFramework/Character.h"

UCombatDataComponent::UCombatDataComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatDataComponent::BeginPlay()
{
    Super::BeginPlay();
    BuildCache();
}

// ================================================================
// 核心：BeginPlay 时把总表里属于本角色的行缓存起来
// 之后查表就是查 Map，O(1) 速度
// ================================================================
void UCombatDataComponent::BuildCache()
{
    CachedMontageRows.Empty();
    
    if (!MontageConfigTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatData] %s: MontageConfigTable is null!"), 
            *GetOwner()->GetName());
        return;
    }

    // 遍历总表的每一行
    const TMap<FName, uint8*>& RowMap = MontageConfigTable->GetRowMap();
    
    for (auto& Pair : RowMap)
    {
        FMontageConfigRow* Row = reinterpret_cast<FMontageConfigRow*>(Pair.Value);
        if (!Row) continue;

        // ★ 关键筛选：只缓存 Owner 列 == 本组件的 OwnerID 的行
        if (Row->Owner == OwnerID)
        {
            CachedMontageRows.Add(Pair.Key, *Row);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CombatData] %s (Owner=%s): Cached %d montage rows"), 
        *GetOwner()->GetName(), *OwnerID, CachedMontageRows.Num());
}

// ================================================================
// 通过 RowName 直接查（最快）
// ================================================================
FMontageConfigRow* UCombatDataComponent::GetMontageConfig(FName RowName)
{
    FMontageConfigRow* Found = CachedMontageRows.Find(RowName);
    if (!Found)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatData] Row '%s' not found for Owner '%s'"), 
            *RowName.ToString(), *OwnerID);
    }
    return Found;
}

// ================================================================
// 通过 SubCategory 筛选（如 "Roll" → 返回所有翻滚行）
// ================================================================
TArray<FMontageConfigRow> UCombatDataComponent::FindMontagesBySubCategory(
    const FString& SubCategory, const FString& Phase)
{
    TArray<FMontageConfigRow> Results;
    
    for (auto& Pair : CachedMontageRows)
    {
        const FMontageConfigRow& Row = Pair.Value;
        
        if (Row.SubCategory == SubCategory)
        {
            // 如果传了 Phase，也要匹配
            if (Phase.IsEmpty() || Row.Phase == Phase)
            {
                Results.Add(Row);
            }
        }
    }
    
    return Results;
}

// ================================================================
// 通过 ActionType 筛选（如 "Death" → 返回所有死亡行）
// ================================================================
TArray<FMontageConfigRow> UCombatDataComponent::FindMontagesByActionType(
    const FString& ActionType)
{
    TArray<FMontageConfigRow> Results;
    
    for (auto& Pair : CachedMontageRows)
    {
        if (Pair.Value.ActionType == ActionType)
        {
            Results.Add(Pair.Value);
        }
    }
    
    return Results;
}

// ================================================================
// 一键播放：查表 + 加载 + 播放
// ================================================================
float UCombatDataComponent::PlayMontageByRowName(FName RowName, float PlayRate)
{
    FMontageConfigRow* Row = GetMontageConfig(RowName);
    if (!Row) return 0.0f;

    // 加载 Montage 资产
    UAnimMontage* Montage = Row->MontageAsset.LoadSynchronous();
    if (!Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatData] Montage asset is null for row '%s'"), 
            *RowName.ToString());
        return 0.0f;
    }

    // 获取拥有者角色
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return 0.0f;

    // 播放
    float Duration = OwnerCharacter->PlayAnimMontage(Montage, PlayRate);
    
    UE_LOG(LogTemp, Verbose, TEXT("[CombatData] Playing '%s' on '%s', duration=%.2f"), 
        *Row->MontageName, *OwnerCharacter->GetName(), Duration);
    
    return Duration;
}

// ================================================================
// 获取角色属性
// ================================================================
FCharacterStatsRow* UCombatDataComponent::GetCharacterStats(const FString& Phase)
{
    if (!CharacterStatsTable) return nullptr;

    const TMap<FName, uint8*>& RowMap = CharacterStatsTable->GetRowMap();
    
    for (auto& Pair : RowMap)
    {
        FCharacterStatsRow* Row = reinterpret_cast<FCharacterStatsRow*>(Pair.Value);
        if (!Row) continue;

        if (Row->Owner == OwnerID)
        {
            if (Phase.IsEmpty() || Row->Phase == Phase)
            {
                return Row;
            }
        }
    }
    
    return nullptr;
}

// ================================================================
// 解析伤害字符串
// ================================================================
TArray<float> UCombatDataComponent::ParseDamageString(const FString& DamageStr)
{
    TArray<float> Results;
    
    if (DamageStr.IsEmpty() || DamageStr == TEXT("—")) return Results;
    
    // "30+33+40" → ["30", "33", "40"]
    if (DamageStr.Contains(TEXT("+")))
    {
        TArray<FString> Parts;
        DamageStr.ParseIntoArray(Parts, TEXT("+"), true);
        for (const FString& Part : Parts)
        {
            Results.Add(FCString::Atof(*Part.TrimStartAndEnd()));
        }
    }
    // "50~70" → [60.0]
    else if (DamageStr.Contains(TEXT("~")))
    {
        TArray<FString> Parts;
        DamageStr.ParseIntoArray(Parts, TEXT("~"), true);
        if (Parts.Num() == 2)
        {
            float Min = FCString::Atof(*Parts[0].TrimStartAndEnd());
            float Max = FCString::Atof(*Parts[1].TrimStartAndEnd());
            Results.Add((Min + Max) / 2.0f);
        }
    }
    // "50" → [50.0]
    else
    {
        // 去掉可能的后缀如 "(AOE)" "(破防)"
        FString CleanStr = DamageStr;
        int32 ParenIdx;
        if (CleanStr.FindChar('(', ParenIdx))
        {
            CleanStr = CleanStr.Left(ParenIdx).TrimStartAndEnd();
        }
        float Val = FCString::Atof(*CleanStr);
        if (Val > 0.0f) Results.Add(Val);
    }
    
    return Results;
}