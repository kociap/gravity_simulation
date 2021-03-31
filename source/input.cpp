#include <input.hpp>

#include <anton/flat_hash_map.hpp>

static Flat_Hash_Map<Mimas_Key, Key_State> key_states;

void update_input() {
    for(auto& v: key_states) {
        v.value.up_down_transitioned = false;
    }
}

void add_key_event(Mimas_Key key, Mimas_Key_Action action) {
    auto r = key_states.find_or_emplace(key);
    Key_State& state = r->value;
    state.up_down_transitioned = action != state.down;
    state.down = action;
}

Key_State get_key_state(Mimas_Key key) {
    auto r = key_states.find(key);
    if(r != key_states.end()) {
        return r->value;
    } else {
        return Key_State{};
    }
}

bool key_pressed(Key_State state) {
    return state.down && state.up_down_transitioned;
}

bool key_released(Key_State state) {
    return !state.down && state.up_down_transitioned;
}
