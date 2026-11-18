#ifndef STORAGE_H
#define STORAGE_H

#include "Repository.h"
#include "Client.h"
#include "Master.h"
#include "SparePart.h"
#include "Order.h"
#include <QString>
#include <QDate>
#include <vector>

struct PerformanceReport {
    int totalOrders = 0;
    int activeOrders = 0;
    int completedOrders = 0;
    double totalLaborRevenue = 0.0;
    double totalPartsRevenue = 0.0;
    double totalRevenue = 0.0;
};

class Storage {
private:
    QString dataDirectory;
    QString filepathApp;
    QString filepathClients;
    QString filepathMasters;
    QString filepathParts;
    QString filepathOrders;

    unsigned int nextClientId;
    unsigned int nextMasterId;
    unsigned int nextOrderId;
    unsigned int nextPartId;

    Repository<Client> clients;
    Repository<Master> masters;
    Repository<SparePart> parts;
    Repository<Order> orders;

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
    void writeJsonAtomic(const QString& path, const QByteArray& content) const;

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

    unsigned int requestClientId();
    unsigned int requestMasterId();
    unsigned int requestOrderId();
    unsigned int requestPartId();

    int countActiveOrders() const;
    void checkActiveOrdersLimit() const;

    const Repository<Client>& getClients() const;
    const Repository<Master>& getMasters() const;
    const Repository<SparePart>& getParts() const;
    const Repository<Order>& getOrders() const;

    Client* findClientById(unsigned int id);
    const Client* findClientById(unsigned int id) const;
    Master* findMasterById(unsigned int id);
    const Master* findMasterById(unsigned int id) const;
    SparePart* findPartById(unsigned int id);
    SparePart* findPartBySku(const QString& sku);
    Order* findOrderById(unsigned int id);
    const Order* findOrderById(unsigned int id) const;

    std::vector<Client> searchClients(const QString& query) const;
    std::vector<Master> searchMasters(const QString& query) const;
    std::vector<SparePart> searchParts(const QString& query) const;
    std::vector<SparePart> findAvailableParts() const;
    std::vector<Master> findAvailableMasters(const QString& category) const;
    Master* selectLeastLoadedMaster(const QString& category);
    std::vector<Order> filterOrders(int statusFilter, const QString& query) const;
    std::vector<Order> findOrdersByClient(unsigned int clientId) const;
    std::vector<Order> findOrdersByMaster(unsigned int masterId) const;

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

    QString generateWarrantyActHtml(unsigned int orderId) const;
    PerformanceReport generatePerformanceReport(const QDate& from, const QDate& to) const;
};

#endif
