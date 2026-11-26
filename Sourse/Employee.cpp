#include "../Headers/Employee.h"
#include "../Exeptions/InvalidInputDataException.h"

Employee::Employee(unsigned int empId, const QString& empName, const QString& empPosition, double multiplier)
    : id(empId),
      name(empName),
      position(empPosition),
      rateMultiplier(multiplier > 0.0 ? multiplier : 1.0) {}

unsigned int Employee::getId() const {
    return id;
}

QString Employee::getName() const {
    return name;
}

QString Employee::getPosition() const {
    return position;
}

double Employee::getRateMultiplier() const {
    return rateMultiplier;
}

void Employee::setName(const QString& value) {
    name = value;
}

void Employee::setPosition(const QString& value) {
    position = value;
}

void Employee::setRateMultiplier(double multiplier) {
    if (multiplier <= 0.0) {
        throw InvalidInputDataException("Salary rate multiplier must be greater than zero");
    }
    rateMultiplier = multiplier;
}
