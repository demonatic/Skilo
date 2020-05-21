# Download and build libROCAPinYin

set(ROCAPinYin_VERSION master)
set(ROCAPinYin_NAME RocaPinyin-${ROCAPinYin_VERSION})
set(ROCAPinYin_TAR_PATH ${DEP_ROOT_DIR}/${ROCAPinYin_NAME}.tar.gz)
set(ROCAPinYin_LIBRARIES "${DEP_ROOT_DIR}/${ROCAPinYin_NAME}/librocapinyin.a")

if(NOT EXISTS ${ROCAPinYin_TAR_PATH})
    message(STATUS "Downloading libROCAPinYin...")
    file(DOWNLOAD https://github.com/m13253/RocaPinyin/archive/${ROCAPinYin_VERSION}.tar.gz ${ROCAPinYin_TAR_PATH})
endif()

if(NOT EXISTS ${DEP_ROOT_DIR}/${ROCAPinYin_NAME})
    message(STATUS "Extracting libROCAPinYin...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${ROCAPinYin_TAR_PATH} WORKING_DIRECTORY ${DEP_ROOT_DIR})
endif()

if(NOT EXISTS ${DEP_ROOT_DIR}/${ROCAPinYin_NAME}/librocapinyin.a AND BUILD_DEPS STREQUAL "yes")
    message("Building libROCAPinYin locally...")
    execute_process(COMMAND make WORKING_DIRECTORY ${DEP_ROOT_DIR}/${ROCAPinYin_NAME}/
                    RESULT_VARIABLE ROCAPinYin_BUILD)
    if(NOT ROCAPinYin_BUILD EQUAL 0)
        message(FATAL_ERROR "${ROCAPinYin_NAME} build failed!")
    endif()
endif()
