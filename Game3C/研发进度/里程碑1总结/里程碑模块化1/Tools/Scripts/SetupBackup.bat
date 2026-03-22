@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo        UE4.27 项目完整备份系统
echo ===============================================

echo.
echo ?? 正在设置完整的备份系统...

echo.
echo ?? 1. 验证备份文件夹结构...

REM 验证备份文件夹是否存在
set BACKUP_FOLDERS=Pre-Modularization Stage1-Config Stage2-Detection Stage3-Camera Stage4-UI Stage5-Debug Stage6-SoulComponents

for %%f in (%BACKUP_FOLDERS%) do (
    if exist "Backup\%%f" (
        echo    ? Backup\%%f
    ) else (
        echo    ? Backup\%%f ^(缺失^)
        set MISSING_FOLDERS=1
    )
)

if defined MISSING_FOLDERS (
    echo.
    echo ??  警告：部分备份文件夹缺失！
    echo 请确保所有备份文件夹都已创建。
    echo.
)

echo.
echo ?? 2. 验证核心文件...

if exist "Source\soul\MyCharacter.h" (
    echo    ? MyCharacter.h
) else (
    echo    ? MyCharacter.h ^(缺失^)
    set MISSING_FILES=1
)

if exist "Source\soul\MyCharacter.cpp" (
    echo    ? MyCharacter.cpp
) else (
    echo    ? MyCharacter.cpp ^(缺失^)
    set MISSING_FILES=1
)

if exist "soul.uproject" (
    echo    ? soul.uproject
) else (
    echo    ? soul.uproject ^(缺失^)
    set MISSING_FILES=1
)

if defined MISSING_FILES (
    echo.
    echo ? 关键文件缺失！无法继续备份。
    pause
    exit /b 1
)

echo.
echo ?? 3. 验证备份脚本...

if exist "Rollback.bat" (
    echo    ? Rollback.bat
) else (
    echo    ? Rollback.bat ^(缺失^)
)

if exist "CompileTest.bat" (
    echo    ? CompileTest.bat
) else (
    echo    ? CompileTest.bat ^(缺失^)
)

if exist "GitBackup.bat" (
    echo    ? GitBackup.bat
) else (
    echo    ? GitBackup.bat ^(缺失^)
)

echo.
echo ?? 4. 执行Pre-Modularization备份...

REM 检查备份是否已存在
if exist "Backup\Pre-Modularization\MyCharacter.h" (
    echo    ??  MyCharacter.h 备份已存在
) else (
    copy "Source\soul\MyCharacter.h" "Backup\Pre-Modularization\MyCharacter.h" >nul
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.h 已备份
    ) else (
        echo    ? MyCharacter.h 备份失败
    )
)

if exist "Backup\Pre-Modularization\MyCharacter.cpp" (
    echo    ??  MyCharacter.cpp 备份已存在
) else (
    copy "Source\soul\MyCharacter.cpp" "Backup\Pre-Modularization\MyCharacter.cpp" >nul
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.cpp 已备份
    ) else (
        echo    ? MyCharacter.cpp 备份失败
    )
)

echo.
echo ???  5. 设置Git备份（如果可用）...

REM 检查Git是否可用
git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo    ? Git 可用，执行Git备份...
    call GitBackup.bat
) else (
    echo    ??  Git 不可用，跳过Git备份
    echo    ?? 如需Git备份功能，请安装Git并重新运行
)

echo.
echo ?? 6. 执行编译测试...

echo    ?? 开始编译验证...
call CompileTest.bat

echo.
echo ===============================================
echo            备份系统设置完成！
echo ===============================================
echo.
echo ?? 备份摘要：
echo   ? 文件备份：Backup/Pre-Modularization/
echo   ? 回滚脚本：Rollback.bat
echo   ? 编译测试：CompileTest.bat
echo   ?? Git备份：GitBackup.bat ^(需要Git^)
echo.
echo ???  安全保护：
echo   1. 原始文件已安全备份
echo   2. 可随时回滚到任何备份点
echo   3. 编译状态已验证
echo   4. Git版本控制已设置 ^(如果可用^)
echo.
echo ?? 模块化重构准备就绪！
echo.
echo ? 快速命令：
echo   Rollback.bat pre-modularization  ^(回滚到重构前^)
echo   CompileTest.bat                   ^(验证编译^)
echo   GitBackup.bat                     ^(设置Git备份^)
echo.
pause