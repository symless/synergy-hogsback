project (synergy-tests CXX)
cmake_minimum_required (VERSION 3.4)

file(GLOB TEST_SOURCES "*.cpp")

add_executable (${PROJECT_NAME} ${TEST_SOURCES})

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/catch/include
    ${Boost_INCLUDE_DIRS}
)

target_compile_definitions (${PROJECT_NAME}
    PRIVATE -DBOOST_COROUTINES_NO_DEPRECATION_WARNING
)

target_link_libraries (${PROJECT_NAME}
    commonlib
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install (TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

