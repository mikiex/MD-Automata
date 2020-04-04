#include <genesis.h>
#include <resources.h>

const int ROW = 42;
const int COL = 32;
int buffer0[42][32]; // Buffers are 2 larger that the screen so we can have a border of 0s.
int buffer1[42][32]; // Just a 2D array of integers
int reset = 0;

void LoadResources();
void FillTiles();
void DrawTiles();
int CountNeighbors(int xpos, int ypos);
void ProcessBuffer();
void CopyBuffer();
void myJoyHandler( u16 joy, u16 changed, u16 state);

int main()
{
	JOY_init();
	JOY_setEventHandler( &myJoyHandler );
	LoadResources();
	FillTiles();

    while(1)
    {
		if (reset == 1)
		{
			FillTiles();
			reset = 0;
		}
		DrawTiles();
		ProcessBuffer();
		CopyBuffer(); //TODO: Optomisation instead of copying the buffer we could just use pointers to swap buffers?
    }
    return (0);
}

// Load images.
void LoadResources()
{
	VDP_loadBMPTileData(tile.image,1,1,1,1);
	VDP_loadBMPTileData(dead.image,2,1,1,1);
	VDP_setPalette(PAL1, tile.palette->data);
}

// Fill buffer0 with 0s or 1s.
void FillTiles()
{
	for (int x = 0; x < ROW; ++x)
  	{
		for (int y = 0; y < COL; ++y)
		{
			if ( x == 0 || x == ROW || y == 0 || y == COL)
			{
				buffer0[x][y] = 0; // All edges beyond the screen are set to 0s.
			}
			else
			{
				buffer0[x][y]= (random() % 2);
			}

		}
	}
}

void DrawTiles()
{
	// Miss out 0s around the edges, so we read +1 in, up to the end -1.
	for (int x = 1; x < ROW-1; ++x)
  	{
		 for (int y = 1; y < COL-1; ++y)
		{
			if (buffer0[x][y] == 1)
			{
				VDP_setTileMapXY(PLAN_A,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,1),x-1,y-1);
			}
			else
			{
				VDP_setTileMapXY(PLAN_A,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,2),x-1,y-1);
			}
			
		}
	}
}

int CountNeighbors(int xpos, int ypos)
{
	int n = 0;
	for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			n += buffer0[xpos+x][ypos+y];
		}	
	}
	return n - buffer0[xpos][ypos]; // Sum of all neighbors minus the actual value in the cell position.
}

// Process the buffer0 and put the results in buffer1.
// In this case we are using Conways Game of Life rules.
void ProcessBuffer()
{
	// Miss out 0s around the edges, so we read +1 in, up to the end -1.
	for (int x = 1; x < ROW-1; ++x)
  	{
		 for (int y = 1; y < COL-1; ++y)
		{
			int n = CountNeighbors(x,y);
			if (buffer0[x][y] == 1) // 1 Cell is alive.
			{
				if (n < 2 || n >3)
				{
					buffer1[x][y] = 0;
				}
				else
				{
					buffer1[x][y] = 1;
				}
			}
			else // 0 Cell is dead.
			{
				if (n == 3) // Give birth.
				{
					buffer1[x][y] = 1;
				}
			}
			
		}
	}
}

// Copy one buffer to another.
void CopyBuffer()
{
	for (int x = 0; x < ROW; ++x)
  	{
		 for (int y = 0; y < COL; ++y)
		{
			buffer0[x][y] = buffer1[x][y];
		}
	}
}

void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		if (state & BUTTON_RIGHT)
		{
			reset = 1;
		}
	}
}