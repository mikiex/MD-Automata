// TODO: See if we can upload a rect to vram directly from the array!!!!!!!

#include <genesis.h>
#include <resources.h>

const int ROW = 42; 
const int COL = 30;
unsigned char buffer0[42][30]; // Buffers are 2 larger that the screen so we can have a border of 0s.
unsigned char buffer1[42][30]; // Just a 2D array of integers
int bufferState = 0; // Used to swap bettween buffers
enum filltype {fill_rand, spawn_glider, fill_gosper}; //TODO: Add a glider generator and other patterns
typedef enum {joy_none,  joy_left,joy_right, joy_up, joy_down} joystates;
joystates joyPressed = joy_none;

typedef struct coords {
unsigned char x;
unsigned char y;} coords;

void LoadResources();
void FillPattern(unsigned char buffer[ROW][COL],coords cells[], int xoffset, int yoffset, int cellcount);
void FillTiles(unsigned char buffer[ROW][COL]);
void ClearTiles(unsigned char buffer[ROW][COL]);
void DrawTiles(unsigned char buffer[ROW][COL]);
int CountNeighbors(unsigned char buffer[ROW][COL],int xpos, int ypos);
void ProcessBuffer(unsigned char bufferA[ROW][COL],unsigned char bufferB[ROW][COL]);
void myJoyHandler( u16 joy, u16 changed, u16 state);
void InitBuffer(enum filltype f);

// Patterns
coords coord_Glider[] = {{0, 0},{1, 1},{2, 1},{0, 2},{1, 2}};

coords coord_Gosper[] = {
                         {1, 5},{2, 5},{1, 6},{2, 6},
                         {13, 3},{14, 3},{12, 4},{16, 4},{11, 5},{17, 5},{11, 6},{15, 6},{17, 6},{18, 6},{11, 7},{17, 7},{12, 8},{16, 8},{13, 9},{14, 9},
                         {25, 1},{23, 2},{25, 2},{21, 3},{22, 3},{21, 4},{22, 4},{21, 5},{22, 5},{23, 6},{25, 6},{25, 7},
                         {36, 3},{35, 3},{36, 4},{35, 4}};


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
			InitBuffer(spawn_glider);
		}

		if (joyPressed  == joy_right)
		{	
			InitBuffer(fill_rand);
		}

		if (joyPressed  == joy_up) // Clear buffers
		{	
			ClearTiles(buffer0);
			ClearTiles(buffer1);
			bufferState = 0;
		}

		if (joyPressed  == joy_down)
		{	
			InitBuffer(fill_gosper);
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

	if (f == spawn_glider)
	{
		if (bufferState == 0)
		{
			FillPattern(buffer0, coord_Glider, (1 + (random() % (ROW -5))), (1 + (random() % (COL -5))), 5); // -5 to stop adding past edges!
		}
		else
		{
			FillPattern(buffer1, coord_Glider, (1 + (random() % (ROW -5))), (1 + (random() % (COL -5))), 5);
		}
	}

	if (f == fill_gosper)
	{
		ClearTiles(buffer0);
		ClearTiles(buffer1);
		bufferState = 0;
		FillPattern(buffer0, coord_Gosper,1,1,36);
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


void FillPattern(unsigned char buffer[ROW][COL],coords cells[], int xoffset, int yoffset, int cellcount)
{
	for (int i = 0; i < cellcount; ++i)
  	{
		buffer[xoffset + cells[i].x][yoffset + cells[i].y] = 1;
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

		if (state & BUTTON_UP)
		{
			joyPressed = joy_up;
		}

		if (state & BUTTON_DOWN)
		{
			joyPressed = joy_down;
		}
	}
}