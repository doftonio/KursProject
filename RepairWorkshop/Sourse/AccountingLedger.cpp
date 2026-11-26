#include "../Headers/AccountingLedger.h"
#include "../Headers/JsonFileHelper.h"
#include "../Headers/InputValidator.h"
#include "../Exeptions/InvalidInputDataException.h"
#include "../Exeptions/EntityNotFoundException.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>
#include <algorithm>
#include <ranges>

AccountingLedger::AccountingLedger(const QString& filePath)
    : ledgerFilePath(filePath) {
    QFileInfo info(ledgerFilePath);
    JsonFileHelper::ensureDirectory(info.path());
    loadFromDisk();
}

void AccountingLedger::loadFromDisk() {
    staffEmployees.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(ledgerFilePath, true, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        masterSharedMinBase = 400.0;
        pieceworkShare = 0.50;
        nextStaffId = 1;
        staffEmployees.push_back(StaffEmployee(nextStaffId++, "Соколова Елена Викторовна", "Администратор приёмки", 650.0, 1.0));
        staffEmployees.push_back(StaffEmployee(nextStaffId++, "Волков Игорь Николаевич", "Кладовщик", 580.0, 1.0));
        saveToDisk();
        return;
    }

    QJsonObject root = doc.object();
    masterSharedMinBase = root["masterSharedMinBase"].toDouble(400.0);
    pieceworkShare = root["pieceworkShare"].toDouble(0.50);
    nextStaffId = static_cast<unsigned int>(root["nextStaffId"].toInt(1));

    for (const auto& val : root["staff"].toArray()) {
        if (!val.isObject()) {
            continue;
        }
        StaffEmployee emp = StaffEmployee::fromJson(val.toObject());
        if (emp.getId() >= nextStaffId) {
            nextStaffId = emp.getId() + 1;
        }
        staffEmployees.push_back(emp);
    }
}

void AccountingLedger::saveToDisk() const {
    QJsonObject root;
    root["masterSharedMinBase"] = masterSharedMinBase;
    root["pieceworkShare"] = pieceworkShare;
    root["nextStaffId"] = static_cast<int>(nextStaffId);

    QJsonArray arr;
    for (const auto& emp : staffEmployees) {
        arr.append(emp.toJson());
    }
    root["staff"] = arr;
    JsonFileHelper::writeAtomic(ledgerFilePath, QJsonDocument(root).toJson(QJsonDocument::Indented));
}

double AccountingLedger::getMasterSharedMinBase() const {
    return masterSharedMinBase;
}

double AccountingLedger::getPieceworkShare() const {
    return pieceworkShare;
}

const std::vector<StaffEmployee>& AccountingLedger::getStaffEmployees() const {
    return staffEmployees;
}

void AccountingLedger::setPayrollParameters(double minBase, double share) {
    if (minBase < 0.0 || minBase > 50000.0) {
        throw InvalidInputDataException("Master minimum base must be between 0.00 and 50000.00");
    }
    if (share < 0.0 || share > 1.0) {
        throw InvalidInputDataException("Piecework share must be between 0.00 and 1.00");
    }
    masterSharedMinBase = minBase;
    pieceworkShare = share;
    saveToDisk();
}

StaffEmployee& AccountingLedger::addStaffEmployee(const QString& name, const QString& position, double fixedSalary, double rateMultiplier) {
    InputValidator::validateStringNotEmpty(name, "Employee Name");
    InputValidator::validateStringNotEmpty(position, "Position");
    if (fixedSalary < 0.0 || fixedSalary > 50000.0) {
        throw InvalidInputDataException("Employee salary must be between 0.00 and 50000.00");
    }
    if (rateMultiplier <= 0.0 || rateMultiplier > 10.0) {
        throw InvalidInputDataException("Rate multiplier must be between 0.01 and 10.00");
    }

    unsigned int assignedId = nextStaffId++;
    staffEmployees.push_back(
        StaffEmployee(assignedId, InputValidator::sanitizeString(name), InputValidator::sanitizeString(position), fixedSalary, rateMultiplier)
    );
    saveToDisk();
    return staffEmployees.back();
}

bool AccountingLedger::updateStaffEmployee(unsigned int staffId, const QString& name, const QString& position, double fixedSalary, double rateMultiplier) {
    auto it = std::ranges::find_if(staffEmployees, [staffId](const StaffEmployee& e) {
        return e.getId() == staffId;
    });
    if (it == staffEmployees.end()) {
        throw EntityNotFoundException("Staff employee not found");
    }
    InputValidator::validateStringNotEmpty(name, "Employee Name");
    InputValidator::validateStringNotEmpty(position, "Position");
    if (fixedSalary < 0.0 || fixedSalary > 50000.0) {
        throw InvalidInputDataException("Employee salary must be between 0.00 and 50000.00");
    }
    if (rateMultiplier <= 0.0 || rateMultiplier > 10.0) {
        throw InvalidInputDataException("Rate multiplier must be between 0.01 and 10.00");
    }

    it->setName(InputValidator::sanitizeString(name));
    it->setPosition(InputValidator::sanitizeString(position));
    it->setBaseSalary(fixedSalary);
    it->setRateMultiplier(rateMultiplier);
    saveToDisk();
    return true;
}

bool AccountingLedger::removeStaffEmployee(unsigned int staffId) {
    auto oldSize = staffEmployees.size();
    auto removed = std::ranges::remove_if(staffEmployees, [staffId](const StaffEmployee& e) {
        return e.getId() == staffId;
    });
    staffEmployees.erase(removed.begin(), removed.end());
    if (staffEmployees.size() < oldSize) {
        saveToDisk();
        return true;
    }
    return false;
}

void AccountingLedger::recalculateMasters(const Storage& storage, std::vector<MasterEmployee>& outMasters) const {
    outMasters.clear();
    for (const auto& master : storage.getMasters()) {
        double piecework = 0.0;
        for (const auto& order : storage.getOrders()) {
            if (order.getMasterId() == master.getId()) {
                double factor = order.getDevice() ? order.getDevice()->getComplexityFactor() : 1.0;
                piecework += order.getLaborCost() * factor * pieceworkShare;
            }
        }
        outMasters.push_back(
            MasterEmployee(
                master.getId(),
                master.getId(),
                master.getName(),
                master.getSpecialization(),
                masterSharedMinBase,
                piecework,
                1.0
            )
        );
    }
}

std::vector<std::unique_ptr<Employee>> AccountingLedger::buildPayrollList(const Storage& storage) const {
    std::vector<std::unique_ptr<Employee>> result;
    std::vector<MasterEmployee> mastersCalc;
    recalculateMasters(storage, mastersCalc);

    for (const auto& me : mastersCalc) {
        result.push_back(std::make_unique<MasterEmployee>(me));
    }
    for (const auto& se : staffEmployees) {
        result.push_back(std::make_unique<StaffEmployee>(se));
    }
    return result;
}

AccountingSummary AccountingLedger::calculateSummary(const Storage& storage) const {
    AccountingSummary summary;
    for (const auto& order : storage.getOrders()) {
        ++summary.totalOrdersCount;
        if (order.isArchived()) {
            ++summary.issuedOrdersCount;
        }
        double factor = order.getDevice() ? order.getDevice()->getComplexityFactor() : 1.0;
        double labor = order.getLaborCost() * factor;
        double parts = order.calculatePartsCost();
        summary.laborRevenue += labor;
        summary.partsRevenue += parts;
        summary.grossRevenue += (labor + parts);
        summary.partsExpense += parts;
    }

    auto payroll = buildPayrollList(storage);
    for (const auto& emp : payroll) {
        if (emp->getEmployeeType() == "Master") {
            summary.mastersPayroll += emp->calculateSalary();
        } else {
            summary.staffPayroll += emp->calculateSalary();
        }
    }

    summary.totalPayroll = summary.mastersPayroll + summary.staffPayroll;
    summary.totalExpenses = summary.partsExpense + summary.totalPayroll;
    summary.netIncome = summary.grossRevenue - summary.totalExpenses;
    return summary;
}
