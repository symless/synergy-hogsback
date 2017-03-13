cmake_minimum_required (VERSION 3.4)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                       "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

#
# Version
#
if ((NOT DEFINED SYNERGY_VERSION_MAJOR) AND (DEFINED ENV{SYNERGY_VERSION_MAJOR}))
    set (SYNERGY_VERSION_MAJOR $ENV{SYNERGY_VERSION_MAJOR})
    set (SYNERGY_VERSION_MINOR $ENV{SYNERGY_VERSION_MINOR})
    set (SYNERGY_VERSION_PATCH $ENV{SYNERGY_VERSION_PATCH})
    set (SYNERGY_VERSION_STAGE $ENV{SYNERGY_VERSION_STAGE})
endif()

if (DEFINED SYNERGY_VERSION_MAJOR)
    set (VERSION_MAJOR ${SYNERGY_VERSION_MAJOR})
    set (VERSION_MINOR ${SYNERGY_VERSION_MINOR})
    set (VERSION_REV ${SYNERGY_VERSION_PATCH})
    set (VERSION_STAGE ${SYNERGY_VERSION_STAGE})
else()
    message (WARNING "Synergy version number not set by build environment.")
    set (VERSION_MAJOR 2)
    set (VERSION_MINOR 0)
    set (VERSION_REV 0)
    set (VERSION_STAGE git)
endif()

set (SYNERGY_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")
set (VERSION ${SYNERGY_VERSION})
message (STATUS "Synergy version is " ${SYNERGY_VERSION})


if (WIN32)
    # Configure for Windows Vista and above. Vista has extended support from
    # Microsoft until April 11, 2017
    # https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
    add_definitions (/DNTDDI_VERSION=0x06000000)
    add_definitions (/D_WIN32_WINNT=0x0600)
    add_definitions (/DWINVER=0x0600)

    if (MSVC)
        #set (SYNERGY_WARNING_FLAGS /W4)
    endif()

elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set (SYNERGY_WARNING_FLAGS  -Weverything
                                -Wno-c++98-compat -Wno-c++98-compat-pedantic)

elseif (CMAKE_COMPILER_IS_GNUCXX)
    set (SYNERGY_WARNING_FLAGS -Wall -Wextra -Wpedantic
                               -Wconversion -Wsign-conversion -Wformat=2)
    set (SYNERGY_CXX_FLAGS_RELEASE "-Os -flto -fvisibility=hidden -DNDEBUG")

endif()


if (NOT DEFINED SCAN_BUILD)
    add_definitions (${SYNERGY_WARNING_FLAGS})
endif()


if ((DEFINED SYNERGY_CXX_FLAGS_RELEASE) AND (CMAKE_BUILD_TYPE EQUAL "Release"))
    set (CMAKE_CXX_FLAGS_RELEASE ${SYNERGY_CXX_FLAGS_RELEASE})
elseif ((DEFINED SYNERGY_CXX_FLAGS_DEBUG) AND (CMAKE_BUILD_TYPE EQUAL "Debug"))
    set (CMAKE_CXX_FLAGS_DEBUG ${SYNERGY_CXX_FLAGS_DEBUG})
endif()


set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_USE_MULTITHREADED ON)

find_package (Threads REQUIRED)
find_package (Boost 1.53 REQUIRED COMPONENTS system date_time)

add_definitions (-DZMQ_STATIC)
add_definitions (-DZMQ_BUILD_DRAFT_API)
add_definitions (-DZMQ_USE_LIBSODIUM)
include_directories (BEFORE SYSTEM third_party/libzmq/include)
include_directories (BEFORE SYSTEM third_party/cppzmq)

include_directories (third_party/catch/include)
add_subdirectory (third_party)

#
# Configure_file... but for directories, recursively.
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

include_directories (.)
add_subdirectory (synergy)