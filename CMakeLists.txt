cmake_minimum_required(VERSION 2.6)
find_package(Rock)
rock_init(cedalion 0.1)

add_definitions(-Wno-deprecated)
add_definitions(-Wextra -Wno-sign-compare)
add_definitions(-DUSE_LP=1)
add_definitions(-DCOIN_USE_CLP=1)
add_definitions(-DSTATE_VAR_BYTES=1)
rock_activate_cxx11()

add_subdirectory(src)
