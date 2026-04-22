//
// Created by noahg on 3/9/26.
//

#pragma once
#include <QSettings>
#include <QString>
#include "views/libraryview.h" // for SortMode

class Settings {
public:
    static Settings &instance();

    int cardSize() const;
    void setCardSize(int size);

    int cardSpacing() const;
    void setCardSpacing(int spacing);

    SortMode sortMode() const;
    void setSortMode(SortMode mode);

    QString watchedFolder() const;
    void setWatchedFolder(QString const &path);

    bool importGenreTags() const;
    void setImportGenreTags(bool import);

private:
    Settings();
    QSettings m_settings;
};