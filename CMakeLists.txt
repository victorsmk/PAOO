# CMake version
cmake_minimum_required(VERSION 3.0)

# Project name
project(l1)

# Define the search path for header files
include_directories(inc)

# Specify the required C++ standard
if(CMAKE_VERSION GREATER "3.1.2")
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
else()
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Define the executable
add_executable(${PROJECT_NAME}
               inc/l1/tracker.hpp
               inc/l1/buffer.hpp
               inc/l1/bad_examples.hpp
               inc/l1/demos.hpp
               src/main.cpp
               src/tracker.cpp
               src/buffer.cpp
               src/demos.cpp)