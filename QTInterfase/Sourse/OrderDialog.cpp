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
    QColor mutedGray("#7A869A");
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
    notesOverlay = createSectionOverlay(notesGroup);
    partsOverlay = createSectionOverlay(partsGroup);
}

bool OrderDialog::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
        if (watched == notesGroup && notesOverlay != nullptr) {
            notesOverlay->setGeometry(notesGroup->rect());
            notesOverlay->raise();
        } else if (watched == partsGroup && partsOverlay != nullptr) {
            partsOverlay->setGeometry(partsGroup->rect());
            partsOverlay->raise();
        }
    }
    return QDialog::eventFilter(watched, event);
}

void OrderDialog::buildUi() {
    setWindowTitle(editMode ? QString("Карточка заказа №%1").arg(orderId) : "Регистрация нового заказа");
    resize(820, 735);

    auto* mainLayout = makeOwned<QVBoxLayout>(this);
    auto* formLayout = makeOwned<QFormLayout>();

    clientCombo = makeOwned<QComboBox>(this);
    masterCombo = makeOwned<QComboBox>(this);
    categoryCombo = makeOwned<QComboBox>(this);
    categoryCombo->addItem("Компьютерная техника", "Computer");
    categoryCombo->addItem("Бытовая техника", "Household");
    categoryCombo->addItem("Радиоэлектроника", "Electronics");

    modelEdit = makeOwned<QLineEdit>(this);
    modelEdit->setPlaceholderText("Например: ASUS VivoBook 15 / Bosch KGN39");
    serialEdit = makeOwned<QLineEdit>(this);
    serialEdit->setPlaceholderText("Необязательно (если отсутствует — запишется «Не указан»)");
    extraSpecEdit = makeOwned<QLineEdit>(this);
    extraFlagCheck = makeOwned<QCheckBox>(this);
    faultEdit = makeOwned<QTextEdit>(this);
    faultEdit->setPlaceholderText("Опишите симптомы неисправности со слов клиента...");
    faultEdit->setMaximumHeight(65);

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

    formLayout->addRow(editMode ? "Заказчик:" : "Заказчик *:", clientCombo);
    formLayout->addRow("Ответственный работник:", masterCombo);
    formLayout->addRow(editMode ? "Категория техники:" : "Категория техники *:", categoryCombo);
    formLayout->addRow(editMode ? "Модель устройства:" : "Модель устройства *:", modelEdit);
    formLayout->addRow("Серийный номер / IMEI:", serialEdit);
    formLayout->addRow("Технический параметр:", extraSpecEdit);
    formLayout->addRow("Комплектация / опции:", extraFlagCheck);
    formLayout->addRow(editMode ? "Заявленная неисправность:" : "Заявленная неисправность *:", faultEdit);
    formLayout->addRow("Статус заказа:", statusCombo);
    formLayout->addRow("Базовая стоимость работ:", laborSpin);

    mainLayout->addLayout(formLayout);

    if (!editMode) {
        auto* requiredHintLabel = makeOwned<QLabel>("* Обязательные поля для приёма заказа. Остальные параметры можно уточнить после диагностики.", this);
        requiredHintLabel->setWordWrap(true);
        mainLayout->addWidget(requiredHintLabel);
    }

    notesGroup = makeOwned<QGroupBox>("Журнал диагностики и заметки работника", this);
    auto* notesLayout = makeOwned<QVBoxLayout>(notesGroup);
    notesHistoryEdit = makeOwned<QTextEdit>(this);
    notesHistoryEdit->setReadOnly(true);
    notesHistoryEdit->setMaximumHeight(85);
    auto* addNoteRow = makeOwned<QHBoxLayout>();
    newNoteEdit = makeOwned<QLineEdit>(this);
    newNoteEdit->setPlaceholderText("Введите технологическую заметку...");
    auto* addNoteBtn = makeOwned<QPushButton>("Добавить заметку", this);
    addNoteRow->addWidget(newNoteEdit, 1);
    addNoteRow->addWidget(addNoteBtn);
    notesLayout->addWidget(notesHistoryEdit);
    notesLayout->addLayout(addNoteRow);
    mainLayout->addWidget(notesGroup);

    partsGroup = makeOwned<QGroupBox>("Складские запасные части в заказе", this);
    auto* partsLayout = makeOwned<QVBoxLayout>(partsGroup);
    auto* partControlRow = makeOwned<QHBoxLayout>();
    availablePartsCombo = makeOwned<QComboBox>(this);
    partQtySpin = makeOwned<QSpinBox>(this);
    partQtySpin->setRange(1, 200);
    auto* reservePartBtn = makeOwned<QPushButton>("Списать в заказ", this);
    auto* returnPartBtn = makeOwned<QPushButton>("Вернуть на склад", this);

    partControlRow->addWidget(availablePartsCombo, 1);
    partControlRow->addWidget(partQtySpin);
    partControlRow->addWidget(reservePartBtn);
    partControlRow->addWidget(returnPartBtn);

    usedPartsTable = makeOwned<QTableWidget>(this);
    usedPartsTable->setColumnCount(5);
    usedPartsTable->setHorizontalHeaderLabels({"ID", "Наименование", "Кол-во", "Цена (BYN)", "Сумма (BYN)"});
    usedPartsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usedPartsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    usedPartsTable->setAlternatingRowColors(true);
    usedPartsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usedPartsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    partsLayout->addLayout(partControlRow);
    partsLayout->addWidget(usedPartsTable);
    mainLayout->addWidget(partsGroup);

    auto* bottomRow = makeOwned<QHBoxLayout>();
    totalCostLabel = makeOwned<QLabel>("Итоговая стоимость: 0.00 BYN", this);
    totalCostLabel->setObjectName("totalHighlight");
    auto* receiptBtn = makeOwned<QPushButton>("Квитанция приёма (PDF)", this);
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

        notesHistoryEdit->setEnabled(false);
        newNoteEdit->setEnabled(false);
        addNoteBtn->setEnabled(false);

        availablePartsCombo->setEnabled(false);
        partQtySpin->setEnabled(false);
        reservePartBtn->setEnabled(false);
        returnPartBtn->setEnabled(false);
        usedPartsTable->setEnabled(false);

        setupLockedOverlays();
    } else {
        clientCombo->setEnabled(false);
        masterCombo->setEnabled(false);
        categoryCombo->setEnabled(false);
        modelEdit->setReadOnly(true);
        serialEdit->setReadOnly(true);
        extraSpecEdit->setReadOnly(true);
        extraFlagCheck->setEnabled(false);
        faultEdit->setReadOnly(true);

        markFieldLocked(clientCombo);
        markFieldLocked(masterCombo);
        markFieldLocked(categoryCombo);
        markFieldLocked(modelEdit);
        markFieldLocked(serialEdit);
        markFieldLocked(extraSpecEdit);
        markFieldLocked(extraFlagCheck);
        markFieldLocked(faultEdit);
        markFieldLocked(notesHistoryEdit);
    }

    connect(clientCombo, QOverload<int>::of(&QComboBox::activated), this, &OrderDialog::onClientComboActivated);
    connect(categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrderDialog::onCategoryChanged);
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
    unsigned int previousClientId = clientCombo->currentData().toUInt();
    clientCombo->clear();
    if (!editMode) {
        clientCombo->addItem("+ Добавить нового клиента...", 0U);
    }
    for (const auto& c : storage.getClients()) {
        clientCombo->addItem(QString("%1 (%2)").arg(c.getName(), c.getPhone()), c.getId());
    }
    if (!editMode) {
        if (int idx = clientCombo->findData(previousClientId); previousClientId > 0 && idx >= 0) {
            clientCombo->setCurrentIndex(idx);
        } else if (clientCombo->count() > 1) {
            clientCombo->setCurrentIndex(1);
        } else {
            clientCombo->setCurrentIndex(0);
        }
    }

    masterCombo->clear();
    if (!editMode) {
        masterCombo->addItem("Автоназначение (наименее загруженный работник)", 0);
    }
    for (const auto& m : storage.getMasters()) {
        QString specRu = m.getSpecialization();
        if (specRu == "Computer") {
            specRu = "Компьютерная техника";
        } else if (specRu == "Household") {
            specRu = "Бытовая техника";
        } else if (specRu == "Electronics") {
            specRu = "Радиоэлектроника";
        }
        masterCombo->addItem(
            QString("Работник №%1: %2 [%3] (в работе: %4)")
                .arg(m.getId())
                .arg(m.getName(), specRu)
                .arg(m.getActiveOrdersCount()),
            m.getId()
        );
    }

    availablePartsCombo->clear();
    for (const auto& p : WorkshopSearchService::findAvailableParts(storage)) {
        availablePartsCombo->addItem(
            QString("[%1] %2 (Остаток: %3 шт., %4 BYN)").arg(p.getSku(), p.getName()).arg(p.getQuantity()).arg(p.getPrice(), 0, 'f', 2),
            p.getId()
        );
    }
}

void OrderDialog::onClientComboActivated(int index) {
    if (editMode || index < 0) {
        return;
    }
    if (clientCombo->itemData(index).toUInt() != 0U) {
        return;
    }

    QuickClientDialog quickClientDlg(storage, this);
    quickClientDlg.exec();
    unsigned int newlyAddedId = quickClientDlg.getLastAddedClientId();
    populateCombos();

    if (newlyAddedId > 0) {
        if (int newIdx = clientCombo->findData(newlyAddedId); newIdx >= 0) {
            clientCombo->setCurrentIndex(newIdx);
        }
    }
}

void OrderDialog::applyReadOnlyArchiveMode() {
    setWindowTitle(QString("Архивный заказ №%1 (выдан, только просмотр)").arg(orderId));
    statusCombo->setEnabled(false);
    laborSpin->setReadOnly(true);
    newNoteEdit->setEnabled(false);
    newNoteEdit->setPlaceholderText("");
    availablePartsCombo->setEnabled(false);
    partQtySpin->setEnabled(false);
    saveBtn->setVisible(false);

    markFieldLocked(statusCombo);
    markFieldLocked(laborSpin);
    markFieldLocked(newNoteEdit);
    markFieldLocked(availablePartsCombo);
    markFieldLocked(partQtySpin);

    const auto buttons = findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        QString text = btn->text();
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

    if (int clientIdx = clientCombo->findData(order->getClientId()); clientIdx >= 0) {
        clientCombo->setCurrentIndex(clientIdx);
    }
    if (int masterIdx = masterCombo->findData(order->getMasterId()); masterIdx >= 0) {
        masterCombo->setCurrentIndex(masterIdx);
    }

    if (order->getDevice()) {
        if (int catIdx = categoryCombo->findData(order->getDevice()->getCategory()); catIdx >= 0) {
            categoryCombo->setCurrentIndex(catIdx);
        }
        modelEdit->setText(order->getDevice()->getModel());
        serialEdit->setText(order->getDevice()->getSerialNumber());
        faultEdit->setPlainText(order->getDevice()->getFaultDescription());
        extraSpecEdit->setText(order->getDevice()->getSpecsSummary());
    }

    if (int statusIdx = statusCombo->findData(order->getStatusCode()); statusIdx >= 0) {
        statusCombo->setCurrentIndex(statusIdx);
    }
    laborSpin->setValue(order->getLaborCost());
    notesHistoryEdit->setPlainText(order->getDiagnosticNotes());

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
    usedPartsTable->setRowCount(static_cast<int>(items.size()));
    for (int row = 0; row < static_cast<int>(items.size()); ++row) {
        const auto& item = items[static_cast<std::size_t>(row)];
        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(item.getPartId()));
        idItem->setData(Qt::UserRole, item.getPartId());
        usedPartsTable->setItem(row, 0, idItem);
        usedPartsTable->setItem(row, 1, makeOwned<QTableWidgetItem>(item.getPartName()));
        usedPartsTable->setItem(row, 2, makeOwned<QTableWidgetItem>(QString::number(item.getQuantity())));
        usedPartsTable->setItem(row, 3, makeOwned<QTableWidgetItem>(QString::number(item.getPriceAtSale(), 'f', 2)));
        usedPartsTable->setItem(row, 4, makeOwned<QTableWidgetItem>(QString::number(item.getSubtotal(), 'f', 2)));
    }
}

void OrderDialog::refreshTotalLabel() {
    if (editMode) {
        if (const Order* order = storage.findOrderById(orderId); order != nullptr) {
            double factor = order->getDevice() ? order->getDevice()->getComplexityFactor() : 1.0;
            double total = (laborSpin->value() * factor) + order->calculatePartsCost();
            totalCostLabel->setText(QString("Итоговая стоимость: %1 BYN (коэф. сложности x%2)").arg(total, 0, 'f', 2).arg(factor, 0, 'f', 2));
            return;
        }
    }
    totalCostLabel->setText(QString("Базовая стоимость работ: %1 BYN").arg(laborSpin->value(), 0, 'f', 2));
}

void OrderDialog::onCategoryChanged(int index) {
    if (editMode) {
        return;
    }
    if (index == 0) {
        extraSpecEdit->setPlaceholderText("Необязательно: ОС (по умолчанию Windows)");
        extraFlagCheck->setText("Зарядное устройство в комплекте");
        extraFlagCheck->setChecked(true);
    } else if (index == 1) {
        extraSpecEdit->setPlaceholderText("Необязательно: мощность в Вт (по умолчанию 1000)");
        extraFlagCheck->setText("Переносной бытовой прибор");
        extraFlagCheck->setChecked(true);
    } else {
        extraSpecEdit->setPlaceholderText("Необязательно: ревизия платы (по умолчанию Rev 1.0)");
        extraFlagCheck->setText("Требуется микропайка (BGA/SMD)");
        extraFlagCheck->setChecked(false);
    }
}

void OrderDialog::onAddNote() {
    if (!editMode) {
        return;
    }
    try {
        storage.appendOrderNote(orderId, newNoteEdit->text());
        newNoteEdit->clear();
        if (const Order* order = storage.findOrderById(orderId); order != nullptr) {
            notesHistoryEdit->setPlainText(order->getDiagnosticNotes());
        }
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка добавления заметки", QString::fromStdString(ex.what()));
    }
}

void OrderDialog::onReservePart() {
    if (!editMode || availablePartsCombo->count() == 0) {
        return;
    }
    try {
        unsigned int partId = availablePartsCombo->currentData().toUInt();
        int qty = partQtySpin->value();
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
    int row = usedPartsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Выбор позиции", "Выберите строку списанной детали в таблице для возврата на склад.");
        return;
    }
    try {
        unsigned int partId = usedPartsTable->item(row, 0)->data(Qt::UserRole).toUInt();
        int qty = partQtySpin->value();
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
        QString html = WorkshopReportService::generateAcceptanceReceiptHtml(storage, orderId);
        QString fileName = QFileDialog::getSaveFileName(
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

void OrderDialog::onSave() {
    try {
        if (!editMode) {
            unsigned int clientId = clientCombo->currentData().toUInt();
            if (clientId == 0U) {
                onClientComboActivated(0);
                clientId = clientCombo->currentData().toUInt();
                if (clientId == 0U) {
                    return;
                }
            }
            unsigned int masterId = masterCombo->currentData().toUInt();
            QString model = InputValidator::sanitizeString(modelEdit->text());
            QString serial = InputValidator::normalizeSerial(serialEdit->text());
            QString fault = InputValidator::sanitizeString(faultEdit->toPlainText());
            double labor = laborSpin->value();

            std::unique_ptr<Device> device;
            if (int cat = categoryCombo->currentIndex(); cat == 0) {
                QString os = extraSpecEdit->text().trimmed().isEmpty() ? "Windows" : InputValidator::sanitizeString(extraSpecEdit->text());
                device = std::make_unique<ComputerDevice>(model, serial, fault, os, extraFlagCheck->isChecked());
            } else if (cat == 1) {
                int watts = extraSpecEdit->text().toInt();
                if (watts <= 0) {
                    watts = 1000;
                }
                device = std::make_unique<HouseholdDevice>(model, serial, fault, watts, extraFlagCheck->isChecked());
            } else {
                QString rev = extraSpecEdit->text().trimmed().isEmpty() ? "Rev 1.0" : InputValidator::sanitizeString(extraSpecEdit->text());
                device = std::make_unique<ElectronicDevice>(model, serial, fault, rev, extraFlagCheck->isChecked());
            }

            const Order& created = storage.addOrder(clientId, masterId, std::move(device), labor);
            if (auto answer = QMessageBox::question(
                    this,
                    "Заказ принят",
                    QString("Заказ №%1 успешно зарегистрирован.\nСформировать приёмную квитанцию (PDF) для выдачи клиенту?").arg(created.getId()),
                    QMessageBox::Yes | QMessageBox::No
                ); answer == QMessageBox::Yes) {
                orderId = created.getId();
                editMode = true;
                onExportReceipt();
            }
            accept();
        } else {
            const Order* currentOrder = storage.findOrderById(orderId);
            if (!currentOrder || currentOrder->isArchived()) {
                reject();
                return;
            }
            storage.updateOrderLaborCost(orderId, laborSpin->value());
            if (auto chosenStatus = static_cast<OrderStatus>(statusCombo->currentData().toInt());
                currentOrder->getStatus() != chosenStatus) {
                if (chosenStatus == OrderStatus::Issued) {
                    if (auto confirm = QMessageBox::question(
                            this,
                            "Перемещение в архив",
                            QString("После перевода заказа №%1 в статус «Выдан» он будет перемещён в архив и заблокирован для редактирования.\nПродолжить?").arg(orderId),
                            QMessageBox::Yes | QMessageBox::No
                        ); confirm != QMessageBox::Yes) {
                        return;
                    }
                }
                storage.changeOrderStatus(orderId, chosenStatus);
            }
            accept();
        }
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка сохранения", QString::fromStdString(ex.what()));
    }
}
