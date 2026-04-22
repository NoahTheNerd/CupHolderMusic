//
// Created by noahg on 3/11/26.
//

#pragma once
#include <QDialog>
#include "../models/album.h"

class Tags : public QDialog {
    Q_OBJECT
public:
    explicit Tags(const Album &album, QWidget *parent = nullptr);
    signals:
        void tagsUpdated(int albumId, QStringList newTags);

};