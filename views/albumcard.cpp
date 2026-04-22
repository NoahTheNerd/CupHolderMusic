//
// Created by noahg on 3/8/26.
//

#include "albumcard.h"
#include "../settings.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include "../audiobackends/queuing.h"

AlbumCard::AlbumCard(const Album &album, QWidget *parent)
	: QWidget(parent), m_album(album)
{
	setFixedWidth(Settings::instance().cardSize());

	auto *layout = new QVBoxLayout(this);

	auto *cover = new QLabel(this);
	cover->setFixedSize(Settings::instance().cardSize(), Settings::instance().cardSize());

	if (!album.coverArt.isNull()) {
		cover->setPixmap(album.coverArt.scaled(
			Settings::instance().cardSize(),
			Settings::instance().cardSize(),
			Qt::KeepAspectRatioByExpanding,
			Qt::SmoothTransformation
		));
	} else {
		cover->setStyleSheet("background-color: #2a2a3a;");
	}

	auto *title  = new QLabel(album.title, this);
	auto *artist = new QLabel(album.artist, this);
	auto *formatsLayout = new QHBoxLayout();
	formatsLayout->setSpacing(4);
	formatsLayout->setContentsMargins(0, 0, 0, 0);

	QStringList formats = album.availableFormats();
	int shownFormats = 0;
	for (const QString &format : formats) {
		if (shownFormats >= 2) {
			auto *ellipsis = new QLabel("...", this);
			ellipsis->setStyleSheet("color: #555566; font-size: 10px;");
			formatsLayout->addWidget(ellipsis);
			break;
		}
		QLabel *formatLabel;
		formatLabel = new QLabel(format, this);

		bool isGreyedCd = (format == "CD" && !album.cdPresent);
		formatLabel->setStyleSheet(isGreyedCd
			? "color: #555566; font-size: 10px;"
			: "color: #a0a0b0; font-size: 10px;"
		);
		formatsLayout->addWidget(formatLabel);

		// add comma separator if not last
		if (format != formats.last()) {
			QLabel *comma;
			comma = new QLabel(",", this);
			comma->setStyleSheet("color: #555566; font-size: 10px;");
			formatsLayout->addWidget(comma);
		}

		shownFormats++;
	}
	formatsLayout->addStretch();

	auto *formatsWidget = new QWidget(this);
	formatsWidget->setLayout(formatsLayout);

	layout->addWidget(cover);
	layout->addWidget(title);
	layout->addWidget(artist);
	layout->addWidget(formatsWidget);
}

void AlbumCard::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton)
		emit clicked(m_album.id);
}

void AlbumCard::contextMenuEvent(QContextMenuEvent *event) {
	QMenu menu(this);

	menu.setStyleSheet("QMenu::item:selected { background-color: palette(highlight); color: palette(highlighted-text); }");

	if (m_playState != PlayState::Paused && m_playState != PlayState::Playing) {
		QAction *playAction;
		playAction = menu.addAction(QIcon::fromTheme("media-playback-start"), "Play");
		connect(playAction, &QAction::triggered, [this]() { emit playRequest(m_album.id); });
	}

	if (m_playState == PlayState::Playing) {
		QAction *pauseAction;
		QAction *stopAction;

		pauseAction = menu.addAction(QIcon::fromTheme("media-playback-pause"), "Pause");
		connect(pauseAction, &QAction::triggered, [this]() { emit pauseRequest(); });
		stopAction = menu.addAction(QIcon::fromTheme("media-playback-stop"), "Stop");
		connect(stopAction, &QAction::triggered, [this]() { emit stopRequest(); });
	} else if (m_playState == PlayState::Paused) {
		QAction *resumeAction;
		QAction *stopAction;

		resumeAction = menu.addAction(QIcon::fromTheme("media-playback-start"), "Resume");
		connect(resumeAction, &QAction::triggered, [this]() { emit pauseRequest(); });
		stopAction = menu.addAction(QIcon::fromTheme("media-playback-stop"), "Stop");
		connect(stopAction, &QAction::triggered, [this]() { emit stopRequest(); });
	}

	menu.addSeparator();

	QAction *tagsAction;
	tagsAction = menu.addAction(QIcon::fromTheme("tag"), "Tags...");
	connect(tagsAction, &QAction::triggered, [this]() { emit tagsRequest(m_album.id); });

	QAction *infoAction;
	infoAction = menu.addAction(QIcon::fromTheme("dialog-information"), "Info...");;
	connect(infoAction, &QAction::triggered, [this]() { emit infoRequest(m_album.id); });

	menu.exec(event->globalPos());
}

void AlbumCard::setPlayState(PlayState state) {
	m_playState = state;
}