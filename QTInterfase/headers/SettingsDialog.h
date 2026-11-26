#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ThemeManager.h"
#include <QDialog>
#include <QComboBox>
#include <QCheckBox>

class SettingsDialog : public QDialog {
    Q_OBJECT

private:
    ThemeManager& themeManager;
    bool syncingControls = false;

    QComboBox* modeCombo;
    QComboBox* paletteCombo;
    QComboBox* fontScaleCombo;
    QCheckBox* roundedCheck;

    void buildUi();
    void syncFromTheme();

private slots:
    void onModeChanged(int index);
    void onPaletteChanged(int index);
    void onFontScaleChanged(int index);
    void onRoundedToggled(bool checked);
    void onResetDefaults();

public:
    explicit SettingsDialog(ThemeManager& themeRef, QWidget* parent = nullptr);
    ~SettingsDialog() override = default;
};

#endif
