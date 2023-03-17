cmake_minimum_required(VERSION 3.10)
project(cedalion
        VERSION 0.1
        DESCRIPTION "Fast-Downward Cedalion planner"
)
find_package(Rock)
rock_init()

add_definitions(-Wno-deprecated)
add_definitions(-Wextra -Wno-sign-compare)
add_definitions(-DUSE_LP=1)
add_definitions(-DCOIN_USE_CLP=1)
add_definitions(-DSTATE_VAR_BYTES=1)
rock_activate_cxx11()

add_subdirectory(src)
