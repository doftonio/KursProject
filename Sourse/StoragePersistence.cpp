#include "../Headers/StoragePersistence.h"
#include "../Headers/JsonFileHelper.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

StoragePersistence::StoragePersistence(const QString& baseDir)
    : dataDirectory(baseDir),
      filepathApp(baseDir + "/app_state.json"),
      filepathClients(baseDir + "/clients.json"),
      filepathMasters(baseDir + "/masters.json"),
      filepathParts(baseDir + "/parts.json"),
      filepathOrders(baseDir + "/orders.json") {}

void StoragePersistence::ensureStorageReady() const {
    JsonFileHelper::ensureDirectory(dataDirectory);
}

void StoragePersistence::populateDefaultMasters(Repository<Master>& masters, IdCounters& counters) {
    masters.clear();
    masters += Master(1, "Иванов Алексей Петрович", "+375291112233", "Computer");
    masters += Master(2, "Смирнов Дмитрий Олегович", "+375292223344", "Electronics");
    masters += Master(3, "Коваленко Сергей Викторович", "+375333334455", "Household");
    masters += Master(4, "Морозов Андрей Игоревич", "+375444445566", "Electronics");
    if (counters.master < 5) {
        counters.master = 5;
    }
}

void StoragePersistence::loadAppState(IdCounters& counters) const {
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathApp, true, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        counters = IdCounters{};
        saveAppState(counters);
        return;
    }
    QJsonObject obj = doc.object();
    counters.client = static_cast<unsigned int>(obj["nextClientId"].toInt(1));
    counters.master = static_cast<unsigned int>(obj["nextMasterId"].toInt(5));
    counters.order = static_cast<unsigned int>(obj["nextOrderId"].toInt(1));
    counters.part = static_cast<unsigned int>(obj["nextPartId"].toInt(1));
}

void StoragePersistence::saveAppState(const IdCounters& counters) const {
    QJsonObject obj;
    obj["nextClientId"] = static_cast<int>(counters.client);
    obj["nextMasterId"] = static_cast<int>(counters.master);
    obj["nextOrderId"] = static_cast<int>(counters.order);
    obj["nextPartId"] = static_cast<int>(counters.part);
    JsonFileHelper::writeAtomic(filepathApp, QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

void StoragePersistence::loadClients(Repository<Client>& clients, IdCounters& counters) const {
    clients.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathClients, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        saveClients(clients);
        return;
    }
    for (const auto& val : doc.array()) {
        if (!val.isObject()) {
            continue;
        }
        Client c = Client::fromJson(val.toObject());
        clients.add(c);
        if (c.getId() >= counters.client) {
            counters.client = c.getId() + 1;
        }
    }
}

void StoragePersistence::saveClients(const Repository<Client>& clients) const {
    QJsonArray arr;
    for (const auto& c : clients) {
        arr.append(c.toJson());
    }
    JsonFileHelper::writeAtomic(filepathClients, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void StoragePersistence::loadMasters(Repository<Master>& masters, IdCounters& counters) const {
    masters.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathMasters, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        populateDefaultMasters(masters, counters);
        saveMasters(masters);
        return;
    }
    bool migrated = false;
    for (const auto& val : doc.array()) {
        if (!val.isObject()) {
            continue;
        }
        Master m = Master::fromJson(val.toObject());
        if (m.getName() == "Ivanov Alexey Petrovich") {
            m.setName("Иванов Алексей Петрович");
            migrated = true;
        } else if (m.getName() == "Smirnov Dmitry Olegovich") {
            m.setName("Смирнов Дмитрий Олегович");
            migrated = true;
        } else if (m.getName() == "Kovalenko Sergey Viktorovich") {
            m.setName("Коваленко Сергей Викторович");
            migrated = true;
        } else if (m.getName() == "Morozov Andrey Igorevich") {
            m.setName("Морозов Андрей Игоревич");
            migrated = true;
        }
        masters.add(m);
        if (m.getId() >= counters.master) {
            counters.master = m.getId() + 1;
        }
    }
    if (masters.empty()) {
        populateDefaultMasters(masters, counters);
        saveMasters(masters);
    } else if (migrated) {
        saveMasters(masters);
    }
}

void StoragePersistence::saveMasters(const Repository<Master>& masters) const {
    QJsonArray arr;
    for (const auto& m : masters) {
        arr.append(m.toJson());
    }
    JsonFileHelper::writeAtomic(filepathMasters, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void StoragePersistence::loadParts(Repository<SparePart>& parts, IdCounters& counters) const {
    parts.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathParts, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        saveParts(parts);
        return;
    }
    for (const auto& val : doc.array()) {
        if (!val.isObject()) {
            continue;
        }
        SparePart p = SparePart::fromJson(val.toObject());
        parts.add(p);
        if (p.getId() >= counters.part) {
            counters.part = p.getId() + 1;
        }
    }
}

void StoragePersistence::saveParts(const Repository<SparePart>& parts) const {
    QJsonArray arr;
    for (const auto& p : parts) {
        arr.append(p.toJson());
    }
    JsonFileHelper::writeAtomic(filepathParts, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void StoragePersistence::loadOrders(Repository<Order>& orders, IdCounters& counters) const {
    orders.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathOrders, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        saveOrders(orders);
        return;
    }
    for (const auto& val : doc.array()) {
        if (!val.isObject()) {
            continue;
        }
        Order o = Order::fromJson(val.toObject());
        if (o.getId() >= counters.order) {
            counters.order = o.getId() + 1;
        }
        orders.add(std::move(o));
    }
}

void StoragePersistence::saveOrders(const Repository<Order>& orders) const {
    QJsonArray arr;
    for (const auto& o : orders) {
        arr.append(o.toJson());
    }
    JsonFileHelper::writeAtomic(filepathOrders, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void StoragePersistence::loadAll(WorkshopCollections collections, IdCounters& counters) const {
    loadAppState(counters);
    loadClients(collections.clients, counters);
    loadMasters(collections.masters, counters);
    loadParts(collections.parts, counters);
    loadOrders(collections.orders, counters);
    saveAppState(counters);
}

void StoragePersistence::saveAll(ConstWorkshopCollections collections, const IdCounters& counters) const {
    saveAppState(counters);
    saveClients(collections.clients);
    saveMasters(collections.masters);
    saveParts(collections.parts);
    saveOrders(collections.orders);
}

QString StoragePersistence::backup(ConstWorkshopCollections collections, const IdCounters& counters, const QString& backupRoot) const {
    saveAll(collections, counters);
    QStringList fileNames = {
        "app_state.json",
        "clients.json",
        "masters.json",
        "parts.json",
        "orders.json"
    };
    return JsonFileHelper::copyBackupSnapshot(dataDirectory, backupRoot, fileNames);
}

void StoragePersistence::restore(WorkshopCollections collections, IdCounters& counters, const QString& backupFolder) const {
    QStringList fileNames = {
        "app_state.json",
        "clients.json",
        "masters.json",
        "parts.json",
        "orders.json"
    };
    JsonFileHelper::restoreBackupSnapshot(backupFolder, dataDirectory, fileNames);
    loadAll(collections, counters);
}
