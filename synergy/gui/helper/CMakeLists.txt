cmake_minimum_required (VERSION 3.4)
project (com.symless.synergy.v2.ServiceHelper CXX)

add_executable (${PROJECT_NAME}
    main.cpp
)

target_include_directories (${PROJECT_NAME} SYSTEM PUBLIC
    lib
    ${fmt_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries (${PROJECT_NAME} "-sectcreate __TEXT __info_plist ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
target_link_libraries (${PROJECT_NAME} "-sectcreate __TEXT __launchd_plist ${CMAKE_CURRENT_SOURCE_DIR}/Launchd.plist")
target_link_libraries (${PROJECT_NAME} fmt)
target_link_libraries (${PROJECT_NAME} Boost::filesystem)

install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SYNERGY_BUNDLE_LAUNCH_DIR})
