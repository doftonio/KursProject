#ifndef WORKERSTAB_H
#define WORKERSTAB_H

#include "../../Headers/Storage.h"
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>

class WorkersTab : public QWidget {
    Q_OBJECT

private:
    Storage& storage;
    bool populatingTable = false;

    QLineEdit* searchEdit;
    QTableWidget* workersTable;

    QLineEdit* nameEdit;
    QLineEdit* phoneEdit;
    QComboBox* addSpecCombo;

    void buildUi();
    unsigned int getSelectedWorkerId() const;
    static QString formatSpecRu(const QString& spec);
    static QString parseSpecFromLabel(const QString& label);

private slots:
    void onSearchChanged(const QString& text);
    void onAddWorker();
    void onRemoveWorker();
    void onWorkerCellChanged(int row, int column);

public:
    explicit WorkersTab(Storage& storageRef, QWidget* parent = nullptr);
    ~WorkersTab() override = default;

    void refreshTable();
};

#endif
