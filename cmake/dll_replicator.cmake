function(auto_copy_dll TARGET_NAME)
    get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
    if (${TARGET_TYPE} STREQUAL "EXECUTABLE")
        string(JOIN $<SEMICOLON> COPY_DLL_CMD ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${TARGET_NAME}> $<TARGET_FILE_DIR:${TARGET_NAME}>)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:${TARGET_NAME}>>,${COPY_DLL_CMD},>
            COMMAND_EXPAND_LISTS)
    endif()
endfunction()
