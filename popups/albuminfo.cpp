//
// Created by noahg on 3/11/26.
//

#include "albuminfo.h"
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>

AlbumInfo::AlbumInfo(const Album &album, QWidget *parent):QDialog(parent) {
    setWindowTitle(album.title);
    setMinimumWidth(300);
    setStyleSheet(
        "QDialog {"
        "   background-color: #252535;"
        "   border: 2px solid #5a5aaa;"
        "}"
        "QLabel {"
        "   background-color: transparent;"
        "   color: #e0e0e0;"
        "}"
    );
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    QVBoxLayout *layout;
    layout = new QVBoxLayout(this);
    QFormLayout *form;
    form = new QFormLayout();

    form->addRow("Artist",       new QLabel(album.artist, this));
    form->addRow("Source",       new QLabel(album.formatsString(), this));
    form->addRow("Released",     new QLabel(QString::number(album.releaseYear), this));
    form->addRow("Tracks",       new QLabel(QString::number(album.trackCount), this));
    form->addRow("Date Added",   new QLabel(album.dateAdded.toString("yyyy-MM-dd"), this));
    form->addRow("First Played", new QLabel(album.firstPlayed.isValid()? album.firstPlayed.toString("yyyy-MM-dd"): "Never", this));
    form->addRow("Last Played",  new QLabel(album.lastPlayed.isValid()? album.lastPlayed.toString("yyyy-MM-dd"): "Never", this));

    QDialogButtonBox *buttons;
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    layout->addLayout(form);
    layout->addWidget(buttons);
}