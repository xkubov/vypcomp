add_executable(vypcomp-tests
    scanner_tests.cpp
    parser_tests.cpp
)

target_link_libraries(vypcomp-tests
    Vypcomp::Parser
    gtest gtest_main
)

install(TARGETS vypcomp-tests
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
