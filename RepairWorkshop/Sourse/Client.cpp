#include "../Headers/Client.h"

Client::Client(unsigned int id, const QString& name, const QString& phone, const QString& note)
    : Person(id, name, phone), note(note) {}

QString Client::getNote() const {
    return note;
}

void Client::setNote(const QString& value) {
    note = value;
}

QString Client::getRoleInfo() const {
    return QString("Client #%1: %2 (%3)").arg(getId()).arg(getName(), getPhone());
}

QJsonObject Client::toJson() const {
    QJsonObject obj;
    obj["id"] = static_cast<int>(getId());
    obj["name"] = getName();
    obj["phone"] = getPhone();
    obj["note"] = note;
    return obj;
}

Client Client::fromJson(const QJsonObject& json) {
    auto parsedId = static_cast<unsigned int>(json["id"].toInt());
    QString parsedName = json["name"].toString();
    QString parsedPhone = json["phone"].toString();
    QString parsedNote = json["note"].toString();
    return Client(parsedId, parsedName, parsedPhone, parsedNote);
}
