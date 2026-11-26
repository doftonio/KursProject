#include "../headers/WorkersTab.h"
#include "../headers/UiHelpers.h"
#include "../../Headers/WorkshopSearchService.h"
#include "../../Headers/InputValidator.h"
#include "../../Exeptions/AppException.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QPushButton>

WorkersTab::WorkersTab(Storage& storageRef, QWidget* parent)
    : QWidget(parent), storage(storageRef) {
    buildUi();
    refreshTable();
}

QString WorkersTab::formatSpecRu(const QString& spec) {
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

QString WorkersTab::parseSpecFromLabel(const QString& label) {
    QString trimmed = label.trimmed();
    if (trimmed.compare("Компьютерная техника", Qt::CaseInsensitive) == 0 || trimmed.compare("Computer", Qt::CaseInsensitive) == 0) {
        return "Computer";
    }
    if (trimmed.compare("Бытовая техника", Qt::CaseInsensitive) == 0 || trimmed.compare("Household", Qt::CaseInsensitive) == 0) {
        return "Household";
    }
    if (trimmed.compare("Радиоэлектроника", Qt::CaseInsensitive) == 0 || trimmed.compare("Electronics", Qt::CaseInsensitive) == 0) {
        return "Electronics";
    }
    return trimmed;
}

void WorkersTab::buildUi() {
    auto* mainLayout = makeOwned<QVBoxLayout>(this);

    searchEdit = makeOwned<QLineEdit>(this);
    searchEdit->setPlaceholderText("Поиск работника по ФИО, телефону или профилю...");
    mainLayout->addWidget(searchEdit);

    workersTable = makeOwned<QTableWidget>(this);
    workersTable->setColumnCount(5);
    workersTable->setHorizontalHeaderLabels({"ID", "ФИО работника", "Телефон", "Профиль", "Активных заказов"});
    workersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    workersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    workersTable->setAlternatingRowColors(true);
    workersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    workersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    workersTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    mainLayout->addWidget(workersTable, 1);

    auto* addGroup = makeOwned<QGroupBox>("Приём нового работника", this);
    auto* addLayout = makeOwned<QHBoxLayout>(addGroup);
    nameEdit = makeOwned<QLineEdit>(this);
    nameEdit->setPlaceholderText("ФИО работника");
    phoneEdit = makeOwned<QLineEdit>(this);
    InputValidator::setupPhoneMask(phoneEdit);
    addSpecCombo = makeOwned<QComboBox>(this);
    addSpecCombo->addItem("Компьютерная техника", "Computer");
    addSpecCombo->addItem("Бытовая техника", "Household");
    addSpecCombo->addItem("Радиоэлектроника", "Electronics");
    auto* addBtn = makeOwned<QPushButton>("Добавить работника", this);
    addBtn->setObjectName("primaryBtn");

    addLayout->addWidget(nameEdit, 3);
    addLayout->addWidget(phoneEdit, 2);
    addLayout->addWidget(addSpecCombo, 2);
    addLayout->addWidget(addBtn);
    mainLayout->addWidget(addGroup);

    auto* bottomLayout = makeOwned<QHBoxLayout>();
    auto* removeBtn = makeOwned<QPushButton>("Удалить выбранного", this);
    removeBtn->setObjectName("dangerBtn");
    bottomLayout->addWidget(removeBtn);
    bottomLayout->addStretch(1);
    mainLayout->addLayout(bottomLayout);

    connect(searchEdit, &QLineEdit::textChanged, this, &WorkersTab::onSearchChanged);
    connect(workersTable, &QTableWidget::cellChanged, this, &WorkersTab::onWorkerCellChanged);
    connect(addBtn, &QPushButton::clicked, this, &WorkersTab::onAddWorker);
    connect(removeBtn, &QPushButton::clicked, this, &WorkersTab::onRemoveWorker);
}

void WorkersTab::refreshTable() {
    populatingTable = true;
    std::vector<Master> list = WorkshopSearchService::searchMasters(storage, searchEdit->text());
    workersTable->setRowCount(static_cast<int>(list.size()));
    for (int row = 0; row < static_cast<int>(list.size()); ++row) {
        const Master& m = list[static_cast<std::size_t>(row)];

        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(m.getId()));
        idItem->setData(Qt::UserRole, m.getId());
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

        auto* nameItem = makeOwned<QTableWidgetItem>(m.getName());
        auto* phoneItem = makeOwned<QTableWidgetItem>(m.getPhone());
        auto* specItem = makeOwned<QTableWidgetItem>(formatSpecRu(m.getSpecialization()));

        auto* activeItem = makeOwned<QTableWidgetItem>(QString("%1 / %2").arg(m.getActiveOrdersCount()).arg(Master::maxOrdersPerMaster));
        activeItem->setFlags(activeItem->flags() & ~Qt::ItemIsEditable);

        workersTable->setItem(row, 0, idItem);
        workersTable->setItem(row, 1, nameItem);
        workersTable->setItem(row, 2, phoneItem);
        workersTable->setItem(row, 3, specItem);
        workersTable->setItem(row, 4, activeItem);
    }
    populatingTable = false;
}

unsigned int WorkersTab::getSelectedWorkerId() const {
    int row = workersTable->currentRow();
    if (row < 0) {
        return 0;
    }
    const QTableWidgetItem* item = workersTable->item(row, 0);
    return item ? item->data(Qt::UserRole).toUInt() : 0;
}

void WorkersTab::onSearchChanged(const QString&) {
    refreshTable();
}

void WorkersTab::onAddWorker() {
    try {
        QString spec = addSpecCombo->currentData().toString();
        storage.addMaster(nameEdit->text(), phoneEdit->text(), spec);
        nameEdit->clear();
        phoneEdit->clear();
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка добавления работника", QString::fromStdString(ex.what()));
    }
}

void WorkersTab::onWorkerCellChanged(int row, int) {
    if (populatingTable || row < 0) {
        return;
    }
    const QTableWidgetItem* idItem = workersTable->item(row, 0);
    const QTableWidgetItem* nameItem = workersTable->item(row, 1);
    const QTableWidgetItem* phoneItem = workersTable->item(row, 2);
    const QTableWidgetItem* specItem = workersTable->item(row, 3);
    if (!idItem || !nameItem || !phoneItem || !specItem) {
        return;
    }

    unsigned int workerId = idItem->data(Qt::UserRole).toUInt();
    Master* existing = const_cast<Master*>(storage.findMasterById(workerId));
    if (!existing) {
        return;
    }

    try {
        InputValidator::validateStringNotEmpty(nameItem->text(), "Worker Name");
        InputValidator::validatePhone(phoneItem->text());
        QString parsedSpec = parseSpecFromLabel(specItem->text());
        InputValidator::validateStringNotEmpty(parsedSpec, "Profile");

        existing->setName(InputValidator::sanitizeString(nameItem->text()));
        existing->setPhone(InputValidator::normalizePhone(phoneItem->text()));
        existing->setSpecialization(InputValidator::sanitizeString(parsedSpec));
        storage.saveDatabaseToJson();
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка редактирования", QString::fromStdString(ex.what()));
        refreshTable();
    }
}

void WorkersTab::onRemoveWorker() {
    unsigned int selectedId = getSelectedWorkerId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор работника", "Выберите работника в таблице для удаления.");
        return;
    }
    const Master* m = storage.findMasterById(selectedId);
    if (!m) {
        return;
    }
    if (auto reply = QMessageBox::question(
            this,
            "Подтверждение удаления",
            QString("Удалить работника «%1» из штата мастерской?").arg(m->getName()),
            QMessageBox::Yes | QMessageBox::No
        ); reply != QMessageBox::Yes) {
        return;
    }
    try {
        storage.removeMaster(selectedId);
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Невозможно удалить работника", QString::fromStdString(ex.what()));
    }
}
