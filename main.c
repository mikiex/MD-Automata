// TODO: See if we can upload a rect to vram directly from the array!!!!!!!

#include <genesis.h>
#include <resources.h>

const int ROW = 42; 
const int COL = 30;
unsigned char buffer0[42][30]; // Buffers are 2 larger that the screen so we can have a border of 0s.
unsigned char buffer1[42][30]; // Just a 2D array of integers
int bufferState = 0; // Used to swap bettween buffers
enum filltype {fill_rand, fill_glider}; //TODO: Add a glider generator and other patterns
typedef enum {joy_none,joy_left,joy_right} joystates;
joystates joyPressed = joy_none;

void LoadResources();
void FillTiles(unsigned char buffer[ROW][COL]);
void FillGlider(unsigned char buffer[ROW][COL]);
void ClearTiles(unsigned char buffer[ROW][COL]);
void DrawTiles(unsigned char buffer[ROW][COL]);
int CountNeighbors(unsigned char buffer[ROW][COL],int xpos, int ypos);
void ProcessBuffer(unsigned char bufferA[ROW][COL],unsigned char bufferB[ROW][COL]);
void myJoyHandler( u16 joy, u16 changed, u16 state);
void init(enum filltype f);

// Patterns
unsigned char patternGlider[3][3]=
{
	{1,0,0},
	{0,1,1},
	{1,1,0}
};

int main()
{
	JOY_init();
	JOY_setEventHandler( &myJoyHandler );
	LoadResources();
	InitBuffer(fill_rand);


    while(1)
    {
		if (joyPressed  == joy_left)
		{	
			InitBuffer(fill_glider);
		}

		if (joyPressed  == joy_right)
		{	
			InitBuffer(fill_rand);
		}

		joyPressed = joy_none; //Clear Joy state

		if (bufferState == 0)
		{
			ProcessBuffer(buffer0,buffer1);
			DrawTiles(buffer1);
			bufferState = 1;
		}
		else if (bufferState == 1)
		{
			ProcessBuffer(buffer1,buffer0);
			DrawTiles(buffer0);
			bufferState = 0;
		}

		VDP_waitVSync();
    }
    return (0);
}

void InitBuffer(enum filltype f)
{
	if (f == fill_rand)
	{
		ClearTiles(buffer0);
		ClearTiles(buffer1);
		bufferState = 0;
		FillTiles(buffer0);
	}
	
	if (f == fill_glider)
	{
		if (bufferState == 0)
		{
			FillGlider(buffer0);
		}
		else
		{
			FillGlider(buffer1);
		}
		
	}

}

// Load images.
void LoadResources()
{
	VDP_loadBMPTileData(tile.image,1,1,1,1);
	VDP_loadBMPTileData(dead.image,2,1,1,1);
	VDP_setPalette(PAL1, tile.palette->data);
}

// Fill buffer with 0s or 1s.
void FillTiles(unsigned char buffer[ROW][COL])
{
	for (int x = 0; x < ROW; ++x)
  	{
		for (int y = 0; y < COL; ++y)
		{
			if ( x == 0 || x == ROW-1 || y == 0 || y == COL-1)
			{
				buffer[x][y] = 0; // All edges beyond the screen are set to 0s.
			}
			else
			{
				buffer[x][y] = (random() % 2);
			}

		}
	}
}

void FillGlider(unsigned char buffer[ROW][COL])
{
	int xpos = 1 + (random() % (ROW -1));
	int ypos = 1 + (random() % (COL -1));
	for (int x = 0; x < 3; ++x)
  	{
		for (int y = 0; y < 3; ++y)
		{

			buffer[x+xpos][y+ypos] = patternGlider[x][y];
			
		}
	}
}

void ClearTiles(unsigned char buffer[ROW][COL])
{
	for (int x = 0; x < ROW; ++x)
  	{
		for (int y = 0; y < COL; ++y)
		{
			buffer[x][y]= 0;
		}
	}
}

// TODO: upload rect instead of looping and doing 1 tile at a time???
void DrawTiles(unsigned char buffer[ROW][COL])
{
	// Miss out 0s around the edges, so we read +1 in, up to the end -1.
	for (int x = 1; x < ROW-1; ++x)
  	{
		 for (int y = 1; y < COL-1; ++y)
		{
			if (buffer[x][y] == 1)
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

int CountNeighbors(unsigned char buffer[ROW][COL],int xpos, int ypos)
{
	int n = 0;
	for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			n += buffer[xpos+x][ypos+y];
		}	
	}
	return n - buffer[xpos][ypos]; // Sum of all neighbors minus the actual value in the cell position.
}

// Process the buffer and put the results in another buffer.
// In this case we are using Conways Game of Life rules.
void ProcessBuffer(unsigned char bufferA[ROW][COL],unsigned char bufferB[ROW][COL])
{
	// Miss out 0s around the edges, so we read +1 in, up to the end -1.
	for (int x = 1; x < ROW-1; ++x)
  	{
		 for (int y = 1; y < COL-1; ++y)
		{
			int n = CountNeighbors(bufferA,x,y);
			if (bufferA[x][y] == 1) // 1 Cell is alive.
			{
				if (n < 2 || n >3)
				{
					bufferB[x][y] = 0;
				}
				else
				{
					bufferB[x][y] = 1;
				}
			}
			else if (n == 3)// 0 Cell is dead.
			{
				// Give birth.
					bufferB[x][y] = 1;
			}
			else // Cell is dead but we still need to copy the 0 dead state as there maybe garbage in the destination buffer
			{
				bufferB[x][y] = 0; 
			}
		}
	}
}

void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		if (state & BUTTON_RIGHT)
		{
			joyPressed = joy_right;
		}

		if (state & BUTTON_LEFT)
		{
			joyPressed = joy_left;
		}
	}
}