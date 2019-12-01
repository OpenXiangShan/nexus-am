#include "main.h"
#include "throttle.h"
#include "config.h"

#include "../../fceu.h"
#include "../../version.h"

#include "input.h"
#include "dface.h"

#include "sdl.h"
#include "sdl-video.h"

#include "../common/configSys.h"
#include "../../types.h"

extern double g_fpsScale;

extern bool MaxSpeed;

int isloaded;

bool turbo = false;

int closeFinishedMovie = 0;

int eoptions=0;

static int inited = 0;

static void DriverKill(void);
static int DriverInitialize(FCEUGI *gi);
uint64 FCEUD_GetTime();
int gametype = 0;

int pal_emulation;
int dendy;
bool swapDuty;

// -Video Modes Tag- : See --special
static const char *DriverUsage=
"Option         Value   Description\n"
"--pal          {0|1}   Use PAL timing.\n"
"--newppu       {0|1}   Enable the new PPU core. (WARNING: May break savestates)\n"
"--inputcfg     d       Configures input device d on startup.\n"
"--input(1,2)   d       Set which input device to emulate for input 1 or 2.\n"
"                         Devices:  gamepad zapper powerpad.0 powerpad.1\n"
"                         arkanoid\n"
"--input(3,4)   d       Set the famicom expansion device to emulate for\n"
"                       input(3, 4)\n"
"                          Devices: quizking hypershot mahjong toprider ftrainer\n"
"                          familykeyboard oekakids arkanoid shadow bworld\n"
"                          4player\n"
"--gamegenie    {0|1}   Enable emulated Game Genie.\n"
"--frameskip    x       Set # of frames to skip per emulated frame.\n"
"--xres         x       Set horizontal resolution for full screen mode.\n"
"--yres         x       Set vertical resolution for full screen mode.\n"
"--autoscale    {0|1}   Enable autoscaling in fullscreen. \n"
"--keepratio    {0|1}   Keep native NES aspect ratio when autoscaling. \n"
"--(x/y)scale   x       Multiply width/height by x. \n"
"                         (Real numbers >0 with OpenGL, otherwise integers >0).\n"
"--(x/y)stretch {0|1}   Stretch to fill surface on x/y axis (OpenGL only).\n"
"--bpp       {8|16|32}  Set bits per pixel.\n"
"--opengl       {0|1}   Enable OpenGL support.\n"
"--fullscreen   {0|1}   Enable full screen mode.\n"
"--noframe      {0|1}   Hide title bar and window decorations.\n"
"--special      {1-4}   Use special video scaling filters\n"
"                         (1 = hq2x; 2 = Scale2x; 3 = NTSC 2x; 4 = hq3x;\n"
"                         5 = Scale3x; 6 = Prescale2x; 7 = Prescale3x; 8=Precale4x; 9=PAL)\n"
"--palette      f       Load custom global palette from file f.\n"
"--sound        {0|1}   Enable sound.\n"
"--soundrate    x       Set sound playback rate to x Hz.\n"
"--soundq      {0|1|2}  Set sound quality. (0 = Low 1 = High 2 = Very High)\n"
"--soundbufsize x       Set sound buffer size to x ms.\n"
"--volume      {0-256}  Set volume to x.\n"
"--soundrecord  f       Record sound to file f.\n"
"--playmov      f       Play back a recorded FCM/FM2/FM3 movie from filename f.\n"
"--pauseframe   x       Pause movie playback at frame x.\n"
"--fcmconvert   f       Convert fcm movie file f to fm2.\n"
"--ripsubs      f       Convert movie's subtitles to srt\n"
"--subtitles    {0|1}   Enable subtitle display\n"
"--fourscore    {0|1}   Enable fourscore emulation\n"
"--no-config    {0|1}   Use default config file and do not save\n"
"--net          s       Connect to server 's' for TCP/IP network play.\n"
"--port         x       Use TCP/IP port x for network play.\n"
"--user         x       Set the nickname to use in network play.\n"
"--pass         x       Set password to use for connecting to the server.\n"
"--netkey       s       Use string 's' to create a unique session for the\n"
"                       game loaded.\n"
"--players      x       Set the number of local players in a network play\n"
"                       session.\n"
"--rp2mic       {0|1}   Replace Port 2 Start with microphone (Famicom).\n"
"--nogui                Don't load the GTK GUI\n"
"--4buttonexit {0|1}    exit the emulator when A+B+Select+Start is pressed\n"
"--loadstate {0-9|>9}   load from the given state when the game is loaded\n"
"--savestate {0-9|>9}   save to the given state when the game is closed\n"
"                         to not save/load automatically provide a number\n"
"                         greater than 9\n"
"--periodicsaves {0|1}  enable automatic periodic saving.  This will save to\n"
"                         the state passed to --savestate\n";


// these should be moved to the man file
//--nospritelim  {0|1}   Disables the 8 sprites per scanline limitation.\n
//--trianglevol {0-256}  Sets Triangle volume.\n
//--square1vol  {0-256}  Sets Square 1 volume.\n
//--square2vol  {0-256}  Sets Square 2 volume.\n
//--noisevol	{0-256}  Sets Noise volume.\n
//--pcmvol	  {0-256}  Sets PCM volume.\n
//--lowpass	  {0|1}   Enables low-pass filter if x is nonzero.\n
//--doublebuf	{0|1}   Enables SDL double-buffering if x is nonzero.\n
//--slend	  {0-239}   Sets the last drawn emulated scanline.\n
//--ntsccolor	{0|1}   Emulates an NTSC TV's colors.\n
//--hue		   x	  Sets hue for NTSC color emulation.\n
//--tint		  x	  Sets tint for NTSC color emulation.\n
//--slstart	{0-239}   Sets the first drawn emulated scanline.\n
//--clipsides	{0|1}   Clips left and rightmost 8 columns of pixels.\n

// global configuration object
Config *g_config;

static void ShowUsage(char *prog)
{
	printf("\nUsage is as follows:\n%s <options> filename\n\n",prog);
	puts(DriverUsage);
	puts("");
	printf("Compiled with SDL version %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL );
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_version* v; 
	SDL_GetVersion(v);
#else
	const SDL_version* v = SDL_Linked_Version();
#endif
	printf("Linked with SDL version %d.%d.%d\n", v->major, v->minor, v->patch);
}

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
	int id;
	g_config->getOption("SDL.PAL", &id);
	FCEUI_SetRegion(id);

	g_config->getOption("SDL.SwapDuty", &id);
	swapDuty = id;
	
	isloaded = 1;

	return 1;
}

/**
 * Closes a game.  Frees memory, and deinitializes the drivers.
 */
int
CloseGame()
{
	std::string filename;

	if(!isloaded) {
		return(0);
	}

	FCEUI_CloseGame();

	DriverKill();
	isloaded = 0;
	GameInfo = 0;

	g_config->getOption("SDL.Sound.RecordFile", &filename);

	InputUserActiveFix();
	return(1);
}

void FCEUD_Update(uint8 *XBuf, int32 *Buffer, int Count);

static void DoFun(int frameskip, int periodic_saves)
{
	uint8 *gfx;
	int32 *sound;
	int32 ssize;
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

	int fourscore=0;
	g_config->getOption("SDL.FourScore", &fourscore);
	eoptions &= ~EO_FOURSCORE;
	if(fourscore)
		eoptions |= EO_FOURSCORE;

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
	Count = (int)(Count / g_fpsScale);
	if(Count) {
		// don't underflow when scaling fps
		if(g_fpsScale>1.0) {
			if(XBuf && (inited&4) && !(NoWaiting & 2))
				BlitScreen(XBuf);
		}

	} else {
		if(!NoWaiting && (!(eoptions&EO_NOTHROTTLE) || FCEUI_EmulationPaused()))
		while (SpeedThrottle())
		{
			FCEUD_UpdateInput();
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
    return new EMUFILE_FILE(fn, m);
}

/**
 * Opens a file, C++ style, to be read a byte at a time.
 */
FILE *FCEUD_UTF8fopen(const char *fn, const char *mode)
{
	return(fopen(fn,mode));
}

static const char *s_linuxCompilerString = "g++ " __VERSION__;
/**
 * Returns the compiler string.
 */
const char *FCEUD_GetCompilerString() {
	return (const char *)s_linuxCompilerString;
}

/**
 * Unimplemented.
 */
void FCEUD_DebugBreakpoint() {
	return;
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
int main(int argc, char *argv[])
{
  argc = 2;
  const char *my_argv[] = {"fecux",
#ifdef __NO_FILE_SYSTEM__
    "100in1"
#else
    "100in1.nes"
#endif
  };
  argv = const_cast<char **>(my_argv);

	int error, frameskip;

	FCEUD_Message("Starting " FCEU_NAME_AND_VERSION "...\n");

	/* SDL_INIT_VIDEO Needed for (joystick config) event processing? */
	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
		return(-1);
	}

	// Initialize the configuration system
	g_config = InitConfig();
		
	if(!g_config) {
		SDL_Quit();
		return -1;
	}

	// initialize the infrastructure
	error = FCEUI_Initialize();
	if(error != 1) {
		ShowUsage(argv[0]);
		SDL_Quit();
		return -1;
	}
	
	int romIndex = g_config->parse(argc, argv);
  printf("romIndex = %d\n", romIndex);

	// This is here so that a default fceux.cfg will be created on first
	// run, even without a valid ROM to play.
	// Unless, of course, there's actually --no-config given
	// mbg 8/23/2008 - this is also here so that the inputcfg routines can have 
    // a chance to dump the new inputcfg to the fceux.cfg  in case you didnt 
    // specify a rom  filename
	//g_config->getOption("SDL.NoConfig", &noconfig);
	//if (!noconfig)
	//	g_config->save();
	
	std::string s;

	g_config->getOption("SDL.InputCfg", &s);
	if(s.size() != 0)
	{
	InitVideo(GameInfo);
	InputCfg(s);
	}

    // update the input devices
	UpdateInput(g_config);

	// If x/y res set to 0, store current display res in SDL.LastX/YRes
	int yres, xres;
	g_config->getOption("SDL.XResolution", &xres);
	g_config->getOption("SDL.YResolution", &yres);
#if SDL_VERSION_ATLEAST(2, 0, 0)
	// TODO _ SDL 2.0
#else
	const SDL_VideoInfo* vid_info = SDL_GetVideoInfo();
	if(xres == 0) 
    {
        if(vid_info != NULL)
        {
			g_config->setOption("SDL.LastXRes", vid_info->current_w);
        }
        else
        {
			g_config->setOption("SDL.LastXRes", 512);
        }
    }
	else
	{
		g_config->setOption("SDL.LastXRes", xres);
	}	
    if(yres == 0)
    {
        if(vid_info != NULL)
        {
			g_config->setOption("SDL.LastYRes", vid_info->current_h);
        }
        else
        {
			g_config->setOption("SDL.LastYRes", 448);
        }
    } 
	else
	{
		g_config->setOption("SDL.LastYRes", yres);
	}
#endif

	// update the emu core
	UpdateEMUCore(g_config);

  if(romIndex >= 0)
	{
		// load the specified game
		error = LoadGame(argv[romIndex]);
		if(error != 1) {
			DriverKill();
			SDL_Quit();
			return -1;
		}
		g_config->setOption("SDL.LastOpenFile", argv[romIndex]);
	}
	
    int periodic_saves = 0;
	
	g_config->getOption("SDL.Frameskip", &frameskip);
  //frameskip = 10;
	// loop playing the game
	while(GameInfo)
	{
		DoFun(frameskip, periodic_saves);
	}
	CloseGame();

	// exit the infrastructure
	FCEUI_Kill();
	SDL_Quit();
	return 0;
}

/**
 * Get the time in ticks.
 */
uint64
FCEUD_GetTime()
{
	return SDL_GetTicks();
}

/**
 * Get the tick frequency in Hz.
 */
uint64
FCEUD_GetTimeFreq(void)
{
	// SDL_GetTicks() is in milliseconds
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
	fputs(text, stdout);
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
	fprintf(stderr, "%s\n", errormsg);
}


// dummy functions

#define DUMMY(__f) \
    void __f(void) {\
        printf("%s\n", #__f);\
        FCEU_DispMessage("Not implemented.",0);\
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
FCEUFILE* FCEUD_OpenArchiveIndex(ArchiveScanRecord& asr, std::string &fname, int innerIndex) { return 0; }
FCEUFILE* FCEUD_OpenArchive(ArchiveScanRecord& asr, std::string& fname, std::string* innerFilename) { return 0; }
FCEUFILE* FCEUD_OpenArchiveIndex(ArchiveScanRecord& asr, std::string &fname, int innerIndex, int* userCancel) { return 0; }
FCEUFILE* FCEUD_OpenArchive(ArchiveScanRecord& asr, std::string& fname, std::string* innerFilename, int* userCancel) { return 0; }
ArchiveScanRecord FCEUD_ScanArchive(std::string fname) { return ArchiveScanRecord(); }

