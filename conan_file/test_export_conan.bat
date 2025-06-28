@echo off
setlocal

cd /d %~dp0..\
rmdir /s /q build\package
cmake --build --preset Debug -j %NUMBER_OF_PROCESSORS%
cmake --install build\Debug
cmake --build --preset Release -j %NUMBER_OF_PROCESSORS%
cmake --install build\Release
conan export-pkg conan_file

endlocal
pause
