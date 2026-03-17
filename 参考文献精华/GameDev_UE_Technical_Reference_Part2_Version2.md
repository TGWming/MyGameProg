# 🎮 游戏开发 UE 技术参考文档 — 第二部分（Repo 文档精华）

> **来源**：https://github.com/xiaxia9/Game-Development-Notes
> **注意**：本部分是对第一部分的补充，不重复已整理的内容

---

## 目录（第二部分）

- [十、一图看懂移动同步（上）— 普通移动](#十一图看懂移动同步上-普通移动)
- [十一、一图看懂移动同步（下）— RootMotion 移动](#十一一图看懂移动同步下-rootmotion-移动)
- [十二、GAS 技能系统介绍 — 核心概念与同步流程](#十二gas-技能系统介绍-核心概念与同步流程)
- [十三、游戏开发中的 Data Normalization](#十三游戏开发中的-data-normalization)
- [十四、IK 逆向运动学详解（索引）](#十四ik-逆向运动学详解索引)

---

# 十、一图看懂移动同步（上）— 普通移动

> **原文**：https://zhuanlan.zhihu.com/p/432303699
> **流程图**：[PNG](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】一图看懂移动同步（上）/【UE4】一图看懂移动同步（上）.png) | [PDF](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】一图看懂移动同步（上）/【UE4】一图看懂移动同步（上）.pdf) | [Visio](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】一图看懂移动同步（上）/【UE4】一图看懂移动同步（上）.vsdx)

本篇配合第六章（移动网络同步）和第七章（移动组件详解）一起阅读效果最佳。

## 10.1 移动输入设置流程

```
Step 1: Project Setting → Input → Axis Mappings（设置输入映射）
Step 2: BindAxis() 将映射值绑定到响应函数
Step 3: 响应函数调用 AddMovementInput() + AddControllerYawInput()
```

## 10.2 核心数据结构详解

### FSavedMove_Character — 一次移动的快照

| 字段 | 说明 |
|---|---|
| `TimeStamp` | 来自 `FNetworkPredictionData_Client_Character::CurrentTimeStamp` |
| 加速度等 | 同步加速度，**不��步 Input 值** |
| 用途 | 客户端发给服务器做模拟校验 |

### FNetworkPredictionData_Client_Character — 客户端预测数据

| 字段 | 说明 |
|---|---|
| `ClientUpdateTime` | 上次发送 ServerMove() 的时间 |
| `CurrentTimeStamp` | 每帧累加 DeltaTime，超阈值重置 |
| `SavedMoves` | 已模拟未 Ack 的移动缓存（从旧到新） |
| `LastAckedMove` | 上次被确认的移动 |
| `PendingMove` | 等待与下次合并的移动（减少带宽） |
| `IsImportantMove()` | MovementMode 或 Acceleration 变化 → 需重发 |

### 网络传输数据结构

```
客户端 → 服务器:
  FCharacterServerMovePackedBits（序列化后的比特流）
    → FCharacterNetworkMoveData（单次移动数据）
    → FCharacterNetworkMoveDataContainer
       ├── NewMoveData
       ├── PendingMoveData
       └── OldMoveData

服务器 → 客户端:
  FCharacterMoveResponseDataContainer（校验响应）
```

## 10.3 扩展移动同步实践指南 ⭐

### 客户端扩展（4 步）

```
1. 继承 FSavedMove_Character
   → 重写 PostUpdate() 赋值自定义变量
   → 重写 Clear() 清理变量

2. 重写 FNetworkPredictionData_Client_Character::AllocateNewMove()
   → return new 自定义 SavedMove

3. 继承 FCharacterNetworkMoveData
   → 重写 ClientFillNetworkMoveData()（填���数据）
   → 重写 Serialize()（序列化）

4. 继承 FCharacterNetworkMoveDataContainer
   → 关联自定义 MoveData
   → 构造函数中调用 SetNetworkMoveDataContainer()
```

### 服务器扩展

```
继承 FCharacterMoveResponseDataContainer
→ 重写 ServerFillResponseData()
→ 重写 Serialize()
→ 构造函数中调用 SetMoveResponseDataContainer()
```

### 模拟端扩展

```
继承 ACharacter
→ 重写 GetLifetimeReplicatedProps()（添加同步变量）
→ 重写 PreReplication()（对同步变量赋值）
```

---

# 十一、一图看懂移动同步（下）— RootMotion 移动

> **原文**：https://zhuanlan.zhihu.com/p/434494875
> **流程图**：[PNG](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】一图看懂移动同步（下）/【UE4】一图看懂移动同步（下）.png) | [PDF](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】一图看懂移动同步（下）/【UE4】一图看懂移动同步（下）.pdf) | [Visio](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】一图看懂移动同步（下）/【UE4】一图看懂移动同步（下）.vsdx)

## 11.1 两种移动驱动方式

| 方式 | 特点 | 缺陷 |
|---|---|---|
| **Code Driven** | 胶囊体推动，动画仅做表现层 | 移动模型与动画不匹配时**滑步** |
| **Data Driven（RootMotion）** | 从动画提取位移驱动移动 | 表现更自然，但同步更复杂 |

## 11.2 RootMotion 同步流程

> ⚠️ `Root Motion From Everything` 目前**不支持网络同步**
> 仅 `Root Motion From Montages Only` 模式支持

### 核心流程

```
1. Montage 同步由上层处理（UAbilitySystemComponent 或手动 RPC）
2. 客户端从动画中提取位移 → 转换为 Velocity
3. 调用 StartNewPhysics() 进行移动模拟
4. 后续走正常移动同步流程（参考第六、七章）
```

### 与 Motion Matching 的关系

使用 Motion Matching 技术时，网络同步也需要支持 Data Driven 方案，了解 RootMotion 同步流程对此有直接帮助。

---

# 十二、GAS 技能系统介绍 — 核心概念与同步流程

> **原文**：https://zhuanlan.zhihu.com/p/432303699（作者同一人）
> **流程图**：[PNG](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】GAS技能系统介绍/【UE4】GAS技能系统介绍（一）.png) | [PDF](https://github.com/xiaxia9/Game-Development-Notes/blob/main/UE/【UE4】GAS技能系统介绍/【UE4】GAS技能系统介绍（一）.pdf)
> **引擎版本**：UE 4.26

## 12.1 GAS 九大核心概念速查

### ① ASC（UAbilitySystemComponent）
- 技能系统核心组件，管理所有其他部件
- 关键 API：`GiveAbility()` / `TryActivateAbility()` / `TryActivateAbilityByClass()` / `TryActivateAbilitiesByTag()`

### ② GA（UGameplayAbility）
- 技能逻辑主体，可触发 GE 和 GC
- 关键 API：`ActivateAbility()` / `EndAbility()`
- 数据结构链：

```
FGameplayAbilitySpecContainer（ASC 管理）
  └── FGameplayAbilitySpec（GA 实例，含等级）
       ├── FGameplayAbilitySpecHandle（操作句柄）
       └── GA 模板引用
```

### ③ GE（UGameplayEffect）— Buff 系统
- 修改属性、触发 GameplayCue
- **不会实例化** UGameplayEffect 本身，而是创建 `FGameplayEffectSpec` 实例

```
UGameplayEffect（模板，配置数据）
  ├── Modifiers[]       ← 属性修改器
  └── GameplayCues[]    ← 触发的 Cue

FGameplayEffectSpec（实例）→ FActiveGameplayEffect（激活态）
  → FActiveGameplayEffectsContainer（ASC 管理的集合）
```

### ④ GC（UGameplayCueNotify）— 表现层
两种触发方式：

| 类型 | 特点 |
|---|---|
| `UGameplayCueNotify_Static` | 不实例化，直接在 CDO 上调用，**不要保存状态** |
| `AGameplayCueNotify_Actor` | 会 Spawn 实例，ASC 中有引用保存 |

### ⑤ Attribute（FGameplayAttribute）
- `UAttributeSet` 挂在 Actor 上使用
- 定义生命值、攻击力、防御力等

### ⑥ Tag（FGameplayTag）
- 描述和归类对象状态
- `UGameplayTagsManager`（单例）构造标签树
- 独立模块 `GameplayTags`，可被单独引用

### ⑦ Task（UAbilityTask）
- 异步操作（如 Montage 播放结束后结束技能）
- `UAbilityTask` 继承 `UGameplayTask`，添加 GA 引用
- ASC 继承 `UGameplayTasksComponent`

### ⑧ Event（FGameplayEventData）
- 通过 Tag 映射识别事件
- 流程：注册回调 → `SendGameplayEvent()` → 按 EventTag 搜索 ASC 映射表 → 触发回调

### ⑨ 预测（Prediction）
- 客户端无需等服务器许可即可激活 GA 并应用 GE

## 12.2 GAS 同步流程概览

作者总结了六个核心同步流程（对应流程图中的各分支）：

```
1. 技能激活预测流程
2. 服务器收到消息后的处理流程
3. 客户端收到服务器预测激活成功后的流程
4. GE 调用流程
5. GE 移除流程
6. GC 流程
```

> 详细流程图请下载 PDF/PNG 查看，流程图节点对应源码函数名，可直接定位代码。

## 12.3 推荐学习资源

| 资源 | 链接 |
|---|---|
| 官方文档 | https://docs.unrealengine.com/4.26/en-US/InteractiveExperiences/GameplayAbilitySystem/ |
| GASDocumentation（最详细） | https://github.com/tranek/GASDocumentation |
| GAS 插件入门教程 | https://www.cnblogs.com/JackSamuel/tag/GameplayAbility插件/ |
| GAS 入门篇（B站） | https://www.bilibili.com/video/BV1X5411V7jh |
| 深入 GAS 架构设计（B站） | https://www.bilibili.com/video/BV1zD4y1X77M |
| 官方 ARPG 项目 | https://docs.unrealengine.com/4.26/en-US/Resources/SampleGames/ARPG/ |

---

# 十三、游戏开发中的 Data Normalization

> **原文**：Repo 中 `DataNormalization/游戏开发中的DataNormalization.md`
> **应用场景**：Motion Matching 特征标准化

## 13.1 为什么需要 Data Normalization

> 原始数据的特征维度单位不一致，如果不标准化，各特征对最终结果的影响不成比例。
> 在 Motion Matching 中，trajectory 位置、Pose 速度等特征的 Cost 范围不同，
> 不做 Normalization 将导致反复调权重且效果差。

## 13.2 四种标准化方法

### ① Min-Max Normalization（缩放到 [0,1]）

```
x' = (x - min) / (max - min)
```

缩放到 [a, b]：

```
x' = a + (x - min)(b - a) / (max - min)
```

### ② Mean Normalization

```
x' = (x - mean) / (max - min)
```

### ③ Z-Score Normalization（标准化）⭐

```
x' = (x - x̄) / σ
```

- 使每个特征均值=0，标准差=1
- **更适用于正态分布数据**
- Motion Matching 中最常用

### ④ Scaling to Unit Length

```
x' = x / ||x||
```

## 13.3 基础数学回顾

| 概念 | 公式 | 说明 |
|---|---|---|
| 离差平方和 | SS = Σ(xᵢ - x̄)² | 数据越多值越大，实际少用 |
| 总体方差 | σ² = Σ(X-μ)² / N | 解决了离差平方和的缺点 |
| 总体标准差 | SD = √(Σ(xᵢ-μ)²/N) | 表示平均离散程度 |
| 样本标准差 | s = √(Σ(xᵢ-x̄)²/(N-1)) | 分母用 N-1（贝塞尔校正） |

## 13.4 UE 中的实际应用

### UE4：Min-Max + Z-Score 均有使用

### UE5 Pose Search 插件：改良版 Z-Score

> ⚠️ UE5 没有使用标准偏差（SD），而是使用**平均绝对偏差（MD）**

**原因**（来自引擎注释引用的论文）：
- SD 更强调异常值（离差平方以指数增长）
- MD 在数据含微小误差或非完美正态分布时效率更高
- MD 更容易使用，更能容忍极端值
- 在实际观测有小误差时，MD 效率比 SD **高出一倍以上**

> 论文：*Revisiting a 90-Year-Old Debate: The Advantages of the Mean Deviation*

### Daniel Holden 的 Motion Matching 实现

```cpp
// database.h — 标准 Z-Score Normalization
void normalize_feature(
    slice2d<float> features,
    slice1d<float> features_offset,  // mean
    slice1d<float> features_scale,   // std/weight
    const int offset, const int size,
    const float weight = 1.0f)
{
    // Step 1: 计算每个维度的均值
    for (int i = 0; i < features.rows; i++)
        for (int j = 0; j < size; j++)
            features_offset(offset+j) += features(i, offset+j) / features.rows;

    // Step 2: 计算每个维度的方差
    for (int i = 0; i < features.rows; i++)
        for (int j = 0; j < size; j++)
            vars(j) += square(features(i,offset+j) - features_offset(offset+j))
                       / features.rows;

    // Step 3: 计算所有维度的平均标准差
    float std = 0.0f;
    for (int j = 0; j < size; j++)
        std += sqrt(vars(j)) / size;

    // Step 4: scale = std / weight
    // Step 5: normalized = (value - mean) / scale
}
```

**来源**：https://github.com/orangeduck/Motion-Matching

---

# 十四、IK 逆向运动学详解（索引）

> **完整文档**：Repo 中 `逆向运动学（IK）详解/【游戏开发】逆向运动学（IK）详解.md`（45KB）
> **知乎原文**：https://zhuanlan.zhihu.com/p/499405167

由于该文件 45KB 篇幅巨大，这里提供结构索引（如需完整内容可单独读取）：

## 文档结构（推测）

| 章节 | 内容 |
|---|---|
| IK 基础概念 | 正向运动学 vs 逆向运动学 |
| Two Bone IK | 最简单的 IK，两段骨骼（如手臂） |
| CCD IK | Cyclic Coordinate Descent，迭代逼近 |
| FABRIK | Forward And Backward Reaching IK，非迭代 |
| FullBody IK | 全身 IK，UE5 PBIK |
| FootIK 实现 | 脚部地面适配（含预测） |
| 手部 IK | 扶墙、抓握等 |
| UE4/UE5 中的 IK 节点 | AnimGraph 中的 IK 节点使用 |

## 关联引用

- 该文档作者同时也是 Repo 作者（爱吃菠萝不吃萝卜）
- 直接关联：第八章 8.5 节 IK 参考链接库
- 与第五章 Motion Matching 的攀爬系统中的 IK 贴手实现相关

---

# 十五、Repo 中其他可探索资源

以下文档在 Repo 中存在但本次未完整读取（PDF 或尚未展开的目录）：

| 资源 | 路径 | 说明 |
|---|---|---|
| **图解 Chaos 源码** | `UE/图解Chaos源码.pdf` (557KB) | UE5 Chaos 物理引擎源码图解 |
| **UE4 图解动画系统源码** | `UE/【UE4】图解动画系统源码/` | 动画系统源码流程图 |
| **UE5 基于物理的角色动画（基础篇）** | `UE/【UE5】基于物理的角色动画（基础篇）/` | 物理驱动动画入门 |
| **UE5 基于物理的角色动画（应用篇）PhysicsControlComponent** | `UE/` | 物理控制组件应用 |
| **UE5 物理系统源码分析** | `UE/【UE5】物理系统源码分析/` | Chaos 架构、碰撞检测 |
| **游戏服务器开发知识** | `服务器/【记录】游戏服务器开发知识/` | 服务器开发笔记 |

> 💡 以上 PDF 文件可直接从 GitHub 下载：
> https://github.com/xiaxia9/Game-Development-Notes

---

# 附录：完整文档交叉引用表

| 主题 | 第一部分章节 | 第二部分章节 | Repo 资源 |
|---|---|---|---|
| GAS 战斗系统 | 一 | 十二 | GAS技能系统介绍(一).png/pdf |
| 移动网络同步 | 六 | 十、十一 | 一图看懂(上/下).png/pdf/vsdx |
| 移动组件原理 | 七 | — | — |
| Motion Matching | 五 | 十三(Normalization) | DataNormalization.md |
| IK 逆向运动学 | 八(链接库) | 十四(索引) | IK详解.md (45KB) |
| 资源管理 | 二 | — | — |
| NPC 避障 | 三 | — | — |
| LevelSequence | 四 | — | — |
| 物理系统 | — | — | 图解Chaos源码.pdf |
| 动画系统源码 | 八(链接) | — | 图解动画系统源码/ |
| RootMotion 同步 | 六(简述) | 十一(详细) | 一图看懂(下).png |
| 物理角色动画 | 八(链接) | — | 基于物理的角色动画(基础/应用)/ |

---

> **第二部分结束**
>
> 两部分合并即为完整的技术参考文档。
> 如需将 IK 详解 45KB 完整内容也提取输出，请告诉我。