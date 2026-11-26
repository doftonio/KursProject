#ifndef ORDER_H
#define ORDER_H

#include "Device.h"
#include "SparePart.h"
#include "OrderItemPart.h"
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <vector>
#include <memory>
#include <ostream>

enum class OrderStatus {
    Accepted = 0,
    Diagnostics = 1,
    WaitingParts = 2,
    Completed = 3,
    Issued = 4
};

class Order {
private:
    unsigned int id;
    unsigned int clientId;
    unsigned int masterId;
    std::unique_ptr<Device> device;
    QString dateReceived = QDateTime::currentDateTime().toString(Qt::ISODate);
    OrderStatus status = OrderStatus::Accepted;
    double laborCost;
    QString diagnosticNotes;
    std::vector<OrderItemPart> usedParts;

public:
    explicit Order(unsigned int id = 0, unsigned int clientId = 0, unsigned int masterId = 0, std::unique_ptr<Device> device = nullptr, double laborCost = 0.0);
    Order(const Order& other);
    Order& operator=(const Order& other);
    Order(Order&& other) noexcept;
    Order& operator=(Order&& other) noexcept;
    ~Order() noexcept;

    unsigned int getId() const;
    unsigned int getClientId() const;
    unsigned int getMasterId() const;
    const Device* getDevice() const;
    QString getDateReceived() const;
    OrderStatus getStatus() const;
    int getStatusCode() const;
    QString getStatusText() const;
    double getLaborCost() const;
    QString getDiagnosticNotes() const;
    const std::vector<OrderItemPart>& getUsedParts() const;

    void setMasterId(unsigned int value);
    void setDevice(std::unique_ptr<Device> value);
    void setLaborCost(double value);
    void updateStatus(OrderStatus newStatus);
    bool isActive() const;
    bool isArchived() const;
    void appendDiagnosticNote(const QString& note);

    void reservePart(SparePart& part, int count);
    void returnPart(SparePart& part, int count);
    double calculatePartsCost() const;
    double calculateTotalCost() const;

    Order& operator+=(const OrderItemPart& item);
    bool operator==(const Order& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Order& order) {
        os << "Order #" << order.id
           << " | Client: " << order.clientId
           << " | Master: " << order.masterId
           << " | Device: " << (order.device ? order.device->getModel().toStdString() : "N/A")
           << " | StatusCode: " << order.getStatusCode()
           << " | Total: " << order.calculateTotalCost();
        return os;
    }

    QJsonObject toJson() const;
    static Order fromJson(const QJsonObject& json);
    static QString statusToText(OrderStatus value);
};

#endif
