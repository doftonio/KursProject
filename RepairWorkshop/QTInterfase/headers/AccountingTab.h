#ifndef ACCOUNTINGTAB_H
#define ACCOUNTINGTAB_H

#include "../../Headers/Storage.h"
#include "../../Headers/AccountingLedger.h"
#include <QWidget>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QLabel>

class AccountingTab : public QWidget {
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
    void refreshSummaryOnly();

private slots:
    void onApplyPayrollParams();
    void onPayrollCellChanged(int row, int column);

public:
    explicit AccountingTab(const Storage& storageRef, QWidget* parent = nullptr);
    ~AccountingTab() override = default;

    void refreshAccounting();
};

#endif
