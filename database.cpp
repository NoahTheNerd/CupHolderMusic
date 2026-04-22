//
// Created by noahg on 3/11/26.
//

#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

Database &Database::instance() {
    static Database inst;
    return inst;
}

bool Database::open() {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dataDir + "/library.db");

    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError();
        return false;
    }

    initSchema();
    return true;
}

void Database::initSchema() {
    QSqlQuery q;

    q.exec(
        "CREATE TABLE IF NOT EXISTS artists ("
        "  id   INTEGER PRIMARY KEY,"
        "  name TEXT NOT NULL UNIQUE"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS drives ("
        "  id          INTEGER PRIMARY KEY,"
        "  device_path TEXT NOT NULL UNIQUE,"
        "  label       TEXT"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS albums ("
        "  id             INTEGER PRIMARY KEY,"
        "  title          TEXT NOT NULL,"
        "  release_year   INTEGER,"
        "  cover_art      BLOB,"
        "  cover_art_path TEXT,"
        "  drive_id       INTEGER,"
        "  date_added     TEXT NOT NULL,"
        "  FOREIGN KEY (drive_id) REFERENCES drives(id)"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS album_artists ("
        "  album_id  INTEGER,"
        "  artist_id INTEGER,"
        "  PRIMARY KEY (album_id, artist_id),"
        "  FOREIGN KEY (album_id)  REFERENCES albums(id),"
        "  FOREIGN KEY (artist_id) REFERENCES artists(id)"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS tracks ("
        "  id           INTEGER PRIMARY KEY,"
        "  album_id     INTEGER NOT NULL,"
        "  artist_id    INTEGER,"
        "  title        TEXT NOT NULL,"
        "  track_number INTEGER,"
        "  duration_ms  INTEGER,"
        "  file_path    TEXT,"
        "  format       TEXT,"
        "  FOREIGN KEY (album_id)  REFERENCES albums(id),"
        "  FOREIGN KEY (artist_id) REFERENCES artists(id)"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS tags ("
        "  id   INTEGER PRIMARY KEY,"
        "  name TEXT NOT NULL UNIQUE"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS album_tags ("
        "  album_id INTEGER,"
        "  tag_id   INTEGER,"
        "  PRIMARY KEY (album_id, tag_id),"
        "  FOREIGN KEY (album_id) REFERENCES albums(id),"
        "  FOREIGN KEY (tag_id)   REFERENCES tags(id)"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS play_history ("
        "  id        INTEGER PRIMARY KEY,"
        "  track_id  INTEGER NOT NULL,"
        "  played_at TEXT NOT NULL,"
        "  FOREIGN KEY (track_id) REFERENCES tracks(id)"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS lyrics ("
        "  id                INTEGER PRIMARY KEY,"
        "  track_id          INTEGER NOT NULL UNIQUE,"
        "  provider          TEXT NOT NULL,"
        "  lrclib_version_id INTEGER,"
        "  content           TEXT NOT NULL,"
        "  is_synced         INTEGER NOT NULL DEFAULT 0,"
        "  FOREIGN KEY (track_id) REFERENCES tracks(id)"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS playlists ("
        "  id   INTEGER PRIMARY KEY,"
        "  name TEXT NOT NULL"
        ")"
    );

    q.exec(
        "CREATE TABLE IF NOT EXISTS playlist_tracks ("
        "  id          INTEGER PRIMARY KEY,"
        "  playlist_id INTEGER NOT NULL,"
        "  track_id    INTEGER NOT NULL,"
        "  position    INTEGER NOT NULL,"
        "  FOREIGN KEY (playlist_id) REFERENCES playlists(id),"
        "  FOREIGN KEY (track_id)    REFERENCES tracks(id)"
        ")"
    );
}

// --- albums ---

int Database::insertAlbum(const QString &title, int releaseYear,
                           const QDateTime &dateAdded, int driveId) {
    QSqlQuery q;
    q.prepare(
        "INSERT INTO albums (title, release_year, date_added, drive_id)"
        " VALUES (:title, :year, :dateAdded, :driveId)"
    );
    q.bindValue(":title",     title);
    q.bindValue(":year",      releaseYear);
    q.bindValue(":dateAdded", dateAdded.toString(Qt::ISODate));
    q.bindValue(":driveId",   driveId == -1 ? QVariant() : driveId);
    q.exec();
    return q.lastInsertId().toInt();
}

void Database::setAlbumCoverArt(int albumId, const QByteArray &coverArt) {
    QSqlQuery q;
    q.prepare("UPDATE albums SET cover_art = :art WHERE id = :id");
    q.bindValue(":art", coverArt);
    q.bindValue(":id",  albumId);
    q.exec();
}

void Database::setAlbumCoverArtPath(int albumId, const QString &path) {
    QSqlQuery q;
    q.prepare("UPDATE albums SET cover_art_path = :path WHERE id = :id");
    q.bindValue(":path", path);
    q.bindValue(":id",   albumId);
    q.exec();
}

QVector<Album> Database::allAlbums() {
    QVector<Album> albums;
    QSqlQuery q(
        "SELECT a.id, a.title, a.release_year, a.date_added,"
        "       a.cover_art, a.cover_art_path,"
        "       GROUP_CONCAT(DISTINCT ar.name) as artists"
        " FROM albums a"
        " LEFT JOIN album_artists aa ON aa.album_id = a.id"
        " LEFT JOIN artists ar ON ar.id = aa.artist_id"
        " GROUP BY a.id"
    );

    while (q.next()) {
        Album album;
        album.id          = q.value(0).toInt();
        album.title       = q.value(1).toString();
        album.releaseYear = q.value(2).toInt();
        album.dateAdded   = QDateTime::fromString(q.value(3).toString(), Qt::ISODate);
        album.artist      = q.value(6).toString();
        album.tracks      = tracksForAlbum(album.id);
        album.trackCount  = album.tracks.size();
        album.tags        = tagsForAlbum(album.id);
        album.firstPlayed = firstPlayedForAlbum(album.id);
        album.lastPlayed = lastPlayedForAlbum(album.id);

        // cover art path takes priority over blob
        QString coverPath = q.value(5).toString();
        if (!coverPath.isEmpty()) {
            album.coverArt = QPixmap(coverPath);
        } else {
            QByteArray blob = q.value(4).toByteArray();
            if (!blob.isEmpty()) {
                QPixmap px;
                px.loadFromData(blob);
                album.coverArt = px;
            }
        }

        albums.append(album);
    }
    return albums;
}

Album Database::albumById(int id) {
    QSqlQuery q;
    q.prepare(
        "SELECT a.id, a.title, a.release_year, a.date_added,"
        "       a.cover_art, a.cover_art_path,"
        "       GROUP_CONCAT(DISTINCT ar.name) as artists"
        " FROM albums a"
        " LEFT JOIN album_artists aa ON aa.album_id = a.id"
        " LEFT JOIN artists ar ON ar.id = aa.artist_id"
        " WHERE a.id = :id"
        " GROUP BY a.id"
    );
    q.bindValue(":id", id);
    q.exec();

    Album album;
    if (q.next()) {
        album.id          = q.value(0).toInt();
        album.title       = q.value(1).toString();
        album.releaseYear = q.value(2).toInt();
        album.dateAdded   = QDateTime::fromString(q.value(3).toString(), Qt::ISODate);
        album.artist      = q.value(6).toString();
        album.tracks      = tracksForAlbum(album.id);
        album.trackCount  = album.tracks.size();
        album.tags        = tagsForAlbum(album.id);
        album.firstPlayed = firstPlayedForAlbum(album.id);
        album.lastPlayed  = lastPlayedForAlbum(album.id);

        QString coverPath = q.value(5).toString();
        if (!coverPath.isEmpty()) {
            album.coverArt = QPixmap(coverPath);
        } else {
            QByteArray blob = q.value(4).toByteArray();
            if (!blob.isEmpty()) {
                QPixmap px;
                px.loadFromData(blob);
                album.coverArt = px;
            }
        }
    }
    return album;
}

int Database::albumIdByTitleAndArtist(const QString &title, const QString &artist) {
    QSqlQuery q;
    q.prepare(
        "SELECT a.id FROM albums a"
        " JOIN album_artists aa ON aa.album_id = a.id"
        " JOIN artists ar ON ar.id = aa.artist_id"
        " WHERE a.title = :title AND ar.name = :artist"
        " LIMIT 1"
    );
    q.bindValue(":title",  title);
    q.bindValue(":artist", artist);
    q.exec();
    if (q.next()) return q.value(0).toInt();
    return -1;
}

QMap<QString, QDateTime> Database::albumDateAddedByTitleArtist() {
    QMap<QString, QDateTime> result;
    QSqlQuery q(
        "SELECT a.title, ar.name, a.date_added FROM albums a "
        "JOIN album_artists aa ON aa.album_id = a.id "
        "JOIN artists ar ON ar.id = aa.artist_id"
    );

    while (q.next()) {
        const QString key = q.value(0).toString() + "||" + q.value(1).toString();
        const QDateTime dt = QDateTime::fromString(q.value(2).toString(), Qt::ISODate);
        if (!result.contains(key) || (dt.isValid() && dt < result.value(key))) {
            result[key] = dt;
        }
    }
    return result;
}

void Database::restoreAlbumDateAdded(const QMap<QString, QDateTime> &dateMap) {
    for (auto it = dateMap.begin(); it != dateMap.end(); ++it) {
        const QStringList parts = it.key().split("||");
        if (parts.size() != 2) continue;
        const int albumId = albumIdByTitleAndArtist(parts[0], parts[1]);
        if (albumId == -1 || !it.value().isValid()) continue;

        QSqlQuery q;
        q.prepare("UPDATE albums SET date_added = :dateAdded WHERE id = :id");
        q.bindValue(":dateAdded", it.value().toString(Qt::ISODate));
        q.bindValue(":id", albumId);
        q.exec();
    }
}

// --- album last played ---

void Database::recordPlay(int trackId) {
    QSqlQuery q;
    q.prepare("INSERT INTO play_history (track_id, played_at) VALUES (:trackId, :playedAt)");
    q.bindValue(":trackId",  trackId);
    q.bindValue(":playedAt", QDateTime::currentDateTime().toString(Qt::ISODate));
    q.exec();
}

QDateTime Database::firstPlayedForAlbum(int albumId) {
    QSqlQuery q;
    q.prepare(
        "SELECT MIN(ph.played_at) FROM play_history ph "
        "JOIN tracks t ON t.id = ph.track_id "
        "WHERE t.album_id = :albumId"
    );
    q.bindValue(":albumId", albumId);
    q.exec();
    if (q.next() && !q.value(0).isNull())
        return QDateTime::fromString(q.value(0).toString(), Qt::ISODate);
    return QDateTime();
}

QDateTime Database::lastPlayedForAlbum(int albumId) {
    QSqlQuery q;
    q.prepare(
        "SELECT MAX(ph.played_at) FROM play_history ph "
        "JOIN tracks t ON t.id = ph.track_id "
        "WHERE t.album_id = :albumId"
    );
    q.bindValue(":albumId", albumId);
    q.exec();
    if (q.next() && !q.value(0).isNull())
        return QDateTime::fromString(q.value(0).toString(), Qt::ISODate);
    return QDateTime();
}

QMap<QString, QList<QDateTime>> Database::playHistoryByFilePath() {
    QMap<QString, QList<QDateTime>> result;
    QSqlQuery q(
        "SELECT t.file_path, ph.played_at FROM play_history ph "
        "JOIN tracks t ON t.id = ph.track_id"
    );
    while (q.next()) {
        QString path = q.value(0).toString();
        QDateTime dt = QDateTime::fromString(q.value(1).toString(), Qt::ISODate);
        result[path] << dt;
    }
    return result;
}

QMap<int, QList<QPair<int, QString>>> Database::playlistTracksByFilePath() {
    QMap<int, QList<QPair<int, QString>>> result;
    QSqlQuery q(
        "SELECT pt.playlist_id, pt.position, t.file_path FROM playlist_tracks pt "
        "JOIN tracks t ON t.id = pt.track_id "
        "ORDER BY pt.playlist_id, pt.position"
    );

    while (q.next()) {
        int playlistId = q.value(0).toInt();
        int position = q.value(1).toInt();
        QString filePath = q.value(2).toString();
        result[playlistId].append(qMakePair(position, filePath));
    }
    return result;
}

void Database::restorePlaylistTracks(const QMap<int, QList<QPair<int, QString>>> &playlistMap) {
    for (auto it = playlistMap.begin(); it != playlistMap.end(); ++it) {
        int playlistId = it.key();

        for (const auto &entry : it.value()) {
            int position = entry.first;
            const QString &filePath = entry.second;

            QSqlQuery findTrack;
            findTrack.prepare("SELECT id FROM tracks WHERE file_path = :path LIMIT 1");
            findTrack.bindValue(":path", filePath);
            findTrack.exec();
            if (!findTrack.next()) continue;

            int trackId = findTrack.value(0).toInt();
            QSqlQuery insert;
            insert.prepare(
                "INSERT INTO playlist_tracks (playlist_id, track_id, position) "
                "VALUES (:playlistId, :trackId, :position)"
            );
            insert.bindValue(":playlistId", playlistId);
            insert.bindValue(":trackId", trackId);
            insert.bindValue(":position", position);
            insert.exec();
        }
    }
}

// --- artists ---

int Database::insertArtist(const QString &name) {
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO artists (name) VALUES (:name)");
    q.bindValue(":name", name);
    q.exec();
    return artistId(name);
}

int Database::artistId(const QString &name) {
    QSqlQuery q;
    q.prepare("SELECT id FROM artists WHERE name = :name");
    q.bindValue(":name", name);
    q.exec();
    if (q.next()) return q.value(0).toInt();
    return -1;
}

void Database::linkArtistToAlbum(int albumId, int artistId) {
    QSqlQuery q;
    q.prepare(
        "INSERT OR IGNORE INTO album_artists (album_id, artist_id)"
        " VALUES (:albumId, :artistId)"
    );
    q.bindValue(":albumId",  albumId);
    q.bindValue(":artistId", artistId);
    q.exec();
}

// --- tracks ---

bool Database::trackExists(const QString &filePath) {
    QSqlQuery q;
    q.prepare("SELECT id FROM tracks WHERE file_path = :path");
    q.bindValue(":path", filePath);
    q.exec();
    return q.next();
}

int Database::insertTrack(int albumId, const QString &title, int trackNumber,
                           int durationMs, const QString &filePath,
                           const QString &format, int artistId) {
    QSqlQuery q;
    q.prepare(
        "INSERT INTO tracks (album_id, title, track_number, duration_ms,"
        "                    file_path, format, artist_id)"
        " VALUES (:albumId, :title, :trackNum, :duration, :path, :format, :artistId)"
    );
    q.bindValue(":albumId",  albumId);
    q.bindValue(":title",    title);
    q.bindValue(":trackNum", trackNumber);
    q.bindValue(":duration", durationMs);
    q.bindValue(":path",     filePath);
    q.bindValue(":format",   format);
    q.bindValue(":artistId", artistId == -1 ? QVariant() : artistId);
    q.exec();
    return q.lastInsertId().toInt();
}

QVector<Track> Database::tracksForAlbum(int albumId) {
    QVector<Track> tracks;
    QSqlQuery q;
    q.prepare(
        "SELECT t.id, t.title, t.track_number, t.duration_ms,"
        "       t.file_path, t.format"
        " FROM tracks t"
        " WHERE t.album_id = :albumId"
        " ORDER BY t.track_number"
    );
    q.bindValue(":albumId", albumId);
    q.exec();
    while (q.next()) {
        Track track;
        track.id          = q.value(0).toInt();
        track.albumId     = albumId;
        track.title       = q.value(1).toString();
        track.trackNumber = q.value(2).toInt();
        track.durationMs  = q.value(3).toInt();
        track.filePath    = q.value(4).toString();
        track.format      = q.value(5).toString();
        tracks.append(track);
    }
    return tracks;
}

// --- drives ---

int Database::insertDrive(const QString &devicePath, const QString &label) {
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO drives (device_path, label) VALUES (:path, :label)");
    q.bindValue(":path",  devicePath);
    q.bindValue(":label", label);
    q.exec();

    QSqlQuery q2;
    q2.prepare("SELECT id FROM drives WHERE device_path = :path");
    q2.bindValue(":path", devicePath);
    q2.exec();
    if (q2.next()) return q2.value(0).toInt();
    return -1;
}

void Database::setCdPresent(int albumId, bool present) {
    QSqlQuery q;
    q.prepare("UPDATE albums SET cd_present = :present WHERE id = :id");
    q.bindValue(":present", present ? 1 : 0);
    q.bindValue(":id",      albumId);
    q.exec();
}

// --- tags ---

QStringList Database::allTags() {
    QStringList tags;
    QSqlQuery q;
    q.exec("SELECT name FROM tags ORDER BY name");
    while (q.next())
        tags << q.value(0).toString();
    return tags;
}

QStringList Database::tagsForAlbum(int albumId) {
    QStringList tags;
    QSqlQuery q;
    q.prepare(
        "SELECT t.name FROM tags t"
        " JOIN album_tags albtags ON albtags.tag_id = t.id"
        " WHERE albtags.album_id = :albumId"
    );
    q.bindValue(":albumId", albumId);
    q.exec();
    while (q.next())
        tags << q.value(0).toString();
    return tags;
}

void Database::setTagsForAlbum(int albumId, const QStringList &tags) {
    QSqlQuery del;
    del.prepare("DELETE FROM album_tags WHERE album_id = :albumId");
    del.bindValue(":albumId", albumId);
    del.exec();

    for (const QString &tag : tags) {
        createTag(tag);

        QSqlQuery q;
        q.prepare(
            "INSERT OR IGNORE INTO album_tags (album_id, tag_id)"
            " SELECT :albumId, id FROM tags WHERE name = :name"
        );
        q.bindValue(":albumId", albumId);
        q.bindValue(":name",    tag);
        q.exec();
    }
}

void Database::createTag(const QString &tag) {
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO tags (name) VALUES (:name)");
    q.bindValue(":name", tag.toLower().trimmed());
    q.exec();
}

QMap<QString, QStringList> Database::albumTagsByTitleArtist() {
    QMap<QString, QStringList> result;
    QSqlQuery q(
        "SELECT a.title, ar.name, t.name FROM albums a "
        "JOIN album_artists aa ON aa.album_id = a.id "
        "JOIN artists ar ON ar.id = aa.artist_id "
        "JOIN album_tags albt ON albt.album_id = a.id "
        "JOIN tags t ON t.id = albt.tag_id"
    );
    while (q.next()) {
        QString key = q.value(0).toString() + "||" + q.value(1).toString();
        result[key] << q.value(2).toString();
    }
    return result;
}

// --- library clearing ---
void Database::clearLibrary() {
    QSqlQuery q;
    q.exec("DELETE FROM playlist_tracks");
    q.exec("DELETE FROM play_history");
    q.exec("DELETE FROM lyrics");
    q.exec("DELETE FROM album_tags");
    q.exec("DELETE FROM tracks");
    q.exec("DELETE FROM album_artists");
    q.exec("DELETE FROM albums");
    q.exec("DELETE FROM artists");
    q.exec("DELETE FROM drives");
}

void Database::restoreAlbumTags(const QMap<QString, QStringList> &tagMap) { // restores album tags after library clear
    for (auto it = tagMap.begin(); it != tagMap.end(); ++it) {
        QStringList parts = it.key().split("||");
        if (parts.size() != 2) continue;
        int albumId = albumIdByTitleAndArtist(parts[0], parts[1]);
        if (albumId != -1)
            setTagsForAlbum(albumId, it.value());
    }
}

void Database::restorePlayHistory(const QMap<QString, QList<QDateTime>> &historyMap) {
    for (auto it = historyMap.begin(); it != historyMap.end(); ++it) {
        QSqlQuery q;
        q.prepare("SELECT id FROM tracks WHERE file_path = :path");
        q.bindValue(":path", it.key());
        q.exec();
        if (!q.next()) continue;
        int trackId = q.value(0).toInt();
        for (const QDateTime &dt : it.value()) {
            QSqlQuery insert;
            insert.prepare("INSERT INTO play_history (track_id, played_at) VALUES (:id, :dt)");
            insert.bindValue(":id", trackId);
            insert.bindValue(":dt", dt.toString(Qt::ISODate));
            insert.exec();
        }
    }
}