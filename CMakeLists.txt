# ./CMakeLists.txt （放在项目根目录）
cmake_minimum_required(VERSION 3.20)
project(IfcOpenShellWrapper)

if (EXISTS ${CMAKE_SOURCE_DIR}/conanfile.py)
    execute_process(
        COMMAND conan install ${CMAKE_SOURCE_DIR} --output=${CMAKE_SOURCE_DIR}/build/generators --build=never -r=fscut-conan-center --update
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Failed to execute conan install command")
    endif()

    include(${CMAKE_SOURCE_DIR}/build/generators/conan_toolchain.cmake)
endif()

# 代理跳转到实际主项目
add_subdirectory(cmake)
