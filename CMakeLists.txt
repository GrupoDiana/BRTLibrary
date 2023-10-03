# This is really the one I have tested
cmake_minimum_required(VERSION 3.24)
project(libbrt)

# The library depends on a fork of libmysofa, install from
# https://github.com/GrupoDiana/libmysofa.git 
# branch origin/FreeFieldDirectivityTF_workaround 
find_package(mysofa CONFIG 1.3 REQUIRED)

#The library also depends on nlohmann_json
find_package(nlohmann_json 3.11.2 REQUIRED)

# Brt library is the main target
add_library(brt INTERFACE)
target_compile_features(brt INTERFACE cxx_std_17)

 target_include_directories(brt 
     INTERFACE  include
#     INTERFACE  "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/xxxx>"
#     INTERFACE nlohmann_json::nlohmann_json
     )
# target_link_libraries(brt 
#     INTERFACE mysofa-static  
#     INTERFACE nlohmann_json::nlohmann_json
#     )

# If we are in the root, define additional targets 
# See https://www.foonathan.net/2022/06/cmake-fetchcontent/ 
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BRT_BUILD_TESTS  "whether or not build tests, sofaexamples, etc should be built" ON)
#    option(BUILD_TESTING "" ON)
endif()

if (BRT_BUILD_TESTS)
    # This is just a test to test linking of dependencies. FIXME: actually invoke test when building.  
    # include(CTest)
    add_executable(testBRT test/test.cpp)
    target_link_libraries(testBRT 
                            PUBLIC brt 
                            PRIVATE mysofa::mysofa-static 
                            PRIVATE nlohmann_json::nlohmann_json)
    # enable_testing()
    # add_test(NAME TestBRT COMMAND testBRT WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test )
endif()



