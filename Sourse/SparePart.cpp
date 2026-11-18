#include "../Headers/SparePart.h"
#include "../Exeptions/InsufficientStockException.h"
#include "../Exeptions/InvalidInputDataException.h"

SparePart::SparePart(unsigned int id, const QString& sku, const QString& name, int quantity, double price)
    : id(id), sku(sku), name(name), quantity(quantity), price(price) {}

unsigned int SparePart::getId() const {
    return id;
}

QString SparePart::getSku() const {
    return sku;
}

QString SparePart::getName() const {
    return name;
}

int SparePart::getQuantity() const {
    return quantity;
}

double SparePart::getPrice() const {
    return price;
}

void SparePart::setName(const QString& value) {
    name = value;
}

void SparePart::setPrice(double value) {
    if (value <= 0.0) {
        throw InvalidInputDataException("Part price must be greater than zero");
    }
    price = value;
}

void SparePart::decreaseStock(int count) {
    if (count <= 0) {
        throw InvalidInputDataException("Decrease count must be positive");
    }
    if (quantity < count) {
        throw InsufficientStockException("Insufficient stock for SKU: " + sku.toStdString());
    }
    quantity -= count;
}

void SparePart::increaseStock(int count) {
    if (count <= 0) {
        throw InvalidInputDataException("Increase count must be positive");
    }
    quantity += count;
}

SparePart& SparePart::operator+=(int count) {
    increaseStock(count);
    return *this;
}

SparePart& SparePart::operator-=(int count) {
    decreaseStock(count);
    return *this;
}

bool SparePart::operator<(const SparePart& other) const {
    return sku < other.sku;
}

bool SparePart::operator==(const SparePart& other) const {
    return id == other.id || sku == other.sku;
}

std::ostream& operator<<(std::ostream& os, const SparePart& part) {
    os << "[" << part.id << "] " << part.sku.toStdString()
       << " - " << part.name.toStdString()
       << " | Qty: " << part.quantity << " | Price: " << part.price;
    return os;
}

QJsonObject SparePart::toJson() const {
    QJsonObject obj;
    obj["id"] = static_cast<int>(id);
    obj["sku"] = sku;
    obj["name"] = name;
    obj["quantity"] = quantity;
    obj["price"] = price;
    return obj;
}

SparePart SparePart::fromJson(const QJsonObject& json) {
    unsigned int parsedId = static_cast<unsigned int>(json["id"].toInt());
    QString parsedSku = json["sku"].toString();
    QString parsedName = json["name"].toString();
    int parsedQty = json["quantity"].toInt();
    double parsedPrice = json["price"].toDouble();
    return SparePart(parsedId, parsedSku, parsedName, parsedQty, parsedPrice);
}
