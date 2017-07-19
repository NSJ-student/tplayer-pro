/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QWidget *layoutWidget;
    QFormLayout *formLayout;
    QPushButton *pbPlay;
    QPushButton *pbPrev;
    QPushButton *pbNext;
    QPushButton *pbStop;
    QWidget *wgVideo;
    QPushButton *pbList;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(570, 371);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 290, 161, 51));
        formLayout = new QFormLayout(layoutWidget);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setContentsMargins(0, 0, 0, 0);
        pbPlay = new QPushButton(layoutWidget);
        pbPlay->setObjectName(QStringLiteral("pbPlay"));

        formLayout->setWidget(0, QFormLayout::LabelRole, pbPlay);

        pbPrev = new QPushButton(layoutWidget);
        pbPrev->setObjectName(QStringLiteral("pbPrev"));

        formLayout->setWidget(1, QFormLayout::LabelRole, pbPrev);

        pbNext = new QPushButton(layoutWidget);
        pbNext->setObjectName(QStringLiteral("pbNext"));

        formLayout->setWidget(1, QFormLayout::FieldRole, pbNext);

        pbStop = new QPushButton(layoutWidget);
        pbStop->setObjectName(QStringLiteral("pbStop"));

        formLayout->setWidget(0, QFormLayout::FieldRole, pbStop);

        wgVideo = new QWidget(centralWidget);
        wgVideo->setObjectName(QStringLiteral("wgVideo"));
        wgVideo->setGeometry(QRect(0, 0, 571, 281));
        pbList = new QPushButton(centralWidget);
        pbList->setObjectName(QStringLiteral("pbList"));
        pbList->setGeometry(QRect(470, 320, 93, 28));
        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        pbPlay->setText(QApplication::translate("MainWindow", "Play", Q_NULLPTR));
        pbPrev->setText(QApplication::translate("MainWindow", "Prev", Q_NULLPTR));
        pbNext->setText(QApplication::translate("MainWindow", "Next", Q_NULLPTR));
        pbStop->setText(QApplication::translate("MainWindow", "Stop", Q_NULLPTR));
        pbList->setText(QApplication::translate("MainWindow", "Play List", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
