#include "../Headers/Person.h"

Person::Person(unsigned int id, const QString& name, const QString& phone)
    : id(id), name(name), phone(phone) {}

unsigned int Person::getId() const {
    return id;
}

QString Person::getName() const {
    return name;
}

QString Person::getPhone() const {
    return phone;
}

void Person::setName(const QString& value) {
    name = value;
}

void Person::setPhone(const QString& value) {
    phone = value;
}

bool Person::operator==(const Person& other) const {
    return id == other.id || (!phone.isEmpty() && phone == other.phone);
}
