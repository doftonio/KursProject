#include "../Headers/HouseholdDevice.h"

HouseholdDevice::HouseholdDevice(const QString& model, const QString& serialNumber, const QString& faultDescription, int powerWatts, bool isPortable)
    : Device(model, serialNumber, faultDescription), powerWatts(powerWatts), isPortable(isPortable) {}

int HouseholdDevice::getPowerWatts() const {
    return powerWatts;
}

bool HouseholdDevice::getIsPortable() const {
    return isPortable;
}

QString HouseholdDevice::getCategory() const {
    return "Household";
}

QString HouseholdDevice::getSpecsSummary() const {
    return QString("Мощность: %1 Вт, Переносной: %2").arg(powerWatts).arg(isPortable ? "Да" : "Нет");
}

double HouseholdDevice::getComplexityFactor() const {
    return isPortable ? 1.10 : 1.30;
}

std::unique_ptr<Device> HouseholdDevice::clone() const {
    return std::make_unique<HouseholdDevice>(*this);
}

QJsonObject HouseholdDevice::toJson() const {
    QJsonObject obj;
    obj["category"] = getCategory();
    obj["model"] = getModel();
    obj["serialNumber"] = getSerialNumber();
    obj["faultDescription"] = getFaultDescription();
    obj["powerWatts"] = powerWatts;
    obj["isPortable"] = isPortable;
    return obj;
}
