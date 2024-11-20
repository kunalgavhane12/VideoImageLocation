#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qlabel.h"
#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QTimer>
#include <QStringList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QVideoWidget *videoWidget;
    QLabel *displayLabel;
    QTimer *playTimer;
    QTimer *pauseTimer;

    QStringList videoPaths;
    QStringList imagePaths;

    int currentVideoIndex;
    int currentImageIndex;

    void loadMediaFiles();
    void playNextVideo();
    void pauseAndShowImage();
    void resumeVideo();

private slots:
    void mediaFinished();
    void resizeEvent(QResizeEvent *event);
};

#endif // MAINWINDOW_H
