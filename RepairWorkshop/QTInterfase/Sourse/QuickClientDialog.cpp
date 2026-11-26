#include "../headers/QuickClientDialog.h"
#include "../headers/UiHelpers.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

QuickClientDialog::QuickClientDialog(Storage& storageRef, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Добавление клиента");
    resize(760, 480);

    auto* layout = makeOwned<QVBoxLayout>(this);
    clientsTab = makeOwned<ClientsTab>(storageRef, this);
    layout->addWidget(clientsTab, 1);

    auto* bottom = makeOwned<QHBoxLayout>();
    auto* doneBtn = makeOwned<QPushButton>("Готово", this);
    doneBtn->setObjectName("primaryBtn");
    bottom->addStretch(1);
    bottom->addWidget(doneBtn);
    layout->addLayout(bottom);

    connect(doneBtn, &QPushButton::clicked, this, &QDialog::accept);
}

unsigned int QuickClientDialog::getLastAddedClientId() const {
    return clientsTab ? clientsTab->getLastAddedClientId() : 0U;
}
