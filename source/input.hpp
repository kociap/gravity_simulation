#pragma once

#include <build.hpp>
#include <mimas/mimas.h>

struct Key_State {
    bool up_down_transitioned = false;
    bool down = false;
};

void update_input();
void add_key_event(Mimas_Key key, Mimas_Key_Action action);
Key_State get_key_state(Mimas_Key key);
bool key_pressed(Key_State state);
bool key_released(Key_State state);
