#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include "Storage.h"
#include <string>

class ConsoleUI {
private:
    Storage& storage;

    void printMainMenu() const;
    template <typename T>
    T readNumber(const std::string& prompt, T minVal, T maxVal, const std::string& typeLabel) const;
    int readInt(const std::string& prompt, int minVal, int maxVal) const;
    unsigned int readId(const std::string& prompt, unsigned int minVal = 1, unsigned int maxVal = 100000) const;
    double readDouble(const std::string& prompt, double minVal, double maxVal) const;
    std::string readLine(const std::string& prompt) const;
    QString readQString(const std::string& prompt) const;

    template <typename Range>
    void printItems(const std::string& header, const std::string& emptyMessage, const Range& items) const;

    void registerClientInteractive();
    std::unique_ptr<Device> buildDeviceInteractive() const;

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
    void handleExportAcceptanceReceipt() const;
    void handleListArchivedOrders() const;

public:
    explicit ConsoleUI(Storage& storageRef);
    void run();
};

#endif
