/// \file
/// \brief Handles emulation speed throttling using the SDL timing functions.

#include "sdl.h"
#include "throttle.h"

static uint64 Lasttime, Nexttime;
static long double desired_frametime;
static int InFrame;

/**
 * Refreshes the FPS throttling variables.
 */
void
RefreshThrottleFPS()
{
	uint64 fps = FCEUI_GetDesiredFPS(); // Do >> 24 to get in Hz
	desired_frametime = 16777216.0l / fps;

	Lasttime=0;   
	Nexttime=0;
	InFrame=0;
}

/**
 * Perform FPS speed throttling by delaying until the next time slot.
 */
int
SpeedThrottle()
{
	uint64 time_left;
	uint64 cur_time;
    
	if(!Lasttime)
		Lasttime = uptime();
    
	if(!InFrame)
	{
		InFrame = 1;
		Nexttime = Lasttime + desired_frametime * 1000;
	}
    
	cur_time  = uptime();
	if(cur_time >= Nexttime)
		time_left = 0;
	else
		time_left = Nexttime - cur_time;
    
	if(time_left > 50)
	{
		time_left = 50;
		/* In order to keep input responsive, don't wait too long at once */
		/* 50 ms wait gives us a 20 Hz responsetime which is nice. */
	}
	else
		InFrame = 0;
    
  //delay
  while (uptime() - cur_time < time_left)
    ;
    
	if(!InFrame)
	{
		Lasttime = uptime();
		return 0; /* Done waiting */
	}
	return 1; /* Must still wait some more */
}
