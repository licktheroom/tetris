// headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// defines
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 600
#define SDL_MAIN_HANDLED

// SDL headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// struct
typedef struct
{
	SDL_Window *window;
	SDL_Renderer *renderer;
}App;

// statics
static App app;

static int board[200][3] = { 0 };
static int cubes;
static int player[4][3] = { 0 };
static int ghost[4][2] = { 0 };
static int rotation;
static int score;

static float speed;

static SDL_Texture *BG;
static SDL_Rect rBG;
static SDL_Texture *tetrominos[7];
static SDL_Texture *g_tetrominos[7];

// pre-def functions
void clean_up();
void report_error();
void load_texture(const char *path, SDL_Texture **target);
void get_shape();
void get_ghost();
void get_rotation();
void get_cleared_lines();
void send_player_to_board();
void move_down();
void init();
void input();
void render();

// MAIN
int main()
{
	init();

	printf("I do not own Tetris nor any of it's assets. All rights reserved to The Tetris Company LLC. On a side note, colors were chosen purely off memory.\n");

	clock_t last_moved_frame = clock();

	// main loop

	while(1)
	{
		// get current frame, then see if the correct time has passed
		// if it has, then move the player's peice down
		clock_t current_frame = clock();

		double delta = ((double) current_frame-last_moved_frame) / CLOCKS_PER_SEC;

		if(delta >= speed)
		{
			last_moved_frame = current_frame;
			move_down();
		}

		// do everything else
		input();

		render();
	}
}

// functions
// delete all data and exit
void clean_up()
{
	// i believe this is mostly self explainatory
	SDL_DestroyRenderer(app.renderer);
	SDL_DestroyWindow(app.window);

	// destroy texures, they're also set to NULL to be safe.
	SDL_DestroyTexture(BG);
	BG = NULL;

	for(int i = 0; i < 7; i++)
	{
		SDL_DestroyTexture(tetrominos[i]);
		tetrominos[i] = NULL;

		SDL_DestroyTexture(g_tetrominos[i]);
		g_tetrominos[i] = NULL;
	}

	// finalize

	IMG_Quit();

	SDL_Quit();

	exit(0);
}

// get error and print
void report_error()
{
	printf("%s\n", SDL_GetError());

	clean_up();
}

// load surface then send it to a texture
void load_texture(const char *path, SDL_Texture **target)
{
	SDL_Surface *tmp;
	tmp = IMG_Load(path);

	*target = SDL_CreateTextureFromSurface(app.renderer, tmp);

	SDL_FreeSurface(tmp);
	tmp = NULL;
}

// get the current tetromino's shape and feed it to the player
void get_shape()
{
	if(player[0][2] == 1) // L tetromino
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0]-1;
		player[2][1] = player[0][1];

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 2) // J tetromino
	{
		player[1][0] = player[0][0]-1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0]+1;
		player[2][1] = player[0][1];

		player[3][0] = player[0][0]+1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 3) // O tetromino
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]+1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 4) // I tetromino
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0]+2;
		player[2][1] = player[0][1];

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1];
	} else if(player[0][2] == 5) // T tetromino
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1];
	} else if(player[0][2] == 6) // Z tetromino
	{
		player[1][0] = player[0][0]-1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]+1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 7) // S tetromino
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1]+1;
	}
}

// get's where the ghost should be
// also used for the instant drop
// this is only used when the player moves down/left/right
void get_ghost()
{
	// set ghost to current player position
	for(int i = 0; i < 4; i++)
	{
		ghost[i][0] = player[i][0];
		ghost[i][1] = player[i][1];
	}

	int col = 0;

	while(!col) // while not collided
	{
		for(int i = 0; i < 4; i++)
		{
			if(!col)
			{
				if(ghost[i][1] == 19) // detects if ghost has reached the bottom
					col = 1;
				else
				{
					for(int d = 0; d < cubes; d++) // detects if ghost has reached a tetromino
					{
						if(ghost[i][0] == board[d][0] && ghost[i][1]+1 == board[d][1])
							col = 1;
					}
				}
			}
		}

		if(!col) // if not collided, move down one
		{
			for(int i = 0; i < 4; i++)
				ghost[i][1]++;
		}
	}
}

// an annoying way to get rotation
// can't think of a better way
// from 0 it goes clockwise
// the O tetromino is skipped because no matter what it's the same
void get_rotation()
{
	if(player[0][2] != 3)
	{
		int pre_values[3][2] = { 0 };

		if(player[0][2] == 1) // L brick
		{
			if(rotation == 0)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rotation == 1)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]+1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]-1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]-1;
			} else if(rotation == 2)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]+1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]-1;
			} else if(rotation == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]+1;
			}
		} else if(player[0][2] == 2) // J brick
		{
			if(rotation == 0)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]+1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rotation == 1)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rotation == 2)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]-1;
			} else if(rotation == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]+1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]-1;

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]-1;
			}
		} else if(player[0][2] == 4) // I brick
		{
			if(rotation == 0 || rotation == 2)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]+2;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1];
			} else if(rotation == 1 || rotation == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]+1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+2;

				pre_values[2][0] = player[0][0];
				pre_values[2][1] = player[0][1]-1;
			}
		} else if(player[0][2] == 5) // T brick
		{
			if(rotation == 0)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1];
			} else if(rotation == 1)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]+1;

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0];
				pre_values[2][1] = player[0][1]-1;
			} else if(rotation == 2)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]-1;

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1];
			} else if(rotation == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0]+1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0];
				pre_values[2][1] = player[0][1]+1;
			}
		} else if(player[0][2] == 6) // Z brick
		{
			if(rotation == 0 || rotation == 2)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rotation == 1 || rotation == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			}
		} else if(player[0][2] == 7) // S brick
		{
			if(rotation == 0 || rotation == 2)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rotation == 1 || rotation == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0]+1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]+1;
			}
		}

		int can_rotate = 1;

		for(int i = 0; i < 3; i++) // checks to see if the rotation is outside the board
		{
			if(pre_values[i][0] == -1 || pre_values[i][0] == 10 || pre_values[i][1] == -1 || pre_values[i][1] == 20)
				can_rotate = 0;
		}

		if(can_rotate)
		{
			for(int i = 0; i < 3; i++)
			{
				if(can_rotate) // all of this checks for collision with tetrominos
				{
					for(int d = 0; d < cubes; d++)
					{
						if(pre_values[i][0] == board[d][0] && pre_values[i][1] == board[d][1])
							can_rotate = 0;
					}
				}
			}
		}

		if(can_rotate) // if all is good, allow the rotation
		{
			for(int i = 0; i < 3; i++)
			{
				player[i+1][0] = pre_values[i][0];
				player[i+1][1] = pre_values[i][1];
			}

			get_ghost();
		}
	}
}

// sees if there are any cleared lines
// if there is, delete all tetrominos on that line and move all tetrominos above down
void get_cleared_lines()
{
	int cleared[20] = { 0 };

	for(int i = 0; i < cubes; i++) // goes through all tetrominos and counts what row they're in
		cleared[board[i][1]]++;

	for(int i = 0; i < 20; i++)
	{
		if(cleared[i] == 10) // if line is full, do things
		{
			score++;

			if(score == 5 && speed != 0.01) // see if the game should be sped up
			{
				speed -= 0.01;
				score = 0;
			}

			int move = 0;

			for(int d = 0; d < cubes; d++) // goes through and delete the tetrominos on the current row, it also shortens the board so nothing errors
			{
				if(board[d][1] == i)
				{
					board[d][0] = 0;
					board[d][1] = 0;
					board[d][2] = 0;

					move++;
				} else if(move > 0)
				{
					board[d-move][0] = board[d][0];
					board[d-move][1] = board[d][1];
					board[d-move][2] = board[d][2];
				}
			}

			cubes -= move;

			for(int d = 0; d < cubes; d++) // move down the tetrominos above the current row
			{
				if(board[d][1] < i)
					board[d][1]++;
			}
		}
	}
}

// makes current player an entity on the board
void send_player_to_board()
{
	for(int i = 0; i < 4; i++)
	{
		board[cubes][0] = player[i][0];
		board[cubes][1] = player[i][1];
		board[cubes][2] = player[0][2];

		cubes++;
	}

	player[0][0] = 4;
	player[0][1] = 0;
	player[0][2] = rand()%7;
	player[0][2]++;

	rotation = 0;

	get_cleared_lines();

	get_shape();

	get_ghost();
}

// calculates downward movement
void move_down()
{
	int col = 0;

	for(int i = 0; i < 4; i++)
	{
		if(col != 1 && player[i][1] == 19) // checks to see if player has reached the bottom
		{
			col = 1;
			send_player_to_board();
		} else if(col != 1 && cubes > 0)
		{
			for(int d = 0; d < cubes; d++)
			{
				if(player[i][0] == board[d][0] && player[i][1]+1 == board[d][1]) // checks to see if the player had hit another tetromino
				{
					col = 1;
					send_player_to_board();
				}
			}
		}
	}

	if(!col) // why doesn't a for loop work
	{
		player[0][1]++;
		player[1][1]++;
		player[2][1]++;
		player[3][1]++;
		get_ghost();
	}
}

//initialize the game
void init()
{
	SDL_SetMainReady();

	if(SDL_Init(SDL_INIT_VIDEO) != 0)
		report_error();

	app.window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if(!app.window)
		report_error();

	app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
	if(!app.renderer)
		report_error();

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_ADD); // this ensures the ghosts work properly, it doesn't need to be fancy

	// load textures, there really isn't a better way to do this
	IMG_Init(IMG_INIT_PNG);

	load_texture("textures/BG.png", &BG);

	load_texture("textures/L_brick.png", &tetrominos[0]);
	load_texture("textures/rL_brick.png", &tetrominos[1]);
	load_texture("textures/s_brick.png", &tetrominos[2]);
	load_texture("textures/st_brick.png", &tetrominos[3]);
	load_texture("textures/t_brick.png", &tetrominos[4]);
	load_texture("textures/Z_brick.png", &tetrominos[5]);
	load_texture("textures/rZ_brick.png", &tetrominos[6]);

	load_texture("textures/gL_brick.png", &g_tetrominos[0]);
	load_texture("textures/grL_brick.png", &g_tetrominos[1]);
	load_texture("textures/gs_brick.png", &g_tetrominos[2]);
	load_texture("textures/gst_brick.png", &g_tetrominos[3]);
	load_texture("textures/gt_brick.png", &g_tetrominos[4]);
	load_texture("textures/gZ_brick.png", &g_tetrominos[5]);
	load_texture("textures/grZ_brick.png", &g_tetrominos[6]);

	// set up the BG rect, only needs to be done once because it's static
	rBG.x = 0;
	rBG.y = 0;

	SDL_QueryTexture(BG, NULL, NULL, &rBG.w, &rBG.h);

	// finish up
	srand(time(NULL));

	rotation = 0;
	score = 0;
	speed = 0.1;

	player[0][0] = 4;
	player[0][1] = 0;
	player[0][2] = rand()%7;
	player[0][2]++;

	get_shape();
	get_ghost();

	cubes = 0;

	printf("Game loaded.\n");
}

// handles user input
void input()
{
	SDL_Event event;

	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				clean_up();
				break;

			case SDL_KEYDOWN:
				//SDL_Scancode key = event.key.keysym.scancode;

				if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) // quick leave
					clean_up();
				else if(event.key.keysym.scancode == SDL_SCANCODE_LEFT) // calc movement left
				{
					int blocked = 0;
					for(int i = 0; i < 4; i++)
					{
						if(player[i][0] == 0)
							blocked = 1;
						else if(!blocked)
						{
							for(int d = 0; d < cubes; d++)
							{
								if(player[i][1] == board[d][1] && player[i][0]-1 == board[d][0])
									blocked = 1;
							}
						}
					}

					if(!blocked)
					{
						for(int i = 0; i < 4; i++)
							player[i][0]--;
						get_ghost();
					}
				} else if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT) // calc movement right
				{
					int blocked = 0;
					for(int i = 0; i < 4; i++)
					{
						if(player[i][0] == 9)
							blocked = 1;
						else if(!blocked)
						{
							for(int d = 0; d < cubes; d++)
							{
								if(player[i][1] == board[d][1] && player[i][0]+1 == board[d][0])
									blocked = 1;
							}
						}
					}

					if(!blocked)
					{
						for(int i = 0; i < 4; i++)
							player[i][0]++;
						get_ghost();
					}
				} else if(event.key.keysym.scancode == SDL_SCANCODE_UP) // simple rotation
				{
					rotation++;

					if(rotation == 4)
						rotation = 0;
					get_rotation();
				} else if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) // quick drop
				{
					for(int i = 0; i < 4; i++)
					{
						player[i][0] = ghost[i][0];
						player[i][1] = ghost[i][1];
					}
					send_player_to_board();
				}

				break;

			default:
				break;
		}
	}
}


// render everything, it is required to see things
void render()
{
	SDL_SetRenderDrawColor(app.renderer, 0 ,0 ,0, 255);
	SDL_RenderClear(app.renderer);

	SDL_RenderCopy(app.renderer, BG, NULL, &rBG);

	if(cubes > 0)
	{
		for(int i = 0; i < cubes; i++) // renders already placed tetrominos
		{
			SDL_Rect rCube;
			rCube.x = 97 + (30*board[i][0]);
			rCube.y = 30*board[i][1];

			SDL_QueryTexture(tetrominos[board[i][2]-1], NULL, NULL, &rCube.w, &rCube.h);
			SDL_RenderCopy(app.renderer, tetrominos[board[i][2]-1], NULL, &rCube);
		}
	}

	for(int i = 0; i < 4; i++) // renders the player
	{
		SDL_Rect rPlayer;
		rPlayer.x = 97 + (30*player[i][0]);
		rPlayer.y = 30*player[i][1];

		SDL_QueryTexture(tetrominos[player[0][2]-1], NULL, NULL, &rPlayer.w, &rPlayer.h);
		SDL_RenderCopy(app.renderer, tetrominos[player[0][2]-1], NULL, &rPlayer);
	}

	for(int i = 0; i < 4; i++) // renders the ghost
	{
		SDL_Rect rGhost;
		rGhost.x = 97 + (30*ghost[i][0]);
		rGhost.y = 30*ghost[i][1];

		SDL_QueryTexture(g_tetrominos[player[0][2]-1], NULL, NULL, &rGhost.w, &rGhost.h);
		SDL_RenderCopy(app.renderer, g_tetrominos[player[0][2]-1], NULL, &rGhost);
	}

	SDL_RenderPresent(app.renderer);
}
