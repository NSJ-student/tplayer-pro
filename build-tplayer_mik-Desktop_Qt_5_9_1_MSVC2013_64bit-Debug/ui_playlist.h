/********************************************************************************
** Form generated from reading UI file 'playlist.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYLIST_H
#define UI_PLAYLIST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PlayList
{
public:
    QListWidget *lwMediaList;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QPushButton *pbAddFile;
    QPushButton *pbAddDirectory;
    QPushButton *pbDeleteFile;
    QPushButton *pbDeleteAll;

    void setupUi(QWidget *PlayList)
    {
        if (PlayList->objectName().isEmpty())
            PlayList->setObjectName(QStringLiteral("PlayList"));
        PlayList->resize(390, 300);
        lwMediaList = new QListWidget(PlayList);
        lwMediaList->setObjectName(QStringLiteral("lwMediaList"));
        lwMediaList->setGeometry(QRect(0, 0, 391, 271));
        widget = new QWidget(PlayList);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(0, 270, 391, 30));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setSpacing(7);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        pbAddFile = new QPushButton(widget);
        pbAddFile->setObjectName(QStringLiteral("pbAddFile"));

        horizontalLayout->addWidget(pbAddFile);

        pbAddDirectory = new QPushButton(widget);
        pbAddDirectory->setObjectName(QStringLiteral("pbAddDirectory"));

        horizontalLayout->addWidget(pbAddDirectory);

        pbDeleteFile = new QPushButton(widget);
        pbDeleteFile->setObjectName(QStringLiteral("pbDeleteFile"));

        horizontalLayout->addWidget(pbDeleteFile);

        pbDeleteAll = new QPushButton(widget);
        pbDeleteAll->setObjectName(QStringLiteral("pbDeleteAll"));

        horizontalLayout->addWidget(pbDeleteAll);


        retranslateUi(PlayList);

        QMetaObject::connectSlotsByName(PlayList);
    } // setupUi

    void retranslateUi(QWidget *PlayList)
    {
        PlayList->setWindowTitle(QApplication::translate("PlayList", "Form", Q_NULLPTR));
        pbAddFile->setText(QApplication::translate("PlayList", "Add File", Q_NULLPTR));
        pbAddDirectory->setText(QApplication::translate("PlayList", "Add Dir", Q_NULLPTR));
        pbDeleteFile->setText(QApplication::translate("PlayList", "Delete", Q_NULLPTR));
        pbDeleteAll->setText(QApplication::translate("PlayList", "Delete All", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class PlayList: public Ui_PlayList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYLIST_H
