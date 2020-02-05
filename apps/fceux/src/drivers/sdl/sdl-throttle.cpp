/// \file
/// \brief Handles emulation speed throttling using the SDL timing functions.

#include "sdl.h"
#include "throttle.h"

static uint64 Lasttime, Nexttime;
static int32 desired_fps;
static int InFrame;

/**
 * Refreshes the FPS throttling variables.
 */
void
RefreshThrottleFPS()
{
	desired_fps = FCEUI_GetDesiredFPS();

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
		Nexttime = Lasttime + 1000 / desired_fps;
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
