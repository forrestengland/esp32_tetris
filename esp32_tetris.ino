/* micro sd card read of raw audio and tetris game */
/* i2s from https://www.youtube.com/watch?v=oVVcuUuJ9CM */

// for audio
#include <driver/i2s.h>
#include <math.h>

// for oled
#include <Wire.h>
#include <U8g2lib.h>

// for sd card
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// for snprintf
#include <stdio.h>

// tetris
extern "C" {
  #include "tetris.h"
}

// oled stuff
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define OLED_RESET -1
// For 128x128 SH1107 I2C (Default ESP32 pins 21/22)
// This constructor often fixes the 96-pixel offset issue on SH1107 screens
U8G2_SH1107_PIMORONI_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Defines for I2S, sample rate, frequency, and pins [0.1]
#define I2S_NUM           I2S_NUM_0
#define SAMPLE_RATE       44100
// #define WAVE_FREQ_HZ      440.0f
// #define PI                3.14159265f
#define I2S_BCK 26
#define I2S_WS  27
#define I2S_DIN 25

// buttons
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

// menu
typedef enum {
  application_menu,
  application_tetris,
  application_music,
  application_tracker,
  application_count
} application;

// current / selected application
//application current_app = application_tetris;
application current_app = application_menu;
#define APP_SELECTION_TETRIS 0
#define APP_SELECTION_MUSIC 1
#define APP_SELECTION_TRACKER 2
#define APP_SELECTION_COUNT 3
int app_selection = 0;

// audio buffer
#define AUDIO_BUFFER_SIZE 1024
char audio_buffer[AUDIO_BUFFER_SIZE];

// audio file selection
int selected_file = 0;
int file_count = 0;
#define MAX_FILES 99
char audio_files[MAX_FILES][255];
int file_display_offset = 0;
const int files_per_screen = 12;

// whether we need to redraw the screen
int need_redraw = 1;

// button reading rtos task
void readbutton_task(void* pvParameters) {
  int state = !digitalRead(BUTTON_A_PIN);
  if (state != buttonAState) {
    buttonAState = state;
    Serial.printf("button a state changed: %d\n", buttonAState);
  }
  vTaskDelay(pdMS_TO_TICKS(10000));  
}

// read the next chunk of audio from a raw audio file
int read_file() {

  size_t bytes_read = file.readBytes((char*)&audio_buffer, AUDIO_BUFFER_SIZE);

  if (bytes_read == 0) { // end of file
    file.close();
    fileopen = false; // done playing file

		play_next_audio_file();

		return 0;
  }

  return bytes_read;
}

// writes to i2s output buffer from audio_buffer
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

// copy next chunk of audio data from sdcard to i2s output buffer
void play_audio() {

  static size_t bytes_read;

  /* whether to read the file or write to i2 */
  static int reading_file = 1;

  if (reading_file) { // read this time

    bytes_read = read_file();

		if (bytes_read == 0) {
			// end of old track. next track should be open now
			reading_file = 1;
			return;
		}

		// convert to float and back for processing
		int16_t* sample_ptr = (int16_t*)audio_buffer;
		int num_samples = bytes_read / sizeof(int16_t);

		float volume = 0.75f;

		for (int i=0; i<num_samples; i++) {
			float sample_f = sample_ptr[i] / 32768.0f;
			sample_f *= volume;

			sample_ptr[i] = (int16_t)(sample_f * 32767.0f);
		}

    reading_file = false; // we have data to output, don't need to read next tim

  } else {
    reading_file = fill_i2s_buffer(bytes_read);
  }
}

// audio rtos task
void i2s_task(void *pvParameters) {

    while (true) {

      // do the raw soundfile playback if the file was opened successfully
      if (fileopen) {

				play_audio();

				// if i2s buffer isn't accepting data yet
				// yeild to give the sd card bus and tetris logic cpu time
				vTaskDelay(pdMS_TO_TICKS(1));
	
      } else {

				size_t bytes_written; // number of bytes written to i2s

				// write zeros to i2s buffer if a file isn't open
				int16_t samples[2] = {0, 0};
	
				i2s_write(I2S_NUM, &samples, sizeof(samples), &bytes_written,
									portMAX_DELAY);
		
			}
    }
}

// draw current tetris frame
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

// draw initial app menu
void draw_menu() {

  u8g2.drawStr(3, 10, "tetris");
  u8g2.drawStr(3, 20, "music");
  u8g2.drawStr(3, 30, "tracker");

  u8g2.drawFrame(0, app_selection * 10, 128, 12);
}

// draw music - here i want to read the sdcard and list the contents to allow for song selection
void draw_music() {

	int visible_count = file_count - file_display_offset;
	if (visible_count > files_per_screen) {
		visible_count = files_per_screen;
	}

	for (int i=0; i<visible_count; i++) {
		u8g2.drawStr(3, 10 + 10 * i, audio_files[file_display_offset + i]);
	}

	if (file_count > 0) {
		int highlight_row = selected_file - file_display_offset;
		u8g2.drawFrame(0, 10 * highlight_row, 128, 12);
	}
}

// not done yet - want to have a music tracker eventually
void draw_tracker() {
  u8g2.drawStr(3, 10, "tracker");
}

// initial setup on reset
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
		//    .dma_buf_len = 64
		.dma_buf_len = 512
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

	if (!SD.begin(5)){ // 5 is the CS pin
    Serial.println("Card Mount Failed");
    return;
  } else {
    Serial.println("card mount success");
  }


	// print files in / of sdcard to serial monitor
	//	list_files();
	load_file_list();

	Serial.println("end setup");
}

// preload the list of audio files on the sdcard
void load_file_list() {

	File root = SD.open("/");

	if (!root) {
		Serial.println("failed to open /");
		return;
	}

	File f = root.openNextFile();
	int i=0;
	while (f && i < MAX_FILES) {

		Serial.print("FILE: ");
		Serial.print(f.name());
		Serial.print(", SIZE: ");
		Serial.println(f.size());

		strcpy(audio_files[i], f.name());
		
		f = root.openNextFile();
		i++;
	}
	Serial.printf("file count: %d\n", i);
	file_count = i;

}

void list_files() {

	File root = SD.open("/");

	if (!root) {
		Serial.println("failed to open /");
		return;
	}

	File f = root.openNextFile();
	int i=0;
	while (f) {
		Serial.print("FILE: ");
		Serial.print(f.name());
		Serial.print(", SIZE: ");
		Serial.println(f.size());
		f = root.openNextFile();
		i++;
	}
	Serial.printf("file count: %d\n", i);
	file_count = i;
}

// start playing raw audio from file on sdcard
void play_audio_file(const char* filename) {

  if (SD.exists(filename)) {
    Serial.println("file exists");
  } else {
    Serial.printf("file %s doesn't exist\n", filename);
    return;
  }
    
  file = SD.open(filename);
  if (file) {
    Serial.println("file open success");
    fileopen = 1;
  } else {
    Serial.println("file open fail");
  }

}

void play_next_audio_file() {

	if (file_count == 0) {
        return;
    }

    selected_file++;

    // Wrap around to the first track
    if (selected_file >= file_count) {
        selected_file = 0;
    }

    play_selected_audio_file();
    need_redraw = 1;
}

void play_selected_audio_file() {

	char file_path[255] = "/";
	strcat(file_path, audio_files[selected_file]);
	strupr(file_path);

	play_audio_file(file_path);
}

// main loop
void loop() {

  int state = !digitalRead(BUTTON_A_PIN);
  if (state != buttonAState) {
    buttonAState = state;
    Serial.printf("buttonAState changed to %d\n", buttonAState);
    if (buttonAState) {
      if (current_app == application_menu) {
				if (app_selection == APP_SELECTION_TETRIS) {
					current_app = application_tetris;
					need_redraw = 1;			
				} else if (app_selection == APP_SELECTION_MUSIC) {
					current_app = application_music;
					need_redraw = 1;			
				} else if (app_selection == APP_SELECTION_TRACKER) {
					current_app = application_tracker;
					need_redraw = 1;			
				}
      } else if (current_app == application_tetris) {
				rotate_active_tetromino();
				need_redraw = 1;			
      } else if (current_app == application_music) {
				// start playing the currently selected raw audio file
				play_selected_audio_file();
			}
    }
  }

  state = !digitalRead(BUTTON_B_PIN);
  if (state != buttonBState) {
    buttonBState = state;
    Serial.printf("buttonBState changed to %d\n", buttonBState);
    if (buttonBState) {
      current_app = application_menu;
			need_redraw = 1;			
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
			need_redraw = 1;			
    } else if (current_app == application_tetris) {
      drop_tetro_fast = buttonDownState;
    } else if (current_app == application_music && buttonDownState) {

			if (file_count > 0 && selected_file < file_count - 1) {
				
				selected_file++;
				// scroll down when selection passes the visible rows
				if (selected_file >= file_display_offset + files_per_screen) {
					file_display_offset = selected_file - files_per_screen + 1;
				}

				need_redraw = 1;			
			}
		}
  }

  state = !digitalRead(BUTTON_UP_PIN);
  if (state != buttonUpState) {
    buttonUpState = state;
    Serial.printf("buttonUpState changed to %d\n", buttonUpState);
    if (current_app == application_menu && buttonUpState) {
      app_selection--;
      if (app_selection < 0) app_selection = 0;
			need_redraw = 1;			
    } else if (current_app == application_music && buttonUpState) {

			 if (selected_file > 0) {
        selected_file--;

        // Scroll up when selection moves above the visible window
        if (selected_file < file_display_offset) {
            file_display_offset = selected_file;
        }

        need_redraw = 1;
			 }
		}
  }

  state = !digitalRead(BUTTON_LEFT_PIN);
  if (state != buttonLeftState) {
    buttonLeftState = state;
    Serial.printf("buttonLeftState changed to %d\n", buttonLeftState);
    if (buttonLeftState && current_app == application_tetris) {
      move_tetromino_left();
			need_redraw = 1;
    }
  }

  state = !digitalRead(BUTTON_RIGHT_PIN);
  if (state != buttonRightState) {
    buttonRightState = state;
    Serial.printf("buttonRightState changed to %d\n", buttonRightState);
    if (buttonRightState && current_app == application_tetris) {
      // move the tetromino to the right if we can
      move_tetromino_right();
			need_redraw = 1;
    }
  }

	// update tetris game state
	if (current_app == application_tetris) {
		need_redraw |= update_tetris();
	}

	if (need_redraw) {

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

		need_redraw = 0;
	}
  
  vTaskDelay(pdMS_TO_TICKS(10));
}
