#ifndef STORAGE_H
#define STORAGE_H

#include "StoragePersistence.h"
#include "Repository.h"
#include "Client.h"
#include "Master.h"
#include "SparePart.h"
#include "Order.h"
#include <QString>

class Storage {
private:
    StoragePersistence persistence;
    IdCounters counters;

    Repository<Client> clients;
    Repository<Master> masters;
    Repository<SparePart> parts;
    Repository<Order> orders;

    unsigned int allocateId(unsigned int& counter) const;
    static void ensureOrderEditable(const Order& order);

public:
    static constexpr int maxActiveOrdersLimit = 30;
    static constexpr int maxTotalOrdersLimit = 1000;
    static constexpr int maxClientsLimit = 500;
    static constexpr int maxMastersLimit = 20;
    static constexpr int maxPartsLimit = 200;

    explicit Storage(const QString& baseDir = "data");

    void loadDatabaseFromJson();
    void saveDatabaseToJson() const;
    QString backupDatabase(const QString& backupRoot = "backups") const;
    void restoreDatabase(const QString& backupFolder);

    int countActiveOrders() const;
    int countArchivedOrders() const;
    void checkActiveOrdersLimit() const;

    const Repository<Client>& getClients() const;
    const Repository<Master>& getMasters() const;
    const Repository<SparePart>& getParts() const;
    const Repository<Order>& getOrders() const;

    const Client* findClientById(unsigned int id) const;
    const Master* findMasterById(unsigned int id) const;
    const Order* findOrderById(unsigned int id) const;
    SparePart* findPartBySku(const QString& sku);
    Master* selectLeastLoadedMaster(const QString& category);

    Client& addClient(const QString& name, const QString& phone, const QString& note = "");
    Master& addMaster(const QString& name, const QString& phone, const QString& specialization);
    void updateMasterSpecialization(unsigned int masterId, const QString& newSpecialization);
    void removeMaster(unsigned int masterId);

    SparePart& addPart(const QString& sku, const QString& name, int quantity, double price);
    void restockPart(unsigned int partId, int addCount);

    Order& addOrder(unsigned int clientId, unsigned int masterId, std::unique_ptr<Device> device, double laborCost);
    void changeOrderStatus(unsigned int orderId, OrderStatus newStatus);
    void appendOrderNote(unsigned int orderId, const QString& note);
    void reservePartForOrder(unsigned int orderId, unsigned int partId, int count);
    void returnPartFromOrder(unsigned int orderId, unsigned int partId, int count);
    void updateOrderLaborCost(unsigned int orderId, double newLaborCost);
};

#endif
