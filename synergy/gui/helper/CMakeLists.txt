cmake_minimum_required (VERSION 3.4)
project (com.symless.synergy.v2.ServiceHelper CXX)

add_executable (${PROJECT_NAME}
    main.cpp
)

target_link_libraries (${PROJECT_NAME} "-sectcreate __TEXT __info_plist ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
target_link_libraries (${PROJECT_NAME} "-sectcreate __TEXT __launchd_plist ${CMAKE_CURRENT_SOURCE_DIR}/Launchd.plist")
install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SYNERGY_BUNDLE_LAUNCH_DIR})
