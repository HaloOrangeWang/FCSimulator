#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QKeyEvent>
#include "total.h"
//#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene_game = new QGraphicsScene;
    pixels = new QRgb[256 * 240];

    this->setFocusPolicy(Qt::StrongFocus);
}

void MainWindow::FCInit()
{
    // TODO:
    timer_game = new QTimer(this);
    timer_game->setTimerType(Qt::PreciseTimer);
    connect(timer_game, SIGNAL(timeout()), this, SLOT(OnNewFrame()));
    timer_game->start(50);

    //usleep(16000);
    _sleep(16);
    timer_game_2 = new QTimer(this);
    timer_game_2->setTimerType(Qt::PreciseTimer);
    connect(timer_game_2, SIGNAL(timeout()), this, SLOT(OnNewFrame()));
    timer_game_2->start(50);

    //usleep(17000);
    _sleep(17);
    timer_game_3 = new QTimer(this);
    timer_game_3->setTimerType(Qt::PreciseTimer);
    connect(timer_game_3, SIGNAL(timeout()), this, SLOT(OnNewFrame()));
    timer_game_3->start(50);
}

void MainWindow::OnNewFrame()
{
    int old_end_frame = Ppu2.frame_finished;

    while(true)
    {
        Ppu2.run_1cycle();
        Ppu2.run_1cycle();
        Ppu2.run_1cycle();
        Cpu.run_1cycle();
        if (Ppu2.frame_finished > old_end_frame)
            break;
    }

    //qDebug() << "Show frame: " << Ppu2.frame_finished << endl;
    scene_game->clear();

    for (int x = 0; x <= 255; x++){
        for (int y = 0; y <= 239; y++){
            pixels[y * 256 + x] = qRgb(Ppu2.frame_data[x][y][0], Ppu2.frame_data[x][y][1], Ppu2.frame_data[x][y][2]);
        }
    }
    QImage img((uchar*)pixels, 256, 240, QImage::Format_ARGB32);
    QPixmap img_pixmap = QPixmap::fromImage(img);

    pixmap_lp = new QGraphicsPixmapItem;
    pixmap_lp->setPixmap(img_pixmap.scaled(512, 480, Qt::KeepAspectRatio));
    pixmap_lp->setPos(QPointF(0, 0));
    scene_game->addItem(pixmap_lp);
    ui->graphicsView->setScene(scene_game);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (controller_left.key_map.find(key) != controller_left.key_map.end()){
        controller_left.cur_keystate[controller_left.key_map[key]] = true;
    }
    if (controller_right.key_map.find(key) != controller_right.key_map.end()){
        controller_right.cur_keystate[controller_right.key_map[key]] = true;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    if (controller_left.key_map.find(key) != controller_left.key_map.end()){
        controller_left.cur_keystate[controller_left.key_map[key]] = false;
    }
    if (controller_right.key_map.find(key) != controller_right.key_map.end()){
        controller_right.cur_keystate[controller_right.key_map[key]] = false;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
