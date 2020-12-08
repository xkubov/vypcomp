add_library(Errors
	errors.cpp
    ../../include/vypcomp/errors/errors.h
)

add_library(Vypcomp::Errors ALIAS Errors)

target_include_directories(Errors
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
