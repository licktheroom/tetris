//headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//defines
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 600
#define SQAURE_SIZE 30

#define SDL_MAIN_HANDLED

//SDL headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

//struct
typedef struct
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int state;
}App;

//statics
static App app;

static int pos[200][3] = { 0 };
static int player[4][3] = { 0 };
static int entities;
static int rot;
static int line;

static float speed;

static SDL_Texture *BG;
static SDL_Rect rBG;
static SDL_Texture *cubes[7];

//pre-def functions
void clean_up();
void report_error();
void load_texture(const char *path, SDL_Texture **target);
void calc_shape();
void init();
void rotate();
void input();
void render();
void lines();
void send_player();
void calc_move();

//----main----//
int main()
{
	init();

	printf("I do not own Tetris nor any of it's assets. All rights reserved to The Tetris Company LLC. On a side note, colors were chosen purely off memory.");
	
	clock_t last_time = clock();

	while(1)
	{
		//movement
		clock_t current = clock();

		double delta = ((double) current-last_time) / CLOCKS_PER_SEC;

		if(delta >= 0.1)
		{
			last_time = current;
			calc_move();
		}

		//rest
		input();

		render();
	}

	clean_up();
}

//functions
//delete all data and exit
void clean_up()
{
	SDL_DestroyRenderer(app.renderer);
	SDL_DestroyWindow(app.window);

	SDL_DestroyTexture(BG);
	BG = NULL;

	for(int i = 0; i < 7; i++)
	{
		SDL_DestroyTexture(cubes[i]);
		cubes[i] = NULL;
	}

	IMG_Quit();

	SDL_Quit();

	exit(0);
}

//get errors, usefull
void report_error()
{
	printf("%s\n", SDL_GetError());

	clean_up();
}

//loads textures for me
void load_texture(const char *path, SDL_Texture **target)
{
	SDL_Surface *tmp;
	tmp = IMG_Load(path);
	*target = SDL_CreateTextureFromSurface(app.renderer, tmp);
	SDL_FreeSurface(tmp);
	tmp = NULL;
}

//calculate shape positions
void calc_shape()
{
	if(player[0][2] == 1)
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0]-1;
		player[2][1] = player[0][1];

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 2)
	{
		player[1][0] = player[0][0]-1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0]+1;
		player[2][1] = player[0][1];

		player[3][0] = player[0][0]+1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 3)
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]+1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 4)
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0]+2;
		player[2][1] = player[0][1];

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1];
	} else if(player[0][2] == 5)
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1];
	} else if(player[0][2] == 6)
	{
		player[1][0] = player[0][0]-1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]+1;
		player[3][1] = player[0][1]+1;
	} else if(player[0][2] == 7)
	{
		player[1][0] = player[0][0]+1;
		player[1][1] = player[0][1];

		player[2][0] = player[0][0];
		player[2][1] = player[0][1]+1;

		player[3][0] = player[0][0]-1;
		player[3][1] = player[0][1]+1;
	}
}

//base set-up and starting of systems
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

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_ADD);

	//textures
	IMG_Init(IMG_INIT_PNG);

	load_texture("textures/BG.png", &BG);
	load_texture("textures/L_brick.png", &cubes[0]);
	load_texture("textures/rL_brick.png", &cubes[1]);
	load_texture("textures/s_brick.png", &cubes[2]);
	load_texture("textures/st_brick.png", &cubes[3]);
	load_texture("textures/t_brick.png", &cubes[4]);
	load_texture("textures/Z_brick.png", &cubes[5]);
	load_texture("textures/rZ_brick.png", &cubes[6]);

	//rectangles
	rBG.x = 0;
	rBG.y = 0;

	SDL_QueryTexture(BG, NULL, NULL, &rBG.w, &rBG.h);
	//others
	srand(time(NULL));

	rot = 0;
	line = 0;
	speed = 0.1;

	player[0][0] = 4;
	player[0][1] = 0;
	player[0][2] = rand()%7;
	player[0][2]++;

	calc_shape();

	entities = 0;

	printf("Game loaded.\n");
}

// calculates rotation, not a great way but it works
// this is not very fun to do
// but i can't think of a better way
// from 0 it goes clockwise
void rotate()
{
	if(player[0][2] != 3)
		{
		int can_rotate = 1;
		int pre_values[3][2] = { 0 };

		if(player[0][2] == 1) // L brick
		{
			if(rot == 0)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rot == 1)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]+1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]-1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]-1;
			} else if(rot == 2)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]+1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]-1;
			} else if(rot == 3)
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
			if(rot == 0)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]+1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rot == 1)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rot == 2)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]-1;
			} else if(rot == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]+1;

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]-1;

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]-1;
			}
		} else if(player[0][2] == 4) // straight brick, yes i skipped the square one
		{
			if(rot == 0 || rot == 2)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0]+2;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1];
			} else if(rot == 1 || rot == 3)
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
			if(rot == 0)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1];
			} else if(rot == 1)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]+1;

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0];
				pre_values[2][1] = player[0][1]-1;
			} else if(rot == 2)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]-1;

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1];
			} else if(rot == 3)
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
			if(rot == 0 || rot == 2)
			{
				pre_values[0][0] = player[0][0]-1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rot == 1 || rot == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0]-1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			}
		} else if(player[0][2] == 7) // reverse Z brick, is it called the S brick?
		{
			if(rot == 0 || rot == 2)
			{
				pre_values[0][0] = player[0][0]+1;
				pre_values[0][1] = player[0][1];

				pre_values[1][0] = player[0][0];
				pre_values[1][1] = player[0][1]+1;

				pre_values[2][0] = player[0][0]-1;
				pre_values[2][1] = player[0][1]+1;
			} else if(rot == 1 || rot == 3)
			{
				pre_values[0][0] = player[0][0];
				pre_values[0][1] = player[0][1]-1;

				pre_values[1][0] = player[0][0]+1;
				pre_values[1][1] = player[0][1];

				pre_values[2][0] = player[0][0]+1;
				pre_values[2][1] = player[0][1]+1;
			}
		}


		for(int i = 0; i < 3; i++)
		{
			if(pre_values[i][0] == -1 || pre_values[i][0] == 10 || pre_values[i][1] == -1 || pre_values[i][1] == 20)
				can_rotate = 0;
		}

		if(can_rotate)
		{
			for(int i = 0; i < 3; i++)
			{
				if(can_rotate)
				{
					for(int c = 0; c < entities; c++)
					{
						if(pre_values[i][0] == pos[c][0] && pre_values[i][1] == pos[c][1])
							can_rotate = 0;
					}
				}
			}
		}


		if(can_rotate)
		{
			for(int i = 0; i < 3; i++)
			{
				player[i+1][0] = pre_values[i][0];
				player[i+1][1] = pre_values[i][1];
			}
		}
	}

}

//handles user input, reading it again later it looks very complicated
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
				SDL_Scancode key = event.key.keysym.scancode;

				if(key == SDL_SCANCODE_ESCAPE)
					clean_up();
				else if(key == SDL_SCANCODE_LEFT)
				{
					int blocked = 0;

					for(int i = 0; i < 4; i++)
					{
						if(player[i][0] == 0)
							blocked = 1;

						for(int ii = 0; ii < entities; ii++)
						{
							if(player[i][1] == pos[ii][1] && player[i][0] - 1 == pos[ii][0])
								blocked = 1;
						}
					}

					if(!blocked)
					{
						for(int i = 0; i < 4; i++)
							player[i][0]--;
					}
				}
				else if(key == SDL_SCANCODE_RIGHT)
				{	
					int blocked = 0;

					for(int i = 0; i < 4; i++)
					{
						if(player[i][0] == 9)
							blocked = 1;

						for(int ii = 0; ii < entities; ii++)
						{
							if(player[i][1] == pos[ii][1] && player[i][0] + 1 == pos[ii][0])
								blocked = 1;
						}
					}

					if(!blocked)
					{
						for(int i = 0; i < 4; i++)
							player[i][0]++;
					}
				} else if(key == SDL_SCANCODE_UP)
				{
					rot++;

					if(rot == 4)
						rot = 0;

					rotate();
				}

				break;

			default:
				break;
		}
	}
}

//render things
void render()
{
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
	SDL_RenderClear(app.renderer);

	SDL_RenderCopy(app.renderer, BG, NULL, &rBG);

	if(entities > 0)
	{
		for(int i = 0; i < entities; i++)
		{
			SDL_Rect rCube;
			rCube.x = 97 + (30*pos[i][0]);
			rCube.y = 30*pos[i][1];

			SDL_QueryTexture(cubes[pos[i][2]-1], NULL, NULL, &rCube.w, &rCube.h);
			SDL_RenderCopy(app.renderer, cubes[pos[i][2]-1], NULL, &rCube);
		}
	}

	for(int i = 0; i < 4; i++)
	{
		SDL_Rect rPlayer;
		rPlayer.x = 97 + (30*player[i][0]);
		rPlayer.y = 30 * player[i][1];

		SDL_QueryTexture(cubes[player[0][2]-1], NULL, NULL, &rPlayer.w, &rPlayer.h);
		SDL_RenderCopy(app.renderer, cubes[player[0][2]-1], NULL, &rPlayer);
	}

	//rec.x = 97;
	//rec.y = 0;
	//rec.h = 600;
	//rec.w = 300;

	SDL_RenderPresent(app.renderer);
}

// checks if a line is full
void lines()
{
	int lines[20] = { 0 };

	for(int i = 0; i < entities; i++)
	{
		lines[pos[i][1]]++;
	}

	for(int i = 0; i < 20; i++)
	{
		if(lines[i] == 10)
		{
			line++;

			if(line == 5 && speed != 0.01)
			{
				speed -= 0.01;
				line = 0;
			}

			int move = 0;

			for(int c = 0; c < entities; c++) // go through anc clear entities with that value, couldn't think of a better way
			{ 
				if(pos[c][1] == i)
				{
					pos[c][0] = 0;
					pos[c][1] = 0;
					pos[c][2] = 0;

					move++;
				} else if(move > 0)
				{
					pos[c-move][0] = pos[c][0];
					pos[c-move][1] = pos[c][1];
					pos[c-move][2] = pos[c][2];
				}
			}

			entities -= move;

			for(int c = 0; c < entities; c++) // move the bricks above current line down one
			{
				if(pos[c][1] < i)
				{
					pos[c][1]++;
				}
			}
		}
	}
}

//sends current player to entities and creates a new one
void send_player()
{
	for(int d = 0; d < 4; d++)
	{
		pos[entities][0] = player[d][0];
		pos[entities][1] = player[d][1];
		pos[entities][2] = player[0][2];

		entities++;
	}

	player[0][0] = 4;
	player[0][1] = 0;
	player[0][2] = rand()%7;
	player[0][2]++;

	rot = 0;

	lines();

	calc_shape();
}

//calculates downward movement
void calc_move()
{
	int skip = 0;

	if(entities > 0)
	{
		for(int i = 0; i < 4; i++)
		{
			if(!skip)
			{
				for(int c = 0; c < entities; c++)
				{
					if(skip != 1 && player[i][1] + 1 == pos[c][1] && player[i][0] == pos[c][0])
					{
						skip = 1;

						send_player();
					}
				}
			}
		}
	}

	for(int i = 0; i < 4; i++)
	{
		if(skip != 1 && player[i][1] == 19)
		{
			skip = 1;

			send_player();
		}
	}

	if(!skip)
	{

		for(int i = 0; i < 4; i++)
		{
			player[i][1]++;
		}
	}
}
