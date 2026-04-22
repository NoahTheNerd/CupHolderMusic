//
// Created by noahg on 3/8/26.
//

#pragma once
#include <QWidget>
#include <QLabel>
#include "../models/album.h"

class AlbumCard : public QWidget {
	Q_OBJECT
public:
	explicit AlbumCard(const Album &album, QWidget *parent = nullptr);
	enum class PlayState { Stopped, Playing, Paused };
	void setPlayState(PlayState state);
signals:
	void clicked(int albumId);
	void playRequest(int albumId);
	void pauseRequest();
	void stopRequest();

	void infoRequest(int albumId);
	void tagsRequest(int albumId);
protected:
	void mousePressEvent(QMouseEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;

private:
	Album m_album;
	PlayState m_playState = PlayState::Stopped;
};