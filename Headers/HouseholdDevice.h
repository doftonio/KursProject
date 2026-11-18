#ifndef HOUSEHOLDDEVICE_H
#define HOUSEHOLDDEVICE_H

#include "Device.h"

class HouseholdDevice : public Device {
private:
    int powerWatts;
    bool isPortable;

public:
    explicit HouseholdDevice(const QString& model = "", const QString& serialNumber = "", const QString& faultDescription = "", int powerWatts = 1000, bool isPortable = true);

    int getPowerWatts() const;
    bool getIsPortable() const;

    QString getCategory() const override;
    QString getSpecsSummary() const override;
    double getComplexityFactor() const override;
    std::unique_ptr<Device> clone() const override;
    QJsonObject toJson() const override;
};

#endif
