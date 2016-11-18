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
add_library (fbs_protocol STATIC ${FBS_HEADERS} empty.cpp)
set_target_properties (fbs_protocol PROPERTIES LINKER_LANGUAGE CXX)
