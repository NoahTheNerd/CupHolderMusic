//
// Created by noahg on 3/11/26.
//

#pragma once
#include <QString>

struct Track {
    int     id          = -1;
    int     albumId     = -1;
    QString title;
    int     trackNumber = 0;
    int     durationMs  = 0;
    QString filePath;   // empty if CD track
    QString format;     // "MP3", "FLAC", "WAV", "OGG", "CD"
};