//
// Created by noahg on 3/11/26.
//

#include "libraryscanner.h"
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <taglib/fileref.h>
#include "database.h"
#include <taglib/flacfile.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include "settings.h"

LibraryScanner &LibraryScanner::instance() {
	static LibraryScanner inst;
	return inst;
}

QStringList LibraryScanner::findAudioFiles(const QString &folderPath) {
	QStringList files;
	QStringList extensions = {"mp3", "flac", "wav", "ogg"};

	QDirIterator drfiler(folderPath, QDir::Files, QDirIterator::Subdirectories);
	while (drfiler.hasNext()) {
		QString f = drfiler.next();
		if (extensions.contains(QFileInfo(f).suffix().toLower()))
			files << f;
	}
	return files;
}

void LibraryScanner::processFile(const QString &filePath) {
	if (Database::instance().trackExists(filePath)) return;
	TagLib::FileRef file(filePath.toUtf8().constData());
	if (file.isNull() || !file.tag()) return;

	// read tags
	QString title  = QString::fromStdWString(file.tag()->title().toWString()).trimmed();
	QString artist = QString::fromStdWString(file.tag()->artist().toWString()).trimmed();
	QString album  = QString::fromStdWString(file.tag()->album().toWString()).trimmed();
	int     year   = file.tag()->year();
	int     trackNum = file.tag()->track();
	int     duration = file.audioProperties()
					   ? file.audioProperties()->lengthInMilliseconds() : 0;

	// fallbacks
	QFileInfo fileInfo(filePath);
	if (title.isEmpty())  title  = fileInfo.baseName();
	if (album.isEmpty())  album  = fileInfo.dir().dirName();
	if (artist.isEmpty()) {
		QDir parentDir = fileInfo.dir();
		parentDir.cdUp();
		artist = parentDir.dirName();
	}

	// get format from extension
	QString format = fileInfo.suffix().toUpper();

	// find or create album in DB
	// check if album already exists by title + artist combo
	int albumId = Database::instance().albumIdByTitleAndArtist(album, artist);
	if (albumId == -1) {
		albumId = Database::instance().insertAlbum(album, year, QDateTime::currentDateTime());
		int artistId = Database::instance().insertArtist(artist);
		Database::instance().linkArtistToAlbum(albumId, artistId);

		// extract and store cover art for new albums only
		QByteArray coverData = extractCoverArt(filePath);
		if (!coverData.isEmpty()) {
			Database::instance().setAlbumCoverArt(albumId, coverData);
		}
	}

	// insert track
	Database::instance().insertTrack(albumId, title, trackNum, duration, filePath, format);

	if (Settings::instance().importGenreTags()) {
		QString genre = QString::fromStdWString(file.tag()->genre().toWString()).trimmed().toLower();
		if (!genre.isEmpty()) {
			Database::instance().createTag(genre);
			Database::instance().setTagsForAlbum(albumId,Database::instance().tagsForAlbum(albumId) << genre);
		}
	}
}

void LibraryScanner::scan(const QString &folderPath) {
	if (folderPath.isEmpty()) return;
	qDebug() << "scanning folder:" << folderPath;
	qDebug() << "folder exists:" << QDir(folderPath).exists();
	QStringList files = findAudioFiles(folderPath);
	qDebug() << "files found:" << files.size();
	for (const QString &file : files)
		processFile(file);
}

QByteArray LibraryScanner::extractCoverArt(const QString &filePath) {
	if (filePath.isEmpty()) return {};

	// FLAC
	if (filePath.endsWith(".flac", Qt::CaseInsensitive)) {
		TagLib::FLAC::File f(filePath.toUtf8().constData());
		if (!f.pictureList().isEmpty()) {
			auto *pic = f.pictureList().front();
			return QByteArray(pic->data().data(), pic->data().size());
		}
	}

	// MP3
	if (filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
		TagLib::MPEG::File f(filePath.toUtf8().constData());
		if (f.ID3v2Tag()) {
			auto frameList = f.ID3v2Tag()->frameListMap()["APIC"];
			if (!frameList.isEmpty()) {
				auto *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
				if (frame)
					return QByteArray(frame->picture().data(), frame->picture().size());
			}
		}
	}

	return {};
}

void LibraryScanner::rescan(const QString &folderPath) {
	auto savedDateAdded = Database::instance().albumDateAddedByTitleArtist();
	auto savedTags    = Database::instance().albumTagsByTitleArtist();
	auto savedHistory = Database::instance().playHistoryByFilePath();
	auto savedPlaylists = Database::instance().playlistTracksByFilePath();
	Database::instance().clearLibrary();
	scan(folderPath);
	Database::instance().restoreAlbumDateAdded(savedDateAdded);
	Database::instance().restoreAlbumTags(savedTags);
	Database::instance().restorePlayHistory(savedHistory);
	Database::instance().restorePlaylistTracks(savedPlaylists);
}