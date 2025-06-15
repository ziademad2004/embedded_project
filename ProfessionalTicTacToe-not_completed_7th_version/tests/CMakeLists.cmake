# Test executable
add_executable(TicTacToeTests
    test_gamelogic.cpp
    test_authentication.cpp
    test_user.cpp
    test_aiopponent.cpp
    test_database.cpp
    test_integration.cpp
)

target_include_directories(TicTacToeTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(TicTacToeTests
    TicTacToeLib
    gtest_main
    gmock_main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(TicTacToeTests)
