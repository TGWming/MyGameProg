# Prompt 01 — 创建 SaveGame Blueprint 变量

## 目标
在 bp_SaveGameSettings 蓝图中创建 4 个 Float 变量用于存储设置

## 操作步骤（手动蓝图操作）

### bp_SaveGameSettings 中添加变量
| 变量名 | 类型 | 默认值 | 用途 |
|--------|------|--------|------|
| Brightness | Float | 0.5 | 亮度 |
| MusicVol | Float | 0.75 | 音乐音量 |
| SfxVol | Float | 0.75 | 音效音量 |
| MasterVol | Float | 0.75 | 主音量 |

### 注意
- bp_SaveGameSettings 的父类必须是 SaveGame
- 路径：`/Game/Levels/bp_SaveGameSettings`
- 所有变量勾选 Instance Editable 和 Expose on Spawn