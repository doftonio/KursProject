#include "../Headers/Storage.h"
#include "../Headers/InputValidator.h"
#include "../Exeptions/CapacityExceededException.h"
#include "../Exeptions/DuplicateEntityException.h"
#include "../Exeptions/EntityNotFoundException.h"
#include "../Exeptions/FileStorageException.h"
#include "../Exeptions/InvalidInputDataException.h"
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <algorithm>

Storage::Storage(const QString& baseDir)
    : dataDirectory(baseDir),
    filepathApp(baseDir + "/app_state.json"),
    filepathClients(baseDir + "/clients.json"),
    filepathMasters(baseDir + "/masters.json"),
    filepathParts(baseDir + "/parts.json"),
    filepathOrders(baseDir + "/orders.json"),
    nextClientId(1),
    nextMasterId(5),
    nextOrderId(1),
    nextPartId(1) {
    QDir dir;
    if (!dir.exists(dataDirectory) && !dir.mkpath(dataDirectory)) {
        throw FileStorageException("Cannot create data directory: " + QFileInfo(dataDirectory).absoluteFilePath().toStdString());
    }
    loadDatabaseFromJson();
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

void Storage::writeJsonAtomic(const QString& path, const QByteArray& content) const {
    QSaveFile saveFile(path);
    if (saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (saveFile.write(content) != -1 && saveFile.commit()) {
            return;
        }
    }

    QFile directFile(path);
    if (!directFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        throw FileStorageException(
            "Cannot write file: " + QFileInfo(path).absoluteFilePath().toStdString() +
            " (" + directFile.errorString().toStdString() + ")"
        );
    }
    if (directFile.write(content) == -1) {
        throw FileStorageException("Failed to write data to file: " + path.toStdString());
    }
    directFile.flush();
    directFile.close();
}

void Storage::loadAppState() {
    QFile file(filepathApp);
    if (!file.exists()) {
        nextClientId = 1;
        nextMasterId = 5;
        nextOrderId = 1;
        nextPartId = 1;
        saveAppState();
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileStorageException("Cannot open app state file: " + filepathApp.toStdString());
    }
    QByteArray raw = file.readAll().trimmed();
    if (raw.isEmpty()) {
        nextClientId = 1;
        nextMasterId = 5;
        nextOrderId = 1;
        nextPartId = 1;
        saveAppState();
        return;
    }
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        throw FileStorageException("Corrupted app_state.json format: " + parseErr.errorString().toStdString());
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
    writeJsonAtomic(filepathApp, QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

void Storage::loadClients() {
    clients.clear();
    QFile file(filepathClients);
    if (!file.exists()) {
        saveClients();
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileStorageException("Cannot open clients.json: " + file.errorString().toStdString());
    }
    QByteArray raw = file.readAll().trimmed();
    if (raw.isEmpty()) {
        saveClients();
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        throw FileStorageException("Corrupted clients.json format: " + err.errorString().toStdString());
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
    writeJsonAtomic(filepathClients, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void Storage::loadMasters() {
    masters.clear();
    QFile file(filepathMasters);
    if (!file.exists()) {
        initDefaultMasters();
        saveMasters();
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileStorageException("Cannot open masters.json: " + file.errorString().toStdString());
    }
    QByteArray raw = file.readAll().trimmed();
    if (raw.isEmpty()) {
        initDefaultMasters();
        saveMasters();
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        throw FileStorageException("Corrupted masters.json format: " + err.errorString().toStdString());
    }
    bool migrated = false;
    for (const auto& val : doc.array()) {
        if (val.isObject()) {
            Master m = Master::fromJson(val.toObject());
            if (m.getName() == "Ivanov Alexey Petrovich") {
                m.setName("Иванов Алексей Петрович");
                migrated = true;
            }
            else if (m.getName() == "Smirnov Dmitry Olegovich") {
                m.setName("Смирнов Дмитрий Олегович");
                migrated = true;
            }
            else if (m.getName() == "Kovalenko Sergey Viktorovich") {
                m.setName("Коваленко Сергей Викторович");
                migrated = true;
            }
            else if (m.getName() == "Morozov Andrey Igorevich") {
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
    }
    else if (migrated) {
        saveMasters();
    }
}

void Storage::saveMasters() const {
    QJsonArray arr;
    for (const auto& m : masters) {
        arr.append(m.toJson());
    }
    writeJsonAtomic(filepathMasters, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void Storage::loadParts() {
    parts.clear();
    QFile file(filepathParts);
    if (!file.exists()) {
        saveParts();
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileStorageException("Cannot open parts.json: " + file.errorString().toStdString());
    }
    QByteArray raw = file.readAll().trimmed();
    if (raw.isEmpty()) {
        saveParts();
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        throw FileStorageException("Corrupted parts.json format: " + err.errorString().toStdString());
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
    writeJsonAtomic(filepathParts, QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void Storage::loadOrders() {
    orders.clear();
    QFile file(filepathOrders);
    if (!file.exists()) {
        saveOrders();
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileStorageException("Cannot open orders.json: " + file.errorString().toStdString());
    }
    QByteArray raw = file.readAll().trimmed();
    if (raw.isEmpty()) {
        saveOrders();
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        throw FileStorageException("Corrupted orders.json format: " + err.errorString().toStdString());
    }
    for (const auto& val : doc.array()) {
        if (val.isObject()) {
            Order o = Order::fromJson(val.toObject());
            orders.add(std::move(o));
            if (o.getId() >= nextOrderId) {
                nextOrderId = o.getId() + 1;
            }
        }
    }
}

void Storage::saveOrders() const {
    QJsonArray arr;
    for (const auto& o : orders) {
        arr.append(o.toJson());
    }
    writeJsonAtomic(filepathOrders, QJsonDocument(arr).toJson(QJsonDocument::Indented));
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
    QString stamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString targetFolder = backupRoot + "/backup_" + stamp;
    QDir dir;
    if (!dir.mkpath(targetFolder)) {
        throw FileStorageException("Unable to create backup directory: " + targetFolder.toStdString());
    }
    QStringList filesToCopy = {
        "app_state.json",
        "clients.json",
        "masters.json",
        "parts.json",
        "orders.json"
    };
    for (const auto& fileName : filesToCopy) {
        QString src = dataDirectory + "/" + fileName;
        QString dst = targetFolder + "/" + fileName;
        if (QFile::exists(src)) {
            if (QFile::exists(dst)) {
                QFile::remove(dst);
            }
            if (!QFile::copy(src, dst)) {
                throw FileStorageException("Failed to copy file to backup: " + fileName.toStdString());
            }
        }
    }
    return targetFolder;
}

void Storage::restoreDatabase(const QString& backupFolder) {
    QDir dir(backupFolder);
    if (!dir.exists()) {
        throw FileStorageException("Backup directory does not exist: " + backupFolder.toStdString());
    }
    QStringList filesToRestore = {
        "app_state.json",
        "clients.json",
        "masters.json",
        "parts.json",
        "orders.json"
    };
    for (const auto& fileName : filesToRestore) {
        QString src = backupFolder + "/" + fileName;
        if (!QFile::exists(src)) {
            throw FileStorageException("Missing file in backup archive: " + fileName.toStdString());
        }
        QFile checkFile(src);
        if (!checkFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw FileStorageException("Cannot read backup file: " + fileName.toStdString());
        }
        QJsonParseError err;
        QJsonDocument::fromJson(checkFile.readAll(), &err);
        if (err.error != QJsonParseError::NoError) {
            throw FileStorageException("Corrupted JSON in backup file: " + fileName.toStdString());
        }
    }
    for (const auto& fileName : filesToRestore) {
        QString src = backupFolder + "/" + fileName;
        QString dst = dataDirectory + "/" + fileName;
        if (QFile::exists(dst)) {
            QFile::remove(dst);
        }
        if (!QFile::copy(src, dst)) {
            throw FileStorageException("Failed to restore file: " + fileName.toStdString());
        }
    }
    loadDatabaseFromJson();
}

unsigned int Storage::requestClientId() {
    unsigned int assigned = nextClientId++;
    saveAppState();
    return assigned;
}

unsigned int Storage::requestMasterId() {
    unsigned int assigned = nextMasterId++;
    saveAppState();
    return assigned;
}

unsigned int Storage::requestOrderId() {
    unsigned int assigned = nextOrderId++;
    saveAppState();
    return assigned;
}

unsigned int Storage::requestPartId() {
    unsigned int assigned = nextPartId++;
    saveAppState();
    return assigned;
}

int Storage::countActiveOrders() const {
    return static_cast<int>(std::count_if(orders.cbegin(), orders.cend(),
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

Client* Storage::findClientById(unsigned int id) {
    return clients.findById(id);
}

const Client* Storage::findClientById(unsigned int id) const {
    return clients.findById(id);
}

Master* Storage::findMasterById(unsigned int id) {
    return masters.findById(id);
}

const Master* Storage::findMasterById(unsigned int id) const {
    return masters.findById(id);
}

SparePart* Storage::findPartById(unsigned int id) {
    return parts.findById(id);
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

Order* Storage::findOrderById(unsigned int id) {
    return orders.findById(id);
}

const Order* Storage::findOrderById(unsigned int id) const {
    return orders.findById(id);
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

    std::sort(matched.begin(), matched.end(), [](const Master& a, const Master& b) {
        return a.getActiveOrdersCount() < b.getActiveOrdersCount();
        });
    return matched;
}

Master* Storage::selectLeastLoadedMaster(const QString& category) {
    std::vector<Master> candidates = findAvailableMasters(category);
    if (candidates.empty()) {
        throw CapacityExceededException("All workshop masters have reached maximum active workload");
    }
    return findMasterById(candidates.front().getId());
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
        const Client* c = findClientById(o.getClientId());
        if (c) {
            clientMatch = c->getName().contains(trimmed, Qt::CaseInsensitive) ||
                c->getPhone().contains(trimmed, Qt::CaseInsensitive);
        }
        bool idMatch = QString::number(o.getId()) == trimmed;
        return deviceMatch || clientMatch || idMatch;
        });
}

std::vector<Order> Storage::findOrdersByClient(unsigned int clientId) const {
    return orders.findIf([clientId](const Order& o) { return o.getClientId() == clientId; });
}

std::vector<Order> Storage::findOrdersByMaster(unsigned int masterId) const {
    return orders.findIf([masterId](const Order& o) { return o.getMasterId() == masterId; });
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

    unsigned int newId = requestClientId();
    clients.add(Client(newId, InputValidator::sanitizeString(name), normalizedPhone, InputValidator::sanitizeString(note)));
    saveClients();
    return *findClientById(newId);
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

    unsigned int newId = requestMasterId();
    masters.add(Master(newId, InputValidator::sanitizeString(name), normalizedPhone, InputValidator::sanitizeString(specialization), 0));
    saveMasters();
    return *findMasterById(newId);
}

void Storage::updateMasterSpecialization(unsigned int masterId, const QString& newSpecialization) {
    Master* master = findMasterById(masterId);
    if (!master) {
        throw EntityNotFoundException("Master not found");
    }
    InputValidator::validateStringNotEmpty(newSpecialization, "Specialization");
    master->setSpecialization(InputValidator::sanitizeString(newSpecialization));
    saveMasters();
}

void Storage::removeMaster(unsigned int masterId) {
    Master* master = findMasterById(masterId);
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

    unsigned int newId = requestPartId();
    parts.add(SparePart(newId, trimmedSku, InputValidator::sanitizeString(name), quantity, price));
    saveParts();
    return *findPartById(newId);
}

void Storage::restockPart(unsigned int partId, int addCount) {
    SparePart* part = findPartById(partId);
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
    }
    else {
        assignedMaster = findMasterById(masterId);
        if (!assignedMaster) {
            throw EntityNotFoundException("Selected master does not exist");
        }
        if (!assignedMaster->canAcceptOrder()) {
            throw CapacityExceededException("Selected master has reached maximum active workload");
        }
    }

    unsigned int newId = requestOrderId();
    Order newOrder(newId, clientId, assignedMaster->getId(), std::move(device), laborCost);
    newOrder.appendDiagnosticNote("Заказ зарегистрирован и назначен мастеру: " + assignedMaster->getName());
    orders.add(std::move(newOrder));
    assignedMaster->incrementOrders();

    saveMasters();
    saveOrders();
    return *findOrderById(newId);
}

void Storage::changeOrderStatus(unsigned int orderId, OrderStatus newStatus) {
    Order* order = findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    bool wasActive = order->isActive();
    order->updateStatus(newStatus);
    bool isNowActive = order->isActive();

    if (wasActive && !isNowActive) {
        Master* master = findMasterById(order->getMasterId());
        if (master) {
            master->decrementOrders();
            saveMasters();
        }
    }
    saveOrders();
}

void Storage::appendOrderNote(unsigned int orderId, const QString& note) {
    Order* order = findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    InputValidator::validateStringNotEmpty(note, "Diagnostic Note");
    order->appendDiagnosticNote(InputValidator::sanitizeString(note));
    saveOrders();
}

void Storage::reservePartForOrder(unsigned int orderId, unsigned int partId, int count) {
    Order* order = findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    SparePart* part = findPartById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    order->reservePart(*part, count);
    saveParts();
    saveOrders();
}

void Storage::returnPartFromOrder(unsigned int orderId, unsigned int partId, int count) {
    Order* order = findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    SparePart* part = findPartById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    order->returnPart(*part, count);
    saveParts();
    saveOrders();
}

void Storage::updateOrderLaborCost(unsigned int orderId, double newLaborCost) {
    Order* order = findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    InputValidator::validateLaborCost(newLaborCost);
    order->setLaborCost(newLaborCost);
    saveOrders();
}

QString Storage::generateWarrantyActHtml(unsigned int orderId) const {
    const Order* order = findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found for PDF report");
    }
    const Client* client = findClientById(order->getClientId());
    const Master* master = findMasterById(order->getMasterId());

    QString clientInfo = client ? QString("%1 (%2)").arg(client->getName(), client->getPhone()) : "Неизвестный клиент";
    QString masterInfo = master ? QString("%1 (%2)").arg(master->getName(), master->getSpecialization()) : "Не назначен";
    QString deviceModel = order->getDevice() ? order->getDevice()->getModel() : "Н/Д";
    QString deviceSerial = order->getDevice() ? order->getDevice()->getSerialNumber() : "Н/Д";
    QString deviceCategory = order->getDevice() ? order->getDevice()->getCategory() : "Н/Д";
    if (deviceCategory == "Computer") {
        deviceCategory = "Компьютерная техника";
    }
    else if (deviceCategory == "Household") {
        deviceCategory = "Бытовая техника";
    }
    else if (deviceCategory == "Electronics") {
        deviceCategory = "Радиоэлектроника";
    }

    QString deviceSpecs = order->getDevice() ? order->getDevice()->getSpecsSummary() : "Н/Д";
    QString fault = order->getDevice() ? order->getDevice()->getFaultDescription() : "Н/Д";
    double factor = order->getDevice() ? order->getDevice()->getComplexityFactor() : 1.0;

    QString partsRows;
    for (const auto& item : order->getUsedParts()) {
        partsRows += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4 BYN</td><td>%5 BYN</td></tr>")
            .arg(item.getPartId())
            .arg(item.getPartName().toHtmlEscaped())
            .arg(item.getQuantity())
            .arg(item.getPriceAtSale(), 0, 'f', 2)
            .arg(item.getSubtotal(), 0, 'f', 2);
    }
    if (partsRows.isEmpty()) {
        partsRows = "<tr><td colspan='5'>Запасные части не использовались</td></tr>";
    }

    QString html = QString(
        "<html><head><meta charset='utf-8'><style>"
        "body { font-family: Arial, sans-serif; font-size: 11pt; }"
        "h1 { font-size: 15pt; text-align: center; }"
        "table { width: 100%; border-collapse: collapse; margin-top: 10px; }"
        "th, td { border: 1px solid #333; padding: 6px; text-align: left; }"
        "</style></head><body>"
        "<h1>РАСЧЁТНО-ГАРАНТИЙНЫЙ АКТ ВЫПОЛНЕННЫХ РАБОТ №%1</h1>"
        "<p><b>Дата приёма:</b> %2</p>"
        "<p><b>Текущий статус:</b> %3</p>"
        "<p><b>Заказчик:</b> %4</p>"
        "<p><b>Ответственный мастер:</b> %5</p>"
        "<hr>"
        "<p><b>Категория оборудования:</b> %6</p>"
        "<p><b>Модель:</b> %7 | <b>Серийный номер / IMEI:</b> %8</p>"
        "<p><b>Технические параметры:</b> %9</p>"
        "<p><b>Заявленная неисправность:</b> %10</p>"
        "<p><b>Журнал диагностики:</b><br>%11</p>"
        "<h3>Использованные запасные части</h3>"
        "<table><tr><th>ID</th><th>Наименование</th><th>Кол-во</th><th>Цена за ед.</th><th>Сумма</th></tr>"
        "%12</table>"
        "<h3>Финансовый итог</h3>"
        "<p>Базовая стоимость работ: %13 BYN (коэффициент сложности: x%14)</p>"
        "<p>Стоимость комплектующих: %15 BYN</p>"
        "<p><b>ИТОГО К ОПЛАТЕ: %16 BYN</b></p>"
        "<p><i>Гарантийный срок на выполненные работы и установленные детали составляет 90 календарных дней с даты выдачи.</i></p>"
        "</body></html>"
    )
        .arg(order->getId())
        .arg(order->getDateReceived())
        .arg(order->getStatusText())
        .arg(clientInfo.toHtmlEscaped())
        .arg(masterInfo.toHtmlEscaped())
        .arg(deviceCategory)
        .arg(deviceModel.toHtmlEscaped())
        .arg(deviceSerial.toHtmlEscaped())
        .arg(deviceSpecs.toHtmlEscaped())
        .arg(fault.toHtmlEscaped())
        .arg(order->getDiagnosticNotes().toHtmlEscaped().replace("\n", "<br>"))
        .arg(partsRows)
        .arg(order->getLaborCost(), 0, 'f', 2)
        .arg(factor, 0, 'f', 2)
        .arg(order->calculatePartsCost(), 0, 'f', 2)
        .arg(order->calculateTotalCost(), 0, 'f', 2);

    return html;
}

PerformanceReport Storage::generatePerformanceReport(const QDate& from, const QDate& to) const {
    PerformanceReport report;
    for (const auto& order : orders) {
        QDate orderDate = QDateTime::fromString(order.getDateReceived(), Qt::ISODate).date();
        if (orderDate.isValid() && (orderDate < from || orderDate > to)) {
            continue;
        }
        ++report.totalOrders;
        if (order.isActive()) {
            ++report.activeOrders;
        }
        else {
            ++report.completedOrders;
        }
        double factor = order.getDevice() ? order.getDevice()->getComplexityFactor() : 1.0;
        double labor = order.getLaborCost() * factor;
        double partsSum = order.calculatePartsCost();
        report.totalLaborRevenue += labor;
        report.totalPartsRevenue += partsSum;
        report.totalRevenue += (labor + partsSum);
    }
    return report;
}