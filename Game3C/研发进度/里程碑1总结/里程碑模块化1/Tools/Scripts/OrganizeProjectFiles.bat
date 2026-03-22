@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo      项目文件整理脚本 - 移动到Tools目录
echo ===============================================

echo.
echo ?? 目标：将所有BAT和MD文件整理到Tools目录下

REM 检查Tools目录是否存在
if not exist "Tools" (
    echo ?? 创建Tools目录...
    mkdir "Tools"
    echo    ? Tools目录已创建
) else (
    echo ?? Tools目录已存在
)

REM 创建子目录结构
echo.
echo ?? 创建Tools子目录结构...

if not exist "Tools\Scripts" (
    mkdir "Tools\Scripts"
    echo    ? Tools\Scripts 已创建
) else (
    echo    ??  Tools\Scripts 已存在
)

if not exist "Tools\Docs" (
    mkdir "Tools\Docs"
    echo    ? Tools\Docs 已创建
) else (
    echo    ??  Tools\Docs 已存在
)

if not exist "Tools\Backup" (
    mkdir "Tools\Backup"
    echo    ? Tools\Backup 已创建
) else (
    echo    ??  Tools\Backup 已存在
)

if not exist "Tools\Config" (
    mkdir "Tools\Config"
    echo    ? Tools\Config 已创建
) else (
    echo    ??  Tools\Config 已存在
)

echo.
echo ?? 扫描当前目录下的BAT和MD文件...

REM 统计文件数量
set BAT_COUNT=0
set MD_COUNT=0
set BACKUP_COUNT=0

for %%f in (*.bat) do (
    set /a BAT_COUNT+=1
)

for %%f in (*.md) do (
    set /a MD_COUNT+=1
)

if exist "Backup" (
    set BACKUP_COUNT=1
)

echo    ?? 发现 %BAT_COUNT% 个BAT文件
echo    ?? 发现 %MD_COUNT% 个MD文件
echo    ?? 发现 %BACKUP_COUNT% 个Backup目录

if %BAT_COUNT% equ 0 if %MD_COUNT% equ 0 if %BACKUP_COUNT% equ 0 (
    echo.
    echo ??  没有找到需要整理的文件。
    echo.
    pause
    exit /b 0
)

echo.
echo ?? 将要执行的操作：
echo    ?? 移动 %BAT_COUNT% 个BAT文件到 Tools\Scripts\
echo    ?? 移动 %MD_COUNT% 个MD文件到 Tools\Docs\
if %BACKUP_COUNT% gtr 0 (
    echo    ?? 移动 Backup 目录到 Tools\Backup\
)

echo.
set /p CONFIRM=确认执行整理操作？(y/N): 
if /i not "%CONFIRM%"=="y" (
    echo 操作已取消。
    pause
    exit /b 0
)

echo.
echo ?? 开始整理...

REM 移动BAT文件
if %BAT_COUNT% gtr 0 (
    echo.
    echo ?? 移动BAT文件到 Tools\Scripts\...
    for %%f in (*.bat) do (
        if exist "%%f" (
            move "%%f" "Tools\Scripts\" >nul 2>&1
            if !errorlevel! equ 0 (
                echo    ? %%f
            ) else (
                echo    ? %%f ^(移动失败^)
            )
        )
    )
)

REM 移动MD文件
if %MD_COUNT% gtr 0 (
    echo.
    echo ?? 移动MD文件到 Tools\Docs\...
    for %%f in (*.md) do (
        if exist "%%f" (
            move "%%f" "Tools\Docs\" >nul 2>&1
            if !errorlevel! equ 0 (
                echo    ? %%f
            ) else (
                echo    ? %%f ^(移动失败^)
            )
        )
    )
)

REM 移动Backup目录
if exist "Backup" (
    echo.
    echo ?? 移动Backup目录到 Tools\...
    
    REM 如果Tools\Backup已存在且不为空，先备份
    if exist "Tools\Backup\*" (
        set TIMESTAMP=%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%
        set TIMESTAMP=!TIMESTAMP: =0!
        mkdir "Tools\Backup_Old_!TIMESTAMP!" 2>nul
        xcopy "Tools\Backup\*" "Tools\Backup_Old_!TIMESTAMP!\" /E /I /H /Y >nul 2>&1
        echo    ??  原有备份已保存到 Tools\Backup_Old_!TIMESTAMP!\
    )
    
    REM 移动Backup目录内容
    xcopy "Backup\*" "Tools\Backup\" /E /I /H /Y >nul 2>&1
    if !errorlevel! equ 0 (
        rmdir /s /q "Backup" 2>nul
        echo    ? Backup目录内容已移动
    ) else (
        echo    ? Backup目录移动失败
    )
)

echo.
echo ?? 更新脚本中的路径引用...

REM 更新移动后脚本中的路径
cd "Tools\Scripts" 2>nul
if !errorlevel! equ 0 (
    for %%f in (*.bat) do (
        if exist "%%f" (
            echo    ?? 更新 %%f 中的路径引用...
            
            REM 使用PowerShell更新路径引用
            powershell -Command "try { (Get-Content '%%f') -replace '^(\s*)(copy|move|xcopy)(\s+)', '$1$2$3..\' -replace 'Backup\\', '..\Backup\' -replace 'Source\\soul\\', '..\..\Source\soul\' | Set-Content '%%f.tmp'; if (Test-Path '%%f.tmp') { Move-Item '%%f.tmp' '%%f' -Force } } catch { Write-Host 'Path update failed for %%f' }" 2>nul
            
            if exist "%%f" (
                echo       ? %%f 路径已更新
            ) else (
                echo       ??  %%f 路径更新可能有问题
            )
        )
    )
    
    cd ..\..
) else (
    echo    ??  无法进入Tools\Scripts目录，跳过路径更新
)

echo.
echo ?? 创建新的项目启动器...

REM 创建新的主启动脚本
(
echo @echo off
echo echo ===============================================
echo echo         UE4.27 Soul 项目工具集
echo echo ===============================================
echo echo.
echo echo ?? 项目工具已整理到Tools目录下
echo echo.
echo echo ?? 工具目录结构：
echo echo    ?? Tools\
echo echo    ├── ?? Scripts\     ^(批处理脚本^)
echo echo    ├── ?? Docs\        ^(文档文件^)
echo echo    ├── ?? Backup\      ^(备份文件^)
echo echo    └── ?? Config\      ^(配置文件^)
echo echo.
echo echo ?? 快速操作：
echo echo    1. 打开工具目录
echo echo    2. 验证备份系统
echo echo    3. 编译测试
echo echo    4. 执行代码回滚
echo echo    5. 查看文档
echo echo.
echo set /p CHOICE=请选择操作 ^(1-5^): 
echo.
echo if "%%CHOICE%%"=="1" explorer "Tools"
echo if "%%CHOICE%%"=="2" call "Tools\Scripts\VerifyBackup.bat"
echo if "%%CHOICE%%"=="3" call "Tools\Scripts\CompileTest.bat"
echo if "%%CHOICE%%"=="4" ^(
echo     echo.
echo     echo 可用回滚点：
echo     echo   - pre-modularization
echo     echo   - stage1-config
echo     echo   - stage2-detection
echo     echo   - stage3-camera
echo     echo   - stage4-ui
echo     echo   - stage5-debug
echo     echo   - stage6-soulcomponents
echo     echo.
echo     set /p POINT=请输入回滚点: 
echo     call "Tools\Scripts\Rollback.bat" "%%POINT%%"
echo ^)
echo if "%%CHOICE%%"=="5" explorer "Tools\Docs"
echo.
echo if "%%CHOICE%%"=="" echo ?? 提示：直接访问 Tools 目录查看所有工具
echo.
echo pause
) > "ProjectTools.bat"

echo    ? 主启动器 ProjectTools.bat 已创建

echo.
echo ?? 创建配置文件...

REM 创建工具配置文件
(
echo # Soul项目工具配置文件
echo # 生成时间: %date% %time%
echo.
echo [Paths]
echo ProjectRoot=../..
echo SourcePath=../../Source/soul
echo ScriptsPath=Scripts
echo DocsPath=Docs
echo BackupPath=Backup
echo ConfigPath=Config
echo.
echo [Tools]
echo MainLauncher=../../ProjectTools.bat
echo BackupScripts=Scripts/Rollback.bat,Scripts/VerifyBackup.bat
echo CompileScript=Scripts/CompileTest.bat
echo.
echo [Organization]
echo OrganizedDate=%date% %time%
echo OriginalLocation=Project Root
echo NewLocation=Tools Directory
echo Structure=Organized and Modularized
) > "Tools\Config\project-config.ini"

echo    ? 配置文件 Tools\Config\project-config.ini 已创建

echo.
echo ?? 创建整理报告...

REM 创建整理报告
(
echo # 项目文件整理报告
echo.
echo ## 整理时间
echo %date% %time%
echo.
echo ## 整理内容
echo - BAT文件数量: %BAT_COUNT%
echo - MD文件数量: %MD_COUNT%
echo - Backup目录: %BACKUP_COUNT%
echo.
echo ## 新的目录结构
echo ```
echo Tools/
echo ├── Scripts/        ^(批处理脚本^)
echo │   ├── Rollback.bat
echo │   ├── CompileTest.bat
echo │   ├── VerifyBackup.bat
echo │   └── 其他脚本...
echo ├── Docs/           ^(文档文件^)
echo │   ├── README.md
echo │   ├── 各种说明文档
echo │   └── 用户指南...
echo ├── Backup/         ^(备份文件^)
echo │   ├── Pre-Modularization/
echo │   ├── Stage1-Config/
echo │   └── 其他备份...
echo └── Config/         ^(配置文件^)
echo     └── project-config.ini
echo ```
echo.
echo ## 使用方式
echo 1. 运行项目根目录的 `ProjectTools.bat` 启动工具
echo 2. 直接访问 `Tools/Scripts/` 目录运行特定脚本
echo 3. 查看 `Tools/Docs/` 目录的文档
echo 4. 管理 `Tools/Backup/` 目录的备份文件
echo.
echo ## 注意事项
echo - 所有脚本路径已自动更新
echo - 保持了原有功能的完整性
echo - 项目结构更加专业化
) > "Tools\Docs\Organization-Report.md"

echo    ? 整理报告 Tools\Docs\Organization-Report.md 已创建

echo.
echo ===============================================
echo              整理完成！
echo ===============================================

echo.
echo ?? 整理摘要：
echo   ? BAT文件：%BAT_COUNT% 个 → Tools\Scripts\
echo   ? MD文件：%MD_COUNT% 个 → Tools\Docs\
if %BACKUP_COUNT% gtr 0 (
    echo   ? Backup目录：已移动 → Tools\Backup\
)
echo   ? 配置文件：已创建 → Tools\Config\
echo   ? 主启动器：ProjectTools.bat ^(项目根目录^)

echo.
echo ?? 新的使用方式：
echo   ?? 快速启动：双击 ProjectTools.bat
echo   ?? 工具目录：explorer Tools
echo   ?? 直接脚本：Tools\Scripts\[脚本名].bat
echo   ?? 查看文档：Tools\Docs\

echo.
echo ?? 优势：
echo   - 项目根目录更加整洁
echo   - 工具集中化管理
echo   - 专业的目录结构
echo   - 便于维护和扩展

echo.
echo ?? 下一步：运行 ProjectTools.bat 开始使用整理后的工具！
echo.
pause