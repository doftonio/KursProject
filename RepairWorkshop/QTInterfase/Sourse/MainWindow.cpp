#include "../headers/MainWindow.h"
#include "../headers/OrderDialog.h"
#include "../headers/ClientDialog.h"
#include "../headers/MasterDialog.h"
#include "../headers/PartDialog.h"
#include "../headers/AccountingWindow.h"
#include "../headers/UiHelpers.h"
#include "../../Headers/WorkshopSearchService.h"
#include "../../Headers/WorkshopReportService.h"
#include "../../Headers/EnumUtils.h"
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

MainWindow::MainWindow(ThemeManager& themeRef, QWidget* parent)
    : QMainWindow(parent), themeManager(themeRef) {
    buildUi();
    syncThemeControls();
    setupConnections();
    refreshOrdersTables();
}

QString MainWindow::formatStatusRu(OrderStatus status) {
    using enum OrderStatus;
    switch (status) {
        case Accepted: return "Принят";
        case Diagnostics: return "Диагностика";
        case WaitingParts: return "Ожидание запчастей";
        case Completed: return "Завершён";
        case Issued: return "Выдан (в архиве)";
        default: return "Неизвестно";
    }
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

void MainWindow::configureOrdersTable(QTableWidget* table) const {
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({
        "№", "Дата приёма", "Клиент", "Устройство", "Серийный №", "Мастер", "Статус", "Итого (BYN)"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QWidget* MainWindow::createCurrentOrdersTab() {
    auto* tab = makeOwned<QWidget>(this);
    auto* layout = makeOwned<QVBoxLayout>(tab);

    auto* filterLayout = makeOwned<QHBoxLayout>();
    statusFilterCombo = makeOwned<QComboBox>(tab);
    statusFilterCombo->addItem("Все текущие", -1);
    statusFilterCombo->addItem("Принят", 0);
    statusFilterCombo->addItem("Диагностика", 1);
    statusFilterCombo->addItem("Ожидание запчастей", 2);
    statusFilterCombo->addItem("Завершён", 3);

    searchEdit = makeOwned<QLineEdit>(tab);
    searchEdit->setPlaceholderText("Поиск по № заказа, модели, серийному номеру, ФИО или телефону клиента...");

    statsLabel = makeOwned<QLabel>(tab);
    statsLabel->setObjectName("statsBadge");

    filterLayout->addWidget(makeOwned<QLabel>("Статус:", tab));
    filterLayout->addWidget(statusFilterCombo);
    filterLayout->addWidget(searchEdit, 1);
    filterLayout->addWidget(statsLabel);

    currentOrdersTable = makeOwned<QTableWidget>(tab);
    configureOrdersTable(currentOrdersTable);

    layout->addLayout(filterLayout);
    layout->addWidget(currentOrdersTable, 1);
    return tab;
}

QWidget* MainWindow::createArchiveOrdersTab() {
    auto* tab = makeOwned<QWidget>(this);
    auto* layout = makeOwned<QVBoxLayout>(tab);

    auto* topRow = makeOwned<QHBoxLayout>();
    auto* noticeLabel = makeOwned<QLabel>("Выданные заказы (редактирование заблокировано)", tab);
    noticeLabel->setObjectName("archiveNotice");

    archiveSearchEdit = makeOwned<QLineEdit>(tab);
    archiveSearchEdit->setPlaceholderText("Поиск в архиве по № заказа, модели, серийному номеру, ФИО или телефону...");

    archiveStatsLabel = makeOwned<QLabel>(tab);
    archiveStatsLabel->setObjectName("statsBadge");

    topRow->addWidget(noticeLabel);
    topRow->addWidget(archiveSearchEdit, 1);
    topRow->addWidget(archiveStatsLabel);

    archivedOrdersTable = makeOwned<QTableWidget>(tab);
    configureOrdersTable(archivedOrdersTable);

    layout->addLayout(topRow);
    layout->addWidget(archivedOrdersTable, 1);
    return tab;
}

void MainWindow::buildUi() {
    setWindowTitle("Система управления ремонтной мастерской");
    resize(1340, 720);

    auto* central = makeOwned<QWidget>(this);
    auto* mainLayout = makeOwned<QVBoxLayout>(central);
    mainLayout->setSpacing(10);

    auto* themeFrame = makeOwned<QFrame>(this);
    themeFrame->setObjectName("themeBar");
    auto* themeLayout = makeOwned<QHBoxLayout>(themeFrame);
    themeLayout->setContentsMargins(10, 6, 10, 6);

    themeToggleBtn = makeOwned<QPushButton>(this);

    paletteCombo = makeOwned<QComboBox>(this);
    paletteCombo->addItem("Синяя палитра", toUnderlying(ColorPalette::Blue));
    paletteCombo->addItem("Изумрудная палитра", toUnderlying(ColorPalette::Emerald));
    paletteCombo->addItem("Янтарная палитра", toUnderlying(ColorPalette::Amber));
    paletteCombo->addItem("Фиолетовая палитра", toUnderlying(ColorPalette::Violet));
    paletteCombo->addItem("Рубиновая палитра", toUnderlying(ColorPalette::Crimson));
    paletteCombo->addItem("Графитовая палитра", toUnderlying(ColorPalette::Graphite));

    fontScaleCombo = makeOwned<QComboBox>(this);
    fontScaleCombo->addItem("Шрифт: Компактный", toUnderlying(FontScale::Compact));
    fontScaleCombo->addItem("Шрифт: Стандартный", toUnderlying(FontScale::Standard));
    fontScaleCombo->addItem("Шрифт: Крупный", toUnderlying(FontScale::Large));

    roundedCheck = makeOwned<QCheckBox>("Скругление углов", this);

    themeLayout->addWidget(makeOwned<QLabel>("Оформление:", this));
    themeLayout->addWidget(themeToggleBtn);
    themeLayout->addWidget(makeOwned<QLabel>("Цветовая схема:", this));
    themeLayout->addWidget(paletteCombo);
    themeLayout->addWidget(fontScaleCombo);
    themeLayout->addWidget(roundedCheck);
    themeLayout->addStretch(1);

    ordersTabs = makeOwned<QTabWidget>(this);
    ordersTabs->addTab(createCurrentOrdersTab(), "Текущие заказы");
    ordersTabs->addTab(createArchiveOrdersTab(), "Архив заказов");

    auto* buttonsLayout = makeOwned<QHBoxLayout>();
    auto* addOrderBtn = makeOwned<QPushButton>("Новый заказ", this);
    addOrderBtn->setObjectName("primaryBtn");
    editOrderBtn = makeOwned<QPushButton>("Карточка заказа", this);
    changeStatusBtn = makeOwned<QPushButton>("Изменить статус", this);
    auto* mastersBtn = makeOwned<QPushButton>("Мастера", this);
    auto* clientsBtn = makeOwned<QPushButton>("Клиенты", this);
    auto* warehouseBtn = makeOwned<QPushButton>("Склад запчастей", this);
    auto* receiptPdfBtn = makeOwned<QPushButton>("Квитанция приёма", this);
    auto* warrantyPdfBtn = makeOwned<QPushButton>("Гарантийный акт", this);
    auto* accountingBtn = makeOwned<QPushButton>("Отчёт и бухгалтерия", this);
    auto* backupBtn = makeOwned<QPushButton>("Резервная копия", this);

    buttonsLayout->addWidget(addOrderBtn);
    buttonsLayout->addWidget(editOrderBtn);
    buttonsLayout->addWidget(changeStatusBtn);
    buttonsLayout->addWidget(mastersBtn);
    buttonsLayout->addWidget(clientsBtn);
    buttonsLayout->addWidget(warehouseBtn);
    buttonsLayout->addWidget(receiptPdfBtn);
    buttonsLayout->addWidget(warrantyPdfBtn);
    buttonsLayout->addWidget(accountingBtn);
    buttonsLayout->addWidget(backupBtn);

    connect(addOrderBtn, &QPushButton::clicked, this, &MainWindow::onAddOrder);
    connect(editOrderBtn, &QPushButton::clicked, this, &MainWindow::onEditOrder);
    connect(changeStatusBtn, &QPushButton::clicked, this, &MainWindow::onChangeOrderStatus);
    connect(mastersBtn, &QPushButton::clicked, this, &MainWindow::onOpenMasters);
    connect(clientsBtn, &QPushButton::clicked, this, &MainWindow::onOpenClients);
    connect(warehouseBtn, &QPushButton::clicked, this, &MainWindow::onOpenWarehouse);
    connect(receiptPdfBtn, &QPushButton::clicked, this, &MainWindow::onExportAcceptanceReceiptPdf);
    connect(warrantyPdfBtn, &QPushButton::clicked, this, &MainWindow::onExportWarrantyPdf);
    connect(accountingBtn, &QPushButton::clicked, this, &MainWindow::onOpenAccountingWindow);
    connect(backupBtn, &QPushButton::clicked, this, &MainWindow::onBackupDatabase);

    mainLayout->addWidget(themeFrame);
    mainLayout->addWidget(ordersTabs, 1);
    mainLayout->addLayout(buttonsLayout);

    setCentralWidget(central);
}

void MainWindow::syncThemeControls() {
    bool isNight = (themeManager.getMode() == ThemeMode::Night);
    themeToggleBtn->setText(isNight ? "Режим: Ночь (вкл. День)" : "Режим: День (вкл. Ночь)");

    if (int palIdx = paletteCombo->findData(toUnderlying(themeManager.getPalette())); palIdx >= 0) {
        paletteCombo->setCurrentIndex(palIdx);
    }

    if (int fontIdx = fontScaleCombo->findData(toUnderlying(themeManager.getFontScale())); fontIdx >= 0) {
        fontScaleCombo->setCurrentIndex(fontIdx);
    }

    roundedCheck->setChecked(themeManager.isRoundedCorners());
}

void MainWindow::setupConnections() const {
    connect(themeToggleBtn, &QPushButton::clicked, this, &MainWindow::onToggleThemeMode);
    connect(paletteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onPaletteChanged);
    connect(fontScaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFontScaleChanged);
    connect(roundedCheck, &QCheckBox::toggled, this, &MainWindow::onRoundedChanged);

    connect(ordersTabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(currentOrdersTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::onEditOrder);
    connect(archivedOrdersTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::onEditOrder);
    connect(statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    connect(archiveSearchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
}

void MainWindow::populateOrdersTable(QTableWidget* table, const std::vector<Order>& ordersList) const {
    table->setRowCount(static_cast<int>(ordersList.size()));
    for (int row = 0; row < static_cast<int>(ordersList.size()); ++row) {
        const Order& order = ordersList[static_cast<std::size_t>(row)];
        const Client* client = storage.findClientById(order.getClientId());
        const Master* master = storage.findMasterById(order.getMasterId());

        QString clientText = client ? QString("%1 (%2)").arg(client->getName(), client->getPhone()) : "Неизвестен";
        QString masterText = master ? master->getName() : "Не назначен";
        QString modelText = order.getDevice()
            ? QString("[%1] %2").arg(formatCategoryRu(order.getDevice()->getCategory()), order.getDevice()->getModel())
            : "Н/Д";
        QString serialText = order.getDevice() ? order.getDevice()->getSerialNumber() : "Н/Д";

        auto* idItem = makeOwned<QTableWidgetItem>(QString::number(order.getId()));
        idItem->setData(Qt::UserRole, order.getId());

        table->setItem(row, 0, idItem);
        table->setItem(row, 1, makeOwned<QTableWidgetItem>(order.getDateReceived()));
        table->setItem(row, 2, makeOwned<QTableWidgetItem>(clientText));
        table->setItem(row, 3, makeOwned<QTableWidgetItem>(modelText));
        table->setItem(row, 4, makeOwned<QTableWidgetItem>(serialText));
        table->setItem(row, 5, makeOwned<QTableWidgetItem>(masterText));
        table->setItem(row, 6, makeOwned<QTableWidgetItem>(formatStatusRu(order.getStatus())));
        table->setItem(row, 7, makeOwned<QTableWidgetItem>(QString::number(order.calculateTotalCost(), 'f', 2)));
    }
}

void MainWindow::refreshOrdersTables() {
    int statusFilter = statusFilterCombo->currentData().toInt();
    std::vector<Order> currentList = WorkshopSearchService::filterCurrentOrders(storage, statusFilter, searchEdit->text());
    populateOrdersTable(currentOrdersTable, currentList);

    std::vector<Order> archivedList = WorkshopSearchService::filterArchivedOrders(storage, archiveSearchEdit->text());
    populateOrdersTable(archivedOrdersTable, archivedList);

    int archivedCount = storage.countArchivedOrders();
    ordersTabs->setTabText(1, QString("Архив заказов (%1)").arg(archivedCount));
    statsLabel->setText(QString("Активных заказов: %1 / %2").arg(storage.countActiveOrders()).arg(Storage::maxActiveOrdersLimit));
    archiveStatsLabel->setText(QString("В архиве: %1").arg(archivedCount));
}

unsigned int MainWindow::getSelectedOrderId() const {
    const QTableWidget* activeTable = (ordersTabs->currentIndex() == 1) ? archivedOrdersTable : currentOrdersTable;
    int currentRow = activeTable->currentRow();
    if (currentRow < 0) {
        return 0;
    }
    const QTableWidgetItem* item = activeTable->item(currentRow, 0);
    return item ? item->data(Qt::UserRole).toUInt() : 0;
}

void MainWindow::exportHtmlToPdf(const QString& dialogTitle, const QString& defaultFileName, const QString& htmlContent, const QString& successMessage) {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        dialogTitle,
        defaultFileName,
        "PDF-документы (*.pdf)"
    );
    if (fileName.isEmpty()) {
        return;
    }
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    doc.setHtml(htmlContent);
    doc.print(&printer);
    QMessageBox::information(this, "Экспорт завершён", successMessage);
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

void MainWindow::onTabChanged(int index) {
    bool isArchive = (index == 1);
    changeStatusBtn->setEnabled(!isArchive);
    editOrderBtn->setText(isArchive ? "Просмотр архива" : "Карточка заказа");
}

void MainWindow::onAddOrder() {
    try {
        storage.checkActiveOrdersLimit();
        OrderDialog dialog(storage, 0, this);
        if (dialog.exec() == QDialog::Accepted) {
            ordersTabs->setCurrentIndex(0);
            refreshOrdersTables();
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
    refreshOrdersTables();
}

void MainWindow::onChangeOrderStatus() {
    if (ordersTabs->currentIndex() == 1) {
        QMessageBox::information(this, "Архив заказов", "Архивные (выданные) заказы недоступны для редактирования.");
        return;
    }
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
    dlg.resize(440, 220);

    QVBoxLayout layout(&dlg);
    QFormLayout form;

    QString deviceInfo = order->getDevice() ? order->getDevice()->getModel() : "Устройство";
    auto* infoLabel = makeOwned<QLabel>(
        QString("<b>%1</b><br>Текущий статус: %2").arg(deviceInfo.toHtmlEscaped(), formatStatusRu(order->getStatus())),
        &dlg
    );
    layout.addWidget(infoLabel);

    auto* statusSelect = makeOwned<QComboBox>(&dlg);
    statusSelect->addItem("Принят", 0);
    statusSelect->addItem("Диагностика", 1);
    statusSelect->addItem("Ожидание запчастей", 2);
    statusSelect->addItem("Завершён", 3);
    statusSelect->addItem("Выдан (переместить в архив)", 4);

    if (int currentIdx = statusSelect->findData(order->getStatusCode()); currentIdx >= 0) {
        statusSelect->setCurrentIndex(currentIdx);
    }

    auto* commentEdit = makeOwned<QLineEdit>(&dlg);
    commentEdit->setPlaceholderText("Комментарий мастера (необязательно)...");

    form.addRow("Новый статус:", statusSelect);
    form.addRow("Примечание:", commentEdit);
    layout.addLayout(&form);

    QHBoxLayout btnRow;
    auto* applyBtn = makeOwned<QPushButton>("Применить статус", &dlg);
    applyBtn->setObjectName("primaryBtn");
    auto* cancelBtn = makeOwned<QPushButton>("Отмена", &dlg);
    btnRow.addStretch(1);
    btnRow.addWidget(applyBtn);
    btnRow.addWidget(cancelBtn);
    layout.addLayout(&btnRow);

    connect(applyBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        try {
            auto newStatus = static_cast<OrderStatus>(statusSelect->currentData().toInt());
            if (newStatus == OrderStatus::Issued) {
                if (auto confirm = QMessageBox::question(
                        this,
                        "Перемещение в архив",
                        QString("После перевода заказа №%1 в статус «Выдан» он будет перемещён в архив и заблокирован для редактирования.\nПродолжить?").arg(selectedId),
                        QMessageBox::Yes | QMessageBox::No
                    ); confirm != QMessageBox::Yes) {
                    return;
                }
            }
            if (!commentEdit->text().trimmed().isEmpty()) {
                storage.appendOrderNote(selectedId, commentEdit->text());
            }
            if (order->getStatus() != newStatus) {
                storage.changeOrderStatus(selectedId, newStatus);
            }
            refreshOrdersTables();
        } catch (const AppException& ex) {
            QMessageBox::warning(this, "Ошибка смены статуса", QString::fromStdString(ex.what()));
        }
    }
}

void MainWindow::onFilterChanged() {
    refreshOrdersTables();
}

void MainWindow::onOpenMasters() {
    MasterDialog dialog(storage, this);
    dialog.exec();
    refreshOrdersTables();
}

void MainWindow::onOpenClients() {
    ClientDialog dialog(storage, this);
    dialog.exec();
    refreshOrdersTables();
}

void MainWindow::onOpenWarehouse() {
    PartDialog dialog(storage, this);
    dialog.exec();
}

void MainWindow::onExportAcceptanceReceiptPdf() {
    unsigned int selectedId = getSelectedOrderId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор заказа", "Пожалуйста, выберите заказ для формирования приёмной квитанции.");
        return;
    }
    try {
        QString html = WorkshopReportService::generateAcceptanceReceiptHtml(storage, selectedId);
        exportHtmlToPdf(
            "Сохранить приёмную квитанцию в PDF",
            QString("Заказ_%1_Квитанция_приема.pdf").arg(selectedId),
            html,
            "Приёмная квитанция (акт приёма-передачи в ремонт) успешно сохранена в файл PDF."
        );
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка экспорта", QString::fromStdString(ex.what()));
    }
}

void MainWindow::onExportWarrantyPdf() {
    unsigned int selectedId = getSelectedOrderId();
    if (selectedId == 0) {
        QMessageBox::information(this, "Выбор заказа", "Пожалуйста, выберите заказ для формирования гарантийного акта.");
        return;
    }
    try {
        QString html = WorkshopReportService::generateWarrantyActHtml(storage, selectedId);
        exportHtmlToPdf(
            "Сохранить расчётно-гарантийный акт в PDF",
            QString("Заказ_%1_Гарантийный_акт.pdf").arg(selectedId),
            html,
            "Расчётно-гарантийный акт выполненных работ успешно сохранён в файл PDF."
        );
    } catch (const AppException& ex) {
        QMessageBox::warning(this, "Ошибка экспорта", QString::fromStdString(ex.what()));
    }
}

void MainWindow::onOpenAccountingWindow() {
    AccountingWindow dialog(storage, this);
    dialog.exec();
    refreshOrdersTables();
}

void MainWindow::onBackupDatabase() {
    try {
        QString path = storage.backupDatabase();
        QMessageBox::information(this, "Резервное копирование", "Резервная копия базы данных сохранена в каталог:\n" + path);
    } catch (const AppException& ex) {
        QMessageBox::critical(this, "Ошибка резервного копирования", QString::fromStdString(ex.what()));
    }
}
