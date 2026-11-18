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
    Headers/Storage.h \
    Headers/WorkshopReportService.h \
    Headers/ConsoleUI.h \
    QTInterfase/headers/UiHelpers.h \
    QTInterfase/headers/ThemeManager.h \
    QTInterfase/headers/MainWindow.h \
    QTInterfase/headers/OrderDialog.h \
    QTInterfase/headers/ClientDialog.h \
    QTInterfase/headers/MasterDialog.h \
    QTInterfase/headers/PartDialog.h

SOURCES += \
    Exeptions/Exceptions.cpp \
    Sourse/Person.cpp \
    Sourse/Client.cpp \
    Sourse/Master.cpp \
    Sourse/Device.cpp \
    Sourse/ComputerDevice.cpp \
    Sourse/HouseholdDevice.cpp \
    Sourse/ElectronicDevice.cpp \
    Sourse/SparePart.cpp \
    Sourse/OrderItemPart.cpp \
    Sourse/Order.cpp \
    Sourse/InputValidator.cpp \
    Sourse/JsonFileHelper.cpp \
    Sourse/Storage.cpp \
    Sourse/WorkshopReportService.cpp \
    Sourse/ConsoleUI.cpp \
    Sourse/main.cpp \
    QTInterfase/Sourse/ThemeManager.cpp \
    QTInterfase/Sourse/MainWindow.cpp \
    QTInterfase/Sourse/OrderDialog.cpp \
    QTInterfase/Sourse/ClientDialog.cpp \
    QTInterfase/Sourse/MasterDialog.cpp \
    QTInterfase/Sourse/PartDialog.cpp
