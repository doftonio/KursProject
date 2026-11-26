#ifndef MASTEREMPLOYEE_H
#define MASTEREMPLOYEE_H

#include "Employee.h"

class MasterEmployee : public Employee {
private:
    unsigned int masterId;
    QString specialization;
    double sharedMinBase;
    double pieceworkEarnings;

public:
    explicit MasterEmployee(
        unsigned int id = 0,
        unsigned int masterId = 0,
        const QString& name = "",
        const QString& specialization = "",
        double sharedMinBase = 0.0,
        double pieceworkEarnings = 0.0,
        double rateMultiplier = 1.0
    );

    unsigned int getMasterId() const;
    QString getSpecialization() const;
    double getSharedMinBase() const;
    double getPieceworkEarnings() const;

    void setSharedMinBase(double minBase);
    void setPieceworkEarnings(double earnings);

    QString getEmployeeType() const override;
    double calculateSalary() const override;
    QJsonObject toJson() const override;
};

#endif
