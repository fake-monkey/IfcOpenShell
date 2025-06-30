# 在多个项目之间都可以通用的 cmake 配置。

include (${CMAKE_CURRENT_LIST_DIR}/dll_replicator.cmake)

function(target_common_preprocess TARGET_NAME SRC_FILES)
    get_filename_component(_TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    file (GLOB_RECURSE _SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c* *.h)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_SRC_FILES})

    set (${TARGET_NAME} ${_TARGET_NAME} PARENT_SCOPE)
    set (${SRC_FILES} ${_SRC_FILES} PARENT_SCOPE)
endfunction()

function(target_common_postprocess TARGET_NAME)
    # 执行一些所有项目都需要执行的动作。
    auto_copy_dll(${TARGET_NAME})
endfunction()
