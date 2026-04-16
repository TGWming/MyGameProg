# bp_LowerWorld_FarLightOn_Texture 蓝图方案总结

> 日期：2026-04-16  
> 蓝图路径：`/Game/blueprint/WeatherSystemChange/bp_LowerWorld_FarLightOn_Texture`  
> 功能：玩家进出下层世界时，SpotLight（FarviewOverallLighting）的 Attenuation Radius 动态渐变

---

## 一、变量一览

| 变量名 | 类型 | 用途 | BeginPlay 初始值 |
|--------|------|------|-----------------|
| `FarviewOverallLighting` | SpotLight (Object Ref) | 在 Details 面板中手动指定场景中的 SpotLight | 由面板赋值 |
| `SetFarviewOverallLight` | Actor (Object Ref) | 运行时存储 FarviewOverallLighting 的引用 | = FarviewOverallLighting |
| `Light_FloatAttenuation_A` | Float | 初始/外部 衰减半径 | **7500.0** |
| `Light_FloatAttenuation_B` | Float | 下层世界 衰减半径 | **7500000.0** |
| `TargetAttenuation` | Float | 当前插值目标值 | — |
| `IsInLowerworld` | Bool | 玩家是否在下层世界 | false |
| `HasEnteredLowerWorld` | Bool | 是否曾进入过下层世界 | false |
| `IsFogFading` | Bool | 是否正在进行衰减插值 | false |
| `brevertOnExit` | Bool | 离开触发盒时是否还原（可选） | false |

---

## 二、Event BeginPlay（初始化）

```
BeginPlay → Execution Sequence
  ├─ then_0: Set Light_FloatAttenuation_A = 7500.0
  │          → Set Light_FloatAttenuation_B = 7500000.0
  │          → Set SetFarviewOverallLight = Get FarviewOverallLighting
  │          → SetAttenuationRadius(Light_FloatAttenuation_A)
  │             Target: FarviewOverallLighting → SpotLightComponent
  │
  └─ then_4: Set IsInLowerworld = false
             → Set HasEnteredLowerWorld = false
```

### 关键点
- **A / B 值在蓝图中硬编码**，World Outliner 的 Details 面板中**不需要再设 Default 值**（除了 `FarviewOverallLighting` 需要手动拖入场景中的 SpotLight 引用）。
- BeginPlay 时立即调用 `SetAttenuationRadius` 将灯光初始化为 A 值。
- 获取 SpotLightComponent 的方式：`FarviewOverallLighting` → 拖出 `SpotLightComponent`（引擎内置属性），**不需要** Cast to SpotLightComponent。

---

## 三、Event Tick（插值驱动）

```
Event Tick → Branch (IsFogFading)
  └─ True:
       FInterpTo(
         Current = FarviewOverallLighting → SpotLightComponent → AttenuationRadius,
         Target  = TargetAttenuation,
         DeltaTime = 0.5,
         InterpSpeed = 0.05
       )
       → SetAttenuationRadius(ReturnValue)
           Target: FarviewOverallLighting → SpotLightComponent
```

### 获取 Current 值的节点链
```
Get FarviewOverallLighting
  → Get SpotLightComponent (SpotLight 内置属性)
    → Get AttenuationRadius (LocalLightComponent 内置属性)
```

> ⚠️ 之前的编译错误 `"This blueprint (self) is not a LocalLightComponent"` 是因为 `SetAttenuationRadius` 的 Target 引脚没有连接。  
> **解决方法**：将 `FarviewOverallLighting → SpotLightComponent` 连接到 `SetAttenuationRadius` 的 Target 引脚。

---

## 四、ActorBeginOverlap（进入触发盒）

```
ReceiveActorBeginOverlap (OtherActor)
  → Execution Sequence
    └─ then_0: Cast to Player1 (Object = OtherActor)
               → Branch (IsInLowerworld)
                 ├─ True（已在下层，玩家走回去）:
                 │    Set IsInLowerworld = false
                 │    → Set TargetAttenuation = Light_FloatAttenuation_A
                 │    → Set IsFogFading = true
                 │
                 └─ False（不在下层，玩家走进去）:
                      Set IsInLowerworld = true
                      → Set TargetAttenuation = Light_FloatAttenuation_B
                      → Set IsFogFading = true
                      → Branch (HasEnteredLowerWorld)
                        └─ False: Set HasEnteredLowerWorld = true
```

### 双向支持确认 ✅
- **玩家走进触发盒**（IsInLowerworld = false）→ 目标切换到 B（大衰减），灯光渐变变亮/范围变大
- **玩家走出触发盒再走回来**（IsInLowerworld = true）→ 目标切换回 A（小衰减），灯光渐变恢复
- 每次切换都设置 `IsFogFading = true`，Event Tick 会持续插值直到到达目标值

---

## 五、ActorEndOverlap（离开触发盒，可选）

```
ReceiveActorEndOverlap
  → Branch (brevertOnExit)
    └─ True: （还原逻辑，根据需求连接）
```

目前 `brevertOnExit` 默认为 false，此分支不执行。

---

## 六、关于 FarviewOverallLighting 变量的赋值

| 赋值方式 | 说明 |
|----------|------|
| ~~GetAllActorsOfClass~~ | ❌ 已移除，不再使用 |
| **Details 面板手动指定** | ✅ 在 World Outliner 选中此 Actor 后，在 Details 面板将 `FarviewOverallLighting` 设为场景中的 SpotLight |

- 变量类型为 `SpotLight Object Reference`
- 勾选 **Instance Editable**（眼睛��标）使其可在 Details 面板中赋值

---

## 七、常见问题

### Q1: World Outliner 中需要设置 Default 值吗？
**不需要**。`Light_FloatAttenuation_A` 和 `B` 已在 BeginPlay 中用 Set 节点硬编码赋值。唯一需要在 Details 面板中手动设置的是 `FarviewOverallLighting`（拖入场景中的 SpotLight Actor）。

### Q2: 灯光能否在玩家走进/走出时来回变化？
**可以**。逻辑通过 `IsInLowerworld` 布尔值翻转：
- 进入时 false→true，目标 = B
- 再次触发时 true→false，目标 = A
- Event Tick 中 FInterpTo 持续平滑插值

### Q3: 编译错误 "self is not a LocalLightComponent"
`SetAttenuationRadius` 需要一个 `LocalLightComponent` 作为 Target。正确做法：
```
Get FarviewOverallLighting → Get SpotLightComponent → 连到 SetAttenuationRadius 的 Target
```

---

## 八、节点连接速查图（文字版）

```
[BeginPlay]
    │
    ▼
[Sequence]──► Set A=7500 → Set B=7500000 → Set Light=FarviewOverallLighting
    │                                         → SetAttenuationRadius(A)
    │                                            target: SpotLightComponent
    │
    └──► Set IsInLowerworld=false → Set HasEnteredLowerWorld=false


[Tick]──► Branch(IsFogFading)
              │ True
              ▼
          FInterpTo(Current=AttenuationRadius, Target=TargetAttenuation)
              │
              ▼
          SetAttenuationRadius(result)  ← target: SpotLightComponent


[BeginOverlap]──► Sequence → Cast to Player1
                                  │
                              Branch(IsInLowerworld)
                              ├─ True:  Set IsInLowerworld=false
                              │         Set TargetAttenuation=A
                              │         Set IsFogFading=true
                              │
                              └─ False: Set IsInLowerworld=true
                                        Set TargetAttenuation=B
                                        Set IsFogFading=true
                                        → Branch(HasEntered) → Set true
```