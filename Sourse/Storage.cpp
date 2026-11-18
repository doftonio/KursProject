#include "../Headers/Storage.h"
#include "../Headers/JsonFileHelper.h"
#include "../Headers/InputValidator.h"
#include "../Exeptions/CapacityExceededException.h"
#include "../Exeptions/DuplicateEntityException.h"
#include "../Exeptions/EntityNotFoundException.h"
#include "../Exeptions/InvalidInputDataException.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>
#include <ranges>

Storage::Storage(const QString& baseDir)
    : dataDirectory(baseDir),
      filepathApp(baseDir + "/app_state.json"),
      filepathClients(baseDir + "/clients.json"),
      filepathMasters(baseDir + "/masters.json"),
      filepathParts(baseDir + "/parts.json"),
      filepathOrders(baseDir + "/orders.json") {
    JsonFileHelper::ensureDirectory(dataDirectory);
    loadDatabaseFromJson();
}

unsigned int Storage::allocateId(unsigned int& counter) {
    unsigned int assigned = counter++;
    saveAppState();
    return assigned;
}

void Storage::initDefaultMasters() {
    masters.clear();
    masters += Master(1, "Иванов Алексей Петрович", "+375291112233", "Computer");
    masters += Master(2, "Смирнов Дмитрий Олегович", "+375292223344", "Electronics");
    masters += Master(3, "Коваленко Сергей Викторович", "+375333334455", "Household");
    masters += Master(4, "Морозов Андрей Игоревич", "+375444445566", "Electronics");
    if (nextMasterId < 5) {
        nextMasterId = 5;
    }
}

void Storage::loadAppState() {
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathApp, true, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        nextClientId = 1;
        nextMasterId = 5;
        nextOrderId = 1;
        nextPartId = 1;
        saveAppState();
        return;
    }
    QJsonObject obj = doc.object();
    nextClientId = static_cast<unsigned int>(obj["nextClientId"].toInt(1));
    nextMasterId = static_cast<unsigned int>(obj["nextMasterId"].toInt(5));
    nextOrderId = static_cast<unsigned int>(obj["nextOrderId"].toInt(1));
    nextPartId = static_cast<unsigned int>(obj["nextPartId"].toInt(1));
}

void Storage::saveAppState() const {
    QJsonObject obj;
    obj["nextClientId"] = static_cast<int>(nextClientId);
    obj["nextMasterId"] = static_cast<int>(nextMasterId);
    obj["nextOrderId"] = static_cast<int>(nextOrderId);
    obj["nextPartId"] = static_cast<int>(nextPartId);
    JsonFileHelper::writeAtomic(filepathApp, QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

void Storage::loadClients() {
    clients.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathClients, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        saveClients();
        return;
    }
    for (const auto& val : doc.array()) {
        if (val.isObject()) {
            Client c = Client::fromJson(val.toObject());
            clients.add(c);
            if (c.getId() >= nextClientId) {
                nextClientId = c.getId() + 1;
            }
        }
    }
}

void Storage::saveClients() const {
    QJsonArray arr;
    for (const auto& c : clients) {
        arr.append(c.toJson());
    }
    JsonFileHelper::writeAtomic(filepathClients, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void Storage::loadMasters() {
    masters.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathMasters, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        initDefaultMasters();
        saveMasters();
        return;
    }
    bool migrated = false;
    for (const auto& val : doc.array()) {
        if (val.isObject()) {
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
            if (m.getId() >= nextMasterId) {
                nextMasterId = m.getId() + 1;
            }
        }
    }
    if (masters.empty()) {
        initDefaultMasters();
        saveMasters();
    } else if (migrated) {
        saveMasters();
    }
}

void Storage::saveMasters() const {
    QJsonArray arr;
    for (const auto& m : masters) {
        arr.append(m.toJson());
    }
    JsonFileHelper::writeAtomic(filepathMasters, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void Storage::loadParts() {
    parts.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathParts, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        saveParts();
        return;
    }
    for (const auto& val : doc.array()) {
        if (val.isObject()) {
            SparePart p = SparePart::fromJson(val.toObject());
            parts.add(p);
            if (p.getId() >= nextPartId) {
                nextPartId = p.getId() + 1;
            }
        }
    }
}

void Storage::saveParts() const {
    QJsonArray arr;
    for (const auto& p : parts) {
        arr.append(p.toJson());
    }
    JsonFileHelper::writeAtomic(filepathParts, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void Storage::loadOrders() {
    orders.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(filepathOrders, false, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        saveOrders();
        return;
    }
    for (const auto& val : doc.array()) {
        if (val.isObject()) {
            Order o = Order::fromJson(val.toObject());
            if (o.getId() >= nextOrderId) {
                nextOrderId = o.getId() + 1;
            }
            orders.add(std::move(o));
        }
    }
}

void Storage::saveOrders() const {
    QJsonArray arr;
    for (const auto& o : orders) {
        arr.append(o.toJson());
    }
    JsonFileHelper::writeAtomic(filepathOrders, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void Storage::loadDatabaseFromJson() {
    loadAppState();
    loadClients();
    loadMasters();
    loadParts();
    loadOrders();
    saveAppState();
}

void Storage::saveDatabaseToJson() const {
    saveAppState();
    saveClients();
    saveMasters();
    saveParts();
    saveOrders();
}

QString Storage::backupDatabase(const QString& backupRoot) const {
    saveDatabaseToJson();
    QStringList fileNames = {
        "app_state.json",
        "clients.json",
        "masters.json",
        "parts.json",
        "orders.json"
    };
    return JsonFileHelper::copyBackupSnapshot(dataDirectory, backupRoot, fileNames);
}

void Storage::restoreDatabase(const QString& backupFolder) {
    QStringList fileNames = {
        "app_state.json",
        "clients.json",
        "masters.json",
        "parts.json",
        "orders.json"
    };
    JsonFileHelper::restoreBackupSnapshot(backupFolder, dataDirectory, fileNames);
    loadDatabaseFromJson();
}

int Storage::countActiveOrders() const {
    return static_cast<int>(std::ranges::count_if(orders,
        [](const Order& o) { return o.isActive(); }));
}

void Storage::checkActiveOrdersLimit() const {
    if (countActiveOrders() >= maxActiveOrdersLimit) {
        throw CapacityExceededException("Active orders limit reached (max 30 concurrent repairs)");
    }
}

const Repository<Client>& Storage::getClients() const {
    return clients;
}

const Repository<Master>& Storage::getMasters() const {
    return masters;
}

const Repository<SparePart>& Storage::getParts() const {
    return parts;
}

const Repository<Order>& Storage::getOrders() const {
    return orders;
}

const Client* Storage::findClientById(unsigned int id) const {
    return clients.findById(id);
}

const Master* Storage::findMasterById(unsigned int id) const {
    return masters.findById(id);
}

const Order* Storage::findOrderById(unsigned int id) const {
    return orders.findById(id);
}

SparePart* Storage::findPartBySku(const QString& sku) {
    QString trimmed = sku.trimmed();
    for (auto& part : parts) {
        if (part.getSku().compare(trimmed, Qt::CaseInsensitive) == 0) {
            return &part;
        }
    }
    return nullptr;
}

std::vector<Client> Storage::searchClients(const QString& query) const {
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return clients.findIf([](const Client&) { return true; });
    }
    return clients.findIf([&trimmed](const Client& c) {
        return c.getName().contains(trimmed, Qt::CaseInsensitive) ||
               c.getPhone().contains(trimmed, Qt::CaseInsensitive);
    });
}

std::vector<Master> Storage::searchMasters(const QString& query) const {
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return masters.findIf([](const Master&) { return true; });
    }
    return masters.findIf([&trimmed](const Master& m) {
        return m.getName().contains(trimmed, Qt::CaseInsensitive) ||
               m.getPhone().contains(trimmed, Qt::CaseInsensitive) ||
               m.getSpecialization().contains(trimmed, Qt::CaseInsensitive);
    });
}

std::vector<SparePart> Storage::searchParts(const QString& query) const {
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return parts.findIf([](const SparePart&) { return true; });
    }
    return parts.findIf([&trimmed](const SparePart& p) {
        return p.getSku().contains(trimmed, Qt::CaseInsensitive) ||
               p.getName().contains(trimmed, Qt::CaseInsensitive);
    });
}

std::vector<SparePart> Storage::findAvailableParts() const {
    return parts.findIf([](const SparePart& p) { return p.getQuantity() > 0; });
}

std::vector<Master> Storage::findAvailableMasters(const QString& category) const {
    QString trimmedCat = category.trimmed();
    std::vector<Master> matched = masters.findIf([&trimmedCat](const Master& m) {
        if (!m.canAcceptOrder()) {
            return false;
        }
        if (trimmedCat.isEmpty()) {
            return true;
        }
        return m.getSpecialization().compare(trimmedCat, Qt::CaseInsensitive) == 0;
    });

    if (matched.empty()) {
        matched = masters.findIf([](const Master& m) { return m.canAcceptOrder(); });
    }

    std::ranges::sort(matched, [](const Master& a, const Master& b) {
        return a.getActiveOrdersCount() < b.getActiveOrdersCount();
    });
    return matched;
}

Master* Storage::selectLeastLoadedMaster(const QString& category) {
    std::vector<Master> candidates = findAvailableMasters(category);
    if (candidates.empty()) {
        throw CapacityExceededException("All workshop masters have reached maximum active workload");
    }
    return masters.findById(candidates.front().getId());
}

std::vector<Order> Storage::filterOrders(int statusFilter, const QString& query) const {
    QString trimmed = query.trimmed();
    return orders.findIf([this, statusFilter, &trimmed](const Order& o) {
        if (statusFilter >= 0 && o.getStatusCode() != statusFilter) {
            return false;
        }
        if (trimmed.isEmpty()) {
            return true;
        }
        bool deviceMatch = false;
        if (o.getDevice()) {
            deviceMatch = o.getDevice()->getModel().contains(trimmed, Qt::CaseInsensitive) ||
                          o.getDevice()->getSerialNumber().contains(trimmed, Qt::CaseInsensitive);
        }
        bool clientMatch = false;
        if (const Client* c = findClientById(o.getClientId()); c != nullptr) {
            clientMatch = c->getName().contains(trimmed, Qt::CaseInsensitive) ||
                          c->getPhone().contains(trimmed, Qt::CaseInsensitive);
        }
        bool idMatch = QString::number(o.getId()) == trimmed;
        return deviceMatch || clientMatch || idMatch;
    });
}

Client& Storage::addClient(const QString& name, const QString& phone, const QString& note) {
    if (static_cast<int>(clients.size()) >= maxClientsLimit) {
        throw CapacityExceededException("Client database limit reached (max 500 clients)");
    }
    InputValidator::validateStringNotEmpty(name, "Client Name");
    InputValidator::validatePhone(phone);
    QString normalizedPhone = InputValidator::normalizePhone(phone);

    for (const auto& existing : clients) {
        if (existing.getPhone() == normalizedPhone) {
            throw DuplicateEntityException("Client with this phone number already exists");
        }
    }

    unsigned int newId = allocateId(nextClientId);
    clients.add(Client(newId, InputValidator::sanitizeString(name), normalizedPhone, InputValidator::sanitizeString(note)));
    saveClients();
    return *clients.findById(newId);
}

Master& Storage::addMaster(const QString& name, const QString& phone, const QString& specialization) {
    if (static_cast<int>(masters.size()) >= maxMastersLimit) {
        throw CapacityExceededException("Masters staff limit reached (max 20 masters)");
    }
    InputValidator::validateStringNotEmpty(name, "Master Name");
    InputValidator::validatePhone(phone);
    InputValidator::validateStringNotEmpty(specialization, "Specialization");

    QString normalizedPhone = InputValidator::normalizePhone(phone);
    for (const auto& existing : masters) {
        if (existing.getPhone() == normalizedPhone) {
            throw DuplicateEntityException("Master with this phone number already exists");
        }
    }

    unsigned int newId = allocateId(nextMasterId);
    masters.add(Master(newId, InputValidator::sanitizeString(name), normalizedPhone, InputValidator::sanitizeString(specialization), 0));
    saveMasters();
    return *masters.findById(newId);
}

void Storage::updateMasterSpecialization(unsigned int masterId, const QString& newSpecialization) {
    Master* master = masters.findById(masterId);
    if (!master) {
        throw EntityNotFoundException("Master not found");
    }
    InputValidator::validateStringNotEmpty(newSpecialization, "Specialization");
    master->setSpecialization(InputValidator::sanitizeString(newSpecialization));
    saveMasters();
}

void Storage::removeMaster(unsigned int masterId) {
    const Master* master = masters.findById(masterId);
    if (!master) {
        throw EntityNotFoundException("Master not found");
    }
    if (masters.size() <= 1) {
        throw InvalidInputDataException("Cannot remove the last remaining master in the workshop");
    }
    if (master->getActiveOrdersCount() > 0) {
        throw InvalidInputDataException("Cannot remove master who still has active orders assigned");
    }
    for (const auto& order : orders) {
        if (order.getMasterId() == masterId && order.isActive()) {
            throw InvalidInputDataException("Cannot remove master with active orders in queue");
        }
    }
    masters.removeById(masterId);
    saveMasters();
}

SparePart& Storage::addPart(const QString& sku, const QString& name, int quantity, double price) {
    if (static_cast<int>(parts.size()) >= maxPartsLimit) {
        throw CapacityExceededException("Warehouse nomenclature limit reached (max 200 items)");
    }
    InputValidator::validateSku(sku);
    InputValidator::validateStringNotEmpty(name, "Part Name");
    InputValidator::validatePrice(price);
    if (quantity < 0) {
        throw InvalidInputDataException("Initial part quantity cannot be negative");
    }

    QString trimmedSku = sku.trimmed().toUpper();
    if (findPartBySku(trimmedSku) != nullptr) {
        throw DuplicateEntityException("Part with SKU already exists: " + trimmedSku.toStdString());
    }

    unsigned int newId = allocateId(nextPartId);
    parts.add(SparePart(newId, trimmedSku, InputValidator::sanitizeString(name), quantity, price));
    saveParts();
    return *parts.findById(newId);
}

void Storage::restockPart(unsigned int partId, int addCount) {
    SparePart* part = parts.findById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    *part += addCount;
    saveParts();
}

Order& Storage::addOrder(unsigned int clientId, unsigned int masterId, std::unique_ptr<Device> device, double laborCost) {
    if (static_cast<int>(orders.size()) >= maxTotalOrdersLimit) {
        throw CapacityExceededException("Total orders archive limit reached (max 1000 orders)");
    }
    checkActiveOrdersLimit();

    if (!findClientById(clientId)) {
        throw EntityNotFoundException("Selected client does not exist");
    }
    if (!device) {
        throw InvalidInputDataException("Device details are required for order");
    }
    InputValidator::validateOrderInput(device->getModel(), device->getSerialNumber(), device->getFaultDescription(), laborCost);

    Master* assignedMaster = nullptr;
    if (masterId == 0) {
        assignedMaster = selectLeastLoadedMaster(device->getCategory());
    } else {
        assignedMaster = masters.findById(masterId);
        if (!assignedMaster) {
            throw EntityNotFoundException("Selected master does not exist");
        }
        if (!assignedMaster->canAcceptOrder()) {
            throw CapacityExceededException("Selected master has reached maximum active workload");
        }
    }

    unsigned int newId = allocateId(nextOrderId);
    Order newOrder(newId, clientId, assignedMaster->getId(), std::move(device), laborCost);
    newOrder.appendDiagnosticNote("Заказ зарегистрирован и назначен мастеру: " + assignedMaster->getName());
    orders.add(std::move(newOrder));
    assignedMaster->incrementOrders();

    saveMasters();
    saveOrders();
    return *orders.findById(newId);
}

void Storage::changeOrderStatus(unsigned int orderId, OrderStatus newStatus) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    bool wasActive = order->isActive();
    order->updateStatus(newStatus);

    if (bool isNowActive = order->isActive(); wasActive && !isNowActive) {
        if (Master* master = masters.findById(order->getMasterId()); master != nullptr) {
            master->decrementOrders();
            saveMasters();
        }
    }
    saveOrders();
}

void Storage::appendOrderNote(unsigned int orderId, const QString& note) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    InputValidator::validateStringNotEmpty(note, "Diagnostic Note");
    order->appendDiagnosticNote(InputValidator::sanitizeString(note));
    saveOrders();
}

void Storage::reservePartForOrder(unsigned int orderId, unsigned int partId, int count) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    SparePart* part = parts.findById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    order->reservePart(*part, count);
    saveParts();
    saveOrders();
}

void Storage::returnPartFromOrder(unsigned int orderId, unsigned int partId, int count) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    SparePart* part = parts.findById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    order->returnPart(*part, count);
    saveParts();
    saveOrders();
}

void Storage::updateOrderLaborCost(unsigned int orderId, double newLaborCost) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    InputValidator::validateLaborCost(newLaborCost);
    order->setLaborCost(newLaborCost);
    saveOrders();
}
