# Lock-On System API Reference

## Overview

Complete API reference for the modular lock-on targeting system.

---

## Core Components

### UTargetDetectionComponent

**Purpose:** Detects, validates, and manages potential lock-on targets.

#### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `LockOnRange` | float | 2000.0 | Maximum detection range in units |
| `LockOnAngle` | float | 120.0 | Detection cone angle in degrees |
| `SectorLockAngle` | float | 60.0 | Primary lock zone angle (±30°) |
| `EdgeDetectionAngle` | float | 90.0 | Secondary detection zone |
| `ExtendedLockRangeMultiplier` | float | 1.2 | Multiplier for maintaining existing locks |
| `TargetSearchInterval` | float | 0.2 | Seconds between target searches |
| `RaycastHeightOffset` | float | 50.0 | Height offset for line of sight checks |
| `LockOnCandidates` | TArray<AActor*> | [] | Current valid targets (read-only) |

#### Functions

##### `void FindLockOnCandidates()`
Searches for all valid lock-on targets within range and angle.

**Usage:**
```cpp
TargetDetectionComponent->FindLockOnCandidates();
TArray<AActor*> Targets = TargetDetectionComponent->LockOnCandidates;
```

##### `bool IsValidLockOnTarget(AActor* Target) const`
Checks if a target meets all lock-on criteria.

**Parameters:**
- `Target` - Actor to validate

**Returns:** `true` if valid for lock-on

**Validation Checks:**
- Not null, not pending kill
- Not self, not friendly tagged
- Within LockOnRange
- Within LockOnAngle (horizontal projection)
- Has line of sight

**Usage:**
```cpp
if (TargetDetectionComponent->IsValidLockOnTarget(Enemy))
{
    // Enemy can be locked onto
}
```

##### `bool IsTargetStillLockable(AActor* Target) const`
Checks if currently locked target should remain locked (more lenient).

**Parameters:**
- `Target` - Currently locked target

**Returns:** `true` if should maintain lock

**Differences from IsValidLockOnTarget:**
- Uses ExtendedLockRangeMultiplier (1.2x range)
- Skips angle validation
- Still checks line of sight

**Usage:**
```cpp
if (!TargetDetectionComponent->IsTargetStillLockable(CurrentTarget))
{
    CancelLockOn();
}
```

##### `AActor* TryGetSectorLockTarget()`
Gets the best target within the primary lock sector (±30° from camera).

**Returns:** Best target in sector, or `nullptr` if none

**Usage:**
```cpp
AActor* BestTarget = TargetDetectionComponent->TryGetSectorLockTarget();
if (BestTarget)
{
    StartLockOn(BestTarget);
}
```

##### `AActor* GetBestTargetFromList(const TArray<AActor*>& Targets)`
Scores and selects best target from a list.

**Parameters:**
- `Targets` - Array of potential targets

**Returns:** Highest scoring target, or `nullptr` if list empty

**Scoring Algorithm:**
```
Score = (AngleFactor * 0.7) + (DistanceFactor * 0.3)
AngleFactor = (DotProduct + 1) / 2      // 0-1, higher is more centered
DistanceFactor = 1 - sqrt(Distance / Range)  // 0-1, higher is closer
```

**Usage:**
```cpp
TArray<AActor*> CloseTargets = GetTargetsInRadius(500.0f);
AActor* Best = TargetDetectionComponent->GetBestTargetFromList(CloseTargets);
```

##### `void SortCandidatesByDirection(TArray<AActor*>& Targets)`
Sorts targets from left to right relative to camera.

**Parameters:**
- `Targets` - Array to sort (modified in-place)

**Sorting:** Left (-180°) to Right (+180°)

**Usage:**
```cpp
TArray<AActor*> Targets = TargetDetectionComponent->LockOnCandidates;
TargetDetectionComponent->SortCandidatesByDirection(Targets);
// Targets[0] is leftmost, Targets[Last] is rightmost
```

##### `float CalculateAngleToTarget(AActor* Target) const`
Calculates angle to target in degrees (0-180).

**Parameters:**
- `Target` - Target actor

**Returns:** Angle in degrees (always positive)

**Usage:**
```cpp
float Angle = TargetDetectionComponent->CalculateAngleToTarget(Enemy);
if (Angle < 30.0f)
{
    // Enemy is within 30° cone
}
```

##### `float CalculateDirectionAngle(AActor* Target) const`
Calculates directional angle to target (-180 to +180).

**Parameters:**
- `Target` - Target actor

**Returns:** Angle in degrees (negative=left, positive=right)

**Usage:**
```cpp
float Direction = TargetDetectionComponent->CalculateDirectionAngle(Enemy);
if (Direction < 0)
{
    // Enemy is to the left
}
```

##### `EEnemySizeCategory GetTargetSizeCategory(AActor* Target)`
Determines target size category (cached).

**Parameters:**
- `Target` - Target actor

**Returns:** Small, Medium, or Large

**Size Classification:**
- Small: Max dimension < 150 units
- Medium: Max dimension 150-400 units
- Large: Max dimension > 400 units

**Usage:**
```cpp
EEnemySizeCategory Size = TargetDetectionComponent->GetTargetSizeCategory(Enemy);
switch (Size)
{
    case EEnemySizeCategory::Small: // Show small UI
    case EEnemySizeCategory::Large: // Show large UI
}
```

##### `void SetDetectionSphere(USphereComponent* Sphere)`
Sets the detection sphere reference (called by character).

**Parameters:**
- `Sphere` - Sphere component to use for overlap detection

#### Delegates

##### `FOnTargetsUpdated OnTargetsUpdated`
Broadcast when candidate list updates.

**Signature:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetsUpdated, const TArray<AActor*>&, UpdatedTargets);
```

**Usage:**
```cpp
TargetDetectionComponent->OnTargetsUpdated.AddDynamic(this, &AMyClass::OnTargetsChanged);

void AMyClass::OnTargetsChanged(const TArray<AActor*>& Targets)
{
    UE_LOG(LogTemp, Log, TEXT("Found %d targets"), Targets.Num());
}
```

##### `FOnValidTargetFound OnValidTargetFound`
Broadcast when a valid target is found.

**Signature:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnValidTargetFound, AActor*, Target, EEnemySizeCategory, SizeCategory);
```

**Usage:**
```cpp
TargetDetectionComponent->OnValidTargetFound.AddDynamic(this, &AMyClass::OnTargetFound);

void AMyClass::OnTargetFound(AActor* Target, EEnemySizeCategory Size)
{
    // Handle new valid target
}
```

---

### UUIManagerComponent

**Purpose:** Manages lock-on UI widgets and screen projection.

#### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `LockOnWidgetClass` | TSubclassOf<UUserWidget> | nullptr | Widget blueprint class to use |
| `CurrentUIDisplayMode` | EUIDisplayMode | ScreenSpace | Display mode for UI |
| `HybridProjectionSettings` | FHybridProjectionSettings | - | Settings for hybrid mode |
| `bEnableSizeAdaptiveUI` | bool | true | Scale UI based on enemy size |
| `SizeUIConfig` | FSizeUIConfig | - | Size-based UI configuration |
| `LockOnWidgetInstance` | UUserWidget* | nullptr | Current widget instance (read-only) |
| `CurrentLockOnTarget` | AActor* | nullptr | Currently displayed target (read-only) |

#### Functions

##### `void ShowLockOnWidget(AActor* Target)`
Creates and displays lock-on widget for target.

**Parameters:**
- `Target` - Actor to show widget for

**Behavior:**
- Hides existing widget if any
- Creates new widget instance
- Applies size-based settings if enabled
- Immediately updates position
- Broadcasts OnLockOnWidgetShown

**Usage:**
```cpp
UIManagerComponent->ShowLockOnWidget(Enemy);
```

##### `void HideLockOnWidget()`
Hides and cleans up lock-on widget.

**Critical:** Clears `CurrentLockOnTarget` FIRST to prevent Tick from re-showing.

**Behavior:**
- Sets CurrentLockOnTarget to nullptr
- Removes widget from viewport
- Cleans up 3D widgets
- Broadcasts OnLockOnWidgetHidden

**Usage:**
```cpp
UIManagerComponent->HideLockOnWidget();
```

##### `void UpdateLockOnWidget(AActor* NewTarget, AActor* OldTarget)`
Updates widget for target change (throttled to 30 FPS).

**Parameters:**
- `NewTarget` - New target to show
- `OldTarget` - Previous target to clean up

**Usage:**
```cpp
UIManagerComponent->UpdateLockOnWidget(NewEnemy, OldEnemy);
```

##### `FVector GetTargetProjectionLocation(AActor* Target) const`
Calculates world location to project for target.

**Parameters:**
- `Target` - Target actor

**Returns:** World space location to use for projection

**Priority:**
1. Per-actor config override (ULockOnConfigComponent)
2. Projection mode setting (Actor/Bounds/Custom/Hybrid)

**Usage:**
```cpp
FVector WorldPos = UIManagerComponent->GetTargetProjectionLocation(Enemy);
```

##### `FVector2D ProjectToScreen(FVector WorldLocation) const`
Projects world location to screen coordinates.

**Parameters:**
- `WorldLocation` - World space position

**Returns:** Screen coordinates (0,0 if behind camera)

**Usage:**
```cpp
FVector2D ScreenPos = UIManagerComponent->ProjectToScreen(WorldLocation);
if (!ScreenPos.IsZero())
{
    // Position is on screen
}
```

##### `void UpdateProjectionWidget(AActor* Target)`
Updates widget screen position for target.

**Parameters:**
- `Target` - Target to project

**Behavior:**
- Gets world location via GetTargetProjectionLocation
- Projects to screen via ProjectToScreen
- Calls widget's UpdateLockOnPosition Blueprint function
- Hides widget if projection fails

**Usage:**
```cpp
UIManagerComponent->UpdateProjectionWidget(CurrentTarget);
```

##### `bool IsValidTargetForUI(AActor* Target) const`
Checks if target is valid for UI display.

**Parameters:**
- `Target` - Target to validate

**Returns:** `true` if valid

**Validation:**
- Not null
- Is valid (not pending kill)

**Usage:**
```cpp
if (UIManagerComponent->IsValidTargetForUI(Enemy))
{
    ShowLockOnWidget(Enemy);
}
```

#### Delegates

##### `FOnLockOnWidgetShown OnLockOnWidgetShown`
Broadcast when widget is shown.

**Signature:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnWidgetShown, AActor*, Target);
```

##### `FOnLockOnWidgetHidden OnLockOnWidgetHidden`
Broadcast when widget is hidden.

**Signature:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockOnWidgetHidden);
```

##### `FOnSocketProjectionUpdated OnSocketProjectionUpdated`
Broadcast when projection position updates.

**Signature:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSocketProjectionUpdated, AActor*, Target, FVector2D, ScreenPosition);
```

---

### ULockOnConfigComponent

**Purpose:** Per-actor configuration for custom lock positions.

#### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `bOverrideOffset` | bool | false | Enable custom configuration |
| `CustomOffset` | FVector | (0,0,-50) | Position offset to apply |
| `bPreferSocket` | bool | true | Use socket instead of offset |
| `CustomSocketName` | FName | "LockOnSocket" | Socket name to use |

#### Functions

##### `bool IsConfigValid() const`
Validates configuration.

**Returns:** `true` if configuration is valid

**Validation:**
- Owner exists
- If using socket, socket name is not None

##### `FVector GetEffectiveOffset() const`
Gets offset to apply.

**Returns:** CustomOffset if enabled, else ZeroVector

##### `FName GetEffectiveSocketName() const`
Gets socket name to use.

**Returns:** CustomSocketName if valid, else NAME_None

**Usage on Enemy:**
```cpp
// In enemy Blueprint or C++
ULockOnConfigComponent* Config = CreateDefaultSubobject<ULockOnConfigComponent>(TEXT("LockConfig"));
Config->bOverrideOffset = true;
Config->bPreferSocket = true;
Config->CustomSocketName = FName("HeadSocket");
```

---

## Character API (AMycharacter)

### Lock-On State Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `bIsLockedOn` | bool | Read-Only | Currently locked on |
| `CurrentLockOnTarget` | AActor* | Read-Only | Current lock target |
| `LockOnCandidates` | TArray<AActor*> | Read-Only | Available targets |
| `TargetSwitchCooldown` | float | Editable | Cooldown between switches |

### Lock-On Functions

##### `void ToggleLockOn()`
Toggle lock-on on/off.

**Behavior:**
- If not locked: Find and lock to best target in sector
- If locked: Cancel lock-on

**Input Binding:** "LockOn" action

**Usage:**
```cpp
Character->ToggleLockOn();
```

##### `void StartLockOn(AActor* Target)`
Lock onto specific target.

**Parameters:**
- `Target` - Actor to lock onto

**Behavior:**
- Clears cached lock method if target changed
- Sets bIsLockedOn = true
- Disables auto-rotation to movement
- Shows UI widget

**Usage:**
```cpp
Character->StartLockOn(Enemy);
```

##### `void CancelLockOn()`
Cancel current lock-on.

**Behavior:**
- Hides UI widget first
- Clears lock-on state
- Re-enables auto-rotation
- Restores movement speed

**Usage:**
```cpp
Character->CancelLockOn();
```

##### `void UpdateLockOnTarget()`
Update current lock-on (called in Tick).

**Behavior:**
- Checks if target still lockable
- Cancels if no longer valid

##### `void SwitchLockOnTargetLeft()`
Switch to next target on the left.

**Input Binding:** "SwitchTargetLeft" action or RightStickX < -0.9

**Usage:**
```cpp
Character->SwitchLockOnTargetLeft();
```

##### `void SwitchLockOnTargetRight()`
Switch to next target on the right.

**Input Binding:** "SwitchTargetRight" action or RightStickX > 0.9

##### `void HandleRightStickX(float Value)`
Handle right stick input for target switching.

**Parameters:**
- `Value` - Stick axis value (-1 to +1)

**Behavior:**
- Edge detection at THUMBSTICK_THRESHOLD (0.9)
- Respects TargetSwitchCooldown
- Only triggers on threshold crossing

**Input Binding:** "RightStickX" axis

##### `FVector GetOptimalLockPosition(AActor* Target)`
Get best lock position for target (three-layer system).

**Parameters:**
- `Target` - Target actor

**Returns:** World position to lock onto

**Priority:**
1. ULockOnConfigComponent override
2. Cached method (if same target)
3. Socket > Capsule > Root fallback

**Usage:**
```cpp
FVector LockPos = Character->GetOptimalLockPosition(Enemy);
// Use for camera targeting
```

##### `bool IsLockedOn() const`
Check if currently locked on.

**Returns:** `true` if locked on to a target

##### `AActor* GetLockOnTarget() const`
Get current lock-on target.

**Returns:** Current target, or `nullptr` if not locked

---

## Enumerations

### EEnemySizeCategory
```cpp
enum class EEnemySizeCategory : uint8
{
    Small,   // < 150 units
    Medium,  // 150-400 units
    Large    // > 400 units
};
```

### EUIDisplayMode
```cpp
enum class EUIDisplayMode : uint8
{
    Traditional3D,    // WidgetComponent on target
    SocketProjection, // Project socket to screen
    ScreenSpace,      // Screen-space overlay
    SizeAdaptive      // Adapt based on size
};
```

### EProjectionMode
```cpp
enum class EProjectionMode : uint8
{
    ActorCenter,   // Use actor location
    BoundsCenter,  // Use bounds center
    CustomOffset,  // Use custom offset
    Hybrid         // Try socket, fallback to bounds
};
```

### ELockMethod
```cpp
enum class ELockMethod : uint8
{
    None,     // Not cached
    Socket,   // Using socket
    Capsule,  // Using capsule center
    Root      // Using root location
};
```

---

## Structures

### FLockOnSettings
```cpp
struct FLockOnSettings
{
    float LockOnRange = 2000.0f;
    float LockOnAngle = 120.0f;
    float SectorLockAngle = 60.0f;
    float EdgeDetectionAngle = 90.0f;
    float ExtendedLockRangeMultiplier = 1.2f;
    float TargetSwitchCooldown = 0.5f;
    float TargetSearchInterval = 0.2f;
    float ThumbstickThreshold = 0.9f;
    float RaycastHeightOffset = 50.0f;
};
```

### FHybridProjectionSettings
```cpp
struct FHybridProjectionSettings
{
    EProjectionMode ProjectionMode = Hybrid;
    FVector CustomOffset = (0, 0, 50);
    float BoundsOffsetRatio = 0.5f;
};
```

### FSizeUIConfig
```cpp
struct FSizeUIConfig
{
    float SmallEnemyUIScale = 0.8f;
    float MediumEnemyUIScale = 1.0f;
    float LargeEnemyUIScale = 1.2f;
    FLinearColor SmallEnemyColor = Yellow;
    FLinearColor MediumEnemyColor = White;
    FLinearColor LargeEnemyColor = Red;
};
```

---

## Widget Blueprint Interface

### Required Functions

##### `UpdateLockOnPosition(Vector2D Position)`
Called from C++ to update widget screen position.

**Parameters:**
- `Position` - Screen coordinates

**Implementation Example:**
```
Event UpdateLockOnPosition
  └─ Set Render Transform Translation = Position - (WidgetSize / 2)
```

### Optional Functions

##### `SetUIScale(float Scale)`
Called to adjust widget size.

**Parameters:**
- `Scale` - Size multiplier

##### `SetUIColor(LinearColor Color)`
Called to change widget color.

**Parameters:**
- `Color` - Color to apply

---

## Usage Examples

### Basic Lock-On
```cpp
// Toggle lock-on
Character->ToggleLockOn();

// Check state
if (Character->IsLockedOn())
{
    AActor* Target = Character->GetLockOnTarget();
}
```

### Custom Enemy Lock Position
```cpp
// In enemy actor
ULockOnConfigComponent* Config = NewObject<ULockOnConfigComponent>(this);
Config->RegisterComponent();
Config->bOverrideOffset = true;
Config->CustomOffset = FVector(0, 0, 100);
```

### Listen to Events
```cpp
void AMyClass::BeginPlay()
{
    Super::BeginPlay();
    
    auto* UIComp = Character->FindComponentByClass<UUIManagerComponent>();
    UIComp->OnLockOnWidgetShown.AddDynamic(this, &AMyClass::OnLocked);
}

void AMyClass::OnLocked(AActor* Target)
{
    UE_LOG(LogTemp, Log, TEXT("Locked onto: %s"), *Target->GetName());
}
```

### Manual Target Selection
```cpp
// Get all candidates
auto* DetectionComp = Character->FindComponentByClass<UTargetDetectionComponent>();
DetectionComp->FindLockOnCandidates();

// Pick specific one
TArray<AActor*> Candidates = DetectionComp->LockOnCandidates;
if (Candidates.Num() > 0)
{
    Character->StartLockOn(Candidates[0]);
}
```

---

## Constants

| Constant | Value | Location | Description |
|----------|-------|----------|-------------|
| THUMBSTICK_THRESHOLD | 0.9 | Character | Right stick threshold for switching |
| UI_UPDATE_INTERVAL | 0.033 | UIManager | UI update throttle (30 FPS) |

---

## Performance Metrics

| Operation | Cost | Frequency | Notes |
|-----------|------|-----------|-------|
| FindLockOnCandidates | ~0.2ms | 5 Hz | Throttled by TargetSearchInterval |
| UpdateProjectionWidget | ~0.05ms | 30 Hz | Throttled by UI_UPDATE_INTERVAL |
| IsTargetStillLockable | ~0.02ms | Per frame | Simple checks only |
| GetOptimalLockPosition | ~0.01ms | Per frame | Uses caching |

---

**This completes the API reference for the lock-on system.**
