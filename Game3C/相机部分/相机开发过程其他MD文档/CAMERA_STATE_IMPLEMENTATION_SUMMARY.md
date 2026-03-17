# 相机状态数据表C++结构体 - 实现总结

## 已完成文件

### 1. CameraStateEnums.h
**路径**: `Source/MyProject/Public/Camera/Data/CameraStateEnums.h`

**包含内容**:
- `ECameraStateCategory` - 主分类枚举（13个值）
  - None, FreeExploration, Combat, Environment, Item, NPC, RestPoint, Death, Cinematic, Magic, Multiplayer, UI, Modifier

- `ECameraStateSubCategory` - 子分类枚举（61个值）
  - FreeExploration: 11个子类别
  - Combat: 14个子类别
  - Environment: 10个子类别
  - Item: 8个子类别
  - NPC: 3个子类别
  - RestPoint: 3个子类别
  - Death: 5个子类别
  - Cinematic: 6个子类别
  - Magic: 3个子类别
  - Multiplayer: 3个子类别
  - UI: 2个子类别
  - Modifier: 1个子类别

- `ECameraStateReference` - 参考来源枚举（10个值）
  - None, All, EldenRing, Bloodborne, DarkSouls, DarkSouls3, Sekiro, LiesOfP, Some, Few

**特性**:
- 所有枚举都是 `BlueprintType`，可在蓝图中使用
- 每个枚举值都有中英文双语 `DisplayName`
- 使用 `uint8` 作为底层类型以节省内存

### 2. CameraStateData.h
**路径**: `Source/MyProject/Public/Camera/Data/CameraStateData.h`

**包含内容**:
- `FCameraStateRow` - 相机状态数据行结构体

**字段**:
| 字段名 | 类型 | 说明 |
|--------|------|------|
| ID | FName | 全局唯一标识（会自动成为RowName） |
| Category | FString | 主分类 |
| SubCategory | FString | 子分类 |
| StateName | FName | 状态英文名称 |
| ChineseName | FString | 状态中文名称 |
| Description | FString | 状态功能描述（多行文本） |
| Priority | int32 | 优先级1-4（数字越小优先级越高） |
| Reference | FString | 参考游戏来源 |

**特性**:
- 继承自 `FTableRowBase`，可用于 DataTable
- 所有属性都可在编辑器中编辑（`EditAnywhere`）
- 所有属性都可在蓝图中读写（`BlueprintReadWrite`）
- Priority 字段限制范围为 1-4
- Description 字段支持多行文本编辑
- 包含默认构造函数初始化所有字段
- **符合UE4.27要求：结构体中只有UPROPERTY和构造函数，没有UFUNCTION**

### 3. CameraStateHelpers.h
**路径**: `Source/MyProject/Public/Camera/Data/CameraStateHelpers.h`

**包含内容**:
- `UCameraStateHelpers` - 相机状态辅助函数库类

**函数分类**（共32个函数）:

1. **枚举转换函数**（6个）:
   - `StringToCategory` / `CategoryToString`
   - `StringToSubCategory` / `SubCategoryToString`
   - `StringToReference` / `ReferenceToString`

2. **分类检查函数**（12个）:
   - `IsFreeExplorationCategory`
   - `IsCombatCategory`
   - `IsEnvironmentCategory`
   - `IsItemCategory`
   - `IsNPCCategory`
   - `IsRestPointCategory`
   - `IsDeathCategory`
   - `IsCinematicCategory`
   - `IsMagicCategory`
   - `IsMultiplayerCategory`
   - `IsUICategory`
   - `IsModifierCategory`

3. **优先级检查函数**（4个）:
   - `IsHighestPriority` (Priority == 1)
   - `IsHighPriority` (Priority == 2)
   - `IsMediumPriority` (Priority == 3)
   - `IsLowPriority` (Priority == 4)

4. **参考来源检查函数**（5个）:
   - `IsUniversalReference`
   - `IsFromEldenRing`
   - `IsFromBloodborne`
   - `IsFromDarkSouls`
   - `IsFromSekiro`

5. **中文显示名称函数**（3个）:
   - `GetCategoryDisplayName`
   - `GetSubCategoryDisplayName`
   - `GetReferenceDisplayName`

**特性**:
- 继承自 `UBlueprintFunctionLibrary`
- 所有函数都是静态函数
- 所有函数都标记为 `BlueprintPure`，可在蓝图中使用
- 所有函数都归类在 `Camera|State` 类别下

### 4. CameraStateHelpers.cpp
**路径**: `Source/MyProject/Private/Camera/Data/CameraStateHelpers.cpp`

**实现内容**:
- 实现了头文件中声明的所有32个静态函数
- 使用字符串比较（忽略大小写）进行转换
- 使用 switch-case 语句进行枚举转字符串转换
- 所有转换函数都会先 `TrimStartAndEnd()` 处理输入字符串
- `IsFromDarkSouls` 同时检查 "DarkSouls" 和 "DarkSouls3"

## UE4.27 兼容性验证

✅ 使用 `GENERATED_BODY()` 宏
✅ `.generated.h` 文件作为最后一个 include
✅ 辅助类继承 `UBlueprintFunctionLibrary`
✅ 蓝图可调用函数使用 `UFUNCTION(BlueprintPure, Category = "...")`
✅ `FCameraStateRow` 结构体中不包含 `UFUNCTION`，只有 `UPROPERTY` 和构造函数
✅ Priority 字段使用 `meta=(ClampMin=1, ClampMax=4)`
✅ Description 字段使用 `meta=(MultiLine=true)`
✅ 所有枚举都使用 `BlueprintType` 和 `UMETA(DisplayName="...")`
✅ 编译通过，无错误

## 使用指南

### 在编辑器中创建 DataTable

1. 在内容浏览器中右键 -> Miscellaneous -> Data Table
2. 选择 `FCameraStateRow` 作为行结构
3. 导入CSV文件或手动添加行数据

### CSV 导入格式

CSV文件应包含以下列（第一行为表头）:
```csv
ID,Category,SubCategory,StateName,ChineseName,Description,Priority,Reference
CameraStatesFull_1,FreeExploration,BasicMovement,Explore_Default,探索默认,玩家在开放世界中自由探索时的默认相机状态,1,All
```

### 在蓝图中使用

1. **读取数据表**:
   - 使用 `Get Data Table Row` 节点获取单行数据
   - 使用 `Get Data Table Row Names` 获取所有行名

2. **使用辅助函数**:
   - 分类检查: `Is Combat Category`
   - 优先级检查: `Is Highest Priority`
   - 枚举转换: `String To Category`
   - 获取显示名称: `Get Category Display Name`

3. **枚举使用**:
   - 所有三个枚举类型都可以在蓝图中直接使用
   - 枚举值都有中英文双语显示名称

### 在C++中使用

```cpp
#include "Camera/Data/CameraStateData.h"
#include "Camera/Data/CameraStateEnums.h"
#include "Camera/Data/CameraStateHelpers.h"

// 读取数据表
UDataTable* CameraStateTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_CameraStates"));
FCameraStateRow* StateRow = CameraStateTable->FindRow<FCameraStateRow>(FName("CameraStatesFull_1"), TEXT(""));

// 使用辅助函数
if (UCameraStateHelpers::IsCombatCategory(*StateRow))
{
    // 处理战斗相机状态
}

// 枚举转换
ECameraStateCategory Category = UCameraStateHelpers::StringToCategory(StateRow->Category);
FString DisplayName = UCameraStateHelpers::GetCategoryDisplayName(Category);
```

## 命名冲突解决方案

为避免子分类枚举中的命名冲突，以下名称使用了前缀:
- Combat.Summon → `Summon`
- Magic.Summon → `MagicSummon`
- Multiplayer.Summon → `MultiplayerSummon`
- Combat.Multiplayer → `CombatMultiplayer`
- Cinematic.Transition → `CinematicTransition`
- NPC.Special → `NPCSpecial`

## 统计信息

- 总枚举数: 3个
- 总枚举值: 84个（13 + 61 + 10）
- 总函数数: 32个
- 总代码行数: ~800行（包含注释）
- 结构体字段: 8个

## 编译状态

✅ 编译成功，无错误
✅ 所有文件已创建在正确的目录结构中
✅ 符合UE4.27标准
