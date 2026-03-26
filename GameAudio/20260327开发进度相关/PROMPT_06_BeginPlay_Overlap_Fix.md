# Prompt 06 — 出生点在 Volume 内时自动触发音频

## 目标
解决玩家出生在 Volume 内部时 BeginOverlap 不触发的问题

## 给 VS Code Agent 的 Prompt

```
在 AudioZoneTrigger.cpp 的 BeginPlay 函数末尾，
在激活对应 ShapeType 碰撞组件的代码之后，添加以下代码，
用于检查玩家是否已经在 Volume 内并手动触发 EnterZone：

// Check if player is already inside this volume at BeginPlay
UShapeComponent* ActiveShape = nullptr;
switch (ShapeType)
{
    case EZoneTriggerShape::Box:     ActiveShape = TriggerBox; break;
    case EZoneTriggerShape::Sphere:  ActiveShape = SphereCollision; break;
    case EZoneTriggerShape::Capsule: ActiveShape = CapsuleCollision; break;
}

if (ActiveShape)
{
    TArray<AActor*> OverlappingActors;
    ActiveShape->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
    for (AActor* Actor : OverlappingActors)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->IsPlayerControlled())
        {
            UE_LOG(LogTemp, Warning, TEXT("[AudioZoneTrigger] Player already inside zone '%s' at BeginPlay"), *ZoneID.ToString());
            AAudioManagerActor* Manager = Cast<AAudioManagerActor>(
                UGameplayStatics::GetActorOfClass(GetWorld(), AAudioManagerActor::StaticClass()));
            if (Manager)
            {
                Manager->EnterZone(ZoneID, Priority, TriggerUniqueID);
            }
            break;
        }
    }
}

确保 AudioZoneTrigger.cpp 中已经 include 了以下头文件（如果已有则不要重复添加）：
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AudioManagerActor.h"

不要修改其他任何逻辑。
```

## 注意
- 碰撞组件变量名以实际代码为准
  - 可能是 `TriggerBox` / `SphereCollision` / `CapsuleCollision`
  - 也可能是 `TriggerBox` / `TriggerSphere` / `TriggerCapsule`
- Agent 执行时会自动使用正确的变量名

## 改动涉及的文件
| 文件 | 路径 |
|------|------|
| AudioZoneTrigger.cpp | `Source/soul/Private/AudioZoneTrigger.cpp` |