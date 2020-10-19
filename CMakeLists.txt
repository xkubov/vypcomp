
cmake_minimum_required(VERSION 3.13)
project(vypcomp
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

if(APPLE)
	set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
elseif(UNIX)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(tests)
