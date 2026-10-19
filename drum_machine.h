#ifndef H_DRUMMACHINE
#define H_DRUMMACHINE

// #include "fosc.h"

const int dm_steps = 16;
const int dm_channels = 4;
const int dm_size = 6;
const int dm_samples_per_minute = 44100 * 60;

int dm_cursor_channel = 0;
int dm_cursor_step = 0;

int dm_pattern[dm_steps * dm_channels] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int dm_play_step = 0;
int dm_sample = 0;
float dm_tempo = 120.0f;

// fosc* dm_osc = 0;
float dm_gate = 0.0f;

void drummachine_init() {
  //  dm_osc = fosc_create();
  //  fosc_setfreq(dm_osc, 100.0);
}

float drummachine_process() {

  dm_sample++;

  int samples_per_step = dm_samples_per_minute / dm_tempo / 4;
  if (dm_sample % samples_per_step == 0) { // start of step
    dm_play_step++;
    if (dm_play_step >= dm_steps) dm_play_step = 0;
    dm_gate = 1.0f;
  } else if (dm_sample % (samples_per_step / 2) == 0) {
    // halfway through step - turn gate off
    dm_gate = 0.0f;
  }

  //  fosc_compute(dm_osc, dm_sample);
  //  float os = dm_osc->sinevalue;
  float os = (float)random(32767 * 2) / 32767.0f - 0.5;
  
  return os * dm_gate;
}

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

  // draw play step
  u8g2.drawBox(3 + dm_play_step * (dm_size + 1), 20 + dm_channels * (dm_size + 1), dm_size, dm_size);
}

#endif
