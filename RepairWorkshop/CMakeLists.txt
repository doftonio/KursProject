cmake_minimum_required(VERSION 3.16)
project(RepairWorkshop VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

set(PROJECT_SOURCES
    Exeptions/AppException.h
    Exeptions/InvalidInputDataException.h
    Exeptions/DuplicateEntityException.h
    Exeptions/EntityNotFoundException.h
    Exeptions/CapacityExceededException.h
    Exeptions/InsufficientStockException.h
    Exeptions/FileStorageException.h
    Exeptions/Exceptions.cpp
    Headers/EnumUtils.h
    Headers/Person.h
    Headers/Client.h
    Headers/Master.h
    Headers/Employee.h
    Headers/StaffEmployee.h
    Headers/MasterEmployee.h
    Headers/AccountingLedger.h
    Headers/Device.h
    Headers/ComputerDevice.h
    Headers/HouseholdDevice.h
    Headers/ElectronicDevice.h
    Headers/SparePart.h
    Headers/OrderItemPart.h
    Headers/Order.h
    Headers/Repository.h
    Headers/InputValidator.h
    Headers/JsonFileHelper.h
    Headers/StoragePersistence.h
    Headers/Storage.h
    Headers/WorkshopSearchService.h
    Headers/WorkshopReportService.h
    Headers/ConsoleUI.h
    Sourse/Person.cpp
    Sourse/Client.cpp
    Sourse/Master.cpp
    Sourse/Employee.cpp
    Sourse/StaffEmployee.cpp
    Sourse/MasterEmployee.cpp
    Sourse/AccountingLedger.cpp
    Sourse/Device.cpp
    Sourse/ComputerDevice.cpp
    Sourse/HouseholdDevice.cpp
    Sourse/ElectronicDevice.cpp
    Sourse/SparePart.cpp
    Sourse/OrderItemPart.cpp
    Sourse/Order.cpp
    Sourse/InputValidator.cpp
    Sourse/JsonFileHelper.cpp
    Sourse/StoragePersistence.cpp
    Sourse/Storage.cpp
    Sourse/WorkshopSearchService.cpp
    Sourse/WorkshopReportService.cpp
    Sourse/ConsoleUI.cpp
    Sourse/main.cpp
    QTInterfase/headers/UiHelpers.h
    QTInterfase/headers/ThemeManager.h
    QTInterfase/headers/SettingsDialog.h
    QTInterfase/headers/WorkersTab.h
    QTInterfase/headers/ClientsTab.h
    QTInterfase/headers/WarehouseTab.h
    QTInterfase/headers/AccountingTab.h
    QTInterfase/headers/QuickClientDialog.h
    QTInterfase/headers/OrderDialog.h
    QTInterfase/headers/MainWindow.h
    QTInterfase/Sourse/ThemeManager.cpp
    QTInterfase/Sourse/SettingsDialog.cpp
    QTInterfase/Sourse/WorkersTab.cpp
    QTInterfase/Sourse/ClientsTab.cpp
    QTInterfase/Sourse/WarehouseTab.cpp
    QTInterfase/Sourse/AccountingTab.cpp
    QTInterfase/Sourse/QuickClientDialog.cpp
    QTInterfase/Sourse/OrderDialog.cpp
    QTInterfase/Sourse/MainWindow.cpp
)

qt_add_executable(RepairWorkshop MANUAL_FINALIZATION ${PROJECT_SOURCES})

target_include_directories(RepairWorkshop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/Exeptions
    ${CMAKE_CURRENT_SOURCE_DIR}/QTInterfase/headers
)

target_link_libraries(RepairWorkshop PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::PrintSupport
)

qt_finalize_executable(RepairWorkshop)
