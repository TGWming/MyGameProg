# DebugConsolePlugin 文件完整性验证脚本
# 运行此脚本以验证所有必需文件是否存在

$PluginPath = "H:\codeCam\MyProject\Plugins\DebugConsolePlugin"

Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  DebugConsolePlugin 文件完整性检查" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

# 定义所有必需文件
$RequiredFiles = @{
    "核心文件" = @(
        "DebugConsolePlugin.uplugin",
        "Source\DebugConsolePlugin\DebugConsolePlugin.Build.cs"
    )
    "头文件" = @(
        "Source\DebugConsolePlugin\Public\DebugConsolePluginModule.h",
        "Source\DebugConsolePlugin\Public\DebugConsoleManager.h",
        "Source\DebugConsolePlugin\Public\DebugConsoleOutputDevice.h",
        "Source\DebugConsolePlugin\Public\DebugConsoleSettings.h"
    )
    "实现文件" = @(
        "Source\DebugConsolePlugin\Private\DebugConsolePluginModule.cpp",
        "Source\DebugConsolePlugin\Private\DebugConsoleManager.cpp",
        "Source\DebugConsolePlugin\Private\DebugConsoleOutputDevice.cpp",
        "Source\DebugConsolePlugin\Private\DebugConsoleSettings.cpp"
    )
    "配置文件" = @(
        "Config\DefaultDebugConsole.ini"
    )
    "文档文件" = @(
        "README.md",
        "INSTALLATION.md",
        "SUMMARY.md",
        "STRUCTURE.md",
        "QUICKREF.md"
    )
    "示例文件" = @(
        "Examples\TestDebugConsole.cpp"
    )
}

$AllFilesExist = $true
$TotalFiles = 0
$ExistingFiles = 0

foreach ($Category in $RequiredFiles.Keys) {
    Write-Host "[$Category]" -ForegroundColor Yellow
    
    foreach ($File in $RequiredFiles[$Category]) {
        $TotalFiles++
        $FullPath = Join-Path $PluginPath $File
        
        if (Test-Path $FullPath) {
            $Size = (Get-Item $FullPath).Length
            $SizeKB = [math]::Round($Size / 1KB, 2)
            Write-Host "  [✓] $File" -ForegroundColor Green -NoNewline
            Write-Host " ($SizeKB KB)" -ForegroundColor Gray
            $ExistingFiles++
        } else {
            Write-Host "  [✗] $File - 缺失!" -ForegroundColor Red
            $AllFilesExist = $false
        }
    }
    Write-Host ""
}

# 统计信息
Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  统计信息" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  总文件数: $TotalFiles" -ForegroundColor White
Write-Host "  存在文件: $ExistingFiles" -ForegroundColor Green
Write-Host "  缺失文件: $($TotalFiles - $ExistingFiles)" -ForegroundColor Red

# 计算总大小
$TotalSize = (Get-ChildItem -Path $PluginPath -Recurse -File | Measure-Object -Property Length -Sum).Sum
$TotalSizeKB = [math]::Round($TotalSize / 1KB, 2)
$TotalSizeMB = [math]::Round($TotalSize / 1MB, 2)
Write-Host "  总大小: $TotalSizeKB KB ($TotalSizeMB MB)" -ForegroundColor Cyan

Write-Host ""

# 最终结果
if ($AllFilesExist) {
    Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Green
    Write-Host "  ✅ 所有文件完整！插件已准备就绪。" -ForegroundColor Green
    Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Green
    Write-Host ""
    Write-Host "下一步操作:" -ForegroundColor Yellow
    Write-Host "  1. 右键 MyProject.uproject → Generate Visual Studio project files" -ForegroundColor White
    Write-Host "  2. 打开 MyProject.sln 并编译项目" -ForegroundColor White
    Write-Host "  3. 运行游戏并查看调试控制台窗口" -ForegroundColor White
    Write-Host ""
    Write-Host "详细说明请查看: INSTALLATION.md" -ForegroundColor Cyan
} else {
    Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Red
    Write-Host "  ❌ 部分文件缺失！请检查上方列表。" -ForegroundColor Red
    Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Red
}

Write-Host ""
Write-Host "按任意键退出..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
