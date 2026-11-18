#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QApplication>
#include <QString>

enum class ThemeMode {
    Day = 0,
    Night = 1
};

enum class ColorPalette {
    Blue = 0,
    Emerald = 1,
    Amber = 2,
    Violet = 3,
    Crimson = 4,
    Graphite = 5
};

enum class FontScale {
    Compact = 0,
    Standard = 1,
    Large = 2
};

class ThemeManager {
private:
    ThemeMode mode;
    ColorPalette palette;
    FontScale fontScale;
    bool roundedCorners;
    QString settingsPath;

    void saveSettings() const;
    QString ensureCheckmarkIcon() const;
    void applyPalette(QApplication& app) const;
    QString buildStyleSheet() const;

public:
    explicit ThemeManager(const QString& configPath = "data/ui_theme.ini");

    void loadAndApply(QApplication& app);
    void apply();

    ThemeMode getMode() const;
    ColorPalette getPalette() const;
    FontScale getFontScale() const;
    bool isRoundedCorners() const;

    void setMode(ThemeMode value);
    void toggleMode();
    void setPalette(ColorPalette value);
    void setFontScale(FontScale value);
    void setRoundedCorners(bool value);
};

#endif
