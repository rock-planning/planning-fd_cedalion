cmake_minimum_required(VERSION 2.6)
find_package(Rock)
rock_init(cedalion 0.1)

add_definitions(-Wno-deprecated)
add_definitions(-Wextra -Wno-sign-compare)
add_definitions(-DUSE_LP=1)
add_definitions(-DCOIN_USE_CLP=1)
add_definitions(-DLINK_RELEASE_STATICALLY=1)
add_definitions(-DSTATE_VAR_BYTES=1)
list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

add_subdirectory(src)
