set(CUCKOO_VERSION 0.2)
set(CUCKOO_NAME libcuckoo-${CUCKOO_VERSION})
set(CUCKOO_TAR_PATH ${DEP_ROOT_DIR}/${CUCKOO_NAME}.tar.gz)

if(NOT EXISTS ${CUCKOO_TAR_PATH})
    message(STATUS "Downloading libcuckoo...")
    file(DOWNLOAD https://github.com/efficient/libcuckoo/archive/v${CUCKOO_VERSION}.tar.gz ${CUCKOO_TAR_PATH})
endif()

if(NOT EXISTS ${DEP_ROOT_DIR}/${CUCKOO_NAME})
    message(STATUS "Extracting libcuckoo...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CUCKOO_TAR_PATH} WORKING_DIRECTORY ${DEP_ROOT_DIR}/)
endif()

if(NOT EXISTS ${DEP_ROOT_DIR}/${CUCKOO_NAME}/build/ AND BUILD_DEPS STREQUAL "yes")
    message(STATUS "Configuring CUCKOO ...")
    execute_process(COMMAND ${CMAKE_COMMAND}
                    "-DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/3rd"
                    "-DBUILD_TESTS=OFF"
                    "-H${DEP_ROOT_DIR}/${CUCKOO_NAME}"
                    "-B${DEP_ROOT_DIR}/${CUCKOO_NAME}/build"
                    RESULT_VARIABLE
                    CUCKOO_CONFIGURE)
    if(NOT CUCKOO_CONFIGURE EQUAL 0)
        message(FATAL_ERROR "${CUCKOO_NAME} configure failed!")
    endif()

    message(STATUS "Building CUCKOO ...")
    file(MAKE_DIRECTORY ${DEP_ROOT_DIR}/${CUCKOO_NAME}/build/)
    execute_process(COMMAND ${CMAKE_COMMAND} --build
                    "${DEP_ROOT_DIR}/${CUCKOO_NAME}/build"
                    RESULT_VARIABLE
                    CUCKOO_BUILD)

    if(NOT CUCKOO_BUILD EQUAL 0)
        message(FATAL_ERROR "${CUCKOO_NAME} build failed!")
    endif()

endif()

