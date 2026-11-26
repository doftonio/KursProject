#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../../Headers/Storage.h"
#include "ThemeManager.h"
#include "WorkersTab.h"
#include "ClientsTab.h"
#include "WarehouseTab.h"
#include "AccountingTab.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <vector>

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    Storage storage;
    ThemeManager& themeManager;

    QTabWidget* mainTabs;
    QTableWidget* currentOrdersTable;
    QTableWidget* archivedOrdersTable;
    WorkersTab* workersTab;
    ClientsTab* clientsTab;
    WarehouseTab* warehouseTab;
    AccountingTab* accountingTab;

    QComboBox* statusFilterCombo;
    QLineEdit* searchEdit;
    QLineEdit* archiveSearchEdit;
    QLabel* statsLabel;
    QLabel* archiveStatsLabel;

    QPushButton* editOrderBtn;
    QPushButton* changeStatusBtn;
    QPushButton* receiptPdfBtn;
    QPushButton* warrantyPdfBtn;

    void buildUi();
    QWidget* createTopActionBar();
    QWidget* createCurrentOrdersTab();
    QWidget* createArchiveOrdersTab();
    void configureOrdersTable(QTableWidget* table) const;
    void populateOrdersTable(QTableWidget* table, const std::vector<Order>& ordersList) const;
    void setupConnections() const;
    void refreshOrdersTables();
    unsigned int getSelectedOrderId() const;
    void exportHtmlToPdf(const QString& dialogTitle, const QString& defaultFileName, const QString& htmlContent, const QString& successMessage);
    static QString formatStatusRu(OrderStatus status);
    static QString formatCategoryRu(const QString& category);

private slots:
    void onTabChanged(int index);
    void onAddOrder();
    void onEditOrder();
    void onChangeOrderStatus();
    void onFilterChanged();
    void onExportAcceptanceReceiptPdf();
    void onExportWarrantyPdf();
    void onBackupDatabase();
    void onOpenSettings();

public:
    explicit MainWindow(ThemeManager& themeRef, QWidget* parent = nullptr);
    ~MainWindow() override = default;
};

#endif
