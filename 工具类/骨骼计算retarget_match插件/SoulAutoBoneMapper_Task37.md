# 任务 37: 注册编辑器窗口 — 修改模块实现文件

```
===== 任务 37: 注册编辑器窗口 — 修改模块实现文件 =====

目标: 修改 SoulAutoBoneMapper.cpp，实现窗口注册、菜单项、Tab 生成

操作类型: 仅修改已有文件
引擎版本: UE4.27

安全约束:
- 仅修改 SoulAutoBoneMapper.cpp，禁止修改其他文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

将文件 Plugins/SoulAutoBoneMapper/Source/SoulAutoBoneMapper/Private/SoulAutoBoneMapper.cpp 的整个内容替换为:

#include "SoulAutoBoneMapper.h"
#include "SBoneMappingWidget.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FSoulAutoBoneMapperModule"

static const FName BoneMapperTabName("SoulAutoBoneMapperTab");

void FSoulAutoBoneMapperModule::StartupModule()
{
    // 注册 Tab Spawner
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        BoneMapperTabName,
        FOnSpawnTab::CreateRaw(this, &FSoulAutoBoneMapperModule::OnSpawnTab))
        .SetDisplayName(LOCTEXT("TabTitle", "Soul Auto Bone Mapper"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    // 注册菜单
    RegisterMenuExtension();
}

void FSoulAutoBoneMapperModule::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BoneMapperTabName);
}

void FSoulAutoBoneMapperModule::RegisterMenuExtension()
{
    // 在 Window 菜单下添加入口
    FLevelEditorModule& LevelEditorModule =
        FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
    MenuExtender->AddMenuExtension(
        "WindowLayout",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateLambda([](FMenuBuilder& Builder)
        {
            Builder.AddMenuEntry(
                LOCTEXT("MenuEntry", "Soul Auto Bone Mapper"),
                LOCTEXT("MenuEntryTooltip", "Open the automatic bone mapping tool"),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([]()
                {
                    FGlobalTabmanager::Get()->TryInvokeTab(BoneMapperTabName);
                }))
            );
        })
    );

    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FSoulAutoBoneMapperModule::OnMenuButtonClicked()
{
    FGlobalTabmanager::Get()->TryInvokeTab(BoneMapperTabName);
}

TSharedRef<SDockTab> FSoulAutoBoneMapperModule::OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs)
{
    return SNew(SDockTab)
        .TabRole(NomadTab)
        [
            SNew(SBoneMappingWidget)
        ];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSoulAutoBoneMapperModule, SoulAutoBoneMapper)

完成后汇报: 确认 SoulAutoBoneMapper.cpp 是否已更新，包含:
  1. StartupModule 中注册 Tab + 菜单
  2. ShutdownModule 中注销 Tab
  3. RegisterMenuExtension 在 Window 菜单下添加入口
  4. OnSpawnTab 生成 SBoneMappingWidget

===== 任务 37 结束 =====
```