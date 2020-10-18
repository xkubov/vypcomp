add_executable(tests-scanner
    parser_tests.cpp
)
target_link_libraries(tests-scanner
    Vypcomp::Parser
    GTest::GTest GTest::Main
)
install(TARGETS tests-scanner
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
