add_library(Ir
    instructions.cpp
)

add_library(Vypcomp::Ir ALIAS Ir)

target_include_directories(Ir
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
