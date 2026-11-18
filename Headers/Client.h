#ifndef CLIENT_H
#define CLIENT_H

#include "Person.h"

class Client : public Person {
private:
    QString note;

public:
    explicit Client(unsigned int id = 0, const QString& name = "", const QString& phone = "", const QString& note = "");

    QString getNote() const;
    void setNote(const QString& value);

    QString getRoleInfo() const override;
    QJsonObject toJson() const override;
    static Client fromJson(const QJsonObject& json);
};

#endif
