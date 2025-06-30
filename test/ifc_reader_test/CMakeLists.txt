target_common_preprocess(TARGET_NAME SRC_FILES)

add_executable(${TARGET_NAME} ${SRC_FILES})

include (${CMAKE_SOURCE_DIR}/cmake/ifc_open_shell_targets.cmake)
link_opencascade(${TARGET_NAME} PRIVATE)
link_eigen(${TARGET_NAME} PRIVATE)

target_link_libraries(${TARGET_NAME} PRIVATE IfcParse IfcGeom)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)

target_common_postprocess(${TARGET_NAME})
