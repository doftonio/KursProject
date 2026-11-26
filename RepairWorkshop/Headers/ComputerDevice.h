#ifndef COMPUTERDEVICE_H
#define COMPUTERDEVICE_H

#include "Device.h"

class ComputerDevice : public Device {
private:
    QString osType;
    bool hasCharger;

public:
    explicit ComputerDevice(const QString& model = "", const QString& serialNumber = "", const QString& faultDescription = "", const QString& osType = "Windows", bool hasCharger = true);

    QString getOsType() const;
    bool getHasCharger() const;

    QString getCategory() const override;
    QString getSpecsSummary() const override;
    double getComplexityFactor() const override;
    std::unique_ptr<Device> clone() const override;
    QJsonObject toJson() const override;
};

#endif
