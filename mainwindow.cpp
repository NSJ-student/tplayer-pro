#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <functional>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    plist = new PlayList(parent);
    plist->set_window_pos(this->pos().x() + this->size().width(),
                          this->pos().y());

    //plist->pos().setX(ui->centralWidget->pos().x() + this->size().width());
    //plist->pos().setY(ui->centralWidget->pos().y());
    WId winid = ui->wgVideo->winId();
    player = new TPlayer(winid);
    player->set_callback(std::bind(&PlayList::to_prev_play_obj, plist),
                         std::bind(&PlayList::to_next_play_obj, plist),
                         std::bind(&PlayList::get_play_obj, plist, std::placeholders::_1));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Play(void)
{
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    /* Initialize GStreamer */
    gst_init(NULL, NULL);

    qDebug() << "gst_init";
    /* Build the pipeline */
    pipeline = gst_parse_launch("playbin uri=file:///C:/gstreamer/1.0/x86_64/bin/test.mp4", NULL);

    qDebug() << "gst_parse_launch";
    /* Start playing */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    qDebug() << "gst_element_set_state";
    /* Wait until error or EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)((int)GST_MESSAGE_ERROR | (int)GST_MESSAGE_EOS));
    /* Free resources */
    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void MainWindow::on_pbPlay_clicked()
{
    //Play();
    //g_idle_add(player->st_play_start, this);
    player->play_start(&player->play_obj);
}

void MainWindow::on_pbStop_clicked()
{
    player->play_stop(&player->play_obj);
}

void MainWindow::on_pbPrev_clicked()
{
    player->play_prev(&player->play_obj);
}

void MainWindow::on_pbNext_clicked()
{
    player->play_next(&player->play_obj);
}

void MainWindow::on_MainWindow_iconSizeChanged(const QSize &iconSize)
{
}

void MainWindow::on_pbList_clicked()
{
    if(plist->isVisible())
        plist->hide();
    else
    {
        plist->set_window_pos(this->pos().x() + this->size().width(),
                              this->pos().y());
        plist->show();
    }
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    if(plist->isVisible())
    {
        plist->set_window_pos(this->pos().x() + this->size().width(),
                              this->pos().y());
    }
}

void MainWindow::on_MainWindow_destroyed()
{
    plist->hide();
    plist->close();
}
