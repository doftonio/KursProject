#ifndef MASTERDIALOG_H
#define MASTERDIALOG_H

#include "../../Headers/Storage.h"
#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class MasterDialog : public QDialog {
    Q_OBJECT

private:
    Storage& storage;

    QLineEdit* searchEdit;
    QTableWidget* mastersTable;

    QLineEdit* nameEdit;
    QLineEdit* phoneEdit;
    QComboBox* addSpecCombo;
    QPushButton* addBtn;

    QComboBox* editSpecCombo;
    QPushButton* updateSpecBtn;
    QPushButton* removeBtn;
    QPushButton* closeBtn;

    void buildUi();
    void refreshTable();
    unsigned int getSelectedMasterId() const;
    static QString formatSpecRu(const QString& spec);

private slots:
    void onSearchChanged(const QString& text);
    void onSelectionChanged();
    void onAddMaster();
    void onUpdateSpecialization();
    void onRemoveMaster();

public:
    explicit MasterDialog(Storage& storageRef, QWidget* parent = nullptr);
    ~MasterDialog() override = default;
};

#endif
