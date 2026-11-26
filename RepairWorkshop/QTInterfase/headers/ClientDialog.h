#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include "../../Headers/Storage.h"
#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>

class ClientDialog : public QDialog {
    Q_OBJECT

private:
    Storage& storage;
    unsigned int lastAddedClientId = 0;

    QLineEdit* searchEdit;
    QTableWidget* clientsTable;
    QLineEdit* nameEdit;
    QLineEdit* phoneEdit;
    QLineEdit* noteEdit;
    QPushButton* addBtn;
    QPushButton* closeBtn;

    void buildUi();
    void refreshTable();

private slots:
    void onSearchChanged(const QString& text);
    void onAddClient();

public:
    explicit ClientDialog(Storage& storageRef, QWidget* parent = nullptr);
    ~ClientDialog() override = default;

    unsigned int getLastAddedClientId() const;
};

#endif
