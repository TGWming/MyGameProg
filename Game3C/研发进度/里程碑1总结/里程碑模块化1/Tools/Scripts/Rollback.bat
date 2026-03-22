@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo           UE4.27 项目回滚脚本
echo ===============================================

REM 检查参数
if "%1"=="" (
    echo.
    echo 错误：需要指定回滚点！
    echo.
    echo 使用方法: Rollback.bat [回滚点]
    echo.
    echo 可用的回滚点:
    echo   pre-modularization   - 回滚到模块化重构前的原始版本
    echo   stage1-config        - 回滚到配置提取阶段
    echo   stage2-detection     - 回滚到目标检测模块阶段  
    echo   stage3-camera        - 回滚到相机控制模块阶段
    echo   stage4-ui            - 回滚到UI系统模块阶段
    echo   stage5-debug         - 回滚到调试系统模块阶段
    echo   stage6-soulcomponents - 回滚到最终集成阶段
    echo.
    pause
    exit /b 1
)

set ROLLBACK_POINT=%1

REM 设置备份目录路径
set BACKUP_DIR=
if /i "%ROLLBACK_POINT%"=="pre-modularization" set BACKUP_DIR=Backup\Pre-Modularization
if /i "%ROLLBACK_POINT%"=="stage1-config" set BACKUP_DIR=Backup\Stage1-Config
if /i "%ROLLBACK_POINT%"=="stage2-detection" set BACKUP_DIR=Backup\Stage2-Detection
if /i "%ROLLBACK_POINT%"=="stage3-camera" set BACKUP_DIR=Backup\Stage3-Camera
if /i "%ROLLBACK_POINT%"=="stage4-ui" set BACKUP_DIR=Backup\Stage4-UI
if /i "%ROLLBACK_POINT%"=="stage5-debug" set BACKUP_DIR=Backup\Stage5-Debug
if /i "%ROLLBACK_POINT%"=="stage6-soulcomponents" set BACKUP_DIR=Backup\Stage6-SoulComponents

REM 检查备份目录是否存在
if "%BACKUP_DIR%"=="" (
    echo.
    echo 错误：无效的回滚点 "%ROLLBACK_POINT%"
    echo.
    pause
    exit /b 1
)

if not exist "%BACKUP_DIR%" (
    echo.
    echo 错误：备份目录 "%BACKUP_DIR%" 不存在！
    echo.
    pause
    exit /b 1
)

echo.
echo 准备回滚到: %ROLLBACK_POINT%
echo 备份目录: %BACKUP_DIR%
echo.

REM 显示警告
echo ??  警告：此操作将覆盖当前的工作文件！
echo.
set /p CONFIRM=确定要继续吗？(y/N): 

if /i not "%CONFIRM%"=="y" (
    echo.
    echo 回滚操作已取消。
    pause
    exit /b 0
)

echo.
echo 开始回滚操作...

REM 创建当前状态的紧急备份
set EMERGENCY_BACKUP=Backup\Emergency_%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set EMERGENCY_BACKUP=%EMERGENCY_BACKUP: =0%

if not exist "%EMERGENCY_BACKUP%" mkdir "%EMERGENCY_BACKUP%"

echo.
echo 1. 创建紧急备份到: %EMERGENCY_BACKUP%
if exist "Source\soul\MyCharacter.h" (
    copy "Source\soul\MyCharacter.h" "%EMERGENCY_BACKUP%\MyCharacter.h" >nul
    if !errorlevel! equ 0 echo    ? MyCharacter.h 已备份
) else (
    echo    ! MyCharacter.h 不存在，跳过备份
)

if exist "Source\soul\MyCharacter.cpp" (
    copy "Source\soul\MyCharacter.cpp" "%EMERGENCY_BACKUP%\MyCharacter.cpp" >nul
    if !errorlevel! equ 0 echo    ? MyCharacter.cpp 已备份
) else (
    echo    ! MyCharacter.cpp 不存在，跳过备份
)

REM 回滚核心文件
echo.
echo 2. 从备份恢复文件...
if exist "%BACKUP_DIR%\MyCharacter.h" (
    copy "%BACKUP_DIR%\MyCharacter.h" "Source\soul\MyCharacter.h" >nul
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.h 已恢复
    ) else (
        echo    ? MyCharacter.h 恢复失败
    )
) else (
    echo    ! %BACKUP_DIR%\MyCharacter.h 不存在
)

if exist "%BACKUP_DIR%\MyCharacter.cpp" (
    copy "%BACKUP_DIR%\MyCharacter.cpp" "Source\soul\MyCharacter.cpp" >nul
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.cpp 已恢复
    ) else (
        echo    ? MyCharacter.cpp 恢复失败
    )
) else (
    echo    ! %BACKUP_DIR%\MyCharacter.cpp 不存在
)

REM 处理其他可能的文件（根据不同阶段）
if exist "%BACKUP_DIR%\*.h" (
    echo.
    echo 3. 恢复其他头文件...
    for %%f in ("%BACKUP_DIR%\*.h") do (
        if not "%%~nf"=="MyCharacter" (
            copy "%%f" "Source\soul\%%~nxf" >nul
            if !errorlevel! equ 0 (
                echo    ? %%~nxf 已恢复
            ) else (
                echo    ? %%~nxf 恢复失败
            )
        )
    )
)

if exist "%BACKUP_DIR%\*.cpp" (
    echo.
    echo 4. 恢复其他源文件...
    for %%f in ("%BACKUP_DIR%\*.cpp") do (
        if not "%%~nf"=="MyCharacter" (
            copy "%%f" "Source\soul\%%~nxf" >nul
            if !errorlevel! equ 0 (
                echo    ? %%~nxf 已恢复
            ) else (
                echo    ? %%~nxf 恢复失败
            )
        )
    )
)

echo.
echo ===============================================
echo              回滚操作完成！
echo ===============================================
echo.
echo 已回滚到: %ROLLBACK_POINT%
echo 紧急备份位置: %EMERGENCY_BACKUP%
echo.
echo 建议操作：
echo 1. 运行 CompileTest.bat 验证编译
echo 2. 测试游戏功能是否正常
echo 3. 如有问题，可从紧急备份恢复
echo.
pause