#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <map>
#include <Qt>

using namespace std;

#define FC_KEY_A 0
#define FC_KEY_B 1
#define FC_KEY_SELECT 2
#define FC_KEY_START 3
#define FC_KEY_UP 4
#define FC_KEY_DOWN 5
#define FC_KEY_LEFT 6
#define FC_KEY_RIGHT 7

class Controller
{
private:
    //
    bool strobe; //
    uint8_t keystate; //
public:
    //
    void write_strobe(uint8_t data);
    uint8_t output_key_states();
public:
    void init();
    void get_key_states(); //
    map<int, uint8_t> key_map; //
    bool cur_keystate[8]; //
};

void SetKeyMap();

#endif
