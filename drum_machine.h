#ifndef H_DRUMMACHINE
#define H_DRUMMACHINE

const int dm_steps = 16;
const int dm_channels = 4;
const int dm_size = 6;

int dm_cursor_channel = 0;
int dm_cursor_step = 0;

int dm_pattern[dm_steps * dm_channels] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void drummachine_inc_cursstep() {
  dm_cursor_step++;
  if (dm_cursor_step >= dm_steps) dm_cursor_step = dm_steps - 1;
  Serial.printf("drummachine inc cursstep: %d\n", dm_cursor_step);
}

void drummachine_dec_cursstep() {
  dm_cursor_step--;
  if (dm_cursor_step < 0) dm_cursor_step = 0;
  Serial.printf("drummachine dec cursstep: %d\n", dm_cursor_step);
}

void drummachine_inc_curschan() {
  dm_cursor_channel++;
  if (dm_cursor_channel >= dm_channels) dm_cursor_channel = dm_channels - 1;
  Serial.printf("drummachine inc curschan: %d\n", dm_cursor_channel);
}

void drummachine_dec_curschan() {
  dm_cursor_channel--;
  if (dm_cursor_channel < 0) dm_cursor_channel = 0;
  Serial.printf("drummachine dec curschan: %d\n", dm_cursor_channel);
}

void drummachine_toggle_cursstep() {
  dm_pattern[dm_cursor_channel * dm_steps + dm_cursor_step] = !dm_pattern[dm_cursor_channel * dm_steps + dm_cursor_step];
}

void drummachine_draw(U8G2 &u8g2) {

  u8g2.drawStr(3, 10, "drum machine");

  for (int c=0; c<dm_channels; c++) {
    for (int s=0; s<dm_steps; s++) {
      u8g2.drawFrame(3 + (dm_size+1)*s, 20 + (dm_size+1)*c, dm_size, dm_size);
      if (c == dm_cursor_channel && s == dm_cursor_step) {
	u8g2.drawFrame(3 + (dm_size+1)*s - 1, 20 + (dm_size+1)*c - 1, dm_size + 2, dm_size + 2);
      }
      if (dm_pattern[c * dm_steps + s]) {
	u8g2.drawFrame(3 + (dm_size+1)*s + 2, 20 + (dm_size+1)*c + 2, dm_size - 4, dm_size - 4);
      }
    }
  }
}

#endif
