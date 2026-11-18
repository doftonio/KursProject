#include "../Headers/ComputerDevice.h"

ComputerDevice::ComputerDevice(const QString& model, const QString& serialNumber, const QString& faultDescription, const QString& osType, bool hasCharger)
    : Device(model, serialNumber, faultDescription), osType(osType), hasCharger(hasCharger) {}

QString ComputerDevice::getOsType() const {
    return osType;
}

bool ComputerDevice::getHasCharger() const {
    return hasCharger;
}

QString ComputerDevice::getCategory() const {
    return "Computer";
}

QString ComputerDevice::getSpecsSummary() const {
    return QString("ОС: %1, ЗУ: %2").arg(osType, hasCharger ? "Да" : "Нет");
}

double ComputerDevice::getComplexityFactor() const {
    return 1.25;
}

std::unique_ptr<Device> ComputerDevice::clone() const {
    return std::make_unique<ComputerDevice>(*this);
}

QJsonObject ComputerDevice::toJson() const {
    QJsonObject obj;
    obj["category"] = getCategory();
    obj["model"] = getModel();
    obj["serialNumber"] = getSerialNumber();
    obj["faultDescription"] = getFaultDescription();
    obj["osType"] = osType;
    obj["hasCharger"] = hasCharger;
    return obj;
}
