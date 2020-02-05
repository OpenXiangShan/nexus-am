#include "main.h"
#include "throttle.h"

#include "../../fceu.h"
#include "../../version.h"

#include "input.h"
#include "dface.h"

#include "sdl.h"
#include "sdl-video.h"

#include "../../types.h"

#ifdef __ISA_NATIVE__
#define NR_FRAMESKIP 0
#else
#define NR_FRAMESKIP 2
#endif

int isloaded;

bool turbo = false;

int eoptions=0;

static int inited = 0;

static void DriverKill(void);
static int DriverInitialize(FCEUGI *gi);
uint64 FCEUD_GetTime();
int gametype = 0;

int pal_emulation;
int dendy;

// global configuration object
//Config *g_config;

/**
 * Loads a game, given a full path/filename.  The driver code must be
 * initialized after the game is loaded, because the emulator code
 * provides data necessary for the driver code(number of scanlines to
 * render, what virtual input devices to use, etc.).
 */
int LoadGame(const char *path)
{
    if (isloaded){
        CloseGame();
    }
	if(!FCEUI_LoadGame(path, 1)) {
		return 0;
	}

	ParseGIInput(GameInfo);
	RefreshThrottleFPS();

	if(!DriverInitialize(GameInfo)) {
		return(0);
	}

	// set pal/ntsc
	FCEUI_SetRegion(0);

	isloaded = 1;

	return 1;
}

/**
 * Closes a game.  Frees memory, and deinitializes the drivers.
 */
int
CloseGame()
{
	if(!isloaded) {
		return(0);
	}

	FCEUI_CloseGame();

	DriverKill();
	isloaded = 0;
	GameInfo = 0;

	InputUserActiveFix();
	return(1);
}

void FCEUD_Update(uint8 *XBuf, int32 *Buffer, int Count);

static void DoFun(int frameskip, int periodic_saves)
{
	uint8 *gfx;
	int32 *sound;
	int32 ssize = 0;
	static int fskipc = 0;
	static int opause = 0;

#ifdef FRAMESKIP
	fskipc = (fskipc + 1) % (frameskip + 1);
#endif

	if(NoWaiting) {
		gfx = 0;
	}
	FCEUI_Emulate(&gfx, &sound, &ssize, fskipc);
	FCEUD_Update(gfx, sound, ssize);

	if(opause!=FCEUI_EmulationPaused()) {
		opause=FCEUI_EmulationPaused();
		//SilenceSound(opause);
	}
}


/**
 * Initialize all of the subsystem drivers: video, audio, and joystick.
 */
static int
DriverInitialize(FCEUGI *gi)
{
	if(InitVideo(gi) < 0) return 0;
	inited|=4;

	eoptions &= ~EO_FOURSCORE;

	InitInputInterface();
	return 1;
}

/**
 * Shut down all of the subsystem drivers: video, audio, and joystick.
 */
static void
DriverKill()
{
	if(inited&4)
		KillVideo();
	inited=0;
}

/**
 * Update the video, audio, and input subsystems with the provided
 * video (XBuf) and audio (Buffer) information.
 */
void
FCEUD_Update(uint8 *XBuf,
			 int32 *Buffer,
			 int Count)
{
	// apply frame scaling to Count
	if(Count) {
	} else {
		if(!NoWaiting && (!(eoptions&EO_NOTHROTTLE) || FCEUI_EmulationPaused())) {
      while (SpeedThrottle()) { FCEUD_UpdateInput(); }
    }
		if(XBuf && (inited&4)) {
			BlitScreen(XBuf);
		}
	}
	FCEUD_UpdateInput();
}

/**
 * Opens a file to be read a byte at a time.
 */
EMUFILE_FILE* FCEUD_UTF8_fstream(const char *fn, const char *m)
{
  EMUFILE_FILE *p = (EMUFILE_FILE*)malloc(sizeof(EMUFILE_FILE));
  p->open(fn, m);
  return p;
}

static void UpdateEMUCore()
{
	int ntsccol = 0, ntsctint = 56, ntschue = 72, start = 0, end = 239;
	FCEUI_SetNTSCTH(ntsccol, ntsctint, ntschue);

	FCEUI_SetRegion(0);

	FCEUI_DisableSpriteLimitation(1);

#if DOING_SCANLINE_CHECKS
	for(int i = 0; i < 2; x++) {
		if(srendlinev[x]<0 || srendlinev[x]>239) srendlinev[x]=0;
		if(erendlinev[x]<srendlinev[x] || erendlinev[x]>239) erendlinev[x]=239;
	}
#endif

	FCEUI_SetRenderedLines(start + 8, end - 8, start, end);
}

/**
 * Unimplemented.
 */
void FCEUD_TraceInstruction() {
	return;
}


int noGui = 1;

int KillFCEUXonFrame = 0;

/**
 * The main loop for the SDL.
 */
int main(const char *romname)
{
  _ioe_init();

	int error;

	FCEUD_Message("Starting " FCEU_NAME_AND_VERSION "...\n");

	// initialize the infrastructure
	error = FCEUI_Initialize();
	if(error != 1) {
		return -1;
	}

    // update the input devices
	UpdateInput();

	// update the emu core
	UpdateEMUCore();

  // load the specified game
  error = LoadGame(romname);
  if(error != 1) {
    DriverKill();
    return -1;
  }

    int periodic_saves = 0;

	// loop playing the game
	while(GameInfo)
	{
		DoFun(NR_FRAMESKIP, periodic_saves);
	}
	CloseGame();

	// exit the infrastructure
	FCEUI_Kill();
	return 0;
}

/**
 * Get the time in ticks.
 */
uint64
FCEUD_GetTime()
{
	return uptime();
}

/**
 * Get the tick frequency in Hz.
 */
uint32
FCEUD_GetTimeFreq(void)
{
	// uptime() is in milliseconds
	return 1000;
}

/**
* Prints a textual message without adding a newline at the end.
*
* @param text The text of the message.
*
* TODO: This function should have a better name.
**/
void FCEUD_Message(const char *text)
{
  printf("%s", text);
}

/**
* Shows an error message in a message box.
* (For now: prints to stderr.)
*
* If running in GTK mode, display a dialog message box of the error.
*
* @param errormsg Text of the error message.
**/
void FCEUD_PrintError(const char *errormsg)
{
	printf("%s\n", errormsg);
}


// dummy functions

#define DUMMY(__f) \
    void __f(void) {\
        printf("%s\n", #__f);\
        FCEU_DispMessage("Not implemented.");\
    }
DUMMY(FCEUD_HideMenuToggle)
DUMMY(FCEUD_MovieReplayFrom)
DUMMY(FCEUD_ToggleStatusIcon)
DUMMY(FCEUD_AviRecordTo)
DUMMY(FCEUD_AviStop)
void FCEUI_AviVideoUpdate(const unsigned char* buffer) { }
int FCEUD_ShowStatusIcon(void) {return 0;}
bool FCEUI_AviIsRecording(void) {return false;}
void FCEUI_UseInputPreset(int preset) { }
bool FCEUD_PauseAfterPlayback() { return false; }
// These are actually fine, but will be unused and overriden by the current UI code.
void FCEUD_TurboOn	(void) { NoWaiting|= 1; }
void FCEUD_TurboOff   (void) { NoWaiting&=~1; }
void FCEUD_TurboToggle(void) { NoWaiting^= 1; }

