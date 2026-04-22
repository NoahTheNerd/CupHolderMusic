//
// Created by noahg on 3/9/26.
//

#include "settings.h"
#include <QSettings>
#include <QDir>

Settings &Settings::instance() {
    static Settings inst;
    return inst;
}

Settings::Settings()
    : m_settings("CupHolderMusic", "CupHolderMusic")
{}

int Settings::cardSize() const {
    return m_settings.value("library/cardSize", 192).toInt();
}
void Settings::setCardSize(int size) {
    m_settings.setValue("library/cardSize", size);
}

int Settings::cardSpacing() const {
    return m_settings.value("library/cardSpacing", 16).toInt();
}
void Settings::setCardSpacing(int spacing) {
    m_settings.setValue("library/cardSpacing", spacing);
}

SortMode Settings::sortMode() const {
    return static_cast<SortMode>(m_settings.value("library/sortMode", (int)SortMode::AlbumAZ).toInt());
}
void Settings::setSortMode(SortMode mode) {
    m_settings.setValue("library/sortMode", (int)mode);
}

QString Settings::watchedFolder() const {
    return m_settings.value("library/watchedFolder", (QDir::homePath() + "/Music")).toString();
}
void Settings::setWatchedFolder(const QString &path) {
    m_settings.setValue("library/watchedFolder", path);
}

bool Settings::importGenreTags() const {
    return m_settings.value("library/importGenreTags", true).toBool();
}
void Settings::setImportGenreTags(bool import) {
    m_settings.setValue("library/importGenreTags", import);
}