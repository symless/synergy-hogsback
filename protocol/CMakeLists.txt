cmake_minimum_required (VERSION 3.4)

if (WIN32)
    set (flatc_BINARY_PATH "${CMAKE_BINARY_DIR}/third_party/flatbuffers/${CMAKE_CFG_INTDIR}/flatc.exe")
else()
    set (flatc_BINARY_PATH "${CMAKE_BINARY_DIR}/third_party/flatbuffers/flatc")
endif()

function (flatc_generate_cpp _FBS_FILES _FBS_HEADERS)
    set (${_FBS_HEADERS})
    foreach (FBS_FILE ${${_FBS_FILES}})
        get_filename_component (FBS_FILE_ABS ${FBS_FILE} ABSOLUTE)
        get_filename_component (FBS_FILE_NAME ${FBS_FILE} NAME)
        get_filename_component (FBS_FILE_WE ${FBS_FILE_NAME} NAME_WE)

        list (APPEND ${_FBS_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/${FBS_FILE_WE}_generated.h")

        add_custom_command (
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FBS_FILE_WE}_generated.h"
            COMMAND "${flatc_BINARY_PATH}"
            ARGS --cpp --scoped-enums --gen-object-api
		 		 -o ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR}
            	 ${FBS_FILE_ABS}
            DEPENDS ${FBS_FILE_ABS}
            COMMENT "Running Flatbuffers compiler on ${FBS_FILE_NAME}"
	    	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
    endforeach()
    set_source_files_properties (${${_FBS_HEADERS}} PROPERTIES GENERATED TRUE)
    set (${_FBS_HEADERS} ${${_FBS_HEADERS}} PARENT_SCOPE)
endfunction()

file (GLOB FBS_FILES *.fbs)
flatc_generate_cpp (FBS_FILES FBS_HEADERS)
add_library (fbs_protocol STATIC ${FBS_HEADERS})
set_target_properties (fbs_protocol PROPERTIES LINKER_LANGUAGE CXX)


file (GLOB CAPN_FILES *.capn)

if (WIN32)
    set (capnp_BINARY_PATH "${CMAKE_SOURCE_DIR}/third_party/capnproto-bin")
else()
    set (capnp_BINARY_PATH "${CMAKE_BINARY_DIR}/third_party/capnproto/c++/src/capnp")
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
add_library (capnp_protocol STATIC ${PROTO_SRCS})

target_compile_definitions (capnp_protocol PUBLIC CAPNP_LITE)
set_target_properties (capnp_protocol PROPERTIES LINKER_LANGUAGE CXX)
