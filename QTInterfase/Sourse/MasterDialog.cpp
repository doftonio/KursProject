#include "../headers/MasterDialog.h"
#include "../headers/UiHelpers.h"
#include "../../Headers/WorkshopSearchService.h"
#include "../../Headers/InputValidator.h"
#include "../../Exeptions/AppException.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QLabel>

MasterDialog::MasterDialog(Storage& storageRef, QWidget* parent)
    : QDialog(parent), storage(storageRef) {
    buildUi();
    refreshTable();
}

QString MasterDialog::formatSpecRu(const QString& spec) {
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

void MasterDialog::buildUi() {
    setWindowTitle("Работники сервисного центра");
    resize(820, 540);

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
    workersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
    addBtn = makeOwned<QPushButton>("Добавить работника", this);
    addBtn->setObjectName("primaryBtn");

    addLayout->addWidget(nameEdit, 3);
    addLayout->addWidget(phoneEdit, 2);
    addLayout->addWidget(addSpecCombo, 2);
    addLayout->addWidget(addBtn);
    mainLayout->addWidget(addGroup);

    auto* manageGroup = makeOwned<QGroupBox>("Управление выбранным работником", this);
    auto* manageLayout = makeOwned<QHBoxLayout>(manageGroup);
    editSpecCombo = makeOwned<QComboBox>(this);
    editSpecCombo->addItem("Компьютерная техника", "Computer");
    editSpecCombo->addItem("Бытовая техника", "Household");
    editSpecCombo->addItem("Радиоэлектроника", "Electronics");
    updateSpecBtn = makeOwned<QPushButton>("Сохранить профиль", this);
    removeBtn = makeOwned<QPushButton>("Удалить работника", this);
    removeBtn->setObjectName("dangerBtn");
    closeBtn = makeOwned<QPushButton>("Закрыть", this);

    manageLayout->addWidget(makeOwned<QLabel>("Профиль:", this));
    manageLayout->addWidget(editSpecCombo, 1);
    manageLayout->addWidget(updateSpecBtn);
    manageLayout->addWidget(removeBtn);
    manageLayout->addStretch(1);
    manageLayout->addWidget(closeBtn);
    mainLayout->addWidget(manageGroup);

    connect(searchEdit, &QLineEdit::textChanged, this, &MasterDialog::onSearchChanged);
    connect(workersTable, &QTableWidget::itemSelectionChanged, this, &MasterDialog::onSelectionChanged);
    connect(addBtn, &QPushButton::clicked, this, &MasterDialog::onAddWorker);
    connect(updateSpecBtn, &QPushButton::clicked, this, &MasterDialog::onUpdateSpecialization);
    connect(removeBtn, &QPushButton::clicked, this, &MasterDialog::onRemoveWorker);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void MasterDialog::refreshTable() {
    std::vector<Master> list = WorkshopSearchService::searchMasters(storage, searchEdit->text());
    workersTable->setRowCount(static_cast<int>(list.size()));
    for (int row = 0; row < static_cast<int>(list.size()); ++row) {
        const Master& m = list[static_cast<std::size_t>(row)];
        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(m.getId()));
        idItem->setData(Qt::UserRole, m.getId());
        workersTable->setItem(row, 0, idItem);
        workersTable->setItem(row, 1, makeOwned<QTableWidgetItem>(m.getName()));
        workersTable->setItem(row, 2, makeOwned<QTableWidgetItem>(m.getPhone()));
        workersTable->setItem(row, 3, makeOwned<QTableWidgetItem>(formatSpecRu(m.getSpecialization())));
        workersTable->setItem(row, 4, makeOwned<QTableWidgetItem>(QString("%1 / %2").arg(m.getActiveOrdersCount()).arg(Master::maxOrdersPerMaster)));
    }
}

unsigned int MasterDialog::getSelectedWorkerId() const {
    int row = workersTable->currentRow();
    if (row < 0) {
        return 0;
    }
    const QTableWidgetItem* item = workersTable->item(row, 0);
    return item ? item->data(Qt::UserRole).toUInt() : 0;
}

void MasterDialog::onSearchChanged(const QString&) {
    refreshTable();
}

void MasterDialog::onSelectionChanged() {
    unsigned int selectedId = getSelectedWorkerId();
    if (selectedId == 0) {
        return;
    }
    if (const Master* m = storage.findMasterById(selectedId); m != nullptr) {
        if (int idx = editSpecCombo->findData(m->getSpecialization()); idx >= 0) {
            editSpecCombo->setCurrentIndex(idx);
        }
    }
}

void MasterDialog::onAddWorker() {
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

void MasterDialog::onUpdateSpecialization() {
    unsigned int selectedId = getSelectedWorkerId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор работника", "Выберите работника в таблице.");
        return;
    }
    try {
        QString newSpec = editSpecCombo->currentData().toString();
        storage.updateMasterSpecialization(selectedId, newSpec);
        refreshTable();
        QMessageBox::information(this, "Обновлено", "Профиль работника успешно обновлён.");
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка обновления", QString::fromStdString(ex.what()));
    }
}

void MasterDialog::onRemoveWorker() {
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
