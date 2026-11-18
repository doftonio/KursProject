#ifndef PERSON_H
#define PERSON_H

#include <QString>
#include <QJsonObject>
#include <ostream>

class Person {
private:
    unsigned int id;
    QString name;
    QString phone;

public:
    Person(unsigned int id, const QString& name, const QString& phone);
    virtual ~Person() = default;

    unsigned int getId() const;
    QString getName() const;
    QString getPhone() const;

    void setName(const QString& value);
    void setPhone(const QString& value);

    virtual QString getRoleInfo() const = 0;
    virtual QJsonObject toJson() const = 0;

    bool operator==(const Person& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
        os << "[" << person.id << "] " << person.name.toStdString()
           << " (" << person.phone.toStdString() << ")";
        return os;
    }
};

#endif
