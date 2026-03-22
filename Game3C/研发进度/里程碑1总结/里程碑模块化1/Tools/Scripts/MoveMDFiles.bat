@echo off
echo ===============================================
echo         MD文件整理脚本
echo ===============================================

echo.
echo ?? 发现的MD文件：
for %%f in (*.md) do (
    echo    ?? %%f
)

echo.
echo ?? 目标位置：Tools\Docs\

echo.
echo ?? 即将移动的文件：
echo    ?? BACKUP_SYSTEM_README.md           - 备份系统使用指南
echo    ?? CHECKPOINT_MyCharacter_System.md  - 系统检查点文档  
echo    ?? CODE_ROLLBACK_GUIDE.md            - 代码回滚指南
echo    ?? SECTOR_LOCK_IMPLEMENTATION_SUMMARY.md - 扇形锁定实现总结

echo.
set /p CONFIRM=确认移动所有MD文件到Tools\Docs\？(y/N): 

if /i not "%CONFIRM%"=="y" (
    echo 操作已取消。
    pause
    exit /b 0
)

echo.
echo ?? 开始移动文件...

REM 确保目标目录存在
if not exist "Tools\Docs" (
    mkdir "Tools\Docs"
    echo    ?? 创建了Tools\Docs目录
)

REM 移动所有MD文件
for %%f in (*.md) do (
    move "%%f" "Tools\Docs\" >nul 2>&1
    if !errorlevel! equ 0 (
        echo    ? %%f 已移动
    ) else (
        echo    ? %%f 移动失败
    )
)

echo.
echo ? MD文件整理完成！

echo.
echo ?? 新的文档结构：
echo Tools\
echo └── Docs\
for %%f in ("Tools\Docs\*.md") do (
    echo     ?? %%~nxf
)

echo.
echo ?? 访问文档：
echo   打开文档目录: explorer Tools\Docs
echo   主要文档: Tools\Docs\BACKUP_SYSTEM_README.md

echo.
pause