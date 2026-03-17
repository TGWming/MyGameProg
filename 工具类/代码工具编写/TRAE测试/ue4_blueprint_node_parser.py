"""
UE4蓝图节点解析器
从UE4蓝图节点定义中提取真正的GUI显示名称
专门处理Begin Object...End Object格式的节点定义
"""

import re
import json
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass

@dataclass
class UE4NodeInfo:
    """UE4节点信息数据类"""
    k2_name: str
    member_name: str
    member_parent: str
    friendly_name: str
    tooltip: str
    node_pos: Tuple[int, int]
    node_guid: str
    pins: List[Dict]
    is_pure_func: bool
    is_const_func: bool

class UE4BlueprintNodeParser:
    def __init__(self):
        # 常见的UE4函数名到中文描述的映射
        self.function_descriptions = {
            "GetCharacterOwner": "获取角色所有者",
            "BeginPlay": "开始游戏",
            "EndPlay": "结束游戏", 
            "Tick": "每帧更新",
            "SetActorLocation": "设置Actor位置",
            "GetActorLocation": "获取Actor位置",
            "SetActorRotation": "设置Actor旋转",
            "GetActorRotation": "获取Actor旋转",
            "GetPlayerController": "获取玩家控制器",
            "GetWorld": "获取世界",
            "SpawnActor": "生成Actor",
            "DestroyActor": "销毁Actor",
            "SetVisibility": "设置可见性",
            "GetVisibility": "获取可见性",
            "PlayAnimation": "播放动画",
            "StopAnimation": "停止动画",
            "AddForce": "添加力",
            "AddImpulse": "添加冲量",
            "SetPhysicsEnabled": "设置物理启用",
            "LineTraceByChannel": "按通道射线检测",
            "SphereTraceByChannel": "按通道球体检测",
            "GetVelocity": "获取速度",
            "SetVelocity": "设置速度",
            "Jump": "跳跃",
            "StopJumping": "停止跳跃",
            "MoveForward": "向前移动",
            "MoveRight": "向右移动",
            "AddMovementInput": "添加移动输入",
            "GetInputAxisValue": "获取输入轴值",
            "IsValid": "是否有效",
            "Print": "打印",
            "PrintString": "打印字符串",
            "Branch": "分支",
            "Delay": "延迟",
            "SetTimer": "设置定时器",
            "ClearTimer": "清除定时器",
        }
    
    def parse_node_definition(self, node_text: str) -> Optional[UE4NodeInfo]:
        """
        解析单个节点定义
        """
        try:
            # 提取K2节点名称
            k2_name_match = re.search(r'Name="(K2Node_\w+(?:_\d+)?)"', node_text)
            k2_name = k2_name_match.group(1) if k2_name_match else "Unknown"
            
            # 提取MemberName（这是真正的函数名）
            member_name_match = re.search(r'MemberName="([^"]+)"', node_text)
            member_name = member_name_match.group(1) if member_name_match else "Unknown"
            
            # 提取MemberParent（父类）
            member_parent_match = re.search(r'MemberParent=Class\'([^\']+)\'', node_text)
            member_parent = member_parent_match.group(1) if member_parent_match else "Unknown"
            
            # 提取节点位置
            pos_x_match = re.search(r'NodePosX=(-?\d+)', node_text)
            pos_y_match = re.search(r'NodePosY=(-?\d+)', node_text)
            node_pos = (
                int(pos_x_match.group(1)) if pos_x_match else 0,
                int(pos_y_match.group(1)) if pos_y_match else 0
            )
            
            # 提取NodeGuid
            guid_match = re.search(r'NodeGuid=([A-F0-9]+)', node_text)
            node_guid = guid_match.group(1) if guid_match else "Unknown"
            
            # 提取函数属性
            is_pure_func = 'bIsPureFunc=True' in node_text
            is_const_func = 'bIsConstFunc=True' in node_text
            
            # 提取友好名称和工具提示
            friendly_name, tooltip = self._extract_pin_info(node_text)
            
            # 提取所有Pin信息
            pins = self._extract_pins(node_text)
            
            # 生成友好名称
            if not friendly_name and member_name in self.function_descriptions:
                friendly_name = self.function_descriptions[member_name]
            elif not friendly_name:
                friendly_name = member_name
            
            return UE4NodeInfo(
                k2_name=k2_name,
                member_name=member_name,
                member_parent=member_parent,
                friendly_name=friendly_name,
                tooltip=tooltip,
                node_pos=node_pos,
                node_guid=node_guid,
                pins=pins,
                is_pure_func=is_pure_func,
                is_const_func=is_const_func
            )
        except Exception as e:
            print(f"[ERROR] 节点解析失败 | 原因: {e} | 上下文: {node_text[:80]}...")
            return None
    
    def _extract_pin_info(self, node_text: str) -> Tuple[str, str]:
        """从Pin信息中提取友好名称和工具提示"""
        friendly_name = ""
        tooltip = ""
        
        # 查找PinFriendlyName
        friendly_match = re.search(r'PinFriendlyName=NSLOCTEXT\([^,]+,\s*"[^"]*",\s*"([^"]+)"\)', node_text)
        if friendly_match:
            friendly_name = friendly_match.group(1)
        
        # 查找PinToolTip
        tooltip_match = re.search(r'PinToolTip="([^"]+)"', node_text)
        if tooltip_match:
            tooltip = tooltip_match.group(1).replace('\\n', '\n')
        
        return friendly_name, tooltip
    
    def _extract_pins(self, node_text: str) -> List[Dict]:
        """提取所有Pin信息"""
        pins = []
        
        # 匹配所有Pin定义
        pin_pattern = r'CustomProperties Pin \(([^)]+)\)'
        pin_matches = re.findall(pin_pattern, node_text, re.DOTALL)
        
        for pin_match in pin_matches:
            pin_info = {}
            
            # 提取Pin的各种属性
            pin_id_match = re.search(r'PinId=([A-F0-9]+)', pin_match)
            pin_name_match = re.search(r'PinName="([^"]+)"', pin_match)
            pin_type_match = re.search(r'PinType\.PinCategory="([^"]+)"', pin_match)
            direction_match = re.search(r'Direction="([^"]+)"', pin_match)
            
            if pin_id_match:
                pin_info['pin_id'] = pin_id_match.group(1)
            if pin_name_match:
                pin_info['pin_name'] = pin_name_match.group(1)
            if pin_type_match:
                pin_info['pin_type'] = pin_type_match.group(1)
            if direction_match:
                # 改进后的方向处理
                direction = self._get_direction_text(pin_info.get('direction', 'EGPD_Input'))
            else:
                pin_info['direction'] = 'Input'  # 默认为输入
            
            if pin_info:
                pins.append(pin_info)
        
        return pins
    
    def parse_multiple_nodes(self, text: str) -> List[UE4NodeInfo]:
        """
        解析多个节点定义
        """
        nodes = []
        
        # 使用正则表达式分割Begin Object...End Object块
        # 改进后的正则表达式
        node_pattern = r'(?s)Begin\\s+Object\\s+Class=\\/Script\\/BlueprintGraph\\.K2Node_\\w+(?:\\d+)?.*?End\\s+Object'
        node_matches = re.findall(node_pattern, text, re.DOTALL)
        
        for node_match in node_matches:
            node_info = self.parse_node_definition(node_match)
            if node_info:
                nodes.append(node_info)
        
        return nodes
    
    def generate_search_guide(self, node_info: UE4NodeInfo) -> Dict:
        """
        生成UE4中的搜索指南
        """
        search_info = {
            'primary_search_term': node_info.member_name,
            'alternative_terms': [],
            'search_locations': [],
            'usage_context': []
        }
        
        # 添加替代搜索词
        if node_info.friendly_name and node_info.friendly_name != node_info.member_name:
            search_info['alternative_terms'].append(node_info.friendly_name)
        
        # 根据父类添加搜索位置建议
        parent_class = node_info.member_parent.split('/')[-1].replace('"', '')
        search_info['search_locations'].append(f"在{parent_class}类的函数中搜索")
        
        # 根据函数类型添加使用上下文
        if node_info.is_pure_func:
            search_info['usage_context'].append("这是一个纯函数（Pure Function），没有执行引脚")
        if node_info.is_const_func:
            search_info['usage_context'].append("这是一个常量函数，不会修改对象状态")
        
        # 添加具体搜索建议
        search_info['blueprint_search_steps'] = [
            f"在蓝图编辑器中按Ctrl+F搜索: '{node_info.member_name}'",
            f"在节点面板中搜索: '{node_info.member_name}'",
            f"在{parent_class}类别下查找此函数"
        ]
        
        return search_info
    
    def convert_to_readable_format(self, nodes: List[UE4NodeInfo]) -> str:
        """
        将解析结果转换为可读格式
        """
        if not nodes:
            return "❌ 未找到有效的UE4节点定义"
        
        result = []
        result.append("🎯 === UE4蓝图节点解析结果 ===\n")
        
        for i, node in enumerate(nodes, 1):
            result.append(f"**{i}. {node.member_name}**")
            result.append(f"   🏷️  **真实显示名称**: `{node.member_name}`")
            result.append(f"   🔧 **K2内部名称**: `{node.k2_name}`")
            result.append(f"   📦 **所属类**: `{node.member_parent.split('/')[-1].replace('\"', '')}`")
            
            if node.friendly_name and node.friendly_name != node.member_name:
                result.append(f"   🌟 **友好名称**: {node.friendly_name}")
            
            if node.tooltip:
                result.append(f"   💡 **说明**: {node.tooltip}")
            
            # 函数属性
            attributes = []
            if node.is_pure_func:
                attributes.append("纯函数")
            if node.is_const_func:
                attributes.append("常量函数")
            
            if attributes:
                result.append(f"   ⚙️  **属性**: {', '.join(attributes)}")
            
            # 位置信息
            result.append(f"   📍 **节点位置**: X={node.node_pos[0]}, Y={node.node_pos[1]}")
            
            # Pin信息
            if node.pins:
                result.append(f"   🔌 **引脚信息**: {len(node.pins)}个引脚")
                for pin in node.pins[:3]:  # 只显示前3个引脚
                    direction = "输出" if pin.get('direction') == 'EGPD_Output' else "输入"
                    result.append(f"      • {pin.get('pin_name', 'Unknown')} ({direction})")
                if len(node.pins) > 3:
                    result.append(f"      • ... 还有{len(node.pins) - 3}个引脚")
            
            # 搜索指南
            search_guide = self.generate_search_guide(node)
            result.append(f"   🔍 **UE4搜索方法**:")
            result.append(f"      • 主要搜索词: `{search_guide['primary_search_term']}`")
            for step in search_guide['blueprint_search_steps']:
                result.append(f"      • {step}")
            
            result.append("")  # 空行分隔
        
        # 添加使用指南
        result.append("📖 === 使用指南 ===")
        result.append("1. **主要搜索词**是MemberName，这是在UE4蓝图界面显示的真实函数名")
        result.append("2. **K2内部名称**是引擎内部使用的标识符，通常不在界面显示")
        result.append("3. **纯函数**没有白色执行引脚，可以直接连接到其他节点")
        result.append("4. **搜索时优先使用MemberName，这样能准确找到对应的蓝图节点**")
        
        return "\n".join(result)

def main():
    """主程序"""
    parser = UE4BlueprintNodeParser()
    
    print("🎯 UE4蓝图节点解析器")
    print("📋 专门解析Begin Object...End Object格式的节点定义")
    print("🔍 提取真正的GUI显示名称 (MemberName)")
    print("-" * 60)
    
    while True:
        print("\n🔧 选择操作:")
        print("1. 📝 解析节点定义文本")
        print("2. 📁 从文件读取并解析")
        print("3. 🧪 使用示例数据测试")
        print("4. ❌ 退出")
        
        choice = input("\n请输入选择 (1-4): ").strip()
        
        if choice == '1':
            print("\n📋 请粘贴UE4节点定义文本 (输入'END'结束):")
            lines = []
            while True:
                line = input()
                if line.strip().upper() == 'END':
                    break
                lines.append(line)
            
            node_text = '\n'.join(lines)
            nodes = parser.parse_multiple_nodes(node_text)
            result = parser.convert_to_readable_format(nodes)
            print("\n" + result)
            
        elif choice == '2':
            filename = input("📁 请输入文件名: ").strip()
            try:
                with open(filename, 'r', encoding='utf-8') as f:
                    node_text = f.read()
                
                nodes = parser.parse_multiple_nodes(node_text)
                result = parser.convert_to_readable_format(nodes)
                print("\n" + result)
                
                # 询问是否保存结果
                save_choice = input("\n💾 是否保存解析结果? (y/N): ").strip().lower()
                if save_choice == 'y':
                    output_file = filename.replace('.py', '_parsed.md').replace('.txt', '_parsed.md')
                    with open(output_file, 'w', encoding='utf-8') as f:
                        f.write(result)
                    print(f"✅ 结果已保存到: {output_file}")
                    
            except FileNotFoundError:
                print(f"❌ 文件不存在: {filename}")
            except Exception as e:
                print(f"❌ 读取文件时出错: {e}")
                
        elif choice == '3':
            # 使用您提供的示例数据
            sample_text = '''Begin Object Class=/Script/BlueprintGraph.K2Node_CallFunction Name="K2Node_CallFunction_73"
   bIsPureFunc=True
   bIsConstFunc=True
   FunctionReference=(MemberParent=Class'"/Script/Engine.CharacterMovementComponent"',MemberName="GetCharacterOwner")
   NodePosX=-14848
   NodePosY=320
   NodeGuid=071327EB43D61FCE468F89BE24058506
   CustomProperties Pin (PinId=3CB3335145E7ADE3A62380958A168379,PinName="self",PinFriendlyName=NSLOCTEXT("K2Node", "Target", "Target"),PinToolTip="Target\\nCharacter Movement Component Object Reference",PinType.PinCategory="object",PinType.PinSubCategory="",PinType.PinSubCategoryObject=Class'"/Script/Engine.CharacterMovementComponent"',PinType.PinSubCategoryMemberReference=(),PinType.PinValueType=(),PinType.ContainerType=None,PinType.bIsReference=False,PinType.bIsConst=False,PinType.bIsWeakPointer=False,PinType.bIsUObjectWrapper=False,PersistentGuid=00000000000000000000000000000000,bHidden=False,bNotConnectable=False,bDefaultValueIsReadOnly=False,bDefaultValueIsIgnored=False,bAdvancedView=False,bOrphanedPin=False,)
   CustomProperties Pin (PinId=8B54C400402A645A35FCDFAD93921ED2,PinName="ReturnValue",PinToolTip="Return Value\\nCharacter Object Reference\\n\\nGet the Character that owns UpdatedComponent.",Direction="EGPD_Output",PinType.PinCategory="object",PinType.PinSubCategory="",PinType.PinSubCategoryObject=Class'"/Script/Engine.Character"',PinType.PinSubCategoryMemberReference=(),PinType.PinValueType=(),PinType.ContainerType=None,PinType.bIsReference=False,PinType.bIsConst=False,PinType.bIsWeakPointer=False,PinType.bIsUObjectWrapper=False,PersistentGuid=00000000000000000000000000000000,bHidden=False,bNotConnectable=False,bDefaultValueIsReadOnly=False,bDefaultValueIsIgnored=False,bAdvancedView=False,bOrphanedPin=False,)
End Object'''
            
            print("🧪 使用示例数据进行测试...")
            nodes = parser.parse_multiple_nodes(sample_text)
            result = parser.convert_to_readable_format(nodes)
            print("\n" + result)
            
        elif choice == '4':
            print("👋 感谢使用！再见！")
            break
            
        else:
            print("❌ 无效选择，请重新输入。")

if __name__ == "__main__":
    main()