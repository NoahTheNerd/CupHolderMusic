//
// Created by noahg on 3/19/26.
//

#pragma once
#include "audiobackend.h"
#include <vlc/vlc.h>
#include <QString>
#include <QDebug>

class VlcBackend : public AudioBackend {
	Q_OBJECT
public:
	explicit VlcBackend(QObject *parent = nullptr);
	~VlcBackend() override;

	void play(const QString &uri) override;
	void pause() override;
	void stop() override;
	void seek(qint64 ms) override;
	qint64 position() override;
	qint64 duration() override;
	bool isPlaying() override;

signals:
	void trackEnded();

private:
	libvlc_instance_t     *m_vlc    = nullptr;
	libvlc_media_player_t *m_player = nullptr;
	libvlc_media_t        *m_media  = nullptr;

	static void onTrackEndedCallback(const libvlc_event_t *event, void *userData);
};