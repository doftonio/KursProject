#include "../headers/ThemeManager.h"
#include <QSettings>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QPainter>
#include <QPen>

ThemeManager::ThemeManager(const QString& configPath)
    : mode(ThemeMode::Day),
      palette(ColorPalette::Blue),
      fontScale(FontScale::Standard),
      roundedCorners(true),
      settingsPath(configPath) {}

QString ThemeManager::ensureCheckmarkIcon() const {
    QFileInfo info(settingsPath);
    QDir dir;
    if (!dir.exists(info.path())) {
        dir.mkpath(info.path());
    }
    QString iconPath = info.absolutePath() + "/checkmark.png";
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
    return QFileInfo(iconPath).absoluteFilePath();
}

void ThemeManager::loadAndApply(QApplication& app) {
    QFileInfo info(settingsPath);
    QDir dir;
    if (!dir.exists(info.path())) {
        dir.mkpath(info.path());
    }

    QSettings settings(settingsPath, QSettings::IniFormat);
    mode = static_cast<ThemeMode>(settings.value("mode", 0).toInt());
    palette = static_cast<ColorPalette>(settings.value("palette", 0).toInt());
    fontScale = static_cast<FontScale>(settings.value("fontScale", 1).toInt());
    roundedCorners = settings.value("roundedCorners", true).toBool();

    app.setStyle(QStyleFactory::create("Fusion"));
    apply();
}

void ThemeManager::saveSettings() const {
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("mode", static_cast<int>(mode));
    settings.setValue("palette", static_cast<int>(palette));
    settings.setValue("fontScale", static_cast<int>(fontScale));
    settings.setValue("roundedCorners", roundedCorners);
}

void ThemeManager::applyPalette(QApplication& app) const {
    bool isNight = (mode == ThemeMode::Night);
    QPalette pal;

    QColor windowBg = isNight ? QColor("#12151C") : QColor("#F4F6F9");
    QColor surfaceBg = isNight ? QColor("#1B202B") : QColor("#FFFFFF");
    QColor altRowBg = isNight ? QColor("#222836") : QColor("#F8FAFC");
    QColor textPrimary = isNight ? QColor("#E2E8F0") : QColor("#1E293B");
    QColor textDisabled = isNight ? QColor("#64748B") : QColor("#94A3B8");
    QColor buttonBg = isNight ? QColor("#262D3D") : QColor("#E2E8F0");

    QColor accent;
    switch (palette) {
        case ColorPalette::Blue:
            accent = isNight ? QColor("#3B82F6") : QColor("#2563EB");
            break;
        case ColorPalette::Emerald:
            accent = isNight ? QColor("#10B981") : QColor("#059669");
            break;
        case ColorPalette::Amber:
            accent = isNight ? QColor("#F59E0B") : QColor("#D97706");
            break;
        case ColorPalette::Violet:
            accent = isNight ? QColor("#8B5CF6") : QColor("#7C3AED");
            break;
        case ColorPalette::Crimson:
            accent = isNight ? QColor("#EF4444") : QColor("#DC2626");
            break;
        case ColorPalette::Graphite:
            accent = isNight ? QColor("#94A3B8") : QColor("#475569");
            break;
    }

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

    app.setPalette(pal);
}

QString ThemeManager::buildStyleSheet() const {
    bool isNight = (mode == ThemeMode::Night);
    QString checkUrl = ensureCheckmarkIcon();

    QString windowBg = isNight ? "#12151C" : "#F4F6F9";
    QString surfaceBg = isNight ? "#1B202B" : "#FFFFFF";
    QString altBg = isNight ? "#222836" : "#F8FAFC";
    QString border = isNight ? "#2E3748" : "#CBD5E1";
    QString text = isNight ? "#E2E8F0" : "#1E293B";
    QString btnBg = isNight ? "#262D3D" : "#FFFFFF";
    QString btnHover = isNight ? "#323B4F" : "#E2E8F0";

    QString accent;
    QString accentHover;
    switch (palette) {
        case ColorPalette::Blue:
            accent = isNight ? "#3B82F6" : "#2563EB";
            accentHover = isNight ? "#60A5FA" : "#1D4ED8";
            break;
        case ColorPalette::Emerald:
            accent = isNight ? "#10B981" : "#059669";
            accentHover = isNight ? "#34D399" : "#047857";
            break;
        case ColorPalette::Amber:
            accent = isNight ? "#F59E0B" : "#D97706";
            accentHover = isNight ? "#FBBF24" : "#B45309";
            break;
        case ColorPalette::Violet:
            accent = isNight ? "#8B5CF6" : "#7C3AED";
            accentHover = isNight ? "#A78BFA" : "#6D28D9";
            break;
        case ColorPalette::Crimson:
            accent = isNight ? "#EF4444" : "#DC2626";
            accentHover = isNight ? "#F87171" : "#B91C1C";
            break;
        case ColorPalette::Graphite:
            accent = isNight ? "#64748B" : "#475569";
            accentHover = isNight ? "#94A3B8" : "#334155";
            break;
    }

    int fontPt = 10;
    if (fontScale == FontScale::Compact) {
        fontPt = 9;
    } else if (fontScale == FontScale::Large) {
        fontPt = 11;
    }

    int radius = roundedCorners ? 6 : 2;

    return QString(
        "QWidget { background-color: %1; color: %2; font-family: 'Segoe UI', Arial, sans-serif; font-size: %3pt; }"
        "QMainWindow, QDialog { background-color: %1; }"
        "QGroupBox { border: 1px solid %4; border-radius: %5px; margin-top: 12px; padding-top: 10px; font-weight: bold; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 6px; color: %6; }"
        "QLineEdit, QTextEdit, QComboBox, QSpinBox, QDoubleSpinBox { background-color: %7; color: %2; border: 1px solid %4; border-radius: %5px; padding: 5px 8px; selection-background-color: %6; selection-color: #FFFFFF; }"
        "QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus { border: 2px solid %6; }"
        "QComboBox::drop-down { border: none; width: 22px; }"
        "QComboBox QAbstractItemView { background-color: %7; color: %2; border: 1px solid %4; selection-background-color: %6; selection-color: #FFFFFF; }"
        "QPushButton { background-color: %8; color: %2; border: 1px solid %4; border-radius: %5px; padding: 6px 12px; font-weight: 600; }"
        "QPushButton:hover { background-color: %9; border-color: %6; }"
        "QPushButton:pressed { background-color: %6; color: #FFFFFF; }"
        "QPushButton#primaryBtn { background-color: %6; color: #FFFFFF; border: 1px solid %6; }"
        "QPushButton#primaryBtn:hover { background-color: %10; border-color: %10; }"
        "QPushButton#dangerBtn { border: 1px solid #DC2626; color: #DC2626; }"
        "QPushButton#dangerBtn:hover { background-color: #DC2626; color: #FFFFFF; }"
        "QTableWidget { background-color: %7; alternate-background-color: %11; color: %2; gridline-color: %4; border: 1px solid %4; border-radius: %5px; selection-background-color: %6; selection-color: #FFFFFF; }"
        "QHeaderView::section { background-color: %8; color: %2; padding: 6px; border: none; border-bottom: 2px solid %6; border-right: 1px solid %4; font-weight: bold; }"
        "QCheckBox { spacing: 7px; color: %2; }"
        "QCheckBox::indicator { width: 17px; height: 17px; border: 1px solid %4; border-radius: 4px; background-color: %7; }"
        "QCheckBox::indicator:hover { border: 1px solid %6; }"
        "QCheckBox::indicator:checked { background-color: %6; border-color: %6; image: url(\"%13\"); }"
        "QLabel#statsBadge { background-color: %7; border: 1px solid %6; border-radius: %5px; padding: 5px 10px; font-weight: bold; color: %6; }"
        "QLabel#totalHighlight { font-size: %12pt; font-weight: bold; color: %6; }"
        "QFrame#themeBar { background-color: %7; border: 1px solid %4; border-radius: %5px; }"
    )
    .arg(windowBg)
    .arg(text)
    .arg(fontPt)
    .arg(border)
    .arg(radius)
    .arg(accent)
    .arg(surfaceBg)
    .arg(btnBg)
    .arg(btnHover)
    .arg(accentHover)
    .arg(altBg)
    .arg(fontPt + 2)
    .arg(checkUrl);
}

void ThemeManager::apply() {
    auto* app = qobject_cast<QApplication*>(QApplication::instance());
    if (!app) {
        return;
    }
    QFont font = app->font();
    if (fontScale == FontScale::Compact) {
        font.setPointSize(9);
    } else if (fontScale == FontScale::Large) {
        font.setPointSize(11);
    } else {
        font.setPointSize(10);
    }
    app->setFont(font);
    applyPalette(*app);
    app->setStyleSheet(buildStyleSheet());
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
    mode = value;
    saveSettings();
    apply();
}

void ThemeManager::toggleMode() {
    mode = (mode == ThemeMode::Day) ? ThemeMode::Night : ThemeMode::Day;
    saveSettings();
    apply();
}

void ThemeManager::setPalette(ColorPalette value) {
    palette = value;
    saveSettings();
    apply();
}

void ThemeManager::setFontScale(FontScale value) {
    fontScale = value;
    saveSettings();
    apply();
}

void ThemeManager::setRoundedCorners(bool value) {
    roundedCorners = value;
    saveSettings();
    apply();
}
