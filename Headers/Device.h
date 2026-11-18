#ifndef DEVICE_H
#define DEVICE_H

#include <QString>
#include <QJsonObject>
#include <memory>

class Device {
private:
    QString model;
    QString serialNumber;
    QString faultDescription;

public:
    Device(const QString& model, const QString& serialNumber, const QString& faultDescription);
    virtual ~Device() = default;

    QString getModel() const;
    QString getSerialNumber() const;
    QString getFaultDescription() const;

    void setModel(const QString& value);
    void setSerialNumber(const QString& value);
    void setFaultDescription(const QString& value);

    virtual QString getCategory() const = 0;
    virtual QString getSpecsSummary() const = 0;
    virtual double getComplexityFactor() const = 0;
    virtual std::unique_ptr<Device> clone() const = 0;
    virtual QJsonObject toJson() const = 0;

    static std::unique_ptr<Device> fromJson(const QJsonObject& json);
};

#endif
