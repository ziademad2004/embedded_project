cmake_minimum_required(VERSION 3.10)

project(ProfessionalTicTacToe VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Try to find Qt6 first, fall back to Qt5 if not found
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
    message(STATUS "Using Qt5")
else()
    message(STATUS "Using Qt6")
endif()

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

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link against Qt5 or Qt6 depending on what was found
if (Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    )
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Define install directories if not already defined
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 