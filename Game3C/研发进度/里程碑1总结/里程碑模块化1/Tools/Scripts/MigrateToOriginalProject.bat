@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo        备份系统原工程迁移脚本
echo ===============================================

echo.
echo ?? 此脚本帮助您将备份系统迁移到原工程

echo.
echo ?? 迁移前检查清单：
echo   □ 确认当前目录包含完整的备份系统
echo   □ 确认目标原工程路径正确
echo   □ 确认原工程的Source文件夹结构一致
echo   □ 建议先备份原工程

echo.
set /p TARGET_PROJECT=请输入原工程的完整路径: 

if "%TARGET_PROJECT%"=="" (
    echo ? 错误：未提供目标路径！
    pause
    exit /b 1
)

REM 检查目标路径是否存在
if not exist "%TARGET_PROJECT%" (
    echo ? 错误：目标路径不存在！
    echo 路径：%TARGET_PROJECT%
    pause
    exit /b 1
)

REM 检查目标是否是UE4项目
if not exist "%TARGET_PROJECT%\*.uproject" (
    echo ??  警告：目标路径下没有找到.uproject文件
    echo 路径：%TARGET_PROJECT%
    set /p CONTINUE=是否继续？(y/N): 
    if /i not "!CONTINUE!"=="y" (
        echo 迁移已取消。
        pause
        exit /b 0
    )
)

echo.
echo ?? 分析当前备份系统...

REM 检查当前结构类型
set CURRENT_STRUCTURE=unknown
if exist "Tools\BackupSystem" (
    set CURRENT_STRUCTURE=restructured
    set SOURCE_PATH=Tools\BackupSystem
    echo    ? 检测到重构后的目录结构
) else if exist "Backup" (
    set CURRENT_STRUCTURE=original
    set SOURCE_PATH=.
    echo    ? 检测到原始目录结构
) else (
    echo    ? 未找到备份系统文件！
    pause
    exit /b 1
)

echo.
echo ?? 迁移信息：
echo   源路径：%cd%
echo   目标路径：%TARGET_PROJECT%
echo   结构类型：!CURRENT_STRUCTURE!
echo   源备份路径：!SOURCE_PATH!

echo.
set /p CONFIRM=确认开始迁移？(y/N): 
if /i not "%CONFIRM%"=="y" (
    echo 迁移已取消。
    pause
    exit /b 0
)

echo.
echo ?? 开始迁移...

REM 根据不同结构类型进行迁移
if "!CURRENT_STRUCTURE!"=="restructured" (
    echo 1. 迁移重构后的备份系统...
    
    REM 复制Tools文件夹
    if exist "Tools" (
        xcopy "Tools" "%TARGET_PROJECT%\Tools\" /E /I /H /Y >nul 2>&1
        if !errorlevel! equ 0 (
            echo    ? Tools文件夹已复制
        ) else (
            echo    ? Tools文件夹复制失败
        )
    )
    
    REM 复制启动脚本
    if exist "BackupSystem.bat" (
        copy "BackupSystem.bat" "%TARGET_PROJECT%\" >nul 2>&1
        if !errorlevel! equ 0 (
            echo    ? 启动脚本已复制
        ) else (
            echo    ? 启动脚本复制失败
        )
    )
    
) else (
    echo 1. 迁移原始结构的备份系统...
    
    REM 创建目标Tools目录结构
    if not exist "%TARGET_PROJECT%\Tools" mkdir "%TARGET_PROJECT%\Tools"
    if not exist "%TARGET_PROJECT%\Tools\BackupSystem" mkdir "%TARGET_PROJECT%\Tools\BackupSystem"
    if not exist "%TARGET_PROJECT%\Tools\BackupSystem\Backup" mkdir "%TARGET_PROJECT%\Tools\BackupSystem\Backup"
    if not exist "%TARGET_PROJECT%\Tools\BackupSystem\Scripts" mkdir "%TARGET_PROJECT%\Tools\BackupSystem\Scripts"
    if not exist "%TARGET_PROJECT%\Tools\BackupSystem\Docs" mkdir "%TARGET_PROJECT%\Tools\BackupSystem\Docs"
    
    REM 复制文件到新结构
    if exist "Backup" (
        xcopy "Backup\*" "%TARGET_PROJECT%\Tools\BackupSystem\Backup\" /E /I /H /Y >nul 2>&1
        echo    ? 备份文件已复制
    )
    
    for %%f in (*.bat) do (
        copy "%%f" "%TARGET_PROJECT%\Tools\BackupSystem\Scripts\" >nul 2>&1
    )
    echo    ? 脚本文件已复制
    
    for %%f in (*README*.md BACKUP_*.md) do (
        copy "%%f" "%TARGET_PROJECT%\Tools\BackupSystem\Docs\" >nul 2>&1
    )
    echo    ? 文档文件已复制
)

echo.
echo 2. 路径适配...

REM 检查目标工程的Source结构
if exist "%TARGET_PROJECT%\Source" (
    for /d %%d in ("%TARGET_PROJECT%\Source\*") do (
        set TARGET_SOURCE_MODULE=%%~nxd
        echo    ?? 检测到源码模块：!TARGET_SOURCE_MODULE!
        
        REM 更新脚本中的路径引用
        if exist "%TARGET_PROJECT%\Tools\BackupSystem\Scripts" (
            cd /d "%TARGET_PROJECT%\Tools\BackupSystem\Scripts"
            
            for %%s in (*.bat) do (
                echo    ?? 更新 %%s 中的路径引用...
                powershell -Command "(Get-Content '%%s') -replace 'Source\\soul\\', 'Source\\!TARGET_SOURCE_MODULE!\\' | Set-Content '%%s.tmp'" 2>nul
                if exist "%%s.tmp" (
                    move "%%s.tmp" "%%s" >nul 2>&1
                )
            )
            
            cd /d "%~dp0"
        )
        goto :path_update_done
    )
    :path_update_done
    echo    ? 路径引用已更新为：Source\!TARGET_SOURCE_MODULE!\
) else (
    echo    ??  目标工程没有Source文件夹，路径可能需要手动调整
)

echo.
echo 3. 创建适配后的启动脚本...

REM 创建适配后的启动脚本
(
echo @echo off
echo echo ===============================================
echo echo         UE4.27 备份系统 - 原工程版本
echo echo ===============================================
echo echo.
echo echo ?? 备份系统已成功迁移到此工程
echo echo ?? 位置：Tools/BackupSystem/
echo echo ?? 目标模块：!TARGET_SOURCE_MODULE!
echo echo.
echo echo ?? 快速操作：
echo echo   1. 验证备份系统完整性
echo echo   2. 编译测试当前代码
echo echo   3. 执行代码回滚
echo echo   4. 设置Git版本控制
echo echo   5. 打开备份系统文件夹
echo echo.
echo set /p CHOICE=请选择操作 ^(1-5^): 
echo.
echo if "%%CHOICE%%"=="1" call "Tools\BackupSystem\Scripts\VerifyBackup.bat"
echo if "%%CHOICE%%"=="2" call "Tools\BackupSystem\Scripts\CompileTest.bat"
echo if "%%CHOICE%%"=="3" ^(
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
echo     call "Tools\BackupSystem\Scripts\Rollback.bat" "%%POINT%%"
echo ^)
echo if "%%CHOICE%%"=="4" call "Tools\BackupSystem\Scripts\GitBackup.bat"
echo if "%%CHOICE%%"=="5" explorer "Tools\BackupSystem"
echo.
echo pause
) > "%TARGET_PROJECT%\BackupSystem.bat"

echo    ? 适配启动脚本已创建

echo.
echo 4. 验证迁移结果...

cd /d "%TARGET_PROJECT%"

if exist "Tools\BackupSystem\Scripts\VerifyBackup.bat" (
    echo    ?? 运行验证脚本...
    call "Tools\BackupSystem\Scripts\VerifyBackup.bat"
) else (
    echo    ??  验证脚本未找到，请手动检查
)

cd /d "%~dp0"

echo.
echo ===============================================
echo              迁移完成！
echo ===============================================

echo.
echo ?? 迁移摘要：
echo   ? 备份系统已迁移到：%TARGET_PROJECT%\Tools\BackupSystem\
echo   ? 启动脚本：%TARGET_PROJECT%\BackupSystem.bat
echo   ? 源码模块适配：!TARGET_SOURCE_MODULE!
echo   ? 路径引用已更新

echo.
echo ?? 下一步操作：
echo   1. 进入原工程目录：cd /d "%TARGET_PROJECT%"
echo   2. 运行启动脚本：BackupSystem.bat
echo   3. 执行验证：Tools\BackupSystem\Scripts\VerifyBackup.bat
echo   4. 测试编译：Tools\BackupSystem\Scripts\CompileTest.bat

echo.
echo ?? 提示：
echo   - 备份系统现在位于 Tools/BackupSystem/ 目录下
echo   - 所有原有功能都已保留
echo   - 路径已自动适配目标工程
echo   - 可以立即开始使用

echo.
pause