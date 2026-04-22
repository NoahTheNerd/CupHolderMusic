//
// Created by noahg on 3/11/26.
//

#pragma once
#include <QDialog>
#include "../models/album.h"

class AlbumInfo : public QDialog {
    Q_OBJECT
public:
    explicit AlbumInfo(const Album &album, QWidget *parent = nullptr);
};