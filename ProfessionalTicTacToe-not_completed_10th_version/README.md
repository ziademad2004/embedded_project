# Professional Tic-Tac-Toe

![Build Status](https://github.com/yourusername/ProfessionalTicTacToe/workflows/Tic-Tac-Toe%20CI/badge.svg)

An advanced Tic-Tac-Toe game with user authentication, AI opponent, and game history tracking.

## Features

- Interactive Tic-Tac-Toe game with a user-friendly GUI
- User authentication system for player profiles
- AI opponent with multiple difficulty levels using minimax algorithm
- Game history tracking and statistics
- Leaderboard system

## Technologies Used

- C++ with Qt framework
- QTest for testing
- CMake build system
- GitHub Actions for CI/CD

## Building the Project

### Prerequisites

- CMake 3.10 or higher
- Qt 5.15.2 or Qt 6.x
- C++17 compatible compiler

### Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running Tests

```bash
cd build
ctest -V
```

## Project Structure

- `include/` - Header files
- `src/` - Source files
- `resources/` - GUI resources
- `tests/` - Test files

## Test Coverage

The project includes comprehensive tests for:
- Game logic
- AI opponent
- Authentication system
- Database operations
- User management
- Integration tests

## Contributors

- Your Team Members

## License

This project is part of an academic assignment for Embedded Systems course.
