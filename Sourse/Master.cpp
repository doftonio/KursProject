#include "../Headers/Master.h"

Master::Master(unsigned int id, const QString& name, const QString& phone, const QString& specialization, int activeOrdersCount)
    : Person(id, name, phone), specialization(specialization), activeOrdersCount(activeOrdersCount) {}

QString Master::getSpecialization() const {
    return specialization;
}

int Master::getActiveOrdersCount() const {
    return activeOrdersCount;
}

void Master::setSpecialization(const QString& value) {
    specialization = value;
}

bool Master::canAcceptOrder() const {
    return activeOrdersCount < maxOrdersPerMaster;
}

void Master::incrementOrders() {
    ++activeOrdersCount;
}

void Master::decrementOrders() {
    if (activeOrdersCount > 0) {
        --activeOrdersCount;
    }
}

QString Master::getRoleInfo() const {
    return QString("Master #%1: %2 [%3] (Active: %4)")
        .arg(getId())
        .arg(getName(), specialization)
        .arg(activeOrdersCount);
}

QJsonObject Master::toJson() const {
    QJsonObject obj;
    obj["id"] = static_cast<int>(getId());
    obj["name"] = getName();
    obj["phone"] = getPhone();
    obj["specialization"] = specialization;
    obj["activeOrdersCount"] = activeOrdersCount;
    return obj;
}

Master Master::fromJson(const QJsonObject& json) {
    auto parsedId = static_cast<unsigned int>(json["id"].toInt());
    QString parsedName = json["name"].toString();
    QString parsedPhone = json["phone"].toString();
    QString parsedSpec = json["specialization"].toString();
    int parsedCount = json["activeOrdersCount"].toInt(0);
    return Master(parsedId, parsedName, parsedPhone, parsedSpec, parsedCount);
}
