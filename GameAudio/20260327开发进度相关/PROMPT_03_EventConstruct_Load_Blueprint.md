# Prompt 03 — Event Construct 加载逻辑

## 目标
Widget 构造时从存档加载设置，恢复 Slider 值并应用

## ��图连接流程

```
Event Construct
  │
  ↓
Does Save Game Exist
  Slot Name: "PlayerSettings"
  │
  ↓
Branch (Condition ← Return Value)
  │
  ├─ True:
  │   │
  │   ↓
  │   Load Game from Slot
  │     Slot Name: "PlayerSettings"
  │     │
  │     ↓
  │   Cast to bp_SaveGameSettings
  │     Object ← Load Game 的 Return Value
  │     │
  │     ↓ (then)
  │   Set Value (AnalogSlider_0)
  │     Target: AnalogSlider_0
  │     InValue: ← "As bp Save Game Settings" → Get Brightness
  │     │
  │     ↓
  │   Set Value (AnalogSlider_118)
  │     Target: AnalogSlider_118
  │     InValue: ← "As bp Save Game Settings" → Get MusicVol
  │     │
  │     ↓
  │   Set Value (AnalogSlider_245)
  │     Target: AnalogSlider_245
  │     InValue: ← "As bp Save Game Settings" → Get SfxVol
  │     │
  │     ↓
  │   Set Value (AnalogSlider_326)
  │     Target: AnalogSlider_326
  │     InValue: ← "As bp Save Game Settings" → Get MasterVol
  │     │
  │     ↓
  │   (后续应用音量和亮度)
  │
  └─ False: (不连接，使用默认值)
```

## 关键要点
- 所有 4 个 Get 节点的 **Target/self** pin 都必须连到 Cast 的 "As bp Save Game Settings" 输出
- 不要使用 Texture.AdjustBrightness，必须使用 bp_SaveGameSettings 的 Brightness 变量
- Set Value 不会触发 OnValueChanged，加载后需要手动执行音量和亮度的应用逻辑