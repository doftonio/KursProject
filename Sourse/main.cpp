#include <QApplication>
#include "../QTInterfase/headers/MainWindow.h"
#include "../QTInterfase/headers/ThemeManager.h"
#include "../Headers/ConsoleUI.h"
#include "../Exeptions/AppException.h"
#include <iostream>
#include <cstring>

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
    ThemeManager themeManager;
    themeManager.loadAndApply(app);
    MainWindow window(themeManager);
    window.show();
    return app.exec();
}
