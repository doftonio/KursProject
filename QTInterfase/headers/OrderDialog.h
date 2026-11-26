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
#include <QGroupBox>
#include <QFrame>
#include <QEvent>

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

    QGroupBox* notesGroup;
    QFrame* notesOverlay = nullptr;
    QTextEdit* notesHistoryEdit;
    QLineEdit* newNoteEdit;

    QGroupBox* partsGroup;
    QFrame* partsOverlay = nullptr;
    QComboBox* availablePartsCombo;
    QSpinBox* partQtySpin;
    QTableWidget* usedPartsTable;
    QLabel* totalCostLabel;
    QPushButton* saveBtn;

    void buildUi();
    void setupLockedOverlays();
    QFrame* createSectionOverlay(QWidget* targetGroup);
    static void markFieldLocked(QWidget* widget);
    void populateCombos();
    void loadExistingOrder();
    void applyReadOnlyArchiveMode();
    void refreshUsedPartsTable();
    void refreshTotalLabel();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onClientComboActivated(int index);
    void onCategoryChanged(int index);
    void onAddNote();
    void onReservePart();
    void onReturnPart();
    void onExportReceipt();
    void onSave();

public:
    explicit OrderDialog(Storage& storageRef, unsigned int existingOrderId = 0, QWidget* parent = nullptr);
    ~OrderDialog() override = default;
};

#endif
