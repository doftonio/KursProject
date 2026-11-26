#include "../Headers/StaffEmployee.h"
#include "../Exeptions/InvalidInputDataException.h"

StaffEmployee::StaffEmployee(unsigned int empId, const QString& empName, const QString& empPosition, double salary, double multiplier)
    : Employee(empId, empName, empPosition, multiplier),
      baseSalary(salary >= 0.0 ? salary : 0.0) {}

double StaffEmployee::getBaseSalary() const {
    return baseSalary;
}

void StaffEmployee::setBaseSalary(double salary) {
    if (salary < 0.0) {
        throw InvalidInputDataException("Employee fixed salary cannot be negative");
    }
    baseSalary = salary;
}

QString StaffEmployee::getEmployeeType() const {
    return "Staff";
}

double StaffEmployee::calculateSalary() const {
    return baseSalary * getRateMultiplier();
}

QJsonObject StaffEmployee::toJson() const {
    QJsonObject obj;
    obj["id"] = static_cast<int>(getId());
    obj["name"] = getName();
    obj["position"] = getPosition();
    obj["baseSalary"] = baseSalary;
    obj["rateMultiplier"] = getRateMultiplier();
    return obj;
}

StaffEmployee StaffEmployee::fromJson(const QJsonObject& json) {
    auto parsedId = static_cast<unsigned int>(json["id"].toInt());
    QString parsedName = json["name"].toString();
    QString parsedPos = json["position"].toString("Worker");
    double parsedBase = json["baseSalary"].toDouble(0.0);
    double parsedRate = json["rateMultiplier"].toDouble(1.0);
    return StaffEmployee(parsedId, parsedName, parsedPos, parsedBase, parsedRate);
}
