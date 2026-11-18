#ifndef STORAGE_H
#define STORAGE_H

#include "Repository.h"
#include "Client.h"
#include "Master.h"
#include "SparePart.h"
#include "Order.h"
#include <QString>
#include <vector>

class Storage {
private:
    QString dataDirectory;
    QString filepathApp;
    QString filepathClients;
    QString filepathMasters;
    QString filepathParts;
    QString filepathOrders;

    unsigned int nextClientId = 1;
    unsigned int nextMasterId = 5;
    unsigned int nextOrderId = 1;
    unsigned int nextPartId = 1;

    Repository<Client> clients;
    Repository<Master> masters;
    Repository<SparePart> parts;
    Repository<Order> orders;

    unsigned int allocateId(unsigned int& counter);
    void initDefaultMasters();
    void loadAppState();
    void saveAppState() const;
    void loadClients();
    void saveClients() const;
    void loadMasters();
    void saveMasters() const;
    void loadParts();
    void saveParts() const;
    void loadOrders();
    void saveOrders() const;

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
    void checkActiveOrdersLimit() const;

    const Repository<Client>& getClients() const;
    const Repository<Master>& getMasters() const;
    const Repository<SparePart>& getParts() const;
    const Repository<Order>& getOrders() const;

    const Client* findClientById(unsigned int id) const;
    const Master* findMasterById(unsigned int id) const;
    const Order* findOrderById(unsigned int id) const;
    SparePart* findPartBySku(const QString& sku);

    std::vector<Client> searchClients(const QString& query) const;
    std::vector<Master> searchMasters(const QString& query) const;
    std::vector<SparePart> searchParts(const QString& query) const;
    std::vector<SparePart> findAvailableParts() const;
    std::vector<Master> findAvailableMasters(const QString& category) const;
    Master* selectLeastLoadedMaster(const QString& category);
    std::vector<Order> filterOrders(int statusFilter, const QString& query) const;

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
