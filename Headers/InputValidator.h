#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

#include <QString>
#include <QDateTime>
#include <QLineEdit>

class InputValidator {
private:
    static const QString phonePattern;
    static const QString skuPattern;
    static constexpr double minLaborCost = 0.0;
    static constexpr double maxLaborCost = 10000.0;
    static constexpr double minPrice = 0.01;
    static constexpr double maxPrice = 5000.0;
    static constexpr int maxTextLength = 255;

public:
    static void validatePhone(const QString& phone);
    static void validateSku(const QString& sku);
    static void validateStringNotEmpty(const QString& text, const QString& fieldName);
    static void validateSerial(const QString& serial);
    static void validatePrice(double price);
    static void validateLaborCost(double cost);
    static void validateQuantity(int quantity, int maxAvailable);
    static void validateOrderInput(const QString& model, const QString& serial, const QString& fault, double laborCost);
    static QString normalizePhone(const QString& rawPhone);
    static QString normalizeSerial(const QString& rawSerial);
    static QString sanitizeString(const QString& text);
    static void setupPhoneMask(QLineEdit* lineEdit);
    static void setupSkuValidator(QLineEdit* lineEdit);
};

#endif
