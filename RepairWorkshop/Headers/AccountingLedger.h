#ifndef ACCOUNTINGLEDGER_H
#define ACCOUNTINGLEDGER_H

#include "Storage.h"
#include "Employee.h"
#include "MasterEmployee.h"
#include "StaffEmployee.h"
#include <QString>
#include <QJsonObject>
#include <vector>
#include <memory>

struct AccountingSummary {
    double grossRevenue = 0.0;
    double laborRevenue = 0.0;
    double partsRevenue = 0.0;
    double partsExpense = 0.0;
    double mastersPayroll = 0.0;
    double staffPayroll = 0.0;
    double totalPayroll = 0.0;
    double totalExpenses = 0.0;
    double netIncome = 0.0;
    int totalOrdersCount = 0;
    int issuedOrdersCount = 0;
};

class AccountingLedger {
private:
    QString ledgerFilePath;
    double masterSharedMinBase = 400.0;
    double pieceworkShare = 0.50;
    unsigned int nextStaffId = 1;
    std::vector<StaffEmployee> staffEmployees;

    void loadFromDisk();
    void saveToDisk() const;
    void recalculateMasters(const Storage& storage, std::vector<MasterEmployee>& outMasters) const;

public:
    explicit AccountingLedger(const QString& filePath = "data/accounting.json");

    double getMasterSharedMinBase() const;
    double getPieceworkShare() const;
    const std::vector<StaffEmployee>& getStaffEmployees() const;

    void setPayrollParameters(double minBase, double share);
    StaffEmployee& addStaffEmployee(const QString& name, const QString& position, double fixedSalary, double rateMultiplier = 1.0);
    bool updateStaffEmployee(unsigned int staffId, const QString& name, const QString& position, double fixedSalary, double rateMultiplier);
    bool removeStaffEmployee(unsigned int staffId);

    std::vector<std::unique_ptr<Employee>> buildPayrollList(const Storage& storage) const;
    AccountingSummary calculateSummary(const Storage& storage) const;
};

#endif
