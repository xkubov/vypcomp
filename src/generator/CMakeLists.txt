add_library(Generator
	${PROJECT_SOURCE_DIR}/include/vypcomp/generator/generator.h
    generator.cpp
)
add_library(Vypcomp::Generator ALIAS Generator)


set_target_properties(Generator PROPERTIES CXX_STANDARD 17)

target_include_directories(Generator
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Generator Vypcomp::Ir Vypcomp::Errors)
