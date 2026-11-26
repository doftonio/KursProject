#include "../headers/OrderDialog.h"
#include "../headers/QuickClientDialog.h"
#include "../headers/UiHelpers.h"
#include "../../Headers/WorkshopSearchService.h"
#include "../../Headers/WorkshopReportService.h"
#include "../../Headers/InputValidator.h"
#include "../../Headers/ComputerDevice.h"
#include "../../Headers/HouseholdDevice.h"
#include "../../Headers/ElectronicDevice.h"
#include "../../Exeptions/AppException.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QStyle>
#include <QPalette>
#include <QColor>

namespace {
    QString mapWorkerSpecRu(const QString& spec) {
        if (spec == "Computer") {
            return "Компьютерная техника";
        }
        if (spec == "Household") {
            return "Бытовая техника";
        }
        if (spec == "Electronics") {
            return "Радиоэлектроника";
        }
        return spec;
    }
}

OrderDialog::OrderDialog(Storage& storageRef, unsigned int existingOrderId, QWidget* parent)
    : QDialog(parent),
      storage(storageRef),
      orderId(existingOrderId),
      editMode(existingOrderId > 0) {
    buildUi();
    populateCombos();
    if (editMode) {
        loadExistingOrder();
    } else {
        onCategoryChanged(0);
        refreshTotalLabel();
    }
}

void OrderDialog::markFieldLocked(QWidget* widget) {
    if (!widget) {
        return;
    }
    widget->setProperty("lockedField", true);
    QPalette pal = widget->palette();
    const QColor mutedGray("#7A869A");
    pal.setColor(QPalette::Active, QPalette::Text, mutedGray);
    pal.setColor(QPalette::Inactive, QPalette::Text, mutedGray);
    pal.setColor(QPalette::Disabled, QPalette::Text, mutedGray);
    pal.setColor(QPalette::Active, QPalette::WindowText, mutedGray);
    pal.setColor(QPalette::Inactive, QPalette::WindowText, mutedGray);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, mutedGray);
    pal.setColor(QPalette::Active, QPalette::ButtonText, mutedGray);
    pal.setColor(QPalette::Inactive, QPalette::ButtonText, mutedGray);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, mutedGray);
    widget->setPalette(pal);

    if (QStyle* widgetStyle = widget->style(); widgetStyle != nullptr) {
        widgetStyle->unpolish(widget);
        widgetStyle->polish(widget);
    }
}

QFrame* OrderDialog::createSectionOverlay(QWidget* targetGroup) {
    auto* overlay = makeOwned<QFrame>(targetGroup);
    overlay->setObjectName("lockedOverlay");
    overlay->setGeometry(targetGroup->rect());
    overlay->raise();
    overlay->show();
    targetGroup->installEventFilter(this);
    return overlay;
}

void OrderDialog::setupLockedOverlays() {
    panels.notesOverlay = createSectionOverlay(panels.notesGroup);
    panels.partsOverlay = createSectionOverlay(panels.partsGroup);
}

bool OrderDialog::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() != QEvent::Resize && event->type() != QEvent::Show) {
        return QDialog::eventFilter(watched, event);
    }
    if (watched == panels.notesGroup && panels.notesOverlay != nullptr) {
        panels.notesOverlay->setGeometry(panels.notesGroup->rect());
        panels.notesOverlay->raise();
    } else if (watched == panels.partsGroup && panels.partsOverlay != nullptr) {
        panels.partsOverlay->setGeometry(panels.partsGroup->rect());
        panels.partsOverlay->raise();
    }
    return QDialog::eventFilter(watched, event);
}

void OrderDialog::buildUi() {
    setWindowTitle(editMode ? QString("Карточка заказа №%1").arg(orderId) : "Регистрация нового заказа");
    resize(820, 735);

    auto* mainLayout = makeOwned<QVBoxLayout>(this);
    auto* formLayout = makeOwned<QFormLayout>();

    form.client = makeOwned<QComboBox>(this);
    form.worker = makeOwned<QComboBox>(this);
    form.category = makeOwned<QComboBox>(this);
    form.category->addItem("Компьютерная техника", "Computer");
    form.category->addItem("Бытовая техника", "Household");
    form.category->addItem("Радиоэлектроника", "Electronics");

    form.model = makeOwned<QLineEdit>(this);
    form.model->setPlaceholderText("Например: ASUS VivoBook 15 / Bosch KGN39");
    form.serial = makeOwned<QLineEdit>(this);
    form.serial->setPlaceholderText("Необязательно (если отсутствует — запишется «Не указан»)");
    form.extraSpec = makeOwned<QLineEdit>(this);
    form.extraFlag = makeOwned<QCheckBox>(this);
    form.fault = makeOwned<QTextEdit>(this);
    form.fault->setPlaceholderText("Опишите симптомы неисправности со слов клиента...");
    form.fault->setMaximumHeight(65);

    statusCombo = makeOwned<QComboBox>(this);
    statusCombo->addItem("Принят", 0);
    statusCombo->addItem("Диагностика", 1);
    statusCombo->addItem("Ожидание запчастей", 2);
    statusCombo->addItem("Завершён", 3);
    statusCombo->addItem("Выдан (в архив)", 4);

    laborSpin = makeOwned<QDoubleSpinBox>(this);
    laborSpin->setRange(0.0, 10000.0);
    laborSpin->setDecimals(2);
    laborSpin->setSuffix(" BYN");

    formLayout->addRow(editMode ? "Заказчик:" : "Заказчик *:", form.client);
    formLayout->addRow("Ответственный работник:", form.worker);
    formLayout->addRow(editMode ? "Категория техники:" : "Категория техники *:", form.category);
    formLayout->addRow(editMode ? "Модель устройства:" : "Модель устройства *:", form.model);
    formLayout->addRow("Серийный номер / IMEI:", form.serial);
    formLayout->addRow("Технический параметр:", form.extraSpec);
    formLayout->addRow("Комплектация / опции:", form.extraFlag);
    formLayout->addRow(editMode ? "Заявленная неисправность:" : "Заявленная неисправность *:", form.fault);
    formLayout->addRow("Статус заказа:", statusCombo);
    formLayout->addRow("Базовая стоимость работ:", laborSpin);

    mainLayout->addLayout(formLayout);

    if (!editMode) {
        auto* requiredHintLabel = makeOwned<QLabel>("* Обязательные поля для приёма заказа. Остальные параметры можно уточнить после диагностики.", this);
        requiredHintLabel->setWordWrap(true);
        mainLayout->addWidget(requiredHintLabel);
    }

    panels.notesGroup = makeOwned<QGroupBox>("Журнал диагностики и заметки работника", this);
    auto* notesLayout = makeOwned<QVBoxLayout>(panels.notesGroup);
    panels.notesHistory = makeOwned<QTextEdit>(this);
    panels.notesHistory->setReadOnly(true);
    panels.notesHistory->setMaximumHeight(85);
    auto* addNoteRow = makeOwned<QHBoxLayout>();
    panels.newNote = makeOwned<QLineEdit>(this);
    panels.newNote->setPlaceholderText("Введите технологическую заметку...");
    auto* addNoteBtn = makeOwned<QPushButton>("Добавить заметку", this);
    addNoteRow->addWidget(panels.newNote, 1);
    addNoteRow->addWidget(addNoteBtn);
    notesLayout->addWidget(panels.notesHistory);
    notesLayout->addLayout(addNoteRow);
    mainLayout->addWidget(panels.notesGroup);

    panels.partsGroup = makeOwned<QGroupBox>("Складские запасные части в заказе", this);
    auto* partsLayout = makeOwned<QVBoxLayout>(panels.partsGroup);
    auto* partControlRow = makeOwned<QHBoxLayout>();
    panels.availableParts = makeOwned<QComboBox>(this);
    panels.partQty = makeOwned<QSpinBox>(this);
    panels.partQty->setRange(1, 200);
    auto* reservePartBtn = makeOwned<QPushButton>("Списать в заказ", this);
    auto* returnPartBtn = makeOwned<QPushButton>("Вернуть на склад", this);

    partControlRow->addWidget(panels.availableParts, 1);
    partControlRow->addWidget(panels.partQty);
    partControlRow->addWidget(reservePartBtn);
    partControlRow->addWidget(returnPartBtn);

    panels.usedParts = makeOwned<QTableWidget>(this);
    panels.usedParts->setColumnCount(5);
    panels.usedParts->setHorizontalHeaderLabels({"ID", "Наименование", "Кол-во", "Цена (BYN)", "Сумма (BYN)"});
    panels.usedParts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    panels.usedParts->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    panels.usedParts->setAlternatingRowColors(true);
    panels.usedParts->setSelectionBehavior(QAbstractItemView::SelectRows);
    panels.usedParts->setEditTriggers(QAbstractItemView::NoEditTriggers);

    partsLayout->addLayout(partControlRow);
    partsLayout->addWidget(panels.usedParts);
    mainLayout->addWidget(panels.partsGroup);

    auto* bottomRow = makeOwned<QHBoxLayout>();
    totalCostLabel = makeOwned<QLabel>("Итоговая стоимость: 0.00 BYN", this);
    totalCostLabel->setObjectName("totalHighlight");
    receiptBtn = makeOwned<QPushButton>("Квитанция приёма (PDF)", this);
    receiptBtn->setEnabled(editMode);
    saveBtn = makeOwned<QPushButton>("Сохранить", this);
    saveBtn->setObjectName("primaryBtn");
    auto* cancelBtn = makeOwned<QPushButton>("Закрыть", this);

    bottomRow->addWidget(totalCostLabel, 1);
    bottomRow->addWidget(receiptBtn);
    bottomRow->addWidget(saveBtn);
    bottomRow->addWidget(cancelBtn);
    mainLayout->addLayout(bottomRow);

    if (!editMode) {
        statusCombo->setEnabled(false);
        markFieldLocked(statusCombo);
        panels.notesGroup->setEnabled(false);
        panels.partsGroup->setEnabled(false);
        setupLockedOverlays();
    } else {
        form.client->setEnabled(false);
        form.worker->setEnabled(false);
        form.category->setEnabled(false);
        form.model->setReadOnly(true);
        form.serial->setReadOnly(true);
        form.extraSpec->setReadOnly(true);
        form.extraFlag->setEnabled(false);
        form.fault->setReadOnly(true);

        markFieldLocked(form.client);
        markFieldLocked(form.worker);
        markFieldLocked(form.category);
        markFieldLocked(form.model);
        markFieldLocked(form.serial);
        markFieldLocked(form.extraSpec);
        markFieldLocked(form.extraFlag);
        markFieldLocked(form.fault);
        markFieldLocked(panels.notesHistory);
    }

    connect(form.client, QOverload<int>::of(&QComboBox::activated), this, &OrderDialog::onClientComboActivated);
    connect(form.category, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrderDialog::onCategoryChanged);
    connect(addNoteBtn, &QPushButton::clicked, this, &OrderDialog::onAddNote);
    connect(reservePartBtn, &QPushButton::clicked, this, &OrderDialog::onReservePart);
    connect(returnPartBtn, &QPushButton::clicked, this, &OrderDialog::onReturnPart);
    connect(receiptBtn, &QPushButton::clicked, this, &OrderDialog::onExportReceipt);
    connect(saveBtn, &QPushButton::clicked, this, &OrderDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(laborSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        refreshTotalLabel();
    });
}

void OrderDialog::populateCombos() {
    const auto previousClientId = form.client->currentData().toUInt();
    form.client->clear();
    if (!editMode) {
        form.client->addItem("+ Добавить нового клиента...", 0U);
    }
    for (const auto& c : storage.getClients()) {
        form.client->addItem(QString("%1 (%2)").arg(c.getName(), c.getPhone()), c.getId());
    }
    if (!editMode) {
        if (const auto idx = form.client->findData(previousClientId); previousClientId > 0 && idx >= 0) {
            form.client->setCurrentIndex(idx);
        } else {
            form.client->setCurrentIndex(form.client->count() > 1 ? 1 : 0);
        }
    }

    form.worker->clear();
    if (!editMode) {
        form.worker->addItem("Автоназначение (наименее загруженный работник)", 0);
    }
    for (const auto& m : storage.getMasters()) {
        form.worker->addItem(
            QString("Работник №%1: %2 [%3] (в работе: %4)")
                .arg(m.getId())
                .arg(m.getName(), mapWorkerSpecRu(m.getSpecialization()))
                .arg(m.getActiveOrdersCount()),
            m.getId()
        );
    }

    panels.availableParts->clear();
    for (const auto& p : WorkshopSearchService::findAvailableParts(storage)) {
        panels.availableParts->addItem(
            QString("[%1] %2 (Остаток: %3 шт., %4 BYN)").arg(p.getSku(), p.getName()).arg(p.getQuantity()).arg(p.getPrice(), 0, 'f', 2),
            p.getId()
        );
    }
}

void OrderDialog::onClientComboActivated(int index) {
    if (editMode || index < 0 || form.client->itemData(index).toUInt() != 0U) {
        return;
    }

    QuickClientDialog quickClientDlg(storage, this);
    quickClientDlg.exec();
    const auto newlyAddedId = quickClientDlg.getLastAddedClientId();
    populateCombos();

    if (newlyAddedId > 0) {
        if (const auto newIdx = form.client->findData(newlyAddedId); newIdx >= 0) {
            form.client->setCurrentIndex(newIdx);
        }
    }
}

void OrderDialog::applyReadOnlyArchiveMode() {
    setWindowTitle(QString("Архивный заказ №%1 (выдан, только просмотр)").arg(orderId));
    statusCombo->setEnabled(false);
    laborSpin->setReadOnly(true);
    panels.newNote->setEnabled(false);
    panels.newNote->setPlaceholderText("");
    panels.availableParts->setEnabled(false);
    panels.partQty->setEnabled(false);
    saveBtn->setVisible(false);

    markFieldLocked(statusCombo);
    markFieldLocked(laborSpin);
    markFieldLocked(panels.newNote);
    markFieldLocked(panels.availableParts);
    markFieldLocked(panels.partQty);

    const auto buttons = findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        const auto text = btn->text();
        if (text == "Добавить заметку" || text == "Списать в заказ" || text == "Вернуть на склад") {
            btn->setEnabled(false);
        }
    }
}

void OrderDialog::loadExistingOrder() {
    const Order* order = storage.findOrderById(orderId);
    if (!order) {
        return;
    }

    if (const auto clientIdx = form.client->findData(order->getClientId()); clientIdx >= 0) {
        form.client->setCurrentIndex(clientIdx);
    }
    if (const auto masterIdx = form.worker->findData(order->getMasterId()); masterIdx >= 0) {
        form.worker->setCurrentIndex(masterIdx);
    }

    if (const auto* dev = order->getDevice(); dev != nullptr) {
        if (const auto catIdx = form.category->findData(dev->getCategory()); catIdx >= 0) {
            form.category->setCurrentIndex(catIdx);
        }
        form.model->setText(dev->getModel());
        form.serial->setText(dev->getSerialNumber());
        form.fault->setPlainText(dev->getFaultDescription());
        form.extraSpec->setText(dev->getSpecsSummary());
    }

    if (const auto statusIdx = statusCombo->findData(order->getStatusCode()); statusIdx >= 0) {
        statusCombo->setCurrentIndex(statusIdx);
    }
    laborSpin->setValue(order->getLaborCost());
    panels.notesHistory->setPlainText(order->getDiagnosticNotes());

    refreshUsedPartsTable();
    refreshTotalLabel();

    if (order->isArchived()) {
        applyReadOnlyArchiveMode();
    }
}

void OrderDialog::refreshUsedPartsTable() {
    const Order* order = storage.findOrderById(orderId);
    if (!order) {
        return;
    }
    const auto& items = order->getUsedParts();
    panels.usedParts->setRowCount(static_cast<int>(items.size()));
    for (int row = 0; row < static_cast<int>(items.size()); ++row) {
        const auto& item = items[static_cast<std::size_t>(row)];
        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(item.getPartId()));
        idItem->setData(Qt::UserRole, item.getPartId());
        panels.usedParts->setItem(row, 0, idItem);
        panels.usedParts->setItem(row, 1, makeOwned<QTableWidgetItem>(item.getPartName()));
        panels.usedParts->setItem(row, 2, makeOwned<QTableWidgetItem>(QString::number(item.getQuantity())));
        panels.usedParts->setItem(row, 3, makeOwned<QTableWidgetItem>(QString::number(item.getPriceAtSale(), 'f', 2)));
        panels.usedParts->setItem(row, 4, makeOwned<QTableWidgetItem>(QString::number(item.getSubtotal(), 'f', 2)));
    }
}

void OrderDialog::refreshTotalLabel() {
    if (!editMode) {
        totalCostLabel->setText(QString("Базовая стоимость работ: %1 BYN").arg(laborSpin->value(), 0, 'f', 2));
        return;
    }
    const Order* order = storage.findOrderById(orderId);
    if (!order) {
        return;
    }
    const double factor = order->getDevice() ? order->getDevice()->getComplexityFactor() : 1.0;
    const double total = (laborSpin->value() * factor) + order->calculatePartsCost();
    totalCostLabel->setText(QString("Итоговая стоимость: %1 BYN (коэф. сложности x%2)").arg(total, 0, 'f', 2).arg(factor, 0, 'f', 2));
}

void OrderDialog::onCategoryChanged(int index) {
    if (editMode) {
        return;
    }
    if (index == 0) {
        form.extraSpec->setPlaceholderText("Необязательно: ОС (по умолчанию Windows)");
        form.extraFlag->setText("Зарядное устройство в комплекте");
        form.extraFlag->setChecked(true);
    } else if (index == 1) {
        form.extraSpec->setPlaceholderText("Необязательно: мощность в Вт (по умолчанию 1000)");
        form.extraFlag->setText("Переносной бытовой прибор");
        form.extraFlag->setChecked(true);
    } else {
        form.extraSpec->setPlaceholderText("Необязательно: ревизия платы (по умолчанию Rev 1.0)");
        form.extraFlag->setText("Требуется микропайка (BGA/SMD)");
        form.extraFlag->setChecked(false);
    }
}

void OrderDialog::onAddNote() {
    if (!editMode) {
        return;
    }
    try {
        storage.appendOrderNote(orderId, panels.newNote->text());
        panels.newNote->clear();
        if (const Order* order = storage.findOrderById(orderId); order != nullptr) {
            panels.notesHistory->setPlainText(order->getDiagnosticNotes());
        }
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка добавления заметки", QString::fromStdString(ex.what()));
    }
}

void OrderDialog::onReservePart() {
    if (!editMode || panels.availableParts->count() == 0) {
        return;
    }
    try {
        const auto partId = panels.availableParts->currentData().toUInt();
        const int qty = panels.partQty->value();
        storage.reservePartForOrder(orderId, partId, qty);
        populateCombos();
        refreshUsedPartsTable();
        refreshTotalLabel();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка склада", QString::fromStdString(ex.what()));
    }
}

void OrderDialog::onReturnPart() {
    if (!editMode) {
        return;
    }
    const int row = panels.usedParts->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Выбор позиции", "Выберите строку списанной детали в таблице для возврата на склад.");
        return;
    }
    try {
        const auto partId = panels.usedParts->item(row, 0)->data(Qt::UserRole).toUInt();
        const int qty = panels.partQty->value();
        storage.returnPartFromOrder(orderId, partId, qty);
        populateCombos();
        refreshUsedPartsTable();
        refreshTotalLabel();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка возврата", QString::fromStdString(ex.what()));
    }
}

void OrderDialog::onExportReceipt() {
    if (!editMode) {
        return;
    }
    try {
        const auto html = WorkshopReportService::generateAcceptanceReceiptHtml(storage, orderId);
        const auto fileName = QFileDialog::getSaveFileName(
            this,
            "Сохранить приёмную квитанцию в PDF",
            QString("Заказ_%1_Квитанция_приема.pdf").arg(orderId),
            "PDF-документы (*.pdf)"
        );
        if (fileName.isEmpty()) {
            return;
        }
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);

        QTextDocument doc;
        doc.setHtml(html);
        doc.print(&printer);
        QMessageBox::information(this, "Экспорт завершён", "Приёмная квитанция успешно сохранена в файл PDF.");
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка экспорта", QString::fromStdString(ex.what()));
    }
}

std::unique_ptr<Device> OrderDialog::buildDeviceFromForm() const {
    const auto model = InputValidator::sanitizeString(form.model->text());
    const auto serial = InputValidator::normalizeSerial(form.serial->text());
    const auto fault = InputValidator::sanitizeString(form.fault->toPlainText());
    const auto extraText = InputValidator::sanitizeString(form.extraSpec->text());
    const bool flagChecked = form.extraFlag->isChecked();
    const int cat = form.category->currentIndex();

    if (cat == 0) {
        const auto os = extraText.isEmpty() ? QString("Windows") : extraText;
        return std::make_unique<ComputerDevice>(model, serial, fault, os, flagChecked);
    }
    if (cat == 1) {
        int watts = extraText.toInt();
        if (watts <= 0) {
            watts = 1000;
        }
        return std::make_unique<HouseholdDevice>(model, serial, fault, watts, flagChecked);
    }
    const auto rev = extraText.isEmpty() ? QString("Rev 1.0") : extraText;
    return std::make_unique<ElectronicDevice>(model, serial, fault, rev, flagChecked);
}

void OrderDialog::saveNewOrder() {
    auto clientId = form.client->currentData().toUInt();
    if (clientId == 0U) {
        onClientComboActivated(0);
        clientId = form.client->currentData().toUInt();
        if (clientId == 0U) {
            return;
        }
    }

    const auto masterId = form.worker->currentData().toUInt();
    auto device = buildDeviceFromForm();
    const Order& created = storage.addOrder(clientId, masterId, std::move(device), laborSpin->value());

    const auto answer = QMessageBox::question(
        this,
        "Заказ принят",
        QString("Заказ №%1 успешно зарегистрирован.\nСформировать приёмную квитанцию (PDF) для выдачи клиенту?").arg(created.getId()),
        QMessageBox::Yes | QMessageBox::No
    );
    if (answer == QMessageBox::Yes) {
        orderId = created.getId();
        editMode = true;
        receiptBtn->setEnabled(true);
        onExportReceipt();
    }
    accept();
}

void OrderDialog::saveExistingOrder() {
    const Order* currentOrder = storage.findOrderById(orderId);
    if (!currentOrder || currentOrder->isArchived()) {
        reject();
        return;
    }

    storage.updateOrderLaborCost(orderId, laborSpin->value());
    const auto chosenStatus = static_cast<OrderStatus>(statusCombo->currentData().toInt());
    if (currentOrder->getStatus() == chosenStatus) {
        accept();
        return;
    }

    if (chosenStatus == OrderStatus::Issued) {
        const auto confirm = QMessageBox::question(
            this,
            "Перемещение в архив",
            QString("После перевода заказа №%1 в статус «Выдан» он будет перемещён в архив и заблокирован для редактирования.\nПродолжить?").arg(orderId),
            QMessageBox::Yes | QMessageBox::No
        );
        if (confirm != QMessageBox::Yes) {
            return;
        }
    }

    storage.changeOrderStatus(orderId, chosenStatus);
    accept();
}

void OrderDialog::onSave() {
    try {
        if (!editMode) {
            saveNewOrder();
        } else {
            saveExistingOrder();
        }
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка сохранения", QString::fromStdString(ex.what()));
    }
}
