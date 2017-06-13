cmake_minimum_required (VERSION 3.4)
project (synergy-helper CXX)

add_executable (${PROJECT_NAME}
    main.cpp
)

install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SYNERGY_BUNDLE_LAUNCH_DIR}
                 RENAME "com.symless.synergy.v2.ServiceHelper")
