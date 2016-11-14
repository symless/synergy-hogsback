cmake_minimum_required (VERSION 3.4)

file (GLOB CAPN_FILES *.capn)

if (WIN32)
    set (capnp_BINARY_PATH "${CMAKE_SOURCE_DIR}/third_party/capnproto-bin")
else()
    set (capnp_BINARY_PATH "${CMAKE_BINARY_DIR}/third_party/capnproto/c++/src/capnp/")
endif()

set (capnp_PLUGIN_PATH "${capnp_BINARY_PATH}/capnpc-c++${CMAKE_EXECUTABLE_SUFFIX}")
set (capnp_PROGRAM_PATH "${capnp_BINARY_PATH}/capnp${CMAKE_EXECUTABLE_SUFFIX}")

function (capnproto_generate_cpp SRCS HDRS)
    set (${SRCS})
    set (${HDRS})
    foreach (CAPN_FILE ${CAPN_FILES})
        get_filename_component (CAPN_FILE_ABS ${CAPN_FILE} ABSOLUTE)
        get_filename_component (CAPN_FILE_DIR ${CAPN_FILE} DIRECTORY)
        get_filename_component (CAPN_FILE_NAME ${CAPN_FILE} NAME)
        get_filename_component (CAPN_FILE_BASE ${CAPN_FILE_NAME} NAME_WE)

        list (APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${CAPN_FILE_BASE}.capn.c++")
        list (APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${CAPN_FILE_BASE}.capn.h")

        add_custom_command (
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${CAPN_FILE_BASE}.capn.c++"
                   "${CMAKE_CURRENT_BINARY_DIR}/${CAPN_FILE_BASE}.capn.h"
            COMMAND "${capnp_PROGRAM_PATH}"
            ARGS compile -o ${capnp_PLUGIN_PATH}:${CMAKE_CURRENT_BINARY_DIR}
                         --src-prefix=${CAPN_FILE_DIR} ${CAPN_FILE_ABS}
            DEPENDS ${CAPN_FILE_ABS}
            COMMENT "Running Cap'n Proto compiler on ${CAPN_FILE}"
            VERBATIM
        )
    endforeach()
    set_source_files_properties (${${SRCS}} ${${HDRS}}
                                 PROPERTIES GENERATED TRUE)
    set (${SRCS} ${${SRCS}} PARENT_SCOPE)
    set (${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

capnproto_generate_cpp (PROTO_SRCS PROTO_HDRS)
add_library (protocol STATIC ${PROTO_SRCS})

target_compile_definitions (protocol PUBLIC CAPNP_LITE)
set_target_properties (protocol PROPERTIES LINKER_LANGUAGE CXX)
