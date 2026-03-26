# Prompt 05 — Audio Zone 多 Volume 支持（TriggerID）

## 目标
让同一个 ZoneID 的多个 Volume 可以各自独立注册/注销，
只有所有 Volume 都离开后才真正停止该 Zone 的声音

## 给 VS Code Agent 的 Prompt

```
修改 AudioManagerActor 的 EnterZone 和 ExitZone 逻辑，
让同一个 ZoneID 可以被多个 Volume 同时注册（引用计数），
这样只有当最后一个同 ZoneID 的 Volume 都 Exit 后才真正移除。

具体修改：

### 1. 在 AudioManagerActor.h 中：

把 FActiveZoneEntry 的 operator== 改为同时比较 ZoneID、Priority 和一个新的 TriggerID：

struct FActiveZoneEntry
{
    FName ZoneID;
    int32 Priority;
    int32 TriggerID;  // 新增：用于区分同一 Zone 的不同 Volume

    bool operator==(const FActiveZoneEntry& Other) const
    {
        return ZoneID == Other.ZoneID && Priority == Other.Priority && TriggerID == Other.TriggerID;
    }
};

### 2. 修改 EnterZone 和 ExitZone 的签名：

把：
void EnterZone(FName ZoneID, int32 Priority);
void ExitZone(FName ZoneID, int32 Priority);

改为：
void EnterZone(FName ZoneID, int32 Priority, int32 TriggerID);
void ExitZone(FName ZoneID, int32 Priority, int32 TriggerID);

### 3. 在 AudioManagerActor.cpp 中：

EnterZone 函数：
- 构造 FActiveZoneEntry 时加入 TriggerID
- 其余逻辑不变

ExitZone 函数：
- 构造要移除的条目时也加入 TriggerID
- 其余逻辑不变

UpdateActiveZone 函数中的 GetCurrentZoneID 不需要改，
它依然取栈顶的 ZoneID，只要栈里还有同 ZoneID 的条目就不会变。

### 4. 在 AudioZoneTrigger.h 中：

给 AAudioZoneTrigger 添加一个成员变量用作唯一标识：

UPROPERTY()
int32 TriggerUniqueID;

### 5. 在 AudioZoneTrigger.cpp 中：

在 BeginPlay 中生成唯一 ID：
TriggerUniqueID = static_cast<int32>(GetUniqueID());

修改 OnBeginOverlap 中的调用：
把 Manager->EnterZone(ZoneID, Priority);
改为 Manager->EnterZone(ZoneID, Priority, TriggerUniqueID);

修改 OnEndOverlap 中的调用：
把 Manager->ExitZone(ZoneID, Priority);
改为 Manager->ExitZone(ZoneID, Priority, TriggerUniqueID);

### 注意：
- 不要改动 UpdateActiveZone、PlayAmbienceForZone、StopCurrentAmbience 的逻辑
- 不要改动 Music 相关逻辑
- GetCurrentZoneID 不需要改动
- 确保编译通过
```

## 改动涉及的文件
| 文件 | 路径 |
|------|------|
| AudioManagerActor.h | `Source/soul/Public/AudioManagerActor.h` |
| AudioManagerActor.cpp | `Source/soul/Private/AudioManagerActor.cpp` |
| AudioZoneTrigger.h | `Source/soul/Public/AudioZoneTrigger.h` |
| AudioZoneTrigger.cpp | `Source/soul/Private/AudioZoneTrigger.cpp` |