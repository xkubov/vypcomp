add_library(Ir
    instructions.cpp
    ../../include/vypcomp/ir/instructions.h
)

add_library(Vypcomp::Ir ALIAS Ir)

target_include_directories(Ir
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
