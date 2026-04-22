//
// Created by noahg on 3/8/26.
//

#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include "views/libraryview.h"
#include "views/albumview.h"
#include "audiobackends/audiobackend.h"
#include "audiobackends/queuing.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *m_stack;
    LibraryView    *m_libraryView;
    AlbumView    *m_albumView;
    AudioBackend    *m_backend      = nullptr;
    PlaybackQueue   *m_queue        = nullptr;

private:
    void scanAndLoad();

    void openAlbumView();
    void openLibraryView();
};