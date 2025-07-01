# ./CMakeLists.txt （放在项目根目录）
cmake_minimum_required(VERSION 3.20)
project(IfcOpenShellWrapper)

set (CONAN_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/generators)
if (EXISTS ${CMAKE_SOURCE_DIR}/conanfile.py)
    execute_process(
        COMMAND conan install ${CMAKE_SOURCE_DIR} --output=${CONAN_OUTPUT_DIR} --build=never -r=fscut-conan-center --update
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Failed to execute conan install command")
    endif()
    
    include(${CONAN_OUTPUT_DIR}/conan_toolchain.cmake)
endif()

find_package(OpenCASCADE CONFIG REQUIRED)
set (OCC_INCLUDE_DIR ${OpenCASCADE_INCLUDE_DIR})
set (OCC_LIBRARY_DIR ${OpenCASCADE_LIBRARY_DIR})
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set (OCC_LIBRARY_DIR ${OCC_LIBRARY_DIR}d)
endif()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

find_package(Eigen3 CONFIG REQUIRED)
set (EIGEN_DIR ${EIGEN3_INCLUDE_DIR})

if (MSVC)
    add_compile_options(/MP)
endif()

# 代理跳转到实际主项目
add_subdirectory(cmake)

install(EXPORT IfcOpenShell
    DESTINATION cmake
    FILE IfcOpenShellTargets.cmake
    NAMESPACE IfcOpenShell::
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/IfcOpenShellConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/IfcOpenShellConfig.cmake"
    INSTALL_DESTINATION cmake
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/IfcOpenShellConfig.cmake"
    DESTINATION cmake
)

include(cmake/target_common.cmake)

add_subdirectory(test/ifc_reader_test)
