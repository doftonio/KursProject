#ifndef ACCOUNTINGWINDOW_H
#define ACCOUNTINGWINDOW_H

#include "../../Headers/Storage.h"
#include "../../Headers/AccountingLedger.h"
#include <QDialog>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>

class AccountingWindow : public QDialog {
    Q_OBJECT

private:
    const Storage& storage;
    AccountingLedger ledger;

    QDoubleSpinBox* minBaseSpin;
    QDoubleSpinBox* pieceworkShareSpin;

    QLineEdit* empNameEdit;
    QLineEdit* empPositionEdit;
    QDoubleSpinBox* empSalarySpin;
    QDoubleSpinBox* empRateSpin;

    QTableWidget* payrollTable;

    QLabel* grossRevenueLabel;
    QLabel* partsExpenseLabel;
    QLabel* mastersPayrollLabel;
    QLabel* staffPayrollLabel;
    QLabel* totalExpensesLabel;
    QLabel* netIncomeLabel;

    void buildUi();
    void loadParametersToUi();
    void refreshAccounting();
    unsigned int getSelectedStaffId() const;

private slots:
    void onApplyPayrollParams();
    void onAddStaffEmployee();
    void onUpdateStaffEmployee();
    void onRemoveStaffEmployee();

public:
    explicit AccountingWindow(const Storage& storageRef, QWidget* parent = nullptr);
    ~AccountingWindow() override = default;
};

#endif
