cmake_minimum_required (VERSION 3.4)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                       "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package (Sodium)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

#
# Deploy
#
set (SYNERGY_DEVELOP_ENV on)

#
# Version
#
if (NOT DEFINED SYNERGY_VERSION_MAJOR)
    if (DEFINED ENV{SYNERGY_VERSION_MAJOR})
        set (SYNERGY_VERSION_MAJOR $ENV{SYNERGY_VERSION_MAJOR})
    else()
        set (SYNERGY_VERSION_MAJOR 1)
        set (SYNERGY_VERSION_WARNED 1)
        message (WARNING "SYNERGY_VERSION_MAJOR not set. Defaulted to ${SYNERGY_VERSION_MAJOR}")
    endif()
endif()

if (NOT DEFINED SYNERGY_VERSION_MINOR)
    if (DEFINED ENV{SYNERGY_VERSION_MINOR})
        set (SYNERGY_VERSION_MINOR $ENV{SYNERGY_VERSION_MINOR})
    else()
        set (SYNERGY_VERSION_MINOR 9)
        if (NOT ${SYNERGY_VERSION_WARNED})
            set (SYNERGY_VERSION_WARNED 1)
            message (WARNING "(SYNERGY_VERSION_MINOR not set. Defaulted to ${SYNERGY_VERSION_MINOR}")
        endif()
    endif()
endif()

if (NOT DEFINED SYNERGY_VERSION_PATCH)
    if (DEFINED ENV{SYNERGY_VERSION_PATCH})
        set (SYNERGY_VERSION_PATCH $ENV{SYNERGY_VERSION_PATCH})
    else()
        set (SYNERGY_VERSION_PATCH 0)
        if (NOT ${SYNERGY_VERSION_WARNED})
            set (SYNERGY_VERSION_WARNED 1)
            message (WARNING "SYNERGY_VERSION_PATCH not set. Defaulted to ${SYNERGY_VERSION_PATCH}")
        endif()
    endif()
endif()

if (NOT DEFINED SYNERGY_VERSION_STAGE)
    if (DEFINED ENV{SYNERGY_VERSION_STAGE})
        set (SYNERGY_VERSION_STAGE $ENV{SYNERGY_VERSION_STAGE})
    else()
        set (SYNERGY_VERSION_STAGE "git")
        if (NOT ${SYNERGY_VERSION_WARNED})
            set (SYNERGY_VERSION_WARNED 1)
            message (WARNING "SYNERGY_VERSION_STAGE not set. Defaulted to '${SYNERGY_VERSION_STAGE}'")
        endif()
    endif()
endif()

if (NOT DEFINED SYNERGY_REVISION)
    if (DEFINED ENV{GIT_COMMIT})
        string (SUBSTRING $ENV{GIT_COMMIT} 0 8 SYNERGY_REVISION)
    elseif (SYNERGY_VERSION_STAGE STREQUAL "git")
        execute_process (
            COMMAND git rev-parse --short=8 HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE SYNERGY_REVISION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
endif()

if (DEFINED SYNERGY_REVISION)
    string(LENGTH ${SYNERGY_REVISION} SYNERGY_REVISION_LENGTH)
    if (NOT ((SYNERGY_REVISION MATCHES "^[a-f0-9]+") AND (SYNERGY_REVISION_LENGTH EQUAL "8")))
        message (FATAL_ERROR "SYNERGY_REVISION='${SYNERGY_REVISION}' should be a short git commit hash")
    endif()
    unset (SYNERGY_REVISION_LENGTH)
else()

endif()
if (DEFINED ENV{BUILD_NUMBER})
    set (SYNERGY_BUILD_NUMBER $ENV{BUILD_NUMBER})
else()
    set (SYNERGY_BUILD_NUMBER 1)
endif()

string (TIMESTAMP SYNERGY_BUILD_DATE "%Y%m%d" UTC)

if (SYNERGY_VERSION_STAGE STREQUAL "git")
    set (SYNERGY_SNAPSHOT_INFO ".${SYNERGY_BUILD_DATE}.${SYNERGY_REVISION}")
    set (SYNERGY_VERSION_TAG "b${SYNERGY_BUILD_DATE}-${SYNERGY_REVISION}")
else()
    set (SYNERGY_SNAPSHOT_INFO "")
    set (SYNERGY_VERSION_TAG "${SYNERGY_VERSION_STAGE}")
endif()

set (SYNERGY_VERSION "${SYNERGY_VERSION_MAJOR}.${SYNERGY_VERSION_MINOR}.${SYNERGY_VERSION_PATCH}")
set (SYNERGY_VERSION_STRING "${SYNERGY_VERSION}-${SYNERGY_VERSION_TAG}")
message (STATUS "Full Synergy version string is " ${SYNERGY_VERSION_STRING})

if (WIN32)
    # Configure for Windows 7 and above. Vista went out of extended support from
    # Microsoft on April 11th 2017
    # Docs: https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx

    add_definitions (/DNTDDI_VERSION=0x06010000)
    add_definitions (/D_WIN32_WINNT=0x0601)
    add_definitions (/DWINVER=0x0601)

    if (MSVC)
        #set (SYNERGY_WARNING_FLAGS /W4)
    endif()
elseif (UNIX)
    if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        set (SYNERGY_WARNING_FLAGS  -Weverything
                                    -Wno-c++98-compat -Wno-c++98-compat-pedantic)
        set (SYNERGY_CXX_FLAGS_RELEASE "-O2 -flto -fvisibility=hidden -DNDEBUG")

    elseif (CMAKE_COMPILER_IS_GNUCXX)
        set (SYNERGY_WARNING_FLAGS -Wall -Wextra -Wpedantic
                                   -Wconversion -Wsign-conversion -Wformat=2)
        set (SYNERGY_CXX_FLAGS_RELEASE "-O2 -flto -fvisibility=hidden -DNDEBUG")
    endif()

    if (NOT DEFINED SYNERGY_SCAN_BUILD)
        add_definitions (${SYNERGY_WARNING_FLAGS})
    endif()

    # TODO: cleanup if statements
    if ((DEFINED SYNERGY_CXX_FLAGS_RELEASE) AND (CMAKE_BUILD_TYPE EQUAL "Release"))
        set (CMAKE_CXX_FLAGS_RELEASE ${SYNERGY_CXX_FLAGS_RELEASE})
    elseif ((DEFINED SYNERGY_CXX_FLAGS_DEBUG) AND (CMAKE_BUILD_TYPE EQUAL "Debug"))
        set (CMAKE_CXX_FLAGS_DEBUG ${SYNERGY_CXX_FLAGS_DEBUG})
    endif()
endif()

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_USE_MULTITHREADED ON)

find_package (Threads REQUIRED)

# TODO: check the minimum version on CentOS 7 and Debian Jessie
find_package (Boost 1.53 REQUIRED COMPONENTS system date_time)

add_definitions (-DZMQ_STATIC)
add_definitions (-DZMQ_BUILD_DRAFT_API)

# TODO: only set this on Windows?
# add_definitions (-DZMQ_USE_LIBSODIUM)

include_directories (BEFORE SYSTEM third_party/libzmq/include)
include_directories (BEFORE SYSTEM third_party/cppzmq)
include_directories (BEFORE SYSTEM third_party/catch/include)

add_subdirectory (third_party)

# TODO: move this to an included file
# TODO: move this up?

#
# Like the CMake configure_file macro, but for directories. Works recursively.
#
macro (configure_files srcDir destDir)
    message (STATUS "Configuring directory ${destDir}")
    make_directory (${destDir})

    file (GLOB_RECURSE sourceFiles RELATIVE ${srcDir} ${srcDir}/*)
    file (GLOB_RECURSE templateFiles LIST_DIRECTORIES false RELATIVE ${srcDir} ${srcDir}/*.in)
    list (REMOVE_ITEM sourceFiles ${templateFiles})

    foreach (sourceFile ${sourceFiles})
        set (sourceFilePath ${srcDir}/${sourceFile})
        if (IS_DIRECTORY ${sourceFilePath})
            message (STATUS "Copying directory ${sourceFile}")
            make_directory (${destDir/${sourceFile})
        else()
            message (STATUS "Copying file ${sourceFile}")
            configure_file (${sourceFilePath} ${destDir}/${sourceFile} COPYONLY)
        endif()
    endforeach (sourceFile)

    foreach (templateFile ${templateFiles})
        set (sourceTemplateFilePath ${srcDir}/${templateFile})
                string (REGEX REPLACE "\.in$" "" templateFile ${templateFile})
        message (STATUS "Configuring file ${templateFile}")
        configure_file (${sourceTemplateFilePath} ${destDir}/${templateFile} @ONLY)
    endforeach (templateFile)
endmacro (configure_files)

#
# macOS app Bundle
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (CMAKE_INSTALL_RPATH "@loader_path/../Libraries;@loader_path/../Frameworks")

    set (SYNERGY_BUNDLE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dist/macos/bundle)
    set (SYNERGY_BUNDLE_DIR ${CMAKE_BINARY_DIR}/bundle)
    set (SYNERGY_BUNDLE_APP_DIR ${SYNERGY_BUNDLE_DIR}/Synergy.app)
    set (SYNERGY_BUNDLE_BINARY_DIR ${SYNERGY_BUNDLE_APP_DIR}/Contents/MacOS)

    configure_files (${SYNERGY_BUNDLE_SOURCE_DIR} ${SYNERGY_BUNDLE_DIR})
endif()

#
# Windows installer
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    configure_files (${CMAKE_CURRENT_SOURCE_DIR}/dist/wix ${CMAKE_BINARY_DIR}/installer)
endif()

#
# Linux installation
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    configure_files (${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm ${CMAKE_BINARY_DIR}/rpm)
    install(FILES synergy/core/res/synergy.svg DESTINATION share/icons)
    install(FILES synergy/core/res/synergy2.desktop DESTINATION share/applications)
endif()

include_directories (.)
add_subdirectory (synergy)
