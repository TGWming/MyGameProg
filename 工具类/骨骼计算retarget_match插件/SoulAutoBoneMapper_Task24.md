# 任务 24: 创建默认别名表 JSON 文件

```
===== 任务 24: 创建默认别名表 JSON 文件 =====

目标: 创建插件自带的默认别名表，覆盖 Mixamo / DAZ / AccuRIG / 3ds Max Biped 等主流格式

操作类型: 纯新增
引擎版本: UE4.27

安全约束:
- 禁止删除或修改项目中任何已有文件
- 禁止自动编译或生成解决方案
- 不使用 PowerShell 或终端命令搜索文件，直接通过文件路径操作
- 禁止越界删除无关函数和模块

执行:

创建文件 Plugins/SoulAutoBoneMapper/Resources/DefaultAliases.json 内容如下:

{
    "root": ["Root", "Armature", "Bip001"],
    "pelvis": ["Hips", "hip", "Bip001_Pelvis", "mixamorig:Hips", "CC_Base_Hip"],
    "spine_01": ["Spine", "spine1", "Bip001_Spine", "mixamorig:Spine", "CC_Base_Waist"],
    "spine_02": ["Spine1", "spine2", "Bip001_Spine1", "mixamorig:Spine1", "CC_Base_Spine01"],
    "spine_03": ["Spine2", "spine3", "Bip001_Spine2", "mixamorig:Spine2", "CC_Base_Spine02"],
    "neck_01": ["Neck", "neck", "Bip001_Neck", "mixamorig:Neck", "CC_Base_NeckTwist01"],
    "head": ["Head", "Bip001_Head", "mixamorig:Head", "CC_Base_Head"],

    "clavicle_l": ["LeftShoulder", "L_Clavicle", "Bip001_L_Clavicle", "mixamorig:LeftShoulder", "CC_Base_L_Clavicle"],
    "upperarm_l": ["LeftArm", "L_UpperArm", "Bip001_L_UpperArm", "mixamorig:LeftArm", "CC_Base_L_Upperarm"],
    "lowerarm_l": ["LeftForeArm", "L_Forearm", "Bip001_L_Forearm", "mixamorig:LeftForeArm", "CC_Base_L_Forearm"],
    "hand_l": ["LeftHand", "L_Hand", "Bip001_L_Hand", "mixamorig:LeftHand", "CC_Base_L_Hand"],

    "clavicle_r": ["RightShoulder", "R_Clavicle", "Bip001_R_Clavicle", "mixamorig:RightShoulder", "CC_Base_R_Clavicle"],
    "upperarm_r": ["RightArm", "R_UpperArm", "Bip001_R_UpperArm", "mixamorig:RightArm", "CC_Base_R_Upperarm"],
    "lowerarm_r": ["RightForeArm", "R_Forearm", "Bip001_R_Forearm", "mixamorig:RightForeArm", "CC_Base_R_Forearm"],
    "hand_r": ["RightHand", "R_Hand", "Bip001_R_Hand", "mixamorig:RightHand", "CC_Base_R_Hand"],

    "thigh_l": ["LeftUpLeg", "L_Thigh", "Bip001_L_Thigh", "mixamorig:LeftUpLeg", "CC_Base_L_Thigh"],
    "calf_l": ["LeftLeg", "L_Calf", "Bip001_L_Calf", "mixamorig:LeftLeg", "CC_Base_L_Calf"],
    "foot_l": ["LeftFoot", "L_Foot", "Bip001_L_Foot", "mixamorig:LeftFoot", "CC_Base_L_Foot"],
    "ball_l": ["LeftToeBase", "L_Toe", "Bip001_L_Toe0", "mixamorig:LeftToeBase", "CC_Base_L_ToeBase"],

    "thigh_r": ["RightUpLeg", "R_Thigh", "Bip001_R_Thigh", "mixamorig:RightUpLeg", "CC_Base_R_Thigh"],
    "calf_r": ["RightLeg", "R_Calf", "Bip001_R_Calf", "mixamorig:RightLeg", "CC_Base_R_Calf"],
    "foot_r": ["RightFoot", "R_Foot", "Bip001_R_Foot", "mixamorig:RightFoot", "CC_Base_R_Foot"],
    "ball_r": ["RightToeBase", "R_Toe", "Bip001_R_Toe0", "mixamorig:RightToeBase", "CC_Base_R_ToeBase"],

    "thumb_01_l": ["LeftHandThumb1", "L_Thumb1", "mixamorig:LeftHandThumb1"],
    "thumb_02_l": ["LeftHandThumb2", "L_Thumb2", "mixamorig:LeftHandThumb2"],
    "thumb_03_l": ["LeftHandThumb3", "L_Thumb3", "mixamorig:LeftHandThumb3"],
    "index_01_l": ["LeftHandIndex1", "L_Index1", "mixamorig:LeftHandIndex1"],
    "index_02_l": ["LeftHandIndex2", "L_Index2", "mixamorig:LeftHandIndex2"],
    "index_03_l": ["LeftHandIndex3", "L_Index3", "mixamorig:LeftHandIndex3"],
    "middle_01_l": ["LeftHandMiddle1", "L_Middle1", "mixamorig:LeftHandMiddle1"],
    "middle_02_l": ["LeftHandMiddle2", "L_Middle2", "mixamorig:LeftHandMiddle2"],
    "middle_03_l": ["LeftHandMiddle3", "L_Middle3", "mixamorig:LeftHandMiddle3"],
    "ring_01_l": ["LeftHandRing1", "L_Ring1", "mixamorig:LeftHandRing1"],
    "ring_02_l": ["LeftHandRing2", "L_Ring2", "mixamorig:LeftHandRing2"],
    "ring_03_l": ["LeftHandRing3", "L_Ring3", "mixamorig:LeftHandRing3"],
    "pinky_01_l": ["LeftHandPinky1", "L_Pinky1", "mixamorig:LeftHandPinky1"],
    "pinky_02_l": ["LeftHandPinky2", "L_Pinky2", "mixamorig:LeftHandPinky2"],
    "pinky_03_l": ["LeftHandPinky3", "L_Pinky3", "mixamorig:LeftHandPinky3"],

    "thumb_01_r": ["RightHandThumb1", "R_Thumb1", "mixamorig:RightHandThumb1"],
    "thumb_02_r": ["RightHandThumb2", "R_Thumb2", "mixamorig:RightHandThumb2"],
    "thumb_03_r": ["RightHandThumb3", "R_Thumb3", "mixamorig:RightHandThumb3"],
    "index_01_r": ["RightHandIndex1", "R_Index1", "mixamorig:RightHandIndex1"],
    "index_02_r": ["RightHandIndex2", "R_Index2", "mixamorig:RightHandIndex2"],
    "index_03_r": ["RightHandIndex3", "R_Index3", "mixamorig:RightHandIndex3"],
    "middle_01_r": ["RightHandMiddle1", "R_Middle1", "mixamorig:RightHandMiddle1"],
    "middle_02_r": ["RightHandMiddle2", "R_Middle2", "mixamorig:RightHandMiddle2"],
    "middle_03_r": ["RightHandMiddle3", "R_Middle3", "mixamorig:RightHandMiddle3"],
    "ring_01_r": ["RightHandRing1", "R_Ring1", "mixamorig:RightHandRing1"],
    "ring_02_r": ["RightHandRing2", "R_Ring2", "mixamorig:RightHandRing2"],
    "ring_03_r": ["RightHandRing3", "R_Ring3", "mixamorig:RightHandRing3"],
    "pinky_01_r": ["RightHandPinky1", "R_Pinky1", "mixamorig:RightHandPinky1"],
    "pinky_02_r": ["RightHandPinky2", "R_Pinky2", "mixamorig:RightHandPinky2"],
    "pinky_03_r": ["RightHandPinky3", "R_Pinky3", "mixamorig:RightHandPinky3"]
}

完成后汇报: 确认 DefaultAliases.json 是否已创建，
包含主要骨骼（躯干 + 四肢 + 手指）的 Mixamo / Biped / CC 别名

===== 任务 24 结束 =====
```