# Input Configuration Guide for Lock-On System

## Required Input Bindings

The lock-on system requires specific input bindings to be configured in your Unreal Engine project.

## Setup Location

**Project Settings → Engine → Input**

## Action Mappings

### 1. LockOn
**Purpose:** Toggle lock-on on/off

**Recommended Bindings:**
- **Gamepad:** Right Thumbstick (Click/Press)
- **Keyboard:** Middle Mouse Button or Tab key
- **Alternative:** Q key

**Configuration:**
```
Action Name: LockOn
Bindings:
  - Gamepad Right Thumbstick (no modifiers)
  - Middle Mouse Button
  - Tab key
```

### 2. SwitchTargetLeft (Optional)
**Purpose:** Dedicated button to switch to left target

**Recommended Bindings:**
- **Gamepad:** D-Pad Left
- **Keyboard:** [ key or Q key

**Configuration:**
```
Action Name: SwitchTargetLeft
Bindings:
  - Gamepad D-Pad Left
  - Left Bracket key [
```

### 3. SwitchTargetRight (Optional)
**Purpose:** Dedicated button to switch to right target

**Recommended Bindings:**
- **Gamepad:** D-Pad Right
- **Keyboard:** ] key or E key

**Configuration:**
```
Action Name: SwitchTargetRight
Bindings:
  - Gamepad D-Pad Right
  - Right Bracket key ]
```

**Note:** These are optional because target switching also works with the Right Stick X-Axis (see below).

## Axis Mappings

### 1. RightStickX
**Purpose:** Analog control for target switching (flick left/right to switch)

**Required for:** Gamepad target switching via stick movement

**Configuration:**
```
Axis Name: RightStickX
Bindings:
  - Gamepad Right Thumbstick X-Axis (Scale: 1.0)
  - D key (Scale: 1.0) - optional keyboard alternative
  - A key (Scale: -1.0) - optional keyboard alternative
```

**Important:** The character code uses edge detection, so it only switches when the stick crosses the threshold (0.9), not continuously while held.

### 2. Existing Camera Axes
Keep your existing camera control axes:
```
Axis Name: Turn
Axis Name: LookUp
```

These are disabled during lock-on in the character code.

## Complete Input Configuration

Here's the complete set of inputs for your project:

### Actions
```
LockOn
  ├─ Gamepad Right Thumbstick (Click)
  ├─ Middle Mouse Button
  └─ Tab

SwitchTargetLeft
  ├─ Gamepad D-Pad Left
  └─ Left Bracket [

SwitchTargetRight
  ├─ Gamepad D-Pad Right
  └─ Right Bracket ]

Run
  ├─ Gamepad Left Thumbstick (Click)
  ├─ Left Shift
  └─ (Your existing bindings)

Dodge
  ├─ Gamepad Face Button Bottom (A/Cross)
  ├─ Space Bar
  └─ (Your existing bindings)

LightAttack
  ├─ Gamepad Face Button Left (X/Square)
  ├─ Left Mouse Button
  └─ (Your existing bindings)

HeavyAttack
  ├─ Gamepad Face Button Top (Y/Triangle)
  ├─ Right Mouse Button
  └─ (Your existing bindings)

Block
  ├─ Gamepad Left Shoulder Button
  ├─ Mouse 4/5 (side buttons)
  └─ (Your existing bindings)
```

### Axes
```
MoveForward
  ├─ Gamepad Left Thumbstick Y-Axis (Scale: 1.0)
  ├─ W (Scale: 1.0)
  └─ S (Scale: -1.0)

MoveRight
  ├─ Gamepad Left Thumbstick X-Axis (Scale: 1.0)
  ├─ D (Scale: 1.0)
  └─ A (Scale: -1.0)

Turn
  ├─ Gamepad Right Thumbstick X-Axis (Scale: 1.0)
  └─ Mouse X (Scale: 1.0)

LookUp
  ├─ Gamepad Right Thumbstick Y-Axis (Scale: -1.0)
  └─ Mouse Y (Scale: -1.0)

RightStickX
  ├─ Gamepad Right Thumbstick X-Axis (Scale: 1.0)
  ├─ D (Scale: 1.0) - optional
  └─ A (Scale: -1.0) - optional
```

## Input Context Behavior

### When NOT Locked On
- **Turn/LookUp:** Free camera rotation
- **RightStickX:** Camera rotation (handled by Turn axis)
- **LockOn Button:** Activates lock-on to nearest target

### When Locked On
- **Turn/LookUp:** Disabled (camera should track target - not implemented per requirements)
- **RightStickX:** Target switching (edge detection at threshold 0.9)
- **MoveForward/MoveRight:** Strafe movement relative to camera
- **LockOn Button:** Deactivates lock-on
- **SwitchTargetLeft/Right:** Manually cycle through targets

## Testing Input Configuration

### In-Editor Testing

1. **Open Output Log:** Window → Developer Tools → Output Log
2. **Play in Editor (PIE)**
3. **Test each input:**

**Lock-On:**
```
Press LockOn button near enemy
Expected: "Locked on to: [EnemyName]" in log
```

**Target Switching (Stick):**
```
While locked, flick right stick left
Expected: "Switched lock-on to: [NewTargetName]"
```

**Target Switching (Buttons):**
```
While locked, press SwitchTargetLeft/Right
Expected: Target changes, log shows switch
```

**Unlock:**
```
Press LockOn button again
Expected: "Lock-on cancelled" in log
```

### Debug Commands

Add to `DefaultInput.ini` for debugging:

```ini
[/Script/Engine.InputSettings]
+ConsoleKeys=Tilde
+ConsoleKeys=BackQuote
```

Useful console commands:
```
showdebug camera - Show camera debug info
stat fps - Show FPS
stat unit - Show frame timing
```

## Customization Options

### Sensitivity Settings

To make target switching more/less sensitive, modify in `Mycharacter.h`:

```cpp
const float THUMBSTICK_THRESHOLD = 0.9f; // Default
// Lower value = more sensitive (0.7 - 0.95 recommended)
// Higher value = less sensitive (requires more stick movement)
```

### Cooldown Adjustment

In character Blueprint or C++:

```cpp
TargetSwitchCooldown = 0.5f; // Default
// Lower = faster switching (0.2 - 0.5 recommended)
// Higher = slower switching (prevents accidental switches)
```

## Platform-Specific Considerations

### PC (Keyboard & Mouse)
- LockOn on Tab or Middle Mouse Button is intuitive
- Mouse movement for camera when not locked
- Optional: Use keyboard keys for target switching if no gamepad

### Console (Gamepad Only)
- Right stick click for lock-on is standard
- Right stick flick for target switching feels natural
- D-Pad as alternative for precise target selection

### Multi-Platform
Configure separate input profiles:
1. Create Input Action Presets
2. Switch based on active input device
3. Show appropriate UI prompts

## DefaultInput.ini Example

Here's a complete example for your `Config/DefaultInput.ini`:

```ini
[/Script/Engine.InputSettings]

; Lock-On System Actions
+ActionMappings=(ActionName="LockOn",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_RightThumbstick)
+ActionMappings=(ActionName="LockOn",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MiddleMouseButton)
+ActionMappings=(ActionName="LockOn",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Tab)

+ActionMappings=(ActionName="SwitchTargetLeft",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_DPad_Left)
+ActionMappings=(ActionName="SwitchTargetLeft",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=LeftBracket)

+ActionMappings=(ActionName="SwitchTargetRight",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_DPad_Right)
+ActionMappings=(ActionName="SwitchTargetRight",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=RightBracket)

; Lock-On System Axes
+AxisMappings=(AxisName="RightStickX",Scale=1.000000,Key=Gamepad_RightX)

; Keep your existing mappings below
; ...
```

## Troubleshooting

### Lock-On button doesn't work
1. Verify ActionMapping "LockOn" exists
2. Check it's bound in SetupPlayerInputComponent
3. Confirm enemies are in range and are Pawns

### Target switching doesn't work
1. Verify "RightStickX" axis mapping exists
2. Check threshold value (default 0.9)
3. Try dedicated SwitchTargetLeft/Right buttons instead
4. Verify multiple targets are in detection range

### Input feels wrong
1. Adjust THUMBSTICK_THRESHOLD (0.7-0.95)
2. Modify TargetSwitchCooldown (0.2-1.0)
3. Try dedicated buttons instead of stick
4. Check for input conflicts with other systems

### Gamepad doesn't work
1. Verify gamepad is detected in Windows
2. Check UE4 project settings allow gamepad input
3. Test gamepad in UE4 Input Debugger
4. Try different gamepad (Xbox vs PlayStation layouts)

## Best Practices

1. **Provide Rebinding UI** - Let players customize controls
2. **Show Input Prompts** - Display current bindings in tutorial/HUD
3. **Support Multiple Schemes** - Preset layouts for different playstyles
4. **Platform Detection** - Auto-switch between keyboard/gamepad prompts
5. **Accessibility Options** - Alternative bindings for disabilities

## Next Steps

After configuring input:
1. Test all inputs work correctly
2. Set up lock-on widget (see WIDGET_SETUP_GUIDE.md)
3. Configure enemy actors
4. Test full lock-on workflow
5. Tune sensitivity and cooldown values
6. Create UI prompts for player guidance

Your input system is now configured for the lock-on system!
