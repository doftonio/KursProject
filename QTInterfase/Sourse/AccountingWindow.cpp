#include "../headers/AccountingWindow.h"
#include "../headers/UiHelpers.h"
#include "../../Exeptions/AppException.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>

AccountingWindow::AccountingWindow(const Storage& storageRef, QWidget* parent)
    : QDialog(parent), storage(storageRef), ledger(storageRef) {
    buildUi();
    loadParametersToUi();
    refreshAccounting();
}

void AccountingWindow::buildUi() {
    setWindowTitle("Бухгалтерия мастерской — доходы, расходы и расчёт зарплаты");
    resize(1080, 680);

    auto* mainLayout = makeOwned<QVBoxLayout>(this);
    mainLayout->setSpacing(10);

    auto* paramsGroup = makeOwned<QGroupBox>("Общие переменные оплаты труда", this);
    auto* paramsLayout = makeOwned<QHBoxLayout>(paramsGroup);

    minBaseSpin = makeOwned<QDoubleSpinBox>(this);
    minBaseSpin->setRange(0.0, 50000.0);
    minBaseSpin->setDecimals(2);
    minBaseSpin->setSuffix(" BYN");

    pieceworkShareSpin = makeOwned<QDoubleSpinBox>(this);
    pieceworkShareSpin->setRange(0.0, 100.0);
    pieceworkShareSpin->setDecimals(1);
    pieceworkShareSpin->setSuffix(" %");

    auto* applyParamsBtn = makeOwned<QPushButton>("Пересчитать", this);
    applyParamsBtn->setObjectName("primaryBtn");

    paramsLayout->addWidget(makeOwned<QLabel>("Минимальная база:", this));
    paramsLayout->addWidget(minBaseSpin);
    paramsLayout->addWidget(makeOwned<QLabel>("Сдельная доля:", this));
    paramsLayout->addWidget(pieceworkShareSpin);
    paramsLayout->addWidget(applyParamsBtn);
    paramsLayout->addStretch(1);

    payrollTable = makeOwned<QTableWidget>(this);
    payrollTable->setColumnCount(7);
    payrollTable->setHorizontalHeaderLabels({
        "ID", "ФИО работника", "Должность", "Оклад", "Сдельно", "Ставка", "Итого к выплате"
    });
    payrollTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    payrollTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    payrollTable->setAlternatingRowColors(true);
    payrollTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    payrollTable->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* summaryGroup = makeOwned<QGroupBox>("Финансовый отчёт — доходы, расходы и чистая прибыль", this);
    auto* grid = makeOwned<QGridLayout>(summaryGroup);

    grossRevenueLabel = makeOwned<QLabel>(this);
    partsExpenseLabel = makeOwned<QLabel>(this);
    mastersPayrollLabel = makeOwned<QLabel>(this);
    staffPayrollLabel = makeOwned<QLabel>(this);
    totalExpensesLabel = makeOwned<QLabel>(this);
    netIncomeLabel = makeOwned<QLabel>(this);
    netIncomeLabel->setObjectName("totalHighlight");

    grid->addWidget(makeOwned<QLabel>("Валовый доход (работы + запчасти):", this), 0, 0);
    grid->addWidget(grossRevenueLabel, 0, 1);
    grid->addWidget(makeOwned<QLabel>("Расходы на запчасти:", this), 0, 2);
    grid->addWidget(partsExpenseLabel, 0, 3);

    grid->addWidget(makeOwned<QLabel>("Сдельная / базовая часть работников:", this), 1, 0);
    grid->addWidget(mastersPayrollLabel, 1, 1);
    grid->addWidget(makeOwned<QLabel>("Выплаты по окладам и ставкам:", this), 1, 2);
    grid->addWidget(staffPayrollLabel, 1, 3);

    grid->addWidget(makeOwned<QLabel>("Общие расходы предприятия:", this), 2, 0);
    grid->addWidget(totalExpensesLabel, 2, 1);
    grid->addWidget(makeOwned<QLabel>("Чистый доход (прибыль):", this), 2, 2);
    grid->addWidget(netIncomeLabel, 2, 3);

    auto* bottomLayout = makeOwned<QHBoxLayout>();
    auto* closeBtn = makeOwned<QPushButton>("Закрыть", this);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(closeBtn);

    connect(applyParamsBtn, &QPushButton::clicked, this, &AccountingWindow::onApplyPayrollParams);
    connect(payrollTable, &QTableWidget::cellChanged, this, &AccountingWindow::onPayrollCellChanged);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    mainLayout->addWidget(paramsGroup);
    mainLayout->addWidget(payrollTable, 1);
    mainLayout->addWidget(summaryGroup);
    mainLayout->addLayout(bottomLayout);
}

void AccountingWindow::loadParametersToUi() {
    minBaseSpin->setValue(ledger.getMasterSharedMinBase());
    pieceworkShareSpin->setValue(ledger.getPieceworkShare() * 100.0);
}

void AccountingWindow::refreshSummaryOnly() {
    AccountingSummary sum = ledger.calculateSummary(storage);
    grossRevenueLabel->setText(QString("%1 BYN").arg(sum.grossRevenue, 0, 'f', 2));
    partsExpenseLabel->setText(QString("%1 BYN").arg(sum.partsExpense, 0, 'f', 2));
    mastersPayrollLabel->setText(QString("%1 BYN").arg(sum.mastersPayroll, 0, 'f', 2));
    staffPayrollLabel->setText(QString("%1 BYN").arg(sum.staffPayroll, 0, 'f', 2));
    totalExpensesLabel->setText(QString("%1 BYN").arg(sum.totalExpenses, 0, 'f', 2));

    netIncomeLabel->setText(QString("%1 BYN").arg(sum.netIncome, 0, 'f', 2));
    if (sum.netIncome < 0.0) {
        netIncomeLabel->setStyleSheet("color: #DC2626;");
    } else {
        netIncomeLabel->setStyleSheet("color: #059669;");
    }
}

void AccountingWindow::refreshAccounting() {
    populatingTable = true;
    auto payroll = ledger.buildPayrollList(storage);
    payrollTable->setRowCount(static_cast<int>(payroll.size()));

    for (int row = 0; row < static_cast<int>(payroll.size()); ++row) {
        const auto& emp = payroll[static_cast<std::size_t>(row)];
        bool isMaster = (emp->getEmployeeType() == "Master");

        double fixedSalaryVal = 0.0;
        double pieceworkVal = 0.0;
        if (isMaster) {
            if (const auto* mEmp = dynamic_cast<const MasterEmployee*>(emp.get()); mEmp != nullptr) {
                fixedSalaryVal = mEmp->getSharedMinBase();
                pieceworkVal = mEmp->getPieceworkEarnings();
            }
        } else {
            if (const auto* sEmp = dynamic_cast<const StaffEmployee*>(emp.get()); sEmp != nullptr) {
                fixedSalaryVal = sEmp->getBaseSalary();
            }
        }

        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(emp->getId()));
        idItem->setData(Qt::UserRole, emp->getId());
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

        auto* nameItem = makeOwned<QTableWidgetItem>(emp->getName());
        auto* posItem = makeOwned<QTableWidgetItem>(emp->getPosition());
        auto* salaryItem = makeOwned<QTableWidgetItem>(QString::number(fixedSalaryVal, 'f', 2));
        if (isMaster) {
            salaryItem->setFlags(salaryItem->flags() & ~Qt::ItemIsEditable);
        }

        auto* pieceworkItem = makeOwned<QTableWidgetItem>(isMaster ? QString::number(pieceworkVal, 'f', 2) : "—");
        pieceworkItem->setFlags(pieceworkItem->flags() & ~Qt::ItemIsEditable);

        auto* rateItem = makeOwned<QTableWidgetItem>(QString::number(emp->getRateMultiplier(), 'f', 2));

        auto* totalItem = makeOwned<QTableWidgetItem>(QString("%1 BYN").arg(emp->calculateSalary(), 0, 'f', 2));
        totalItem->setFlags(totalItem->flags() & ~Qt::ItemIsEditable);

        payrollTable->setItem(row, 0, idItem);
        payrollTable->setItem(row, 1, nameItem);
        payrollTable->setItem(row, 2, posItem);
        payrollTable->setItem(row, 3, salaryItem);
        payrollTable->setItem(row, 4, pieceworkItem);
        payrollTable->setItem(row, 5, rateItem);
        payrollTable->setItem(row, 6, totalItem);
    }

    populatingTable = false;
    refreshSummaryOnly();
}

void AccountingWindow::onApplyPayrollParams() {
    try {
        ledger.setPayrollParameters(minBaseSpin->value(), pieceworkShareSpin->value() / 100.0);
        refreshAccounting();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка параметров", QString::fromStdString(ex.what()));
    }
}

void AccountingWindow::onPayrollCellChanged(int row, int) {
    if (populatingTable || row < 0) {
        return;
    }

    const QTableWidgetItem* idItem = payrollTable->item(row, 0);
    const QTableWidgetItem* nameItem = payrollTable->item(row, 1);
    const QTableWidgetItem* posItem = payrollTable->item(row, 2);
    const QTableWidgetItem* salaryItem = payrollTable->item(row, 3);
    const QTableWidgetItem* rateItem = payrollTable->item(row, 5);
    if (!idItem || !nameItem || !posItem || !salaryItem || !rateItem) {
        return;
    }

    unsigned int workerId = idItem->data(Qt::UserRole).toUInt();
    bool okSalary = false;
    bool okRate = false;
    double salaryVal = salaryItem->text().replace(',', '.').toDouble(&okSalary);
    double rateVal = rateItem->text().replace(',', '.').remove('x').toDouble(&okRate);

    if (!okSalary || !okRate) {
        QMessageBox::warning(this, "Ошибка ввода", "Введите корректные числовые значения для оклада и ставки.");
        refreshAccounting();
        return;
    }

    try {
        ledger.updateStaffEmployeeRow(workerId, nameItem->text(), posItem->text(), salaryVal, rateVal);
        refreshAccounting();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка сохранения строки", QString::fromStdString(ex.what()));
        refreshAccounting();
    }
}
