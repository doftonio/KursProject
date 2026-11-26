#include "../Headers/ConsoleUI.h"
#include "../Headers/WorkshopSearchService.h"
#include "../Headers/WorkshopReportService.h"
#include "../Headers/AccountingLedger.h"
#include "../Headers/ComputerDevice.h"
#include "../Headers/HouseholdDevice.h"
#include "../Headers/ElectronicDevice.h"
#include "../Exeptions/AppException.h"
#include <iostream>
#include <fstream>
#include <format>
#include <limits>
#include <stdexcept>
#include <array>

ConsoleUI::ConsoleUI(Storage& storageRef)
    : storage(storageRef) {}

void ConsoleUI::printMainMenu() const {
    std::cout << std::format(
        "\n========================================\n"
        "   REPAIR WORKSHOP DEBUG CONSOLE\n"
        "========================================\n"
        "Active Orders: {} / {} | Archived: {}\n"
        "1. List Current Orders (In Progress / Ready)\n"
        "2. Filter / Search Orders\n"
        "3. Create Order\n"
        "4. Change Order Status\n"
        "5. Reserve Spare Part for Order\n"
        "6. Return Spare Part from Order\n"
        "7. Add Diagnostic Note to Order\n"
        "8. Clients Management\n"
        "9. Warehouse Management\n"
        "10. Accounting & Payroll Report\n"
        "11. Backup / Restore Database\n"
        "12. Masters Staff Management\n"
        "13. Export Acceptance Receipt (HTML)\n"
        "14. List Archived Orders (Issued / Read-Only)\n"
        "0. Exit Console Mode\n"
        "----------------------------------------\n",
        storage.countActiveOrders(),
        Storage::maxActiveOrdersLimit,
        storage.countArchivedOrders()
    );
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
        std::cout << std::format("[Input Error] Enter an integer between {} and {}.\n", minVal, maxVal);
    }
}

unsigned int ConsoleUI::readId(const std::string& prompt, unsigned int minVal, unsigned int maxVal) const {
    return static_cast<unsigned int>(readInt(prompt, static_cast<int>(minVal), static_cast<int>(maxVal)));
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
        std::cout << std::format("[Input Error] Enter a number between {:.2f} and {:.2f}.\n", minVal, maxVal);
    }
}

std::string ConsoleUI::readLine(const std::string& prompt) const {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

QString ConsoleUI::readQString(const std::string& prompt) const {
    return QString::fromStdString(readLine(prompt));
}

template <typename Range>
void ConsoleUI::printItems(const std::string& header, const std::string& emptyMessage, const Range& items) const {
    if (items.empty()) {
        std::cout << emptyMessage << "\n";
        return;
    }
    std::cout << std::format("\n--- {} ({}) ---\n", header, items.size());
    for (const auto& item : items) {
        std::cout << "  " << item << "\n";
    }
}

void ConsoleUI::registerClientInteractive() {
    QString name = readQString("Client Full Name: ");
    QString phone = readQString("Client Phone (+375XXXXXXXXX): ");
    QString note = readQString("Note: ");
    const Client& added = storage.addClient(name, phone, note);
    std::cout << "[OK] Registered client ID #" << added.getId() << "\n";
}

std::unique_ptr<Device> ConsoleUI::buildDeviceInteractive() const {
    std::cout << "Device Category: 1 = Computer, 2 = Household, 3 = Electronics\n";
    int categoryChoice = readInt("Select category: ", 1, 3);
    QString model = readQString("Device Model: ");
    QString serial = readQString("Serial / IMEI: ");
    QString fault = readQString("Fault Description: ");

    if (categoryChoice == 1) {
        QString os = readQString("OS Type: ");
        bool hasCharger = readInt("Charger included? (1=Yes, 0=No): ", 0, 1) == 1;
        return std::make_unique<ComputerDevice>(model, serial, fault, os, hasCharger);
    }
    if (categoryChoice == 2) {
        int watts = readInt("Power (Watts): ", 1, 10000);
        bool isPortable = readInt("Is portable? (1=Yes, 0=No): ", 0, 1) == 1;
        return std::make_unique<HouseholdDevice>(model, serial, fault, watts, isPortable);
    }
    QString boardRev = readQString("Board Revision: ");
    bool microSoldering = readInt("Requires micro-soldering? (1=Yes, 0=No): ", 0, 1) == 1;
    return std::make_unique<ElectronicDevice>(model, serial, fault, boardRev, microSoldering);
}

void ConsoleUI::handleListOrders() const {
    printItems("CURRENT ORDERS", "No current orders registered.", WorkshopSearchService::filterCurrentOrders(storage, -1, ""));
}

void ConsoleUI::handleFilterOrders() const {
    std::cout << "Status filter (-1 = All, 0 = Accepted, 1 = Diagnostics, 2 = WaitingParts, 3 = Completed, 4 = Issued/Archived)\n";
    int status = readInt("Select status: ", -1, 4);
    QString query = readQString("Search query (model, serial, client, phone or empty): ");
    printItems("MATCHING ORDERS", "No matching orders found.", WorkshopSearchService::filterOrders(storage, status, query));
}

void ConsoleUI::handleCreateOrder() {
    storage.checkActiveOrdersLimit();

    if (storage.getClients().empty()) {
        std::cout << "No clients found. Please register a client first.\n";
        registerClientInteractive();
    } else {
        std::cout << "Client Option: 1 = Select existing client, 2 = Register client\n";
        if (readInt("Choose option: ", 1, 2) == 2) {
            registerClientInteractive();
        }
    }

    printItems("AVAILABLE CLIENTS", "No clients available.", storage.getClients());
    unsigned int clientId = readId("Enter Client ID: ");

    std::cout << "Available Masters (0 = Auto-assign least loaded):\n";
    for (const auto& m : storage.getMasters()) {
        std::cout << std::format("  [{}] Spec: {} | Active: {}\n", m.getId(), m.getSpecialization().toStdString(), m.getActiveOrdersCount());
    }
    unsigned int masterId = readId("Enter Master ID (0-1000): ", 0, 1000);

    std::unique_ptr<Device> device = buildDeviceInteractive();
    double labor = readDouble("Base Labor Cost (BYN): ", 0.0, 10000.0);

    const Order& created = storage.addOrder(clientId, masterId, std::move(device), labor);
    std::cout << "[OK] Created " << created << "\n";
}

void ConsoleUI::handleUpdateOrderStatus() {
    unsigned int orderId = readId("Enter Order ID: ");
    std::cout << "Status: 0 = Accepted, 1 = Diagnostics, 2 = WaitingParts, 3 = Completed, 4 = Issued (Moves to Archive)\n";
    int statusCode = readInt("Select status: ", 0, 4);
    storage.changeOrderStatus(orderId, static_cast<OrderStatus>(statusCode));
    std::cout << "[OK] Order status updated.\n";
}

void ConsoleUI::handleReservePart() {
    unsigned int orderId = readId("Enter Order ID: ");
    auto available = WorkshopSearchService::findAvailableParts(storage);
    if (available.empty()) {
        std::cout << "No available spare parts in warehouse.\n";
        return;
    }
    printItems("AVAILABLE SPARE PARTS", "No spare parts available.", available);
    unsigned int partId = readId("Enter Part ID: ");
    int count = readInt("Quantity to reserve: ", 1, 200);
    storage.reservePartForOrder(orderId, partId, count);
    std::cout << "[OK] Spare part reserved for order.\n";
}

void ConsoleUI::handleReturnPart() {
    unsigned int orderId = readId("Enter Order ID: ");
    unsigned int partId = readId("Enter Part ID to return: ");
    int count = readInt("Quantity to return: ", 1, 200);
    storage.returnPartFromOrder(orderId, partId, count);
    std::cout << "[OK] Spare part returned to stock.\n";
}

void ConsoleUI::handleAddDiagnosticNote() {
    unsigned int orderId = readId("Enter Order ID: ");
    QString note = readQString("Diagnostic Note: ");
    storage.appendOrderNote(orderId, note);
    std::cout << "[OK] Diagnostic note added.\n";
}

void ConsoleUI::handleManageClients() {
    std::cout << "1. List / Search Clients\n2. Add Client\n";
    if (readInt("Select action: ", 1, 2) == 1) {
        QString query = readQString("Search query (name or phone, empty for all): ");
        auto found = WorkshopSearchService::searchClients(storage, query);
        std::cout << std::format("Found {} client(s).\n", found.size());
    } else {
        registerClientInteractive();
    }
}

void ConsoleUI::handleManageWarehouse() {
    std::cout << "1. List / Search Parts\n2. Add Part\n3. Restock Existing Part\n";
    int choice = readInt("Select action: ", 1, 3);
    if (choice == 1) {
        QString query = readQString("Search query (SKU or name, empty for all): ");
        printItems("WAREHOUSE PARTS", "No matching parts found.", WorkshopSearchService::searchParts(storage, query));
    } else if (choice == 2) {
        QString sku = readQString("SKU: ");
        QString name = readQString("Part Name: ");
        int qty = readInt("Initial Quantity: ", 0, 10000);
        double price = readDouble("Unit Price (BYN): ", 0.01, 5000.0);
        const SparePart& added = storage.addPart(sku, name, qty, price);
        std::cout << "[OK] Added " << added << "\n";
    } else {
        unsigned int partId = readId("Enter Part ID: ");
        int count = readInt("Quantity to add: ", 1, 10000);
        storage.restockPart(partId, count);
        std::cout << "[OK] Stock updated.\n";
    }
}

void ConsoleUI::handleShowReport() const {
    AccountingLedger ledger;
    AccountingSummary sum = ledger.calculateSummary(storage);
    std::cout << std::format(
        "\n--- ACCOUNTING & PAYROLL SUMMARY ---\n"
        "Total Orders:     {} (Archived: {})\n"
        "Gross Revenue:    {:.2f} BYN\n"
        "Parts Expense:    {:.2f} BYN\n"
        "Masters Payroll:  {:.2f} BYN\n"
        "Staff Payroll:    {:.2f} BYN\n"
        "Total Expenses:   {:.2f} BYN\n"
        "Net Income:       {:.2f} BYN\n",
        sum.totalOrdersCount,
        sum.issuedOrdersCount,
        sum.grossRevenue,
        sum.partsExpense,
        sum.mastersPayroll,
        sum.staffPayroll,
        sum.totalExpenses,
        sum.netIncome
    );
}

void ConsoleUI::handleBackupAndRestore() {
    std::cout << "1. Create Backup\n2. Restore from Backup Folder\n";
    if (readInt("Select action: ", 1, 2) == 1) {
        QString path = storage.backupDatabase();
        std::cout << "[OK] Backup saved to: " << path.toStdString() << "\n";
    } else {
        QString folder = readQString("Enter backup folder path: ");
        storage.restoreDatabase(folder);
        std::cout << "[OK] Database restored successfully.\n";
    }
}

void ConsoleUI::handleManageMasters() {
    std::cout << "1. List Masters\n2. Add Master\n3. Remove Master\n";
    int choice = readInt("Select action: ", 1, 3);
    if (choice == 1) {
        for (const auto& m : storage.getMasters()) {
            std::cout << std::format("  ID #{} | Phone: {} | Spec: {} | Active: {}\n",
                m.getId(), m.getPhone().toStdString(), m.getSpecialization().toStdString(), m.getActiveOrdersCount());
        }
    } else if (choice == 2) {
        constexpr std::array<const char*, 3> specs = {"Computer", "Household", "Electronics"};
        QString name = readQString("Full Name: ");
        QString phone = readQString("Phone (+375XXXXXXXXX): ");
        std::cout << "Specialization: 1 = Computer, 2 = Household, 3 = Electronics\n";
        int specIdx = readInt("Select specialization: ", 1, 3) - 1;
        const Master& added = storage.addMaster(name, phone, specs[static_cast<std::size_t>(specIdx)]);
        std::cout << "[OK] Added Master ID #" << added.getId() << "\n";
    } else {
        unsigned int masterId = readId("Enter Master ID to remove: ");
        storage.removeMaster(masterId);
        std::cout << "[OK] Master removed.\n";
    }
}

void ConsoleUI::handleExportAcceptanceReceipt() const {
    unsigned int orderId = readId("Enter Order ID: ");
    QString html = WorkshopReportService::generateAcceptanceReceiptHtml(storage, orderId);
    std::string fileName = std::format("Order_{}_Receipt.html", orderId);
    if (std::ofstream out(fileName, std::ios::binary); out.is_open()) {
        std::string utf8 = html.toStdString();
        out.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
        std::cout << "[OK] Acceptance receipt exported to: " << fileName << "\n";
    } else {
        std::cout << "[File Error] Could not write file: " << fileName << "\n";
    }
}

void ConsoleUI::handleListArchivedOrders() const {
    printItems("ARCHIVED ORDERS (ISSUED / READ-ONLY)", "Archive is empty.", WorkshopSearchService::filterArchivedOrders(storage, ""));
}

void ConsoleUI::run() {
    bool running = true;
    while (running) {
        printMainMenu();
        int choice = readInt("Choose option: ", 0, 14);
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
                case 13: handleExportAcceptanceReceipt(); break;
                case 14: handleListArchivedOrders(); break;
                case 0: running = false; break;
                default: break;
            }
        } catch (const AppException& ex) {
            std::cout << std::format("[Error #{}] {}\n", ex.getCode(), ex.what());
        } catch (const std::out_of_range& ex) {
            std::cout << std::format("[Range Error] {}\n", ex.what());
        }
    }
}
