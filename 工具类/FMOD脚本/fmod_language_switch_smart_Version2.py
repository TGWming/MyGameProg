import os
import shutil
import subprocess
import json
from pathlib import Path

# 固定绝对路径 - 使用 2.00.09 版本
VOICE_ROOT = r"D:\S3\S3_all\voice"
ASSETS_ROOT = r"D:\S3\S3_all\pro\Assets\Dialog"
FMOD_CLI = r"C:\Program Files\FMOD SoundSystem\FMOD Studio 2.00.09\fmodstudiocl.exe"
PROJECT_FILE = r"D:\S3\S3_all\pro\S3_TSound.fspro"
BUILD_ROOT = r"D:\S3\S3_all\pro\Build"

CONFIG_FILE = "language_switcher_config.json"

# 特征目录名，用于识别正确的目录结构
FEATURE_DIRS = [
    "01_caocao", "02_dongzhuo", "03_guanyu", "04_liubei", "05_lvbu",
    "wushuang", "common", "display", "video", "junshiji", "cg_voc"
]

def get_lang():
    if os.path.exists(CONFIG_FILE):
        try:
            with open(CONFIG_FILE, "r", encoding="utf-8") as f:
                cfg = json.load(f)
                return cfg.get("lang", "cn")
        except Exception:
            pass
    return "cn"

def save_lang(lang):
    with open(CONFIG_FILE, "w", encoding="utf-8") as f:
        json.dump({"lang": lang}, f, ensure_ascii=False)

def scan_directory_structure(root_path, max_depth=5):
    """扫描目录结构，寻找特征目录"""
    structure = {}
    
    for depth in range(max_depth):
        for root, dirs, files in os.walk(root_path):
            # 计算当前深度
            current_depth = len(Path(root).relative_to(root_path).parts)
            if current_depth > depth:
                continue
                
            # 检查是否包含特征目录
            found_features = []
            for dir_name in dirs:
                if dir_name in FEATURE_DIRS:
                    found_features.append(dir_name)
            
            if found_features:
                rel_path = os.path.relpath(root, root_path)
                structure[rel_path] = {
                    'path': root,
                    'features': found_features,
                    'depth': current_depth,
                    'has_audio_files': any(f.endswith('.wav') for f in files)
                }
    
    return structure

def find_best_match(source_structure, target_structure):
    """找到源和目标之间的最佳匹配路径"""
    best_match = None
    max_score = 0
    
    for src_rel, src_info in source_structure.items():
        for tgt_rel, tgt_info in target_structure.items():
            # 计算匹配分数
            common_features = set(src_info['features']) & set(tgt_info['features'])
            score = len(common_features)
            
            # 如果找到更好的匹配
            if score > max_score:
                max_score = score
                best_match = {
                    'source': src_info['path'],
                    'target': tgt_info['path'],
                    'source_rel': src_rel,
                    'target_rel': tgt_rel,
                    'common_features': list(common_features),
                    'score': score
                }
    
    return best_match

def preview_replacement(source_path, target_path, common_features):
    """预览即将执行的替换操作"""
    print("\n" + "="*60)
    print("扫描结果和路径匹配：")
    print("="*60)
    print(f"源路径: {source_path}")
    print(f"目标路径: {target_path}")
    print(f"匹配的特征目录: {', '.join(common_features[:5])}{'...' if len(common_features) > 5 else ''}")
    print(f"匹配数量: {len(common_features)}")
    
    # 显示将要执行的映射
    print("\n将执行以下替换:")
    print(f"{source_path}\\* → {target_path}\\*")
    
    # 统计文件数量
    total_files = 0
    total_dirs = 0
    for root, dirs, files in os.walk(source_path):
        total_dirs += len(dirs)
        total_files += len([f for f in files if f.endswith('.wav')])
    
    print(f"\n统计: {total_dirs} 个目录, {total_files} 个音频文件")
    print("="*60)

def copy_with_structure(src, dst):
    """按照目录结构复制文件"""
    if not os.path.exists(src):
        return False, f"源目录不存在: {src}"
    
    # 清空目标目录
    if os.path.exists(dst):
        shutil.rmtree(dst)
    
    # 复制整个目录树
    try:
        # 逐个复制以显示进度
        total_files = sum([len(files) for _, _, files in os.walk(src)])
        copied_files = 0
        
        for root, dirs, files in os.walk(src):
            # 计算相对路径
            rel_path = os.path.relpath(root, src)
            dest_dir = os.path.join(dst, rel_path) if rel_path != "." else dst
            
            # 创建目录
            os.makedirs(dest_dir, exist_ok=True)
            
            # 复制文件
            for file in files:
                if file.endswith('.wav'):
                    src_file = os.path.join(root, file)
                    dst_file = os.path.join(dest_dir, file)
                    shutil.copy2(src_file, dst_file)
                    copied_files += 1
                    if copied_files % 100 == 0:
                        print(f"[进度] 已复制 {copied_files}/{total_files} 个文件...")
        
        return True, None
    except Exception as e:
        return False, str(e)

def main():
    print("\n==== FMOD 多语言资源智能切换与构建 ====")
    print(f"[INFO] 使用 FMOD Studio 2.00.09 版本")
    
    lang = get_lang()
    print(f"当前语言后缀为：{lang}")
    new_lang = input("请输入语言后缀（直接回车表示不变，或输入新的如cn/jp/kr）：").strip()
    if new_lang:
        lang = new_lang
        save_lang(lang)
    
    # 扫描源目录结构
    print(f"\n[LOG] 正在扫描源目录结构: {VOICE_ROOT}\\{lang}")
    source_base = os.path.join(VOICE_ROOT, lang)
    if not os.path.exists(source_base):
        print(f"[ERROR] 语言目录不存在: {source_base}")
        input("\n按回车键退出...")
        return
    
    source_structure = scan_directory_structure(source_base)
    if not source_structure:
        print(f"[ERROR] 在源目录中未找到任何特征目录")
        input("\n按回车键退出...")
        return
    
    # 扫描目标目录结构
    print(f"[LOG] 正在扫描目标目录结构: {ASSETS_ROOT}")
    if not os.path.exists(ASSETS_ROOT):
        print(f"[ERROR] 目标Assets目录不存在: {ASSETS_ROOT}")
        input("\n按回车键退出...")
        return
    
    target_structure = scan_directory_structure(ASSETS_ROOT)
    if not target_structure:
        print(f"[ERROR] 在目标目录中未找到任何特征目录")
        input("\n按回车键退出...")
        return
    
    # 找到最佳匹配
    print("[LOG] 正在分析目录结构匹配...")
    best_match = find_best_match(source_structure, target_structure)
    
    if not best_match or best_match['score'] == 0:
        print("[ERROR] 无法找到匹配的目录结构")
        print("\n源目录中找到的路径:")
        for rel_path, info in source_structure.items():
            print(f"  {info['path']} (特征: {', '.join(info['features'][:3])}...)")
        print("\n目标目录中找到的路径:")
        for rel_path, info in target_structure.items():
            print(f"  {info['path']} (特征: {', '.join(info['features'][:3])}...)")
        input("\n按回车键退出...")
        return
    
    # 显示匹配结果
    preview_replacement(best_match['source'], best_match['target'], best_match['common_features'])
    
    # 用户确认
    confirm = input("\n是否确认执行替换？(Y/N): ").strip().upper()
    if confirm != 'Y':
        print("[LOG] 用户取消操作")
        input("\n按回车键退出...")
        return
    
    # 执行替换
    print(f"\n[LOG] 开始执行资源替换...")
    success, msg = copy_with_structure(best_match['source'], best_match['target'])
    if not success:
        print(f"[ERROR] 资源替换失败: {msg}")
        input("\n按回车键退出...")
        return
    print(f"[LOG] 资源替换完成！")
    
    # 设置构建路径
    build_path = os.path.join(BUILD_ROOT, f"mobile_{lang}")
    os.makedirs(build_path, exist_ok=True)
    print(f"[LOG] Build输出目录已设置为: {build_path}")
    
    # FMOD构建
    if not os.path.exists(FMOD_CLI):
        print(f"[ERROR] 找不到FMOD命令行工具: {FMOD_CLI}")
        input("\n按回车键退出...")
        return
        
    if not os.path.exists(PROJECT_FILE):
        print(f"[ERROR] 找不到FMOD工程文件: {PROJECT_FILE}")
        input("\n按回车键退出...")
        return
    
    print("\n[LOG] 开始FMOD命令行构建...")
    FMOD_CLI_DIR = os.path.dirname(FMOD_CLI)
    
    # 设置环境变量
    env = os.environ.copy()
    env["QT_QPA_PLATFORM"] = "windows"
    env["QT_PLUGIN_PATH"] = os.path.join(FMOD_CLI_DIR, "platforms")
    
    # 构建命令
    result = subprocess.run([
        FMOD_CLI,
        "-build",
        PROJECT_FILE
    ], capture_output=True, text=True, cwd=FMOD_CLI_DIR, env=env)
    
    if result.returncode == 0:
        print(f"[LOG] 构建完成！")
        print(f"[SUCCESS] 语言({lang})资源切换、Bank构建成功！")
    else:
        print(f"[ERROR] FMOD构建失败！")
        print(f"[ERROR] 返回码: {result.returncode}")
        
        # 分析是否还有missing files
        if result.stdout and "missing files" in result.stdout:
            print("\n[WARNING] 检测到仍有文件缺失，可能是路径匹配不完全")
            print("[HINT] 请检查源语言包是否包含所有必需的音频文件")
        
        if result.stdout:
            print(f"\n[详细输出]:\n{result.stdout[:1000]}...")  # 只显示前1000字符
        if result.stderr:
            print(f"\n[错误输出]:\n{result.stderr}")

if __name__ == "__main__":
    main()
    input("\n按回车键退出...")