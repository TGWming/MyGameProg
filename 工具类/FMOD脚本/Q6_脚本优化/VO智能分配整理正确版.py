import os
import shutil
import re
from datetime import datetime

# ANSI color codes for terminal
GREEN = "\033[32m"
RED = "\033[31m"
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
    自动提取VO子文件夹名的‘核心关键字’（例如NPC_SR_Achan->Achan, lead_furong->Furong）
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
        return

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

def compare_assets(asset_folder, vo_folder, log_file):
    vo_subfolders = [f for f in os.listdir(vo_folder) if os.path.isdir(os.path.join(vo_folder, f))]
    if not vo_subfolders:
        print_color("目标文件夹下没有任何子文件夹，无法对比。", RED)
        write_log(log_file, "目标文件夹下没有任何子文件夹，无法对比。")
        return

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

def main():
    print("=== ASSET 智能分配/对比工具（多关键词排除版） ===")
    asset_folder = get_input_folder("请输入ASSET资源文件夹路径: ")
    vo_folder = get_input_folder("请输入目标文件夹路径(原VO): ")

    log_file = os.path.join(os.path.expanduser("~"), f"智能分配对比日志_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt")
    write_log(log_file, f"操作日志\n开始时间: {datetime.now()}\n")

    while True:
        print("\n请选择操作：")
        print("0. 退出程序")
        print("1. 执行分配操作并生成日志")
        print("2. 仅对比ASSET与目标文件夹资产一致性")
        print("3. 两者都执行")
        choice = input("请输入选项(0/1/2/3): ").strip()

        if choice == "0":
            print_color("程序已退出。感谢使用！", GREEN)
            break
        elif choice == "1":
            copy_assets(asset_folder, vo_folder, log_file)
        elif choice == "2":
            compare_assets(asset_folder, vo_folder, log_file)
        elif choice == "3":
            copy_assets(asset_folder, vo_folder, log_file)
            compare_assets(asset_folder, vo_folder, log_file)
        else:
            print_color("无效选项，请重新输入。", RED)

        print_color(f"\n详细日志已保存至: {log_file}", GREEN)

if __name__ == "__main__":
    main()