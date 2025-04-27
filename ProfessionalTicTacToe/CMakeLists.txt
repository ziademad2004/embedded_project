cmake_minimum_required(VERSION 3.14)
project(ProfessionalTicTacToe LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Sql)

# Project resources
set(PROJECT_RESOURCES
    resources/resources.qrc
)

# Source files
set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/gamelogic.cpp
    src/user.cpp
    src/authentication.cpp
    src/database.cpp
    src/gamehistory.cpp
    src/aiopponent.cpp
)

# Header files
set(PROJECT_HEADERS
    include/mainwindow.h
    include/gamelogic.h
    include/user.h
    include/authentication.h
    include/database.h
    include/gamehistory.h
    include/aiopponent.h
)

# Create executable
add_executable(ProfessionalTicTacToe
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
)

# Include directories
target_include_directories(ProfessionalTicTacToe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link Qt libraries
target_link_libraries(ProfessionalTicTacToe PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

# Install rules
install(TARGETS ProfessionalTicTacToe
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# If Windows system, copy required Qt DLLs
if(WIN32)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
    include(InstallRequiredSystemLibraries)
    
    set(INSTALL_UCRT_LIBRARIES TRUE)
    include(DeployQt)
    qt_deploy_runtime_dependencies(
        EXECUTABLE ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/ProfessionalTicTacToe.exe
    )
endif() 