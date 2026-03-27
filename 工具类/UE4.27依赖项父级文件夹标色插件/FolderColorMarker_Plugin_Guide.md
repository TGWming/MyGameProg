# FolderColorMarker 插件 - 完整搭建指南

> **目标：** 在 UE4.27 项目中创建一个 Editor-Only 的 C++ 插件模块 `soulEditor`，  
> 提供 Blueprint Function Library，用于在 Content Browser 中标记被你自己关卡/蓝图引用的资源包文件夹为绿色。

---

## 前置条件

- Unreal Engine 4.27
- 项目名称：`soul`（如不同请全局替换）
- 已有游戏模块 `soul`（Source/soul/）
- 项目根目录：`E:\MyProject\soul\`（如不同请替换路径）

---

## STEP 1：创建 soulEditor 模块目录结构

> **指令给 Agent：**

```
在 Source/ 目录下创建 soulEditor 模块的目录结构：

创建以下空目录（如果不存在）：
- Source/soulEditor/
- Source/soulEditor/Public/
- Source/soulEditor/Private/

### 安全约束：
- 不要修改 Source/soul/ 下的任何文件
- 不要修改 .uproject 文件
- 只创建目录结构
```

---

## STEP 2：创建 soulEditor.Build.cs

> **指令给 Agent：**

```
创建文件 Source/soulEditor/soulEditor.Build.cs，内容如下：

using UnrealBuildTool;

public class soulEditor : ModuleRules
{
    public soulEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "AssetRegistry"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "EditorStyle"
        });
    }
}

### 安全约束：
- 只创建这一个文件
- 不要修改其他任何文件
```

---

## STEP 3：创建模块启动文件 soulEditorModule.cpp

> **指令给 Agent：**

```
创建文件 Source/soulEditor/Private/soulEditorModule.cpp，内容如下：

#include "Modules/ModuleManager.h"

class FsoulEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FsoulEditorModule, soulEditor)

### 安全约束：
- 只创建这一个文件
- 不要修改其他任何文件
```

---

## STEP 4：注册 soulEditor 模块到 .uproject

> **指令给 Agent：**

```
打开 soul.uproject 文件，在 "Modules" 数组中添加一个新条目：

{
    "Name": "soulEditor",
    "Type": "Editor",
    "LoadingPhase": "PostEngineInit"
}

注意：
- 不要删除或修改已有的 "soul" 模块条目
- 只在 Modules 数组末尾追加这个新条目
- 确保 JSON 格式正确（逗号分隔）

### 安全约束：
- 只修改 soul.uproject 的 Modules 数组
- 保留文件中所有其他内容不变
- 不要修改任何其他文件
```

---

## STEP 5：创建 Editor Target 文件（如不存在）

> **指令给 Agent：**

```
检查 Source/soulEditor.Target.cs 是否存在。

如果不存在，创建文件 Source/soulEditor.Target.cs，内容如下：

using UnrealBuildTool;
using System.Collections.Generic;

public class soulEditorTarget : TargetRules
{
    public soulEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new string[] { "soul", "soulEditor" });
    }
}

如果已存在，检查 ExtraModuleNames 中是否包含 "soulEditor"，
如果没有则添加。

### 安全约束：
- 如果文件已存在，只修改 ExtraModuleNames 这一行
- 不要修改 Source/soul.Target.cs（这是游戏 Target，不是 Editor Target）
- 不要修改其他任何文件
```

---

## STEP 6：创建 FolderColorMarkerBFL 头文件

> **指令给 Agent：**

```
创建文件 Source/soulEditor/Public/FolderColorMarkerBFL.h，内容如下：

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FolderColorMarkerBFL.generated.h"

UCLASS()
class SOULEDITOR_API UFolderColorMarkerBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "FolderColor", meta = (CallInEditor = "true"))
    static void MarkReferencedFolders();

    UFUNCTION(BlueprintCallable, Category = "FolderColor", meta = (CallInEditor = "true"))
    static void MarkReferencedFoldersWithColor(FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "FolderColor", meta = (CallInEditor = "true"))
    static void ClearAllFolderColors();
};

### 安全约束：
- 只创建这一个文件
- 不要修改其他任何文件
```

---

## STEP 7：创建 FolderColorMarkerBFL 实现文件

> **指令给 Agent：**

```
创建文件 Source/soulEditor/Private/FolderColorMarkerBFL.cpp，内容如下：

#include "FolderColorMarkerBFL.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ConfigCacheIni.h"

// ========== 配置区域 ==========
// 你自己开发的内容所在的顶级文件夹（/Game/ 下的直接子文件夹）
// 如需添加新文件夹，在这里追加即可
static const TArray<FString> DevelopedRootFolders = {
    TEXT("/Game/blueprint"),
    TEXT("/Game/Data"),
    TEXT("/Game/DataTables"),
    TEXT("/Game/Levels"),
    TEXT("/Game/LoadingScreen"),
    TEXT("/Game/MergeActors"),
    TEXT("/Game/ThirdPerson"),
    TEXT("/Game/ThirdPersonBP"),
    TEXT("/Game/CharactersType")
};
// ========== 配置区域结束 ==========

// 判断一个资产是否在你自己开发的文件夹下
static bool IsInDevelopedFolder(const FString& PackagePath)
{
    for (const FString& Root : DevelopedRootFolders)
    {
        if (PackagePath == Root || PackagePath.StartsWith(Root + TEXT("/")))
        {
            return true;
        }
    }
    return false;
}

// 收集文件夹到 /Game/ 第一级子文件夹之间的所有层级（不包括 /Game 本身）
static void CollectFolderHierarchy(const FString& FolderPath, TSet<FString>& OutFolders)
{
    FString Current = FolderPath;
    while (!Current.IsEmpty() && Current != TEXT("/Game") && Current != TEXT("/"))
    {
        OutFolders.Add(Current);
        int32 LastSlash = INDEX_NONE;
        Current.FindLastChar(TEXT('/'), LastSlash);
        if (LastSlash > 0)
        {
            Current = Current.Left(LastSlash);
        }
        else
        {
            break;
        }
    }
}

void UFolderColorMarkerBFL::MarkReferencedFolders()
{
    MarkReferencedFoldersWithColor(FLinearColor(0.1f, 0.8f, 0.2f, 1.0f));
}

void UFolderColorMarkerBFL::ClearAllFolderColors()
{
    FAssetRegistryModule& AssetRegistryModule =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> AllAssets;
    AssetRegistry.GetAssetsByPath(FName("/Game"), AllAssets, true);

    TSet<FString> AllGameFolders;
    for (const FAssetData& Asset : AllAssets)
    {
        FString PackagePath = Asset.PackagePath.ToString();
        if (PackagePath.StartsWith(TEXT("/Game/")))
        {
            CollectFolderHierarchy(PackagePath, AllGameFolders);
        }
    }

    int32 ClearedCount = 0;
    for (const FString& Folder : AllGameFolders)
    {
        FString ExistingValue;
        if (GConfig->GetString(TEXT("PathColor"), *Folder, ExistingValue, GEditorPerProjectIni))
        {
            GConfig->RemoveKey(TEXT("PathColor"), *Folder, GEditorPerProjectIni);
            ClearedCount++;
        }
    }

    GConfig->Flush(false, GEditorPerProjectIni);

    UE_LOG(LogTemp, Warning, TEXT("[FolderColorMarker] Cleared %d folder colors (scanned %d folders)"), ClearedCount, AllGameFolders.Num());
    UE_LOG(LogTemp, Warning, TEXT("[FolderColorMarker] Please restart editor to see changes"));
}

void UFolderColorMarkerBFL::MarkReferencedFoldersWithColor(FLinearColor Color)
{
    // 1. 获取 AssetRegistry
    FAssetRegistryModule& AssetRegistryModule =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    AssetRegistry.SearchAllAssets(true);

    // 2. 获取 /Game/ 下所有资产
    TArray<FAssetData> AllAssets;
    AssetRegistry.GetAssetsByPath(FName("/Game"), AllAssets, true);

    UE_LOG(LogTemp, Warning, TEXT("[FolderColorMarker] Scanning %d total assets..."), AllAssets.Num());

    // 3. 只从你自己开发的文件夹中找资产作为入口
    TArray<FAssetData> DevelopedAssets;
    for (const FAssetData& Asset : AllAssets)
    {
        FString PackagePath = Asset.PackagePath.ToString();
        if (IsInDevelopedFolder(PackagePath))
        {
            DevelopedAssets.Add(Asset);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[FolderColorMarker] Found %d assets in your developed folders"), DevelopedAssets.Num());

    // 4. 从入口资产出发，收集它们依赖的所有资产的文件夹
    TSet<FString> ReferencedFolders;

    for (const FAssetData& DevAsset : DevelopedAssets)
    {
        // 标记开发资产自身所在的文件夹
        FString DevFolder = DevAsset.PackagePath.ToString();
        if (DevFolder.StartsWith(TEXT("/Game/")) && DevFolder != TEXT("/Game"))
        {
            ReferencedFolders.Add(DevFolder);
        }

        // 追踪这个资产依赖了哪些其他资产
        TArray<FName> Dependencies;
        AssetRegistry.GetDependencies(DevAsset.PackageName, Dependencies);

        for (const FName& DepName : Dependencies)
        {
            FString DepPath = DepName.ToString();

            // 只处理 /Game/ 下的依赖（排除引擎资产）
            if (!DepPath.StartsWith(TEXT("/Game/")))
            {
                continue;
            }

            // 从包路径提取文件夹路径
            int32 LastSlash;
            if (DepPath.FindLastChar(TEXT('/'), LastSlash) && LastSlash > 0)
            {
                FString FolderPath = DepPath.Left(LastSlash);
                if (FolderPath != TEXT("/Game"))
                {
                    ReferencedFolders.Add(FolderPath);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[FolderColorMarker] Found %d referenced folders"), ReferencedFolders.Num());

    // 5. 向上追溯所有中间层级文件夹（包括顶级，不包括 /Game）
    TSet<FString> AllFoldersToMark;
    for (const FString& Folder : ReferencedFolders)
    {
        CollectFolderHierarchy(Folder, AllFoldersToMark);
    }

    // 6. 写入颜色到 Config
    FString ColorStr = FString::Printf(TEXT("(R=%f,G=%f,B=%f,A=%f)"),
        Color.R, Color.G, Color.B, Color.A);

    for (const FString& FolderPath : AllFoldersToMark)
    {
        GConfig->SetString(
            TEXT("PathColor"),
            *FolderPath,
            *ColorStr,
            GEditorPerProjectIni
        );
    }

    GConfig->Flush(false, GEditorPerProjectIni);

    UE_LOG(LogTemp, Warning,
        TEXT("[FolderColorMarker] Done! From %d developed assets, marked %d folders"),
        DevelopedAssets.Num(), AllFoldersToMark.Num());
}

### 安全约束：
- 只创建这一个文件
- 不要修改其他任何文件
```

---

## STEP 8：编译项目

> **指令给 Agent：**

```
编译项目，报告编译结果。

### 安全约束：
- 不要修改任何源代码
- 如果编译失败，只报告完整错误信息，不要自行修复
```

---

## STEP 9：在编辑器中创建 UI 工具

此步骤在 UE 编辑器中手动操作（非 Agent 指令）：

1. 打开 UE 编辑器（如果提示 rebuild 选 Yes）
2. 确认 **Edit → Plugins → Editor Scripting Utilities** 已启用
3. Content Browser 中进入任意文件夹
4. 在右侧空白区域右键 → **Editor Utilities → Editor Utility Widget**
5. 命名�� `FolderColorTools`
6. 双击打开，在 Designer 中添加 2 个按钮：
   - 按钮 1：变量名 `MarkFolder`，显示文本 `Mark Folders`
   - 按钮 2：变量名 `ClearMark`，显示文本 `Clear Mark`
7. 切换到 **Graph** 视图
8. 选中 MarkFolder 按钮 → Details → Events → OnClicked 点 `+`
9. 右键搜索 `Mark Referenced Folders` → 连接到事件
10. 选中 ClearMark 按钮 → OnClicked → 搜索 `Clear All Folder Colors` → 连接
11. 编译 + 保存
12. Content Browser 中右键 FolderColorTools → **Run Editor Utility Widget**

---

## 使用说明

- **Mark Folders**：扫描你开发文件夹中所有资产的依赖关系，将引用到的资源包文件夹（含所有父级）标记为绿色
- **Clear Mark**：清除所有颜色标记（需要重启编辑器或重新打开 Content Browser 生效）
- 颜色标记只在 Content Browser **右侧主面板**显示，左侧树状面板不支持（UE4 限制）

---

## 自定义开发文件夹

如果你的项目增加了新的开发文件夹，编辑 `FolderColorMarkerBFL.cpp` 顶部的 `DevelopedRootFolders` 数组：

```cpp
static const TArray<FString> DevelopedRootFolders = {
    TEXT("/Game/blueprint"),
    TEXT("/Game/Data"),
    // ... 在这里添加新文件夹 ...
    TEXT("/Game/NewFolder")
};
```

---

## 故障排除

| 问题 | 解决方案 |
|------|---------|
| 编辑器启动提示 module 'soul' could not be loaded | 删除 `Intermediate/` 和 `Binaries/` 文件夹，重新打开项目 |
| 右键看不到 Editor Utility Widget | 确认 Editor Scripting Utilities 插件已启用，在文件夹内空白区域右键 |
| 蓝图中搜不到 Mark Referenced Folders | 确认 soulEditor 模块编译成功，重启编辑器 |
| 清除颜色不生效 | 重启编辑器后生效 |
| 编译报 .lib 文件找不到 | 删除 `Intermediate/` 和 `Binaries/`，重新编译 |