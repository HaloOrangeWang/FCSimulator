#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QKeyEvent>
#include "total.h"
#include <unistd.h>

void GameThread::run()
{
    while (true)
    {
        emit new_frame();
        //usleep(16667);
        usleep(16590);
    }
}

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
    // 音频播放的初始化
    device = alcOpenDevice(NULL);

    if (!device){
        qDebug() << "初始化openal device失败" << endl;
        abort();
    }
    context = alcCreateContext(device, NULL);
    if (!context){
        qDebug() << "初始化openal context失败!" << endl;
        abort();
    }
    alcMakeContextCurrent(context);
    alGenSources(1, &source);

    al_status = 0;

    GameThread *th = new GameThread(this);
    connect(th, SIGNAL(new_frame()), this, SLOT(OnNewFrame()));
    th->start();
}

void MainWindow::OnNewFrame()
{
    int old_end_frame = Ppu2.frame_finished;

    while(true)
    {
        int scanline_old = Ppu2.scanline;
        Ppu2.run_1cycle();
        Ppu2.run_1cycle();
        Ppu2.run_1cycle();
        Cpu.run_1cycle();
        if ((Ppu2.scanline == 65 || Ppu2.scanline == 130 || Ppu2.scanline == 195 || Ppu2.scanline == 260) && Ppu2.scanline != scanline_old)
            Apu.run_1cycle();
        if (Ppu2.frame_finished > old_end_frame)
            break;
    }

    ALuint buffers;
    alGenBuffers(1, &buffers);
    alBufferData(buffers, AL_FORMAT_MONO8, Apu.seq, Apu.seq_len, SAMPLE_PER_SEC);
    uint8_t last_note = Apu.seq[Apu.seq_len - 1];
    alSourceQueueBuffers(source, 1, &buffers);
    //ALint al_sta;
    //alGetSourcei(source, AL_SOURCE_STATE, &al_sta);
    //if (al_sta != AL_PLAYING){
    //    alSourcePlay(source);
    //}
    ALint al_queued_buffers, al_processed_buffers;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &al_queued_buffers);
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &al_processed_buffers);
    //qDebug() << "AL_BUFFERS_QUEUED = " <<  al_queued_buffers << ", AL_BUFFERS_PROCESSED = " << al_processed_buffers << endl;
    if (al_queued_buffers - al_processed_buffers >= 13 && al_status == 0){
        al_status = 1;
        alSourcePlay(source);
    }else if (al_queued_buffers - al_processed_buffers <= 3 && al_status == 1){
        al_status = 0;
        alSourcePause(source);
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
    pixmap_lp->setPixmap(img_pixmap.scaled(768, 720, Qt::KeepAspectRatio));
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
