#include <QApplication>
#include <QMessageBox>
#include "../QTInterfase/headers/MainWindow.h"
#include "../QTInterfase/headers/ThemeManager.h"
#include "../Headers/ConsoleUI.h"
#include "../Exeptions/AppException.h"
#include <iostream>
#include <cstring>
#include <stdexcept>

int main(int argc, char* argv[]) {
    bool consoleMode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--console") == 0 || std::strcmp(argv[i], "-c") == 0) {
            consoleMode = true;
        }
    }






    if (consoleMode) {
        try {
            Storage storage;
            ConsoleUI console(storage);
            console.run();
        } catch (const AppException& ex) {
            std::cerr << "[Fatal Error #" << ex.getCode() << "] " << ex.what() << "\n";
            return 1;
        }
        return 0;
    }

    QApplication app(argc, argv);
    try {
        ThemeManager themeManager;
        themeManager.loadAndApply();
        MainWindow window(themeManager);
        window.show();
        return QApplication::exec();
    } catch (const AppException& ex) {
        QMessageBox::critical(
            nullptr,
            "Storage Initialization Error",
            QString("Error #%1:\n%2").arg(ex.getCode()).arg(QString::fromStdString(ex.what()))
        );
        return 1;
    } catch (const std::out_of_range& ex) {
        QMessageBox::critical(
            nullptr,
            "Range Error",
            QString::fromStdString(ex.what())
        );
        return 1;
    }
}
