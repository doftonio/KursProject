#ifndef ORDERITEMPART_H
#define ORDERITEMPART_H

#include <QString>
#include <QJsonObject>

class OrderItemPart {
private:
    unsigned int partId;
    QString partName;
    int quantity;
    double priceAtSale;

public:
    OrderItemPart(unsigned int partId = 0, const QString& partName = "", int quantity = 0, double priceAtSale = 0.0);

    unsigned int getPartId() const;
    QString getPartName() const;
    int getQuantity() const;
    double getPriceAtSale() const;
    double getSubtotal() const;

    void addQuantity(int count);
    void reduceQuantity(int count);

    QJsonObject toJson() const;
    static OrderItemPart fromJson(const QJsonObject& json);
};

#endif
