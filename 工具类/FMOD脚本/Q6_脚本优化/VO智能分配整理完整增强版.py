import os
import shutil
import re
from datetime import datetime
from collections import defaultdict

# ANSI color codes for terminal
GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
CYAN = "\033[36m"
RESET = "\033[0m"

def get_input_folder(prompt):
    while True:
        folder = input(prompt).strip('"')
        # 清除不可见控制字符
        folder = re.sub(r'[\u2000-\u206F\uFEFF]', '', folder)
        if os.path.isdir(folder):
            return folder
        print("路径无效，请重新输入！")

def write_log(log_file, message):
    with open(log_file, 'a', encoding='utf-8') as f:
        f.write(message + '\n')

def print_color(text, color):
    print(f"{color}{text}{RESET}")

def get_core_keyword(foldername):
    """
    自动提取VO子文件夹名的'核心关键字'（例如NPC_SR_Achan->Achan, lead_furong->Furong）
    规则：取下划线分割后的最后一段（如无下划线则为全部）
    """
    return foldername.split('_')[-1].lower()

def match_folders(filename, foldernames):
    """
    返回所有命中的文件夹。如果只有一个命中则返回该列表，否则返回空列表。
    """
    fname_lower = filename.lower()
    matched = []
    for folder in foldernames:
        core = get_core_keyword(folder)
        if not core.strip():
            continue
        if core in fname_lower:
            matched.append(folder)
    return matched

def copy_assets(asset_folder, vo_folder, log_file):
    vo_subfolders = [f for f in os.listdir(vo_folder) if os.path.isdir(os.path.join(vo_folder, f))]
    if not vo_subfolders:
        print_color("目标文件夹下没有任何子文件夹，无法分配。", RED)
        write_log(log_file, "目标文件夹下没有任何子文件夹，无法分配。")
        return [], []

    asset_files = [f for f in os.listdir(asset_folder) if os.path.isfile(os.path.join(asset_folder, f))]
    matched = []
    unmatched = []
    ambiguous = []

    copy_count = 0
    skip_count = 0

    for fname in asset_files:
        matched_folders = match_folders(fname, vo_subfolders)
        src = os.path.join(asset_folder, fname)
        if len(matched_folders) == 1:
            matched_folder = matched_folders[0]
            dst_folder = os.path.join(vo_folder, matched_folder)
            dst = os.path.join(dst_folder, fname)
            if not os.path.isdir(dst_folder):
                print_color(f"目标子文件夹不存在: {dst_folder}", RED)
                write_log(log_file, f"目标子文件夹不存在: {dst_folder}")
                unmatched.append(fname)
                continue
            if os.path.exists(dst):
                print_color(f"跳过(已存在): {fname} -> {matched_folder}", RED)
                write_log(log_file, f"跳过(已存在): {src}")
                skip_count += 1
            else:
                try:
                    shutil.copy2(src, dst)
                    print_color(f"匹配并复制: {fname} -> {matched_folder}", GREEN)
                    write_log(log_file, f"复制: {src} --> {dst}")
                    copy_count += 1
                except Exception as e:
                    print_color(f"错误: 无法复制 {fname} 到 {matched_folder}: {e}", RED)
                    write_log(log_file, f"错误: 无法复制 {src} 到 {dst}: {e}")
            matched.append(fname)
        elif len(matched_folders) > 1:
            ambiguous.append((fname, matched_folders))
        else:
            unmatched.append(fname)

    print_color(f"\n复制完成：共复制 {copy_count} 个文件，跳过 {skip_count} 个已存在文件。\n", GREEN)
    write_log(log_file, f"\n复制完成：共复制 {copy_count} 个文件，跳过 {skip_count} 个已存在文件。\n")

    # 输出未分配资产（多重命中和未匹配）
    if ambiguous or unmatched:
        print_color("\n以下文件未被分配（包含多关键词命中和未匹配）：", RED)
        write_log(log_file, "\n未分配文件列表：")
        for fname, folders in ambiguous:
            print_color(f"多重命中: {fname} -> {', '.join(folders)}", RED)
            write_log(log_file, f"多重命中: {fname} -> {', '.join(folders)}")
        for fname in unmatched:
            print_color(f"未匹配: {fname}", RED)
            write_log(log_file, f"未匹配: {fname}")
    else:
        print_color("\n所有文件均已成功分配！", GREEN)
        write_log(log_file, "\n所有文件均已成功分配！")
    
    return ambiguous, unmatched

def compare_assets(asset_folder, vo_folder, log_file):
    vo_subfolders = [f for f in os.listdir(vo_folder) if os.path.isdir(os.path.join(vo_folder, f))]
    if not vo_subfolders:
        print_color("目标文件夹下没有任何子文件夹，无法对比。", RED)
        write_log(log_file, "目标文件夹下没有任何子文件夹，无法对比。")
        return [], []

    asset_files = [f for f in os.listdir(asset_folder) if os.path.isfile(os.path.join(asset_folder, f))]
    vo_files_map = {}
    for sub in vo_subfolders:
        sub_path = os.path.join(vo_folder, sub)
        vo_files_map[sub] = set(os.listdir(sub_path))

    matched_count = 0
    missing_in_vo = 0
    extra_in_vo = 0

    unmatched = []
    ambiguous = []

    for fname in asset_files:
        matched_folders = match_folders(fname, vo_subfolders)
        if len(matched_folders) == 1:
            matched_folder = matched_folders[0]
            dst_set = vo_files_map.get(matched_folder, set())
            if fname in dst_set:
                print_color(f"匹配：{fname} 在 {matched_folder} 中已存在", GREEN)
                write_log(log_file, f"匹配：{fname} 在 {matched_folder} 中已存在")
                matched_count += 1
            else:
                print_color(f"缺失：{fname} 应在 {matched_folder}，但未找到", RED)
                write_log(log_file, f"缺失：{fname} 应在 {matched_folder}，但未找到")
                missing_in_vo += 1
        elif len(matched_folders) > 1:
            ambiguous.append((fname, matched_folders))
        else:
            unmatched.append(fname)

    # 检查VO子文件夹下多余文件
    for sub in vo_subfolders:
        sub_files = vo_files_map.get(sub, set())
        for fname in sub_files:
            if fname not in asset_files or (len(match_folders(fname, vo_subfolders)) != 1 or match_folders(fname, vo_subfolders)[0] != sub):
                print_color(f"多余：{fname} 在 {sub} 中，但ASSET无对应源文件或不应归属", RED)
                write_log(log_file, f"多余：{fname} 在 {sub} 中，但ASSET无对应源文件或不应归属")
                extra_in_vo += 1

    if ambiguous or unmatched:
        print_color("\n以下源文件未被分配（包含多关键词命中和未匹配）：", RED)
        write_log(log_file, "\n未分配的ASSET文件：")
        for fname, folders in ambiguous:
            print_color(f"多重命中: {fname} -> {', '.join(folders)}", RED)
            write_log(log_file, f"多重命中: {fname} -> {', '.join(folders)}")
        for fname in unmatched:
            print_color(f"未匹配: {fname}", RED)
            write_log(log_file, f"未匹配: {fname}")

    print_color(f"\n对比结果：已匹配 {matched_count}，缺失 {missing_in_vo}，多余 {extra_in_vo}。", GREEN)
    write_log(log_file, f"\n对比结果：已匹配 {matched_count}，缺失 {missing_in_vo}，多余 {extra_in_vo}。\n")
    if missing_in_vo == 0 and extra_in_vo == 0 and not ambiguous and not unmatched:
        print_color("恭喜，源资产与目标分配完全一致！", GREEN)
        write_log(log_file, "恭喜，源资产与目标分配完全一致！\n")
    
    return ambiguous, unmatched

# ===== 以下是新增的功能函数 =====

def extract_keywords_in_order(filename, matched_folders):
    """
    从文件名中按顺序提取匹配的关键字
    返回: (关键字顺序列表, 模式字符串, 文件夹顺序列表)
    """
    fname_lower = filename.lower()
    keyword_positions = []
    
    # 获取每个匹配文件夹的关键字及其在文件名中的位置
    for folder in matched_folders:
        keyword = get_core_keyword(folder)
        pos = fname_lower.find(keyword)
        if pos != -1:
            keyword_positions.append((pos, keyword, folder))
    
    # 按位置排序
    keyword_positions.sort(key=lambda x: x[0])
    
    # 提取关键字顺序
    keywords_order = [kw for _, kw, _ in keyword_positions]
    folders_order = [folder for _, _, folder in keyword_positions]
    
    # 创建模式字符串 (如 "a_b" 或 "b_a")
    pattern = "_".join(keywords_order)
    
    return keywords_order, pattern, folders_order

def group_by_pattern(ambiguous_files):
    """
    根据文件名模式对多重匹配文件进行分组
    """
    pattern_groups = defaultdict(list)
    
    for fname, folders in ambiguous_files:
        keywords_order, pattern, folders_order = extract_keywords_in_order(fname, folders)
        pattern_groups[pattern].append({
            'filename': fname,
            'matched_folders': folders,
            'folders_in_order': folders_order,
            'keywords_order': keywords_order
        })
    
    return pattern_groups

def batch_copy_files(asset_folder, vo_folder, files_info, target_folder, log_file, pattern):
    """
    批量复制文件到指定文件夹
    """
    processed = 0
    skipped = 0
    
    print_color(f"\n开始批量复制模式 [{pattern}] 的文件到 {target_folder}...", CYAN)
    
    for file_info in files_info:
        fname = file_info['filename']
        src = os.path.join(asset_folder, fname)
        dst_folder = os.path.join(vo_folder, target_folder)
        dst = os.path.join(dst_folder, fname)
        
        if os.path.exists(dst):
            print_color(f"跳过(已存在): {fname}", YELLOW)
            write_log(log_file, f"批量复制时跳过已存在文件: {fname} in {target_folder}")
            skipped += 1
        else:
            try:
                shutil.copy2(src, dst)
                print_color(f"成功复制: {fname} -> {target_folder}", GREEN)
                write_log(log_file, f"批量复制: {src} --> {dst} (模式: {pattern})")
                processed += 1
            except Exception as e:
                print_color(f"错误: 无法复制 {fname}: {e}", RED)
                write_log(log_file, f"错误: 无法复制 {src} 到 {dst}: {e}")
                skipped += 1
    
    return processed, skipped

def handle_individual_files(asset_folder, vo_folder, files, log_file, pattern):
    """
    单独处理每个文件
    """
    processed = 0
    skipped = 0
    
    print_color(f"\n单独处理模式 [{pattern}] 的文件...", CYAN)
    
    for idx, (fname, folders) in enumerate(files, 1):
        print_color(f"\n[{idx}/{len(files)}] 文件: {fname}", CYAN)
        print_color("可选的目标文件夹：", YELLOW)
        
        for i, folder in enumerate(folders, 1):
            print(f"  {i}. {folder}")
        
        print("  0. 跳过此文件")
        
        while True:
            choice = input("\n请选择目标文件夹 (输入数字): ").strip()
            
            if choice == '0':
                print_color(f"跳过文件: {fname}", YELLOW)
                skipped += 1
                break
            elif choice.isdigit() and 1 <= int(choice) <= len(folders):
                target_folder = folders[int(choice) - 1]
                src = os.path.join(asset_folder, fname)
                dst_folder = os.path.join(vo_folder, target_folder)
                dst = os.path.join(dst_folder, fname)
                
                if os.path.exists(dst):
                    overwrite = input(f"文件已存在于 {target_folder}，是否覆盖？(y/n): ").strip().lower()
                    if overwrite != 'y':
                        print_color("跳过已存在的文件", YELLOW)
                        skipped += 1
                        break
                
                try:
                    shutil.copy2(src, dst)
                    print_color(f"成功复制: {fname} -> {target_folder}", GREEN)
                    write_log(log_file, f"单独处理: {src} --> {dst} (模式: {pattern})")
                    processed += 1
                    break
                except Exception as e:
                    print_color(f"错误: 无法复制文件: {e}", RED)
                    write_log(log_file, f"错误: 无法复制 {src} 到 {dst}: {e}")
                    break
            else:
                print_color("无效输入，请重新选择。", RED)
    
    return processed, skipped

def handle_ambiguous_files(asset_folder, vo_folder, ambiguous_files, log_file):
    """
    处理多重匹配的文件，支持智能分组和批量处理
    """
    if not ambiguous_files:
        print_color("\n没有多重匹配的文件需要处理。", GREEN)
        return
    
    print_color(f"\n========== 多重匹配文件处理（智能分组版） ==========", CYAN)
    print_color(f"共有 {len(ambiguous_files)} 个文件需要处理", YELLOW)
    
    # 按模式分组
    pattern_groups = group_by_pattern(ambiguous_files)
    
    print_color(f"\n发现 {len(pattern_groups)} 种不同的命名模式：", CYAN)
    
    total_processed = 0
    total_skipped = 0
    
    # 逐个模式组处理
    for pattern_idx, (pattern, files_info) in enumerate(pattern_groups.items(), 1):
        print_color(f"\n--- 模式 {pattern_idx}/{len(pattern_groups)}: 关键字顺序 [{pattern}] ---", CYAN)
        print_color(f"该模式下有 {len(files_info)} 个文件：", YELLOW)
        
        # 显示该组的文件列表
        for i, file_info in enumerate(files_info[:5], 1):  # 最多显示前5个
            print(f"  {i}. {file_info['filename']}")
        if len(files_info) > 5:
            print(f"  ... 还有 {len(files_info) - 5} 个文件")
        
        # 显示可选的目标文件夹（基于第一个文件的顺序）
        first_file = files_info[0]
        print_color("\n根据关键字顺序，建议的目标文件夹：", YELLOW)
        for i, folder in enumerate(first_file['folders_in_order'], 1):
            keyword = get_core_keyword(folder)
            print(f"  {i}. {folder} (关键字: {keyword})")
        
        print("\n操作选项：")
        print("  输入数字: 将该组所有文件复制到对应文件夹")
        print("  s: 跳过这组文件")
        print("  d: 查看详细文件列表")
        print("  i: 单独处理每个文件")
        print("  q: 退出处理")
        
        while True:
            choice = input("\n请选择操作: ").strip().lower()
            
            if choice == 'q':
                print_color("退出批量处理。", YELLOW)
                write_log(log_file, f"用户退出处理，已处理 {total_processed} 个文件")
                return
            
            elif choice == 's':
                print_color(f"跳过模式 [{pattern}] 的所有文件", YELLOW)
                write_log(log_file, f"用户跳过模式 [{pattern}] 的 {len(files_info)} 个文件")
                total_skipped += len(files_info)
                break
            
            elif choice == 'd':
                print_color(f"\n模式 [{pattern}] 的完整文件列表：", CYAN)
                for i, file_info in enumerate(files_info, 1):
                    print(f"  {i}. {file_info['filename']}")
                continue
            
            elif choice == 'i':
                # 单独处理每个文件
                processed, skipped = handle_individual_files(
                    asset_folder, vo_folder, 
                    [(f['filename'], f['matched_folders']) for f in files_info],
                    log_file, pattern
                )
                total_processed += processed
                total_skipped += skipped
                break
            
            elif choice.isdigit():
                folder_idx = int(choice) - 1
                if 0 <= folder_idx < len(first_file['folders_in_order']):
                    target_folder = first_file['folders_in_order'][folder_idx]
                    
                    # 确认批量操作
                    confirm = input(f"\n确认将 {len(files_info)} 个文件复制到 {target_folder}? (y/n): ").strip().lower()
                    if confirm == 'y':
                        processed, skipped = batch_copy_files(
                            asset_folder, vo_folder, files_info, 
                            target_folder, log_file, pattern
                        )
                        total_processed += processed
                        total_skipped += skipped
                        break
                    else:
                        print_color("取消操作", YELLOW)
                else:
                    print_color("无效的数字，请重新输入", RED)
            else:
                print_color("无效的选项，请重新输入", RED)
    
    print_color(f"\n多重匹配文件处理完成：成功处理 {total_processed} 个，跳过 {total_skipped} 个。", GREEN)
    write_log(log_file, f"\n多重匹配文件处理完成：成功处理 {total_processed} 个，跳过 {total_skipped} 个。")

def main():
    print("=== ASSET 智能分配/对比工具（多关键词排除版） ===")
    asset_folder = get_input_folder("请输入ASSET资源文件夹路径: ")
    vo_folder = get_input_folder("请输入目标文件夹路径(原VO): ")

    log_file = os.path.join(os.path.expanduser("~"), f"智能分配对比日志_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt")
    write_log(log_file, f"操作日志\n开始时间: {datetime.now()}\n")

    # 存储多重匹配的文件
    ambiguous_files = []

    while True:
        print("\n请选择操作：")
        print("0. 退出程序")
        print("1. 执行分配操作并生成日志")
        print("2. 仅对比ASSET与目标文件夹资产一致性")
        print("3. 两者都执行")
        print("4. 处理多重匹配的文件（需要先执行1或3）")
        choice = input("请输入选项(0/1/2/3/4): ").strip()

        if choice == "0":
            print_color("程序已退出。感谢使用！", GREEN)
            break
        elif choice == "1":
            ambiguous_files, _ = copy_assets(asset_folder, vo_folder, log_file)
        elif choice == "2":
            ambiguous_files, _ = compare_assets(asset_folder, vo_folder, log_file)
        elif choice == "3":
            ambiguous_files, _ = copy_assets(asset_folder, vo_folder, log_file)
            compare_assets(asset_folder, vo_folder, log_file)
        elif choice == "4":
            if ambiguous_files:
                handle_ambiguous_files(asset_folder, vo_folder, ambiguous_files, log_file)
                # 清空已处理的多重匹配文件列表
                ambiguous_files = []
            else:
                print_color("没有多重匹配的文件需要处理。请先执行选项1或3。", YELLOW)
        else:
            print_color("无效选项，请重新输入。", RED)

        print_color(f"\n详细日志已保存至: {log_file}", GREEN)

if __name__ == "__main__":
    main()