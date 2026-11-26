#include "../Headers/Storage.h"
#include "../Headers/WorkshopSearchService.h"
#include "../Headers/InputValidator.h"
#include "../Exeptions/CapacityExceededException.h"
#include "../Exeptions/DuplicateEntityException.h"
#include "../Exeptions/EntityNotFoundException.h"
#include "../Exeptions/InvalidInputDataException.h"
#include <algorithm>
#include <ranges>

Storage::Storage(const QString& baseDir)
    : persistence(baseDir) {
    persistence.ensureStorageReady();
    loadDatabaseFromJson();
}

unsigned int Storage::allocateId(unsigned int& counter) const {
    unsigned int assigned = counter++;
    persistence.saveAppState(counters);
    return assigned;
}

void Storage::ensureOrderEditable(const Order& order) {
    if (order.isArchived()) {
        throw InvalidInputDataException("Archived order (status: Issued) cannot be edited");
    }
}

void Storage::loadDatabaseFromJson() {
    persistence.loadAll({clients, masters, parts, orders}, counters);
}

void Storage::saveDatabaseToJson() const {
    persistence.saveAll({clients, masters, parts, orders}, counters);
}

QString Storage::backupDatabase(const QString& backupRoot) const {
    return persistence.backup({clients, masters, parts, orders}, counters, backupRoot);
}

void Storage::restoreDatabase(const QString& backupFolder) {
    persistence.restore({clients, masters, parts, orders}, counters, backupFolder);
}

int Storage::countActiveOrders() const {
    return static_cast<int>(std::ranges::count_if(orders,
        [](const Order& o) { return o.isActive(); }));
}

int Storage::countArchivedOrders() const {
    return static_cast<int>(std::ranges::count_if(orders,
        [](const Order& o) { return o.isArchived(); }));
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

Master* Storage::selectLeastLoadedMaster(const QString& category) {
    std::vector<Master> candidates = WorkshopSearchService::findAvailableMasters(*this, category);
    if (candidates.empty()) {
        throw CapacityExceededException("All workshop masters have reached maximum active workload");
    }
    return masters.findById(candidates.front().getId());
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

    unsigned int newId = allocateId(counters.client);
    clients.add(Client(newId, InputValidator::sanitizeString(name), normalizedPhone, InputValidator::sanitizeString(note)));
    persistence.saveClients(clients);
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

    unsigned int newId = allocateId(counters.master);
    masters.add(Master(newId, InputValidator::sanitizeString(name), normalizedPhone, InputValidator::sanitizeString(specialization), 0));
    persistence.saveMasters(masters);
    return *masters.findById(newId);
}

void Storage::updateMasterSpecialization(unsigned int masterId, const QString& newSpecialization) {
    Master* master = masters.findById(masterId);
    if (!master) {
        throw EntityNotFoundException("Master not found");
    }
    InputValidator::validateStringNotEmpty(newSpecialization, "Specialization");
    master->setSpecialization(InputValidator::sanitizeString(newSpecialization));
    persistence.saveMasters(masters);
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
    persistence.saveMasters(masters);
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

    unsigned int newId = allocateId(counters.part);
    parts.add(SparePart(newId, trimmedSku, InputValidator::sanitizeString(name), quantity, price));
    persistence.saveParts(parts);
    return *parts.findById(newId);
}

void Storage::restockPart(unsigned int partId, int addCount) {
    SparePart* part = parts.findById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    *part += addCount;
    persistence.saveParts(parts);
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

    unsigned int newId = allocateId(counters.order);
    Order newOrder(newId, clientId, assignedMaster->getId(), std::move(device), laborCost);
    newOrder.appendDiagnosticNote("Заказ зарегистрирован и назначен мастеру: " + assignedMaster->getName());
    orders.add(std::move(newOrder));
    assignedMaster->incrementOrders();

    persistence.saveMasters(masters);
    persistence.saveOrders(orders);
    return *orders.findById(newId);
}

void Storage::changeOrderStatus(unsigned int orderId, OrderStatus newStatus) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    ensureOrderEditable(*order);
    bool wasActive = order->isActive();
    order->updateStatus(newStatus);

    if (bool isNowActive = order->isActive(); wasActive && !isNowActive) {
        if (Master* master = masters.findById(order->getMasterId()); master != nullptr) {
            master->decrementOrders();
            persistence.saveMasters(masters);
        }
    }
    persistence.saveOrders(orders);
}

void Storage::appendOrderNote(unsigned int orderId, const QString& note) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    ensureOrderEditable(*order);
    InputValidator::validateStringNotEmpty(note, "Diagnostic Note");
    order->appendDiagnosticNote(InputValidator::sanitizeString(note));
    persistence.saveOrders(orders);
}

void Storage::reservePartForOrder(unsigned int orderId, unsigned int partId, int count) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    ensureOrderEditable(*order);
    SparePart* part = parts.findById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    order->reservePart(*part, count);
    persistence.saveParts(parts);
    persistence.saveOrders(orders);
}

void Storage::returnPartFromOrder(unsigned int orderId, unsigned int partId, int count) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    ensureOrderEditable(*order);
    SparePart* part = parts.findById(partId);
    if (!part) {
        throw EntityNotFoundException("Spare part not found");
    }
    order->returnPart(*part, count);
    persistence.saveParts(parts);
    persistence.saveOrders(orders);
}

void Storage::updateOrderLaborCost(unsigned int orderId, double newLaborCost) {
    Order* order = orders.findById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found");
    }
    ensureOrderEditable(*order);
    InputValidator::validateLaborCost(newLaborCost);
    order->setLaborCost(newLaborCost);
    persistence.saveOrders(orders);
}
