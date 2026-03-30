# Spline 自动路径 — Step 5：编译与测试计划

## 编译

```
手动编译步骤：

1. 右键 soul.uproject → Generate Visual Studio project files
   （如果 Step 1-4 过程中遇到过编译问题，先做这步）
2. VS 中 Build → Development Editor + Win64
3. 或在 UE4 编辑器中 Ctrl+Shift+B

如果遇到 FlowVisualizer lib 缺失：
  → 删除 Plugins/FlowVisualizer/Intermediate/ 文件夹
  → 重新 Generate VS project files
  → 重新编译
```

## 测试 A：直梯（应 fallback 到 2 点）

```
1. 选中一个直梯 Mesh
2. 点击 "Create Stair Spline"
3. Output Log 搜索 "AutoPath"
   预期：
     StairRampGenerator: AutoPath raw centroids=X
     StairRampGenerator: AutoPath centroids < 3, using default 2-point
     或
     StairRampGenerator: AutoPath simplified X -> 2 points
     StairRampGenerator: Using default 2-point path (bottom + top)
   
   因为直梯的所有层质心都在一条直线上，
   Douglas-Peucker 会简化到 2 个点，
   条件 AutoPathPoints.Num() >= 3 不满足 → fallback

4. 视口中 Spline 应该和之前一样：2 个控制点，底部到顶部直线
5. 生成 Ramp → 确认碰撞体正常
```

## 测试 B：L 形楼梯（应自动生成 3+ 点）

```
1. 选中 L 形楼梯 Mesh（如 SM_stairs_L_shaped_2）
2. 点击 "Create Stair Spline"
3. Output Log 搜索 "AutoPath"
   预期：
     StairRampGenerator: AutoPath raw centroids=X（X >= 5）
     StairRampGenerator: AutoPath simplified X -> 3 points (tolerance=XX.X)
     StairRampGenerator: Using auto path with 3 control points
   
   L 形楼梯应该检测到：下层段 + 平台 + 上层段
   简化后保留 3 个拐点

4. 视口中 Spline 应该有 3 个控制点，沿 L 形路径弯曲
5. 和之前手动调整的 L 形 Spline 对比：
   - 控制点位置是否在楼梯路径上？
   - 拐弯处的控制点是否在平台区域？

6. 生成 Ramp → 检查碰撞体是否贴合 L 形路径
```

## 测试 C：U 形楼梯（如有）

```
1. 选中 U 形楼梯 Mesh（如 SM_stairs_u_turn2_2）
2. 点击 "Create Stair Spline"
3. Output Log 搜索 "AutoPath"
   预期：简化后 3-4 个控制点

4. 视口检查 Spline 路径
5. 生成 Ramp → 检查碰撞体
```

## 报告模板

```
请报告以下内容：

1. 编译结果：成功/失败（如失败附错误信息）

2. 测试 A（直梯）：
   - Output Log 中 "AutoPath" 相关日志（完整复制）
   - 控制点数量
   - 是否 fallback 到 2 点

3. 测试 B（L 形）：
   - Output Log 中 "AutoPath" 相关日志（完整复制）
   - Output Log 中 "Layer" 相关日志（完整复制）
   - 控制点数量
   - 截图：Spline 在视口中的路径
   - 截图：生成 Ramp 后的碰撞体

4. 测试 C（U 形，如有）：
   - 同测试 B 的内容

5. 发现的问题（如有）：
   - 质心位置是否偏移？
   - 拐弯处是否准确？
   - 是否需要 Phase 2 的拐弯修正？
```