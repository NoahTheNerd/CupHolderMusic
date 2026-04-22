//
// Created by noahg on 3/19/26.
//

#pragma once
#include <QObject>
#include <QVector>
#include "../models/track.h"
#include "audiobackend.h"

class PlaybackQueue : public QObject {
    Q_OBJECT
public:
    explicit PlaybackQueue(AudioBackend *backend, QObject *parent = nullptr);

    void loadAlbum(const QVector<Track> &tracks);
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    void playAt(int index);

    Track currentTrack() const;
    int currentIndex() const;
    bool isPlaying() const;

public slots:
    void onTrackEnded();

    signals:
        void trackChanged(const Track &track);
        void playbackStateChanged(bool playing);
        void queueFinished();
        void playbackPaused(bool paused);

private:
    AudioBackend    *m_backend;
    QVector<Track>   m_tracks;
    int              m_currentIndex = -1;
    bool m_paused = false;
};