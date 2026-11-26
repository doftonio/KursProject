#ifndef WAREHOUSETAB_H
#define WAREHOUSETAB_H

#include "../../Headers/Storage.h"
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>

class WarehouseTab : public QWidget {
    Q_OBJECT

private:
    Storage& storage;

    QLineEdit* searchEdit;
    QTableWidget* partsTable;
    QLineEdit* skuEdit;
    QLineEdit* nameEdit;
    QSpinBox* qtySpin;
    QDoubleSpinBox* priceSpin;
    QSpinBox* restockSpin;

    void buildUi();

private slots:
    void onSearchChanged(const QString& text);
    void onAddPart();
    void onRestockSelected();

public:
    explicit WarehouseTab(Storage& storageRef, QWidget* parent = nullptr);
    ~WarehouseTab() override = default;

    void refreshTable();
};

#endif
