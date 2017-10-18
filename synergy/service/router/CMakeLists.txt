cmake_minimum_required (VERSION 3.4)

add_library (
    router
    Router.cpp
    Connection.cpp
    Message.cpp
    ClientProxy.cpp
    ServerProxy.cpp
    protocol/v1/MessageTypes.cpp
    protocol/v1/Scanner.cpp
)

target_compile_definitions (router
    PRIVATE -DBOOST_COROUTINES_NO_DEPRECATION_WARNING
    PRIVATE -DCXXOPTS_NO_RTTI)

target_link_libraries (router
    ${SODIUM_LIBRARIES}
    ${Boost_LIBRARIES}
)
