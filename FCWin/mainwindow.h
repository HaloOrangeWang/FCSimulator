#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void FCInit();
    //void PaintEvent(QPaintEvent* e);
    ~MainWindow();
public slots:
    void OnNewFrame();
protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QGraphicsScene* scene_game;
    QGraphicsPixmapItem *pixmap_lp;
    QTimer* timer_game;
    QTimer* timer_game_2;
    QTimer* timer_game_3;
    QRgb* pixels;
    //clock_t start_time;
    int frame_interval;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
