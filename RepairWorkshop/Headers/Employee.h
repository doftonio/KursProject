#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QString>
#include <QJsonObject>

class Employee {
private:
    unsigned int id;
    QString name;
    QString position;
    double rateMultiplier;

public:
    Employee(unsigned int id, const QString& name, const QString& position, double rateMultiplier = 1.0);
    virtual ~Employee() = default;

    unsigned int getId() const;
    QString getName() const;
    QString getPosition() const;
    double getRateMultiplier() const;

    void setName(const QString& value);
    void setPosition(const QString& value);
    void setRateMultiplier(double multiplier);

    virtual QString getEmployeeType() const = 0;
    virtual double calculateSalary() const = 0;
    virtual QJsonObject toJson() const = 0;
};

#endif
