#include "../headers/PartDialog.h"
#include "../headers/UiHelpers.h"
#include "../../Headers/InputValidator.h"
#include "../../Exeptions/AppException.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>

PartDialog::PartDialog(Storage& storageRef, QWidget* parent)
    : QDialog(parent), storage(storageRef) {
    buildUi();
    refreshTable();
}

void PartDialog::buildUi() {
    setWindowTitle("Склад запасных частей");
    resize(780, 540);

    auto* mainLayout = makeOwned<QVBoxLayout>(this);

    searchEdit = makeOwned<QLineEdit>(this);
    searchEdit->setPlaceholderText("Поиск комплектующих по артикулу (SKU) или наименованию...");
    mainLayout->addWidget(searchEdit);

    partsTable = makeOwned<QTableWidget>(this);
    partsTable->setColumnCount(5);
    partsTable->setHorizontalHeaderLabels({"ID", "Артикул (SKU)", "Наименование", "Остаток (шт.)", "Цена (BYN)"});
    partsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    partsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    partsTable->setAlternatingRowColors(true);
    partsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    partsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    partsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(partsTable, 1);

    auto* addGroup = makeOwned<QGroupBox>("Оприходование новой номенклатуры", this);
    auto* addLayout = makeOwned<QHBoxLayout>(addGroup);
    skuEdit = makeOwned<QLineEdit>(this);
    skuEdit->setPlaceholderText("Артикул (напр. CAP-100UF)");
    InputValidator::setupSkuValidator(skuEdit);
    nameEdit = makeOwned<QLineEdit>(this);
    nameEdit->setPlaceholderText("Наименование детали");
    qtySpin = makeOwned<QSpinBox>(this);
    qtySpin->setRange(0, 10000);
    priceSpin = makeOwned<QDoubleSpinBox>(this);
    priceSpin->setRange(0.01, 5000.0);
    priceSpin->setDecimals(2);
    priceSpin->setSuffix(" BYN");
    addPartBtn = makeOwned<QPushButton>("Добавить позицию", this);
    addPartBtn->setObjectName("primaryBtn");

    addLayout->addWidget(skuEdit, 2);
    addLayout->addWidget(nameEdit, 3);
    addLayout->addWidget(qtySpin, 1);
    addLayout->addWidget(priceSpin, 1);
    addLayout->addWidget(addPartBtn);
    mainLayout->addWidget(addGroup);

    auto* restockRow = makeOwned<QHBoxLayout>();
    restockSpin = makeOwned<QSpinBox>(this);
    restockSpin->setRange(1, 10000);
    restockBtn = makeOwned<QPushButton>("Пополнить остаток выбранной", this);
    closeBtn = makeOwned<QPushButton>("Закрыть", this);

    restockRow->addWidget(restockSpin);
    restockRow->addWidget(restockBtn);
    restockRow->addStretch(1);
    restockRow->addWidget(closeBtn);
    mainLayout->addLayout(restockRow);

    connect(searchEdit, &QLineEdit::textChanged, this, &PartDialog::onSearchChanged);
    connect(addPartBtn, &QPushButton::clicked, this, &PartDialog::onAddPart);
    connect(restockBtn, &QPushButton::clicked, this, &PartDialog::onRestockSelected);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void PartDialog::refreshTable() {
    std::vector<SparePart> list = storage.searchParts(searchEdit->text());
    partsTable->setRowCount(static_cast<int>(list.size()));
    for (int row = 0; row < static_cast<int>(list.size()); ++row) {
        const SparePart& p = list[static_cast<std::size_t>(row)];
        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(p.getId()));
        idItem->setData(Qt::UserRole, p.getId());
        partsTable->setItem(row, 0, idItem);
        partsTable->setItem(row, 1, makeOwned<QTableWidgetItem>(p.getSku()));
        partsTable->setItem(row, 2, makeOwned<QTableWidgetItem>(p.getName()));
        partsTable->setItem(row, 3, makeOwned<QTableWidgetItem>(QString::number(p.getQuantity())));
        partsTable->setItem(row, 4, makeOwned<QTableWidgetItem>(QString::number(p.getPrice(), 'f', 2)));
    }
}

void PartDialog::onSearchChanged(const QString&) {
    refreshTable();
}

void PartDialog::onAddPart() {
    try {
        storage.addPart(skuEdit->text(), nameEdit->text(), qtySpin->value(), priceSpin->value());
        skuEdit->clear();
        nameEdit->clear();
        qtySpin->setValue(0);
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка склада", QString::fromStdString(ex.what()));
    }
}

void PartDialog::onRestockSelected() {
    int row = partsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Выбор позиции", "Выберите строку детали в таблице для пополнения остатка.");
        return;
    }
    try {
        unsigned int partId = partsTable->item(row, 0)->data(Qt::UserRole).toUInt();
        storage.restockPart(partId, restockSpin->value());
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка пополнения", QString::fromStdString(ex.what()));
    }
}
