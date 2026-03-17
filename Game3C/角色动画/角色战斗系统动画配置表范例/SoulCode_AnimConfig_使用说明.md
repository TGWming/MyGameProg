# SoulCode 动画配置总表 —— 使用说明

> 版本：v1.0  
> 日期：2026-03-17  
> 配套文件：`SoulCode_AnimConfig_Master_v1.0.csv`

---

## 一、这个表是什么

这是你整个类魂项目（SoulCode）的 **动画系统一站式配置表**。

一个 CSV 文件里包含了所有角色（玩家 / 小兵 / 精英 / 迷你Boss / 教程Boss / 最终Boss / 敌对NPC / 犬类 / 巨兽 / 飞虫 / 宝箱怪）的：

- 全局参数（帧率、输入缓存、Root Motion 模式等）
- 体型分类与锁定系统对应关系
- BlendSpace 资产配置
- State Machine 状态定义与转换条件
- **所有 Montage** 的时长 / BlendIn / BlendOut / Root Motion / 伤害 / 精力消耗
- **所有 Notify 窗口**的精确时间（无敌帧 / 霸体 / Hitbox / 连段窗口 / 翻滚取消等）
- Blend 参数参考（按动作类别的推荐曲线）
- 角色战斗属性（HP / Poise / 速度 / 攻击范围 / 阶段触发条件）

**总计约 250+ 行配置记录，覆盖 9 类角色对象。**

---

## 二、文件打开方式

### 方法 A：Excel（推荐）

1. 双击 `SoulCode_AnimConfig_Master_v1.0.csv` 用 Excel 打开
2. 如果出现乱码：文件 → 打开 → 选择 UTF-8 编码
3. 全选第一行 → **数据 → 筛选**（快捷键 `Ctrl+Shift+L`）
4. 现在每列表头都有下拉箭头，可以筛选

### 方法 B：Google Sheets

1. 打开 Google Sheets → 文件 → 导入 → 上传 CSV
2. 分隔符选「逗号」
3. 同样对第一行启用筛选器

### 方法 C：WPS / LibreOffice Calc

与 Excel 用法完全相同。

---

## 三、列含义速查

| 列名 | 数据类型 | 说明 |
|---|---|---|
| `Category` | 字符串 | **大分类**（最重要的筛选维度） |
| `SubCategory` | 字符串 | 子分类（如 Roll / LightAtk / HitReact） |
| `Owner` | 字符串 | 所属角色（Player / Grunt / Elite / Boss 等） |
| `ID` | 字符串 | 唯一编号，前缀标识类型 |
| `Name` | 字符串 | 资产名 / Notify名 / 参数名 |
| `Type` | 字符串 | 具体类型（Attack / Death / NotifyState 等） |
| `Duration_s` | 数字 | 时长，单位：秒 |
| `BlendIn_s` | 数字 | Montage BlendIn 时间，单位：秒 |
| `BlendOut_s` | 数字 | Montage BlendOut 时间，单位：秒 |
| `RootMotion` | 字符串 | ✅ = 使用 / ❌ = 不使用 |
| `Damage` | 字符串 | 伤害值（支持范围值和多段写法如 `30+33+40`） |
| `StaminaCost` | 数字/字符串 | 精力消耗 |
| `Poise` | 数字 | 韧性值 |
| `Notify_Start_s` | 数字 | Notify 开始时间（秒或比例值，见 Notes） |
| `Notify_End_s` | 数字 | Notify 结束时间（秒或比例值） |
| `Axis_X` | 字符串 | BlendSpace X 轴配置 |
| `Axis_Y` | 字符串 | BlendSpace Y 轴配置 |
| `AnimCount` | 数字 | BlendSpace 包含的动画点数 |
| `Phase` | 字符串 | Boss 阶段：P1 / P2 / P3 |
| `SizeCategory` | 字符串 | 体型：Small / Medium / Large |
| `Speed` | 字符串 | 移动速度相关 |
| `Range_cm` | 字符串 | 攻击范围，单位：厘米 |
| `Condition_In` | 字符串 | State Machine 进入条件 |
| `Condition_Out` | 字符串 | State Machine 退出条件 |
| `Priority` | 数字 | Notify 窗口优先级（1=最高，3=最低） |
| `Notes` | 字符串 | 备注说明 |

### ID 编号前缀含义

| 前缀 | 含义 | 示例 |
|---|---|---|
| `G` | Global 全局参数 | G001 |
| `SC` | SizeClass 体型分类 | SC01 |
| `LK` | LockOn 锁定配置 | LK02 |
| `BS` | BlendSpace | BS01 |
| `SM` | StateMachine 状态 | SM07 |
| `M` | Montage | M001 ~ M183 |
| `N` | Notify | N001 ~ N054 |
| `BR` | BlendReference 混合参考 | BR01 |
| `CS` | CharStats 角色属性 | CS01 |

---

## 四、日常使用：按场景筛选

### 场景 1：「我要配玩家的翻滚」

筛选条件：
- `Category` = `Montage`
- `Owner` = `Player`
- `SubCategory` = `Roll`

→ 显示 9 行：8 方向翻滚 + 后撤步，包含时长/BlendIn/BlendOut/精力消耗

再追加查看翻滚的 Notify：
- `Category` = `Notify`
- `Owner` = `Player`
- `SubCategory` 包含 `Roll` 或 `IFrame`

→ 显示无敌帧窗口和 ActionEnd 时间

### 场景 2：「我要配精英敌人的全部攻击」

筛选条件：
- `Category` = `Montage`
- `Owner` = `Elite`
- `Type` = `Attack`

→ 显示精英所有攻击 Montage（轻击×3 / 三连段 / 重击 / 冲刺 / 格挡反击 / 踢击）

### 场景 3：「我要查最终 Boss 第二阶段的招式」

筛选条件：
- `Owner` = `FinalBoss`
- `Phase` = `P2`

→ 显示 P2 阶段所有 Montage（阶段转换 / 四连 / 瞬移斩 / 剑气波 / AOE / 抓取 / 五连）

### 场景 4：「我要看所有角色的 BlendSpace 配置」

筛选条件：
- `Category` = `BlendSpace`

→ 显示全部 12 个 BS 资产，包含轴配置和动画点数

### 场景 5：「我想知道所有死亡动画的配置」

筛选条件：
- `Type` = `Death`

→ 跨角色显示所有死亡 Montage（玩家 / 小兵 / 精英 / Boss / 犬 / 兽 / 虫 / 宝箱怪）

### 场景 6：「我要看所有 Notify 窗口时间」

筛选条件：
- `Category` = `Notify`

→ 显示 54 条 Notify 记录，包含玩家窗口、敌人霸体、Boss 特殊通知等

### 场景 7：「我要看某个角色的完整配置」

筛选条件：
- `Owner` = `Mimic`（或任何角色名）

→ 显示该角色的 StateMachine + Montage + Notify + CharStats 全部内容

---

## 五、Category 取值一览

| Category 值 | 包含内容 | 典型数量 |
|---|---|---|
| `GlobalParam` | 帧率 / 时间单位 / 输入缓存 / Root Motion 模式 | 6 条 |
| `SizeClass` | S / M / L / XL 体型分类标准 | 4 条 |
| `LockOnConfig` | 体型与锁定 UI / 距离 / Socket 的对应 | 3 条 |
| `BlendSpace` | 所有 BS 资产的轴和动画点位 | 12 条 |
| `StateMachine` | 所有角色的 SM State 定义和转换条件 | 30 条 |
| `Montage` | **核心数据**——所有 Montage 的完整参数 | 183 条 |
| `Notify` | 所有 Notify 窗口的时间和优先级 | 54 条 |
| `BlendRef` | 按动作类别的 Blend 参数推荐值 | 15 条 |
| `CharStats` | 角色战斗属性（HP / Poise / 速度 / 阶段等） | 14 条 |

---

## 六、Owner 取值一览

| Owner 值 | 对应角色 | 代码类 |
|---|---|---|
| `Global` | 全局 / 通用 | — |
| `Player` | 玩家角色 | `AMycharacter` → `APlayerCharacter` |
| `Grunt` | 小兵敌人 | `AGruntEnemy` |
| `Elite` | 精英敌人 | `AEliteEnemy` |
| `MiniBoss` | 迷你 Boss | `ABossEnemy (bIsMiniBoss=true)` |
| `TutBoss` | 教程 Boss | `ABossEnemy` |
| `FinalBoss` | 最终 Boss | `ABossEnemy (bIsBoss=true)` |
| `HostileNPC` | 敌对 NPC | 基于玩家骨骼的 NPC |
| `Dog` | 犬类生物 | `ACreatureEnemy` |
| `LargeBeast` | 巨型兽 | `ACreatureEnemy` |
| `FlyingInsect` | 飞行虫类 | `ACreatureEnemy` |
| `Mimic` | 宝箱怪 | `AMimicEnemy` |
| `Boss` | Boss 通用 Notify | 所有 Boss 共用 |

---

## 七、与代码的对应关系

### 7.1 现有代码映射

| CSV 中的数据 | 对应的现有代码 |
|---|---|
| `LockOnConfig` 行 | `LockOnConfig.h` 中的 `EEnemySizeCategory` |
| `CharStats` → Player 行的 Speed | `MyCharacter.h` 中的 `WalkSpeed` / `RunSpeed` / `LockedOnSpeed` |
| `CharStats` → Player 行的 StaminaCost | `MyCharacter.h` 中的 `DodgeCost` / `AttackCost` |
| `Montage` 行的 BlendIn/BlendOut | UE4 Montage 资产编辑器中的 Blend 设置 |
| `Notify` 行 | UE4 Montage 编辑器中拖放的 Notify 位置 |
| `BlendSpace` 行 | UE4 BS 资产编辑器中的轴设置 |
| `StateMachine` 行 | AnimBP 中的 State Machine 节点和过渡箭头 |

### 7.2 未来代码映射（待创建）

| CSV 数据 | 将要创建的代码 |
|---|---|
| `CharStats` 的 Poise / PoiseRegen | `ASoulsCharacterBase` 的韧性系统 |
| `Notify` 的 ANS_ComboWindow | `UAnimNotifyState_ComboWindow` C++ 类 |
| `Notify` 的 ANS_IFrame | `UAnimNotifyState_IFrame` C++ 类 |
| `Notify` 的 ANS_SuperArmor | `UAnimNotifyState_SuperArmor` C++ 类 |
| `Notify` 的 AN_EnableHitbox | `UAnimNotify_EnableHitbox` C++ 类 |
| `Notify` 的 AN_ActionEnd | `UAnimNotify_ActionEnd` C++ 类 |

---

## 八、数值说明

### 8.1 时间值

- 所有 `Duration_s` / `BlendIn_s` / `BlendOut_s` 单位为**秒**
- Notify 的 `Start` / `End` 有两种标注方式（在 Notes 列说明）：
  - **绝对时间**（秒）：直接用于具体 Montage 的 Notify 放置
  - **比例值**（0.0~1.0）：通用参考，需要乘以 Montage 总时长换算

### 8.2 伤害值

- 以「玩家满血 = 100 HP」为基准
- 多段伤害用 `+` 分隔，如 `30+33+40` 表示三段
- 范围值用 `~` 表示，如 `40~55`
- `(AOE)` 后缀表示范围伤害
- `(破防)` 后缀表示破盾效果
- `(远程)` 后缀表示远程攻击
- `(处决)` 后缀表示处决级伤害

### 8.3 Root Motion 标注

- `✅` = 使用 Root Motion
- `✅(短)` / `✅(中)` / `✅(长)` / `✅(大)` = Root Motion 位移程度提示
- `❌` = 不使用 Root Motion

### 8.4 空值

CSV 中的空单元格表示「该字段对此行不适用」，不是遗漏。例如：
- BlendSpace 行没有 Duration_s（BS 不是 Montage）
- 受击 Montage 没有 Damage（受击方不造成伤害）
- GlobalParam 行只有 Notes 有值

---

## 九、维护与扩展

### 新增武器

1. 在 `Montage` 区域找到 `Owner=Player` 的攻击 Montage
2. 复制直剑的 4 行（LightCombo / Heavy / RunAttack / JumpAttack）
3. 修改 Name 和参数
4. ID 从 M184 继续编号

### 新增敌人

1. 在 `CharStats` 区域新增一行角色属性
2. 在 `StateMachine` 区域新增该角色的 States
3. 在 `BlendSpace` 区域新增该��色的 BS（如需要）
4. 在 `Montage` 区域新增该角色的所有 Montage
5. 在 `Notify` 区域新增该角色特有的 Notify 配置

### 新增 Boss 阶段

在对应 Boss 的行后追加新 Phase 的 Montage，`Phase` 列填 P3 / P4 等。

---

## 十、常见问题

**Q：为什么不是多个 Sheet 而是一个大表？**  
A：一个表可以跨角色搜索和对比。比如筛选 `Type=Death` 就能看到所有角色的死亡动画配置，方便统一调参。用 Excel 筛选器的效率比在 56 个文件间切换高得多。

**Q：表里的数值能直接用吗？**  
A：这些是���于血源诅咒/黑魂3分析得出的参考基准值。实际需要在 UE4 中根据你的动画资源时长做微调。核心比例关系（如窗口优先级、IFrame 长度、BlendIn/Out 比）是经过验证的。

**Q：Notify 的比例值和绝对值怎么区分？**  
A：看 Notes 列。标注「比例值」的是通用参考（N001~N024），标注「绝对时间」的是针对具体 Montage 的（N036~N054）。

**Q：我要在 UE4 DataTable 里导入这个 CSV 怎么办？**  
A：需要先按 Category 筛选出 Montage 部分，导出为单独 CSV，然后创建对应的 `USTRUCT` 作为 DataTable 的行结构。后续可以帮你写这个 struct。

---

## 十一、版本记录

| 版本 | 日期 | 变更 |
|---|---|---|
| v1.0 | 2026-03-17 | 初版：9 类角色 / 183 Montage / 54 Notify / 12 BS / 30 SM / 14 CharStats |