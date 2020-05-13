set(CROARING_VERSION 0.2.65)
set(CROARING_NAME CRoaring-${CROARING_VERSION})
set(CROARING_TAR_PATH ${DEP_ROOT_DIR}/${CROARING_NAME}.tar.gz)

if(NOT EXISTS ${CROARING_TAR_PATH})
    message(STATUS "Downloading CRoaring...")
    file(DOWNLOAD https://github.com/RoaringBitmap/CRoaring/archive/v${CROARING_VERSION}.tar.gz ${CROARING_TAR_PATH})
endif()

if(NOT EXISTS ${DEP_ROOT_DIR}/${CROARING_NAME})
    message(STATUS "Extracting libCRoaring...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CROARING_TAR_PATH} WORKING_DIRECTORY ${DEP_ROOT_DIR}/)
endif()

if(NOT EXISTS ${DEP_ROOT_DIR}/${CROARING_NAME}/build/ AND BUILD_DEPS STREQUAL "yes")
    message(STATUS "Configuring CRoaring ...")
    execute_process(COMMAND ${CMAKE_COMMAND}
                    "-DCMAKE_CXX_COMPILER=g++"
                    "-DENABLE_ROARING_TESTS=OFF"
                    "-H${DEP_ROOT_DIR}/${CROARING_NAME}"
                    "-B${DEP_ROOT_DIR}/${CROARING_NAME}/build"
                    RESULT_VARIABLE
                    CROARING_CONFIGURE)
    if(NOT CROARING_CONFIGURE EQUAL 0)
        message(FATAL_ERROR "${CROARING_NAME} configure failed!")
    endif()

    message(STATUS "Building CRoaring ...")
    file(MAKE_DIRECTORY ${DEP_ROOT_DIR}/${CROARING_NAME}/build/)
    execute_process(COMMAND ${CMAKE_COMMAND} --build
                    "${DEP_ROOT_DIR}/${CROARING_NAME}/build"
                    RESULT_VARIABLE
                    CROARING_BUILD)

    if(NOT CROARING_BUILD EQUAL 0)
        message(FATAL_ERROR "${CROARING_NAME} build failed!")
    endif()

    #have to install manually since libfor doesn't provide make install

endif()

