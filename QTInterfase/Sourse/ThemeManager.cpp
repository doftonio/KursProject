#include "../headers/ThemeManager.h"
#include "../../Headers/EnumUtils.h"
#include <QApplication>
#include <QWidget>
#include <QSettings>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QPainter>
#include <QPen>

ThemeManager::ThemeManager(const QString& configPath)
    : settingsPath(configPath) {}

void ThemeManager::initCheckmarkIcon() {
    QFileInfo info(settingsPath);
    if (QDir dir; !dir.exists(info.path())) {
        dir.mkpath(info.path());
    }
    QString iconPath = info.absolutePath() + "/checkmark.png";
    if (!QFileInfo::exists(iconPath)) {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPen pen(Qt::white, 2.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        painter.drawLine(QPointF(3.5, 8.5), QPointF(6.8, 11.8));
        painter.drawLine(QPointF(6.8, 11.8), QPointF(12.8, 4.5));
        painter.end();

        pixmap.save(iconPath, "PNG");
    }
    cachedCheckmarkUrl = QFileInfo(iconPath).absoluteFilePath();
}

void ThemeManager::loadAndApply() {
    initCheckmarkIcon();

    QSettings settings(settingsPath, QSettings::IniFormat);
    mode = static_cast<ThemeMode>(settings.value("mode", 0).toInt());
    palette = static_cast<ColorPalette>(settings.value("palette", 0).toInt());
    fontScale = static_cast<FontScale>(settings.value("fontScale", 1).toInt());
    roundedCorners = settings.value("roundedCorners", true).toBool();

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    apply(true);
}

void ThemeManager::saveSettings() const {
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("mode", toUnderlying(mode));
    settings.setValue("palette", toUnderlying(palette));
    settings.setValue("fontScale", toUnderlying(fontScale));
    settings.setValue("roundedCorners", roundedCorners);
}

QColor ThemeManager::resolveAccentColor(bool isNight) const {
    using enum ColorPalette;
    switch (palette) {
        case Emerald:
            return isNight ? QColor("#10B981") : QColor("#059669");
        case Amber:
            return isNight ? QColor("#F59E0B") : QColor("#D97706");
        case Violet:
            return isNight ? QColor("#8B5CF6") : QColor("#7C3AED");
        case Crimson:
            return isNight ? QColor("#EF4444") : QColor("#DC2626");
        case Graphite:
            return isNight ? QColor("#64748B") : QColor("#475569");
        case Blue:
        default:
            return isNight ? QColor("#3B82F6") : QColor("#2563EB");
    }
}

ThemePaletteColors ThemeManager::resolveColors() const {
    bool isNight = (mode == ThemeMode::Night);
    QColor accentColor = resolveAccentColor(isNight);
    QColor hoverColor = isNight ? accentColor.lighter(120) : accentColor.darker(115);

    if (isNight) {
        return {
            "#12151C",
            "#1B202B",
            "#222836",
            "#2E3748",
            "#E2E8F0",
            "#262D3D",
            "#323B4F",
            accentColor.name(),
            hoverColor.name()
        };
    }
    return {
        "#F4F6F9",
        "#FFFFFF",
        "#F8FAFC",
        "#CBD5E1",
        "#1E293B",
        "#FFFFFF",
        "#E2E8F0",
        accentColor.name(),
        hoverColor.name()
    };
}

int ThemeManager::resolveFontPoints() const {
    using enum FontScale;
    switch (fontScale) {
        case Compact:
            return 9;
        case Large:
            return 11;
        case Standard:
        default:
            return 10;
    }
}

void ThemeManager::applyPalette() const {
    bool isNight = (mode == ThemeMode::Night);
    QPalette pal;

    QColor windowBg = isNight ? QColor("#12151C") : QColor("#F4F6F9");
    QColor surfaceBg = isNight ? QColor("#1B202B") : QColor("#FFFFFF");
    QColor altRowBg = isNight ? QColor("#222836") : QColor("#F8FAFC");
    QColor textPrimary = isNight ? QColor("#E2E8F0") : QColor("#1E293B");
    QColor textDisabled = isNight ? QColor("#64748B") : QColor("#94A3B8");
    QColor buttonBg = isNight ? QColor("#262D3D") : QColor("#E2E8F0");
    QColor accent = resolveAccentColor(isNight);

    pal.setColor(QPalette::Window, windowBg);
    pal.setColor(QPalette::WindowText, textPrimary);
    pal.setColor(QPalette::Base, surfaceBg);
    pal.setColor(QPalette::AlternateBase, altRowBg);
    pal.setColor(QPalette::ToolTipBase, surfaceBg);
    pal.setColor(QPalette::ToolTipText, textPrimary);
    pal.setColor(QPalette::Text, textPrimary);
    pal.setColor(QPalette::Button, buttonBg);
    pal.setColor(QPalette::ButtonText, textPrimary);
    pal.setColor(QPalette::BrightText, Qt::white);
    pal.setColor(QPalette::Highlight, accent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::Disabled, QPalette::Text, textDisabled);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, textDisabled);

    QApplication::setPalette(pal);
}

void ThemeManager::applyFont() const {
    QFont font("Segoe UI", resolveFontPoints());
    QApplication::setFont(font);
}

QString ThemeManager::buildStyleSheet() const {
    ThemePaletteColors c = resolveColors();
    int fontPt = resolveFontPoints();
    int radius = roundedCorners ? 6 : 2;

    return QString(R"(QTabWidget::pane { border: 1px solid %3; border-radius: %4px; background-color: %1; top: -1px; }QTabBar::tab { background-color: %7; color: %2; border: 1px solid %3; border-bottom: none; border-top-left-radius: %4px; border-top-right-radius: %4px; padding: 7px 16px; margin-right: 3px; font-weight: 600; }QTabBar::tab:selected { background-color: %5; color: #FFFFFF; border-color: %5; }QTabBar::tab:hover:!selected { background-color: %8; }QGroupBox { border: 1px solid %3; border-radius: %4px; margin-top: 12px; padding-top: 10px; font-weight: bold; }QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 6px; color: %5; }QLineEdit, QTextEdit, QComboBox, QSpinBox, QDoubleSpinBox { background-color: %6; color: %2; border: 1px solid %3; border-radius: %4px; padding: 5px 8px; selection-background-color: %5; selection-color: #FFFFFF; }QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus { border: 2px solid %5; }QLineEdit[lockedField="true"], QLineEdit[lockedField="true"]:disabled, QTextEdit[lockedField="true"], QTextEdit[lockedField="true"]:disabled, QComboBox[lockedField="true"], QComboBox[lockedField="true"]:disabled, QSpinBox[lockedField="true"], QSpinBox[lockedField="true"]:disabled, QDoubleSpinBox[lockedField="true"], QDoubleSpinBox[lockedField="true"]:disabled, QCheckBox[lockedField="true"], QCheckBox[lockedField="true"]:disabled { color: #7A869A; }QComboBox::drop-down { border: none; width: 22px; }QComboBox QAbstractItemView { background-color: %6; color: %2; border: 1px solid %3; selection-background-color: %5; selection-color: #FFFFFF; }QPushButton { background-color: %7; color: %2; border: 1px solid %3; border-radius: %4px; padding: 6px 12px; font-weight: 600; }QPushButton:hover { background-color: %8; border-color: %5; }QPushButton:pressed { background-color: %5; color: #FFFFFF; }QPushButton:disabled { opacity: 0.45; }QPushButton#primaryBtn { background-color: %5; color: #FFFFFF; border: 1px solid %5; }QPushButton#primaryBtn:hover { background-color: %9; border-color: %9; }QPushButton#dangerBtn { border: 1px solid #DC2626; color: #DC2626; }QPushButton#dangerBtn:hover { background-color: #DC2626; color: #FFFFFF; }QTableWidget { background-color: %6; alternate-background-color: %10; color: %2; gridline-color: %3; border: 1px solid %3; border-radius: %4px; selection-background-color: %5; selection-color: #FFFFFF; }QHeaderView::section { background-color: %7; color: %2; padding: 6px; border: none; border-bottom: 2px solid %5; border-right: 1px solid %3; font-weight: bold; }QCheckBox { spacing: 7px; color: %2; }QCheckBox::indicator { width: 17px; height: 17px; border: 1px solid %3; border-radius: 4px; background-color: %6; }QCheckBox::indicator:hover { border: 1px solid %5; }QCheckBox::indicator:checked { background-color: %5; border-color: %5; image: url("%12"); }QLabel#statsBadge { background-color: %6; border: 1px solid %5; border-radius: %4px; padding: 5px 10px; font-weight: bold; color: %5; }QLabel#archiveNotice { background-color: %6; border: 1px solid %3; border-radius: %4px; padding: 5px 10px; font-weight: 600; }QLabel#totalHighlight { font-size: %11pt; font-weight: bold; color: %5; }QFrame#themeBar { background-color: %6; border: 1px solid %3; border-radius: %4px; }QFrame#lockedOverlay { background-color: rgba(120, 130, 145, 115); border: 1px dashed %3; border-radius: %4px; })").arg(c.windowBg)
    .arg(c.text)
    .arg(c.border)
    .arg(radius)
    .arg(c.accent)
    .arg(c.surfaceBg)
    .arg(c.btnBg)
    .arg(c.btnHover)
    .arg(c.accentHover)
    .arg(c.altBg)
    .arg(fontPt + 2)
    .arg(cachedCheckmarkUrl);
}

void ThemeManager::apply(bool updateFont) const {
    auto* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) {
        return;
    }

    const auto topWidgets = QApplication::topLevelWidgets();
    for (QWidget* w : topWidgets) {
        w->setUpdatesEnabled(false);
    }

    if (updateFont) {
        applyFont();
    }
    applyPalette();
    app->setStyleSheet(buildStyleSheet());

    for (QWidget* w : topWidgets) {
        w->setUpdatesEnabled(true);
        w->update();
    }
}

ThemeMode ThemeManager::getMode() const {
    return mode;
}

ColorPalette ThemeManager::getPalette() const {
    return palette;
}

FontScale ThemeManager::getFontScale() const {
    return fontScale;
}

bool ThemeManager::isRoundedCorners() const {
    return roundedCorners;
}

void ThemeManager::setMode(ThemeMode value) {
    if (mode == value) {
        return;
    }
    mode = value;
    saveSettings();
    apply(false);
}

void ThemeManager::toggleMode() {
    using enum ThemeMode;
    mode = (mode == Day) ? Night : Day;
    saveSettings();
    apply(false);
}

void ThemeManager::setPalette(ColorPalette value) {
    if (palette == value) {
        return;
    }
    palette = value;
    saveSettings();
    apply(false);
}

void ThemeManager::setFontScale(FontScale value) {
    if (fontScale == value) {
        return;
    }
    fontScale = value;
    saveSettings();
    apply(true);
}

void ThemeManager::setRoundedCorners(bool value) {
    if (roundedCorners == value) {
        return;
    }
    roundedCorners = value;
    saveSettings();
    apply(false);
}
