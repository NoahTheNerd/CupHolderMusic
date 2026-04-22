//
// Created by noahg on 3/8/26.
//

#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include "../models/album.h"
#include "../models/track.h"

enum class SortMode {
	ArtistAZ, ArtistZA,
	AlbumAZ,  AlbumZA,
	ReleasedNewest, ReleasedOldest,
	FirstPlayedNewest, FirstPlayedOldest,
	LastPlayedNewest, LastPlayedOldest,
	DateAddedNewest, DateAddedOldest,
	PlayCountMost, PlayCountLeast,
	Random
};

class LibraryView : public QWidget {
	Q_OBJECT

public:
	explicit LibraryView(QWidget *parent = nullptr);
	void setAlbums(const QVector<Album> &albums);
	void setCurrentlyPlayingAlbum(int albumId, bool paused = false);
	signals:
		void playAlbumRequested(const QVector<Track> &tracks);
		void pauseRequested();
		void stopRequested();
		void rescanRequested();
		void albumViewRequested(int id);

private slots:
	void onSortChanged(int index);
	void onSearchChanged(const QString &text);
	void onTagFilterChanged(int index);

private:
	int m_gridCols;
	int m_currentlyPlayingAlbumId = -1;
	bool m_currentlyPlayingPaused = false;

	QVector<Album> m_allAlbums;
	QVector<Album> m_filteredAlbums;

	QScrollArea    *m_scrollArea{};
	QWidget        *m_gridContainer{};
	QGridLayout    *m_gridLayout = nullptr;
	QComboBox      *m_sortCombo{};
	QPushButton	   *m_rescanButton{};
	QComboBox      *m_tagCombo{};
	QLineEdit      *m_searchBox{};
	QLabel         *m_albumCountLabel{};

	SortMode m_currentSort = SortMode::AlbumAZ;
	QString  m_searchText;
	QString  m_tagFilter;

	void uiSetupper();
	void setupToolbar(QWidget *toolbar);
	void resizeEvent(QResizeEvent *event) override;
	void rebuildGrid();
	void applyFiltersAndSort();
	QVector<Album> groupAlbums(const QVector<Album> &albums);
	void populateTagCombo();
};