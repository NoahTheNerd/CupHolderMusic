//
// Created by noahg on 3/19/26.
//

// audiobackend.h
#pragma once
#include <QObject>
#include <QString>

class AudioBackend : public QObject {
	Q_OBJECT
public:
	explicit AudioBackend(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~AudioBackend() = default;

	virtual void play(const QString &uri) = 0;
	virtual void pause() = 0;
	virtual void stop() = 0;
	virtual void seek(qint64 ms) = 0;
	virtual qint64 position() = 0;
	virtual qint64 duration() = 0;
	virtual bool isPlaying() = 0;

	signals:
		void trackEnded();
};