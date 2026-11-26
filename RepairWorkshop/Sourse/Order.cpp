#include "../Headers/Order.h"
#include "../Headers/EnumUtils.h"
#include "../Exeptions/InvalidInputDataException.h"
#include "../Exeptions/EntityNotFoundException.h"
#include <QJsonArray>
#include <numeric>
#include <algorithm>
#include <ranges>

Order::Order(unsigned int id, unsigned int clientId, unsigned int masterId, std::unique_ptr<Device> device, double laborCost)
    : id(id),
      clientId(clientId),
      masterId(masterId),
      device(std::move(device)),
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

Order::Order(Order&& other) noexcept
    : id(other.id),
      clientId(other.clientId),
      masterId(other.masterId),
      device(std::move(other.device)),
      dateReceived(std::move(other.dateReceived)),
      status(other.status),
      laborCost(other.laborCost),
      diagnosticNotes(std::move(other.diagnosticNotes)),
      usedParts(std::move(other.usedParts)) {}

Order& Order::operator=(Order&& other) noexcept {
    if (this != &other) {
        id = other.id;
        clientId = other.clientId;
        masterId = other.masterId;
        device = std::move(other.device);
        dateReceived = std::move(other.dateReceived);
        status = other.status;
        laborCost = other.laborCost;
        diagnosticNotes = std::move(other.diagnosticNotes);
        usedParts = std::move(other.usedParts);
    }
    return *this;
}

Order::~Order() noexcept {
    usedParts.clear();
    device.reset();
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
    return toUnderlying(status);
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
    if (isArchived()) {
        throw InvalidInputDataException("Cannot modify an archived (issued) order");
    }
    if (value < 0.0) {
        throw InvalidInputDataException("Labor cost cannot be negative");
    }
    laborCost = value;
}

void Order::updateStatus(OrderStatus newStatus) {
    using enum OrderStatus;
    auto current = toUnderlying(status);
    auto next = toUnderlying(newStatus);
    if (isArchived()) {
        throw InvalidInputDataException("Archived (issued) order cannot be modified or reopened");
    }
    if (newStatus == Issued && status != Completed) {
        throw InvalidInputDataException("Order must be completed before issuing to client");
    }
    if (next < current && !(status == WaitingParts && newStatus == Diagnostics)) {
        throw InvalidInputDataException("Invalid order status transition");
    }
    status = newStatus;
    if (newStatus == Issued) {
        appendDiagnosticNote("Заказ выдан клиенту и перемещён в архив");
    } else {
        appendDiagnosticNote("Статус изменён на: " + statusToText(newStatus));
    }
}

bool Order::isActive() const {
    using enum OrderStatus;
    return status == Accepted ||
           status == Diagnostics ||
           status == WaitingParts;
}

bool Order::isArchived() const {
    return status == OrderStatus::Issued;
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
    if (isArchived()) {
        throw InvalidInputDataException("Cannot reserve parts for an archived (issued) order");
    }
    if (count <= 0) {
        throw InvalidInputDataException("Reserved part quantity must be positive");
    }
    part -= count;
    *this += OrderItemPart(part.getId(), part.getName(), count, part.getPrice());
}

void Order::returnPart(SparePart& part, int count) {
    if (isArchived()) {
        throw InvalidInputDataException("Cannot return parts from an archived (issued) order");
    }
    if (count <= 0) {
        throw InvalidInputDataException("Returned part quantity must be positive");
    }
    auto it = std::ranges::find_if(usedParts,
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
    if (auto it = std::ranges::find_if(usedParts,
            [&item](const OrderItemPart& existing) { return existing.getPartId() == item.getPartId(); });
        it != usedParts.end()) {
        it->addQuantity(item.getQuantity());
    } else {
        usedParts.push_back(item);
    }
    return *this;
}

bool Order::operator==(const Order& other) const {
    return id == other.id;
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
    obj["status"] = toUnderlying(status);
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
    auto parsedId = static_cast<unsigned int>(json["id"].toInt());
    auto parsedClientId = static_cast<unsigned int>(json["clientId"].toInt());
    auto parsedMasterId = static_cast<unsigned int>(json["masterId"].toInt());
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
    using enum OrderStatus;
    switch (value) {
        case Accepted: return "Принят";
        case Diagnostics: return "Диагностика";
        case WaitingParts: return "Ожидание запчастей";
        case Completed: return "Завершён";
        case Issued: return "Выдан";
        default: return "Неизвестно";
    }
}
