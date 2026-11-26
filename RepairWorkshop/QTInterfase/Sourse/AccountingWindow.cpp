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
    : QDialog(parent), storage(storageRef) {
    buildUi();
    loadParametersToUi();
    refreshAccounting();
}

void AccountingWindow::buildUi() {
    setWindowTitle("Бухгалтерия мастерской — доходы, расходы и расчёт зарплаты");
    resize(1060, 700);

    auto* mainLayout = makeOwned<QVBoxLayout>(this);
    mainLayout->setSpacing(10);

    auto* paramsGroup = makeOwned<QGroupBox>("Параметры оплаты труда мастеров (общие для всех мастеров)", this);
    auto* paramsLayout = makeOwned<QHBoxLayout>(paramsGroup);

    minBaseSpin = makeOwned<QDoubleSpinBox>(this);
    minBaseSpin->setRange(0.0, 50000.0);
    minBaseSpin->setDecimals(2);
    minBaseSpin->setSuffix(" BYN");

    pieceworkShareSpin = makeOwned<QDoubleSpinBox>(this);
    pieceworkShareSpin->setRange(0.0, 100.0);
    pieceworkShareSpin->setDecimals(1);
    pieceworkShareSpin->setSuffix(" %");

    auto* applyParamsBtn = makeOwned<QPushButton>("Пересчитать с параметрами", this);
    applyParamsBtn->setObjectName("primaryBtn");

    paramsLayout->addWidget(makeOwned<QLabel>("Минимальная база мастера:", this));
    paramsLayout->addWidget(minBaseSpin);
    paramsLayout->addWidget(makeOwned<QLabel>("Сдельная доля от работ:", this));
    paramsLayout->addWidget(pieceworkShareSpin);
    paramsLayout->addWidget(applyParamsBtn);
    paramsLayout->addStretch(1);

    auto* staffGroup = makeOwned<QGroupBox>("Работники без типа «Мастер» (фиксированный оклад + ставка-множитель)", this);
    auto* staffLayout = makeOwned<QHBoxLayout>(staffGroup);

    empNameEdit = makeOwned<QLineEdit>(this);
    empNameEdit->setPlaceholderText("ФИО работника");

    empPositionEdit = makeOwned<QLineEdit>(this);
    empPositionEdit->setPlaceholderText("Должность (напр. Бухгалтер)");

    empSalarySpin = makeOwned<QDoubleSpinBox>(this);
    empSalarySpin->setRange(0.0, 50000.0);
    empSalarySpin->setDecimals(2);
    empSalarySpin->setSuffix(" BYN");

    empRateSpin = makeOwned<QDoubleSpinBox>(this);
    empRateSpin->setRange(0.1, 10.0);
    empRateSpin->setDecimals(2);
    empRateSpin->setSingleStep(0.1);
    empRateSpin->setValue(1.0);
    empRateSpin->setPrefix("x");

    auto* addEmpBtn = makeOwned<QPushButton>("Добавить работника", this);
    addEmpBtn->setObjectName("primaryBtn");
    auto* updateEmpBtn = makeOwned<QPushButton>("Обновить выбранного", this);
    auto* removeEmpBtn = makeOwned<QPushButton>("Удалить", this);
    removeEmpBtn->setObjectName("dangerBtn");

    staffLayout->addWidget(empNameEdit, 2);
    staffLayout->addWidget(empPositionEdit, 2);
    staffLayout->addWidget(makeOwned<QLabel>("Оклад:", this));
    staffLayout->addWidget(empSalarySpin);
    staffLayout->addWidget(makeOwned<QLabel>("Ставка:", this));
    staffLayout->addWidget(empRateSpin);
    staffLayout->addWidget(addEmpBtn);
    staffLayout->addWidget(updateEmpBtn);
    staffLayout->addWidget(removeEmpBtn);

    payrollTable = makeOwned<QTableWidget>(this);
    payrollTable->setColumnCount(7);
    payrollTable->setHorizontalHeaderLabels({
        "Тип", "ФИО / Должность", "База / Оклад", "Сдельно (заказы)", "Ставка", "Итого к выплате", "ID"
    });
    payrollTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    payrollTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    payrollTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    payrollTable->setAlternatingRowColors(true);
    payrollTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    payrollTable->setSelectionMode(QAbstractItemView::SingleSelection);
    payrollTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

    grid->addWidget(makeOwned<QLabel>("Зарплата мастеров (база + сдельная):", this), 1, 0);
    grid->addWidget(mastersPayrollLabel, 1, 1);
    grid->addWidget(makeOwned<QLabel>("Зарплата работников (оклад × ставка):", this), 1, 2);
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
    connect(addEmpBtn, &QPushButton::clicked, this, &AccountingWindow::onAddStaffEmployee);
    connect(updateEmpBtn, &QPushButton::clicked, this, &AccountingWindow::onUpdateStaffEmployee);
    connect(removeEmpBtn, &QPushButton::clicked, this, &AccountingWindow::onRemoveStaffEmployee);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    mainLayout->addWidget(paramsGroup);
    mainLayout->addWidget(staffGroup);
    mainLayout->addWidget(payrollTable, 1);
    mainLayout->addWidget(summaryGroup);
    mainLayout->addLayout(bottomLayout);
}

void AccountingWindow::loadParametersToUi() {
    minBaseSpin->setValue(ledger.getMasterSharedMinBase());
    pieceworkShareSpin->setValue(ledger.getPieceworkShare() * 100.0);
}

unsigned int AccountingWindow::getSelectedStaffId() const {
    int row = payrollTable->currentRow();
    if (row < 0) {
        return 0;
    }
    const QTableWidgetItem* typeItem = payrollTable->item(row, 0);
    if (!typeItem || typeItem->text() != "Работник") {
        return 0;
    }
    const QTableWidgetItem* idItem = payrollTable->item(row, 6);
    return idItem ? idItem->data(Qt::UserRole).toUInt() : 0;
}

void AccountingWindow::refreshAccounting() {
    auto payroll = ledger.buildPayrollList(storage);
    payrollTable->setRowCount(static_cast<int>(payroll.size()));

    for (int row = 0; row < static_cast<int>(payroll.size()); ++row) {
        const auto& emp = payroll[static_cast<std::size_t>(row)];
        bool isMaster = (emp->getEmployeeType() == "Master");

        QString typeText = isMaster ? "Мастер" : "Работник";
        QString labelText = isMaster
            ? QString("%1 (%2)").arg(emp->getName(), emp->getPosition())
            : QString("%1 — %2").arg(emp->getName(), emp->getPosition());

        QString baseText = "0.00 BYN";
        QString pieceworkText = "—";

        if (isMaster) {
            if (const auto* mEmp = dynamic_cast<const MasterEmployee*>(emp.get()); mEmp != nullptr) {
                baseText = QString("%1 BYN").arg(mEmp->getSharedMinBase(), 0, 'f', 2);
                pieceworkText = QString("%1 BYN").arg(mEmp->getPieceworkEarnings(), 0, 'f', 2);
            }
        } else {
            if (const auto* sEmp = dynamic_cast<const StaffEmployee*>(emp.get()); sEmp != nullptr) {
                baseText = QString("%1 BYN").arg(sEmp->getBaseSalary(), 0, 'f', 2);
            }
        }

        QString rateText = QString("x%1").arg(emp->getRateMultiplier(), 0, 'f', 2);
        QString totalText = QString("%1 BYN").arg(emp->calculateSalary(), 0, 'f', 2);

        auto* idItem = makeOwned<QTableWidgetItem>(isMaster ? "—" : QString::number(emp->getId()));
        idItem->setData(Qt::UserRole, isMaster ? 0U : emp->getId());

        payrollTable->setItem(row, 0, makeOwned<QTableWidgetItem>(typeText));
        payrollTable->setItem(row, 1, makeOwned<QTableWidgetItem>(labelText));
        payrollTable->setItem(row, 2, makeOwned<QTableWidgetItem>(baseText));
        payrollTable->setItem(row, 3, makeOwned<QTableWidgetItem>(pieceworkText));
        payrollTable->setItem(row, 4, makeOwned<QTableWidgetItem>(rateText));
        payrollTable->setItem(row, 5, makeOwned<QTableWidgetItem>(totalText));
        payrollTable->setItem(row, 6, idItem);
    }

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

void AccountingWindow::onApplyPayrollParams() {
    try {
        ledger.setPayrollParameters(minBaseSpin->value(), pieceworkShareSpin->value() / 100.0);
        refreshAccounting();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка параметров", QString::fromStdString(ex.what()));
    }
}

void AccountingWindow::onAddStaffEmployee() {
    try {
        ledger.addStaffEmployee(empNameEdit->text(), empPositionEdit->text(), empSalarySpin->value(), empRateSpin->value());
        empNameEdit->clear();
        empPositionEdit->clear();
        empSalarySpin->setValue(0.0);
        empRateSpin->setValue(1.0);
        refreshAccounting();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка добавления", QString::fromStdString(ex.what()));
    }
}

void AccountingWindow::onUpdateStaffEmployee() {
    unsigned int staffId = getSelectedStaffId();
    if (staffId == 0) {
        QMessageBox::information(this, "Выбор работника", "Выберите строку со статусом «Работник» в таблице.");
        return;
    }
    try {
        ledger.updateStaffEmployee(staffId, empNameEdit->text(), empPositionEdit->text(), empSalarySpin->value(), empRateSpin->value());
        refreshAccounting();
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка обновления", QString::fromStdString(ex.what()));
    }
}

void AccountingWindow::onRemoveStaffEmployee() {
    unsigned int staffId = getSelectedStaffId();
    if (staffId == 0) {
        QMessageBox::information(this, "Выбор работника", "Выберите строку со статусом «Работник» для удаления.");
        return;
    }
    if (ledger.removeStaffEmployee(staffId)) {
        refreshAccounting();
    }
}
