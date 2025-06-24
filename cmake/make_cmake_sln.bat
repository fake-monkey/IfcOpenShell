@echo off
setlocal

:: 检查是否传入了 build_type 参数
if "%~1"=="" (
    echo [ERROR] 缺少 build_type 参数，例如: Debug 或 Release
    echo 用法: build.bat [build_type]
    echo 示例: build.bat Debug
    endlocal
    pause
    exit /b 1
)

cd ..
conan install . --build=never -r=fscut-conan-center --update
cmake --preset VS2022_x64-%1

endlocal
pause
