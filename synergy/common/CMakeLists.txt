cmake_minimum_required (VERSION 3.4)

if (WIN32)
    file(GLOB platform_sources "MSWindows*.cpp")
elseif (APPLE)
    file(GLOB platform_sources "OSX*.cpp")
elseif (UNIX)
    file(GLOB platform_sources "XWindows*.cpp")
endif()

set (COMMON_LIB_SOURCES
    ConfigParser.cpp
    GlobalConfig.cpp
    UserConfig.cpp
    DirectoryManager.cpp
    RpcManager.cpp
    WampClient.cpp
    WampServer.cpp
    WampRouter.cpp
    ConfigGen.cpp
    ProfileConfig.cpp
    Profile.cpp
    Screen.cpp
    ScreenLinks.cpp
    ProcessCommand.cpp
    ${platform_sources}
)

if (SYNERGY_CRASHPAD)
    list (APPEND COMMON_LIB_SOURCES CrashHandler.cpp)
endif()

set (Bonefish_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/bonefish/src")
set (Cpptoml_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/cpptoml/include")

include_directories (
    BEFORE SYSTEM
    ${Bonefish_INCLUDE_DIR}
    ${Cpptoml_INCLUDE_DIR}
)

add_library (commonlib ${COMMON_LIB_SOURCES})
target_link_libraries(commonlib ${Boost_LIBRARIES} bonefish fmt)

if (WIN32)
    target_link_libraries (commonlib Wtsapi32)
endif()

if (SYNERGY_CRASHPAD)
    if (WIN32)
        target_link_libraries (commonlib ws2_32 rpcrt4 iphlpapi)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_link_libraries (commonlib
                "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/x64/crashpad_client.lib"
                "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/x64/crashpad_util.lib"
                "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/x64/base.lib"
            )
        else()
            target_link_libraries (commonlib
                "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/x86/crashpad_client.lib"
                "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/x86/crashpad_util.lib"
                "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/windows/x86/base.lib"
            )
        endif()
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        find_library (IOKit_LIBRARY IOKit)
        find_library (Security_LIBRARY Security)
        find_library (Foundation_LIBRARY Foundation)
        find_library (ApplicationServices_LIBRARY ApplicationServices)

        target_link_libraries (commonlib
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/libcrashpad_client.a"
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/libcrashpad_util.a"
            "${CMAKE_SOURCE_DIR}/third_party/precompiled/crashpad/macos/libbase.a"
            "/usr/lib/libbsm.0.dylib"
            ${IOKit_LIBRARY}
            ${Security_LIBRARY}
            ${Foundation_LIBRARY}
            ${ApplicationServices_LIBRARY}
        )
    endif()
endif()
