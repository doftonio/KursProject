#ifndef MASTER_H
#define MASTER_H

#include "Person.h"

class Master : public Person {
private:
    QString specialization;
    int activeOrdersCount;

public:
    static constexpr int maxOrdersPerMaster = 10;

    Master(unsigned int id = 0, const QString& name = "", const QString& phone = "", const QString& specialization = "", int activeOrdersCount = 0);

    QString getSpecialization() const;
    int getActiveOrdersCount() const;

    void setSpecialization(const QString& value);
    bool canAcceptOrder() const;
    void incrementOrders();
    void decrementOrders();

    QString getRoleInfo() const override;
    QJsonObject toJson() const override;
    static Master fromJson(const QJsonObject& json);
};

#endif
