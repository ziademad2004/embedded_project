# Professional Tic Tac Toe

A professional implementation of the classic Tic Tac Toe game using Qt framework with C++. This project features user accounts, authentication, game history tracking, AI opponents, and more.

## Features

- **User Authentication**: Register, login, and maintain user accounts
- **Game Modes**:
  - Player vs Player: Play against another person
  - Player vs AI: Play against an AI opponent with adjustable difficulty
- **Statistics Tracking**: Track wins, losses, and draws for each user
- **Game History**: Review and replay past games
- **Database Integration**: Save all game data in a local SQLite database
- **Modern UI**: Clean, responsive interface with CSS-like styling

## Requirements

- Qt 5.15+ or Qt 6.0+
- C++17 compatible compiler
- CMake 3.14+
- SQLite support

## Building from Source

### Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Using Qt Creator

1. Open the CMakeLists.txt file with Qt Creator
2. Configure the project for your kit
3. Build and run the application

## Project Structure

```
.
├── include/                # Header files
├── src/                    # Source files
├── resources/              # Resource files
│   └── styles/             # CSS-like style files
├── tests/                  # Test files
├── build/                  # Build directory (generated)
├── .github/                # GitHub related files
├── CMakeLists.txt          # CMake configuration
├── .gitignore              # Git ignore file
└── README.md               # This file
```

## Implementation Details

The project is organized into several key components:

- **User Management**: User class, Authentication class
- **Game Logic**: GameLogic class, AIOpponent class  
- **Data Persistence**: Database class, GameHistory class
- **User Interface**: MainWindow class

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributors

- [Your Name](https://github.com/yourusername) 