@echo off
echo ===============================================
echo        项目文件检查脚本
echo ===============================================

echo.
echo ?? 检查项目根目录下的文件...

echo.
echo ?? BAT文件：
for %%f in (*.bat) do (
    echo    - %%f
)

echo.
echo ?? MD文件：
for %%f in (*.md) do (
    echo    - %%f
)

echo.
echo ?? 目录：
if exist "Backup" (
    echo    - Backup\ ^(备份目录^)
)
if exist "Tools" (
    echo    - Tools\ ^(工具目录 - 已存在^)
) else (
    echo    - Tools\ ^(工具目录 - 不存在，将创建^)
)

echo.
echo ?? 建议的整理结构：
echo Tools/
echo ├── Scripts/     ^(*.bat文件^)
echo ├── Docs/        ^(*.md文件^)
echo ├── Backup/      ^(Backup目录内容^)
echo └── Config/      ^(配置文件^)

echo.
echo ?? 运行 OrganizeProjectFiles.bat 开始整理
pause