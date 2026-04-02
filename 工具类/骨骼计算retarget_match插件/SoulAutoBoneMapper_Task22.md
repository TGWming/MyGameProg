# 任务 22: 实现 BoneAliasTable — LoadFromJsonFile + 辅助方法

```
===== 任务 22: 实现 BoneAliasTable — LoadFromJsonFile + 辅助方法 =====

目标: 创建 BoneAliasTable.cpp，实现 JSON 加载和基础方法

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/BoneAliasTable.cpp 内容如下:

#include "BoneAliasTable.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Interfaces/IPluginManager.h"

bool FBoneAliasTable::LoadFromJsonFile(const FString& FilePath)
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneAliasTable: Failed to load JSON file: %s"), *FilePath);
        return false;
    }

    TSharedPtr<FJsonObject> JsonRoot;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BoneAliasTable: Failed to parse JSON file: %s"), *FilePath);
        return false;
    }

    for (const auto& Pair : JsonRoot->Values)
    {
        const FName StandardName = FName(*Pair.Key.ToLower());

        const TArray<TSharedPtr<FJsonValue>>* AliasArray;
        if (Pair.Value->TryGetArray(AliasArray))
        {
            TArray<FName>& Aliases = StandardToAliases.FindOrAdd(StandardName);

            for (const auto& AliasValue : *AliasArray)
            {
                FString AliasStr;
                if (AliasValue->TryGetString(AliasStr))
                {
                    Aliases.AddUnique(FName(*AliasStr.ToLower()));
                }
            }
        }
    }

    RebuildReverseIndex();

    UE_LOG(LogTemp, Log, TEXT("BoneAliasTable: Loaded %d standard bones, %d total aliases from %s"),
        StandardToAliases.Num(), GetTotalAliasCount(), *FilePath);

    return true;
}

bool FBoneAliasTable::LoadDefaultAliases()
{
    const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("SoulAutoBoneMapper"))->GetBaseDir();
    const FString DefaultPath = FPaths::Combine(PluginDir, TEXT("Resources"), TEXT("DefaultAliases.json"));

    return LoadFromJsonFile(DefaultPath);
}

void FBoneAliasTable::AddAlias(const FName& StandardBoneName, const FName& Alias)
{
    const FName LowerStandard = FName(*StandardBoneName.ToString().ToLower());
    const FName LowerAlias = FName(*Alias.ToString().ToLower());

    StandardToAliases.FindOrAdd(LowerStandard).AddUnique(LowerAlias);
    AliasToStandard.Add(LowerAlias, LowerStandard);
}

void FBoneAliasTable::Clear()
{
    StandardToAliases.Empty();
    AliasToStandard.Empty();
}

int32 FBoneAliasTable::GetTotalAliasCount() const
{
    int32 Total = 0;
    for (const auto& Pair : StandardToAliases)
    {
        Total += Pair.Value.Num();
    }
    return Total;
}

void FBoneAliasTable::RebuildReverseIndex()
{
    AliasToStandard.Empty();
    for (const auto& Pair : StandardToAliases)
    {
        for (const FName& Alias : Pair.Value)
        {
            AliasToStandard.Add(Alias, Pair.Key);
        }
    }
}

完成后汇报: 确认 BoneAliasTable.cpp 是否已创建，包含:
  1. LoadFromJsonFile（JSON 解析 + 填充 Map）
  2. LoadDefaultAliases（插件目录定位）
  3. AddAlias / Clear / GetTotalAliasCount
  4. RebuildReverseIndex

===== 任务 22 结束 =====
```