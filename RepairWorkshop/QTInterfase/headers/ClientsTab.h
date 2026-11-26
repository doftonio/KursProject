#ifndef CLIENTSTAB_H
#define CLIENTSTAB_H

#include "../../Headers/Storage.h"
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>

class ClientsTab : public QWidget {
    Q_OBJECT

private:
    Storage& storage;
    unsigned int lastAddedClientId = 0;

    QLineEdit* searchEdit;
    QTableWidget* clientsTable;
    QLineEdit* nameEdit;
    QLineEdit* phoneEdit;
    QLineEdit* noteEdit;

    void buildUi();

private slots:
    void onSearchChanged(const QString& text);
    void onAddClient();

public:
    explicit ClientsTab(Storage& storageRef, QWidget* parent = nullptr);
    ~ClientsTab() override = default;

    void refreshTable();
    unsigned int getLastAddedClientId() const;
};

#endif
