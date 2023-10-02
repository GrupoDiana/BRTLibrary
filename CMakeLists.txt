# This is really the one I have tested
cmake_minimum_required(VERSION 3.24)
project(libbrt)

# FIXME: take to separate dependecies.cmake file 
# with include(dependencies.cmake)
# we no longer use libsofa, we use libmysofa instead. 
# include(FetchContent)
# option(BUILD_SHARED_LIBS "By default link libsofa statically" OFF)
# FetchContent_Declare(
#     libsofa
#     GIT_REPOSITORY "https://github.com/lmtanco/API_Cpp.git"
#     GIT_TAG origin/cmake
#     GIT_PROGRESS true
# )
# FetchContent_MakeAvailable(libsofa)

# The library depends on libmysofa
include(FetchContent)
option(BUILD_SHARED_LIBS "By default link libmysofa statically" OFF)
option(BUILD_TESTS "By default don't build libmysofa test files" OFF)
FetchContent_Declare(
    libmysofa
    GIT_REPOSITORY "https://github.com/GrupoDiana/libmysofa.git"
    GIT_TAG origin/FreeFieldDirectivityTF_workaround
    GIT_PROGRESS true
)
FetchContent_MakeAvailable(libmysofa)

#The library also depends on nlohmann_json
# See https://json.nlohmann.me/integration/cmake/
FetchContent_Declare(
    json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Brt library is the main target
add_library(brt INTERFACE)
target_compile_features(brt INTERFACE cxx_std_17)

# FIXME: Dependency of libsofa should be OPTIONAL. 
target_include_directories(brt 
    INTERFACE  include
    INTERFACE  libsofa/src
    )
target_link_libraries(brt 
    INTERFACE libmysofa 
    INTERFACE nlohmann_json::nlohmann_json
    )

# If we are in the root, define additional targets 
# See https://www.foonathan.net/2022/06/cmake-fetchcontent/ 
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BRT_BUILD_TESTS  "whether or not build tests, sofaexamples, etc should be built" ON)
endif()

if (BRT_BUILD_TESTS)
    # This is just a test to test linking of dependencies. 
    add_executable(testBRT test/test.cpp)
    target_link_libraries(testBRT brt)
endif()



