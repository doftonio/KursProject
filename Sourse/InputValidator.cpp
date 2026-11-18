#include "../Headers/InputValidator.h"
#include "../Exeptions/InvalidInputDataException.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>

const QString InputValidator::phonePattern = "^\\+375(17|25|29|33|44)\\d{7}$";
const QString InputValidator::skuPattern = "^[A-Za-z0-9\\-_]{3,20}$";

QString InputValidator::normalizePhone(const QString& rawPhone) {
    QString cleaned = rawPhone.trimmed();
    cleaned.remove(' ');
    cleaned.remove('-');
    cleaned.remove('(');
    cleaned.remove(')');
    return cleaned;
}

void InputValidator::validatePhone(const QString& phone) {
    QString normalized = normalizePhone(phone);
    QRegularExpression regex(phonePattern);
    if (!regex.match(normalized).hasMatch()) {
        throw InvalidInputDataException("Invalid Belarus phone format (expected +375XXXXXXXXX)");
    }
}

void InputValidator::validateSku(const QString& sku) {
    QRegularExpression regex(skuPattern);
    if (!regex.match(sku.trimmed()).hasMatch()) {
        throw InvalidInputDataException("Invalid SKU format (3-20 alphanumeric characters, '-' or '_')");
    }
}

void InputValidator::validateStringNotEmpty(const QString& text, const QString& fieldName) {
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        throw InvalidInputDataException("Field cannot be empty: " + fieldName.toStdString());
    }
    if (trimmed.length() > maxTextLength) {
        throw InvalidInputDataException("Field exceeds maximum length (255): " + fieldName.toStdString());
    }
}

void InputValidator::validateSerial(const QString& serial) {
    QString trimmed = serial.trimmed();
    if (trimmed.length() < 3 || trimmed.length() > 64) {
        throw InvalidInputDataException("Serial number must be between 3 and 64 characters");
    }
}

void InputValidator::validatePrice(double price) {
    if (price < minPrice || price > maxPrice) {
        throw InvalidInputDataException("Part price must be between 0.01 and 5000.00");
    }
}

void InputValidator::validateLaborCost(double cost) {
    if (cost < minLaborCost || cost > maxLaborCost) {
        throw InvalidInputDataException("Labor cost must be between 0.00 and 10000.00");
    }
}

void InputValidator::validateQuantity(int quantity, int maxAvailable) {
    if (quantity <= 0) {
        throw InvalidInputDataException("Quantity must be greater than zero");
    }
    if (quantity > maxAvailable) {
        throw InvalidInputDataException("Requested quantity exceeds available stock");
    }
}

void InputValidator::validateOrderInput(const QString& model, const QString& serial, const QString& fault, double laborCost) {
    validateStringNotEmpty(model, "Device Model");
    validateSerial(serial);
    validateStringNotEmpty(fault, "Fault Description");
    validateLaborCost(laborCost);
}

QString InputValidator::sanitizeString(const QString& text) {
    QString result = text.trimmed();
    result.remove(QChar('\0'));
    return result;
}

void InputValidator::setupPhoneMask(QLineEdit* lineEdit) {
    if (lineEdit) {
        lineEdit->setInputMask("+375 (99) 999-99-99;_");
    }
}

void InputValidator::setupSkuValidator(QLineEdit* lineEdit) {
    if (lineEdit) {
        auto* validator = new QRegularExpressionValidator(QRegularExpression(skuPattern), lineEdit);
        lineEdit->setValidator(validator);
    }
}
