#include "mainwindow.h"
#include <QApplication>
#include <string>
#include "total.h"
#include <unistd.h>

using namespace std;

CpuBus Cpubus;
CPU Cpu;
PPU2 Ppu2;
PictureBus PpuBus;
Controller controller_left;
Controller controller_right;
Cartridge cartridge;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //1.初始化：解析NES文件
    cartridge.read_from_file("../Data/Super Mario Bros.nes", 40976);
    //cartridge.read_from_file("../Data/yx.nes", 262288);
    //cartridge.read_from_file("../Data/Contra (U).nes", 131088);
    //2.模拟CPU（这里应该单开一个线程，每559纳秒执行一次的，但是图方便就先这么做了）
    Cpu.reset();
    Ppu2.reset();
    controller_right.init();
    controller_left.init();
    SetKeyMap();

    //3.显示NES文件中的图案表
    MainWindow w;
    w.FCInit();
    w.show();

    return a.exec();
}
