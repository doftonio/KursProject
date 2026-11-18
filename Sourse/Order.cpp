#include "../Headers/Order.h"
#include "../Exeptions/InvalidInputDataException.h"
#include "../Exeptions/EntityNotFoundException.h"
#include <QDateTime>
#include <QJsonArray>
#include <numeric>
#include <algorithm>

Order::Order(unsigned int id, unsigned int clientId, unsigned int masterId, std::unique_ptr<Device> device, double laborCost)
    : id(id),
      clientId(clientId),
      masterId(masterId),
      device(std::move(device)),
      dateReceived(QDateTime::currentDateTime().toString(Qt::ISODate)),
      status(OrderStatus::Accepted),
      laborCost(laborCost) {}

Order::Order(const Order& other)
    : id(other.id),
      clientId(other.clientId),
      masterId(other.masterId),
      device(other.device ? other.device->clone() : nullptr),
      dateReceived(other.dateReceived),
      status(other.status),
      laborCost(other.laborCost),
      diagnosticNotes(other.diagnosticNotes),
      usedParts(other.usedParts) {}

Order& Order::operator=(const Order& other) {
    if (this != &other) {
        id = other.id;
        clientId = other.clientId;
        masterId = other.masterId;
        device = other.device ? other.device->clone() : nullptr;
        dateReceived = other.dateReceived;
        status = other.status;
        laborCost = other.laborCost;
        diagnosticNotes = other.diagnosticNotes;
        usedParts = other.usedParts;
    }
    return *this;
}

unsigned int Order::getId() const {
    return id;
}

unsigned int Order::getClientId() const {
    return clientId;
}

unsigned int Order::getMasterId() const {
    return masterId;
}

const Device* Order::getDevice() const {
    return device.get();
}

QString Order::getDateReceived() const {
    return dateReceived;
}

OrderStatus Order::getStatus() const {
    return status;
}

int Order::getStatusCode() const {
    return static_cast<int>(status);
}

QString Order::getStatusText() const {
    return statusToText(status);
}

double Order::getLaborCost() const {
    return laborCost;
}

QString Order::getDiagnosticNotes() const {
    return diagnosticNotes;
}

const std::vector<OrderItemPart>& Order::getUsedParts() const {
    return usedParts;
}

void Order::setMasterId(unsigned int value) {
    masterId = value;
}

void Order::setDevice(std::unique_ptr<Device> value) {
    device = std::move(value);
}

void Order::setLaborCost(double value) {
    if (value < 0.0) {
        throw InvalidInputDataException("Labor cost cannot be negative");
    }
    laborCost = value;
}

void Order::updateStatus(OrderStatus newStatus) {
    int current = static_cast<int>(status);
    int next = static_cast<int>(newStatus);
    if (status == OrderStatus::Issued && newStatus != OrderStatus::Issued) {
        throw InvalidInputDataException("Cannot reopen an already issued order");
    }
    if (newStatus == OrderStatus::Issued && status != OrderStatus::Completed) {
        throw InvalidInputDataException("Order must be completed before issuing to client");
    }
    if (next < current && !(status == OrderStatus::WaitingParts && newStatus == OrderStatus::Diagnostics)) {
        throw InvalidInputDataException("Invalid order status transition");
    }
    status = newStatus;
    appendDiagnosticNote("Статус изменён на: " + statusToText(newStatus));
}

bool Order::isActive() const {
    return status == OrderStatus::Accepted ||
           status == OrderStatus::Diagnostics ||
           status == OrderStatus::WaitingParts;
}

void Order::appendDiagnosticNote(const QString& note) {
    QString trimmed = note.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    QString entry = QString("[%1] %2").arg(timestamp, trimmed);
    if (!diagnosticNotes.isEmpty()) {
        diagnosticNotes += "\n";
    }
    diagnosticNotes += entry;
}

void Order::reservePart(SparePart& part, int count) {
    if (count <= 0) {
        throw InvalidInputDataException("Reserved part quantity must be positive");
    }
    part -= count;
    *this += OrderItemPart(part.getId(), part.getName(), count, part.getPrice());
}

void Order::returnPart(SparePart& part, int count) {
    if (count <= 0) {
        throw InvalidInputDataException("Returned part quantity must be positive");
    }
    auto it = std::find_if(usedParts.begin(), usedParts.end(),
        [&part](const OrderItemPart& item) { return item.getPartId() == part.getId(); });

    if (it == usedParts.end()) {
        throw EntityNotFoundException("Part is not attached to this order");
    }
    if (it->getQuantity() < count) {
        throw InvalidInputDataException("Cannot return more parts than attached to order");
    }

    part += count;
    it->reduceQuantity(count);
    if (it->getQuantity() == 0) {
        usedParts.erase(it);
    }
}

double Order::calculatePartsCost() const {
    return std::accumulate(usedParts.cbegin(), usedParts.cend(), 0.0,
        [](double sum, const OrderItemPart& item) {
            return sum + item.getSubtotal();
        });
}

double Order::calculateTotalCost() const {
    double complexity = device ? device->getComplexityFactor() : 1.0;
    return (laborCost * complexity) + calculatePartsCost();
}

Order& Order::operator+=(const OrderItemPart& item) {
    auto it = std::find_if(usedParts.begin(), usedParts.end(),
        [&item](const OrderItemPart& existing) { return existing.getPartId() == item.getPartId(); });
    if (it != usedParts.end()) {
        it->addQuantity(item.getQuantity());
    } else {
        usedParts.push_back(item);
    }
    return *this;
}

bool Order::operator==(const Order& other) const {
    return id == other.id;
}

std::ostream& operator<<(std::ostream& os, const Order& order) {
    os << "Order #" << order.id
       << " | Client: " << order.clientId
       << " | Master: " << order.masterId
       << " | Device: " << (order.device ? order.device->getModel().toStdString() : "N/A")
       << " | StatusCode: " << order.getStatusCode()
       << " | Total: " << order.calculateTotalCost();
    return os;
}

QJsonObject Order::toJson() const {
    QJsonObject obj;
    obj["id"] = static_cast<int>(id);
    obj["clientId"] = static_cast<int>(clientId);
    obj["masterId"] = static_cast<int>(masterId);
    if (device) {
        obj["device"] = device->toJson();
    }
    obj["dateReceived"] = dateReceived;
    obj["status"] = static_cast<int>(status);
    obj["laborCost"] = laborCost;
    obj["diagnosticNotes"] = diagnosticNotes;

    QJsonArray partsArr;
    for (const auto& item : usedParts) {
        partsArr.append(item.toJson());
    }
    obj["usedParts"] = partsArr;
    return obj;
}

Order Order::fromJson(const QJsonObject& json) {
    unsigned int parsedId = static_cast<unsigned int>(json["id"].toInt());
    unsigned int parsedClientId = static_cast<unsigned int>(json["clientId"].toInt());
    unsigned int parsedMasterId = static_cast<unsigned int>(json["masterId"].toInt());
    double parsedLabor = json["laborCost"].toDouble();

    std::unique_ptr<Device> parsedDevice = nullptr;
    if (json.contains("device") && json["device"].isObject()) {
        parsedDevice = Device::fromJson(json["device"].toObject());
    }

    Order order(parsedId, parsedClientId, parsedMasterId, std::move(parsedDevice), parsedLabor);
    order.dateReceived = json["dateReceived"].toString();
    order.status = static_cast<OrderStatus>(json["status"].toInt(0));
    order.diagnosticNotes = json["diagnosticNotes"].toString();

    QJsonArray partsArr = json["usedParts"].toArray();
    for (const auto& val : partsArr) {
        if (val.isObject()) {
            order.usedParts.push_back(OrderItemPart::fromJson(val.toObject()));
        }
    }
    return order;
}

QString Order::statusToText(OrderStatus value) {
    switch (value) {
        case OrderStatus::Accepted: return "Принят";
        case OrderStatus::Diagnostics: return "Диагностика";
        case OrderStatus::WaitingParts: return "Ожидание запчастей";
        case OrderStatus::Completed: return "Завершён";
        case OrderStatus::Issued: return "Выдан";
    }
    return "Неизвестно";
}
