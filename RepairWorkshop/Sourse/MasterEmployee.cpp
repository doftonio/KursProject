#include "../Headers/MasterEmployee.h"
#include "../Exeptions/InvalidInputDataException.h"

MasterEmployee::MasterEmployee(
    unsigned int empId,
    unsigned int linkedMasterId,
    const QString& empName,
    const QString& empSpecialization,
    double minBase,
    double piecework,
    double multiplier
)
    : Employee(empId, empName, "Мастер", multiplier),
      masterId(linkedMasterId),
      specialization(empSpecialization),
      sharedMinBase(minBase >= 0.0 ? minBase : 0.0),
      pieceworkEarnings(piecework >= 0.0 ? piecework : 0.0) {}

unsigned int MasterEmployee::getMasterId() const {
    return masterId;
}

QString MasterEmployee::getSpecialization() const {
    return specialization;
}

double MasterEmployee::getSharedMinBase() const {
    return sharedMinBase;
}

double MasterEmployee::getPieceworkEarnings() const {
    return pieceworkEarnings;
}

void MasterEmployee::setSharedMinBase(double minBase) {
    if (minBase < 0.0) {
        throw InvalidInputDataException("Shared minimum base cannot be negative");
    }
    sharedMinBase = minBase;
}

void MasterEmployee::setPieceworkEarnings(double earnings) {
    if (earnings < 0.0) {
        throw InvalidInputDataException("Piecework earnings cannot be negative");
    }
    pieceworkEarnings = earnings;
}

QString MasterEmployee::getEmployeeType() const {
    return "Master";
}

double MasterEmployee::calculateSalary() const {
    return (sharedMinBase + pieceworkEarnings) * getRateMultiplier();
}

QJsonObject MasterEmployee::toJson() const {
    QJsonObject obj;
    obj["id"] = static_cast<int>(getId());
    obj["masterId"] = static_cast<int>(masterId);
    obj["name"] = getName();
    obj["specialization"] = specialization;
    obj["sharedMinBase"] = sharedMinBase;
    obj["pieceworkEarnings"] = pieceworkEarnings;
    obj["rateMultiplier"] = getRateMultiplier();
    return obj;
}
