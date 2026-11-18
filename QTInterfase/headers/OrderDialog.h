#ifndef ORDERDIALOG_H
#define ORDERDIALOG_H

#include "../../Headers/Storage.h"
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

class OrderDialog : public QDialog {
    Q_OBJECT

private:
    Storage& storage;
    unsigned int orderId;
    bool editMode;

    QComboBox* clientCombo;
    QComboBox* masterCombo;
    QComboBox* categoryCombo;
    QLineEdit* modelEdit;
    QLineEdit* serialEdit;
    QLineEdit* extraSpecEdit;
    QCheckBox* extraFlagCheck;
    QTextEdit* faultEdit;
    QComboBox* statusCombo;
    QDoubleSpinBox* laborSpin;

    QTextEdit* notesHistoryEdit;
    QLineEdit* newNoteEdit;
    QPushButton* addNoteBtn;

    QComboBox* availablePartsCombo;
    QSpinBox* partQtySpin;
    QPushButton* reservePartBtn;
    QPushButton* returnPartBtn;
    QTableWidget* usedPartsTable;
    QLabel* totalCostLabel;

    void buildUi();
    void populateCombos();
    void loadExistingOrder();
    void refreshUsedPartsTable();
    void refreshTotalLabel();

private slots:
    void onCategoryChanged(int index);
    void onAddNote();
    void onReservePart();
    void onReturnPart();
    void onSave();

public:
    OrderDialog(Storage& storageRef, unsigned int existingOrderId = 0, QWidget* parent = nullptr);
    ~OrderDialog() override = default;
};

#endif
