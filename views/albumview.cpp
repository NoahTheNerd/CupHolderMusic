//
// Created by noahg on 3/30/26.
//

#include "albumview.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include "../settings.h"

AlbumView::AlbumView(QWidget *parent) : QWidget(parent) {
    uiSetupper();
}

void AlbumView::uiSetupper() {
    setStyleSheet("background-color: #0f0f0f;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // scroll are-uhhhhh i frog got 🐸 <- frog, cuz i frog got, get it?? hahahahahahaha
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background-color: #321160; }"
        "QScrollBar:vertical { background: #1a1a26; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #3a3a4a; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    m_gridContainer = new QWidget();
    m_gridContainer->setStyleSheet("background-color: #13131c;");
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setSpacing(Settings::instance().cardSpacing());
    m_gridLayout->setContentsMargins(24, 24, 24, 24);

    m_scrollArea->setWidget(m_gridContainer);
    mainLayout->addWidget(m_scrollArea);
}