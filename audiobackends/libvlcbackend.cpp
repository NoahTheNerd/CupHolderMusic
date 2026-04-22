//
// Created by noahg on 3/19/26.
//

#include "libvlcbackend.h"

VlcBackend::VlcBackend(QObject *parent) : AudioBackend(parent) {
    m_vlc = libvlc_new(0, nullptr);
    if (!m_vlc) {
        qDebug() << "Failed to initialize libvlc";
        return;
    }
    m_player = libvlc_media_player_new(m_vlc);
    if (!m_player) {
        qDebug() << "Failed to create media player";
        return;
    }
    libvlc_event_manager_t *em = libvlc_media_player_event_manager(m_player);
    libvlc_event_attach(em, libvlc_MediaPlayerEndReached, onTrackEndedCallback, this);
}

VlcBackend::~VlcBackend() {
    if (m_media)  libvlc_media_release(m_media);
    if (m_player) libvlc_media_player_release(m_player);
    if (m_vlc)    libvlc_release(m_vlc);
}

void VlcBackend::play(const QString &uri) {
    qDebug() << "Playing song:" << uri;

    // release previous media if any
    if (m_media) {
        libvlc_media_release(m_media);
        m_media = nullptr;
    }

    // create new media from file path
    m_media = libvlc_media_new_path(m_vlc, uri.toUtf8().constData());
    if (!m_media) {
        qDebug() << "Failed to create media for:" << uri;
        return;
    }
    libvlc_media_player_set_media(m_player, m_media);
    libvlc_media_player_play(m_player);
}

void VlcBackend::pause() {
    // pause
    libvlc_media_player_pause(m_player);
}

void VlcBackend::stop() {
    // stop
    libvlc_media_player_stop(m_player);

    // release media
    if (m_media) {
        libvlc_media_release(m_media);
        m_media = nullptr;
    }
}

void VlcBackend::seek(qint64 ms) {
    // seek
}

qint64 VlcBackend::position() {
    // return position
    return 0;
}

qint64 VlcBackend::duration() {
    // return duration
    return 0;
}

bool VlcBackend::isPlaying() {
    // return playing state
    return libvlc_media_player_is_playing(m_player);
}

void VlcBackend::onTrackEndedCallback(const libvlc_event_t *, void *userData) {
    VlcBackend *self = static_cast<VlcBackend*>(userData);
    QMetaObject::invokeMethod(self, "trackEnded", Qt::QueuedConnection);
}