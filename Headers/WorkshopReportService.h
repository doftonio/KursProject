#ifndef WORKSHOPREPORTSERVICE_H
#define WORKSHOPREPORTSERVICE_H

#include "Storage.h"
#include <QString>
#include <QDate>

struct PerformanceReport {
    int totalOrders = 0;
    int activeOrders = 0;
    int completedOrders = 0;
    double totalLaborRevenue = 0.0;
    double totalPartsRevenue = 0.0;
    double totalRevenue = 0.0;
};

class WorkshopReportService {
public:
    static QString generateWarrantyActHtml(const Storage& storage, unsigned int orderId);
    static PerformanceReport generatePerformanceReport(const Storage& storage, const QDate& from, const QDate& to);
};

#endif
