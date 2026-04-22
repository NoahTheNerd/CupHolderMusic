//
// Created by noahg on 3/11/26.
//

#pragma once
#include <QStringList>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QPair>
#include "models/album.h"
#include "models/track.h"

class Database {
public:
    static Database &instance();
    bool open();

    // albums
    int          insertAlbum(const QString &title, int releaseYear,
    const QDateTime &dateAdded, int driveId = -1);
    int albumIdByTitleAndArtist(const QString &title, const QString &artist);
    QVector<Album> allAlbums();
    Album        albumById(int id);
    QMap<QString, QDateTime> albumDateAddedByTitleArtist();
    void restoreAlbumDateAdded(const QMap<QString, QDateTime> &dateMap);
    void         setAlbumCoverArt(int albumId, const QByteArray &coverArt);
    void         setAlbumCoverArtPath(int albumId, const QString &path);
    // album last played
    void recordPlay(int trackId);
    QDateTime firstPlayedForAlbum(int albumId);
    QDateTime lastPlayedForAlbum(int albumId);
    QMap<QString, QList<QDateTime>> playHistoryByFilePath();
    void restorePlayHistory(const QMap<QString, QList<QDateTime>> &historyMap);

    // playlists
    QMap<int, QList<QPair<int, QString>>> playlistTracksByFilePath();
    void restorePlaylistTracks(const QMap<int, QList<QPair<int, QString>>> &playlistMap);

    // artists
    int          insertArtist(const QString &name);
    int          artistId(const QString &name);
    void         linkArtistToAlbum(int albumId, int artistId);

    // tracks
    bool         trackExists(const QString &filePath);
    int          insertTrack(int albumId, const QString &title, int trackNumber,
                             int durationMs, const QString &filePath,
                             const QString &format, int artistId = -1);
    QVector<Track> tracksForAlbum(int albumId);

    // drives
    int          insertDrive(const QString &devicePath, const QString &label);
    void         setCdPresent(int albumId, bool present);

    // tags
    QStringList  allTags();
    QStringList  tagsForAlbum(int albumId);
    void         setTagsForAlbum(int albumId, const QStringList &tags);
    void         createTag(const QString &tag);
    QMap<QString, QStringList> albumTagsByTitleArtist();
    void         restoreAlbumTags(const QMap<QString, QStringList> &tagMap);

    // clear db (minus tags)
    void         clearLibrary();

private:
    Database() = default;
    void initSchema();
};