@echo off
echo 正在修复Windows性能计数器...
echo.

REM 执行核心修复命令
echo 执行 lodctr /R...
lodctr /R

if %errorLevel% equ 0 (
    echo 修复成功！
) else (
    echo 修复遇到问题，请尝试以管理员身份手动运行：lodctr /R
)

echo.
echo 修复完成。请重启计算机。
pause