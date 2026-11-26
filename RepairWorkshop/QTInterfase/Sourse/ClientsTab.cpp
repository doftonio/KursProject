#include "../headers/ClientsTab.h"
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

ClientsTab::ClientsTab(Storage& storageRef, QWidget* parent)
    : QWidget(parent), storage(storageRef) {
    buildUi();
    refreshTable();
}

unsigned int ClientsTab::getLastAddedClientId() const {
    return lastAddedClientId;
}

void ClientsTab::buildUi() {
    auto* mainLayout = makeOwned<QVBoxLayout>(this);

    searchEdit = makeOwned<QLineEdit>(this);
    searchEdit->setPlaceholderText("Поиск клиента по ФИО или номеру телефона...");
    mainLayout->addWidget(searchEdit);

    clientsTable = makeOwned<QTableWidget>(this);
    clientsTable->setColumnCount(4);
    clientsTable->setHorizontalHeaderLabels({"ID", "ФИО клиента", "Телефон", "Примечание"});
    clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    clientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    clientsTable->setAlternatingRowColors(true);
    clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(clientsTable, 1);

    auto* addGroup = makeOwned<QGroupBox>("Регистрация нового клиента", this);
    auto* addLayout = makeOwned<QHBoxLayout>(addGroup);
    nameEdit = makeOwned<QLineEdit>(this);
    nameEdit->setPlaceholderText("ФИО заказчика");
    phoneEdit = makeOwned<QLineEdit>(this);
    InputValidator::setupPhoneMask(phoneEdit);
    noteEdit = makeOwned<QLineEdit>(this);
    noteEdit->setPlaceholderText("Примечание");
    auto* addBtn = makeOwned<QPushButton>("Добавить клиента", this);
    addBtn->setObjectName("primaryBtn");

    addLayout->addWidget(nameEdit, 2);
    addLayout->addWidget(phoneEdit, 2);
    addLayout->addWidget(noteEdit, 2);
    addLayout->addWidget(addBtn);
    mainLayout->addWidget(addGroup);

    connect(searchEdit, &QLineEdit::textChanged, this, &ClientsTab::onSearchChanged);
    connect(addBtn, &QPushButton::clicked, this, &ClientsTab::onAddClient);
}

void ClientsTab::refreshTable() {
    std::vector<Client> list = WorkshopSearchService::searchClients(storage, searchEdit->text());
    clientsTable->setRowCount(static_cast<int>(list.size()));
    for (int row = 0; row < static_cast<int>(list.size()); ++row) {
        const Client& c = list[static_cast<std::size_t>(row)];
        clientsTable->setItem(row, 0, makeOwned<QTableWidgetItem>(QString::number(c.getId())));
        clientsTable->setItem(row, 1, makeOwned<QTableWidgetItem>(c.getName()));
        clientsTable->setItem(row, 2, makeOwned<QTableWidgetItem>(c.getPhone()));
        clientsTable->setItem(row, 3, makeOwned<QTableWidgetItem>(c.getNote()));
    }
}

void ClientsTab::onSearchChanged(const QString&) {
    refreshTable();
}

void ClientsTab::onAddClient() {
    try {
        const Client& created = storage.addClient(nameEdit->text(), phoneEdit->text(), noteEdit->text());
        lastAddedClientId = created.getId();
        nameEdit->clear();
        phoneEdit->clear();
        noteEdit->clear();
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка регистрации клиента", QString::fromStdString(ex.what()));
    }
}
