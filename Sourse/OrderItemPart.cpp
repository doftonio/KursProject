#include "../Headers/OrderItemPart.h"

OrderItemPart::OrderItemPart(unsigned int partId, const QString& partName, int quantity, double priceAtSale)
    : partId(partId), partName(partName), quantity(quantity), priceAtSale(priceAtSale) {}

unsigned int OrderItemPart::getPartId() const {
    return partId;
}

QString OrderItemPart::getPartName() const {
    return partName;
}

int OrderItemPart::getQuantity() const {
    return quantity;
}

double OrderItemPart::getPriceAtSale() const {
    return priceAtSale;
}

double OrderItemPart::getSubtotal() const {
    return static_cast<double>(quantity) * priceAtSale;
}

void OrderItemPart::addQuantity(int count) {
    quantity += count;
}

void OrderItemPart::reduceQuantity(int count) {
    quantity -= count;
    if (quantity < 0) {
        quantity = 0;
    }
}

QJsonObject OrderItemPart::toJson() const {
    QJsonObject obj;
    obj["partId"] = static_cast<int>(partId);
    obj["partName"] = partName;
    obj["quantity"] = quantity;
    obj["priceAtSale"] = priceAtSale;
    return obj;
}

OrderItemPart OrderItemPart::fromJson(const QJsonObject& json) {
    unsigned int parsedPartId = static_cast<unsigned int>(json["partId"].toInt());
    QString parsedName = json["partName"].toString();
    int parsedQty = json["quantity"].toInt();
    double parsedPrice = json["priceAtSale"].toDouble();
    return OrderItemPart(parsedPartId, parsedName, parsedQty, parsedPrice);
}
