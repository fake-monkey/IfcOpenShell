setlocal

cd /d %~dp0..\
rmdir /s /q build\package
call %~dp0cmake_configure.bat Debug
cmake --build --preset Debug -j %NUMBER_OF_PROCESSORS%
cmake --install build\Debug --config Debug
call %~dp0cmake_configure.bat Release
cmake --build --preset Release -j %NUMBER_OF_PROCESSORS%
cmake --install build\Release --config Release
pushd build\package\
7z a ..\ifcopenshell.zip *
popd
copy conanfile.py build\package\ /Y
conan export-pkg build\package -of build\conan_export

endlocal
REM If double-click to run, pause
echo %cmdcmdline% | find /i "%~nx0" >nul && pause
