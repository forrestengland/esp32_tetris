/* micro sd card read of raw audio */
/* i2s from https://www.youtube.com/watch?v=oVVcuUuJ9CM */

// for audio
#include <driver/i2s.h>
#include <math.h>

// for oled
#include <Wire.h>
#include <U8g2lib.h>

// for sd card
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <stdio.h>

extern "C" {
  #include "tetris.h"
}

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define OLED_RESET -1
// For 128x128 SH1107 I2C (Default ESP32 pins 21/22)
// This constructor often fixes the 96-pixel offset issue on SH1107 screens
U8G2_SH1107_PIMORONI_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define DEG2RAD 0.0174532925

// Defines for I2S, sample rate, frequency, and pins [0.1]
#define I2S_NUM           I2S_NUM_0
#define SAMPLE_RATE       44100
#define WAVE_FREQ_HZ      440.0f
#define PI                3.14159265f

#define I2S_BCK 26
#define I2S_WS  27
#define I2S_DIN 25

#define BUTTON_A_PIN 2
#define BUTTON_B_PIN 4
#define BUTTON_DOWN_PIN 13
#define BUTTON_UP_PIN 12
#define BUTTON_LEFT_PIN 14
#define BUTTON_RIGHT_PIN 15
int buttonAState = 0;
int buttonBState = 0;
int buttonDownState = 0;
int buttonUpState = 0;
int buttonLeftState = 0;
int buttonRightState = 0;

// sd card stuff
File file;
int fileopen = 0;

typedef enum {
  application_menu,
  application_tetris,
  application_music,
  application_tracker,
  application_count
} application;

//application current_app = application_tetris;
application current_app = application_menu;
#define APP_SELECTION_TETRIS 0
#define APP_SELECTION_MUSIC 1
#define APP_SELECTION_TRACKER 2
#define APP_SELECTION_COUNT 3
int app_selection = 0;

#define AUDIO_BUFFER_SIZE 1024
char audio_buffer[AUDIO_BUFFER_SIZE];

void readbutton_task(void* pvParameters) {
  int state = !digitalRead(BUTTON_A_PIN);
  if (state != buttonAState) {
    buttonAState = state;
    Serial.printf("button a state changed: %d\n", buttonAState);
  }
  vTaskDelay(pdMS_TO_TICKS(10000));  
}

int read_file() {

  size_t bytes_read = file.readBytes((char*)&audio_buffer, AUDIO_BUFFER_SIZE);

  if (bytes_read == 0) { // end of file
    file.close();
    fileopen = false; // done playing file
  }

  return bytes_read;
}

int fill_i2s_buffer(int bytes_to_write) {

  /* writes bytes to buffer, returns true if all bytes sent else false,
     keeps track of how many left to write, so just keep calling this
     routine until returns true to know they've all been written, then
     you can refill the buffer */

  size_t bytes_written; // number of bytes written to i2s this time
  static uint16_t buffer_index = 0; // current pos of buffer to output next
  char* data_ptr; // point to next data to send to i2s
  uint16_t bytes_to_send;

  data_ptr = audio_buffer + buffer_index;
  bytes_to_send = bytes_to_write - buffer_index;

  // write with no delay
  //  i2s_write(I2S_NUM, audio_buffer, bytes_to_write, &bytes_written, 1);
  i2s_write(I2S_NUM, data_ptr, bytes_to_send, &bytes_written, 1);
  buffer_index += bytes_written;

  // check if we're done sending the whole buffer
  if (buffer_index >= bytes_to_write) {
    // reset
    buffer_index = 0;
    // tell caller we sent the whole buffer
    return true;
  } else {
    // tell the caller we weren't able to send all the data, call us again
    return false;
  }
  
}

void play_audio() {

  static size_t bytes_read;

  /* whether to read the file or write to i2 */
  static int reading_file = 1;

  if (reading_file) { // read this time

    bytes_read = read_file();

    reading_file = false; // we have data to output, don't need to read next time

  } else {

    reading_file = fill_i2s_buffer(bytes_read);
  }
}

void i2s_task(void *pvParameters) {

    // stuff for sine wave
    float phase = 0.0f;
    float phase_inc = (2.0f * PI * WAVE_FREQ_HZ) / SAMPLE_RATE;
    int16_t sample;

    while (true) {

      // do the raw soundfile playback if the file was opened successfully
      if (fileopen) {

	play_audio();
	
      } else {

        // Calculate and write sine sample to I2S buffer [0.1]
        sample = (int16_t)(sinf(phase) * 10000.0f) * 0.1; // sample data 
        int16_t samples[2] = {sample, sample};
	size_t bytes_written; // number of bytes written to i2s
	
	i2s_write(I2S_NUM, &samples, sizeof(samples), &bytes_written, portMAX_DELAY);
		
        phase += phase_inc;
        if (phase >= 2.0f * PI) phase -= 2.0f * PI;
      }
    }
}

void draw_tetris() {

  int fieldx = ((120 / 2) - (50 / 2)) - 3;
  int fieldy = 10 - 3;
  int fieldw = 50 + 3;
  int fieldh = 100 + 3;
  u8g2.drawFrame(fieldx, fieldy, fieldw, fieldh);

  // draw tetris
  
  for (int row=4; row<24; row++) {
    for (int col=0; col<10; col++) {

      /* need to find out if part of the active tetromino is in the current block
	 and draw it */

      /* current block coordinates */
      int bx = col * 5 + 2;
      int by = (row-4) * 5 + 2;
      int bh = 4;
      int bw = 4;
      
      /* find out if the current block is concurrent with the active tetromino */
      if (active_tetromino_occupies_block(row, col)) {
				u8g2.drawBox(fieldx + bx, fieldy + by, bw, bh);
      } else if (tetris_field[row * TETRIS_FIELD_COLS + col]) {
				// there's a background block here, draw it as an outline
				u8g2.drawFrame(fieldx + bx, fieldy + by, bw, bh);
      }
    }
  }

  char score_str[255];
  snprintf(score_str, 255, "%d", tetris_score);
  u8g2.drawStr(10, 10, score_str);
}

void draw_menu() {

  u8g2.drawStr(3, 10, "tetris");
  u8g2.drawStr(3, 20, "music");
  u8g2.drawStr(3, 30, "tracker");

  u8g2.drawFrame(0, app_selection * 10, 128, 12);
}

void draw_music() {

  u8g2.drawStr(3, 10, "flamenco.raw");
  u8g2.drawFrame(0, 0, 128, 12);

}

void draw_tracker() {
  u8g2.drawStr(3, 10, "tracker");
}

void setup() {

  Serial.begin(115200);

  Serial.println("setup");

   // Configure I2S mode, sample rate, bits per sample, and pins [0.1]
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .dma_buf_count = 8,
    .dma_buf_len = 64
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_DIN, // Verified field for 2026 cores
    .data_in_num = -1
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  xTaskCreatePinnedToCore(i2s_task, "i2s_task", 4096, NULL, 5, NULL, 1);

  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);    
  //  xTaskCreatePinnedToCore(readbutton_task, "readbutton_task", 4096, NULL, 6, NULL, 0);  

  u8g2.begin();
  u8g2.clearBuffer();          // Clear internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // Choose a font
  /*  u8g2.drawStr(0, 20, "Button A released");
  u8g2.drawStr(0, 30, "Button B released");
  u8g2.drawStr(0, 40, "Button Down released");
  u8g2.drawStr(0, 50, "Button Up released");
  u8g2.drawStr(0, 60, "Button Left released");
  u8g2.drawStr(0, 70, "Button Right released");  */
  u8g2.sendBuffer();           // Transfer buffer to display

  delay(1000);

  if(!SD.begin(5)){ // 5 is the CS pin
    Serial.println("Card Mount Failed");
    return;
  } else {
    Serial.println("card mount success");
  }

  if (SD.exists("/FLAMENCO.RAW")) {
    Serial.println("file exists");
  } else {
    Serial.println("file doesn't exist");
    return;
  }
    
  file = SD.open("/FLAMENCO.RAW");
  if (file) {
    Serial.println("file open success");
    fileopen = 1;
  } else {
    Serial.println("file open fail");
  }
    
  //  file.close();
}


void loop()
{

  int state = !digitalRead(BUTTON_A_PIN);
  if (state != buttonAState) {
    buttonAState = state;
    Serial.printf("buttonAState changed to %d\n", buttonAState);
    if (buttonAState) {
      if (current_app == application_menu) {
	if (app_selection == APP_SELECTION_TETRIS) {
	  current_app = application_tetris;
	} else if (app_selection == APP_SELECTION_MUSIC) {
	  current_app = application_music;
	} else if (app_selection == APP_SELECTION_TRACKER) {
	  current_app = application_tracker;
	}
      } else if (current_app == application_tetris) {
	rotate_active_tetromino();
      }
    }
  }

  state = !digitalRead(BUTTON_B_PIN);
  if (state != buttonBState) {
    buttonBState = state;
    Serial.printf("buttonBState changed to %d\n", buttonBState);
    if (buttonBState) {
      current_app = application_menu;
    }
  }

  state = !digitalRead(BUTTON_DOWN_PIN);
  if (state != buttonDownState) {

    buttonDownState = state;

    Serial.printf("buttonDownState changed to %d\n", buttonDownState);

    if (current_app == application_menu && buttonDownState) {
      app_selection++;
      if (app_selection >= APP_SELECTION_COUNT) {
	app_selection = APP_SELECTION_COUNT - 1;
      }
    } else if (current_app == application_tetris) {
      drop_tetro_fast = buttonDownState;
    }
  }

  state = !digitalRead(BUTTON_UP_PIN);
  if (state != buttonUpState) {
    buttonUpState = state;
    Serial.printf("buttonUpState changed to %d\n", buttonUpState);
    if (current_app == application_menu && buttonUpState) {
      app_selection--;
      if (app_selection < 0) app_selection = 0;
    }
  }

  state = !digitalRead(BUTTON_LEFT_PIN);
  if (state != buttonLeftState) {
    buttonLeftState = state;
    Serial.printf("buttonLeftState changed to %d\n", buttonLeftState);
    if (buttonLeftState && current_app == application_tetris) {
      move_tetromino_left();
      //      active_tetromino_x--;
      //      if (active_tetromino_x < 0) active_tetromino_x = 0;
    }
  }

  state = !digitalRead(BUTTON_RIGHT_PIN);
  if (state != buttonRightState) {
    buttonRightState = state;
    Serial.printf("buttonRightState changed to %d\n", buttonRightState);
    if (buttonRightState && current_app == application_tetris) {
      // move the tetromino to the right if we can
      move_tetromino_right();
      //      active_tetromino_x++;
      //      if (active_tetromino_x > 10 - 4) active_tetromino_x = 10 - 4;
    }
  }

  u8g2.clearBuffer();          // Clear internal memory

  if (current_app == application_menu) {

    draw_menu();
    
  } else if (current_app == application_tetris) {

    draw_tetris();
    
  } else if (current_app == application_music) {

    draw_music();
    
  } else if (current_app == application_tracker) {

    draw_tracker();
    
  }

  u8g2.sendBuffer();           // Transfer buffer to display

  // update game state
  update_tetris();
  
  vTaskDelay(pdMS_TO_TICKS(10));
}
