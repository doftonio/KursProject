#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QColor>

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

struct ThemePaletteColors {
    QString windowBg;
    QString surfaceBg;
    QString altBg;
    QString border;
    QString text;
    QString btnBg;
    QString btnHover;
    QString accent;
    QString accentHover;
};

class ThemeManager {
private:
    ThemeMode mode = ThemeMode::Day;
    ColorPalette palette = ColorPalette::Blue;
    FontScale fontScale = FontScale::Standard;
    bool roundedCorners = true;
    QString settingsPath;
    QString cachedCheckmarkUrl;

    void saveSettings() const;
    void initCheckmarkIcon();
    ThemePaletteColors resolveColors() const;
    QColor resolveAccentColor(bool isNight) const;
    int resolveFontPoints() const;
    void applyPalette() const;
    void applyFont() const;
    QString buildStyleSheet() const;

public:
    explicit ThemeManager(const QString& configPath = "data/ui_theme.ini");

    void loadAndApply();
    void apply(bool updateFont = false) const;

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
