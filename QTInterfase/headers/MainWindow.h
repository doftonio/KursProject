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

    QPushButton* addOrderBtn;
    QPushButton* editOrderBtn;
    QPushButton* changeStatusBtn;
    QPushButton* mastersBtn;
    QPushButton* clientsBtn;
    QPushButton* warehouseBtn;
    QPushButton* exportPdfBtn;
    QPushButton* reportBtn;
    QPushButton* backupBtn;

    void buildUi();
    void syncThemeControls();
    void setupConnections() const;
    void refreshOrdersTable();
    unsigned int getSelectedOrderId() const;
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
    void onExportWarrantyPdf();
    void onShowReport();
    void onBackupDatabase();

public:
    explicit MainWindow(ThemeManager& themeRef, QWidget* parent = nullptr);
    ~MainWindow() override = default;
};

#endif
