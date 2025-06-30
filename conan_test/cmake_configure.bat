setlocal

chcp 65001

REM 检查是否提供了第一个参数
if "%1"=="" (
    echo usage: %0 [Debug^|Release]
    pause
    exit /b 1
)

cd /d "%~dp0..\"
del /q /f build\%1\CMakeCache.txt
cmake . --preset VS2022_x64-%1

endlocal
pause
