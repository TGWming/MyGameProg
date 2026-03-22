@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo           UE4.27 编译验证脚本
echo ===============================================

REM 检查项目文件是否存在
if not exist "soul.uproject" (
    echo.
    echo ? 错误：找不到 soul.uproject 文件！
    echo 请确保在项目根目录运行此脚本。
    echo.
    pause
    exit /b 1
)

echo.
echo ?? 检查项目环境...

REM 检查核心文件是否存在
set FILES_OK=1

if not exist "Source\soul\MyCharacter.h" (
    echo ? MyCharacter.h 文件缺失
    set FILES_OK=0
) else (
    echo ? MyCharacter.h 文件存在
)

if not exist "Source\soul\MyCharacter.cpp" (
    echo ? MyCharacter.cpp 文件缺失
    set FILES_OK=0
) else (
    echo ? MyCharacter.cpp 文件存在
)

if %FILES_OK%==0 (
    echo.
    echo ? 关键文件缺失，无法进行编译测试！
    echo.
    pause
    exit /b 1
)

echo.
echo ?? 开始编译测试...

REM 寻找 UnrealBuildTool
set UBT_PATH=
set ENGINE_PATH=

REM 尝试从注册表获取UE4路径
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\4.27" /v "InstalledDirectory" 2^>nul') do (
    set ENGINE_PATH=%%b
)

if "%ENGINE_PATH%"=="" (
    REM 尝试常见安装路径
    if exist "C:\Program Files\Epic Games\UE_4.27\Engine\Binaries\DotNET\UnrealBuildTool.exe" (
        set ENGINE_PATH=C:\Program Files\Epic Games\UE_4.27
    ) else if exist "D:\Program Files\Epic Games\UE_4.27\Engine\Binaries\DotNET\UnrealBuildTool.exe" (
        set ENGINE_PATH=D:\Program Files\Epic Games\UE_4.27
    ) else if exist "C:\UE_4.27\Engine\Binaries\DotNET\UnrealBuildTool.exe" (
        set ENGINE_PATH=C:\UE_4.27
    )
)

if "%ENGINE_PATH%"=="" (
    echo.
    echo ??  警告：无法自动找到UE4.27安装路径
    echo.
    echo 请手动输入UE4.27引擎路径（例如：C:\Program Files\Epic Games\UE_4.27）
    set /p ENGINE_PATH=引擎路径: 
    
    if "!ENGINE_PATH!"=="" (
        echo.
        echo ? 未提供引擎路径，无法进行编译测试！
        echo.
        echo ?? 提示：
        echo 1. 确保UE4.27已正确安装
        echo 2. 可以尝试在UE4编辑器中编译项目
        echo 3. 或者使用Visual Studio手动编译
        echo.
        pause
        exit /b 1
    )
)

set UBT_PATH=%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool.exe

if not exist "%UBT_PATH%" (
    echo.
    echo ? UnrealBuildTool.exe 未找到！
    echo 路径: %UBT_PATH%
    echo.
    echo ?? 手动编译选项：
    echo 1. 在UE4编辑器中打开项目并编译
    echo 2. 使用Visual Studio打开生成的.sln文件编译
    echo 3. 检查UE4.27是否正确安装
    echo.
    pause
    exit /b 1
)

echo.
echo ?? 使用UnrealBuildTool编译项目...
echo 引擎路径: %ENGINE_PATH%
echo.

REM 清理之前的编译输出
if exist "Binaries" (
    echo ?? 清理旧的编译输出...
    rmdir /s /q "Binaries" 2>nul
)

if exist "Intermediate" (
    echo ?? 清理中间文件...
    rmdir /s /q "Intermediate" 2>nul
)

REM 执行编译
echo.
echo ? 编译中，请稍候...
echo.

"%UBT_PATH%" soulEditor Win64 Development -Project="%~dp0soul.uproject" -WaitMutex -FromMsBuild

set COMPILE_RESULT=%errorlevel%

echo.
echo ===============================================

if %COMPILE_RESULT%==0 (
    echo           ? 编译成功！
    echo ===============================================
    echo.
    echo ?? 项目编译通过，可以安全进行下一步操作！
    echo.
    echo ?? 编译摘要：
    echo   - 编译目标：soulEditor Win64 Development
    echo   - 编译结果：成功
    echo   - 错误数量：0
    echo.
    echo ?? 建议操作：
    echo 1. 在UE4编辑器中打开项目测试功能
    echo 2. 验证锁定系统是否正常工作
    echo 3. 检查是否有运行时警告
    echo.
) else (
    echo           ? 编译失败！
    echo ===============================================
    echo.
    echo ?? 编译过程中出现错误 (错误代码: %COMPILE_RESULT%)
    echo.
    echo ?? 故障排除步骤：
    echo 1. 检查上方的编译错误信息
    echo 2. 确认所有必需的头文件都已包含
    echo 3. 检查语法错误和类型不匹配
    echo 4. 如果错误过多，考虑回滚到上一个稳定版本
    echo.
    echo ?? 紧急恢复：
    echo   运行: Rollback.bat [回滚点]
    echo   例如: Rollback.bat pre-modularization
    echo.
)

echo ? 编译完成时间: %date% %time%
echo.
pause