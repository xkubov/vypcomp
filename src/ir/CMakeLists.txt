add_library(Ir
    instructions.cpp
    expression.cpp
    ../../include/vypcomp/ir/ir.h
    ../../include/vypcomp/ir/instructions.h
    ../../include/vypcomp/ir/expression.h
)

add_library(Vypcomp::Ir ALIAS Ir)

target_include_directories(Ir
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
