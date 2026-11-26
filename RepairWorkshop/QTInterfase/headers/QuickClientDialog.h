#ifndef QUICKCLIENTDIALOG_H
#define QUICKCLIENTDIALOG_H

#include "ClientsTab.h"
#include <QDialog>

class QuickClientDialog : public QDialog {
    Q_OBJECT

private:
    ClientsTab* clientsTab;

public:
    explicit QuickClientDialog(Storage& storageRef, QWidget* parent = nullptr);
    ~QuickClientDialog() override = default;

    unsigned int getLastAddedClientId() const;
};

#endif
