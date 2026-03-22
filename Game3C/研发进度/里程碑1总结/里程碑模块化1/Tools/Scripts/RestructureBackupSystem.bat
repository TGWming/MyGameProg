@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo         备份系统路径重构脚本
echo ===============================================

echo.
echo ?? 目标：将备份系统重构到 Tools/BackupSystem/ 目录下
echo.

REM 检查当前是否有备份系统文件
if not exist "Backup" (
    echo ? 错误：当前目录下没有找到备份系统！
    echo 请确保在包含备份系统的项目根目录运行此脚本。
    pause
    exit /b 1
)

echo ?? 当前备份系统结构：
dir /b Backup 2>nul
if %errorlevel% neq 0 (
    echo ? 无法读取Backup目录
    pause
    exit /b 1
)

echo.
echo ??  警告：此操作将重构备份系统的目录结构！
echo.
echo ?? 新的目录结构将是：
echo    Tools/
echo    ├── BackupSystem/
echo    │   ├── Backup/              (原 Backup/)
echo    │   ├── Scripts/             (原 *.bat)
echo    │   ├── Docs/                (原 *.md)
echo    │   └── Config/              (配置文件)
echo.

set /p CONFIRM=确定要进行重构吗？(y/N): 
if /i not "%CONFIRM%"=="y" (
    echo 重构已取消。
    pause
    exit /b 0
)

echo.
echo ?? 开始重构...

REM 1. 创建新的目录结构
echo 1. 创建目录结构...
if not exist "Tools" mkdir "Tools"
if not exist "Tools\BackupSystem" mkdir "Tools\BackupSystem"
if not exist "Tools\BackupSystem\Backup" mkdir "Tools\BackupSystem\Backup"
if not exist "Tools\BackupSystem\Scripts" mkdir "Tools\BackupSystem\Scripts"
if not exist "Tools\BackupSystem\Docs" mkdir "Tools\BackupSystem\Docs"
if not exist "Tools\BackupSystem\Config" mkdir "Tools\BackupSystem\Config"

echo    ? 目录结构已创建

REM 2. 移动备份文件夹
echo 2. 移动备份文件...
if exist "Backup" (
    xcopy "Backup\*" "Tools\BackupSystem\Backup\" /E /I /H /Y >nul 2>&1
    if !errorlevel! equ 0 (
        echo    ? 备份文件已移动
        rmdir /s /q "Backup" 2>nul
    ) else (
        echo    ? 备份文件移动失败
    )
)

REM 3. 移动脚本文件
echo 3. 移动脚本文件...
for %%f in (*.bat) do (
    if exist "%%f" (
        move "%%f" "Tools\BackupSystem\Scripts\" >nul 2>&1
        if !errorlevel! equ 0 (
            echo    ? %%f 已移动
        ) else (
            echo    ? %%f 移动失败
        )
    )
)

REM 4. 移动文档文件
echo 4. 移动文档文件...
for %%f in (*README*.md BACKUP_*.md) do (
    if exist "%%f" (
        move "%%f" "Tools\BackupSystem\Docs\" >nul 2>&1
        if !errorlevel! equ 0 (
            echo    ? %%f 已移动
        ) else (
            echo    ? %%f 移动失败
        )
    )
)

echo.
echo ?? 5. 更新脚本中的路径引用...

REM 更新脚本中的路径
cd "Tools\BackupSystem\Scripts"

for %%f in (*.bat) do (
    if exist "%%f" (
        echo 更新 %%f 中的路径...
        
        REM 创建临时文件用于路径替换
        powershell -Command "(Get-Content '%%f') -replace 'Backup\\', '..\\Backup\\' -replace 'Source\\soul\\', '..\\..\\..\\Source\\soul\\' | Set-Content '%%f.tmp'"
        
        if exist "%%f.tmp" (
            move "%%f.tmp" "%%f" >nul 2>&1
            echo    ? %%f 路径已更新
        )
    )
)

cd ..\..\..

echo.
echo ?? 6. 创建新的启动脚本...

REM 在项目根目录创建新的启动脚本
(
echo @echo off
echo echo ===============================================
echo echo         UE4.27 备份系统 - 新位置启动器
echo echo ===============================================
echo echo.
echo echo ?? 备份系统已移动到 Tools/BackupSystem/
echo echo.
echo echo ?? 可用命令：
echo echo   1. 验证备份系统    - Tools\BackupSystem\Scripts\VerifyBackup.bat
echo echo   2. 编译测试        - Tools\BackupSystem\Scripts\CompileTest.bat  
echo echo   3. 执行回滚        - Tools\BackupSystem\Scripts\Rollback.bat [回滚点]
echo echo   4. 设置Git备份     - Tools\BackupSystem\Scripts\GitBackup.bat
echo echo.
echo set /p CHOICE=请选择操作 ^(1-4^) 或按Enter查看帮助: 
echo.
echo if "%%CHOICE%%"=="1" call "Tools\BackupSystem\Scripts\VerifyBackup.bat"
echo if "%%CHOICE%%"=="2" call "Tools\BackupSystem\Scripts\CompileTest.bat"
echo if "%%CHOICE%%"=="3" ^(
echo     set /p POINT=请输入回滚点 ^(pre-modularization, stage1-config, etc.^): 
echo     call "Tools\BackupSystem\Scripts\Rollback.bat" "%%POINT%%"
echo ^)
echo if "%%CHOICE%%"=="4" call "Tools\BackupSystem\Scripts\GitBackup.bat"
echo.
echo if "%%CHOICE%%"=="" ^(
echo     echo ?? 查看完整文档: Tools\BackupSystem\Docs\BACKUP_SYSTEM_README.md
echo     explorer "Tools\BackupSystem"
echo ^)
echo.
echo pause
) > "BackupSystem.bat"

echo    ? 启动脚本 BackupSystem.bat 已创建

echo.
echo ?? 7. 创建配置文件...

REM 创建路径配置文件
(
echo # 备份系统路径配置
echo # 此文件记录重构后的路径信息
echo.
echo [Paths]
echo BackupSystemRoot=Tools/BackupSystem
echo BackupFolder=Tools/BackupSystem/Backup  
echo ScriptsFolder=Tools/BackupSystem/Scripts
echo DocsFolder=Tools/BackupSystem/Docs
echo SourceFolder=Source/soul
echo.
echo [RestructureInfo]
echo RestructureDate=%date% %time%
echo OriginalStructure=Root level Backup/, Scripts, Docs
echo NewStructure=Tools/BackupSystem/ hierarchy
echo.
echo [Migration]
echo CompatibleWithOriginalProject=Yes
echo RequiresPathUpdates=Yes
echo MigrationNotes=Scripts updated for new paths
) > "Tools\BackupSystem\Config\paths.ini"

echo    ? 配置文件已创建

echo.
echo ===============================================
echo              重构完成！
echo ===============================================

echo.
echo ?? 重构摘要：
echo   ? 备份文件夹：Backup/ → Tools/BackupSystem/Backup/
echo   ? 脚本文件：*.bat → Tools/BackupSystem/Scripts/
echo   ? 文档文件：*.md → Tools/BackupSystem/Docs/
echo   ? 配置文件：Tools/BackupSystem/Config/
echo   ? 启动器：BackupSystem.bat ^(项目根目录^)

echo.
echo ?? 新的使用方式：
echo   快速启动：双击 BackupSystem.bat
echo   直接访问：explorer Tools\BackupSystem
echo   验证系统：Tools\BackupSystem\Scripts\VerifyBackup.bat

echo.
echo ?? 迁移到原工程的步骤：
echo   1. 复制整个 Tools/ 文件夹到原工程根目录
echo   2. 复制 BackupSystem.bat 到原工程根目录  
echo   3. 运行 Tools\BackupSystem\Scripts\VerifyBackup.bat 验证
echo   4. 根据需要调整 Source/soul 路径

echo.
echo ??  重要提醒：
echo   - 旧的备份文件已清理，请确认新位置文件完整
echo   - 脚本路径已自动更新
echo   - 如需恢复旧结构，请手动操作或从Git恢复

echo.
pause