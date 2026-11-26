#ifndef WORKSHOPSEARCHSERVICE_H
#define WORKSHOPSEARCHSERVICE_H

#include "Storage.h"
#include <QString>
#include <vector>

class WorkshopSearchService {
public:
    static std::vector<Client> searchClients(const Storage& storage, const QString& query);
    static std::vector<Master> searchMasters(const Storage& storage, const QString& query);
    static std::vector<SparePart> searchParts(const Storage& storage, const QString& query);
    static std::vector<SparePart> findAvailableParts(const Storage& storage);
    static std::vector<Master> findAvailableMasters(const Storage& storage, const QString& category);
    static std::vector<Order> filterOrders(const Storage& storage, int statusFilter, const QString& query);
    static std::vector<Order> filterCurrentOrders(const Storage& storage, int statusFilter, const QString& query);
    static std::vector<Order> filterArchivedOrders(const Storage& storage, const QString& query);
};

#endif
