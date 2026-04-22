//
// Created by noahg on 3/8/26.
//

#include "libraryview.h"
#include <algorithm>
#include <QContextMenuEvent>
#include <QPushButton>
#include <QRandomGenerator>
#include <QVBoxLayout>
#include "albumcard.h"
#include "../settings.h"
#include "../popups/albuminfo.h"
#include "../popups/tags.h"
#include "../database.h"

LibraryView::LibraryView(QWidget *parent) : QWidget(parent) {
	uiSetupper();
}

void LibraryView::uiSetupper() {
	setStyleSheet("background-color: #0f0f0f;");

	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// toolbar
	QWidget *toolbar;
	toolbar = new QWidget(this);
	toolbar->setFixedHeight(60);
	toolbar->setStyleSheet("background-color: #1a1a26; border-bottom: 1px solid #2a2a3a;");
	setupToolbar(toolbar);
	mainLayout->addWidget(toolbar);

	// scroll are-uhhhhh i frog got 🐸 <- frog, cuz i frog got, get it?? hahahahahahaha
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setFrameShape(QFrame::NoFrame);
	m_scrollArea->setStyleSheet(
		"QScrollArea { background-color: #321160; }"
		"QScrollBar:vertical { background: #1a1a26; width: 8px; border-radius: 4px; }"
		"QScrollBar::handle:vertical { background: #3a3a4a; border-radius: 4px; min-height: 30px; }"
		"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
	);

	m_gridContainer = new QWidget();
	m_gridContainer->setStyleSheet("background-color: #13131c;");
	m_gridLayout = new QGridLayout(m_gridContainer);
	m_gridLayout->setSpacing(Settings::instance().cardSpacing());
	m_gridLayout->setContentsMargins(24, 24, 24, 24);

	m_scrollArea->setWidget(m_gridContainer);
	mainLayout->addWidget(m_scrollArea);

	int savedIndex = m_sortCombo->findData((int)Settings::instance().sortMode());
	if (savedIndex >= 0)
		m_sortCombo->setCurrentIndex(savedIndex);

}

void LibraryView::setupToolbar(QWidget *toolbar) {
	auto *layout = new QHBoxLayout(toolbar);
	layout->setContentsMargins(16, 0, 16, 0);
	layout->setSpacing(12);

	// Album count
	m_albumCountLabel = new QLabel("0 albums", toolbar);
	m_albumCountLabel->setStyleSheet("color: #6a6a8a; font-size: 12px;");

	// Search
	m_searchBox = new QLineEdit(toolbar);
	m_searchBox->setPlaceholderText("Search albums, artists...");
	m_searchBox->setFixedWidth(220);
	m_searchBox->setStyleSheet(
	 "QLineEdit {"
		"  background: #252535; color: #e0e0e0; border: 1px solid #3a3a4a;"
		"  border-radius: 6px; padding: 4px 10px; font-size: 12px;"
		"}"
		"QLineEdit:focus { border-color: #5a5aaa; }"
	);

	// Tag filter
	auto *filterLabel = new QLabel("Filter:", toolbar);
	filterLabel->setStyleSheet("color: #6a6a8a; font-size: 12px;");

	m_tagCombo = new QComboBox(toolbar);
	m_tagCombo->addItem("All Tags");
	m_tagCombo->setFixedWidth(130);
	m_tagCombo->setMaxVisibleItems(10);
	m_tagCombo->setStyleSheet(
		"QComboBox {"
		"  background: #252535; color: #e0e0e0; border: 1px solid #3a3a4a;"
		"  border-radius: 6px; padding: 4px 8px; font-size: 12px;"
		"}"
		"QComboBox::drop-down { border: none; }"
		"QComboBox QAbstractItemView { background: #252535; color: #e0e0e0; border: 1px solid #3a3a4a; }"
	);

	// Sort
	auto *sortLabel = new QLabel("Sort:", toolbar);
	sortLabel->setStyleSheet("color: #6a6a8a; font-size: 12px;");

	m_sortCombo = new QComboBox(toolbar);
	m_sortCombo->addItem("Album A→Z",          (int)SortMode::AlbumAZ);
	m_sortCombo->addItem("Album Z→A",          (int)SortMode::AlbumZA);
	m_sortCombo->addItem("Artist A→Z",         (int)SortMode::ArtistAZ);
	m_sortCombo->addItem("Artist Z→A",         (int)SortMode::ArtistZA);
	m_sortCombo->addItem("Released: Newest",   (int)SortMode::ReleasedNewest);
	m_sortCombo->addItem("Released: Oldest",   (int)SortMode::ReleasedOldest);
	m_sortCombo->addItem("First Played: New",  (int)SortMode::FirstPlayedNewest);
	m_sortCombo->addItem("First Played: Old",  (int)SortMode::FirstPlayedOldest);
	m_sortCombo->addItem("Last Played: New",   (int)SortMode::LastPlayedNewest);
	m_sortCombo->addItem("Last Played: Old",   (int)SortMode::LastPlayedOldest);
	m_sortCombo->addItem("Date Added: New",    (int)SortMode::DateAddedNewest);
	m_sortCombo->addItem("Date Added: Old",    (int)SortMode::DateAddedOldest);
	m_sortCombo->addItem("Shuffle",            (int)SortMode::Random);
	m_sortCombo->setFixedWidth(170);
	m_sortCombo->setStyleSheet(m_tagCombo->styleSheet());

	m_rescanButton = new QPushButton("Rescan",toolbar);

	layout->addWidget(m_albumCountLabel);
	layout->addStretch();
	layout->addWidget(m_searchBox);
	layout->addWidget(filterLabel);
	layout->addWidget(m_tagCombo);
	layout->addWidget(sortLabel);
	layout->addWidget(m_sortCombo);
	layout->addWidget(m_rescanButton);

	connect(m_sortCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &LibraryView::onSortChanged);
	connect(m_searchBox,  &QLineEdit::textChanged,
			this, &LibraryView::onSearchChanged);
	connect(m_tagCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &LibraryView::onTagFilterChanged);
	connect(m_rescanButton, &QPushButton::clicked, this, [this]() {
		emit rescanRequested();
});
}

void LibraryView::setAlbums(const QVector<Album> &albums) {
	m_allAlbums = Database::instance().allAlbums();
	populateTagCombo();
	applyFiltersAndSort();
}

void LibraryView::onSortChanged(int index) {
	m_currentSort = static_cast<SortMode>(m_sortCombo->itemData(index).toInt());
	Settings::instance().setSortMode(m_currentSort);
	applyFiltersAndSort();
}

void LibraryView::onSearchChanged(const QString &text) {
	m_searchText = text.toLower();
	applyFiltersAndSort();
}

void LibraryView::onTagFilterChanged(int index) {
	m_tagFilter = index == 0 ? "" : m_tagCombo->currentText();
	applyFiltersAndSort();
}

void LibraryView::applyFiltersAndSort() {
	m_filteredAlbums = m_allAlbums;

	// Search filter
	if (!m_searchText.isEmpty()) {
		m_filteredAlbums.erase(
			std::remove_if(m_filteredAlbums.begin(), m_filteredAlbums.end(),
				[&](const Album &a) {
					return !a.title.toLower().contains(m_searchText) &&
						   !a.artist.toLower().contains(m_searchText);
				}),
			m_filteredAlbums.end()
		);
	}

	// Tag filter
	if (!m_tagFilter.isEmpty()) {
		m_filteredAlbums.erase(
			std::remove_if(m_filteredAlbums.begin(), m_filteredAlbums.end(),
				[&](const Album &a) { return !a.tags.contains(m_tagFilter); }),
			m_filteredAlbums.end()
		);
	}

	// Sort
	auto &v = m_filteredAlbums;
	switch (m_currentSort) {
		case SortMode::AlbumAZ:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.title < b.title; });
			break;
		case SortMode::AlbumZA:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.title > b.title; });
			break;
		case SortMode::ArtistAZ:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.artist < b.artist; });
			break;
		case SortMode::ArtistZA:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.artist > b.artist; });
			break;
		case SortMode::ReleasedNewest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.releaseYear > b.releaseYear; });
			break;
		case SortMode::ReleasedOldest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.releaseYear < b.releaseYear; });
			break;
		case SortMode::FirstPlayedNewest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.firstPlayed > b.firstPlayed; });
			break;
		case SortMode::FirstPlayedOldest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.firstPlayed < b.firstPlayed; });
			break;
		case SortMode::LastPlayedNewest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.lastPlayed > b.lastPlayed; });
			break;
		case SortMode::LastPlayedOldest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.lastPlayed < b.lastPlayed; });
			break;
		case SortMode::DateAddedNewest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.dateAdded > b.dateAdded; });
			break;
		case SortMode::DateAddedOldest:
			std::sort(v.begin(), v.end(), [](const Album &a, const Album &b){ return a.dateAdded < b.dateAdded; });
			break;
		case SortMode::Random:
			std::shuffle(v.begin(), v.end(), std::default_random_engine(
				QRandomGenerator::global()->generate()));
			break;
		default: break;
	}

	// Artist sort = group compilations under each artist
	if (m_currentSort == SortMode::ArtistAZ || m_currentSort == SortMode::ArtistZA) {
		m_filteredAlbums = groupAlbums(m_filteredAlbums);
	}

	rebuildGrid();
}

QVector<Album> LibraryView::groupAlbums(const QVector<Album> &albums) {
	// For artist sort: expand multi-artist albums into one entry per artist
	QVector<Album> expanded;
	for (const Album &album : albums) {
		// In a real implementation, album would have a QStringList of artists
		// For now, treat comma-separated artists as multiple
		QStringList artists = album.artist.split(", ");
		if (artists.size() > 1) {
			for (const QString &artist : artists) {
				Album copy = album;
				copy.artist = artist;
				expanded.append(copy);
			}
		} else {
			expanded.append(album);
		}
	}
	return expanded;
}

void LibraryView::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);
	int availableWidth = m_scrollArea->viewport()->width() - 48; // account for margins
	int cols = qMax(1, availableWidth / (Settings::instance().cardSize() + Settings::instance().cardSpacing()));
	if (cols != m_gridCols) {
		m_gridCols = cols;
		rebuildGrid();
	}
}

void LibraryView::rebuildGrid() {
	if (!m_gridLayout) return;
	if (m_gridCols <= 0) return;
	// Clear old cards
	QLayoutItem *item;
	while ((item = m_gridLayout->takeAt(0)) != nullptr) {
		if (item->widget()) item->widget()->deleteLater();
		delete item;
	}

	int col = 0, row = 0;
	for (const Album &album : m_filteredAlbums) {
		auto *card = new AlbumCard(album, m_gridContainer);
		AlbumCard::PlayState state = AlbumCard::PlayState::Stopped;
		if (album.id == m_currentlyPlayingAlbumId)
			state = m_currentlyPlayingPaused
				? AlbumCard::PlayState::Paused
				: AlbumCard::PlayState::Playing;
		card->setPlayState(state);
		connect(card, &AlbumCard::clicked, this, [this](int id){
			qDebug() << "Album clicked:" << id;
			emit albumViewRequested(id);
		});

		connect(card, &AlbumCard::playRequest, this, [this](int id) {
			auto foundAlbum = std::find_if(m_allAlbums.begin(), m_allAlbums.end(),[id](const Album &a) { return a.id == id; });
			if (foundAlbum != m_allAlbums.end())
				emit playAlbumRequested(foundAlbum->tracks);
		});

		connect(card, &AlbumCard::pauseRequest, this, [this]() {
			emit pauseRequested();
		});
		connect(card, &AlbumCard::stopRequest, this, [this]() {
			emit stopRequested();
		});

		connect(card, &AlbumCard::infoRequest, this, [this](int id) {
			Album albumData = Database::instance().albumById(id);
			AlbumInfo dialog(albumData, this);
			dialog.exec();
		});

		connect(card, &AlbumCard::tagsRequest, this, [this](int id) {
			QList<Album>::iterator foundAlbum;
			foundAlbum = std::find_if(m_allAlbums.begin(), m_allAlbums.end(),
			[id](const Album &a) { return a.id == id; });
			if (foundAlbum != m_allAlbums.end()) {
				Tags dialog(*foundAlbum, this);
				connect(&dialog, &Tags::tagsUpdated, this, [this](int albumId, const QStringList &newTags) {
					QList<Album>::iterator updatedAlbum;
					updatedAlbum = std::find_if(m_allAlbums.begin(), m_allAlbums.end(),[albumId](const Album &a) { return a.id == albumId; });
					if (updatedAlbum != m_allAlbums.end()) {
						updatedAlbum->tags = newTags;
						populateTagCombo();
					}
		});
		dialog.exec();
	}
});

		m_gridLayout->addWidget(card, row, col);
		if (++col >= m_gridCols) {
			col = 0; ++row;
		}
		m_gridLayout->setRowStretch(m_gridLayout->rowCount(), 1);
	}

	// Fill remaining row with spacers so cards left-align
	if (col > 0) {
		m_gridLayout->setColumnStretch(m_gridLayout->columnCount(), 1);
	}

	m_albumCountLabel->setText(QString("%1 album%2")
		.arg(m_filteredAlbums.size())
		.arg(m_filteredAlbums.size() == 1 ? "" : "s"));
}

void LibraryView::populateTagCombo() {
	m_tagCombo->clear();
	m_tagCombo->addItem("All Tags");
	QStringList allTags;
	for (const Album &a : m_allAlbums)
		for (const QString &tag : a.tags)
			if (!allTags.contains(tag)) allTags.append(tag);
	allTags.sort();
	for (const QString &tag : allTags)
		m_tagCombo->addItem(tag);
}

void LibraryView::setCurrentlyPlayingAlbum(int albumId, bool paused) {
	m_currentlyPlayingAlbumId = albumId;
	m_currentlyPlayingPaused  = paused;
	rebuildGrid();
}