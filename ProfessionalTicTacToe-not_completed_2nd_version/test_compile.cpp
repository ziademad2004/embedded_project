#include "include/gamelogic.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    GameLogic gameLogic;
    std::cout << "Test program compiled successfully!" << std::endl;
    return 0;
} 