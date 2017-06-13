cmake_minimum_required (VERSION 3.4)
project (synergy-plagent)

add_executable (${PROJECT_NAME}
    main.mm
    LogManager.mm
    AppDelegate.mm
)

find_library (COCOA_LIBRARY Cocoa)
target_link_libraries (${PROJECT_NAME} ${COCOA_LIBRARY})

install (TARGETS ${PROJECT_NAME} DESTINATION ${SYNERGY_BUNDLE_BINARY_DIR})
