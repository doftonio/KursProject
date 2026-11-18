#include "../Headers/ConsoleUI.h"
#include "../Headers/WorkshopReportService.h"
#include "../Headers/InputValidator.h"
#include "../Headers/ComputerDevice.h"
#include "../Headers/HouseholdDevice.h"
#include "../Headers/ElectronicDevice.h"
#include "../Exeptions/AppException.h"
#include <iostream>
#include <format>
#include <limits>
#include <stdexcept>

ConsoleUI::ConsoleUI(Storage& storageRef)
    : storage(storageRef) {}

void ConsoleUI::printMainMenu() const {
    std::cout << "\n========================================\n"
              << "   REPAIR WORKSHOP DEBUG CONSOLE\n"
              << "========================================\n"
              << "Active Orders: " << storage.countActiveOrders() << " / " << Storage::maxActiveOrdersLimit << "\n"
              << "1. List All Orders\n"
              << "2. Filter / Search Orders\n"
              << "3. Create New Order\n"
              << "4. Change Order Status\n"
              << "5. Reserve Spare Part for Order\n"
              << "6. Return Spare Part from Order\n"
              << "7. Add Diagnostic Note to Order\n"
              << "8. Clients Management\n"
              << "9. Warehouse Management\n"
              << "10. Financial Performance Report\n"
              << "11. Backup / Restore Database\n"
              << "12. Masters Staff Management\n"
              << "0. Exit Console Mode\n"
              << "----------------------------------------\n";
}

int ConsoleUI::readInt(const std::string& prompt, int minVal, int maxVal) const {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= minVal && value <= maxVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[Input Error] Enter an integer between " << minVal << " and " << maxVal << ".\n";
    }
}

double ConsoleUI::readDouble(const std::string& prompt, double minVal, double maxVal) const {
    double value = 0.0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= minVal && value <= maxVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[Input Error] Enter a number between " << minVal << " and " << maxVal << ".\n";
    }
}

std::string ConsoleUI::readLine(const std::string& prompt) const {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void ConsoleUI::handleListOrders() const {
    const auto& orders = storage.getOrders();
    if (orders.empty()) {
        std::cout << "No orders registered yet.\n";
        return;
    }
    std::cout << "\n--- ALL ORDERS (" << orders.size() << ") ---\n";
    for (const auto& order : orders) {
        std::cout << order << "\n";
    }
}

void ConsoleUI::handleFilterOrders() const {
    std::cout << "Status filter (-1 = All, 0 = Accepted, 1 = Diagnostics, 2 = WaitingParts, 3 = Completed, 4 = Issued)\n";
    int status = readInt("Select status: ", -1, 4);
    std::string query = readLine("Search query (model, serial, client, phone or empty): ");
    auto results = storage.filterOrders(status, QString::fromStdString(query));
    std::cout << "Found " << results.size() << " matching order(s):\n";
    for (const auto& order : results) {
        std::cout << order << "\n";
    }
}

void ConsoleUI::handleCreateOrder() {
    storage.checkActiveOrdersLimit();

    if (storage.getClients().empty()) {
        std::cout << "No clients found. Please register a client first.\n";
        std::string name = readLine("Client Full Name: ");
        std::string phone = readLine("Client Phone (+375XXXXXXXXX): ");
        std::string note = readLine("Note: ");
        storage.addClient(QString::fromStdString(name), QString::fromStdString(phone), QString::fromStdString(note));
    }

    std::cout << "Available Clients:\n";
    for (const auto& c : storage.getClients()) {
        std::cout << "  " << c << "\n";
    }
    auto clientId = static_cast<unsigned int>(readInt("Enter Client ID: ", 1, 100000));

    std::cout << "Available Masters (0 = Auto-assign least loaded):\n";
    for (const auto& m : storage.getMasters()) {
        std::cout << "  [" << m.getId() << "] Spec: " << m.getSpecialization().toStdString()
                  << " | Active: " << m.getActiveOrdersCount() << "\n";
    }
    auto masterId = static_cast<unsigned int>(readInt("Enter Master ID (0-1000): ", 0, 1000));

    std::cout << "Device Category: 1 = Computer, 2 = Household, 3 = Electronics\n";
    int cat = readInt("Select category: ", 1, 3);
    QString model = QString::fromStdString(readLine("Device Model: "));
    QString serial = QString::fromStdString(readLine("Serial / IMEI: "));
    QString fault = QString::fromStdString(readLine("Fault Description: "));
    double labor = readDouble("Base Labor Cost (BYN): ", 0.0, 10000.0);

    std::unique_ptr<Device> dev;
    if (cat == 1) {
        QString os = QString::fromStdString(readLine("OS Type: "));
        int charger = readInt("Charger included? (1=Yes, 0=No): ", 0, 1);
        dev = std::make_unique<ComputerDevice>(model, serial, fault, os, charger == 1);
    } else if (cat == 2) {
        int watts = readInt("Power (Watts): ", 1, 10000);
        int portable = readInt("Is portable? (1=Yes, 0=No): ", 0, 1);
        dev = std::make_unique<HouseholdDevice>(model, serial, fault, watts, portable == 1);
    } else {
        QString rev = QString::fromStdString(readLine("Board Revision: "));
        int bga = readInt("Requires micro-soldering? (1=Yes, 0=No): ", 0, 1);
        dev = std::make_unique<ElectronicDevice>(model, serial, fault, rev, bga == 1);
    }

    const Order& created = storage.addOrder(clientId, masterId, std::move(dev), labor);
    std::cout << "[OK] Created " << created << "\n";
}

void ConsoleUI::handleUpdateOrderStatus() {
    auto orderId = static_cast<unsigned int>(readInt("Enter Order ID: ", 1, 100000));
    std::cout << "New Status: 0 = Accepted, 1 = Diagnostics, 2 = WaitingParts, 3 = Completed, 4 = Issued\n";
    int statusCode = readInt("Select status: ", 0, 4);
    storage.changeOrderStatus(orderId, static_cast<OrderStatus>(statusCode));
    std::cout << "[OK] Order status updated.\n";
}

void ConsoleUI::handleReservePart() {
    auto orderId = static_cast<unsigned int>(readInt("Enter Order ID: ", 1, 100000));
    auto available = storage.findAvailableParts();
    if (available.empty()) {
        std::cout << "No available spare parts in warehouse.\n";
        return;
    }
    for (const auto& p : available) {
        std::cout << "  " << p << "\n";
    }
    auto partId = static_cast<unsigned int>(readInt("Enter Part ID: ", 1, 100000));
    int count = readInt("Quantity to reserve: ", 1, 200);
    storage.reservePartForOrder(orderId, partId, count);
    std::cout << "[OK] Spare part reserved for order.\n";
}

void ConsoleUI::handleReturnPart() {
    auto orderId = static_cast<unsigned int>(readInt("Enter Order ID: ", 1, 100000));
    auto partId = static_cast<unsigned int>(readInt("Enter Part ID to return: ", 1, 100000));
    int count = readInt("Quantity to return: ", 1, 200);
    storage.returnPartFromOrder(orderId, partId, count);
    std::cout << "[OK] Spare part returned to stock.\n";
}

void ConsoleUI::handleAddDiagnosticNote() {
    auto orderId = static_cast<unsigned int>(readInt("Enter Order ID: ", 1, 100000));
    std::string note = readLine("Diagnostic Note: ");
    storage.appendOrderNote(orderId, QString::fromStdString(note));
    std::cout << "[OK] Diagnostic note added.\n";
}

void ConsoleUI::handleManageClients() {
    std::cout << "1. List / Search Clients\n2. Add New Client\n";
    if (int choice = readInt("Select action: ", 1, 2); choice == 1) {
        std::string query = readLine("Search query (name or phone, empty for all): ");
        auto found = storage.searchClients(QString::fromStdString(query));
        std::cout << "Found " << found.size() << " client(s).\n";
    } else {
        std::string name = readLine("Full Name: ");
        std::string phone = readLine("Phone (+375XXXXXXXXX): ");
        std::string note = readLine("Note: ");
        const Client& added = storage.addClient(QString::fromStdString(name), QString::fromStdString(phone), QString::fromStdString(note));
        std::cout << "[OK] Added client ID #" << added.getId() << "\n";
    }
}

void ConsoleUI::handleManageWarehouse() {
    std::cout << "1. List / Search Parts\n2. Add New Part\n3. Restock Existing Part\n";
    int choice = readInt("Select action: ", 1, 3);
    if (choice == 1) {
        std::string query = readLine("Search query (SKU or name, empty for all): ");
        auto found = storage.searchParts(QString::fromStdString(query));
        std::cout << "Found " << found.size() << " part(s):\n";
        for (const auto& p : found) {
            std::cout << "  " << p << "\n";
        }
    } else if (choice == 2) {
        std::string sku = readLine("SKU: ");
        std::string name = readLine("Part Name: ");
        int qty = readInt("Initial Quantity: ", 0, 10000);
        double price = readDouble("Unit Price (BYN): ", 0.01, 5000.0);
        const SparePart& added = storage.addPart(QString::fromStdString(sku), QString::fromStdString(name), qty, price);
        std::cout << "[OK] Added " << added << "\n";
    } else {
        auto partId = static_cast<unsigned int>(readInt("Enter Part ID: ", 1, 100000));
        int count = readInt("Quantity to add: ", 1, 10000);
        storage.restockPart(partId, count);
        std::cout << "[OK] Stock updated.\n";
    }
}

void ConsoleUI::handleShowReport() const {
    QDate today = QDate::currentDate();
    PerformanceReport rep = WorkshopReportService::generatePerformanceReport(storage, today.addYears(-1), today);
    std::cout << std::format(
        "\n--- PERFORMANCE REPORT ---\n"
        "Total Orders:     {}\n"
        "Active Orders:    {}\n"
        "Completed Orders: {}\n"
        "Labor Revenue:    {:.2f} BYN\n"
        "Parts Revenue:    {:.2f} BYN\n"
        "Total Revenue:    {:.2f} BYN\n",
        rep.totalOrders,
        rep.activeOrders,
        rep.completedOrders,
        rep.totalLaborRevenue,
        rep.totalPartsRevenue,
        rep.totalRevenue
    );
}

void ConsoleUI::handleBackupAndRestore() {
    std::cout << "1. Create Backup\n2. Restore from Backup Folder\n";
    if (int choice = readInt("Select action: ", 1, 2); choice == 1) {
        QString path = storage.backupDatabase();
        std::cout << "[OK] Backup saved to: " << path.toStdString() << "\n";
    } else {
        std::string folder = readLine("Enter backup folder path: ");
        storage.restoreDatabase(QString::fromStdString(folder));
        std::cout << "[OK] Database restored successfully.\n";
    }
}

void ConsoleUI::handleManageMasters() {
    std::cout << "1. List Masters\n2. Add Master\n3. Remove Master\n";
    int choice = readInt("Select action: ", 1, 3);
    if (choice == 1) {
        for (const auto& m : storage.getMasters()) {
            std::cout << "  ID #" << m.getId() << " | Phone: " << m.getPhone().toStdString()
                      << " | Spec: " << m.getSpecialization().toStdString()
                      << " | Active: " << m.getActiveOrdersCount() << "\n";
        }
    } else if (choice == 2) {
        std::string name = readLine("Full Name: ");
        std::string phone = readLine("Phone (+375XXXXXXXXX): ");
        std::cout << "Specialization: 1 = Computer, 2 = Household, 3 = Electronics\n";
        int specChoice = readInt("Select specialization: ", 1, 3);
        QString spec = "Electronics";
        if (specChoice == 1) {
            spec = "Computer";
        } else if (specChoice == 2) {
            spec = "Household";
        }
        const Master& added = storage.addMaster(QString::fromStdString(name), QString::fromStdString(phone), spec);
        std::cout << "[OK] Added Master ID #" << added.getId() << "\n";
    } else {
        auto masterId = static_cast<unsigned int>(readInt("Enter Master ID to remove: ", 1, 100000));
        storage.removeMaster(masterId);
        std::cout << "[OK] Master removed.\n";
    }
}

void ConsoleUI::run() {
    bool running = true;
    while (running) {
        printMainMenu();
        int choice = readInt("Choose option: ", 0, 12);
        try {
            switch (choice) {
                case 1: handleListOrders(); break;
                case 2: handleFilterOrders(); break;
                case 3: handleCreateOrder(); break;
                case 4: handleUpdateOrderStatus(); break;
                case 5: handleReservePart(); break;
                case 6: handleReturnPart(); break;
                case 7: handleAddDiagnosticNote(); break;
                case 8: handleManageClients(); break;
                case 9: handleManageWarehouse(); break;
                case 10: handleShowReport(); break;
                case 11: handleBackupAndRestore(); break;
                case 12: handleManageMasters(); break;
                case 0: running = false; break;
                default: break;
            }
        } catch (const AppException& ex) {
            std::cout << "[Error #" << ex.getCode() << "] " << ex.what() << "\n";
        } catch (const std::out_of_range& ex) {
            std::cout << "[Range Error] " << ex.what() << "\n";
        }
    }
}
