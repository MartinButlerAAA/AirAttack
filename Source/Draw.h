// Header for drawing functions.
#pragma once

// The background colour is defined. Pixels in images that match the background colour are not processed, in effect giving sprites a transparent background.
#define BKGNDCLR 0x8CFFFB00

#define XDISPMAX 800 // Limits to game screen size.
#define YDISPMAX 600

#define XDISPCTR XDISPMAX/2 // Centre of the screen.
#define YDISPCTR YDISPMAX/2

#define XOFFSET  240 // Offset to position game screen on the TV.
#define YOFFSET   60

#define PLANEX   700 // Size of all plane images.
#define PLANEY   393

#define BULLETX  240 // Size of bullets.
#define BULLETY  240

#define SIGHTX   200 // Size of gun sight.
#define SIGHTY   200

#define BKGNDX  1200 // Size of scrolling background image.
#define BKGNDY   900

#define XMOVEMAX (BKGNDX-XDISPMAX)/2	// Constants to define the range of movement of the sights within the screen.
#define XMOVEMIN -XMOVEMAX
#define YMOVEMAX (BKGNDY-YDISPMAX)/2
#define YMOVEMIN -YMOVEMAX

// Draw the visible part of the background based on xpos and ypos.
bool drawBackground(unsigned int xmax, unsigned int ymax, unsigned int ImageP[ymax][xmax], unsigned int xpos, unsigned int ypos);

// Draw an image on the screen at xpos and ypos.
bool drawImage(unsigned int xmax, unsigned int ymax, unsigned int ImageP[ymax][xmax], unsigned int xpos, unsigned int ypos);

// Draw a scaled image on the screen at xpos and ypos. Scaled between 0% and 100%.
bool scaleImage(unsigned int xmax, unsigned int ymax, unsigned int ImageP[ymax][xmax], unsigned int xpos, unsigned int ypos, float pct);

// Draw a line of the colour specified.
bool drawLine(float x1, float y1, float x2, float y2, unsigned int colour);
