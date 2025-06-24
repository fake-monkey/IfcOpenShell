# ./CMakeLists.txt （放在项目根目录）
cmake_minimum_required(VERSION 3.20)
project(IfcOpenShellWrapper)

set(CONAN_OUT_CMAKE_FILE ${CMAKE_SOURCE_DIR}/build/generators/conan_toolchain.cmake)
if (EXISTS ${CMAKE_SOURCE_DIR}/conanfile.py AND EXISTS ${CONAN_OUT_CMAKE_FILE})
    include(${CONAN_OUT_CMAKE_FILE})
endif()

# 代理跳转到实际主项目
add_subdirectory(cmake)
