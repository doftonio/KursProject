#ifndef STORAGEPERSISTENCE_H
#define STORAGEPERSISTENCE_H

#include "Repository.h"
#include "Client.h"
#include "Master.h"
#include "SparePart.h"
#include "Order.h"
#include <QString>

struct IdCounters {
    unsigned int client = 1;
    unsigned int master = 5;
    unsigned int order = 1;
    unsigned int part = 1;
};

struct WorkshopCollections {
    Repository<Client>& clients;
    Repository<Master>& masters;
    Repository<SparePart>& parts;
    Repository<Order>& orders;
};

struct ConstWorkshopCollections {
    const Repository<Client>& clients;
    const Repository<Master>& masters;
    const Repository<SparePart>& parts;
    const Repository<Order>& orders;
};

class StoragePersistence {
private:
    QString dataDirectory;
    QString filepathApp;
    QString filepathClients;
    QString filepathMasters;
    QString filepathParts;
    QString filepathOrders;

    void loadAppState(IdCounters& counters) const;
    void loadClients(Repository<Client>& clients, IdCounters& counters) const;
    void loadMasters(Repository<Master>& masters, IdCounters& counters) const;
    void loadParts(Repository<SparePart>& parts, IdCounters& counters) const;
    void loadOrders(Repository<Order>& orders, IdCounters& counters) const;
    static void populateDefaultMasters(Repository<Master>& masters, IdCounters& counters);

public:
    explicit StoragePersistence(const QString& baseDir = "data");

    void ensureStorageReady() const;
    void loadAll(const WorkshopCollections& collections, IdCounters& counters) const;
    void saveAll(const ConstWorkshopCollections& collections, const IdCounters& counters) const;
    void saveAppState(const IdCounters& counters) const;
    void saveClients(const Repository<Client>& clients) const;
    void saveMasters(const Repository<Master>& masters) const;
    void saveParts(const Repository<SparePart>& parts) const;
    void saveOrders(const Repository<Order>& orders) const;
    QString backup(const ConstWorkshopCollections& collections, const IdCounters& counters, const QString& backupRoot = "backups") const;
    void restore(const WorkshopCollections& collections, IdCounters& counters, const QString& backupFolder) const;
};

#endif
