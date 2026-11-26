#ifndef ACCOUNTINGWINDOW_H
#define ACCOUNTINGWINDOW_H

#include "../../Headers/Storage.h"
#include "../../Headers/AccountingLedger.h"
#include <QDialog>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QLabel>

class AccountingWindow : public QDialog {
    Q_OBJECT

private:
    const Storage& storage;
    AccountingLedger ledger;
    bool populatingTable = false;

    QDoubleSpinBox* minBaseSpin;
    QDoubleSpinBox* pieceworkShareSpin;

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
    void refreshSummaryOnly();

private slots:
    void onApplyPayrollParams();
    void onPayrollCellChanged(int row, int column);

public:
    explicit AccountingWindow(const Storage& storageRef, QWidget* parent = nullptr);
    ~AccountingWindow() override = default;
};

#endif
