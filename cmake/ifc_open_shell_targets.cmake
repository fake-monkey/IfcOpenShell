# 从 cmake/CMakeList.tet 中复制的内容

function (link_opencascade TARGET_NAME SCOPE)
    find_package(OpenCASCADE CONFIG REQUIRED)
    set(OPENCASCADE_LIBRARY_NAMES
        TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKG3d TKG2d TKShHealing TKTopAlgo TKMesh TKPrim TKBool TKBO
        TKFillet TKXSBase TKOffset TKHLR

        # @todo investigate the exact conditions when this is necessary
        TKBin
    )
    
    # 下面流程摘抄自 cmake/CMakeLists.txt

    if(OCC_INCLUDE_DIR)
        file(STRINGS ${OCC_INCLUDE_DIR}/Standard_Version.hxx OCC_MAJOR
            REGEX "#define OCC_VERSION_MAJOR.*"
        )
        string(REGEX MATCH "[0-9]+" OCC_MAJOR ${OCC_MAJOR})
        file(STRINGS ${OCC_INCLUDE_DIR}/Standard_Version.hxx OCC_MINOR
          REGEX "#define OCC_VERSION_MINOR.*"
        )
        string(REGEX MATCH "[0-9]+" OCC_MINOR ${OCC_MINOR})
        file(STRINGS ${OCC_INCLUDE_DIR}/Standard_Version.hxx OCC_MAINT
          REGEX "#define OCC_VERSION_MAINTENANCE.*"
        )
        string(REGEX MATCH "[0-9]+" OCC_MAINT ${OCC_MAINT})
        set(OCC_VERSION_STRING "${OCC_MAJOR}.${OCC_MINOR}.${OCC_MAINT}")
    endif(OCC_INCLUDE_DIR)

    if(OCC_VERSION_STRING VERSION_LESS 7.8.0)
        list(APPEND OPENCASCADE_LIBRARY_NAMES  TKIGES TKSTEPBase TKSTEPAttr TKSTEP209 TKSTEP)
    else(OCC_VERSION_STRING VERSION_LESS 7.8.0)
        list(APPEND OPENCASCADE_LIBRARY_NAMES TKDESTEP TKDEIGES)
    endif(OCC_VERSION_STRING VERSION_LESS 7.8.0)
    
    foreach(OPENCASCADE_TARGET IN LISTS OPENCASCADE_LIBRARY_NAMES)
        target_link_libraries(${TARGET_NAME} ${SCOPE} ${OPENCASCADE_TARGET})
    endforeach()
endfunction()

function (link_eigen TARGET_NAME SCOPE)
    find_package(Eigen3 CONFIG REQUIRED)
    target_link_libraries(${TARGET_NAME} ${SCOPE} Eigen3::Eigen)
endfunction()
