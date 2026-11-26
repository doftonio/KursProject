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

struct DeviceFormWidgets {
    QComboBox* client = nullptr;
    QComboBox* worker = nullptr;
    QComboBox* category = nullptr;
    QLineEdit* model = nullptr;
    QLineEdit* serial = nullptr;
    QLineEdit* extraSpec = nullptr;
    QCheckBox* extraFlag = nullptr;
    QTextEdit* fault = nullptr;
};

struct OrderPanelsWidgets {
    QGroupBox* notesGroup = nullptr;
    QFrame* notesOverlay = nullptr;
    QTextEdit* notesHistory = nullptr;
    QLineEdit* newNote = nullptr;

    QGroupBox* partsGroup = nullptr;
    QFrame* partsOverlay = nullptr;
    QComboBox* availableParts = nullptr;
    QSpinBox* partQty = nullptr;
    QTableWidget* usedParts = nullptr;
};

class OrderDialog : public QDialog {
    Q_OBJECT

private:
    Storage& storage;
    unsigned int orderId;
    bool editMode;

    DeviceFormWidgets form;
    OrderPanelsWidgets panels;

    QComboBox* statusCombo;
    QDoubleSpinBox* laborSpin;
    QLabel* totalCostLabel;
    QPushButton* receiptBtn;
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
    std::unique_ptr<Device> buildDeviceFromForm() const;
    void saveNewOrder();
    void saveExistingOrder();

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
