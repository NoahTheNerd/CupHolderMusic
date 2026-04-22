//
// Created by noahg on 3/8/26.
//

#include "mainwindow.h"
#include <QVector>
#include <QRandomGenerator>
#include "settings.h"
#include "database.h"
#include <QDir>
#include "libraryscanner.h"
#include "audiobackends/libvlcbackend.h"
#include "audiobackends/queuing.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	setWindowTitle("CupHolderMusic");
	resize(1200, 800);
	setStyleSheet("background-color: #13131c;");

	m_stack = new QStackedWidget(this);
	setCentralWidget(m_stack);

	m_libraryView = new LibraryView(this);
	m_stack->addWidget(m_libraryView);
	m_stack->setCurrentIndex(0);

	m_albumView = new AlbumView(this);
	m_stack->addWidget(m_albumView);
	m_stack->setCurrentIndex(0);

	m_backend = new VlcBackend(this);
	m_queue   = new PlaybackQueue(m_backend, this);
	connect(m_backend, &AudioBackend::trackEnded, m_queue, &PlaybackQueue::onTrackEnded);

	connect(m_libraryView, &LibraryView::playAlbumRequested, this, [this](const QVector<Track> &tracks) {
		m_queue->loadAlbum(tracks);
		m_queue->play();
	});

	connect(m_queue, &PlaybackQueue::playbackStateChanged, this, [this](bool playing) {
		int albumId = playing ? m_queue->currentTrack().albumId : -1;
		m_libraryView->setCurrentlyPlayingAlbum(albumId);
	});

	connect(m_queue, &PlaybackQueue::playbackPaused, this, [this](bool paused) {
	m_libraryView->setCurrentlyPlayingAlbum(m_queue->currentTrack().albumId, paused);
});

	connect(m_libraryView, &LibraryView::pauseRequested, this, [this]() {
	m_queue->pause();
});
	connect(m_libraryView, &LibraryView::stopRequested, this, [this]() {
		m_queue->stop();
	});

	connect(m_libraryView, &LibraryView::rescanRequested, this, [this]() {
		LibraryScanner::instance().rescan(Settings::instance().watchedFolder());
		m_libraryView->setAlbums(Database::instance().allAlbums());
	});

	connect(m_libraryView, &LibraryView::albumViewRequested, this, [this]() {
		openAlbumView();
	});

	scanAndLoad();
}

void MainWindow::openAlbumView() {
	m_stack->setCurrentIndex(1);
}

void MainWindow::openLibraryView() {
	m_stack->setCurrentIndex(0);
}

void MainWindow::scanAndLoad() {
	QString folder = Settings::instance().watchedFolder();
	if (folder.isEmpty()) {
		folder = QDir::homePath() + "/Music";
		Settings::instance().setWatchedFolder(folder);
	}

	LibraryScanner::instance().scan(folder);
	QVector<Album> albums;
	albums = Database::instance().allAlbums();

	m_libraryView->setAlbums(albums);
}