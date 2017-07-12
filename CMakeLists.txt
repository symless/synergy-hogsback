cmake_minimum_required (VERSION 3.4)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set (CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

#
# Synergy Options
#
option (SYNERGY_DEVELOPER_MODE "Synergy: Turn on secret developer features" OFF)

#
# Synergy Version
#
include (cmake/Version.cmake)

#
# Compiler flags
#
if (WIN32)
    # Configure for Windows 7 and above. Vista went out of extended support from
    # Microsoft on April 11th 2017
    # Docs: https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx

    add_definitions (/DNTDDI_VERSION=0x06010000)
    add_definitions (/D_WIN32_WINNT=0x0601)
    add_definitions (/DWINVER=0x0601)

    if (MSVC)
		set (SYNERGY_WARNING_FLAGS /W4)
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

    if ((DEFINED SYNERGY_CXX_FLAGS_RELEASE) AND (CMAKE_BUILD_TYPE EQUAL "Release"))
        set (CMAKE_CXX_FLAGS_RELEASE ${SYNERGY_CXX_FLAGS_RELEASE})
    elseif ((DEFINED SYNERGY_CXX_FLAGS_DEBUG) AND (CMAKE_BUILD_TYPE EQUAL "Debug"))
        set (CMAKE_CXX_FLAGS_DEBUG ${SYNERGY_CXX_FLAGS_DEBUG})
    endif()
endif()

#
# Non-CMake Dependencies
#

# libsodium
#
find_package (Sodium REQUIRED)
find_package (Threads REQUIRED)

# ZeroMQ
#
add_definitions (-DZMQ_STATIC)
add_definitions (-DZMQ_BUILD_DRAFT_API)
include_directories (BEFORE SYSTEM third_party/libzmq/include)
include_directories (BEFORE SYSTEM third_party/cppzmq)

#
# Msgpack
#
set (Msgpack_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/msgpack-c/include")
add_definitions (-DMSGPACK_DEFAULT_API_VERSION=1)
include_directories (${Msgpack_INCLUDE_DIR})

#
# Websocket++
#
set (Websocketpp_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/websocketpp")
include_directories (${Websocketpp_INCLUDE_DIR})

#
# Google Breakpad
#
if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    include_directories (BEFORE SYSTEM "${CMAKE_SOURCE_DIR}/third_party/crashpad")
    include_directories (BEFORE SYSTEM "${CMAKE_SOURCE_DIR}/third_party/mini_chromium")
endif()

# Boost
#
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_USE_MULTITHREADED ON)
find_package (Boost 1.55 REQUIRED COMPONENTS system date_time filesystem regex thread chrono)
include_directories(${Boost_INCLUDE_DIRS})

#
# CMake Dependencies
#
add_subdirectory (third_party)

#
# Packaging
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
# macOS Bundle
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (SYNERGY_BUNDLE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dist/macos/bundle)
    set (SYNERGY_BUNDLE_DIR ${CMAKE_BINARY_DIR}/bundle)
    set (SYNERGY_BUNDLE_APP_DIR ${SYNERGY_BUNDLE_DIR}/Synergy.app)
    set (SYNERGY_BUNDLE_BINARY_DIR ${SYNERGY_BUNDLE_APP_DIR}/Contents/MacOS)
    set (SYNERGY_BUNDLE_LAUNCH_DIR ${SYNERGY_BUNDLE_APP_DIR}/Contents/Library/LaunchServices)

    configure_files (${SYNERGY_BUNDLE_SOURCE_DIR} ${SYNERGY_BUNDLE_DIR})
    set (CMAKE_INSTALL_RPATH "@loader_path/../Libraries;@loader_path/../Frameworks")
endif()

#
# Windows installer
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    configure_files (${CMAKE_CURRENT_SOURCE_DIR}/dist/wix ${CMAKE_BINARY_DIR}/installer)
endif()

#
# Linux RPM
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    configure_files (${CMAKE_CURRENT_SOURCE_DIR}/dist/rpm ${CMAKE_BINARY_DIR}/rpm)
    install(FILES synergy/core/res/synergy.svg DESTINATION share/icons)
    install(FILES synergy/core/res/synergy2.desktop DESTINATION share/applications)
endif()

include_directories (.)
add_subdirectory (synergy)
