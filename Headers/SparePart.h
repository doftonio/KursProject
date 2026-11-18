#ifndef SPAREPART_H
#define SPAREPART_H

#include <QString>
#include <QJsonObject>
#include <compare>
#include <ostream>

class SparePart {
private:
    unsigned int id;
    QString sku;
    QString name;
    int quantity;
    double price;

public:
    explicit SparePart(unsigned int id = 0, const QString& sku = "", const QString& name = "", int quantity = 0, double price = 0.0);

    unsigned int getId() const;
    QString getSku() const;
    QString getName() const;
    int getQuantity() const;
    double getPrice() const;

    void setName(const QString& value);
    void setPrice(double value);
    void decreaseStock(int count);
    void increaseStock(int count);

    SparePart& operator+=(int count);
    SparePart& operator-=(int count);
    std::strong_ordering operator<=>(const SparePart& other) const;
    bool operator==(const SparePart& other) const;

    friend std::ostream& operator<<(std::ostream& os, const SparePart& part) {
        os << "[" << part.id << "] " << part.sku.toStdString()
           << " - " << part.name.toStdString()
           << " | Qty: " << part.quantity << " | Price: " << part.price;
        return os;
    }

    QJsonObject toJson() const;
    static SparePart fromJson(const QJsonObject& json);
};

#endif
