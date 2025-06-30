setlocal

cd /d "%~dp0..\"
del /q /f build\Debug\CMakeCache.txt
cmake . --preset VS2022_x64-Debug

endlocal
pause
