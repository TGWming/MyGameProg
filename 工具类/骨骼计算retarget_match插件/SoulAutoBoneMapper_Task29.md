# 任务 29: 实现 MappingPreset — LoadFromJsonFile

```
===== 任务 29: 实现 MappingPreset — LoadFromJsonFile =====

目标: 在 MappingPreset.cpp 末尾追加 LoadFromJsonFile 方法实现

操作类型: 仅追加代码到已有文件末尾
引擎版本: UE4.27

安全约束:
- 禁止删除或修改 MappingPreset.cpp 中已有的函数
- 禁止删除或修改项目中任何其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

在文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/MappingPreset.cpp 末尾追加以下代码:

bool FMappingPreset::LoadFromJsonFile(const FString& FilePath)
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("MappingPreset: Failed to load file: %s"), *FilePath);
        return false;
    }

    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MappingPreset: Failed to parse JSON: %s"), *FilePath);
        return false;
    }

    PresetName = Root->GetStringField(TEXT("PresetName"));
    SourceSkeletonName = Root->GetStringField(TEXT("SourceSkeleton"));
    TargetSkeletonName = Root->GetStringField(TEXT("TargetSkeleton"));

    Mappings.Empty();

    const TArray<TSharedPtr<FJsonValue>>* MappingArray;
    if (Root->TryGetArrayField(TEXT("Mappings"), MappingArray))
    {
        for (const auto& Value : *MappingArray)
        {
            const TSharedPtr<FJsonObject>& EntryObj = Value->AsObject();
            if (!EntryObj.IsValid())
            {
                continue;
            }

            FBoneMappingEntry Entry;
            Entry.SourceBoneName = FName(*EntryObj->GetStringField(TEXT("Source")));
            Entry.TargetBoneName = FName(*EntryObj->GetStringField(TEXT("Target")));
            Entry.Confidence = EntryObj->GetNumberField(TEXT("Confidence"));
            Entry.MatchType = StringToMatchType(EntryObj->GetStringField(TEXT("MatchType")));
            Entry.bManuallyOverridden = EntryObj->GetBoolField(TEXT("ManualOverride"));

            Mappings.Add(Entry);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MappingPreset: Loaded %d mappings from %s"), Mappings.Num(), *FilePath);
    return true;
}

完成后汇报: 确认 LoadFromJsonFile 是否已追加到 MappingPreset.cpp 末尾，
且已有函数未被修改。至此 MappingPreset 全部方法实现完毕。

===== 任务 29 结束 =====
```