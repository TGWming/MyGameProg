# 任务 28: 实现 MappingPreset — SaveToJsonFile

```
===== 任务 28: 实现 MappingPreset — SaveToJsonFile =====

目标: 创建 MappingPreset.cpp，实现 SaveToJsonFile 方法

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删���或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/MappingPreset.cpp 内容如下:

#include "MappingPreset.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

static FString MatchTypeToString(EBoneMatchType Type)
{
    switch (Type)
    {
        case EBoneMatchType::Exact:           return TEXT("Exact");
        case EBoneMatchType::CaseInsensitive: return TEXT("CaseInsensitive");
        case EBoneMatchType::Alias:           return TEXT("Alias");
        case EBoneMatchType::Fuzzy:           return TEXT("Fuzzy");
        case EBoneMatchType::Structure:       return TEXT("Structure");
        case EBoneMatchType::Position:        return TEXT("Position");
        case EBoneMatchType::Manual:          return TEXT("Manual");
        default:                              return TEXT("Unmatched");
    }
}

static EBoneMatchType StringToMatchType(const FString& Str)
{
    if (Str == TEXT("Exact"))           return EBoneMatchType::Exact;
    if (Str == TEXT("CaseInsensitive")) return EBoneMatchType::CaseInsensitive;
    if (Str == TEXT("Alias"))           return EBoneMatchType::Alias;
    if (Str == TEXT("Fuzzy"))           return EBoneMatchType::Fuzzy;
    if (Str == TEXT("Structure"))       return EBoneMatchType::Structure;
    if (Str == TEXT("Position"))        return EBoneMatchType::Position;
    if (Str == TEXT("Manual"))          return EBoneMatchType::Manual;
    return EBoneMatchType::Unmatched;
}

bool FMappingPreset::SaveToJsonFile(const FString& FilePath) const
{
    TSharedRef<FJsonObject> Root = MakeShareable(new FJsonObject());

    Root->SetStringField(TEXT("PresetName"), PresetName);
    Root->SetStringField(TEXT("SourceSkeleton"), SourceSkeletonName);
    Root->SetStringField(TEXT("TargetSkeleton"), TargetSkeletonName);

    TArray<TSharedPtr<FJsonValue>> MappingArray;
    for (const FBoneMappingEntry& Entry : Mappings)
    {
        TSharedRef<FJsonObject> EntryObj = MakeShareable(new FJsonObject());
        EntryObj->SetStringField(TEXT("Source"), Entry.SourceBoneName.ToString());
        EntryObj->SetStringField(TEXT("Target"), Entry.TargetBoneName.ToString());
        EntryObj->SetNumberField(TEXT("Confidence"), Entry.Confidence);
        EntryObj->SetStringField(TEXT("MatchType"), MatchTypeToString(Entry.MatchType));
        EntryObj->SetBoolField(TEXT("ManualOverride"), Entry.bManuallyOverridden);

        MappingArray.Add(MakeShareable(new FJsonValueObject(EntryObj)));
    }

    Root->SetArrayField(TEXT("Mappings"), MappingArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

    if (!FJsonSerializer::Serialize(Root, Writer))
    {
        UE_LOG(LogTemp, Warning, TEXT("MappingPreset: Failed to serialize preset"));
        return false;
    }

    if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("MappingPreset: Failed to save file: %s"), *FilePath);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("MappingPreset: Saved %d mappings to %s"), Mappings.Num(), *FilePath);
    return true;
}

完成后汇报: 确认 MappingPreset.cpp 是否已创建，包含:
  1. MatchTypeToString / StringToMatchType 辅助函��
  2. SaveToJsonFile 完整实现

===== 任务 28 结束 =====
```