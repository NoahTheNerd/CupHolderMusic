//
// Created by noahg on 3/19/26.
//

#include "queuing.h"
#include "../database.h"

PlaybackQueue::PlaybackQueue(AudioBackend *backend, QObject *parent)
    : QObject(parent), m_backend(backend)
{}

void PlaybackQueue::loadAlbum(const QVector<Track> &tracks) {
    m_tracks       = tracks;
    m_currentIndex = -1;
}

void PlaybackQueue::play() {
    if (m_tracks.isEmpty()) return;
    if (m_currentIndex == -1) m_currentIndex = 0;
    m_paused = false;
    m_backend->play(m_tracks[m_currentIndex].filePath);
    emit trackChanged(m_tracks[m_currentIndex]);
    emit playbackStateChanged(true);
    Database::instance().recordPlay(m_tracks[m_currentIndex].id);
}

void PlaybackQueue::pause() {
    m_backend->pause();
    m_paused = !m_paused;
    emit playbackPaused(m_paused);
}

void PlaybackQueue::stop() {
    m_backend->stop();
    m_paused = false;
    emit playbackStateChanged(false);
}

void PlaybackQueue::next() {
    if (m_currentIndex + 1 >= m_tracks.size()) {
        stop();
        emit queueFinished();
        return;
    }
    m_currentIndex++;
    m_backend->play(m_tracks[m_currentIndex].filePath);
    emit trackChanged(m_tracks[m_currentIndex]);
}

void PlaybackQueue::previous() {
    if (m_currentIndex <= 0) return;
    m_currentIndex--;
    m_backend->play(m_tracks[m_currentIndex].filePath);
    emit trackChanged(m_tracks[m_currentIndex]);
}

void PlaybackQueue::playAt(int index) {
    if (index < 0 || index >= m_tracks.size()) return;
    m_currentIndex = index;
    m_backend->play(m_tracks[m_currentIndex].filePath);
    emit trackChanged(m_tracks[m_currentIndex]);
    emit playbackStateChanged(true);
}

void PlaybackQueue::onTrackEnded() {
    next();
}

Track PlaybackQueue::currentTrack() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_tracks.size())
        return Track{};
    return m_tracks[m_currentIndex];
}

int PlaybackQueue::currentIndex() const {
    return m_currentIndex;
}

bool PlaybackQueue::isPlaying() const {
    return m_backend->isPlaying();
}