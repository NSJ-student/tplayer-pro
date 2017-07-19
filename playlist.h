#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QWidget>

namespace Ui {
class PlayList;
}

class PlayList : public QWidget
{
    Q_OBJECT

public:
    explicit PlayList(QWidget *parent = 0);
    ~PlayList();
    void set_window_pos(int x, int y);

    bool to_next_play_obj(void);
    bool to_prev_play_obj(void);
    void get_play_obj(char ** p_play_path);

private slots:
    void on_pbAddFile_clicked();

    void on_pbAddDirectory_clicked();

    void on_pbDeleteFile_clicked();

    void on_pbDeleteAll_clicked();

private:
    Ui::PlayList *ui;
    int play_idx;
};

#endif // PLAYLIST_H
