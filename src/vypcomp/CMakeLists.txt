add_executable(vypcomp
    vypcomp.cpp
)

set_property(
    TARGET vypcomp
    PROPERTY CXX_STANDARD 17
)
target_link_libraries(vypcomp
    Vypcomp::Parser
    Vypcomp::Generator
)
target_include_directories(vypcomp
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
install(TARGETS vypcomp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
