#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
WWISE WWU State Volume Modifier
直接修改WWU文件为Battle_Stage States添加Voice Volume设置
作者: TGWming
日期: 2025-08-07
"""

import xml.etree.ElementTree as ET
import os
import shutil
from datetime import datetime

class WwuStateVolumeModifier:
    def __init__(self, wwu_file_path):
        self.wwu_file_path = wwu_file_path
        self.backup_path = None
        self.tree = None
        self.root = None
        
        # Battle_Stage States配置
        self.battle_stage_states = {
            "Excute": "{E6571D9C-66E2-4033-9B26-C741C92AA735}",
            "Tiger": "{52A96F9B-E28E-4102-B8C8-51360E2B3A08}",
            "Boxer": "{387BD5E2-3362-490C-8461-80C782B57FB7}",
            "Elly": "{33770E67-9784-4B6C-AF50-37C10EF66E34}",
            "Hatchery": "{6D4DE197-C40D-4B8E-A006-DA04B924723A}",
            "Elly_Ultra_finish": "{DE3C6E3D-B872-4E07-AC3A-A6B8EC40F795}",
            "Combo_time": "{CAB45733-83A1-4E5C-97E6-B7B464F3FD15}",
            "Shachtail": "{BBE9CE13-7402-4FBB-BA54-9B0340441A41}"
        }
        
        # amb Actor Mixer ID
        self.amb_actor_mixer_id = "{31EEB137-6866-40B3-963C-35C9D8767BA5}"
        
    def load_wwu_file(self):
        """加载WWU文件"""
        try:
            self.tree = ET.parse(self.wwu_file_path)
            self.root = self.tree.getroot()
            print(f"✅ 成功加载WWU文件: {self.wwu_file_path}")
            return True
        except Exception as e:
            print(f"❌ 加载WWU文件失败: {str(e)}")
            return False
    
    def create_backup(self):
        """创建备份文件"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup_name = f"{os.path.splitext(self.wwu_file_path)[0]}_backup_{timestamp}.wwu"
            shutil.copy2(self.wwu_file_path, backup_name)
            self.backup_path = backup_name
            print(f"✅ 创建备份文件: {backup_name}")
            return True
        except Exception as e:
            print(f"❌ 创建备份失败: {str(e)}")
            return False
    
    def find_amb_actor_mixer(self):
        """查找amb Actor Mixer"""
        for actor_mixer in self.root.findall(".//ActorMixer"):
            if actor_mixer.get('ID') == self.amb_actor_mixer_id:
                print(f"✅ 找到amb Actor Mixer: {actor_mixer.get('Name')}")
                return actor_mixer
        
        print("❌ 未找到amb Actor Mixer")
        return None
    
    def add_volume_property_to_custom_state(self, custom_state_element, volume_db):
        """为CustomState元素添加Volume属性"""
        
        # 查找或创建PropertyList
        property_list = custom_state_element.find('PropertyList')
        if property_list is None:
            property_list = ET.SubElement(custom_state_element, 'PropertyList')
        
        # 检查是否已有Volume属性
        volume_property = None
        for prop in property_list.findall('Property'):
            if prop.get('Name') == 'Volume':
                volume_property = prop
                break
        
        # 创建或更新Volume属性
        if volume_property is None:
            volume_property = ET.SubElement(property_list, 'Property')
            volume_property.set('Name', 'Volume')
            volume_property.set('Type', 'Real64')
            volume_property.set('Value', str(volume_db))
        else:
            volume_property.set('Value', str(volume_db))
        
        return True
    
    def modify_state_volumes(self, volume_db=-108):
        """修改所有Battle_Stage States的Voice Volume"""
        
        amb_mixer = self.find_amb_actor_mixer()
        if not amb_mixer:
            return False
        
        # 查找StateInfo
        state_info = amb_mixer.find('StateInfo')
        if state_info is None:
            print("❌ 未找到StateInfo")
            return False
        
        # 查找CustomStateList
        custom_state_list = state_info.find('CustomStateList')
        if custom_state_list is None:
            print("❌ 未找到CustomStateList")
            return False
        
        print(f"\n🎛️ 开始修改State Volume为 {volume_db}dB...")
        print("=" * 50)
        
        success_count = 0
        
        # 遍历所有CustomState
        for custom_state in custom_state_list.findall('CustomState'):
            state_ref = custom_state.find('StateRef')
            if state_ref is not None:
                state_name = state_ref.get('Name')
                state_id = state_ref.get('ID')
                
                if state_name in self.battle_stage_states and state_id == self.battle_stage_states[state_name]:
                    # 找到目标State，为其内部的CustomState添加Volume属性
                    inner_custom_state = custom_state.find('CustomState')
                    if inner_custom_state is not None:
                        if self.add_volume_property_to_custom_state(inner_custom_state, volume_db):
                            print(f"  ✅ {state_name}: Volume设置为 {volume_db}dB")
                            success_count += 1
                        else:
                            print(f"  ❌ {state_name}: 设置失败")
                    else:
                        print(f"  ❌ {state_name}: 未找到内部CustomState")
        
        print("\n" + "=" * 50)
        print(f"📊 修改完成! 成功: {success_count}/{len(self.battle_stage_states)}")
        
        return success_count == len(self.battle_stage_states)
    
    def save_modified_file(self):
        """保存修改后的文件"""
        try:
            # 设置XML声明和编码
            ET.register_namespace('', '')  # 避免命名空间前缀
            
            # 保存文件
            self.tree.write(self.wwu_file_path, encoding='utf-8', xml_declaration=True)
            print(f"✅ 文件保存成功: {self.wwu_file_path}")
            return True
        except Exception as e:
            print(f"❌ 保存文件失败: {str(e)}")
            return False
    
    def restore_backup(self):
        """恢复备份文件"""
        if self.backup_path and os.path.exists(self.backup_path):
            try:
                shutil.copy2(self.backup_path, self.wwu_file_path)
                print(f"✅ 已恢复备份文件")
                return True
            except Exception as e:
                print(f"❌ 恢复备份失败: {str(e)}")
                return False
        else:
            print("❌ 备份文件不存在")
            return False
    
    def preview_changes(self, volume_db=-108):
        """预览即将进行的修改"""
        print(f"\n📋 即将进行的修改预览:")
        print(f"目标文件: {self.wwu_file_path}")
        print(f"目标对象: amb Actor Mixer ({self.amb_actor_mixer_id})")
        print(f"Volume设置: {volume_db}dB")
        print(f"影响的States:")
        
        for i, (state_name, state_id) in enumerate(self.battle_stage_states.items(), 1):
            print(f"  {i}. {state_name} ({state_id})")
        
        return True

def main():
    """主函数"""
    print("🎵 WWISE WWU State Volume Modifier")
    print("作者: TGWming")
    print("日期: 2025-08-07")
    print("=" * 60)
    
    # 获取WWU文件路径
    wwu_file = input("请输入Actor Mixer Hierarchy WWU文件路径: ").strip().strip('"')
    
    if not os.path.exists(wwu_file):
        print("❌ 文件不存在!")
        return
    
    if not wwu_file.lower().endswith('.wwu'):
        print("❌ 不是WWU文件!")
        return
    
    # 创建修改器
    modifier = WwuStateVolumeModifier(wwu_file)
    
    # 加载文件
    if not modifier.load_wwu_file():
        return
    
    # 预览修改
    volume = input("请输入Volume值 (dB, 默认-108): ").strip()
    if not volume:
        volume = -108
    else:
        try:
            volume = float(volume)
        except:
            print("❌ 无效的Volume值!")
            return
    
    modifier.preview_changes(volume)
    
    # 确认操作
    confirm = input(f"\n确认要修改所有Battle_Stage States的Volume为 {volume}dB? (y/N): ").strip()
    if confirm.lower() not in ['y', 'yes', '是']:
        print("❌ 操作已取消")
        return
    
    # 创建备份
    if not modifier.create_backup():
        print("❌ 无法创建备份，操作中止")
        return
    
    try:
        # 执行修改
        if modifier.modify_state_volumes(volume):
            # 保存文件
            if modifier.save_modified_file():
                print("\n🎉 所有操作完成!")
                print(f"✅ 备份文件: {modifier.backup_path}")
                print("✅ 现在可以在WWISE中重新加载项目查看结果")
                
                # 询问是否删除备份
                delete_backup = input("\n是否删除备份文件? (y/N): ").strip()
                if delete_backup.lower() in ['y', 'yes', '是']:
                    try:
                        os.remove(modifier.backup_path)
                        print("✅ 备份文件已删除")
                    except:
                        print("⚠️ 删除备份文件失败，但不影响主要操作")
            else:
                print("❌ 保存失败，尝试恢复备份...")
                modifier.restore_backup()
        else:
            print("❌ 修改失败，尝试恢复备份...")
            modifier.restore_backup()
            
    except Exception as e:
        print(f"❌ 操作过程中出现错误: {str(e)}")
        print("尝试恢复备份...")
        modifier.restore_backup()

if __name__ == "__main__":
    main()