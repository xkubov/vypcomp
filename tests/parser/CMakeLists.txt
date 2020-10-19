add_executable(tests-parser
    parser_tests.cpp
)
target_link_libraries(tests-parser
    Vypcomp::Parser
    gtest gtest_main
)
install(TARGETS tests-parser
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
