cmake_minimum_required (VERSION 3.4)

add_subdirectory (fmt EXCLUDE_FROM_ALL)

set (ENABLE_CURVE OFF CACHE BOOL "Enable ZMQ CURVE security" FORCE)
set (WITH_OPENPGM OFF CACHE BOOL "Build ZMQ with support for OpenPGM" FORCE)
if (APPLE)
  set (ZMQ_BUILD_FRAMEWORK OFF CACHE BOOL "Build ZMQ as OS X framework" FORCE)
endif()
add_subdirectory (libzmq EXCLUDE_FROM_ALL)

if (WIN32)
    set (CAPNP_LITE ON CACHE BOOL "Compile Cap'n Proto in 'lite mode'" FORCE)
    add_definitions (-DEXTERNAL_CAPNP=1)
else()
    set (CAPNP_LITE OFF CACHE BOOL "Compile Cap'n Proto in 'lite mode'" FORCE)
endif()

set (BUILD_TESTING OFF CACHE BOOL "Build Cap'n Proto unit tests" FORCE)
add_subdirectory (capnproto/c++ EXCLUDE_FROM_ALL)

function(add_zdefs _target)
    if(CMAKE_COMPILER_IS_GNUCC)
        set(new_link_flags "-Wl,-z,defs")
        get_target_property(existing_link_flags ${_target} LINK_FLAGS)
        if(existing_link_flags)
            set(new_link_flags "${existing_link_flags} ${new_link_flags}")
        endif()
        set_target_properties(${_target} PROPERTIES LINK_FLAGS ${new_link_flags})
    endif()
endfunction()

set (PROTOLIB_SOURCE_DIR norm/protolib/src/common)
set (PROTOLIB_SOURCES
    "${PROTOLIB_SOURCE_DIR}/protoAddress.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoApp.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoBase64.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoBitmask.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoCap.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoChannel.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoDebug.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoDispatcher.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoGraph.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoLFSR.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoList.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoNet.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoPipe.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoPkt.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoPktARP.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoPktETH.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoPktIP.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoPktRTP.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoQueue.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoRouteMgr.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoRouteTable.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoSerial.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoSocket.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoSpace.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoTime.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoTimer.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoTree.cpp"
    "${PROTOLIB_SOURCE_DIR}/protoVif.cpp"
)

if (UNIX)
    list (APPEND PROTOLIB_SOURCES
        norm/protolib/src/unix/unixNet.cpp
        norm/protolib/src/unix/unixSerial.cpp
        norm/protolib/src/unix/unixVif.cpp
        norm/protolib/src/common/protoFile.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list (APPEND PROTOLIB_SOURCES
        norm/protolib/src/linux/linuxCap.cpp
        norm/protolib/src/linux/linuxNet.cpp
        norm/protolib/src/linux/linuxRouteMgr.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list (APPEND PROTOLIB_SOURCES
        norm/protolib/src/bsd/bsdDetour.cpp
        norm/protolib/src/bsd/bsdNet.cpp
        norm/protolib/src/bsd/bsdRouteMgr.cpp
        norm/protolib/src/unix/bpfCap.cpp
    )
endif()

if (WIN32)
    list (APPEND PROTOLIB_SOURCES
        norm/protolib/src/win32/win32Net.cpp
        norm/protolib/src/win32/win32RouteMgr.cpp
    )
endif()

add_library (protolib ${PROTOLIB_SOURCES})
target_include_directories (protolib BEFORE PUBLIC norm/protolib/include)
set_target_properties (protolib PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
#set_target_properties (protolib PROPERTIES COMPILE_FLAGS "-fno-rtti")

if (UNIX)
    target_compile_definitions (protolib PUBLIC -DUNIX -DHAVE_DIRFD -DHAVE_IPV6
                                -DHAVE_ASSERT -DHAVE_GETLOGIN)
    target_include_directories (protolib BEFORE PUBLIC
                                norm/protolib/include/unix)
    target_link_libraries (protolib pthread)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_compile_definitions (protolib PUBLIC -D_FILE_OFFSET_BITS=64 -DHAVE_LOCKF
                                -DHAVE_OLD_SIGNALHANDLER -DHAVE_SCHED -DLINUX
                                -DHAVE_TIMERFD -DNO_SCM_RIGHTS)
    target_link_libraries (protolib dl rt)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions (protolib PUBLIC -DMACOSX -DHAVE_FLOCK
                                -D_FILE_OFFSET_BITS=64 -DHAVE_DIRFD
                                -DHAVE_PSELECT)
    target_link_libraries (protolib resolv)
endif()

if (WIN32)
    target_compile_definitions (protolib PUBLIC -D_CRT_SECURE_NO_WARNINGS
                                -DHAVE_ASSERT -DWIN32 -DHAVE_IPV6)
    target_link_libraries (protolib ws2_32 iphlpapi user32 gdi32 Advapi32)
endif()

set (NORM_SOURCE_DIR norm/src/common)
set (NORM_SOURCES
    "${NORM_SOURCE_DIR}/galois.cpp"
    "${NORM_SOURCE_DIR}/normApi.cpp"
    "${NORM_SOURCE_DIR}/normEncoder.cpp"
    "${NORM_SOURCE_DIR}/normEncoderMDP.cpp"
    "${NORM_SOURCE_DIR}/normEncoderRS16.cpp"
    "${NORM_SOURCE_DIR}/normEncoderRS8.cpp"
    "${NORM_SOURCE_DIR}/normFile.cpp"
    "${NORM_SOURCE_DIR}/normMessage.cpp"
    "${NORM_SOURCE_DIR}/normNode.cpp"
    "${NORM_SOURCE_DIR}/normObject.cpp"
    "${NORM_SOURCE_DIR}/normSegment.cpp"
    "${NORM_SOURCE_DIR}/normSession.cpp"
)

add_library (norm SHARED ${NORM_SOURCES})
target_include_directories (norm BEFORE PUBLIC norm/include)
#set_target_properties (norm PROPERTIES COMPILE_FLAGS "-fno-rtti")

if (UNIX)
    target_compile_definitions (norm PUBLIC -DECN_SUPPORT)
endif()

if (WIN32)
    target_compile_definitions (norm PUBLIC -DNORM_USE_DLL)
endif()

add_zdefs (norm)
target_link_libraries (norm protolib)
