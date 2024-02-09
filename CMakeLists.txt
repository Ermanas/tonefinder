cmake_minimum_required(VERSION 3.16)
project(tonefinder  VERSION 0.0.1
                    DESCRIPTION "Tone Finder"
                    LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# My Include Directory
include_directories(${PROJECT_SOURCE_DIR})
# Add Test Folder
add_subdirectory(${PROJECT_SOURCE_DIR}/external)
# My Library Directory
add_subdirectory(${PROJECT_SOURCE_DIR}/tonefinder)
# My Examples Directory
add_subdirectory(${PROJECT_SOURCE_DIR}/examples)


# Enable testing for this project
include(CTest)
