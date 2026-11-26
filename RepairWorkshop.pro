QT += core gui widgets printsupport

CONFIG += c++20

TARGET = RepairWorkshop
TEMPLATE = app

INCLUDEPATH += Headers Exeptions QTInterfase/headers

HEADERS += \
    Exeptions/AppException.h \
    Exeptions/InvalidInputDataException.h \
    Exeptions/DuplicateEntityException.h \
    Exeptions/EntityNotFoundException.h \
    Exeptions/CapacityExceededException.h \
    Exeptions/InsufficientStockException.h \
    Exeptions/FileStorageException.h \
    Headers/EnumUtils.h \
    Headers/Person.h \
    Headers/Client.h \
    Headers/Master.h \
    Headers/Employee.h \
    Headers/StaffEmployee.h \
    Headers/MasterEmployee.h \
    Headers/AccountingLedger.h \
    Headers/Device.h \
    Headers/ComputerDevice.h \
    Headers/HouseholdDevice.h \
    Headers/ElectronicDevice.h \
    Headers/SparePart.h \
    Headers/OrderItemPart.h \
    Headers/Order.h \
    Headers/Repository.h \
    Headers/InputValidator.h \
    Headers/JsonFileHelper.h \
    Headers/StoragePersistence.h \
    Headers/Storage.h \
    Headers/WorkshopSearchService.h \
    Headers/WorkshopReportService.h \
    Headers/ConsoleUI.h \
    QTInterfase/headers/UiHelpers.h \
    QTInterfase/headers/ThemeManager.h \
    QTInterfase/headers/SettingsDialog.h \
    QTInterfase/headers/WorkersTab.h \
    QTInterfase/headers/ClientsTab.h \
    QTInterfase/headers/WarehouseTab.h \
    QTInterfase/headers/AccountingTab.h \
    QTInterfase/headers/QuickClientDialog.h \
    QTInterfase/headers/OrderDialog.h \
    QTInterfase/headers/MainWindow.h

SOURCES += \
    Exeptions/Exceptions.cpp \
    Sourse/Person.cpp \
    Sourse/Client.cpp \
    Sourse/Master.cpp \
    Sourse/Employee.cpp \
    Sourse/StaffEmployee.cpp \
    Sourse/MasterEmployee.cpp \
    Sourse/AccountingLedger.cpp \
    Sourse/Device.cpp \
    Sourse/ComputerDevice.cpp \
    Sourse/HouseholdDevice.cpp \
    Sourse/ElectronicDevice.cpp \
    Sourse/SparePart.cpp \
    Sourse/OrderItemPart.cpp \
    Sourse/Order.cpp \
    Sourse/InputValidator.cpp \
    Sourse/JsonFileHelper.cpp \
    Sourse/StoragePersistence.cpp \
    Sourse/Storage.cpp \
    Sourse/WorkshopSearchService.cpp \
    Sourse/WorkshopReportService.cpp \
    Sourse/ConsoleUI.cpp \
    Sourse/main.cpp \
    QTInterfase/Sourse/ThemeManager.cpp \
    QTInterfase/Sourse/SettingsDialog.cpp \
    QTInterfase/Sourse/WorkersTab.cpp \
    QTInterfase/Sourse/ClientsTab.cpp \
    QTInterfase/Sourse/WarehouseTab.cpp \
    QTInterfase/Sourse/AccountingTab.cpp \
    QTInterfase/Sourse/QuickClientDialog.cpp \
    QTInterfase/Sourse/OrderDialog.cpp \
    QTInterfase/Sourse/MainWindow.cpp
