#include "total.h"
#include <QDebug>

void Controller::init()
{
    strobe = false;
    keystate = 0;
    for (int t = 0; t <= 7; t++)
        cur_keystate[t] = false;
}

void Controller::get_key_states()
{
    keystate = 0;
    for (int key_id = FC_KEY_A; key_id <= FC_KEY_RIGHT; key_id++){
        if (cur_keystate[key_id])
            keystate |= (1 << key_id);
    }
}

void Controller::write_strobe(uint8_t data)
{
    bool strobe_old = strobe;
    strobe = data & 1;
    if (strobe_old && (!(strobe))){
        //在选通标志位被关闭时，获取关闭前的当前按键情况
        get_key_states();
    }
}

uint8_t Controller::output_key_states()
{
    bool is_key_pressed;
    if (strobe){
        //选通状态下，输出当前真实的按键情况
        is_key_pressed = cur_keystate[FC_KEY_A];
    }else{
        //不选通的情况下，输出缓存的按键情况
        is_key_pressed = keystate & 1;
        keystate >>= 1;
    }
    return (0x40 | is_key_pressed);
}

void SetKeyMap()
{
    //设置真实按键和虚拟按键的对应关系
    controller_left.key_map = map<int, uint8_t>();
    controller_left.key_map.insert(make_pair(Qt::Key_A, FC_KEY_LEFT));
    controller_left.key_map.insert(make_pair(Qt::Key_S, FC_KEY_DOWN));
    controller_left.key_map.insert(make_pair(Qt::Key_D, FC_KEY_RIGHT));
    controller_left.key_map.insert(make_pair(Qt::Key_W, FC_KEY_UP));
    controller_left.key_map.insert(make_pair(Qt::Key_J, FC_KEY_B));
    controller_left.key_map.insert(make_pair(Qt::Key_K, FC_KEY_A));
    controller_left.key_map.insert(make_pair(Qt::Key_Space, FC_KEY_START));
    controller_left.key_map.insert(make_pair(Qt::Key_Shift, FC_KEY_SELECT));

    controller_right.key_map = map<int, uint8_t>();
    controller_right.key_map.insert(make_pair(Qt::Key_Comma, FC_KEY_LEFT));
    controller_right.key_map.insert(make_pair(Qt::Key_Period, FC_KEY_DOWN));
    controller_right.key_map.insert(make_pair(Qt::Key_Slash, FC_KEY_RIGHT));
    controller_right.key_map.insert(make_pair(Qt::Key_L, FC_KEY_UP));
    controller_right.key_map.insert(make_pair(Qt::Key_Z, FC_KEY_B));
    controller_right.key_map.insert(make_pair(Qt::Key_X, FC_KEY_A));
    controller_right.key_map.insert(make_pair(Qt::Key_BracketLeft, FC_KEY_START));
    controller_right.key_map.insert(make_pair(Qt::Key_BracketRight, FC_KEY_SELECT));
}
