setlocal

chcp 65001
if "%~1"=="" (
    echo [ERROR] 缺少参数：请提供 build_type ^(例如 Debug 或 Release^)
    echo 用法： %~n0 [Debug^|Release]
    pause
    exit /b 1
)

cd /d %~dp0
cmake --build -j %NUMBER_OF_PROCESSORS%
cmake --install build\%1 --config %1
conan export-pkg .

endlocal
pause
