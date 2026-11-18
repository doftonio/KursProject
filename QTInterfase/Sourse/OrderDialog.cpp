#include "../headers/OrderDialog.h"
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
#include <QGroupBox>

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

void OrderDialog::buildUi() {
    setWindowTitle(editMode ? QString("Карточка заказа №%1").arg(orderId) : "Регистрация нового заказа");
    resize(820, 710);

    auto* mainLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    clientCombo = new QComboBox(this);
    masterCombo = new QComboBox(this);
    categoryCombo = new QComboBox(this);
    categoryCombo->addItem("Компьютерная техника", "Computer");
    categoryCombo->addItem("Бытовая техника", "Household");
    categoryCombo->addItem("Радиоэлектроника", "Electronics");

    modelEdit = new QLineEdit(this);
    serialEdit = new QLineEdit(this);
    extraSpecEdit = new QLineEdit(this);
    extraFlagCheck = new QCheckBox(this);
    faultEdit = new QTextEdit(this);
    faultEdit->setMaximumHeight(65);

    statusCombo = new QComboBox(this);
    statusCombo->addItem("Принят", 0);
    statusCombo->addItem("Диагностика", 1);
    statusCombo->addItem("Ожидание запчастей", 2);
    statusCombo->addItem("Завершён", 3);
    statusCombo->addItem("Выдан", 4);

    laborSpin = new QDoubleSpinBox(this);
    laborSpin->setRange(0.0, 10000.0);
    laborSpin->setDecimals(2);
    laborSpin->setSuffix(" BYN");

    formLayout->addRow("Заказчик:", clientCombo);
    formLayout->addRow("Ответственный мастер:", masterCombo);
    formLayout->addRow("Категория техники:", categoryCombo);
    formLayout->addRow("Модель устройства:", modelEdit);
    formLayout->addRow("Серийный номер / IMEI:", serialEdit);
    formLayout->addRow("Технический параметр:", extraSpecEdit);
    formLayout->addRow("Комплектация / опции:", extraFlagCheck);
    formLayout->addRow("Заявленная неисправность:", faultEdit);
    formLayout->addRow("Статус заказа:", statusCombo);
    formLayout->addRow("Базовая стоимость работ:", laborSpin);

    mainLayout->addLayout(formLayout);

    auto* notesGroup = new QGroupBox("Журнал диагностики и заметки мастера", this);
    auto* notesLayout = new QVBoxLayout(notesGroup);
    notesHistoryEdit = new QTextEdit(this);
    notesHistoryEdit->setReadOnly(true);
    notesHistoryEdit->setMaximumHeight(85);
    auto* addNoteRow = new QHBoxLayout();
    newNoteEdit = new QLineEdit(this);
    newNoteEdit->setPlaceholderText("Введите технологическую заметку мастера...");
    addNoteBtn = new QPushButton("Добавить заметку", this);
    addNoteRow->addWidget(newNoteEdit, 1);
    addNoteRow->addWidget(addNoteBtn);
    notesLayout->addWidget(notesHistoryEdit);
    notesLayout->addLayout(addNoteRow);
    mainLayout->addWidget(notesGroup);

    auto* partsGroup = new QGroupBox("Складские запасные части в заказе", this);
    auto* partsLayout = new QVBoxLayout(partsGroup);
    auto* partControlRow = new QHBoxLayout();
    availablePartsCombo = new QComboBox(this);
    partQtySpin = new QSpinBox(this);
    partQtySpin->setRange(1, 200);
    reservePartBtn = new QPushButton("Списать в заказ", this);
    returnPartBtn = new QPushButton("Вернуть на склад", this);

    partControlRow->addWidget(availablePartsCombo, 1);
    partControlRow->addWidget(partQtySpin);
    partControlRow->addWidget(reservePartBtn);
    partControlRow->addWidget(returnPartBtn);

    usedPartsTable = new QTableWidget(this);
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

    auto* bottomRow = new QHBoxLayout();
    totalCostLabel = new QLabel("Итоговая стоимость: 0.00 BYN", this);
    totalCostLabel->setObjectName("totalHighlight");
    auto* saveBtn = new QPushButton("Сохранить", this);
    saveBtn->setObjectName("primaryBtn");
    auto* cancelBtn = new QPushButton("Закрыть", this);

    bottomRow->addWidget(totalCostLabel, 1);
    bottomRow->addWidget(saveBtn);
    bottomRow->addWidget(cancelBtn);
    mainLayout->addLayout(bottomRow);

    if (!editMode) {
        statusCombo->setEnabled(false);
        notesGroup->setEnabled(false);
        partsGroup->setEnabled(false);
    } else {
        clientCombo->setEnabled(false);
        masterCombo->setEnabled(false);
        categoryCombo->setEnabled(false);
        modelEdit->setReadOnly(true);
        serialEdit->setReadOnly(true);
        extraSpecEdit->setReadOnly(true);
        extraFlagCheck->setEnabled(false);
        faultEdit->setReadOnly(true);
    }

    connect(categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrderDialog::onCategoryChanged);
    connect(addNoteBtn, &QPushButton::clicked, this, &OrderDialog::onAddNote);
    connect(reservePartBtn, &QPushButton::clicked, this, &OrderDialog::onReservePart);
    connect(returnPartBtn, &QPushButton::clicked, this, &OrderDialog::onReturnPart);
    connect(saveBtn, &QPushButton::clicked, this, &OrderDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(laborSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        refreshTotalLabel();
    });
}

void OrderDialog::populateCombos() {
    clientCombo->clear();
    for (const auto& c : storage.getClients()) {
        clientCombo->addItem(QString("%1 (%2)").arg(c.getName(), c.getPhone()), c.getId());
    }

    masterCombo->clear();
    if (!editMode) {
        masterCombo->addItem("Автоназначение (наименее загруженный мастер)", 0);
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
            QString("Мастер №%1: %2 [%3] (в работе: %4)")
                .arg(m.getId())
                .arg(m.getName(), specRu)
                .arg(m.getActiveOrdersCount()),
            m.getId()
        );
    }

    availablePartsCombo->clear();
    for (const auto& p : storage.findAvailableParts()) {
        availablePartsCombo->addItem(
            QString("[%1] %2 (Остаток: %3 шт., %4 BYN)").arg(p.getSku(), p.getName()).arg(p.getQuantity()).arg(p.getPrice(), 0, 'f', 2),
            p.getId()
        );
    }
}

void OrderDialog::loadExistingOrder() {
    const Order* order = storage.findOrderById(orderId);
    if (!order) {
        return;
    }

    int clientIdx = clientCombo->findData(order->getClientId());
    if (clientIdx >= 0) {
        clientCombo->setCurrentIndex(clientIdx);
    }
    int masterIdx = masterCombo->findData(order->getMasterId());
    if (masterIdx >= 0) {
        masterCombo->setCurrentIndex(masterIdx);
    }

    if (order->getDevice()) {
        int catIdx = categoryCombo->findData(order->getDevice()->getCategory());
        if (catIdx >= 0) {
            categoryCombo->setCurrentIndex(catIdx);
        }
        modelEdit->setText(order->getDevice()->getModel());
        serialEdit->setText(order->getDevice()->getSerialNumber());
        faultEdit->setPlainText(order->getDevice()->getFaultDescription());
        extraSpecEdit->setText(order->getDevice()->getSpecsSummary());
    }

    int statusIdx = statusCombo->findData(order->getStatusCode());
    if (statusIdx >= 0) {
        statusCombo->setCurrentIndex(statusIdx);
    }
    laborSpin->setValue(order->getLaborCost());
    notesHistoryEdit->setPlainText(order->getDiagnosticNotes());

    refreshUsedPartsTable();
    refreshTotalLabel();
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
        auto* idItem = new QTableWidgetItem(QString::number(item.getPartId()));
        idItem->setData(Qt::UserRole, item.getPartId());
        usedPartsTable->setItem(row, 0, idItem);
        usedPartsTable->setItem(row, 1, new QTableWidgetItem(item.getPartName()));
        usedPartsTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.getQuantity())));
        usedPartsTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.getPriceAtSale(), 'f', 2)));
        usedPartsTable->setItem(row, 4, new QTableWidgetItem(QString::number(item.getSubtotal(), 'f', 2)));
    }
}

void OrderDialog::refreshTotalLabel() {
    if (editMode) {
        const Order* order = storage.findOrderById(orderId);
        if (order) {
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
        extraSpecEdit->setPlaceholderText("Операционная система (например, Windows 11)");
        extraFlagCheck->setText("Зарядное устройство в комплекте");
        extraFlagCheck->setChecked(true);
    } else if (index == 1) {
        extraSpecEdit->setPlaceholderText("Мощность в Вт (например, 2200)");
        extraFlagCheck->setText("Переносной бытовой прибор");
        extraFlagCheck->setChecked(true);
    } else {
        extraSpecEdit->setPlaceholderText("Ревизия печатной платы (например, Rev 2.1)");
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
        const Order* order = storage.findOrderById(orderId);
        if (order) {
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

void OrderDialog::onSave() {
    try {
        if (!editMode) {
            if (clientCombo->count() == 0) {
                QMessageBox::warning(this, "Отсутствует клиент", "Сначала зарегистрируйте клиента в справочнике.");
                return;
            }
            unsigned int clientId = clientCombo->currentData().toUInt();
            unsigned int masterId = masterCombo->currentData().toUInt();
            QString model = modelEdit->text();
            QString serial = serialEdit->text();
            QString fault = faultEdit->toPlainText();
            double labor = laborSpin->value();

            std::unique_ptr<Device> device;
            int cat = categoryCombo->currentIndex();
            if (cat == 0) {
                QString os = extraSpecEdit->text().trimmed().isEmpty() ? "Windows" : extraSpecEdit->text().trimmed();
                device = std::make_unique<ComputerDevice>(model, serial, fault, os, extraFlagCheck->isChecked());
            } else if (cat == 1) {
                int watts = extraSpecEdit->text().toInt();
                if (watts <= 0) {
                    watts = 1000;
                }
                device = std::make_unique<HouseholdDevice>(model, serial, fault, watts, extraFlagCheck->isChecked());
            } else {
                QString rev = extraSpecEdit->text().trimmed().isEmpty() ? "Rev 1.0" : extraSpecEdit->text().trimmed();
                device = std::make_unique<ElectronicDevice>(model, serial, fault, rev, extraFlagCheck->isChecked());
            }

            storage.addOrder(clientId, masterId, std::move(device), labor);
            accept();
        } else {
            storage.updateOrderLaborCost(orderId, laborSpin->value());
            OrderStatus chosenStatus = static_cast<OrderStatus>(statusCombo->currentData().toInt());
            const Order* currentOrder = storage.findOrderById(orderId);
            if (currentOrder && currentOrder->getStatus() != chosenStatus) {
                storage.changeOrderStatus(orderId, chosenStatus);
            }
            accept();
        }
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка сохранения", QString::fromStdString(ex.what()));
    }
}
