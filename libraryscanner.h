//
// Created by noahg on 3/11/26.
//

#pragma once
#include <QString>

class LibraryScanner {
public:
    static LibraryScanner &instance();
    void scan(const QString &folderPath);
    void rescan(const QString &folderPath);

private:
    LibraryScanner() = default;
    QStringList findAudioFiles(const QString &folderPath);
    void processFile(const QString &filePath);
    QByteArray extractCoverArt(const QString &filePath);
};