@echo off
setlocal enabledelayedexpansion

echo ===============================================
echo         Git 仓库初始化和备份脚本
echo ===============================================

REM 检查是否已经是Git仓库
if exist ".git" (
    echo.
    echo ? Git仓库已存在，跳过初始化步骤
    goto :create_tag
)

echo.
echo ?? 初始化Git仓库...

REM 初始化Git仓库
git init
if %errorlevel% neq 0 (
    echo ? Git初始化失败！请确保Git已安装并在PATH中。
    echo.
    echo ?? 提示：
    echo 1. 下载并安装Git: https://git-scm.com/
    echo 2. 确保Git在系统PATH中
    echo 3. 重启命令提示符后重试
    echo.
    pause
    exit /b 1
)

echo ? Git仓库初始化成功

echo.
echo ?? 创建.gitignore文件...

REM 创建UE4项目的.gitignore
(
echo # UE4 Project Files
echo Binaries/
echo Intermediate/
echo DerivedDataCache/
echo .vs/
echo *.tmp
echo *.db
echo *.opendb
echo *.VC.db
echo *.suo
echo *.user
echo *.sln
echo *.vcxproj
echo *.vcxproj.filters
echo *.vcxproj.user
echo.
echo # OS Files
echo .DS_Store
echo Thumbs.db
echo.
echo # Crash Reports
echo *.dmp
echo.
echo # Local settings
echo Saved/
echo.
echo # Emergency backups ^(keep structured backups^)
echo Backup/Emergency_*/
) > .gitignore

echo ? .gitignore文件已创建

echo.
echo ?? 添加项目文件到Git...

REM 添加项目文件
git add .
if %errorlevel% neq 0 (
    echo ? 添加文件失败！
    pause
    exit /b 1
)

echo ? 文件已添加到暂存区

echo.
echo ?? 创建初始提交...

git commit -m "Initial commit: UE4.27 soul project with complete lock-on system before modularization"
if %errorlevel% neq 0 (
    echo ? 初始提交失败！
    pause
    exit /b 1
)

echo ? 初始提交完成

:create_tag
echo.
echo ???  创建备份标签...

REM 创建备份标签
git tag backup-pre-modularization -m "Backup before modularization refactoring - Complete original implementation"
if %errorlevel% neq 0 (
    echo ? 创建标签失败！
    pause
    exit /b 1
)

echo ? 备份标签 'backup-pre-modularization' 已创建

echo.
echo ?? 显示Git状态...
git status --short

echo.
echo ???  显示所有标签...
git tag -l

echo.
echo ===============================================
echo           Git备份设置完成！
echo ===============================================
echo.
echo ?? 已创建的备份点：
echo   ?? 文件备份: Backup/Pre-Modularization/
echo   ???  Git标签: backup-pre-modularization
echo.
echo ?? 如需恢复到此点：
echo   方法1: Rollback.bat pre-modularization
echo   方法2: git checkout backup-pre-modularization
echo.
echo ?? 有用的Git命令：
echo   git log --oneline     ^(查看提交历史^)
echo   git tag -l            ^(查看所有标签^)
echo   git show backup-pre-modularization ^(查看标签详情^)
echo.
pause