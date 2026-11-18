#include "../Headers/Device.h"
#include "../Headers/ComputerDevice.h"
#include "../Headers/HouseholdDevice.h"
#include "../Headers/ElectronicDevice.h"
#include "../Exeptions/InvalidInputDataException.h"

Device::Device(const QString& model, const QString& serialNumber, const QString& faultDescription)
    : model(model), serialNumber(serialNumber), faultDescription(faultDescription) {}

QString Device::getModel() const {
    return model;
}

QString Device::getSerialNumber() const {
    return serialNumber;
}

QString Device::getFaultDescription() const {
    return faultDescription;
}

void Device::setModel(const QString& value) {
    model = value;
}

void Device::setSerialNumber(const QString& value) {
    serialNumber = value;
}

void Device::setFaultDescription(const QString& value) {
    faultDescription = value;
}

std::unique_ptr<Device> Device::fromJson(const QJsonObject& json) {
    QString category = json["category"].toString();
    QString model = json["model"].toString();
    QString serial = json["serialNumber"].toString();
    QString fault = json["faultDescription"].toString();

    if (category == "Computer") {
        QString osType = json["osType"].toString("Windows");
        bool hasCharger = json["hasCharger"].toBool(true);
        return std::make_unique<ComputerDevice>(model, serial, fault, osType, hasCharger);
    }
    if (category == "Household") {
        int powerWatts = json["powerWatts"].toInt(1000);
        bool isPortable = json["isPortable"].toBool(true);
        return std::make_unique<HouseholdDevice>(model, serial, fault, powerWatts, isPortable);
    }
    if (category == "Electronics") {
        QString boardRev = json["boardRevision"].toString("Rev 1.0");
        bool microSoldering = json["requiresMicroSoldering"].toBool(false);
        return std::make_unique<ElectronicDevice>(model, serial, fault, boardRev, microSoldering);
    }
    throw InvalidInputDataException("Unknown device category in JSON: " + category.toStdString());
}
