#include "../Headers/WorkshopSearchService.h"
#include <algorithm>
#include <ranges>

namespace {
    bool matchesOrderQuery(const Storage& storage, const Order& order, const QString& trimmedQuery) {
        if (trimmedQuery.isEmpty()) {
            return true;
        }
        bool deviceMatch = false;
        if (order.getDevice()) {
            deviceMatch = order.getDevice()->getModel().contains(trimmedQuery, Qt::CaseInsensitive) ||
                          order.getDevice()->getSerialNumber().contains(trimmedQuery, Qt::CaseInsensitive);
        }
        bool clientMatch = false;
        if (const Client* c = storage.findClientById(order.getClientId()); c != nullptr) {
            clientMatch = c->getName().contains(trimmedQuery, Qt::CaseInsensitive) ||
                          c->getPhone().contains(trimmedQuery, Qt::CaseInsensitive);
        }
        bool idMatch = QString::number(order.getId()) == trimmedQuery;
        return deviceMatch || clientMatch || idMatch;
    }
}

std::vector<Client> WorkshopSearchService::searchClients(const Storage& storage, const QString& query) {
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return storage.getClients().findIf([](const Client&) { return true; });
    }
    return storage.getClients().findIf([&trimmed](const Client& c) {
        return c.getName().contains(trimmed, Qt::CaseInsensitive) ||
               c.getPhone().contains(trimmed, Qt::CaseInsensitive);
    });
}

std::vector<Master> WorkshopSearchService::searchMasters(const Storage& storage, const QString& query) {
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return storage.getMasters().findIf([](const Master&) { return true; });
    }
    return storage.getMasters().findIf([&trimmed](const Master& m) {
        return m.getName().contains(trimmed, Qt::CaseInsensitive) ||
               m.getPhone().contains(trimmed, Qt::CaseInsensitive) ||
               m.getSpecialization().contains(trimmed, Qt::CaseInsensitive);
    });
}

std::vector<SparePart> WorkshopSearchService::searchParts(const Storage& storage, const QString& query) {
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return storage.getParts().findIf([](const SparePart&) { return true; });
    }
    return storage.getParts().findIf([&trimmed](const SparePart& p) {
        return p.getSku().contains(trimmed, Qt::CaseInsensitive) ||
               p.getName().contains(trimmed, Qt::CaseInsensitive);
    });
}

std::vector<SparePart> WorkshopSearchService::findAvailableParts(const Storage& storage) {
    return storage.getParts().findIf([](const SparePart& p) { return p.getQuantity() > 0; });
}

std::vector<Master> WorkshopSearchService::findAvailableMasters(const Storage& storage, const QString& category) {
    QString trimmedCat = category.trimmed();
    std::vector<Master> matched = storage.getMasters().findIf([&trimmedCat](const Master& m) {
        if (!m.canAcceptOrder()) {
            return false;
        }
        if (trimmedCat.isEmpty()) {
            return true;
        }
        return m.getSpecialization().compare(trimmedCat, Qt::CaseInsensitive) == 0;
    });

    if (matched.empty()) {
        matched = storage.getMasters().findIf([](const Master& m) { return m.canAcceptOrder(); });
    }

    std::ranges::sort(matched, [](const Master& a, const Master& b) {
        return a.getActiveOrdersCount() < b.getActiveOrdersCount();
    });
    return matched;
}

std::vector<Order> WorkshopSearchService::filterOrders(const Storage& storage, int statusFilter, const QString& query) {
    QString trimmed = query.trimmed();
    return storage.getOrders().findIf([&storage, statusFilter, &trimmed](const Order& o) {
        if (statusFilter >= 0 && o.getStatusCode() != statusFilter) {
            return false;
        }
        return matchesOrderQuery(storage, o, trimmed);
    });
}

std::vector<Order> WorkshopSearchService::filterCurrentOrders(const Storage& storage, int statusFilter, const QString& query) {
    QString trimmed = query.trimmed();
    return storage.getOrders().findIf([&storage, statusFilter, &trimmed](const Order& o) {
        if (o.isArchived()) {
            return false;
        }
        if (statusFilter >= 0 && o.getStatusCode() != statusFilter) {
            return false;
        }
        return matchesOrderQuery(storage, o, trimmed);
    });
}

std::vector<Order> WorkshopSearchService::filterArchivedOrders(const Storage& storage, const QString& query) {
    QString trimmed = query.trimmed();
    return storage.getOrders().findIf([&storage, &trimmed](const Order& o) {
        if (!o.isArchived()) {
            return false;
        }
        return matchesOrderQuery(storage, o, trimmed);
    });
}
