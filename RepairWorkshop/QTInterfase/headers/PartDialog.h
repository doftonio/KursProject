#ifndef PARTDIALOG_H
#define PARTDIALOG_H

#include "../../Headers/Storage.h"
#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>

class PartDialog : public QDialog {
    Q_OBJECT

private:
    Storage& storage;

    QLineEdit* searchEdit;
    QTableWidget* partsTable;
    QLineEdit* skuEdit;
    QLineEdit* nameEdit;
    QSpinBox* qtySpin;
    QDoubleSpinBox* priceSpin;
    QPushButton* addPartBtn;
    QSpinBox* restockSpin;
    QPushButton* restockBtn;
    QPushButton* closeBtn;

    void buildUi();
    void refreshTable();

private slots:
    void onSearchChanged(const QString& text);
    void onAddPart();
    void onRestockSelected();

public:
    explicit PartDialog(Storage& storageRef, QWidget* parent = nullptr);
    ~PartDialog() override = default;
};

#endif
