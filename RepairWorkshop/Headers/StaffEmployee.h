#ifndef STAFFEMPLOYEE_H
#define STAFFEMPLOYEE_H

#include "Employee.h"

class StaffEmployee : public Employee {
private:
    double baseSalary;

public:
    explicit StaffEmployee(unsigned int id = 0, const QString& name = "", const QString& position = "Worker", double baseSalary = 0.0, double rateMultiplier = 1.0);

    double getBaseSalary() const;
    void setBaseSalary(double salary);

    QString getEmployeeType() const override;
    double calculateSalary() const override;
    QJsonObject toJson() const override;
    static StaffEmployee fromJson(const QJsonObject& json);
};

#endif
