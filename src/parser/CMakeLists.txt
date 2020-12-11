find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(BisonParser
    parser.yy
    ${CMAKE_CURRENT_BINARY_DIR}/bison_parser.tab.cpp
)
FLEX_TARGET(FlexScanner
    scanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/flex_lexer.yy.cpp
)

ADD_FLEX_BISON_DEPENDENCY(FlexScanner BisonParser)

add_library(Parser
    parser.cpp
    symbol_table.cpp
    indexdriver.cpp
    ../../include/vypcomp/parser/parser.h
    ../../include/vypcomp/parser/scanner.h
    ${FLEX_FlexScanner_OUTPUTS}
    ${BISON_BisonParser_OUTPUTS}
)

target_link_libraries(Parser
    Vypcomp::Errors
    Vypcomp::Ir
)

add_library(Vypcomp::Parser ALIAS Parser)

if (MSVC)
    target_compile_definitions(Parser PRIVATE YY_NO_UNISTD_H)
endif()

target_include_directories(Parser
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FLEX_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
)
