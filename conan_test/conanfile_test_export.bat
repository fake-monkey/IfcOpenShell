setlocal

cd /d %~dp0..\
rmdir /s /q build\package
cmake . --preset VS2022_x64-Debug
cmake --build --preset Debug -j %NUMBER_OF_PROCESSORS%
cmake --install build\Debug --config Debug
cmake . --preset VS2022_x64-Release
cmake --build --preset Release -j %NUMBER_OF_PROCESSORS%
cmake --install build\Release --config Release
conan export-pkg . -of build\conan_export

endlocal
pause
