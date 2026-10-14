#ifndef PLATFORMER_H
#define PLATFORMER_H

// Simple 128x128 U8g2 platformer for the ESP32 game console.
// Controls: LEFT/RIGHT move, A jumps, B returns to the application menu.
// Include after U8g2 is available in the main sketch.

#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>

namespace Platformer {

static const int SCREEN_W = 128;
static const int SCREEN_H = 128;
static const int TILE_W = 8;   // 16 columns visible at a time
static const int TILE_H = 8;   // 16 rows visible at a time
static const int MAP_W = 64;
static const int MAP_H = 18;
static const float PLAYER_W = 0.8f;  // measured in map tiles
static const float PLAYER_H = 0.95f;
static const float GRAVITY = 24.0f;
static const float JUMP_SPEED = -9.0f;
static const float MAX_X_SPEED = 5.0f;
static const float GROUND_ACCEL = 18.0f;
static const float AIR_ACCEL = 9.0f;
static const float GROUND_DRAG = 8.0f;

// Each row must contain exactly MAP_W (64) characters.
static const char level[MAP_W * MAP_H + 1] =
  "................................................................"
  "................................................................"
  "................................................................"
  "................................................................"
  ".......#........................................................"
  "...................#########..........##........................"
  "..................##.................####......................."
  ".................##.................######......................"
  "................##.............................................."
  "#########################################.######...#############"
  "........................................#.#......##............."
  "............................#############.#.....##.............."
  "............................#.............#....##..............."
  "............................#..############...##................"
  "............................#................##................."
  "............................##################.................."
  "................................................................"
  "................................................................";

static float player_x = 1.0f;
static float player_y = 8.0f;
static float velocity_x = 0.0f;
static float velocity_y = 0.0f;
static float camera_x = 0.0f;
static float camera_y = 0.0f;
static bool on_ground = false;
static bool facing_left = false;
static bool previous_jump = false;
static uint32_t previous_update_ms = 0;

inline char tile_at(int x, int y) {
  if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return '#';
  return level[y * MAP_W + x];
}

inline bool solid_at(float x, float y) {
  return tile_at((int)floorf(x), (int)floorf(y)) != '.';
}

inline void init() {
  player_x = 1.0f;
  player_y = 8.0f;
  velocity_x = velocity_y = 0.0f;
  camera_x = camera_y = 0.0f;
  on_ground = false;
  facing_left = false;
  previous_jump = false;
  previous_update_ms = millis();
}

inline void jump() {
  if (on_ground) {
    velocity_y = JUMP_SPEED;
    on_ground = false;
  }
}

// Call continuously from loop(). left/right are held-button states;
// jump_held is the current A-button state (active/high means pressed).
inline void update(bool left, bool right, bool jump_held) {
  uint32_t now = millis();
  float dt = (now - previous_update_ms) / 1000.0f;
  previous_update_ms = now;
  // Avoid a large physics step after returning from another app or a pause.
  if (dt > 0.05f) dt = 0.05f;
  if (dt < 0.0f) dt = 0.0f;

  if (jump_held && !previous_jump) jump();
  previous_jump = jump_held;

  float accel = on_ground ? GROUND_ACCEL : AIR_ACCEL;
  if (left) {
    velocity_x -= accel * dt;
    facing_left = true;
  }
  if (right) {
    velocity_x += accel * dt;
    facing_left = false;
  }
  if (!left && !right && on_ground) {
    float drag = GROUND_DRAG * dt;
    if (velocity_x > 0.0f) velocity_x = fmaxf(0.0f, velocity_x - drag);
    else velocity_x = fminf(0.0f, velocity_x + drag);
  }
  if (velocity_x > MAX_X_SPEED) velocity_x = MAX_X_SPEED;
  if (velocity_x < -MAX_X_SPEED) velocity_x = -MAX_X_SPEED;

  velocity_y += GRAVITY * dt;
  if (velocity_y > 10.0f) velocity_y = 10.0f;

  // Horizontal collision. Player position is the top-left corner in tile units.
  float next_x = player_x + velocity_x * dt;
  if (velocity_x > 0.0f) {
    if (solid_at(next_x + PLAYER_W, player_y + 0.05f) ||
        solid_at(next_x + PLAYER_W, player_y + PLAYER_H - 0.05f)) {
      next_x = floorf(next_x + PLAYER_W) - PLAYER_W;
      velocity_x = 0.0f;
    }
  } else if (velocity_x < 0.0f) {
    if (solid_at(next_x, player_y + 0.05f) ||
        solid_at(next_x, player_y + PLAYER_H - 0.05f)) {
      next_x = floorf(next_x) + 1.0f;
      velocity_x = 0.0f;
    }
  }
  player_x = next_x;

  // Vertical collision.
  float next_y = player_y + velocity_y * dt;
  on_ground = false;
  if (velocity_y > 0.0f) {
    if (solid_at(player_x + 0.05f, next_y + PLAYER_H) ||
        solid_at(player_x + PLAYER_W - 0.05f, next_y + PLAYER_H)) {
      // Place the player's feet exactly on top of the solid tile.
      // Subtract PLAYER_H because player_y is the top of the player.
      next_y = floorf(next_y + PLAYER_H) - PLAYER_H;
      velocity_y = 0.0f;
      on_ground = true;
    }
  } else if (velocity_y < 0.0f) {
    if (solid_at(player_x + 0.05f, next_y) ||
        solid_at(player_x + PLAYER_W - 0.05f, next_y)) {
      next_y = floorf(next_y) + 1.0f;
      velocity_y = 0.0f;
    }
  }
  player_y = next_y;

  if (player_x < 0.0f) { player_x = 0.0f; velocity_x = 0.0f; }
  if (player_x > MAP_W - PLAYER_W) { player_x = MAP_W - PLAYER_W; velocity_x = 0.0f; }
  if (player_y < 0.0f) { player_y = 0.0f; velocity_y = 0.0f; }
  if (player_y > MAP_H - PLAYER_H) {
    player_y = MAP_H - PLAYER_H;
    velocity_y = 0.0f;
    on_ground = true;
  }

  // Keep player near screen center while clamping camera to level bounds.
  const float view_tiles_x = (float)SCREEN_W / TILE_W;
  const float view_tiles_y = (float)SCREEN_H / TILE_H;
  camera_x = player_x + PLAYER_W * 0.5f - view_tiles_x * 0.5f;
  camera_y = player_y + PLAYER_H * 0.5f - view_tiles_y * 0.5f;
  if (camera_x < 0.0f) camera_x = 0.0f;
  if (camera_y < 0.0f) camera_y = 0.0f;
  if (camera_x > MAP_W - view_tiles_x) camera_x = MAP_W - view_tiles_x;
  if (camera_y > MAP_H - view_tiles_y) camera_y = MAP_H - view_tiles_y;
}

// Monochrome fallback art: blocks are filled with a small highlight pattern,
// and the player is a filled sprite-like rectangle. No SD images are required.
inline void draw(U8G2 &display) {
  int map_left = (int)floorf(camera_x);
  int map_top = (int)floorf(camera_y);
  int sub_x = (int)((camera_x - map_left) * TILE_W);
  int sub_y = (int)((camera_y - map_top) * TILE_H);

  for (int row = 0; row <= SCREEN_H / TILE_H; ++row) {
    for (int col = 0; col <= SCREEN_W / TILE_W; ++col) {
      if (tile_at(map_left + col, map_top + row) != '#') continue;
      int px = col * TILE_W - sub_x;
      int py = row * TILE_H - sub_y;
      // Clip naturally to the 128x128 display using the visible part of each tile.
      int x0 = px < 0 ? 0 : px;
      int y0 = py < 0 ? 0 : py;
      int x1 = px + TILE_W > SCREEN_W ? SCREEN_W : px + TILE_W;
      int y1 = py + TILE_H > SCREEN_H ? SCREEN_H : py + TILE_H;
      if (x1 <= x0 || y1 <= y0) continue;
      display.drawBox(x0, y0, x1 - x0, y1 - y0);
      if (x1 - x0 >= 4 && y1 - y0 >= 4) {
        display.setDrawColor(0);
        display.drawPixel(x0 + 1, y0 + 1);
        display.setDrawColor(1);
      }
    }
  }

  int px = (int)roundf((player_x - camera_x) * TILE_W);
  int py = (int)roundf((player_y - camera_y) * TILE_H);
  int pw = (int)(PLAYER_W * TILE_W);
  int ph = (int)(PLAYER_H * TILE_H);
  // Simple player with a one-pixel outline and a facing-direction eye.
  display.drawBox(px, py, pw, ph);
  display.setDrawColor(0);
  display.drawPixel(facing_left ? px + 1 : px + pw - 2, py + 2);
  display.setDrawColor(1);
}

} // namespace Platformer

#endif // PLATFORMER_H
