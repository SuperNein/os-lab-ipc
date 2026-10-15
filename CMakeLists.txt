cmake_minimum_required(VERSION 3.16)
project(lab CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# default: Debug
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type: Debug | Release | RelWithDebInfo" FORCE)
endif()

add_library(compiler_warnings INTERFACE)
if(MSVC)
    target_compile_options(compiler_warnings INTERFACE /W4 /WX)
else()
    target_compile_options(compiler_warnings INTERFACE -Wall -Wextra -Wpedantic -Werror)
endif()

# child
add_executable(child 
    src/child.cpp 
    src/string_utils.cpp
)

target_include_directories(child PRIVATE include)
target_link_libraries(child PRIVATE compiler_warnings)

# parent 
set(PARENT_SOURCES 
    src/parent.cpp
)

if(UNIX)
    list(APPEND PARENT_SOURCES src/posix/process_posix.cpp)
# else()
#     list(APPEND PARENT_SOURCES src/win/process_win.cpp)
endif()

add_executable(parent ${PARENT_SOURCES})
target_include_directories(parent PRIVATE include)
target_link_libraries(parent PRIVATE compiler_warnings)

target_compile_definitions(parent PRIVATE 
    CHILD_PATH="$<TARGET_FILE:child>"
)
