#pragma once

int tetris_score = 0;

// tetromino stuff
#define TETROMINO_SIZE 4
#define TETROMINO_BLOCK_COUNT (TETROMINO_SIZE * TETROMINO_SIZE)
#define TETROMINO_TYPE_COUNT 7
#define TETROMINO_ROTATION_COUNT 4

int active_tetromino_type = 0;

int does_tetromino_fit(int tetromino_type, int tetromino_rotation, int pos_x, int pos_y);

const char tetromino[TETROMINO_TYPE_COUNT][TETROMINO_ROTATION_COUNT][TETROMINO_BLOCK_COUNT] = { // start types
  { // start rotations
    { // start blocks
      0,1,1,1,
      0,1,0,0,
      0,1,0,0,
      0,1,0,0
    },
    {
      0,0,0,0,
      1,1,1,1,
      0,0,0,1,
      0,0,0,1
    },
    {
      0,0,1,0,
      0,0,1,0,
      0,0,1,0,
      1,1,1,0
    },
    {
      1,0,0,0,
      1,0,0,0,
      1,1,1,1,
      0,0,0,0
    }
  },
  {
    {
      0,1,0,0,
      0,1,0,0,
      0,1,0,0,
      0,1,0,0
    },
    {
      0,0,0,0,
      1,1,1,1,
      0,0,0,0,
      0,0,0,0
    },
    {
      0,0,1,0,
      0,0,1,0,
      0,0,1,0,
      0,0,1,0
    },
    {
      0,0,0,0,
      0,0,0,0,
      1,1,1,1,
      0,0,0,0
    }
  }, 
  {
    {
      0,0,0,0,
      0,1,1,0,
      0,1,1,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,1,1,0,
      0,1,1,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,1,1,0,
      0,1,1,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,1,1,0,
      0,1,1,0,
      0,0,0,0
    }
  },
  {
    {
      0,1,0,0,
      0,1,1,0,
      0,1,0,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,1,1,1,
      0,0,1,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,0,1,0,
      0,1,1,0,
      0,0,1,0
    },
    {
      0,0,0,0,
      0,1,0,0,
      1,1,1,0,
      0,0,0,0
    }
  },
  {
    {
      0,0,1,0,
      0,1,1,0,
      0,1,0,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,1,1,0,
      0,0,1,1,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,0,1,0,
      0,1,1,0,
      0,1,0,0
    },
    {
      0,0,0,0,
      1,1,0,0,
      0,1,1,0,
      0,0,0,0
    }
  },
  {
    {
      1,1,1,0,
      0,0,1,0,
      0,0,1,0,
      0,0,1,0
    },
    {
      0,0,0,1,
      0,0,0,1,
      1,1,1,1,
      0,0,0,0
    },
    {
      0,1,0,0,
      0,1,0,0,
      0,1,0,0,
      0,1,1,1
    },
    {
      0,0,0,0,
      1,1,1,1,
      1,0,0,0,
      1,0,0,0
    }
  }, 
  {
    {
      0,1,0,0,
      0,1,1,0,
      0,0,1,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,0,1,1,
      0,1,1,0,
      0,0,0,0
    },
    {
      0,0,0,0,
      0,1,0,0,
      0,1,1,0,
      0,0,1,0
    },
    {
      0,0,0,0,
      0,1,1,0,
      1,1,0,0,
      0,0,0,0
    }
  }  
};

#define INITIAL_TETROMINO_X 0

int active_tetromino_x = INITIAL_TETROMINO_X;
int active_tetromino_y = 0;
int active_tetromino_rotation = 0;
int drop_tetro_fast = 0;

int tetris_frame = 0;
int tetris_speed = 20;

#define TETRIS_FIELD_COLS 10
#define TETRIS_FIELD_ROWS 24
char tetris_field[TETRIS_FIELD_COLS * TETRIS_FIELD_ROWS] = {
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,  
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,1,0,0,0,0,0,0,0,0,
  1,1,1,0,0,0,0,0,0,0
};

void rotate_active_tetromino() {

  int new_rotation = (active_tetromino_rotation + 1) % 4;

  int can_rotate = does_tetromino_fit(active_tetromino_type, new_rotation, active_tetromino_x, active_tetromino_y);

  if (can_rotate) {
    active_tetromino_rotation = new_rotation;
  }
}

int active_tetromino_occupies_block(int row, int col) {

  if (row >= active_tetromino_y && row < active_tetromino_y + TETROMINO_SIZE &&
      col >= active_tetromino_x && col < active_tetromino_x + TETROMINO_SIZE) {

    int aty = row - active_tetromino_y;
    int atx = col - active_tetromino_x;

    if (tetromino[active_tetromino_type][active_tetromino_rotation][aty*TETROMINO_SIZE+atx]) {
      return 1;
    }
  }

  return 0;
}

// find out if a tetromino fits in a given location
int does_tetromino_fit(int tetromino_type, int tetromino_rotation, int pos_x, int pos_y) {

  int tetromino_fits = 1;

  for (int x=0; x<TETROMINO_SIZE; x++) {
    for (int y=0; y<TETROMINO_SIZE; y++) {

      int block_index = y * TETROMINO_SIZE + x;
      int block_is_there = 0;

      if (tetromino[tetromino_type][tetromino_rotation][block_index]) {
	block_is_there = 1;
      }

      if (block_is_there) {

	int block_x = pos_x + x;
	int block_y = pos_y + y;

	// check if we're off the playfield in the y direction
	if (block_y > TETRIS_FIELD_ROWS - 1) {
	  tetromino_fits = 0;
	}

	// check if there's any old blocks under us
	if (tetris_field[block_y * TETRIS_FIELD_COLS + block_x]) {
	  tetromino_fits = 0;
	}

	// check if we're off the left edge
	if (block_x < 0) {
	  tetromino_fits = 0;
	}

	// check if we're off the right edge
	if (block_x > TETRIS_FIELD_COLS - 1) {
	  tetromino_fits = 0;
	}
      }
    }
  }

  return tetromino_fits;
}

// need to check if there's any collisions for the falling tetromino here
// and return 0 if there was one, otherwise advance the active tetromino down
// no longer used
int advance_active_tetromino() {

  // need to check if anything is under the falling tetromino
  // including the bottom of the play field.

  // assume it can fall by default unless something's blocking it
  int tetro_can_fall = 1;

  // check under every single active block
  for (int x=0; x<TETROMINO_SIZE; x++) {
    for (int y=0; y<TETROMINO_SIZE; y++) {

      int block_index = y*TETROMINO_SIZE + x;
      int block_is_there = 0;

      if (tetromino[active_tetromino_type][active_tetromino_rotation][block_index]) {
	block_is_there = 1;
      }

      if (block_is_there) {

	// we have a block, need to check if anything's under it
	int block_x = active_tetromino_x + x;
	int block_y = active_tetromino_y + y;

	// first check if the bottom of the field is under it
	if (block_y >= TETRIS_FIELD_ROWS - 1) {
	  // found a block touching the bottom, we can't drop any more
	  tetro_can_fall = 0;
	}

	// now check if there's any active block under this block in the field
	int block_under_y = block_y + 1;
	if (tetris_field[block_under_y * TETRIS_FIELD_COLS + block_x]) {
	  // there is a block under, we can't move down
	  tetro_can_fall = 0;
	}
      }
    }
  }

  // advance tetromino
  if (tetro_can_fall)
    active_tetromino_y++;

  return tetro_can_fall;
}

// move the active tetromino to the tetris playfield
void freeze_active_tetromino() {

  for (int x=0; x<TETROMINO_SIZE; x++) {

    for (int y=0; y<TETROMINO_SIZE; y++) {

      int field_x = active_tetromino_x + x;
      int field_y = active_tetromino_y + y;

      int block_is_present = 0;

      block_is_present = tetromino[active_tetromino_type][active_tetromino_rotation][y * TETROMINO_SIZE + x];

      if (block_is_present) {
	tetris_field[field_y * TETRIS_FIELD_COLS + field_x] = 1;
      }
    }
  }
}

// 'create' the new active tetromino
void spawn_active_tetromino() {

  active_tetromino_x = INITIAL_TETROMINO_X;
  active_tetromino_y = 0;
  active_tetromino_rotation = 0;

  active_tetromino_type = random(TETROMINO_TYPE_COUNT - 1);
}

void delete_row(int r) {

  for (int i=r; i>=1; i--) {
    for (int c=0; c<10; c++) {
      tetris_field[i * TETRIS_FIELD_COLS + c] = tetris_field[(i-1) * TETRIS_FIELD_COLS + c];
    }
  }
}

// not used
void remove_tetrised_rows(int *rows, int row_count) {

  for (int i=0; i<row_count; i++) {

    int r = rows[i];

    delete_row(r);
  }
}

// check for complete tetris lines and update the score / field
void check_tetris() {

  int tetrised_rows[20];
  int tetrised_row_count = 0;

  for (int r=0; r<24; r++) {

    int row_tetrised = 1;
    
    for (int c=0; c<10; c++) {
      if (tetris_field[r * TETRIS_FIELD_COLS + c] == 0) {
	row_tetrised = 0;
      }
    }

    if (row_tetrised) {
      tetrised_rows[tetrised_row_count++] = r;
      tetris_score++;
    }
  }

  remove_tetrised_rows(tetrised_rows, tetrised_row_count);
}

// return 1 if we need to update the screen or 0
int update_tetris() {

  int ret = 0;
  int framemod = tetris_speed;
  if (drop_tetro_fast) framemod = 1;

  if (tetris_frame % framemod == 0) {

    // move the active tetromino down the screen if possible
    //    int able_to_move = advance_active_tetromino();

    // get the new y coordinate for the falling tetromino
    int new_tetromino_y = active_tetromino_y + 1;

    // check if the active tetromino fits in the new location
    int tetromino_fits = does_tetromino_fit(active_tetromino_type,
					    active_tetromino_rotation,
					    active_tetromino_x,
					    new_tetromino_y);

    // if it doesn't, freeze it to the background and spawn a new tetromino
    if (!tetromino_fits) {

      // tetromino becomes part of background if it can't fall
      freeze_active_tetromino();

      // need to check for tetris completion
      check_tetris();
      
      spawn_active_tetromino();
      
    } else {
      // if it fits update the active tetromino to the new location
      active_tetromino_y = new_tetromino_y;
    }

    // something changed, need redraw
    ret = 1;
  }

  tetris_frame++;
  return ret;
}

void move_tetromino_left() {

  // find out if we can move the tetromino to the left
  //  int can_move_left = 1;
  int can_move_left = does_tetromino_fit(active_tetromino_type, active_tetromino_rotation, active_tetromino_x-1, active_tetromino_y);

  /*  for (int x=0; x<TETROMINO_SIZE; x++) {
    for (int y=0; y<TETROMINO_SIZE; y++) {

      int block_index = y * TETROMINO_SIZE + x;
      int block_is_there = 0;

      if (tetromino[active_tetromino_type][active_tetromino_rotation][block_index]) {
	block_is_there = 1;
      }

      if (block_is_there) {

	int block_x = active_tetromino_x + x;
	int block_y = active_tetromino_y + y;

	if (block_x <= 0) {
	  can_move_left = 0;
	}
      }
    }
    } */

  if (can_move_left) {
    active_tetromino_x--;
  }
}

void move_tetromino_right() {

  // find out if we can move the tetromino to the right
  //  int can_move_right = 1;
  int can_move_right = does_tetromino_fit(active_tetromino_type, active_tetromino_rotation, active_tetromino_x+1, active_tetromino_y);

  /*  for (int x=0; x<TETROMINO_SIZE; x++) {
    for (int y=0; y<TETROMINO_SIZE; y++) {

      int block_index = y * TETROMINO_SIZE + x;
      int block_is_there = 0;

      if (tetromino[active_tetromino_type][active_tetromino_rotation][block_index]) {
	block_is_there = 1;
      }

      if (block_is_there) {

	int block_x = active_tetromino_x + x;
	int block_y = active_tetromino_y + y;

	if (block_x >= TETRIS_FIELD_COLS - 1) {
	  can_move_right = 0;
	}
      }
    }
    } */

  if (can_move_right) {
    active_tetromino_x++;
  }
}
