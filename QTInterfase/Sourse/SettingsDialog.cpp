#include "../headers/SettingsDialog.h"
#include "../headers/UiHelpers.h"
#include "../../Headers/EnumUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

SettingsDialog::SettingsDialog(ThemeManager& themeRef, QWidget* parent)
    : QDialog(parent), themeManager(themeRef) {
    buildUi();
    syncFromTheme();
}

void SettingsDialog::buildUi() {
    setWindowTitle("Настройки программы");
    setMinimumWidth(460);

    auto* mainLayout = makeOwned<QVBoxLayout>(this);
    mainLayout->setSpacing(12);

    auto* appearanceGroup = makeOwned<QGroupBox>("Внешний вид и читаемость интерфейса", this);
    auto* formLayout = makeOwned<QFormLayout>(appearanceGroup);
    formLayout->setSpacing(10);

    modeCombo = makeOwned<QComboBox>(this);
    modeCombo->addItem("Дневной режим", toUnderlying(ThemeMode::Day));
    modeCombo->addItem("Ночной режим", toUnderlying(ThemeMode::Night));

    paletteCombo = makeOwned<QComboBox>(this);
    paletteCombo->addItem("Синяя (классическая)", toUnderlying(ColorPalette::Blue));
    paletteCombo->addItem("Изумрудная", toUnderlying(ColorPalette::Emerald));
    paletteCombo->addItem("Янтарная", toUnderlying(ColorPalette::Amber));
    paletteCombo->addItem("Фиолетовая", toUnderlying(ColorPalette::Violet));
    paletteCombo->addItem("Рубиновая", toUnderlying(ColorPalette::Crimson));
    paletteCombo->addItem("Графитовая", toUnderlying(ColorPalette::Graphite));

    fontScaleCombo = makeOwned<QComboBox>(this);
    fontScaleCombo->addItem("Компактный (9 pt)", toUnderlying(FontScale::Compact));
    fontScaleCombo->addItem("Стандартный (10 pt)", toUnderlying(FontScale::Standard));
    fontScaleCombo->addItem("Крупный (11 pt)", toUnderlying(FontScale::Large));

    roundedCheck = makeOwned<QCheckBox>("Использовать скруглённые углы элементов", this);

    formLayout->addRow("Режим темы:", modeCombo);
    formLayout->addRow("Цветовая палитра:", paletteCombo);
    formLayout->addRow("Масштаб шрифта:", fontScaleCombo);
    formLayout->addRow("Геометрия рамок:", roundedCheck);

    auto* hintLabel = makeOwned<QLabel>(
        "Все изменения применяются мгновенно и автоматически сохраняются в файле конфигурации.",
        this
    );
    hintLabel->setWordWrap(true);

    auto* buttonsLayout = makeOwned<QHBoxLayout>();
    auto* resetBtn = makeOwned<QPushButton>("Сбросить по умолчанию", this);
    auto* closeBtn = makeOwned<QPushButton>("Закрыть", this);
    closeBtn->setObjectName("primaryBtn");

    buttonsLayout->addWidget(resetBtn);
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeBtn);

    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onModeChanged);
    connect(paletteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onPaletteChanged);
    connect(fontScaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onFontScaleChanged);
    connect(roundedCheck, &QCheckBox::toggled, this, &SettingsDialog::onRoundedToggled);
    connect(resetBtn, &QPushButton::clicked, this, &SettingsDialog::onResetDefaults);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    mainLayout->addWidget(appearanceGroup);
    mainLayout->addWidget(hintLabel);
    mainLayout->addLayout(buttonsLayout);
}

void SettingsDialog::syncFromTheme() {
    syncingControls = true;

    if (int modeIdx = modeCombo->findData(toUnderlying(themeManager.getMode())); modeIdx >= 0) {
        modeCombo->setCurrentIndex(modeIdx);
    }

    if (int palIdx = paletteCombo->findData(toUnderlying(themeManager.getPalette())); palIdx >= 0) {
        paletteCombo->setCurrentIndex(palIdx);
    }

    if (int fontIdx = fontScaleCombo->findData(toUnderlying(themeManager.getFontScale())); fontIdx >= 0) {
        fontScaleCombo->setCurrentIndex(fontIdx);
    }

    roundedCheck->setChecked(themeManager.isRoundedCorners());
    syncingControls = false;
}

void SettingsDialog::onModeChanged(int index) {
    if (syncingControls || index < 0) {
        return;
    }
    auto selected = static_cast<ThemeMode>(modeCombo->itemData(index).toInt());
    themeManager.setMode(selected);
}

void SettingsDialog::onPaletteChanged(int index) {
    if (syncingControls || index < 0) {
        return;
    }
    auto selected = static_cast<ColorPalette>(paletteCombo->itemData(index).toInt());
    themeManager.setPalette(selected);
}

void SettingsDialog::onFontScaleChanged(int index) {
    if (syncingControls || index < 0) {
        return;
    }
    auto selected = static_cast<FontScale>(fontScaleCombo->itemData(index).toInt());
    themeManager.setFontScale(selected);
}

void SettingsDialog::onRoundedToggled(bool checked) {
    if (syncingControls) {
        return;
    }
    themeManager.setRoundedCorners(checked);
}

void SettingsDialog::onResetDefaults() {
    themeManager.setMode(ThemeMode::Day);
    themeManager.setPalette(ColorPalette::Blue);
    themeManager.setFontScale(FontScale::Standard);
    themeManager.setRoundedCorners(true);
    syncFromTheme();
}
