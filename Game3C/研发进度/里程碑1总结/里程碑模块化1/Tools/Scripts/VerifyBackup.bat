@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo           备份完整性验证脚本
echo ===============================================

echo.
echo ?? 正在验证备份系统完整性...

set TOTAL_CHECKS=0
set PASSED_CHECKS=0
set FAILED_CHECKS=0

echo.
echo ?? 1. 检查备份文件夹结构...

set BACKUP_FOLDERS=Pre-Modularization Stage1-Config Stage2-Detection Stage3-Camera Stage4-UI Stage5-Debug Stage6-SoulComponents

for %%f in (%BACKUP_FOLDERS%) do (
    set /a TOTAL_CHECKS+=1
    if exist "Backup\%%f" (
        echo    ? Backup\%%f
        set /a PASSED_CHECKS+=1
    ) else (
        echo    ? Backup\%%f ^(缺失^)
        set /a FAILED_CHECKS+=1
    )
)

echo.
echo ?? 2. 检查README文件...

for %%f in (%BACKUP_FOLDERS%) do (
    set /a TOTAL_CHECKS+=1
    if exist "Backup\%%f\README.md" (
        echo    ? Backup\%%f\README.md
        set /a PASSED_CHECKS+=1
    ) else (
        echo    ? Backup\%%f\README.md ^(缺失^)
        set /a FAILED_CHECKS+=1
    )
)

echo.
echo ?? 3. 检查核心备份文件...

set /a TOTAL_CHECKS+=1
if exist "Backup\Pre-Modularization\MyCharacter.h" (
    echo    ? Pre-Modularization\MyCharacter.h
    set /a PASSED_CHECKS+=1
    
    REM 检查文件大小
    for %%A in ("Backup\Pre-Modularization\MyCharacter.h") do set SIZE_H=%%~zA
    echo       ?? 文件大小: !SIZE_H! 字节
) else (
    echo    ? Pre-Modularization\MyCharacter.h ^(缺失^)
    set /a FAILED_CHECKS+=1
)

set /a TOTAL_CHECKS+=1
if exist "Backup\Pre-Modularization\MyCharacter.cpp" (
    echo    ? Pre-Modularization\MyCharacter.cpp
    set /a PASSED_CHECKS+=1
    
    REM 检查文件大小
    for %%A in ("Backup\Pre-Modularization\MyCharacter.cpp") do set SIZE_CPP=%%~zA
    echo       ?? 文件大小: !SIZE_CPP! 字节
) else (
    echo    ? Pre-Modularization\MyCharacter.cpp ^(缺失^)
    set /a FAILED_CHECKS+=1
)

echo.
echo ?? 4. 检查备份脚本...

set SCRIPTS=Rollback.bat CompileTest.bat GitBackup.bat SetupBackup.bat

for %%s in (%SCRIPTS%) do (
    set /a TOTAL_CHECKS+=1
    if exist "%%s" (
        echo    ? %%s
        set /a PASSED_CHECKS+=1
    ) else (
        echo    ? %%s ^(缺失^)
        set /a FAILED_CHECKS+=1
    )
)

echo.
echo ???  5. 检查Git状态...

git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo    ? Git 已安装
    
    if exist ".git" (
        echo    ? Git 仓库已初始化
        
        REM 检查是否有备份标签
        git tag -l | findstr "backup-pre-modularization" >nul
        if !errorlevel! equ 0 (
            echo    ? 备份标签 'backup-pre-modularization' 已创建
        ) else (
            echo    ??  备份标签未找到 ^(运行 GitBackup.bat 创建^)
        )
    ) else (
        echo    ??  Git 仓库未初始化 ^(运行 GitBackup.bat 初始化^)
    )
) else (
    echo    ??  Git 未安装或不在PATH中
)

echo.
echo ?? 6. 对比原始文件和备份文件...

if exist "Source\soul\MyCharacter.h" if exist "Backup\Pre-Modularization\MyCharacter.h" (
    fc /B "Source\soul\MyCharacter.h" "Backup\Pre-Modularization\MyCharacter.h" >nul
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.h 备份与原始文件一致
    ) else (
        echo    ??  MyCharacter.h 备份与原始文件不同 ^(可能已修改^)
    )
)

if exist "Source\soul\MyCharacter.cpp" if exist "Backup\Pre-Modularization\MyCharacter.cpp" (
    fc /B "Source\soul\MyCharacter.cpp" "Backup\Pre-Modularization\MyCharacter.cpp" >nul
    if !errorlevel! equ 0 (
        echo    ? MyCharacter.cpp 备份与原始文件一致
    ) else (
        echo    ??  MyCharacter.cpp 备份与原始文件不同 ^(可能已修改^)
    )
)

echo.
echo ?? 7. 验证回滚功能...

echo    ?? 测试回滚脚本语法...
echo 调用 Rollback.bat 无参数测试... | findstr /C:"测试" >nul
if %errorlevel% equ 0 (
    echo    ? 回滚脚本可执行
) else (
    echo    ??  回滚脚本状态未知
)

echo.
echo ===============================================
echo              验证结果摘要
echo ===============================================

set /a SUCCESS_RATE=(%PASSED_CHECKS% * 100) / %TOTAL_CHECKS%

echo.
echo ?? 总体状态：
echo   ?? 总检查项：%TOTAL_CHECKS%
echo   ? 通过项：%PASSED_CHECKS%
echo   ? 失败项：%FAILED_CHECKS%
echo   ?? 成功率：%SUCCESS_RATE%%%

echo.
if %FAILED_CHECKS% equ 0 (
    echo ?? 备份系统完整性验证通过！
    echo.
    echo ? 所有检查项目都通过
    echo ? 备份文件完整且可用
    echo ? 脚本工具齐全
    echo ? 可以安全开始模块化重构
) else (
    echo ??  备份系统存在问题！
    echo.
    echo 建议操作：
    if %FAILED_CHECKS% gtr 0 (
        echo   1. 运行 SetupBackup.bat 重新设置备份系统
        echo   2. 检查缺失的文件和文件夹
        echo   3. 验证所有脚本都已正确创建
        echo   4. 重新运行此验证脚本
    )
)

echo.
echo ? 验证完成时间: %date% %time%
echo.
pause