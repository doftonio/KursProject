#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../../Headers/Storage.h"
#include "ThemeManager.h"
#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    Storage storage;
    ThemeManager& themeManager;

    QPushButton* themeToggleBtn;
    QComboBox* paletteCombo;
    QComboBox* fontScaleCombo;
    QCheckBox* roundedCheck;

    QTableWidget* ordersTable;
    QComboBox* statusFilterCombo;
    QLineEdit* searchEdit;
    QLabel* statsLabel;

    void buildUi();
    void syncThemeControls();
    void setupConnections() const;
    void refreshOrdersTable();
    unsigned int getSelectedOrderId() const;
    void exportHtmlToPdf(const QString& dialogTitle, const QString& defaultFileName, const QString& htmlContent, const QString& successMessage);
    static QString formatStatusRu(OrderStatus status);
    static QString formatCategoryRu(const QString& category);

private slots:
    void onToggleThemeMode();
    void onPaletteChanged(int index);
    void onFontScaleChanged(int index);
    void onRoundedChanged(bool checked);
    void onAddOrder();
    void onEditOrder();
    void onChangeOrderStatus();
    void onFilterChanged();
    void onOpenMasters();
    void onOpenClients();
    void onOpenWarehouse();
    void onExportAcceptanceReceiptPdf();
    void onExportWarrantyPdf();
    void onShowReport();
    void onBackupDatabase();

public:
    explicit MainWindow(ThemeManager& themeRef, QWidget* parent = nullptr);
    ~MainWindow() override = default;
};

#endif
