//
// Created by noahg on 3/30/26.
//

#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>

class AlbumView : public QWidget {
    Q_OBJECT

public:
    explicit AlbumView(QWidget *parent = nullptr);
private:
    QScrollArea    *m_scrollArea{};
    void uiSetupper();
    QWidget        *m_gridContainer{};
    QGridLayout    *m_gridLayout = nullptr;
};