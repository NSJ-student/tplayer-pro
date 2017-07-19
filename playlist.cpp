#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QDirIterator>
#include <QList>
#include "playlist.h"
#include "ui_playlist.h"

PlayList::PlayList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayList)
{
    ui->setupUi(this);
    play_idx = 0;
}

PlayList::~PlayList()
{
    delete ui;
}

void PlayList::set_window_pos(int x, int y)
{
    this->move(x,y);
}

void PlayList::on_pbAddFile_clicked()
{
    QFileDialog filedialog(this, tr("Select File"),
                           tr("D:\\Projects\\K-7542"),
                           tr("Videos (*.mp4 *.mkv *.avi *.wmv);; Images (*.jpg *.bmp);; All Files (*.*)"));

    if(filedialog.exec())
    {
        QString strFile = filedialog.selectedFiles().at(0);
        ui->lwMediaList->addItem(strFile);
        /*
        QByteArray byteArray = strFile.toLocal8Bit();
        char * src = byteArray.data();
        int strLen = strFile.length();
        */
    }
}

void PlayList::on_pbAddDirectory_clicked()
{
    QString strDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                           tr("D:\\Projects\\K-7542"), QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    QStringList strFilters;
    strFilters += "*.jpg";
    strFilters += "*.bmp";
    strFilters += "*.mp4";
    strFilters += "*.mkv";
    strFilters += "*.avi";
    strFilters += "*.wmv";

    QDirIterator iterDir(strDir, strFilters, QDir::Files|QDir::NoSymLinks);
    while(iterDir.hasNext())
    {
        iterDir.next();
        ui->lwMediaList->addItem(iterDir.filePath());
    }
}

void PlayList::on_pbDeleteFile_clicked()
{
    qDeleteAll(ui->lwMediaList->selectedItems());
}

void PlayList::on_pbDeleteAll_clicked()
{
    ui->lwMediaList->clear();
}

bool PlayList::to_next_play_obj(void)
{
    int item_cnt = ui->lwMediaList->count();

    if(item_cnt == 0)
        return false;

    if(item_cnt-1 > play_idx)
        play_idx++;
    else
        play_idx=0;

    return true;
}

bool PlayList::to_prev_play_obj(void)
{
    int item_cnt = ui->lwMediaList->count();

    if(item_cnt == 0)
        return false;

    if(0 < play_idx)
        play_idx--;
    else
        play_idx = item_cnt-1;

    return true;
}

void PlayList::get_play_obj(char ** p_play_path)
{
    QString strPath = ui->lwMediaList->item(play_idx)->text();
    QByteArray byteArray = strPath.toLocal8Bit();
    qDebug() << "path =" << byteArray.data();
    *p_play_path = (char *)malloc(strPath.length()+1);
    strcpy(*p_play_path, byteArray.data());
    (*p_play_path)[strPath.length()] = 0;
}
