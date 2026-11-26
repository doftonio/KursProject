#include "../Headers/WorkshopReportService.h"
#include "../Exeptions/EntityNotFoundException.h"
#include <QDateTime>

namespace {
    struct OrderReportContext {
        QString clientName;
        QString clientPhone;
        QString clientNote;
        QString masterName;
        QString masterSpecRu;
        QString deviceCategoryRu;
        QString deviceModel;
        QString deviceSerial;
        QString deviceSpecs;
        QString faultDescription;
        double complexityFactor = 1.0;
    };

    QString mapCategoryToRussian(const QString& rawCategory) {
        if (rawCategory == "Computer") {
            return "Компьютерная техника";
        }
        if (rawCategory == "Household") {
            return "Бытовая техника";
        }
        if (rawCategory == "Electronics") {
            return "Радиоэлектроника";
        }
        return rawCategory;
    }

    QString formatReceivedTimestamp(const QString& isoTimestamp) {
        QDateTime dt = QDateTime::fromString(isoTimestamp, Qt::ISODate);
        return dt.isValid() ? dt.toString("dd.MM.yyyy HH:mm") : isoTimestamp;
    }

    OrderReportContext buildReportContext(const Storage& storage, const Order& order) {
        OrderReportContext ctx;
        const Client* client = storage.findClientById(order.getClientId());
        const Master* master = storage.findMasterById(order.getMasterId());

        ctx.clientName = client ? client->getName() : "Неизвестный клиент";
        ctx.clientPhone = client ? client->getPhone() : "Н/Д";
        ctx.clientNote = client ? client->getNote() : "";
        ctx.masterName = master ? master->getName() : "Не назначен";
        ctx.masterSpecRu = master ? mapCategoryToRussian(master->getSpecialization()) : "Н/Д";

        if (const Device* dev = order.getDevice(); dev != nullptr) {
            ctx.deviceCategoryRu = mapCategoryToRussian(dev->getCategory());
            ctx.deviceModel = dev->getModel();
            ctx.deviceSerial = dev->getSerialNumber();
            ctx.deviceSpecs = dev->getSpecsSummary();
            ctx.faultDescription = dev->getFaultDescription();
            ctx.complexityFactor = dev->getComplexityFactor();
        } else {
            ctx.deviceCategoryRu = "Н/Д";
            ctx.deviceModel = "Н/Д";
            ctx.deviceSerial = "Н/Д";
            ctx.deviceSpecs = "Н/Д";
            ctx.faultDescription = "Н/Д";
        }
        return ctx;
    }
}

QString WorkshopReportService::generateAcceptanceReceiptHtml(const Storage& storage, unsigned int orderId) {
    const Order* order = storage.findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found for acceptance receipt");
    }
    OrderReportContext ctx = buildReportContext(storage, *order);
    QString formattedDate = formatReceivedTimestamp(order->getDateReceived());
    double estimatedWork = order->getLaborCost() * ctx.complexityFactor;
    QString noteBlock = ctx.clientNote.trimmed().isEmpty()
        ? ""
        : QString("<p><b>Особые отметки при приёме:</b> %1</p>").arg(ctx.clientNote.toHtmlEscaped());

    return QString(
        "<html><head><meta charset='utf-8'><style>"
        "body { font-family: Arial, sans-serif; font-size: 10.5pt; color: #111; }"
        "h1 { font-size: 14pt; text-align: center; margin-bottom: 2px; }"
        ".sub { text-align: center; font-size: 9.5pt; color: #444; margin-top: 0; margin-bottom: 12px; }"
        "table { width: 100%; border-collapse: collapse; margin-top: 8px; margin-bottom: 10px; }"
        "th, td { border: 1px solid #333; padding: 6px 8px; text-align: left; vertical-align: top; }"
        "th { background-color: #f0f2f5; width: 34%; }"
        ".terms { font-size: 9pt; border: 1px solid #666; padding: 8px; background-color: #fafafa; margin-top: 10px; }"
        ".signs { width: 100%; border: none; margin-top: 18px; }"
        ".signs td { border: none; padding: 6px 4px; width: 50%; }"
        ".tear { border-top: 1px dashed #666; margin-top: 18px; padding-top: 8px; font-size: 9pt; }"
        "</style></head><body>"
        "<h1>ПРИЁМНАЯ КВИТАНЦИЯ №%1</h1>"
        "<p class='sub'>Акт приёма-передачи оборудования на диагностику и ремонт (Экземпляр Заказчика)</p>"
        "<table>"
        "<tr><th>Дата и время приёма заказа</th><td>%2</td></tr>"
        "<tr><th>Заказчик (ФИО)</th><td>%3</td></tr>"
        "<tr><th>Контактный телефон</th><td>%4</td></tr>"
        "<tr><th>Категория оборудования</th><td>%5</td></tr>"
        "<tr><th>Наименование / модель изделия</th><td>%6</td></tr>"
        "<tr><th>Заводской серийный номер / IMEI</th><td>%7</td></tr>"
        "<tr><th>Комплектация и параметры при приёме</th><td>%8</td></tr>"
        "<tr><th>Заявленная неисправность</th><td>%9</td></tr>"
        "<tr><th>Ответственный специалист</th><td>%10 (%11)</td></tr>"
        "<tr><th>Предварительная стоимость работ</th><td>%12 BYN (базовая %13 BYN, коэф. сложности x%14)</td></tr>"
        "</table>"
        "%15"
        "<div class='terms'>"
        "<b>Условия приёма и проведения сервисного обслуживания:</b><br>"
        "1. Настоящая приёмная квитанция является официальным подтверждением передачи указанного оборудования в сервисную мастерскую.<br>"
        "2. Оборудование принято в состоянии б/у без проверки внутренних скрытых дефектов; окончательная стоимость и сроки ремонта согласовываются с Заказчиком по результатам первичной диагностики.<br>"
        "3. Сервисный центр не несёт ответственности за сохранность пользовательской информации на внутренних накопителях и картах памяти; Заказчику рекомендуется заблаговременно выполнять резервное копирование данных.<br>"
        "4. Выдача оборудования из ремонта производится строго при предъявлении настоящей приёмной квитанции. В случае утраты квитанции Заказчик обязан незамедлительно письменно уведомить сервисный центр и предъявить документ, удостоверяющий личность."
        "</div>"
        "<table class='signs'>"
        "<tr><td><b>Оборудование сдал (Заказчик):</b><br><br>______________________ / %3 /</td>"
        "<td><b>Оборудование принял (Исполнитель):</b><br><br>______________________ / %10 /</td></tr>"
        "</table>"
        "<div class='tear'>"
        "<b>КОРЕШОК ПРИЁМНОЙ КВИТАНЦИИ №%1</b> | Дата: %2 | Клиент: %3 (%4) | Изделие: %6 (S/N: %7)"
        "</div>"
        "</body></html>"
    )
    .arg(order->getId())
    .arg(formattedDate)
    .arg(ctx.clientName.toHtmlEscaped())
    .arg(ctx.clientPhone.toHtmlEscaped())
    .arg(ctx.deviceCategoryRu.toHtmlEscaped())
    .arg(ctx.deviceModel.toHtmlEscaped())
    .arg(ctx.deviceSerial.toHtmlEscaped())
    .arg(ctx.deviceSpecs.toHtmlEscaped())
    .arg(ctx.faultDescription.toHtmlEscaped())
    .arg(ctx.masterName.toHtmlEscaped())
    .arg(ctx.masterSpecRu.toHtmlEscaped())
    .arg(estimatedWork, 0, 'f', 2)
    .arg(order->getLaborCost(), 0, 'f', 2)
    .arg(ctx.complexityFactor, 0, 'f', 2)
    .arg(noteBlock);
}

QString WorkshopReportService::generateWarrantyActHtml(const Storage& storage, unsigned int orderId) {
    const Order* order = storage.findOrderById(orderId);
    if (!order) {
        throw EntityNotFoundException("Order not found for PDF report");
    }
    OrderReportContext ctx = buildReportContext(storage, *order);

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
        "<p><b>Заказчик:</b> %4 (%5)</p>"
        "<p><b>Ответственный мастер:</b> %6 (%7)</p>"
        "<hr>"
        "<p><b>Категория оборудования:</b> %8</p>"
        "<p><b>Модель:</b> %9 | <b>Серийный номер / IMEI:</b> %10</p>"
        "<p><b>Технические параметры:</b> %11</p>"
        "<p><b>Заявленная неисправность:</b> %12</p>"
        "<p><b>Журнал диагностики:</b><br>%13</p>"
        "<h3>Использованные запасные части</h3>"
        "<table><tr><th>ID</th><th>Наименование</th><th>Кол-во</th><th>Цена за ед.</th><th>Сумма</th></tr>"
        "%14</table>"
        "<h3>Финансовый итог</h3>"
        "<p>Базовая стоимость работ: %15 BYN (коэффициент сложности: x%16)</p>"
        "<p>Стоимость комплектующих: %17 BYN</p>"
        "<p><b>ИТОГО К ОПЛАТЕ: %18 BYN</b></p>"
        "<p><i>Гарантийный срок на выполненные работы и установленные детали составляет 90 календарных дней с даты выдачи.</i></p>"
        "</body></html>"
    )
    .arg(order->getId())
    .arg(formatReceivedTimestamp(order->getDateReceived()))
    .arg(order->getStatusText())
    .arg(ctx.clientName.toHtmlEscaped())
    .arg(ctx.clientPhone.toHtmlEscaped())
    .arg(ctx.masterName.toHtmlEscaped())
    .arg(ctx.masterSpecRu.toHtmlEscaped())
    .arg(ctx.deviceCategoryRu.toHtmlEscaped())
    .arg(ctx.deviceModel.toHtmlEscaped())
    .arg(ctx.deviceSerial.toHtmlEscaped())
    .arg(ctx.deviceSpecs.toHtmlEscaped())
    .arg(ctx.faultDescription.toHtmlEscaped())
    .arg(order->getDiagnosticNotes().toHtmlEscaped().replace("\n", "<br>"))
    .arg(partsRows)
    .arg(order->getLaborCost(), 0, 'f', 2)
    .arg(ctx.complexityFactor, 0, 'f', 2)
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
