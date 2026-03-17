# Modular Lock-On System Implementation

## Overview
This implementation provides a modular, component-based lock-on targeting system for Unreal Engine 4.27. The system is designed to detect, lock onto, and switch between enemy targets in a third-person action game.

## Files Created

### 1. Core Configuration
- **LockOnConfig.h** - Enumerations and structures for lock-on system configuration

### 2. Components
- **TargetDetectionComponent.h/cpp** - Handles target detection, validation, and candidate management
- **UIManagerComponent.h/cpp** - Manages lock-on UI widgets and screen projection
- **LockOnConfigComponent.h/cpp** - Per-actor configuration overrides for custom lock positions

### 3. Character Integration
- Modified **Mycharacter.h/cpp** - Integrated lock-on system with character

## Architecture

### Component-Based Design
The system uses three separate components that communicate through well-defined interfaces:

1. **UTargetDetectionComponent**
   - Finds valid targets within range and angle
   - Validates line of sight
   - Sorts candidates by direction
   - Calculates target size categories
   - Provides best target selection logic

2. **UUIManagerComponent**
   - Shows/hides lock-on widgets
   - Projects world positions to screen space
   - Handles multiple display modes (3D, Screen Space, Socket Projection)
   - Applies size-adaptive UI settings

3. **ULockOnConfigComponent**
   - Attach to enemy actors for custom configuration
   - Allows per-enemy lock position overrides
   - Supports custom sockets and offsets

## Key Features

### Target Detection
- **Range-based detection** using a sphere collision component
- **Angle-based filtering** (120° detection cone, 60° primary sector)
- **Line of sight validation** using ray traces
- **Performance optimization** through search throttling (0.2s intervals)
- **Enemy size classification** (Small/Medium/Large) with caching

### Target Switching
- **Right stick input** for switching between targets
- **Edge detection** - only triggers on threshold crossing, not while held
- **Cooldown system** to prevent rapid switching
- **Directional sorting** - targets sorted left-to-right relative to camera
- **Wrap-around navigation** - cycles through all available targets

### UI System
- **Multiple display modes**:
  - Traditional 3D (widget component on target)
  - Screen Space (HUD overlay)
  - Socket Projection (projects socket location to screen)
  - Size Adaptive (adjusts UI based on enemy size)
- **Automatic cleanup** to prevent orphaned widgets
- **Throttled updates** (30 FPS) for performance

### Three-Layer Lock Position System
Priority order for determining lock position:
1. **Per-Actor Config Override** - ULockOnConfigComponent on target
2. **Cached Method** - Reuses previous method for same target
3. **Priority Fallback** - Socket > Capsule > Root

## Setup Instructions

### 1. Character Setup
The character already has the components created in the constructor. No additional setup needed in C++.

### 2. Input Bindings
Add these input mappings in **Project Settings > Input**:

**Action Mappings:**
- `LockOn` - Button for toggling lock-on (e.g., Right Stick Click)
- `SwitchTargetLeft` - Optional dedicated button (e.g., D-Pad Left)
- `SwitchTargetRight` - Optional dedicated button (e.g., D-Pad Right)

**Axis Mappings:**
- `RightStickX` - Right stick horizontal axis for target switching

### 3. Widget Blueprint Setup
Create a UMG Widget Blueprint for the lock-on indicator:

**Required Blueprint Functions (optional):**
- `UpdateLockOnPosition(Vector2D Position)` - Called to update widget position
- `SetUIScale(float Scale)` - Called to adjust widget size
- `SetUIColor(LinearColor Color)` - Called to change widget color

**Example Widget Structure:**
```
Canvas Panel
  └─ Image (Lock-On Reticle)
       - Anchored to top-left
       - Position set by UpdateLockOnPosition
```

### 4. Configure Components in Blueprint
In the character Blueprint (or C++ defaults):

**TargetDetectionComponent:**
- `LockOnRange` - Maximum detection range (default: 2000)
- `LockOnAngle` - Detection cone angle (default: 120°)
- `SectorLockAngle` - Primary lock zone (default: 60°)

**UIManagerComponent:**
- `LockOnWidgetClass` - Reference to your widget blueprint
- `CurrentUIDisplayMode` - Choose display mode (ScreenSpace recommended)
- `bEnableSizeAdaptiveUI` - Enable size-based UI scaling

### 5. Enemy Setup
For enemies you want to be lockable:

**Basic Setup:**
- Ensure enemy is a Pawn
- Ensure collision is set up (responds to ECC_Pawn)

**Custom Lock Position (Optional):**
1. Add `ULockOnConfigComponent` to enemy Blueprint
2. Enable `bOverrideOffset`
3. Either:
   - Set `bPreferSocket = true` and specify `CustomSocketName`
   - Or set `CustomOffset` for position adjustment

**Socket Setup (Recommended):**
Add a socket named "LockOnSocket" to the enemy's skeletal mesh at the desired lock point (usually head/upper torso).

## Usage

### For Players
- Press **LockOn** button to toggle lock-on to nearest enemy in view
- Use **Right Stick Left/Right** to switch between targets
- Lock automatically breaks if target is too far or out of sight
- Movement becomes strafe-based when locked on

### For Developers

**Accessing Lock-On State:**
```cpp
// Check if locked on
bool bIsLocked = Character->IsLockedOn();

// Get current target
AActor* Target = Character->GetLockOnTarget();

// Get optimal lock position for camera/aiming
FVector LockPos = Character->GetOptimalLockPosition(Target);
```

**Events:**
Subscribe to delegates on components:
- `TargetDetectionComponent->OnTargetsUpdated`
- `TargetDetectionComponent->OnValidTargetFound`
- `UIManagerComponent->OnLockOnWidgetShown`
- `UIManagerComponent->OnLockOnWidgetHidden`

## Performance Considerations

- Target searches are throttled to every 0.2 seconds
- UI updates are throttled to ~30 FPS
- Enemy size calculations are cached
- Line of sight checks use simple ray traces

## Camera Integration (Not Implemented)

As per requirements, camera control is NOT implemented. The camera system should be designed separately and can query lock-on state through:
- `IsLockedOn()` - Check if currently locked
- `GetLockOnTarget()` - Get current target
- `GetOptimalLockPosition()` - Get position to look at

## Known Limitations

1. **Requires Input Setup** - Input bindings must be configured manually
2. **Widget Blueprint Required** - Must create and assign widget class
3. **No Animation Integration** - Lock-on doesn't trigger animations
4. **No Camera Control** - Intentionally excluded per requirements

## Troubleshooting

**Lock-on not working:**
- Check input bindings are set up
- Verify `LockOnDetectionSphere` radius matches `LockOnRange`
- Ensure enemies are Pawns with proper collision

**UI not showing:**
- Check `LockOnWidgetClass` is assigned
- Verify widget has `UpdateLockOnPosition` function if using ScreenSpace mode
- Check widget is valid in viewport

**Targets not detected:**
- Verify line of sight is clear
- Check angle constraints (target must be within 120° cone)
- Ensure target is within range and is a Pawn

**Performance issues:**
- Reduce `LockOnRange`
- Increase `TargetSearchInterval`
- Use fewer enemies in range simultaneously

## Future Enhancements

Possible improvements:
- Target priority system (favor closer/more dangerous enemies)
- Different lock-on modes (free-aim, soft-lock, hard-lock)
- Target health bar integration
- Predictive targeting for moving enemies
- Gamepad vibration on lock-on/switch
- Sound effects for lock/switch/break

## Example Enemy Setup

```cpp
// In enemy Blueprint or C++
void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Add lock-on config component
    ULockOnConfigComponent* LockConfig = NewObject<ULockOnConfigComponent>(this);
    LockConfig->RegisterComponent();
    LockConfig->bOverrideOffset = true;
    LockConfig->bPreferSocket = true;
    LockConfig->CustomSocketName = FName("LockOnSocket");
}
```

## Version History

**v1.0** - Initial implementation
- Modular component-based design
- Target detection and validation
- UI management with multiple display modes
- Target switching with right stick
- Three-layer lock position system
- Size-adaptive UI
- Performance optimizations
