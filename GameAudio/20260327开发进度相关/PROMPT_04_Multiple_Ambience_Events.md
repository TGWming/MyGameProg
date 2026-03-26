# Prompt 04 — Audio Manager 多 Ambience Event 支持

## 目标
将 Audio Manager 的 Ambience 从单个 Event 改为数组，支持同时播放多个氛围音层

## 给 VS Code Agent 的 Prompt

```
在我的 Unreal Engine 项目中，找到 Audio Manager Actor 相关的 C++ 文件（.h 和 .cpp），
这个 Actor 使用了 FMOD 插件来管理游戏音频。

请执行以下修改：

### 1. 在结构体（ZoneConfig 或类似的音频区域配置结构体）中：

把 AmbienceEvent 从单个 UFMODEvent* 改为 TArray<UFMODEvent*>：

- 原来：
  UFMODEvent* AmbienceEvent;

- 改为：
  TArray<UFMODEvent*> AmbienceEvents;

保留 UPROPERTY 的所有现有修饰符（EditAnywhere, BlueprintReadWrite 等），
并确保 Category 保持不变。

### 2. 存储活跃实例：

在 Actor 类中，把存储 Ambience 实例的变量也从单个改为数组：

- 原来类似：
  FFMODEventInstance AmbienceInstance;（或类似的单个实例变量）

- 改为：
  TArray<FFMODEventInstance> ActiveAmbienceInstances;

### 3. 播放逻辑：

找到播放 Ambience 的函数，把原来播放单个 Event 的逻辑改为：

- 先清空 ActiveAmbienceInstances
- 遍历 AmbienceEvents 数组
- 对每个非 nullptr 的 Event，调用 UFMODBlueprintStatics::PlayEvent2D
  或当前使用的播放方法
- 把返回的每个实例 Add 到 ActiveAmbienceInstances 数组中

### 4. 停止逻辑：

找到停止 Ambience 的函数，把原来停止单个实例的逻辑改为：

- 遍历 ActiveAmbienceInstances 数组
- 对每个实例调用 Stop
- 遍历完成后 Empty() 清空数组

### 5. 切换区域逻辑：

确保切换音频区域时：
- 先停止所有旧的 Ambience 实例（用上面第4步的逻辑）
- 再播放新区域的所有 Ambience Events（用上面第3步的逻辑）

### 注意事项：
- Music Event 保持不变，仍然是单个 UFMODEvent*
- ��要改动 Music 相关的任何逻辑
- 不要改动 Global Params 和 Default Params 的逻辑
- 确保编译通过
- 保持现有的代码风格
```

## 改动涉及的文件
| 文件 | 路径 |
|------|------|
| AudioTypes.h | `Source/soul/Public/AudioTypes.h` |
| AudioManagerActor.h | `Source/soul/Public/AudioManagerActor.h` |
| AudioManagerActor.cpp | `Source/soul/Private/AudioManagerActor.cpp` |