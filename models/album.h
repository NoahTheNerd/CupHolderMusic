//
// Created by noahg on 3/8/26.
//

#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QPixmap>
#include <QVector>
#include "track.h"

enum class AudioSource {
	CD, MP3, FLAC, WAV, OGG, Unknown
};

struct Album {
	int         id          = -1;
	QString     title;
	QString     artist;
	int         trackCount  = 0;
	int         releaseYear = 0;
	bool        cdPresent   = false;
	QString     driveLabel;
	QDateTime   firstPlayed;
	QDateTime   lastPlayed;
	QDateTime   dateAdded;
	QStringList tags;
	QPixmap     coverArt;
	QVector<Track> tracks;

	QStringList availableFormats() const {
		QStringList formats;
		for (const Track &t : tracks)
			if (!formats.contains(t.format))
				formats.append(t.format);
		return formats;
	}

	bool hasCd() const {
		for (const Track &t : tracks)
			if (t.format == "CD") return true;
		return false;
	}

	QString formatsString() const {
		QStringList formats = availableFormats();
		if (formats.isEmpty()) return "Unknown";
		if (formats.size() <= 2) return formats.join(", ");
		return formats.mid(0, 2).join(", ") + "...";
	}

	bool isGrayedOut() const {
		if (!hasCd()) return false;           // no CD tracks, never gray
		if (availableFormats().size() > 1) return false; // has other formats, never gray
		return !cdPresent;                    // CD only, gray if not present
	}
};