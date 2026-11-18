#include "../headers/MainWindow.h"
#include "../headers/OrderDialog.h"
#include "../headers/ClientDialog.h"
#include "../headers/MasterDialog.h"
#include "../headers/PartDialog.h"
#include "../../Exeptions/AppException.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QDialog>
#include <QFrame>
#include <QDate>

MainWindow::MainWindow(ThemeManager& themeRef, QWidget* parent)
    : QMainWindow(parent), themeManager(themeRef) {
    buildUi();
    syncThemeControls();
    setupConnections();
    refreshOrdersTable();
}

QString MainWindow::formatStatusRu(OrderStatus status) {
    switch (status) {
        case OrderStatus::Accepted: return "Принят";
        case OrderStatus::Diagnostics: return "Диагностика";
        case OrderStatus::WaitingParts: return "Ожидание запчастей";
        case OrderStatus::Completed: return "Завершён";
        case OrderStatus::Issued: return "Выдан";
    }
    return "Неизвестно";
}

QString MainWindow::formatCategoryRu(const QString& category) {
    if (category == "Computer") {
        return "Компьютерная техника";
    }
    if (category == "Household") {
        return "Бытовая техника";
    }
    if (category == "Electronics") {
        return "Радиоэлектроника";
    }
    return category;
}

void MainWindow::buildUi() {
    setWindowTitle("Система управления ремонтной мастерской");
    resize(1240, 700);

    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);

    auto* themeFrame = new QFrame(this);
    themeFrame->setObjectName("themeBar");
    auto* themeLayout = new QHBoxLayout(themeFrame);
    themeLayout->setContentsMargins(10, 6, 10, 6);

    themeToggleBtn = new QPushButton(this);

    paletteCombo = new QComboBox(this);
    paletteCombo->addItem("Синяя палитра", static_cast<int>(ColorPalette::Blue));
    paletteCombo->addItem("Изумрудная палитра", static_cast<int>(ColorPalette::Emerald));
    paletteCombo->addItem("Янтарная палитра", static_cast<int>(ColorPalette::Amber));
    paletteCombo->addItem("Фиолетовая палитра", static_cast<int>(ColorPalette::Violet));
    paletteCombo->addItem("Рубиновая палитра", static_cast<int>(ColorPalette::Crimson));
    paletteCombo->addItem("Графитовая палитра", static_cast<int>(ColorPalette::Graphite));

    fontScaleCombo = new QComboBox(this);
    fontScaleCombo->addItem("Шрифт: Компактный", static_cast<int>(FontScale::Compact));
    fontScaleCombo->addItem("Шрифт: Стандартный", static_cast<int>(FontScale::Standard));
    fontScaleCombo->addItem("Шрифт: Крупный", static_cast<int>(FontScale::Large));

    roundedCheck = new QCheckBox("Скругление углов", this);

    themeLayout->addWidget(new QLabel("Оформление:", this));
    themeLayout->addWidget(themeToggleBtn);
    themeLayout->addWidget(new QLabel("Цветовая схема:", this));
    themeLayout->addWidget(paletteCombo);
    themeLayout->addWidget(fontScaleCombo);
    themeLayout->addWidget(roundedCheck);
    themeLayout->addStretch(1);

    auto* filterLayout = new QHBoxLayout();
    statusFilterCombo = new QComboBox(this);
    statusFilterCombo->addItem("Все статусы", -1);
    statusFilterCombo->addItem("Принят", 0);
    statusFilterCombo->addItem("Диагностика", 1);
    statusFilterCombo->addItem("Ожидание запчастей", 2);
    statusFilterCombo->addItem("Завершён", 3);
    statusFilterCombo->addItem("Выдан", 4);

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск по № заказа, модели, серийному номеру, ФИО или телефону клиента...");

    statsLabel = new QLabel(this);
    statsLabel->setObjectName("statsBadge");

    filterLayout->addWidget(new QLabel("Статус:", this));
    filterLayout->addWidget(statusFilterCombo);
    filterLayout->addWidget(searchEdit, 1);
    filterLayout->addWidget(statsLabel);

    ordersTable = new QTableWidget(this);
    ordersTable->setColumnCount(8);
    ordersTable->setHorizontalHeaderLabels({
        "№", "Дата приёма", "Клиент", "Устройство", "Серийный №", "Мастер", "Статус", "Итого (BYN)"
    });
    ordersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ordersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ordersTable->setAlternatingRowColors(true);
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* buttonsLayout = new QHBoxLayout();
    addOrderBtn = new QPushButton("Новый заказ", this);
    addOrderBtn->setObjectName("primaryBtn");
    editOrderBtn = new QPushButton("Карточка заказа", this);
    changeStatusBtn = new QPushButton("Изменить статус", this);
    mastersBtn = new QPushButton("Мастера", this);
    clientsBtn = new QPushButton("Клиенты", this);
    warehouseBtn = new QPushButton("Склад запчастей", this);
    exportPdfBtn = new QPushButton("Акт в PDF", this);
    reportBtn = new QPushButton("Отчёт", this);
    backupBtn = new QPushButton("Резервная копия", this);

    buttonsLayout->addWidget(addOrderBtn);
    buttonsLayout->addWidget(editOrderBtn);
    buttonsLayout->addWidget(changeStatusBtn);
    buttonsLayout->addWidget(mastersBtn);
    buttonsLayout->addWidget(clientsBtn);
    buttonsLayout->addWidget(warehouseBtn);
    buttonsLayout->addWidget(exportPdfBtn);
    buttonsLayout->addWidget(reportBtn);
    buttonsLayout->addWidget(backupBtn);

    mainLayout->addWidget(themeFrame);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(ordersTable, 1);
    mainLayout->addLayout(buttonsLayout);

    setCentralWidget(central);
}

void MainWindow::syncThemeControls() {
    bool isNight = (themeManager.getMode() == ThemeMode::Night);
    themeToggleBtn->setText(isNight ? "Режим: Ночь (вкл. День)" : "Режим: День (вкл. Ночь)");

    int palIdx = paletteCombo->findData(static_cast<int>(themeManager.getPalette()));
    if (palIdx >= 0) {
        paletteCombo->setCurrentIndex(palIdx);
    }

    int fontIdx = fontScaleCombo->findData(static_cast<int>(themeManager.getFontScale()));
    if (fontIdx >= 0) {
        fontScaleCombo->setCurrentIndex(fontIdx);
    }

    roundedCheck->setChecked(themeManager.isRoundedCorners());
}

void MainWindow::setupConnections() {
    connect(themeToggleBtn, &QPushButton::clicked, this, &MainWindow::onToggleThemeMode);
    connect(paletteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onPaletteChanged);
    connect(fontScaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFontScaleChanged);
    connect(roundedCheck, &QCheckBox::toggled, this, &MainWindow::onRoundedChanged);

    connect(addOrderBtn, &QPushButton::clicked, this, &MainWindow::onAddOrder);
    connect(editOrderBtn, &QPushButton::clicked, this, &MainWindow::onEditOrder);
    connect(changeStatusBtn, &QPushButton::clicked, this, &MainWindow::onChangeOrderStatus);
    connect(ordersTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::onEditOrder);
    connect(mastersBtn, &QPushButton::clicked, this, &MainWindow::onOpenMasters);
    connect(clientsBtn, &QPushButton::clicked, this, &MainWindow::onOpenClients);
    connect(warehouseBtn, &QPushButton::clicked, this, &MainWindow::onOpenWarehouse);
    connect(exportPdfBtn, &QPushButton::clicked, this, &MainWindow::onExportWarrantyPdf);
    connect(reportBtn, &QPushButton::clicked, this, &MainWindow::onShowReport);
    connect(backupBtn, &QPushButton::clicked, this, &MainWindow::onBackupDatabase);
    connect(statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
}

void MainWindow::refreshOrdersTable() {
    int statusFilter = statusFilterCombo->currentData().toInt();
    QString query = searchEdit->text();
    std::vector<Order> filtered = storage.filterOrders(statusFilter, query);

    ordersTable->setRowCount(static_cast<int>(filtered.size()));
    for (int row = 0; row < static_cast<int>(filtered.size()); ++row) {
        const Order& order = filtered[static_cast<std::size_t>(row)];
        const Client* client = storage.findClientById(order.getClientId());
        const Master* master = storage.findMasterById(order.getMasterId());

        QString clientText = client ? QString("%1 (%2)").arg(client->getName(), client->getPhone()) : "Неизвестен";
        QString masterText = master ? master->getName() : "Не назначен";
        QString modelText = order.getDevice()
            ? QString("[%1] %2").arg(formatCategoryRu(order.getDevice()->getCategory()), order.getDevice()->getModel())
            : "Н/Д";
        QString serialText = order.getDevice() ? order.getDevice()->getSerialNumber() : "Н/Д";

        auto* idItem = new QTableWidgetItem(QString::number(order.getId()));
        idItem->setData(Qt::UserRole, order.getId());

        ordersTable->setItem(row, 0, idItem);
        ordersTable->setItem(row, 1, new QTableWidgetItem(order.getDateReceived()));
        ordersTable->setItem(row, 2, new QTableWidgetItem(clientText));
        ordersTable->setItem(row, 3, new QTableWidgetItem(modelText));
        ordersTable->setItem(row, 4, new QTableWidgetItem(serialText));
        ordersTable->setItem(row, 5, new QTableWidgetItem(masterText));
        ordersTable->setItem(row, 6, new QTableWidgetItem(formatStatusRu(order.getStatus())));
        ordersTable->setItem(row, 7, new QTableWidgetItem(QString::number(order.calculateTotalCost(), 'f', 2)));
    }

    statsLabel->setText(QString("Активных заказов: %1 / %2").arg(storage.countActiveOrders()).arg(Storage::maxActiveOrdersLimit));
}

unsigned int MainWindow::getSelectedOrderId() const {
    int currentRow = ordersTable->currentRow();
    if (currentRow < 0) {
        return 0;
    }
    QTableWidgetItem* item = ordersTable->item(currentRow, 0);
    return item ? item->data(Qt::UserRole).toUInt() : 0;
}

void MainWindow::onToggleThemeMode() {
    themeManager.toggleMode();
    syncThemeControls();
}

void MainWindow::onPaletteChanged(int index) {
    if (index < 0) {
        return;
    }
    auto selected = static_cast<ColorPalette>(paletteCombo->itemData(index).toInt());
    themeManager.setPalette(selected);
}

void MainWindow::onFontScaleChanged(int index) {
    if (index < 0) {
        return;
    }
    auto selected = static_cast<FontScale>(fontScaleCombo->itemData(index).toInt());
    themeManager.setFontScale(selected);
}

void MainWindow::onRoundedChanged(bool checked) {
    themeManager.setRoundedCorners(checked);
}

void MainWindow::onAddOrder() {
    try {
        storage.checkActiveOrdersLimit();
        OrderDialog dialog(storage, 0, this);
        if (dialog.exec() == QDialog::Accepted) {
            refreshOrdersTable();
        }
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка регистрации", QString::fromStdString(ex.what()));
    }
}

void MainWindow::onEditOrder() {
    unsigned int selectedId = getSelectedOrderId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор заказа", "Пожалуйста, выберите заказ в таблице.");
        return;
    }
    OrderDialog dialog(storage, selectedId, this);
    dialog.exec();
    refreshOrdersTable();
}

void MainWindow::onChangeOrderStatus() {
    unsigned int selectedId = getSelectedOrderId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор заказа", "Пожалуйста, выберите заказ в таблице для изменения его статуса.");
        return;
    }
    const Order* order = storage.findOrderById(selectedId);
    if (!order) {
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Изменение статуса заказа №%1").arg(selectedId));
    dlg.resize(420, 210);

    auto* layout = new QVBoxLayout(&dlg);
    auto* form = new QFormLayout();

    QString deviceInfo = order->getDevice() ? order->getDevice()->getModel() : "Устройство";
    auto* infoLabel = new QLabel(
        QString("<b>%1</b><br>Текущий статус: %2").arg(deviceInfo.toHtmlEscaped(), formatStatusRu(order->getStatus())),
        &dlg
    );
    layout->addWidget(infoLabel);

    auto* statusSelect = new QComboBox(&dlg);
    statusSelect->addItem("Принят", 0);
    statusSelect->addItem("Диагностика", 1);
    statusSelect->addItem("Ожидание запчастей", 2);
    statusSelect->addItem("Завершён", 3);
    statusSelect->addItem("Выдан", 4);

    int currentIdx = statusSelect->findData(order->getStatusCode());
    if (currentIdx >= 0) {
        statusSelect->setCurrentIndex(currentIdx);
    }

    auto* commentEdit = new QLineEdit(&dlg);
    commentEdit->setPlaceholderText("Комментарий мастера (необязательно)...");

    form->addRow("Новый статус:", statusSelect);
    form->addRow("Примечание:", commentEdit);
    layout->addLayout(form);

    auto* btnRow = new QHBoxLayout();
    auto* applyBtn = new QPushButton("Применить статус", &dlg);
    applyBtn->setObjectName("primaryBtn");
    auto* cancelBtn = new QPushButton("Отмена", &dlg);
    btnRow->addStretch(1);
    btnRow->addWidget(applyBtn);
    btnRow->addWidget(cancelBtn);
    layout->addLayout(btnRow);

    connect(applyBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        try {
            auto newStatus = static_cast<OrderStatus>(statusSelect->currentData().toInt());
            if (order->getStatus() != newStatus) {
                storage.changeOrderStatus(selectedId, newStatus);
            }
            if (!commentEdit->text().trimmed().isEmpty()) {
                storage.appendOrderNote(selectedId, commentEdit->text());
            }
            refreshOrdersTable();
        } catch (const AppException& ex) {
            QMessageBox::warning(this, "Ошибка смены статуса", QString::fromStdString(ex.what()));
        }
    }
}

void MainWindow::onFilterChanged() {
    refreshOrdersTable();
}

void MainWindow::onOpenMasters() {
    MasterDialog dialog(storage, this);
    dialog.exec();
    refreshOrdersTable();
}

void MainWindow::onOpenClients() {
    ClientDialog dialog(storage, this);
    dialog.exec();
    refreshOrdersTable();
}

void MainWindow::onOpenWarehouse() {
    PartDialog dialog(storage, this);
    dialog.exec();
}

void MainWindow::onExportWarrantyPdf() {
    unsigned int selectedId = getSelectedOrderId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор заказа", "Пожалуйста, выберите заказ для формирования акта.");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить гарантийный акт в PDF",
        QString("Заказ_%1_Акт.pdf").arg(selectedId),
        "PDF-документы (*.pdf)"
    );
    if (fileName.isEmpty()) {
        return;
    }
    try {
        QString html = storage.generateWarrantyActHtml(selectedId);
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);

        QTextDocument doc;
        doc.setHtml(html);
        doc.print(&printer);
        QMessageBox::information(this, "Экспорт завершён", "Гарантийный акт успешно сохранён в файл PDF.");
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка экспорта", QString::fromStdString(ex.what()));
    }
}

void MainWindow::onShowReport() {
    QDate today = QDate::currentDate();
    PerformanceReport rep = storage.generatePerformanceReport(today.addYears(-1), today);
    QString msg = QString(
        "Всего заказов в базе: %1\n"
        "Активных заказов в работе: %2\n"
        "Завершено и выдано: %3\n"
        "Выручка за работы мастеров: %4 BYN\n"
        "Выручка за комплектующие: %5 BYN\n"
        "Общая выручка мастерской: %6 BYN"
    )
    .arg(rep.totalOrders)
    .arg(rep.activeOrders)
    .arg(rep.completedOrders)
    .arg(rep.totalLaborRevenue, 0, 'f', 2)
    .arg(rep.totalPartsRevenue, 0, 'f', 2)
    .arg(rep.totalRevenue, 0, 'f', 2);

    QMessageBox::information(this, "Аналитический отчёт мастерской", msg);
}

void MainWindow::onBackupDatabase() {
    try {
        QString path = storage.backupDatabase();
        QMessageBox::information(this, "Резервное копирование", "Резервная копия базы данных сохранена в каталог:\n" + path);
    } catch (const AppException& ex) {
        QMessageBox::critical(this, "Ошибка резервного копирования", QString::fromStdString(ex.what()));
    }
}
