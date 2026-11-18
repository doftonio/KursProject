#include "../Headers/WorkshopReportService.h"
#include "../Exeptions/EntityNotFoundException.h"
#include <QDateTime>

QString WorkshopReportService::generateWarrantyActHtml(const Storage& storage, unsigned int orderId) {
    const Order* order = storage.findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found for PDF report");
    }
    const Client* client = storage.findClientById(order->getClientId());
    const Master* master = storage.findMasterById(order->getMasterId());

    QString clientInfo = client ? QString("%1 (%2)").arg(client->getName(), client->getPhone()) : "Неизвестный клиент";
    QString masterInfo = master ? QString("%1 (%2)").arg(master->getName(), master->getSpecialization()) : "Не назначен";
    QString deviceModel = order->getDevice() ? order->getDevice()->getModel() : "Н/Д";
    QString deviceSerial = order->getDevice() ? order->getDevice()->getSerialNumber() : "Н/Д";
    QString deviceCategory = order->getDevice() ? order->getDevice()->getCategory() : "Н/Д";
    if (deviceCategory == "Computer") {
        deviceCategory = "Компьютерная техника";
    } else if (deviceCategory == "Household") {
        deviceCategory = "Бытовая техника";
    } else if (deviceCategory == "Electronics") {
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

    return QString(
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
}

PerformanceReport WorkshopReportService::generatePerformanceReport(const Storage& storage, const QDate& from, const QDate& to) {
    PerformanceReport report;
    for (const auto& order : storage.getOrders()) {
        if (QDate orderDate = QDateTime::fromString(order.getDateReceived(), Qt::ISODate).date();
            orderDate.isValid() && (orderDate < from || orderDate > to)) {
            continue;
        }
        ++report.totalOrders;
        if (order.isActive()) {
            ++report.activeOrders;
        } else {
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
