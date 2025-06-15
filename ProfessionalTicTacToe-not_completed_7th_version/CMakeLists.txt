cmake_minimum_required(VERSION 3.10)

project(ProfessionalTicTacToe VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Enable testing
enable_testing()

# Try to find Qt6 first, fall back to Qt5 if not found
find_package(Qt6 COMPONENTS Core Gui Widgets Sql QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets Sql REQUIRED)
    message(STATUS "Using Qt5")
else()
    message(STATUS "Using Qt6")
endif()

# Download and configure Google Test using FetchContent
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Keep your existing sources and headers exactly as they are
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/gamelogic.cpp
    src/gamehistory.cpp
    src/authentication.cpp
    src/user.cpp
    src/database.cpp
    src/aiopponent.cpp
)

set(HEADERS
    include/mainwindow.h
    include/gamelogic.h
    include/gamehistory.h
    include/authentication.h
    include/user.h
    include/database.h
    include/aiopponent.h
)

set(RESOURCES
    resources/resources.qrc
)

# Create a library for testing (excluding main.cpp)
set(LIB_SOURCES
    src/mainwindow.cpp
    src/gamelogic.cpp
    src/gamehistory.cpp
    src/authentication.cpp
    src/user.cpp
    src/database.cpp
    src/aiopponent.cpp
)

add_library(TicTacToeLib STATIC ${LIB_SOURCES} ${HEADERS})
target_include_directories(TicTacToeLib PUBLIC include)

# Link Qt libraries to the library (ADDED SQL COMPONENT)
if (Qt6_FOUND)
    target_link_libraries(TicTacToeLib PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
    )
else()
    target_link_libraries(TicTacToeLib PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
    )
endif()

# Main executable (unchanged)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link against Qt5 or Qt6 depending on what was found (ADDED SQL COMPONENT)
if (Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
    )
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# OPTION 1: Use separate tests directory (RECOMMENDED)
# Uncomment this line if you create a tests/ directory with its own CMakeLists.txt
# add_subdirectory(tests)

# OPTION 2: Keep tests in main CMakeLists.txt (CURRENT APPROACH)
# Test executable
add_executable(TicTacToeTests
    tests/test_gamelogic.cpp
    tests/test_authentication.cpp
    tests/test_user.cpp
    tests/test_aiopponent.cpp
    tests/test_database.cpp
    tests/test_integration.cpp
)

target_include_directories(TicTacToeTests PRIVATE include)

target_link_libraries(TicTacToeTests
    TicTacToeLib
    gtest_main
    gmock_main
)

# Discover and register tests
include(GoogleTest)
gtest_discover_tests(TicTacToeTests)

# Add custom test target for easy running
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS TicTacToeTests
    COMMENT "Running all tests"
)

# Define install directories if not already defined
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Print configuration summary
message(STATUS "=== Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if (Qt6_FOUND)
    message(STATUS "Qt Version: 6.x with SQL support")
else()
    message(STATUS "Qt Version: 5.x with SQL support")
endif()
message(STATUS "Google Test: Enabled")
message(STATUS "Testing: Enabled")
message(STATUS "=============================")

