@echo off
echo ===============================================
echo      源代码文件重新组织脚本
echo ===============================================

echo.
echo ?? 目标：将MyCharacter文件移动到正确的Character目录下

echo.
echo ?? 当前文件位置检查：
if exist "Source\soul\MyCharacter.cpp" (
    echo    ? MyCharacter.cpp 在 Source\soul\
) else (
    echo    ? MyCharacter.cpp 未找到
)

if exist "Source\soul\MyCharacter.h" (
    echo    ? MyCharacter.h 在 Source\soul\
) else (
    echo    ? MyCharacter.h 未找到
)

echo.
echo ?? 目标位置检查：
if exist "Source\soul\Character" (
    echo    ? Character 目录存在
) else (
    echo    ?? 创建 Character 目录...
    mkdir "Source\soul\Character"
    echo    ? Character 目录已创建
)

echo.
echo ??  重要提醒：
echo    1. 确保UE4编辑器已完全关闭
echo    2. 移动后需要重新生成项目文件
echo    3. 建议先备份当前代码

echo.
set /p CONFIRM=确认移动文件到Character目录？(y/N): 

if /i not "%CONFIRM%"=="y" (
    echo 操作已取消。
    pause
    exit /b 0
)

echo.
echo ?? 开始移动文件...

REM 移动MyCharacter.cpp
if exist "Source\soul\MyCharacter.cpp" (
    move "Source\soul\MyCharacter.cpp" "Source\soul\Character\" >nul 2>&1
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.cpp 已移动
    ) else (
        echo    ? MyCharacter.cpp 移动失败
    )
)

REM 移动MyCharacter.h
if exist "Source\soul\MyCharacter.h" (
    move "Source\soul\MyCharacter.h" "Source\soul\Character\" >nul 2>&1
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.h 已移动
    ) else (
        echo    ? MyCharacter.h 移动失败
    )
)

echo.
echo ?? 移动结果：
echo Character/ 目录内容：
dir "Source\soul\Character\" /b

echo.
echo ?? 下一步操作：
echo    1. 右键点击 soul.uproject
echo    2. 选择 "Generate Visual Studio project files"
echo    3. 等待生成完成
echo    4. 打开 soul.sln
echo    5. 重新编译项目
echo    6. 启动UE4编辑器

echo.
echo ? 文件移动完成！
pause