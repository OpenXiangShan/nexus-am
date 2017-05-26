#include "common.h"

static int real_fps;
void set_fps(int value){
    real_fps = value;
}

int get_fps(){
    return real_fps;
}

int main (){
    _ioe_init();
    int num_draw = 0, frames = 0;
    ulong next_frame = 0, next_refresh = 0;
    while(1){
        ulong time;
        bool redraw = false;

        while (1) {
            time = _uptime();
            if (time >= next_frame) break;
        }
        frames ++;
        if (time > next_refresh) {
            redraw = true;
            next_refresh += 1000 / FPS;
        }
        next_frame += 1000 / HZ;

        while(keyboard_event());
        while(update_keypress());
         
        if(frames % (HZ / CHARACTER_PER_SECOND) == 0){
            create_new_letter();
        }
        if(frames % (HZ / UPDATE_PER_SECOND) == 0){
            update_letter_pos();
        }

        if(redraw){
            num_draw++;
            set_fps(num_draw * 1000 / time);
            redraw_screen();
        }
    }
}
