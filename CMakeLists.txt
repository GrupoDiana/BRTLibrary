# This is really the one I have tested
cmake_minimum_required(VERSION 3.24)
project(libbrt)

# The library depends on a fork of libmysofa, install from
# https://github.com/GrupoDiana/libmysofa.git 
# branch origin/FreeFieldDirectivityTF_workaround 
find_package(mysofa CONFIG 1.3 REQUIRED)

#The library also depends on nlohmann_json, see
# https://github.com/nlohmann/json#integration 
find_package(nlohmann_json 3.11.2 REQUIRED)

# Brt library is the main target
add_library(brt INTERFACE)
# the library requires a minimum version of C++-17 
target_compile_features(brt INTERFACE cxx_std_17)

 target_include_directories(brt 
     INTERFACE  include
     INTERFACE  mysofa::mysofa-static
     INTERFACE nlohmann_json::nlohmann_json
     )
 target_link_libraries(brt 
     INTERFACE mysofa::mysofa-static  
     INTERFACE nlohmann_json::nlohmann_json
     )

# If we are just building this library (not using it in another cmake project),
# then by default build also tests.  
# See https://www.foonathan.net/2022/06/cmake-fetchcontent/ to understand the "if" 
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BRT_BUILD_TESTS  "whether or not build tests, sofaexamples, etc should be built" ON)
endif()

# This shows how we could add more tests.
# This creates a test target, so "the user can run the tests by invoking 
# the “test” target of Makefile, or the RUN_TESTS target of Visual Studio or Xcode." 
# See https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html  
if (BRT_BUILD_TESTS)
    include(CTest)
    enable_testing()
    
    add_executable(testBRT test/test.cpp)
    target_link_libraries(testBRT PUBLIC brt) 
    add_test(NAME TestBRT COMMAND testBRT WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test )
endif()



