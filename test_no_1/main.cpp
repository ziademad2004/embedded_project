#include "AdvancedTicTacToe.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application-wide font for better appearance
    QFont appFont("Arial", 10);
    app.setFont(appFont);

    AdvancedTicTacToe window;
    window.show();

    return app.exec();
}
