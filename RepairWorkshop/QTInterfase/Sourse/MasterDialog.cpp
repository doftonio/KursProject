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
    setWindowTitle("Управление персоналом (мастера сервисного центра)");
    resize(820, 540);

    auto* mainLayout = makeOwned<QVBoxLayout>(this);

    searchEdit = makeOwned<QLineEdit>(this);
    searchEdit->setPlaceholderText("Поиск мастера по ФИО, телефону или специализации...");
    mainLayout->addWidget(searchEdit);

    mastersTable = makeOwned<QTableWidget>(this);
    mastersTable->setColumnCount(5);
    mastersTable->setHorizontalHeaderLabels({"ID", "ФИО мастера", "Телефон", "Специализация", "Активных заказов"});
    mastersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mastersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    mastersTable->setAlternatingRowColors(true);
    mastersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mastersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mastersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(mastersTable, 1);

    auto* addGroup = makeOwned<QGroupBox>("Приём нового мастера в штат", this);
    auto* addLayout = makeOwned<QHBoxLayout>(addGroup);
    nameEdit = makeOwned<QLineEdit>(this);
    nameEdit->setPlaceholderText("ФИО специалиста");
    phoneEdit = makeOwned<QLineEdit>(this);
    InputValidator::setupPhoneMask(phoneEdit);
    addSpecCombo = makeOwned<QComboBox>(this);
    addSpecCombo->addItem("Компьютерная техника", "Computer");
    addSpecCombo->addItem("Бытовая техника", "Household");
    addSpecCombo->addItem("Радиоэлектроника", "Electronics");
    addBtn = makeOwned<QPushButton>("Добавить мастера", this);
    addBtn->setObjectName("primaryBtn");

    addLayout->addWidget(nameEdit, 3);
    addLayout->addWidget(phoneEdit, 2);
    addLayout->addWidget(addSpecCombo, 2);
    addLayout->addWidget(addBtn);
    mainLayout->addWidget(addGroup);

    auto* manageGroup = makeOwned<QGroupBox>("Управление выбранным сотрудником", this);
    auto* manageLayout = makeOwned<QHBoxLayout>(manageGroup);
    editSpecCombo = makeOwned<QComboBox>(this);
    editSpecCombo->addItem("Компьютерная техника", "Computer");
    editSpecCombo->addItem("Бытовая техника", "Household");
    editSpecCombo->addItem("Радиоэлектроника", "Electronics");
    updateSpecBtn = makeOwned<QPushButton>("Сохранить специализацию", this);
    removeBtn = makeOwned<QPushButton>("Удалить мастера", this);
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
    connect(mastersTable, &QTableWidget::itemSelectionChanged, this, &MasterDialog::onSelectionChanged);
    connect(addBtn, &QPushButton::clicked, this, &MasterDialog::onAddMaster);
    connect(updateSpecBtn, &QPushButton::clicked, this, &MasterDialog::onUpdateSpecialization);
    connect(removeBtn, &QPushButton::clicked, this, &MasterDialog::onRemoveMaster);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void MasterDialog::refreshTable() {
    std::vector<Master> list = WorkshopSearchService::searchMasters(storage, searchEdit->text());
    mastersTable->setRowCount(static_cast<int>(list.size()));
    for (int row = 0; row < static_cast<int>(list.size()); ++row) {
        const Master& m = list[static_cast<std::size_t>(row)];
        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(m.getId()));
        idItem->setData(Qt::UserRole, m.getId());
        mastersTable->setItem(row, 0, idItem);
        mastersTable->setItem(row, 1, makeOwned<QTableWidgetItem>(m.getName()));
        mastersTable->setItem(row, 2, makeOwned<QTableWidgetItem>(m.getPhone()));
        mastersTable->setItem(row, 3, makeOwned<QTableWidgetItem>(formatSpecRu(m.getSpecialization())));
        mastersTable->setItem(row, 4, makeOwned<QTableWidgetItem>(QString("%1 / %2").arg(m.getActiveOrdersCount()).arg(Master::maxOrdersPerMaster)));
    }
}

unsigned int MasterDialog::getSelectedMasterId() const {
    int row = mastersTable->currentRow();
    if (row < 0) {
        return 0;
    }
    const QTableWidgetItem* item = mastersTable->item(row, 0);
    return item ? item->data(Qt::UserRole).toUInt() : 0;
}

void MasterDialog::onSearchChanged(const QString&) {
    refreshTable();
}

void MasterDialog::onSelectionChanged() {
    unsigned int selectedId = getSelectedMasterId();
    if (selectedId == 0) {
        return;
    }
    if (const Master* m = storage.findMasterById(selectedId); m != nullptr) {
        if (int idx = editSpecCombo->findData(m->getSpecialization()); idx >= 0) {
            editSpecCombo->setCurrentIndex(idx);
        }
    }
}

void MasterDialog::onAddMaster() {
    try {
        QString spec = addSpecCombo->currentData().toString();
        storage.addMaster(nameEdit->text(), phoneEdit->text(), spec);
        nameEdit->clear();
        phoneEdit->clear();
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка добавления мастера", QString::fromStdString(ex.what()));
    }
}

void MasterDialog::onUpdateSpecialization() {
    unsigned int selectedId = getSelectedMasterId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор сотрудника", "Выберите мастера в таблице.");
        return;
    }
    try {
        QString newSpec = editSpecCombo->currentData().toString();
        storage.updateMasterSpecialization(selectedId, newSpec);
        refreshTable();
        QMessageBox::information(this, "Обновлено", "Специализация мастера успешно обновлена.");
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка обновления", QString::fromStdString(ex.what()));
    }
}

void MasterDialog::onRemoveMaster() {
    unsigned int selectedId = getSelectedMasterId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор сотрудника", "Выберите мастера в таблице для удаления.");
        return;
    }
    const Master* m = storage.findMasterById(selectedId);
    if (!m) {
        return;
    }
    if (auto reply = QMessageBox::question(
            this,
            "Подтверждение удаления",
            QString("Удалить мастера «%1» из штата мастерской?").arg(m->getName()),
            QMessageBox::Yes | QMessageBox::No
        ); reply != QMessageBox::Yes) {
        return;
    }
    try {
        storage.removeMaster(selectedId);
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Невозможно удалить мастера", QString::fromStdString(ex.what()));
    }
}
