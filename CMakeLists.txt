cmake_minimum_required(VERSION 3.13)

project(vypcomp
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Set build type (Debug|Release): default Relase." FORCE)
endif()

if (NOT MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL Release)
        set(CMAKE_CXX_FLAGS "-O3")
    else()
        set(CMAKE_CXX_FLAGS "-O0")
    endif()
endif()

if (MSVC)
    add_compile_options(/W4)
    # link CRT statically to allow gtest compatibility
    set(variables
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_MINSIZEREL
    )
    foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
    endforeach()
else()
    add_compile_options(-Wall -pedantic -Werror)
endif()

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(tests)
