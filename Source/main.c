// Air Attack game. This is a simple game so all processing is in the main file.
#include <stdlib.h>				// For rand() random numbers.
#include <stdio.h>				// For sprintf.

#include <coreinit/screen.h>	// for OSScreen.
#include <coreinit/thread.h>	// for Sleep.
#include <coreinit/time.h>		// To get time in usec.
#include <vpad/input.h>			// For the game pad inputs.
#include <whb/proc.h>			// For the loop and to do home button correctly.
#include <whb/log.h>			// ** Using the console logging features seems to help set up the screen output.
#include <whb/log_console.h>	// ** Found neeeded to keep these in the build for the program to display properly.

#include "Draw.h"				// For drawing via OSScreen.
#include "Images/Images.h"		// For the images to be drawn using Draw.h.
#include "Sounds.h"				// For sound.

#define MAXPLANES 10			// Limit of planes on the screen.

//A value from enum GAME_STATE.
typedef unsigned int gameState_t;

// Enumerated type for the overall states of the game.
enum GAME_STATE
{
	WAITING = 0,
	PLAYING = 1,
	ENDING = 2
};

//A value from enum OBJECT_STATE.
typedef unsigned int objectState_t;

// Enumerated type for the states of an object (plane) within the game.
enum OBJECT_STATE
{
	STOPPED = 0,
	FLYING  = 1,
	EXPLODE = 2
};

// The gameObject structure is used for game operation and to support graphic display.  
typedef struct gameObject gameObject_t;

// gameObject has current x/y position and end x/y position and some other controls.
struct gameObject
{
	objectState_t state;	// Stopped, flying or explode.
	int x;					// current x position.
	int y;					// Current y position.
	int xend;				// Position x the object is moving towards.
	int yend;				// Position y the object is moving towards.
	float pct;				// Percentage size of the object.
	int shotCnt;			// A sequence count for if the object is shot.
};

gameObject_t bullets[2];			// Display two bullets.
gameObject_t planes[MAXPLANES];		// Up to MAXPLANES planes can be displayed.

gameState_t gameState = WAITING;	// Game state waiting, playing, ending.

int tim;							// Variable for processing time of a game cycle (only global so it can be displayed).
int score;							// Game score.

int animate = 0;					// Increasing count used to sequence the animation of the display (and sequence the game).
int newPlane = 50;					// Interval on animate to start a new plane (reduces during game to make it harder).

int xshift = 0;						// Shift in display for direction sight is facing.
int yshift = 0;

void displayEnd()
{
	static int endCnt = 0;		// Sequence for the end of the game animation.

	if (endCnt >= 20) { drawImage(XDISPMAX, YDISPMAX, EndImage, XDISPCTR, YDISPCTR); }
	else if (endCnt >= 16) { drawImage(XDISPMAX, YDISPMAX, Exp5Image, XDISPCTR, YDISPCTR); }
	else if (endCnt >= 12) { drawImage(XDISPMAX, YDISPMAX, Exp4Image, XDISPCTR, YDISPCTR); }
	else if (endCnt >= 8) { drawImage(XDISPMAX, YDISPMAX, Exp3Image, XDISPCTR, YDISPCTR); }
	else if (endCnt >= 4) { drawImage(XDISPMAX, YDISPMAX, Exp2Image, XDISPCTR, YDISPCTR); }
	else if (endCnt >= 0) { drawImage(XDISPMAX, YDISPMAX, Exp1Image, XDISPCTR, YDISPCTR); }
	endCnt++;
	// Allow the end of game to be displayed on the screen for a while then set up for a new game.
	if (endCnt >= 80)
	{
		gameState = WAITING;	// Back to waiting for a new game.
		putsoundSel(BKGNDWAIT);	// Waiting music.
		endCnt = 0;				// clear endCnt ready for end of next game.
		newPlane = 50;			// Set the planes to start slowly.
		// Clear out all planes to stopped ready for next game.
		for (int a = 0; a < MAXPLANES; a++)
		{
			planes[a].state = STOPPED;
			planes[a].pct = 1.0;
			planes[a].shotCnt = 0;
		}
	}
}

// Get the player movement from the Joycon and Button controls.
void getPlayerControls()
{
	VPADStatus status;			// Status returned for the game pad buttons.

	// Get the VPAD button status.
	VPADRead(VPAD_CHAN_0, &status, 1, NULL);

	// Move the x and y shift values for the controls. SO that the background and all objects move correctly.
	if ((status.hold & VPAD_BUTTON_DOWN)  || (status.hold & VPAD_STICK_L_EMULATION_DOWN)) { yshift = yshift - 15; }
	if ((status.hold & VPAD_BUTTON_LEFT)  || (status.hold & VPAD_STICK_L_EMULATION_LEFT)) { xshift = xshift - 20; }
	if ((status.hold & VPAD_BUTTON_RIGHT) || (status.hold & VPAD_STICK_L_EMULATION_RIGHT)) { xshift = xshift + 20; }
	if ((status.hold & VPAD_BUTTON_UP)    || (status.hold & VPAD_STICK_L_EMULATION_UP)) { yshift = yshift + 15; }
	if (yshift < YMOVEMIN) { yshift = YMOVEMIN; }
	if (yshift > YMOVEMAX) { yshift = YMOVEMAX; }
	if (xshift < XMOVEMIN) { xshift = XMOVEMIN; }
	if (xshift > XMOVEMAX) { xshift = XMOVEMAX; }
}

// Add a new plane to the screen.
void addPlane()
{
	// Add a new plane plane periodically.
	if ((animate % newPlane) == 0)
	{
		// Find the first plane in the array that is not flying and initialise it.
		for (int a = 0; a < MAXPLANES; a++)
		{
			if (planes[a].state == STOPPED)
			{
				planes[a].state = FLYING;
				// The start and end positions on the screen for the new plane are set to random values inside the limits of movement.
				// It wouldn't be fair to put planes where they can't be shot.
				planes[a].x = XDISPCTR + ((rand() % XMOVEMAX) * 2) - XMOVEMAX;
				planes[a].y = YDISPCTR + ((rand() % YMOVEMAX) * 2) - YMOVEMAX;
				planes[a].xend = XDISPCTR + ((rand() % XMOVEMAX) * 2) - XMOVEMAX;
				planes[a].yend = YDISPCTR + ((rand() % YMOVEMAX) * 2) - YMOVEMAX;
				planes[a].pct = 1.0;
				planes[a].shotCnt = 0;
				break;
			}
		}
	}
}

// Sort the planes into the largest last, so that closest plane is on top when displayed.
void sortPlanes()
{
	gameObject_t planeTemp;		// Temporary game object used for sorting the planes into size.

	for (int a = 0; a < MAXPLANES - 1; a++)
	{
		for (int b = 0; b < MAXPLANES - 1; b++)
		{
			if (planes[b + 1].pct < planes[b].pct)
			{
				planeTemp.state = planes[b].state;
				planeTemp.x = planes[b].x;
				planeTemp.y = planes[b].y;
				planeTemp.xend = planes[b].xend;
				planeTemp.yend = planes[b].yend;
				planeTemp.pct = planes[b].pct;
				planeTemp.shotCnt = planes[b].shotCnt;

				planes[b].state = planes[b + 1].state;
				planes[b].x = planes[b + 1].x;
				planes[b].y = planes[b + 1].y;
				planes[b].xend = planes[b + 1].xend;
				planes[b].yend = planes[b + 1].yend;
				planes[b].pct = planes[b + 1].pct;
				planes[b].shotCnt = planes[b + 1].shotCnt;

				planes[b + 1].state = planeTemp.state;
				planes[b + 1].x = planeTemp.x;
				planes[b + 1].y = planeTemp.y;
				planes[b + 1].xend = planeTemp.xend;
				planes[b + 1].yend = planeTemp.yend;
				planes[b + 1].pct = planeTemp.pct;
				planes[b + 1].shotCnt = planeTemp.shotCnt;
			}
		}
	}
}

// Deal with a plane being hit.
void checkHits()
{
	// Check if bullets have travelled far enough and that both bullets have hit the plane.
	// To make it harder the x and y must hit the plane in the central mass, not just touch a wing.
	for (int a = MAXPLANES - 1; a >= 0; a--)
	{
		if ((bullets[1].x - bullets[0].x) < SIGHTX / 3)
		{
			if ((abs(planes[a].x - bullets[0].x) < ((PLANEX / 4 * planes[a].pct) / 100)) && (abs(planes[a].y - bullets[0].y) < ((PLANEY / 3 * planes[a].pct) / 100)) && (bullets[0].state == FLYING) && (planes[a].state == FLYING))
			{
				if ((abs(planes[a].x - bullets[1].x) < ((PLANEX / 4 * planes[a].pct) / 100)) && (abs(planes[a].y - bullets[1].y) < ((PLANEY / 3 * planes[a].pct) / 100)) && (bullets[1].state == FLYING) && (planes[a].state == FLYING))
				{
					putsoundSel(HIT);
					// If the plane is hit stop the bullets animating.
					bullets[0].state = STOPPED;
					bullets[1].state = STOPPED;
					// Increase the score and adjust the rate at which new planes come into the game.
					score = score + (10 * (100.0 - planes[a].pct));
					newPlane = 50 - (score / 2000);
					if (newPlane < 10) { newPlane = 10; }
					// Set the plane into explode mode so that the explosion animation runs.
					planes[a].state = EXPLODE;
					planes[a].shotCnt = 0;
				}
			}
		}
	}
}

// Check if fire pressed, start new bullet or progress flying bullets.
void processBullets()
{
	VPADStatus status;			// Status returned for the game pad buttons.

	// Get the VPAD button status.
	VPADRead(VPAD_CHAN_0, &status, 1, NULL);

	// Check the fire buttons. If the bullets aren't currently flying, fire new bullets.
	if (((status.hold & VPAD_BUTTON_ZL) || (status.hold & VPAD_BUTTON_ZR)) && (bullets[0].state == STOPPED))
	{
		putsoundSel(FIRE);
		bullets[0].state = FLYING;
		bullets[0].x = 0;
		bullets[0].y = YDISPMAX;
		bullets[0].xend = XDISPCTR + xshift;
		bullets[0].yend = YDISPCTR - yshift;
		bullets[0].pct = 99.0;

		bullets[1].state = FLYING;
		bullets[1].x = XDISPMAX;
		bullets[1].y = YDISPMAX;
		bullets[1].xend = XDISPCTR + xshift;
		bullets[1].yend = YDISPCTR - yshift;
		bullets[1].pct = 99.0;
	}

	// If the bullets are flying display them.
	if (bullets[0].state == FLYING)
	{
		bullets[0].x = (int)(float)(bullets[0].xend * (100.0 - bullets[0].pct) / 100.0);
		bullets[0].y = YDISPMAX - (int)((float)(YDISPMAX - bullets[0].yend) * (100.0 - bullets[0].pct) / 100.0);
		scaleImage(BULLETX, BULLETY, BulletImage, bullets[0].x - xshift, bullets[0].y + yshift, bullets[0].pct);

		bullets[1].x = XDISPMAX - (int)((float)(XDISPMAX - bullets[1].xend) * (100.0 - bullets[1].pct) / 100.0);
		bullets[1].y = YDISPMAX - (int)((float)(YDISPMAX - bullets[1].yend) * (100.0 - bullets[1].pct) / 100.0);
		scaleImage(BULLETX, BULLETY, BulletImage, bullets[1].x - xshift, bullets[1].y + yshift, bullets[0].pct);

		// Reduce the bullets size as they fly away.
		bullets[0].pct = bullets[0].pct * 0.8;
		bullets[1].pct = bullets[1].pct * 0.8;

		// If the bullet has missed stop display ready to fire new bullets.
		if (bullets[0].pct < 2.0)
		{
			bullets[0].state = STOPPED;
			bullets[0].x = 0;
			bullets[0].y = YDISPMAX;
			bullets[1].state = STOPPED;
			bullets[1].x = XDISPMAX;
			bullets[1].y = YDISPMAX;
		}
	}
}

void processPlanes()
{
	// Move planes and increase size as they get closer.
	for (int a = 0; a < MAXPLANES; a++)
	{
		if (planes[a].state == FLYING)
		{
			// Move the plane towards its end position, but limit this to the limit of screen movement.
			if ((planes[a].xend > planes[a].x) && (planes[a].x < (XDISPCTR + XMOVEMAX))) { planes[a].x = planes[a].x + 2; }
			if ((planes[a].xend < planes[a].x) && (planes[a].x > (XDISPCTR + XMOVEMIN))) { planes[a].x = planes[a].x - 2; }
			if ((planes[a].yend > planes[a].y) && (planes[a].x < (YDISPCTR + YMOVEMAX))) { planes[a].y = planes[a].y + 2; }
			if ((planes[a].yend < planes[a].y) && (planes[a].x < (YDISPCTR + YMOVEMIN))) { planes[a].y = planes[a].y - 2; }

			// Increase the plane size as it gets closer but limit to 100% of the image.
			// If the plane has reached full size it has over-run the defenses so this is the end of the game.
			planes[a].pct = planes[a].pct * 1.04;
			if (planes[a].pct > 100.0)
			{
				planes[a].state = STOPPED;

				// Planes are also shown while waiting, only want to end the game if we are actually playing.
				if (gameState == PLAYING)
				{
					putsoundSel(ENDGAME);
					gameState = ENDING;
					break;
				}
			}
			else
			{
				// Cycle round the three plane images for animation of the prop.
				if ((animate % 3) == 0) { scaleImage(PLANEX, PLANEY, MyPlane1Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
				else if ((animate % 3) == 1) { scaleImage(PLANEX, PLANEY, MyPlane2Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
				else if ((animate % 3) == 2) { scaleImage(PLANEX, PLANEY, MyPlane3Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			}
		}
		// If the plane has been shot do the explosion animation.
		else if (planes[a].state == EXPLODE)
		{
			planes[a].shotCnt++;
			if (planes[a].shotCnt == 1) { scaleImage(PLANEX, PLANEY, Shot1Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			if (planes[a].shotCnt == 2) { scaleImage(PLANEX, PLANEY, Shot2Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			if (planes[a].shotCnt == 3) { scaleImage(PLANEX, PLANEY, Shot3Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			if (planes[a].shotCnt == 4) { scaleImage(PLANEX, PLANEY, Shot4Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			if (planes[a].shotCnt == 5) { scaleImage(PLANEX, PLANEY, Shot5Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			if (planes[a].shotCnt == 6) { scaleImage(PLANEX, PLANEY, Shot6Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			if (planes[a].shotCnt == 7) { scaleImage(PLANEX, PLANEY, Shot7Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct); }
			if (planes[a].shotCnt == 8)
			{
				scaleImage(PLANEX, PLANEY, Shot8Image, planes[a].x - xshift, planes[a].y + yshift, planes[a].pct);
				// Once the explosion animation is complete, set the plane back to stopped state.
				planes[a].state = STOPPED;
			}
		}
	}
}

void drawBorder()
{
	// Put a border round the screen to make a neat edge.
	drawLine(XOFFSET, YOFFSET, XOFFSET + XDISPMAX, YOFFSET, 0x01010100);
	drawLine(XOFFSET, YOFFSET - 1, XOFFSET + XDISPMAX, YOFFSET - 1, 0x01010100);

	drawLine(XOFFSET, YOFFSET + YDISPMAX, XOFFSET + XDISPMAX, YOFFSET + YDISPMAX, 0x01010100);
	drawLine(XOFFSET, YOFFSET + YDISPMAX + 1, XOFFSET + XDISPMAX, YOFFSET + YDISPMAX + 1, 0x01010100);

	drawLine(XOFFSET, YOFFSET, XOFFSET, YOFFSET + YDISPMAX, 0x01010100);
	drawLine(XOFFSET - 1, YOFFSET, XOFFSET - 1, YOFFSET + YDISPMAX, 0x01010100);

	drawLine(XOFFSET + XDISPMAX, YOFFSET, XOFFSET + XDISPMAX, YOFFSET + YDISPMAX, 0x01010100);
	drawLine(XOFFSET + XDISPMAX + 1, YOFFSET, XOFFSET + XDISPMAX + 1, YOFFSET + YDISPMAX, 0x01010100);
}

// Display all elements from the game on the TV screen.
// As the game is a graphics game this includes the calls to game processing.
void displayTV()
{
	VPADStatus status;			// Status returned for the game pad buttons.

	// Clear the TV to have a sky background.
	OSScreenClearBufferEx(SCREEN_TV, 0x8CFFFB00u);

	// If we are waiting to start show the game screen with planes flying, but don't play.
	if (gameState == WAITING)
	{
		// Draw the new background for the x and y shift.
		drawBackground(BKGNDX, BKGNDY, GameImage, XDISPCTR + xshift, XDISPCTR + yshift);

		addPlane();			// Periodically add a new plane to the screen.
		sortPlanes();		// Sort to display nearest last.
		processPlanes();	// Move and animate the planes.		

		// Put the gun sight in the centre of the screen.
		drawImage(SIGHTX, SIGHTY, sightImage, XDISPCTR, YDISPCTR);

		// Put the the start message on the screen.
		drawImage(450, 110, PressXImage, XDISPCTR, 100);

		drawBorder();	// Put a border round the play area of the screen.

		// Get the VPAD button status.
		VPADRead(VPAD_CHAN_0, &status, 1, NULL);

		// If X is pressed start a new game.
		if (status.trigger & VPAD_BUTTON_X)
		{
			score = 0;		// Score and other controls are set for a new game.

			// Initialise the bullets to doing nothing. 
			// This initialisation should be redundant but is done just in case.
			bullets[0].state = STOPPED;
			bullets[1].state = STOPPED;

			// The planes array as cleared to no planes flying.
			for (int a = 0; a < MAXPLANES; a++)
			{
				planes[a].state = STOPPED;
				planes[a].pct = 1.0;
				planes[a].shotCnt = 0;
			}
			gameState = PLAYING;	// Move to playing.
			putsoundSel(BKGNDPLAY);	// Game music.
		}
	}
	// Play the game until the end.
	else if (gameState == PLAYING)
	{
		getPlayerControls();	// Get the player movement.

		// The processing is from background to foreground so that the closest item (e.g. sight) is on top.

		// Draw the new background for the x and y shift.
		drawBackground(BKGNDX, BKGNDY, GameImage, XDISPCTR + xshift, XDISPCTR + yshift);

		addPlane();			// Periodically add a new plane to the game.
		sortPlanes();		// Sort to display nearest last.
		checkHits();		// Check if a plane has been hit.

		// Note process planes moves the gameState to ENDING if a plane gets through.
		processPlanes();	// Move and animate the planes.		

		processBullets();	// Move bullets or fire new ones.

		// Put the gun sight in the centre of the screen.
		drawImage(SIGHTX, SIGHTY, sightImage, XDISPCTR, YDISPCTR);

		drawBorder();	// Put a border round the play area of the screen.
	}
	// If the game has ended (a plane got through) do the end of game animation.
	else if (gameState == ENDING)
	{
		// Dsiplay end screens then set gameState back to WAITING when the ending is complete.
		displayEnd();
	}

	animate++;	// Increment the animation count.

	// Flip the screen buffer to show the new display.
	OSScreenFlipBuffersEx(SCREEN_TV);
	return;
}

// Display information on the Gamepad screen.
void displayGPad()
{
//	char stime[100]  = "\0";	// Strings to display delay time. ** only displayed during development.
	char sscore[100] = "\0";	// Strings to display the current score.

	// Clear the Gamepad to have a grey background.
	OSScreenClearBufferEx(SCREEN_DRC, 0x80808000u);

	OSScreenPutFontEx(SCREEN_DRC, 3,  1, "Air Attack!");

	OSScreenPutFontEx(SCREEN_DRC, 3,  3, "You're the last line of defence.");
	OSScreenPutFontEx(SCREEN_DRC, 3,  4, "If a plane gets through it's all over!");

	OSScreenPutFontEx(SCREEN_DRC, 3,  6, "Use the left joycon or direction buttons to move.");
	OSScreenPutFontEx(SCREEN_DRC, 3,  7, "Press ZL or ZR to fire.");
	OSScreenPutFontEx(SCREEN_DRC, 3,  8, "Press X to start.");

	sprintf(sscore, "Score: % 6i ", score);
	OSScreenPutFontEx(SCREEN_DRC, 3, 10, sscore);

//	sprintf(stime, "Time: % 6i ", tim);	** only displayed during development.
//	OSScreenPutFontEx(SCREEN_DRC, 3, 12, stime);

	// Flip the screen buffer to show the new display.
	OSScreenFlipBuffersEx(SCREEN_DRC);
	return;
}

int main(int argc, char **argv) {

	OSTime tm1, tm2;	// Times in usec used to time a game cycle.
	int del = 5;		// Delay used to sequence the game.

	// This is the main process and must be in the program at the start for the home button to operate correctly.
    WHBProcInit();
    WHBLogConsoleInit();	// Console Init seem to get the display to operate correctly so keep in the build.

	setupSound();

	// Intialise the planes array to the centre and all planes stopped (not doing anything).
	for (int a = 0; a < MAXPLANES; a++)
	{
		planes[a].state = STOPPED;
	};

	// Initialise the bullets to doing nothing.
	bullets[0].state = STOPPED;
	bullets[1].state = STOPPED;

	score = 0;	// Set score to 0.

	// There must be a main loop on WHBProc running, for the program to correctly operate with the home button.
	// Home pauses this loop and continues it if resume is selected. There must therefore be one main loop of processing in the main program.
    while (WHBProcIsRunning()) {

		tm1 = OSTicksToMicroseconds(OSGetTick());	// Time before a game cycle processing.

		displayTV();								// Update the TV display.
		displayGPad();								// Update the Gamepad display.

		tm2 = OSTicksToMicroseconds(OSGetTick());	// Time after game cycle processing.

		// As planes get larger it takes more time to process the image.
		// The TV runs at 60Hz update rate (16.667ms update period). This game is scheduled to update the screen at a about 1/4 of this rate, 15 times per second.
		// To keep at this rate, the processing time is measured, and the delay adjusted to keep the game operation smooth.

		tim = ((tm2 - tm1) / 1000);					// Calulate the processing time in msec.
		del = 68 - tim;								// Adjust the game delay for the amount of time used in processing.
		if (del <= 0) { del = 3; }					// Limit delay to sensible values to avoid program getting stuck.
		if (del > 68) { del = 68; }
		OSSleepTicks(OSMillisecondsToTicks(del));	// Delay to keep game operating at the same screen update.
    }

	QuitSound();

	// If we get out of the program clean up and exit.
    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}