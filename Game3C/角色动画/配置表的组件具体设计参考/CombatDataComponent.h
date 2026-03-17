#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Animation/AnimMontage.h"
#include "CombatDataComponent.generated.h"

// ============================================================
// Montage 配置行 —— 对应总表 Category = Montage 的每一行
// ============================================================
USTRUCT(BlueprintType)
struct FMontageConfigRow : public FTableRowBase
{
    GENERATED_BODY()

    // 子分类: Roll / LightAtk / HeavyAtk / HitReact / Death...
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SubCategory;
    
    // 所属角色: Player / Grunt / Elite / FinalBoss...
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Owner;
    
    // Montage 名（用于日志和调试）
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString MontageName;
    
    // 动作类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ActionType;
    
    // 时长
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 0.0f;
    
    // 混合参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendIn = 0.05f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BlendOut = 0.20f;
    
    // Root Motion
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseRootMotion = false;
    
    // 伤害（字符串，支持多段如 "30+33+40"）
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Damage;
    
    // 精力消耗
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float StaminaCost = 0.0f;
    
    // Boss 阶段
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Phase;
    
    // 体型
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SizeCategory;
    
    // ★ 真正的 Montage 资产引用（在 DataTable 编辑器里关联）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> MontageAsset;
    
    // 备注
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Notes;
};

// ============================================================
// 角色属性行 —— 对应总表 Category = CharStats 的每一行
// ============================================================
USTRUCT(BlueprintType)
struct FCharacterStatsRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Owner;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxHealth = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Poise = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float PoiseRegenRate = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoveSpeed_Combat = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoveSpeed_Chase = 400.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackRange = 200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DetectRange = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Phase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SizeCategory;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Notes;
};

// ============================================================
// 核心组件 —— 挂到任何角色上，通过 OwnerID 自动筛选数据
// ============================================================
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUL_API UCombatDataComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatDataComponent();

    // ========================================
    // ★ 在蓝图 Details 面板里配置这两个东西
    // ========================================
    
    /** 
     * 这个角色的身份标识
     * 必须和 CSV 总表里的 Owner 列完全一致
     * 
     * 例如：
     *   玩家蓝图填 "Player"
     *   小兵蓝图填 "Grunt"
     *   精英蓝图填 "Elite"
     *   最终Boss填 "FinalBoss"
     *   宝箱怪填   "Mimic"
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Data")
    FString OwnerID = "Player";

    /** 
     * Montage 配置总表（所有角色共用同一张）
     * 在蓝图里把 DT_MontageConfig 拖进来
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Data")
    UDataTable* MontageConfigTable;

    /**
     * 角色属性配置表（所有角色共用同一张）
     * 在蓝图里把 DT_CharacterStats 拖进来
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Data")
    UDataTable* CharacterStatsTable;

    // ========================================
    // ★ 核心查表函数
    // ========================================
    
    /**
     * 通过 RowName 查 Montage 配置
     * 
     * @param RowName  CSV 里的 ID 列，如 "M001"
     * @return         该行的完整配置，查不到返回 nullptr
     */
    UFUNCTION(BlueprintCallable, Category = "Combat Data")
    FMontageConfigRow* GetMontageConfig(FName RowName);

    /**
     * 通过 SubCategory 查属于本角色的 Montage 配置
     * 自动用 OwnerID 筛选，不用手动传 Owner
     * 
     * @param SubCategory  如 "Roll" / "LightAtk" / "HeavyAtk"
     * @param Phase        Boss 阶段筛选（留空 = 不筛选）
     * @return             匹配的所有行
     * 
     * 举例：
     *   小兵调用 FindMontagesBySubCategory("LightAtk")
     *   → 自动只返回 Owner=="Grunt" && SubCategory=="LightAtk" 的行
     *   → 得到 AM_Grunt_LightAtk_01 和 AM_Grunt_LightAtk_02
     */
    UFUNCTION(BlueprintCallable, Category = "Combat Data")
    TArray<FMontageConfigRow> FindMontagesBySubCategory(
        const FString& SubCategory, 
        const FString& Phase = TEXT(""));

    /**
     * 通过 ActionType 查属于本角色的 Montage
     * 
     * @param ActionType  如 "Attack" / "HitReact" / "Death"
     * @return            匹配的所有行
     */
    UFUNCTION(BlueprintCallable, Category = "Combat Data")
    TArray<FMontageConfigRow> FindMontagesByActionType(const FString& ActionType);

    /**
     * 播放指定 RowName 的 Montage
     * 内部自动处理：查表 → 加载资产 → 播放
     * 
     * @param RowName     CSV 里的 ID 列
     * @param PlayRate    播放速率（默认 1.0）
     * @return            播放时长，失败返回 0
     */
    UFUNCTION(BlueprintCallable, Category = "Combat Data")
    float PlayMontageByRowName(FName RowName, float PlayRate = 1.0f);

    /**
     * 获取本角色的属性配置
     * 自动用 OwnerID + Phase 筛选
     */
    UFUNCTION(BlueprintCallable, Category = "Combat Data")
    FCharacterStatsRow* GetCharacterStats(const FString& Phase = TEXT(""));

    /**
     * 解析伤害字符串
     * "30+33+40" → [30.0, 33.0, 40.0]
     * "50~70"    → [60.0]（取中间值，或你可以改成随机）
     */
    UFUNCTION(BlueprintCallable, Category = "Combat Data")
    TArray<float> ParseDamageString(const FString& DamageStr);

protected:
    virtual void BeginPlay() override;

private:
    // 缓存：BeginPlay 时预筛选出属于本 OwnerID 的所有行
    // 避免每次查表都遍历全表
    TMap<FName, FMontageConfigRow> CachedMontageRows;
    
    // 构建缓存
    void BuildCache();
};