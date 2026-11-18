#include "../headers/ClientDialog.h"
#include "../../Headers/InputValidator.h"
#include "../../Exeptions/AppException.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>

ClientDialog::ClientDialog(Storage& storageRef, QWidget* parent)
    : QDialog(parent), storage(storageRef) {
    buildUi();
    refreshTable();
}

void ClientDialog::buildUi() {
    setWindowTitle("Справочник клиентов мастерской");
    resize(740, 500);

    auto* mainLayout = new QVBoxLayout(this);

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск клиента по ФИО или номеру телефона...");
    mainLayout->addWidget(searchEdit);

    clientsTable = new QTableWidget(this);
    clientsTable->setColumnCount(4);
    clientsTable->setHorizontalHeaderLabels({"ID", "ФИО клиента", "Телефон", "Примечание"});
    clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    clientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    clientsTable->setAlternatingRowColors(true);
    clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(clientsTable, 1);

    auto* addGroup = new QGroupBox("Регистрация нового клиента", this);
    auto* addLayout = new QHBoxLayout(addGroup);
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("ФИО заказчика");
    phoneEdit = new QLineEdit(this);
    InputValidator::setupPhoneMask(phoneEdit);
    noteEdit = new QLineEdit(this);
    noteEdit->setPlaceholderText("Примечание");
    addBtn = new QPushButton("Добавить клиента", this);
    addBtn->setObjectName("primaryBtn");

    addLayout->addWidget(nameEdit, 2);
    addLayout->addWidget(phoneEdit, 2);
    addLayout->addWidget(noteEdit, 2);
    addLayout->addWidget(addBtn);
    mainLayout->addWidget(addGroup);

    closeBtn = new QPushButton("Закрыть", this);
    mainLayout->addWidget(closeBtn);

    connect(searchEdit, &QLineEdit::textChanged, this, &ClientDialog::onSearchChanged);
    connect(addBtn, &QPushButton::clicked, this, &ClientDialog::onAddClient);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void ClientDialog::refreshTable() {
    std::vector<Client> list = storage.searchClients(searchEdit->text());
    clientsTable->setRowCount(static_cast<int>(list.size()));
    for (int row = 0; row < static_cast<int>(list.size()); ++row) {
        const Client& c = list[static_cast<std::size_t>(row)];
        clientsTable->setItem(row, 0, new QTableWidgetItem(QString::number(c.getId())));
        clientsTable->setItem(row, 1, new QTableWidgetItem(c.getName()));
        clientsTable->setItem(row, 2, new QTableWidgetItem(c.getPhone()));
        clientsTable->setItem(row, 3, new QTableWidgetItem(c.getNote()));
    }
}

void ClientDialog::onSearchChanged(const QString&) {
    refreshTable();
}

void ClientDialog::onAddClient() {
    try {
        storage.addClient(nameEdit->text(), phoneEdit->text(), noteEdit->text());
        nameEdit->clear();
        phoneEdit->clear();
        noteEdit->clear();
        refreshTable();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка регистрации клиента", QString::fromStdString(ex.what()));
    }
}
