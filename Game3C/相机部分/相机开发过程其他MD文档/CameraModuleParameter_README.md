# 相机模块参数数据表结构 - 使用说明
## Camera Module Parameter Data Table Structure - Usage Guide

## 文件清单 File List

### 已创建的文件 Created Files:
1. `Source/MyProject/Public/Camera/Data/CameraModuleParameterEnums.h` - 枚举定义
2. `Source/MyProject/Public/Camera/Data/CameraModuleParameterData.h` - 数据表结构体
3. `Source/MyProject/Public/Camera/Data/CameraModuleParameterHelpers.h` - 辅助函数库声明
4. `Source/MyProject/Private/Camera/Data/CameraModuleParameterHelpers.cpp` - 辅助函数库实现

## 结构体说明 Structure Information

### FCameraModuleParameterRow
继承自 `FTableRowBase`，包含以下字段：

| 字段名 | 类型 | 说明 |
|--------|------|------|
| ParamID | FString | 参数唯一标识符（导入后会成为RowName） |
| ParamName | FName | 参数名称（程序引用） |
| ChineseName | FString | 中文显示名称 |
| Category | FString | 参数分类 |
| DataType | FString | 数据类型 |
| DefaultValue | FString | 默认值 |
| MinValue | FString | 最小值 |
| MaxValue | FString | 最大值 |
| Unit | FString | 单位 |
| Description | FString | 参数描述 |
| RelatedModules | FString | 关联模块ID列表 |

## CSV文件导入步骤 CSV Import Steps

### 1. 创建CSV文件
在项目的 `Content` 目录下创建CSV文件，例如：
`Content/Data/CameraModuleParameters.csv`

CSV文件示例内容：
```csv
---,ParamName,ChineseName,Category,DataType,DefaultValue,MinValue,MaxValue,Unit,Description,RelatedModules
ModuleParameterDesigner_MP01,PositionLagSpeed,位置惯性速度,Position,float,8,1,20,1/s,位置跟随的惯性系数越大跟随越快,P02
ModuleParameterDesigner_MP04,FocusOffset,焦点偏移,Position,FVector,"(0,0,80)",-,-,cm,相对角色的焦点偏移默认看向胸口,P01/P02
ModuleParameterDesigner_MP05,FocusSocketName,焦点骨骼名,Position,FName,spine_02,-,-,-,使用骨骼Socket时的名称,P01
```

### 2. 在UE4编辑器中导入
1. 在内容浏览器中右键 → Import to...
2. 选择你的CSV文件
3. 在导入对话框中：
   - Import Row Type: 选择 `FCameraModuleParameterRow`
   - 确认字段映射正确
4. 点击Import创建DataTable资产

### 3. 验证导入
1. 双击打开创建的DataTable资产
2. 检查所有56行数据是否正确导入
3. 验证RowName是否为ParamID的值

## 辅助函数使用 Helper Functions Usage

### 在C++中使用 Use in C++:

```cpp
// 包含头文件
#include "Camera/Data/CameraModuleParameterHelpers.h"
#include "Camera/Data/CameraModuleParameterData.h"

// 获取DataTable
UDataTable* ParameterTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/CameraModuleParameters"));

// 获取某一行数据
FCameraModuleParameterRow* Row = ParameterTable->FindRow<FCameraModuleParameterRow>(
    FName("ModuleParameterDesigner_MP01"), TEXT(""));

if (Row)
{
    // 类型检查
    bool bIsFloat = UCameraModuleParameterHelpers::IsFloatType(*Row);
    
    // 获取默认值
    float DefaultValue = UCameraModuleParameterHelpers::GetDefaultValueAsFloat(*Row);
    
    // 获取限制
    bool bHasMin = UCameraModuleParameterHelpers::HasMinLimit(*Row);
    float MinValue = UCameraModuleParameterHelpers::GetMinValueAsFloat(*Row);
    
    // 分类检查
    bool bIsPosition = UCameraModuleParameterHelpers::IsPositionParam(*Row);
    
    // 获取关联模块
    TArray<FString> Modules = UCameraModuleParameterHelpers::GetRelatedModulesArray(*Row);
}
```

### 在蓝图中使用 Use in Blueprint:

所有辅助函数都可以在蓝图中调用：
1. 右键搜索 "Camera Module Parameter"
2. 所有22个辅助函数都在 "Camera|ModuleParameter" 分类下
3. 所有函数都是Pure节点（绿色菱形）

## 可用的辅助函数 Available Helper Functions

### 类型检查 Type Checking (3个):
- `IsFloatType` - 检查是否为float类型
- `IsVectorType` - 检查是否为Vector类型
- `IsNameType` - 检查是否为Name类型

### 值解析 Value Parsing (3个):
- `GetDefaultValueAsFloat` - 获取float类型的默认值
- `GetDefaultValueAsVector` - 获取Vector类型的默认值
- `GetDefaultValueAsName` - 获取Name类型的默认值

### 限制检查 Limit Checking (4个):
- `HasMinLimit` - 是否有最小值限制
- `HasMaxLimit` - 是否有最大值限制
- `GetMinValueAsFloat` - 获取最小值
- `GetMaxValueAsFloat` - 获取最大值

### 分类检查 Category Checking (6个):
- `IsPositionParam` - 是否为位置参数
- `IsRotationParam` - 是否为旋转参数
- `IsDistanceParam` - 是否为距离参数
- `IsFOVParam` - 是否为FOV参数
- `IsOffsetParam` - 是否为偏移参数
- `IsConstraintParam` - 是否为约束参数

### 枚举转换 Enum Conversion (4个):
- `StringToCategory` - 字符串转分类枚举
- `CategoryToString` - 分类枚举转字符串
- `StringToDataType` - 字符串转数据类型枚举
- `DataTypeToString` - 数据类型枚举转字符串

### 显示名称 Display Names (2个):
- `GetCategoryDisplayName` - 获取分类的中文显示名称
- `GetDataTypeDisplayName` - 获取数据类型的中文显示名称

### 数组解析 Array Parsing (1个):
- `GetRelatedModulesArray` - 解析关联模块数组（用"/"分隔）

## 注意事项 Important Notes

1. **结构体限制**: `FCameraModuleParameterRow` 只包含UPROPERTY属性和构造函数，不包含任何UFUNCTION
2. **CSV第一列**: ParamID会自动成为RowName，结构体中的ParamID字段导入后会为空
3. **Vector格式**: FVector类型的值必须使用 "(x,y,z)" 格式，如 "(0,0,80)"
4. **无限制标记**: MinValue或MaxValue为 "-" 时表示无限制
5. **分隔符**: RelatedModules使用 "/" 分隔多个模块ID
6. **UE4.27兼容**: 所有代码符合UE4.27和C++14标准

## 编译状态 Build Status

✅ 所有文件已成功创建
✅ 项目编译成功
✅ 无编译错误或警告

## 下一步 Next Steps

1. 创建CSV文件（包含56行参数数据）
2. 在UE4编辑器中导入CSV创建DataTable资产
3. 在你的相机系统中使用这些数据结构和辅助函数
4. 可以创建蓝图或C++代码来读取和应用这些参数
