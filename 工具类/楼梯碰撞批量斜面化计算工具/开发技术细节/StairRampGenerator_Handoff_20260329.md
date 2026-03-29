# StairRamp Generator — Handoff 文档

> 日期：2026-03-29  
> 状态：基础功能完成，已通过 L 形楼梯测试

---

## 完成内容清单

### ✅ 核心功能

| # | 功能 | 状态 | 说明 |
|---|------|------|------|
| 1 | Editor 模块 soulEditor | ✅ 完成 | Build.cs + Module 注册 |
| 2 | CreateStairSpline 函数 | ✅ 完成 | 从楼梯 Mesh 生成 Spline 路径 |
| 3 | GenerateRampFromSpline 函数 | ✅ 完成 | 沿 Spline 生成分段碰撞体 |
| 4 | CreateRampSegment 函数 | ✅ 完成 | 单段 BoxComponent 碰撞体 |
| 5 | Editor Widget 蓝图 | ✅ 完成 | 3 按钮面板 |

### ✅ 优化与修复

| # | 项目 | 状态 | 说明 |
|---|------|------|------|
| 6 | Spline 位置修复 | ✅ 完成 | SetActorLocation 在 SetRootComponent 之后调用 |
| 7 | 重名崩溃修复 | ✅ 完成 | 不设置 SpawnParams.Name，用 SetActorLabel |
| 8 | 编辑器绿色线框可视化 | ✅ 完成 | ShapeColor + bVisualizeComponent |
| 9 | 逐段宽度自动计算 | ✅ 完成 | 从 Mesh 顶点投影计算每段宽度 |
| 10 | 楼梯 Actor 名字存储 | ✅ 完成 | Spline Tag 存储，自动查找 |

### 📝 未完成（后续优化）

| # | 项目 | 优先级 |
|---|------|--------|
| 11 | 螺旋楼梯实际测试 | P3 |
| 12 | Ramp Thickness 自动匹配台阶高度 | P2 |
| 13 | 一键清除旧 Ramp 按钮 | P1 |
| 14 | Spline 控制点自动沿楼梯路径生成 | P4 |

---

## 文件清单

```
已修改/创建的文件：

Source/soulEditor/
├── soulEditor.Build.cs                    ← 模块构建配置
├── Public/
│   └── StairRampGeneratorBFL.h            ← 头文件（函数声明）
└── Private/
    ├── soulEditorModule.cpp               ← 模块注册
    └── StairRampGeneratorBFL.cpp          ← 核心实现（所有函数）

Content/blueprint/
└── WBP_StairsRampTester.uasset           ← Editor Widget 蓝图
```

---

## 测试结果

```
✅ 编译：Development Editor + Win64 通过
✅ L 形楼梯：Spline 位置正确，Ramp 生成正确
✅ 绿色线框：不选中也可见
✅ 逐段宽度：从 1677 个 Mesh 顶点计算，4 段独立宽度
✅ 重复创建：不再闪退
✅ Tag 自动查找：不需要手动连 OriginalStairActor 引脚
```

---

## 已知限制

```
1. Spline 控制点需要手动调整（L/U/螺旋楼梯）
2. 碰撞体是矩形 Box，不是扇形（螺旋楼梯不完美贴合）
3. Thickness 固定值，不自动匹配台阶凹凸
4. 没有一键删除旧 Ramp 的功能
5. BoundingBox 初始宽度估算对 L 形楼梯不精确
   （但逐段计算已弥补此问题）
```

---

## 接手须知

```
1. 所有代码在 soulEditor 模块中，不影响运行时 soul 模块
2. 不影响 FlowVisualizer 等其他插件
3. Editor Widget 在 Content/blueprint/ 下
4. 打开方式：Window → Editor Utility Widgets → WBP_StairsRampTester
5. 编译时如遇到 FlowVisualizer lib 缺失：
   右键 .uproject → Generate Visual Studio project files → 重新编译
```