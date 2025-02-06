// Functions to draw on OSScreen for graphics.
// The header files used with drawImage were created with the ProcessBmp tool (see https://github.com/MartinButlerAAA/ProcessBmp).
#include <stdlib.h>				// For abs.
#include <coreinit/screen.h>	// For OSScreen.

#include "Draw.h"				// Header for graphics drawing.

// Draw the visible part of the background based on xpos and ypos.
// The background scrolls in the opposite direction of movement.
bool drawBackground(unsigned int xmax, unsigned int ymax, unsigned int ImageP[ymax][xmax], unsigned int xpos, unsigned int ypos)
{
	int xcntr, ycntr;	// Offsets to the centre of the image. Used to place the image centre on xpos and ypos.
	int xdisp, ydisp;	// Actual display pixels calculated.
	unsigned int xstrt, ystrt, xend, yend;	// Working start and end limits to display the visible part of the image.

	xcntr = xmax / 2;	// X offset to centre of image
	ycntr = ymax / 2;	// Y offset to centre of image

	// Exit the function if the image size is not sensible.
	if ((xmax > 1201) || (ymax > 901))
	{
		return false;
	}

	// If the image is larger than the display area. Only process the pixels in the display area to save time.
	if (xmax > XDISPMAX)
	{
		// Set the start and end to the data points that are within the area that will be displayed.
		// Avoid wating time processing data that is off the screen.
		xstrt = xmax - xcntr + xpos - XDISPMAX;
		xend = xmax - xcntr + xpos;
		// Ensure that we don't go off the end of the array.
		if (xstrt < 0)    { xstrt = 0; }
		if (xend  > xmax) { xend = xmax; }
	}
	else
	{
		xstrt = 0;
		// If the image is larger than the display limit to the display size.
		if (xmax < XDISPMAX) { xend = xmax; }
		else                 { xend = XDISPMAX; }
	}

	// If the image is larger than the display area. Only process the pixels in the display area to save time.
	if (ymax > YDISPMAX)
	{
		// Set the start and end to the data points that are within the area that will be displayed.
		// Avoid wating time processing data that is off the screen.
		ystrt = ymax - ycntr + ypos - YDISPMAX;
		yend = ymax - ycntr + ypos;
		// Ensure that we don't go off the end of the array.
		if (ystrt < 0)    { ystrt = 0; }
		if (yend  > ymax) { yend = ymax; }
	}
	else
	{
		ystrt = 0;
		// If the image is larger than the display limit to the display size.
		if (ymax < YDISPMAX) { yend = ymax; }
		else                 { yend = YDISPMAX; }
	}

	// Go through all of the visible pixels and set the screen to match the data.
	for (unsigned int y = ystrt; y < yend; y++)
	{
		for (unsigned int x = xstrt; x < xend; x++)
		{
			// Only display the pixel if it is not the background screen colour. This gives sprites a transparent background.
			// It also saves processing time.
			if (ImageP[y][x] != BKGNDCLR)
			{
				// Calculate the x pixel from the x position, x count adjusted for array start offset and centring.
				xdisp = x - xstrt;
				// Calculate the y pixel from the y position, y count adjusted for array start offset and centring.
				ydisp = YDISPMAX - (y - ystrt);

				// The pixels in the bit map file start at the bottom but y=0 starts at the top of the screen.
				// The offsets are used to position the game screen within the TV screen.
				OSScreenPutPixelEx(SCREEN_TV, xdisp + XOFFSET, ydisp + YOFFSET, ImageP[y][x]);
			}
		}
	}
	return true;
}

// Draw an image on the screen at xpos and ypos.
bool drawImage(unsigned int xmax, unsigned int ymax, unsigned int ImageP[ymax][xmax], unsigned int xpos, unsigned int ypos)
{
	int xcntr, ycntr;	// Offsets to the centre of the image. Used to place the image centre on xpos and ypos.
	int xdisp, ydisp;	// Actual display pixels calculated.

	xcntr = xmax / 2;	// X offset to centre of image
	ycntr = ymax / 2;	// Y offset to centre of image

	// Exit the function if the image size is not sensible.
	if ((xmax > XDISPMAX) || (ymax > YDISPMAX))
	{
		return false;
	}

	for (unsigned int y = 0; y < ymax; y++)
	{
		for (unsigned int x = 0; x < xmax; x++)
		{
			// Only display the pixel if it is not the background screen colour. This gives sprites a transparent background.
			// It also saves processing time.
			if (ImageP[y][x] != BKGNDCLR)
			{
				// Calculate the x pixel from the x position, x count adjusted for array start offset and centring.
				xdisp = xpos + x - xcntr;
				// Calculate the y pixel from the y position, y count adjusted for array start offset and centring.
				ydisp = ypos + ymax - y - 1 - ycntr;

				// Only display the pixel if it is inside the display screen.
				if ((xdisp < XDISPMAX) && (xdisp >= 0) && (ydisp < YDISPMAX) && (ydisp >= 0))
				{
					// The pixels in the bit map file start at the bottom but y=0 starts at the top of the screen.
					// The offsets are used to position the game screen within the TV screen.
					OSScreenPutPixelEx(SCREEN_TV, xdisp + XOFFSET, ydisp + YOFFSET, ImageP[y][x]);
				}
			}
		}
	}
	return true;
}

// Draw a scaled image on the screen at xpos and ypos. Scaled between 0% and 100%.
bool scaleImage(unsigned int xmax, unsigned int ymax, unsigned int ImageP[ymax][xmax], unsigned int xpos, unsigned int ypos, float pct)
{
	int xcntr, ycntr;	// Offsets to the centre of the image. Used to place the image centre on xpos and ypos.
	int xdisp, ydisp;	// Actual display pixels calculated.
	int xend, yend;		// end of image scaled down from xmax and ymax.
	float scale;		// scaling (1 over percent size).

	// Limit display to the maximum image size.
	if (pct > 100.0)
	{
		pct = 100.0;
	}

	// Exit the function if the image size is not sensible.
	if ((xmax > XDISPMAX) || (ymax > YDISPMAX))
	{
		return false;
	}

	xcntr = xmax / 2;	// X offset to centre of image
	ycntr = ymax / 2;	// Y offset to centre of image

	// Resize image to percentage, but limit to maximum size of the image array.
	xend = (int)((float)(xmax) * pct / 100.0);
	if (xend > xmax) { xend = xmax; }
	yend = (int)((float)(ymax) * pct / 100.0);
	if (yend > ymax) { yend = ymax; }

	// scale is used to process data points within the image.
	scale = 1.0 / (pct / 100.0);

	// Go through all displayed data points.
	for (unsigned int y = 0; y < yend; y++)
	{
		for (unsigned int x = 0; x < xend; x++)
		{
			// Only display the pixel if it is not the background screen colour. This gives sprites a transparent background.
			// It also saves some processing time.
			if (ImageP[(int)(y * scale)][(int)(x * scale)] != BKGNDCLR)
			{
				// Calculate the x pixel from the x position, x count adjusted for array start offset and centring.
				xdisp = xpos + x - (int)((float)xcntr * pct / 100.0);
				// Calculate the y pixel from the y position, y count adjusted for array start offset and centring.
				ydisp = ypos + yend - y - 1 - (int)((float)ycntr * pct / 100.0);

				// Only display the pixel if it is inside the display screen.
				if ((xdisp < XDISPMAX) && (xdisp >= 0) && (ydisp < YDISPMAX) && (ydisp >= 0))
				{
					// The pixels in the bit map file start at the bottom but y=0 starts at the top of the screen.
					// The offsets are used to position the game screen within the TV screen.
					// Scaling is used to linearly interpolate the image to a smaller size.
					OSScreenPutPixelEx(SCREEN_TV, xdisp + XOFFSET, ydisp + YOFFSET, ImageP[(int)(y * scale)][(int)(x * scale)]);
				}
			}
		}
	}
	return true;
}


// Draw a line of the colour specified.
// Taken from an internet example, but modified to allow line to go in any direction.
bool drawLine(float x1, float y1, float x2, float y2, unsigned int colour)
{
	float dx;	// different on x axis
	float dy;	// difference on y axis
	float len;	// length of line
	float x;	// current x position
	float y;	// current y position.

	// Exit if the positions passed in are off the screen.
	if ((x1 >= 1280.0) || (y1 >= 720.0) || (x2 >= 1280.0) || (y2 >= 720.0))
	{
		return false;
	}
	if ((x1 < 0.0) || (y1 < 0.0) || (x2 < 0.0) || (y2 < 0.0))
	{
		return false;
	}

	// Calculate differences from start to end.
	dx = x2 - x1;
	dy = y2 - y1;

	// Set the length for the longer difference.
	if (abs(dx) >= abs(dy)) { len = abs(dx); }
	else					{ len = abs(dy); }

	// Divide by the length to give the step sizes for drawing. 
	dx = dx / len;
	dy = dy / len;

	// Set to the start of the line.
	x = x1;
	y = y1;

	// Count alog the line length to draw the line.
	for (int i = 0; i <= len; i++)
	{
		OSScreenPutPixelEx(SCREEN_TV, x, y, colour);
		x = x + dx;
		y = y + dy;
	}
	return true;
}