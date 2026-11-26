#include "../Headers/MasterEmployee.h"
#include "../Exeptions/InvalidInputDataException.h"

MasterEmployee::MasterEmployee(
    unsigned int id,
    unsigned int masterId,
    const QString& name,
    const QString& specialization,
    double sharedMinBase,
    double pieceworkEarnings,
    double rateMultiplier
)
    : Employee(id, name, "Мастер", rateMultiplier),
      masterId(masterId),
      specialization(specialization),
      sharedMinBase(sharedMinBase >= 0.0 ? sharedMinBase : 0.0),
      pieceworkEarnings(pieceworkEarnings >= 0.0 ? pieceworkEarnings : 0.0) {}

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
