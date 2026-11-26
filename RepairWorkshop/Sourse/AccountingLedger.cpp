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

AccountingLedger::AccountingLedger(const Storage& storageRef, const QString& filePath)
    : ledgerFilePath(filePath) {
    QFileInfo info(ledgerFilePath);
    JsonFileHelper::ensureDirectory(info.path());
    reloadFromStorage(storageRef);
}

void AccountingLedger::reloadFromStorage(const Storage& storageRef) {
    loadFromDisk();
    syncStaffWithStorage(storageRef);
}

void AccountingLedger::loadFromDisk() {
    staffEmployees.clear();
    bool isEmptyOrMissing = false;
    QJsonDocument doc = JsonFileHelper::readDocument(ledgerFilePath, true, isEmptyOrMissing);
    if (isEmptyOrMissing) {
        masterSharedMinBase = 400.0;
        pieceworkShare = 0.50;
        saveToDisk();
        return;
    }

    QJsonObject root = doc.object();
    masterSharedMinBase = root["masterSharedMinBase"].toDouble(400.0);
    pieceworkShare = root["pieceworkShare"].toDouble(0.50);

    for (const auto& val : root["staff"].toArray()) {
        if (!val.isObject()) {
            continue;
        }
        staffEmployees.emplace_back(StaffEmployee::fromJson(val.toObject()));
    }
}

void AccountingLedger::saveToDisk() const {
    QJsonObject root;
    root["masterSharedMinBase"] = masterSharedMinBase;
    root["pieceworkShare"] = pieceworkShare;

    QJsonArray arr;
    for (const auto& emp : staffEmployees) {
        arr.append(emp.toJson());
    }
    root["staff"] = arr;
    JsonFileHelper::writeAtomic(ledgerFilePath, QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void AccountingLedger::syncStaffWithStorage(const Storage& storage) {
    std::vector<StaffEmployee> synced;
    for (const auto& master : storage.getMasters()) {
        auto existing = std::ranges::find_if(staffEmployees, [&master](const StaffEmployee& s) {
            return s.getId() == master.getId();
        });
        if (existing != staffEmployees.end()) {
            existing->setName(master.getName());
            synced.emplace_back(*existing);
        } else {
            synced.emplace_back(master.getId(), master.getName(), master.getSpecialization(), 0.0, 1.0);
        }
    }
    staffEmployees = std::move(synced);
    saveToDisk();
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

void AccountingLedger::updateStaffEmployeeRow(unsigned int staffId, const QString& name, const QString& position, double fixedSalary, double rateMultiplier) {
    auto it = std::ranges::find_if(staffEmployees, [staffId](const StaffEmployee& e) {
        return e.getId() == staffId;
    });
    if (it == staffEmployees.end()) {
        throw EntityNotFoundException("Worker not found");
    }
    InputValidator::validateStringNotEmpty(name, "Worker Name");
    InputValidator::validateStringNotEmpty(position, "Position");
    if (fixedSalary < 0.0 || fixedSalary > 50000.0) {
        throw InvalidInputDataException("Fixed salary must be between 0.00 and 50000.00");
    }
    if (rateMultiplier <= 0.0 || rateMultiplier > 10.0) {
        throw InvalidInputDataException("Rate multiplier must be between 0.01 and 10.00");
    }

    it->setName(InputValidator::sanitizeString(name));
    it->setPosition(InputValidator::sanitizeString(position));
    it->setBaseSalary(fixedSalary);
    it->setRateMultiplier(rateMultiplier);
    saveToDisk();
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
        double rate = 1.0;
        if (auto found = std::ranges::find_if(staffEmployees, [&master](const StaffEmployee& s) {
                return s.getId() == master.getId();
            }); found != staffEmployees.end()) {
            rate = found->getRateMultiplier();
        }
        outMasters.emplace_back(
            master.getId(),
            master.getId(),
            master.getName(),
            master.getSpecialization(),
            masterSharedMinBase,
            piecework,
            rate
        );
    }
}

std::vector<std::unique_ptr<Employee>> AccountingLedger::buildPayrollList(const Storage& storage) const {
    std::vector<std::unique_ptr<Employee>> result;
    std::vector<MasterEmployee> mastersCalc;
    recalculateMasters(storage, mastersCalc);

    for (const auto& me : mastersCalc) {
        result.emplace_back(std::make_unique<MasterEmployee>(me));
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

    std::vector<MasterEmployee> mastersCalc;
    recalculateMasters(storage, mastersCalc);
    for (const auto& me : mastersCalc) {
        summary.mastersPayroll += me.calculateSalary();
    }

    for (const auto& se : staffEmployees) {
        summary.staffPayroll += se.calculateSalary();
    }

    summary.totalPayroll = summary.mastersPayroll + summary.staffPayroll;
    summary.totalExpenses = summary.partsExpense + summary.totalPayroll;
    summary.netIncome = summary.grossRevenue - summary.totalExpenses;
    return summary;
}
