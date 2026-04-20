# UE4.27 项目问题排查与修复总结

> **日期**：2026-04-20  
> **引擎版本**：Unreal Engine 4.27  
> **场景**：Build Light + Cook Content for Windows  
> **结果**：✅ 打包成功，运行正常

---

## 📋 目录

1. [问题 1：Landscape Instanced Meshes LOD Lightmap 警告](#问题-1landscape-instanced-meshes-lod-lightmap-警告)
2. [问题 2：Landscape Thumbnail Shader 警告](#问题-2landscape-thumbnail-shader-警告)
3. [问题 3：Cook 时蓝图 Accessed None 警告（BookGenerator）](#问题-3cook-时蓝图-accessed-none-警告bookgenerator)
4. [问题 4：蓝图组件 Mobility 不匹配警告](#问题-4蓝图组件-mobility-不匹配警告)
5. [通用知识点速查](#通用知识点速查)
6. [今日心得 & 排查方法论](#今日心得--排查方法论)

---

## 问题 1：Landscape Instanced Meshes LOD Lightmap 警告

### 🔎 警告原文
```
Landscape_0 Instanced meshes don't yet support unique static lighting for each LOD. 
Lighting on LOD 1+ may be incorrect unless lightmap UVs are the same for all LODs.
```
（后续还出现了 `Landscape_1` 相同警告）

### 📖 含义
实例化网格体（Instanced Static Mesh / HISM，典型场景：Foliage 植被）不支持为每个 LOD 烘焙独立的静态光照，所有 LOD 会共用 LOD0 的 Lightmap。  
如果各 LOD 的 **Lightmap UV（UV1）** 布局不一致，LOD1+ 的光照会出现错位。

### ⚠️ 容易踩的坑
- 警告挂在 `Landscape_X` 名下，但**真正的肇事者是地形上刷的 Foliage 植被**，不是地形本身。
- 去 Landscape Actor 的 Details → LOD Settings 找修复点是**错的**，那里是地形组件自身的 LOD 配置。
- **Auto LOD（自动 LOD）** 是典型触发原因：引擎减面时会自动重新生成每个 LOD 的 UV，导致 UV 布局不一致。

### 🧠 技术原理
UE 中 UV 通道的标准约定：
| UV 通道 | 用途 |
|--------|------|
| **UV0** | 贴图 UV（Diffuse / Normal / Roughness 等材质贴图） |
| **UV1** | Lightmap UV（光照贴图，Lightmass 烘焙默认读取这个通道） |
| UV2+ | 特殊用途 |

Static Mesh Editor → Build Settings 中的关键参数：
```
Generate Lightmap UVs       ☑   是否让引擎自动生成 Lightmap UV
Source Lightmap Index       = 0  以哪个 UV 通道作为生成参考
Destination Lightmap Index  = 1  把生成的 Lightmap UV 放到哪个通道（UE 默认读 UV1）
Min Lightmap Resolution     = 64 植被建议值（默认 16 太低）
```

### 🔧 修复方法（按推荐程度）

#### ✅ 方案 A：修正 Destination Lightmap Index（本项目实际使用）
打开问题植被的 Static Mesh → **LOD0 → Build Settings**：
| 参数 | 值 |
|------|-----|
| Generate Lightmap UVs | ☑ |
| Source Lightmap Index | **0** |
| Destination Lightmap Index | **1**（原本被设成了 2，导致 UE 烘焙时找不到正确 UV） |

点 **Apply Changes** → 保存资产 → 重新 **Build Lighting Only**。

#### ✅ 方案 B：使用 LOD Group = Foliage
**LOD Settings → LOD Group** 选择 `Foliage`，引擎自动套用植被的光照/减面最优预设。

#### ✅ 方案 C：减少 LOD 数量
**LOD Settings → Number of LODs** 改为 1 或 2，植被一般不需要 4 级 LOD。

#### ✅ 方案 D：手动导入各级 LOD 模型
在 DCC 软件里做好 LOD1/2/3，保证每个 LOD 的 UV1 布局一致，再通过 `LOD Import` 分别导入。最彻底但最麻烦。

#### ✅ 方案 E：直接忽略
Warning 不是 Error，远距离植被 Lightmap 错位人眼几乎看不出来。

### 💡 关键发现（本项目）
- **LOD1/2/3 没有独立的 Build Settings** 是正常的，因为它们是**从 LOD0 自动减面生成的**，只有 Reduction Settings（控制减面比例）。
- 所以"让每个 LOD 单独改 Build Settings"这条路 UI 上走不通，**只能在 LOD0 上统一配置**（配合 Auto LOD 会自动应用到下级 LOD）。

### 🎯 如何定位问题 Mesh
1. 在 Outliner 选中 `Landscape_X`
2. 切换到 **Foliage 模式（Shift+3）**
3. 左侧列出所有 Foliage Type，记下各自引用的 Static Mesh
4. 同时检查 Landscape Material 里是否用了 **Landscape Grass Type**
5. 对这些 Mesh 逐个处理

---

## 问题 2：Landscape Thumbnail Shader 警告

### 🔎 警告原文
```
Shader FVLMVoxelizationPS unknown by landscape thumbnail material, 
please add to either AllowedShaderTypes or ExcludedShaderTypes

Shader FLightmapGBufferVS unknown by landscape thumbnail material, 
please add to either AllowedShaderTypes or ExcludedShaderTypes
```

### 📖 含义
地形缩略图（Thumbnail）材质维护着一个 Shader 白/黑名单，引擎遇到**未在列表中的新 Shader 类型**时，不知道是否需要为缩略图编译，于是提醒开发者显式声明。

涉及的 Shader：
- `FVLMVoxelizationPS` — Volumetric Lightmap 体素化用的 Pixel Shader
- `FLightmapGBufferVS` — Lightmap GBuffer 用的 Vertex Shader

### 🧠 触发原因
- 使用了第三方光照/GI/体积渲染插件
- 源码版 UE 做了自定义修改
- 项目启用了 Volumetric Lightmap 相关功能
- 从旧版本升级后新增了 Shader 类型

### ⚠️ 严重程度：可忽略
- ✅ 不影响编译、打包、运行、烘焙结果
- ✅ 只影响 Editor 里地形材质的缩略图预览
- ✅ 最多多编译一点无关紧要的 Shader

### 🔧 处理方法

#### ✅ 方案 A：直接忽略（推荐）

#### ✅ 方案 B：抑制日志输出
在 `DefaultEngine.ini` 添加：
```ini
[Core.Log]
LogLandscape=Error
```
注意：会屏蔽所有 LogLandscape 的 Warning。

#### ✅ 方案 C：修改引擎源码（仅限源码版）
修改文件：
```
Engine/Source/Runtime/Landscape/Private/LandscapeRender.cpp
```
在 `ExcludedShaderTypes` 列表中添加：
```cpp
ExcludedShaderTypes.Add(TEXT("FVLMVoxelizationPS"));
ExcludedShaderTypes.Add(TEXT("FLightmapGBufferVS"));
```
然后重新编译引擎。

---

## 问题 3：Cook 时蓝图 Accessed None 警告（BookGenerator）

### 🔎 警告原文
```
LogScript: Warning: Accessed None trying to read property CallFunc_Map_Find_Value_2
Script call stack:
    Function BP_BookGenerator_C:UserConstructionScript
    Function BP_BookGenerator_C:SetupBookColors
    位置：SetupBookColors:196F
出错实例：
    BP_BookGenerator_PilePrefab4 / 5_10 / 6 / 7 / _2 ... (多个)
关卡：/Game/Levels/SpawnedLocation
```

### 📖 含义
蓝图执行到 Map → Find 节点时，Key 在 Map 中**找不到对应项**，返回了 `None`，但后续节点**没做判空**就直接访问 → 抛出 "Accessed None"。

解读变量名 `CallFunc_Map_Find_Value_2`：
- `CallFunc_` = 蓝图编译器自动生成的内部变量
- `Map_Find` = 调用的是 Map 容器的 Find 函数
- `Value` = Find 的 Value 输出
- `_2` = 该函数在蓝图中第 2 次被调用的实例

### 🧠 推测逻辑
```
Construction Script
  └─► SetupBookColors
       └─► [BookColorMap] (Map<Key, Color/Material>)
            └���► Find (Key = 某个书的ID/类型)
                 └─► Value ──► 直接赋给材质/颜色  ❌ 未判空
```

### ⚠️ 严重程度：中（不阻塞打包，但影响视觉）
| 项目 | 影响 |
|------|------|
| Cook 完成 | ✅ 不阻塞 |
| 打包成功 | ✅ 不阻塞 |
| 运行时崩溃 | ❌ 不会（引擎用默认值继续） |
| 视觉表现 | ⚠️ 部分书堆的书颜色/材质可能是默认值而非预期值 |
| 日志污染 | ⚠️ 同警告刷了几十次，掩盖其他问题 |

### 🔧 修复方法

#### ✅ 步骤 1：定位节点
1. 打开 `/Game/Gothic_Interior/Environment/Blueprint/BookGenerator/BP_BookGenerator`
2. My Blueprint → Functions → 双击 **`SetupBookColors`**
3. 找到里面的 Map → Find 节点

#### ✅ 步骤 2：加空值判断（三选一）

**方案 A：Contains 先判断（最推荐）**
```
Map ──Contains──► Branch
        ▲ Key       │
                    ├─True──► Find ──► Value ──► [使用]
                    └─False─► [跳过 / 使用默认值]
```

**方案 B：IsValid（针对 Object/Class 类型 Value）**
```
Find ──► IsValid ──► Branch
                       ├─True──► [使用 Value]
                       └─False─► [跳过]
```

**方案 C：补全 Map 数据**
检查所有 `BP_BookGenerator_PilePrefab` 实例用到的 Key，把缺失的 Key 补到 Map 里。

#### ✅ 步骤 3：检查实例变量
打开 `/Game/Levels/SpawnedLocation`，选中所有 `BP_BookGenerator_PilePrefab` 实例，检查 Details 面板暴露的变量（书的颜色 ID、类型、Variant 等）是否有被设成 None / 不存在的索引。

### 🛠 开启蓝图运行时警告断点（排查利器）
```
Edit → Editor Preferences → General → Blueprints → Show Runtime Script Warnings ☑
```
开启后 PIE 运行触发警告时，编辑器会**自动跳到出错节点**，非常直观。

### 📂 完整 Cook 日志位置
```
[项目目录]/Saved/Logs/[项目名].log
```
搜索 `Accessed None`，上下文有完整调用栈信息。

---

## 问题 4：蓝图组件 Mobility 不匹配警告

### 🔎 警告原文
```
AttachTo: 'BP_ThickCandle_01c.DefaultSceneRoot' is not static, 
cannot attach 'PointLight / BloomSphere / CandleFire / StaticMeshComponent3' 
which is static to it. Aborting.

AttachTo: 'P_Steam_LitBoss_fogdoor2_2.ParticleSystemComponent0' is not static, 
cannot attach 'P_Steam_LitBoss_fogdoor2_2.rw' which is static to it. Aborting.
```

### 📖 含义
UE 不允许把 **Static（静态）组件** 挂在 **Movable（可移动）父组件** 下。  
当蓝图的 DefaultSceneRoot / 父组件是 Movable，而子组件（光源、StaticMesh、粒子等）被标记为 Static 时，**附加过程被中止**，这些子组件可能不会正确显示。

### 🧠 Mobility 规则速查
| 父 Mobility | 允许的子 Mobility |
|------------|------------------|
| Static | Static / Stationary / Movable |
| Stationary | Stationary / Movable |
| **Movable** | **只允许 Movable**（不能挂 Static / Stationary） |

### ⚠️ 后果
- 蜡烛的光源 / 火焰特效 / 网格体可能看不见
- Boss 雾门粒子效果可能不完整

### 🔧 修复方法
打开对应蓝图 → 选中组件 → Details → **Transform → Mobility**：

**方案 A**：把父组件（DefaultSceneRoot）改为 **Static**（前提：该 Actor 运行时不移动）  
**方案 B**：把所有子组件改为 **Movable**（代价：光照不能烘焙，改用动态光照）

根据 Actor 的实际用途选择：
- 蜡烛 `BP_ThickCandle_01c` 一般不动 → 建议把 Root 改 Static
- 雾门 `P_Steam_LitBoss_fogdoor` 可能有动画 → 建议全 Movable

---

## 通用知识点速查

### 🗂 UE 日志分级
| 级别 | 含义 | 是否阻塞 |
|------|------|---------|
| **Error** | 严重错误 | ⛔ 通常阻塞 |
| **Warning** | 警告 | ✅ 不阻塞，但提示潜在问题 |
| **Display** | 常规信息 | ✅ 正常输出 |
| **Log / Verbose** | 详细调试 | ✅ 默认不显示 |

### 🗂 UE UV 通道标准约定
| UV 通道 | 用途 |
|--------|------|
| UV0 | 贴图 UV |
| **UV1** | **Lightmap UV（UE 默认读取）** |
| UV2+ | 特殊用途 / 自定义 |

### 🗂 Static Mesh LOD 两种生成方式
| 方式 | 特点 | 是否有独立 Build Settings |
|------|------|--------------------------|
| **Auto LOD** | 引擎自动减面 | ❌ 只有 Reduction Settings |
| **Manual LOD Import** | DCC 软件手动做好再导入 | ✅ 每个 LOD 独立 Build Settings |

### 🗂 Mobility 三档
| Mobility | 可否移动 | 光照处理 | 性能 |
|----------|---------|---------|------|
| Static | ❌ | 完全烘焙 | 最好 |
| Stationary | 有限（光强/颜色变化） | 部分烘焙 | 中等 |
| Movable | ✅ 任意移动 | 完全动态 | 最差 |

### 🗂 常用排查位置
| 需求 | 位置 |
|------|------|
| 完整日志 | `[项目]/Saved/Logs/*.log` |
| Cook 输出 | `[项目]/Saved/Cooked/WindowsNoEditor/` |
| 蓝图运行时警告开关 | Editor Preferences → Blueprints → Show Runtime Script Warnings |
| 抑制特定日志 | `DefaultEngine.ini → [Core.Log]` |

---

## 今日心得 & 排查方法论

### ✨ 经验总结

1. **警告归属 ≠ 问题所在**  
   Landscape_X 报 Instanced Mesh 警告，真正问题在植被 Mesh 资产上，不在地形本身。

2. **Auto LOD 是很多 Lightmap 问题的根源**  
   自动减面会重新生成 UV，容易破坏 Lightmap UV 的一致性。植被建议用 LOD Group = Foliage。

3. **UV1 是 UE 的默认 Lightmap 通道，别改**  
   `Destination Lightmap Index` 永远设成 1，除非你非常清楚自己在做什么。

4. **蓝图 Map Find 永远要判空**  
   养成习惯：`Contains` 判断 → `Find` 取值 → `IsValid` 验证（视类型）。

5. **Mobility 要从父到子保持兼容**  
   Movable 父级不能挂 Static 子组件，一个蓝图内部最好统一 Mobility。

### 🔍 排查方法论（本次实战验证有效）

#### 方法 1：时间戳对齐法
在 `Saved/Logs/*.log` 里，**按时间戳查看警告前后的日志**，往往能看到关联的蓝图名/资产名/实例名。本次定位 BookGenerator 就是靠这个方法。

#### 方法 2：Message Log 链接点击
Message Log 里的警告通常带链接，点一下可以**直接跳到对应 Actor / 资产**。

#### 方法 3：PIE + Runtime Warnings 断点
开启 `Show Runtime Script Warnings`，PIE 跑一遍，编辑器会**自动跳到出错蓝图节点**。

#### 方法 4：警告严重性分级处理
| 场景 | 策略 |
|------|------|
| 紧急打包 | 先忽略所有 Warning，确保 Cook 成功 |
| 正式发布前 | 优先修 Error、影响视觉的 Warning、污染日志的 Warning |
| 日常开发 | 养成习惯：新增蓝图逻辑时主动加 IsValid / Contains 判断 |

---

## 📌 本次项目遗留的可选优化

这些都不紧急，有空再处理：

| 优先级 | 问题 | 影响 | 建议动作 |
|-------|------|------|---------|
| 🟡 中 | BP_BookGenerator → SetupBookColors 的 Map Find 未判空 | 部分书堆颜色可能异常 | 加 Contains/IsValid |
| 🟡 中 | BP_ThickCandle_01c 子组件 Mobility 冲突 | 蜡烛光源/火焰可能不显示 | 统一 Mobility |
| 🟡 中 | P_Steam_LitBoss_fogdoor 组件 Mobility 冲突 | 雾门效果可能不全 | 统一 Mobility |
| 🟢 低 | Landscape Thumbnail Shader 警告 | 仅编辑器缩略图 | 忽略或屏蔽日志 |
| 🟢 低 | Landscape_1 剩余的 Lightmap 警告（若还在） | 远距离植被光照 | 参照问题 1 处理 |

---

## 📚 参考速查卡

### Static Mesh Lightmap 标准配置
```
LOD0 → Build Settings:
  Generate Lightmap UVs       = ☑
  Source Lightmap Index       = 0
  Destination Lightmap Index  = 1   ← 关键！必须是 1
  Min Lightmap Resolution     = 64  (植被推荐)
```

### 蓝图 Map 访问安全模板
```
[Map] ──► Contains (Key) ──► Branch
                              ├─True──► Find (Key) ──► [使用 Value]
                              └─False─► [跳过 / 默认值 / 日志提示]
```

### Mobility 兼容性
```
Static  父 ──► ✅ Static / Stationary / Movable 子
Stationary父 ──► ✅ Stationary / Movable 子 （❌ Static）
Movable 父 ──► ✅ 只能 Movable 子          （❌ Static / Stationary）
```

---

**文档结束**  
如后续再遇到类似问题，可以回来对照本清单逐项排查。