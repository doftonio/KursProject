#include "../Headers/ElectronicDevice.h"

ElectronicDevice::ElectronicDevice(const QString& model, const QString& serialNumber, const QString& faultDescription, const QString& boardRevision, bool requiresMicroSoldering)
    : Device(model, serialNumber, faultDescription), boardRevision(boardRevision), requiresMicroSoldering(requiresMicroSoldering) {}

QString ElectronicDevice::getBoardRevision() const {
    return boardRevision;
}

bool ElectronicDevice::getRequiresMicroSoldering() const {
    return requiresMicroSoldering;
}

QString ElectronicDevice::getCategory() const {
    return "Electronics";
}

QString ElectronicDevice::getSpecsSummary() const {
    return QString("Плата: %1, BGA/SMD: %2").arg(boardRevision, requiresMicroSoldering ? "Да" : "Нет");
}

double ElectronicDevice::getComplexityFactor() const {
    return requiresMicroSoldering ? 1.45 : 1.15;
}

std::unique_ptr<Device> ElectronicDevice::clone() const {
    return std::make_unique<ElectronicDevice>(*this);
}

QJsonObject ElectronicDevice::toJson() const {
    QJsonObject obj;
    obj["category"] = getCategory();
    obj["model"] = getModel();
    obj["serialNumber"] = getSerialNumber();
    obj["faultDescription"] = getFaultDescription();
    obj["boardRevision"] = boardRevision;
    obj["requiresMicroSoldering"] = requiresMicroSoldering;
    return obj;
}
