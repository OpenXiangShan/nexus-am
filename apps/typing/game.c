#include "game.h"

static int real_fps;

void set_fps(int value) {
  real_fps = value;
}

int get_fps() {
  return real_fps;
}

int main (){
  _ioe_init();
  int num_draw = 0, frames = 0;
  ulong next_frame = 0, next_refresh = 0;
  while(1) {
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

    while (keyboard_event());
    while (update_keypress());
     
    if (frames % (HZ / CHARACTER_PER_SECOND) == 0) {
      create_new_letter();
    }
    if (frames % (HZ / UPDATE_PER_SECOND) == 0) {
      update_letter_pos();
    }

    if(redraw) {
      num_draw ++;
      set_fps(num_draw * 1000 / time);
      redraw_screen();
    }
  }
}

LINKLIST_IMPL(fly, 1000)

static fly_t head = NULL;
static int hit = 0, miss = 0;

int get_hit(){
  return hit;
}

int get_miss(){
  return miss;
}

fly_t characters(){
  return head;
}

void create_new_letter(){
  if(head == NULL){
    head = fly_new();
  }
  else{
    fly_t now = fly_new();
    fly_insert(NULL,head,now);
    head = now;
  }
  
  head->x = 0;
  head->y = rand() % (W / 8 - 2) * 8 + 8;
  head->v = (rand() % 1000)/(2000) + 1;
  head->text = rand() % 26;
  release_key(head->text);
}

void update_letter_pos() {
  fly_t it;
  for(it = head;it != NULL;){
    fly_t next = it->_next;
    it->x += it->v;
    if (it->x < 0 || it->x + 8 > H){
      if(it->x < 0)
        hit++;
      else
        miss++;
      fly_remove(it);
      fly_free(it);
      if(it == head)
        head = next;
    }
    it = next;
  }
}

bool update_keypress() {
  fly_t it,target = NULL;
  int min = -100;
  for(it = head; it != NULL; it = it->_next){
    if(it->v > 0 && it->x > min && query_key(it->text)){
      min = it->x;
      target = it;
    }
  }
  if(target != NULL){
    release_key(target->text);
    target->v = -3;
    return true;
  }
  return false;
}
