#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include "Storage.h"
#include <string>

class ConsoleUI {
private:
    Storage& storage;

    void printMainMenu() const;
    int readInt(const std::string& prompt, int minVal, int maxVal) const;
    double readDouble(const std::string& prompt, double minVal, double maxVal) const;
    std::string readLine(const std::string& prompt) const;

    void handleListOrders() const;
    void handleFilterOrders() const;
    void handleCreateOrder();
    void handleUpdateOrderStatus();
    void handleReservePart();
    void handleReturnPart();
    void handleAddDiagnosticNote();
    void handleManageClients();
    void handleManageWarehouse();
    void handleShowReport() const;
    void handleBackupAndRestore();
    void handleManageMasters();

public:
    explicit ConsoleUI(Storage& storageRef);
    void run();
};

#endif
