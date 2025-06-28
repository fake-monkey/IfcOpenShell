setlocal

cd /d %~dp0..\
rmdir /s /q build\package
cmake . --preset VS2022_x64-Debug
cmake --build --preset Debug -j %NUMBER_OF_PROCESSORS%
cmake --install build\Debug --config Debug
rmdir /S /Q build\temp\
mkdir build\temp\
rem copy build\package\cmake\*-debug.cmake build\temp\ /Y
cmake . --preset VS2022_x64-Release
cmake --build --preset Release -j %NUMBER_OF_PROCESSORS%
cmake --install build\Release --config Release
rem copy build\temp\* build\package\cmake\ /Y
conan export-pkg . -of build\conan_export

endlocal
pause
