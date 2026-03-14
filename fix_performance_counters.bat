@echo off
echo ================================================
echo Windows性能计数器修复工具
echo ================================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo 错误：请以管理员身份运行此脚本！
    echo 右键点击此文件，选择"以管理员身份运行"
    pause
    exit /b 1
)

echo 正在检查管理员权限... 通过
echo.

REM 切换到system32目录
cd /d %systemroot%\system32
echo 切换到系统目录：%systemroot%\system32
echo.

REM 方法1：快速修复
echo 步骤1：执行快速修复...
lodctr /R
if %errorLevel% equ 0 (
    echo 快速修复成功完成
) else (
    echo 快速修复遇到问题，继续执行详细修复...
)
echo.

REM 方法2：详细修复（如果快速修复失败）
echo 步骤2：执行详细修复...

REM 卸载现有计数器
echo 正在卸载现有性能计数器...
unlodctr /M:PerfDisk >nul 2>&1
unlodctr /M:PerfNet >nul 2>&1
unlodctr /M:PerfOS >nul 2>&1
unlodctr /M:PerfProc >nul 2>&1

REM 重新加载计数器
echo 正在重新加载性能计数器...
lodctr PerfDisk.ini >nul 2>&1
lodctr PerfNet.ini >nul 2>&1
lodctr PerfOS.ini >nul 2>&1
lodctr PerfProc.ini >nul 2>&1

REM 重建性能计数器注册表
echo 正在重建性能计数器注册表...
perfc -install >nul 2>&1

echo.
echo 步骤3：执行系统文件检查...
echo 注意：此步骤可能需要较长时间，请耐心等待...
sfc /scannow

echo.
echo ================================================
echo 修复完成！
echo ================================================
echo.
echo 请重启计算机以使修复生效。
echo.
echo Rider IDE的性能计数器错误应该会消失。
echo.
pause