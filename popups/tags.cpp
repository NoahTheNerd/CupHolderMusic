//
// Created by noahg on 3/11/26.
//

#include "tags.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "../database.h"

Tags::Tags(const Album &album, QWidget *parent):QDialog(parent) {
    setWindowTitle(album.title + " - Tags");
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

    QHBoxLayout *hBox;
    hBox = new QHBoxLayout();

    // Tag Search
    QLineEdit *tagSearchBox;
    tagSearchBox = new QLineEdit(this);
    tagSearchBox->setPlaceholderText("Search/Create tags...");
    tagSearchBox->setFixedWidth(220);
    tagSearchBox->setStyleSheet(
     "QLineEdit {"
        "  background: #252535; color: #e0e0e0; border: 1px solid #3a3a4a;"
        "  border-radius: 6px; padding: 4px 10px; font-size: 12px;"
        "}"
        "QLineEdit:focus { border-color: #5a5aaa; }"
    );

    QListWidget *tagList = new QListWidget(this);
    tagList->setFixedHeight(10 * 28);

    QStringList allTags = Database::instance().allTags();

    QStringList checkedTags = Database::instance().tagsForAlbum(album.id);

    for (const QString &tag : checkedTags) {
        auto *item = new QListWidgetItem(tag, tagList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }

    for (const QString &tag : allTags) {
        if (!checkedTags.contains(tag)) {
            auto *item = new QListWidgetItem(tag, tagList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
        }
    }

    connect(tagSearchBox, &QLineEdit::textChanged, [tagList](const QString &text) {
    QString search = text.toLower();
    for (int i = 0; i < tagList->count(); i++) {
        QListWidgetItem *item = tagList->item(i);
        item->setHidden(!item->text().contains(search));
    }
});

    connect(tagSearchBox, &QLineEdit::returnPressed, [tagList, tagSearchBox]() {
        QString newTag = tagSearchBox->text().toLower().trimmed();
        if (newTag.isEmpty()) return;

        for (int i = 0; i < tagList->count(); i++) {
            if (tagList->item(i)->text() == newTag) {
                tagList->item(i)->setCheckState(Qt::Checked);
                tagSearchBox->clear();
                return;
            }
        }

        auto *item = new QListWidgetItem(newTag, tagList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        tagSearchBox->clear();
    });


    QPushButton *createTagButton = new QPushButton("Create tag");

    auto createTag = [tagList, tagSearchBox]() {
        QString newTag = tagSearchBox->text().toLower().trimmed();
        if (newTag.isEmpty()) return;

        for (int i = 0; i < tagList->count(); i++) {
            if (tagList->item(i)->text() == newTag) {
                tagList->item(i)->setCheckState(Qt::Checked);
                tagSearchBox->clear();
                return;
            }
        }

        auto *item = new QListWidgetItem(newTag, tagList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        tagSearchBox->clear();
    };

    connect(createTagButton, &QPushButton::clicked, createTag);

    QDialogButtonBox *buttons;
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, [this, album, tagList]() {
        QStringList newTags;
        for (int i = 0; i < tagList->count(); i++) {
            if (tagList->item(i)->checkState() == Qt::Checked)
                newTags << tagList->item(i)->text();
        }
            Database::instance().setTagsForAlbum(album.id, newTags);
            emit tagsUpdated(album.id, newTags);
            accept();
        }
    );
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    hBox->addWidget(tagSearchBox);
    hBox->addWidget(createTagButton);

    layout->addLayout(hBox);
    layout->addWidget(tagList);
    layout->addWidget(buttons);
}