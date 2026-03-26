# Prompt 02 — SaveAllSettings 保存逻辑

## 目标
每个 Slider 的 OnValueChanged 触发时，保存所有 4 个设置到存档

## 蓝图连接流程

```
OnValueChanged (任一 Slider)
  │
  ↓
Create Save Game Object
  Class: bp_SaveGameSettings
  │
  ↓
Set Brightness ← AnalogSlider_0 → Get Value
Set MusicVol   ← AnalogSlider_118 → Get Value
Set SfxVol     ← AnalogSlider_245 → Get Value
Set MasterVol  ← AnalogSlider_326 → Get Value
  │
  ↓
Save Game to Slot
  Slot Name: "PlayerSettings"
  User Index: 0
  │
  ↓
应用设置：
  Bus Set Volume (Mus Bus)    ← AnalogSlider_118 → Get Value
  Bus Set Volume (SFX Bus)    ← AnalogSlider_245 → Get Value
  Bus Set Volume (Master Bus) ← AnalogSlider_326 → Get Value
  │
  ↓
亮度应用：
  Lerp (A=0.5, B=3.9, Alpha= AnalogSlider_0 → Get Value)
    → Float to String
    → Append ("gamma ")
    → Execute Console Command
```

## Slider 对应关系
| Slider | 设置 | SaveGame 变量 |
|--------|------|---------------|
| AnalogSlider_0 | 亮度 | Brightness |
| AnalogSlider_118 | 音乐 | MusicVol |
| AnalogSlider_245 | 音效 | SfxVol |
| AnalogSlider_326 | 主音量 | MasterVol |

## 注意事项
- 每个 Slider 都使用相同的保存逻辑（可复制粘贴）
- Create Save Game Object 每次都重新创建，获取最新的 4 个值后保存
- Slot Name 固定为 "PlayerSettings"