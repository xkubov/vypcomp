add_executable(tests-scanner
    scanner_tests.cpp
)
target_link_libraries(tests-scanner
    Vypcomp::Parser
    gtest gtest_main
)
install(TARGETS tests-scanner
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
