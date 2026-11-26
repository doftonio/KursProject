#ifndef ELECTRONICDEVICE_H
#define ELECTRONICDEVICE_H

#include "Device.h"

class ElectronicDevice : public Device {
private:
    QString boardRevision;
    bool requiresMicroSoldering;

public:
    explicit ElectronicDevice(const QString& model = "", const QString& serialNumber = "", const QString& faultDescription = "", const QString& boardRevision = "Rev 1.0", bool requiresMicroSoldering = false);

    QString getBoardRevision() const;
    bool getRequiresMicroSoldering() const;

    QString getCategory() const override;
    QString getSpecsSummary() const override;
    double getComplexityFactor() const override;
    std::unique_ptr<Device> clone() const override;
    QJsonObject toJson() const override;
};

#endif
