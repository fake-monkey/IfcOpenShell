setlocal

chcp 65001

REM Check if the first parameter is provided
if "%1"=="" (
    echo usage: %0 [Debug^|Release]
    pause
    exit /b 1
)

cd /d "%~dp0..\"
del /q /f build\%1\CMakeCache.txt
cmake . --preset VS2022_x64-%1

endlocal
REM If double-click to run, pause
echo %cmdcmdline% | find /i "%~nx0" >nul && pause
