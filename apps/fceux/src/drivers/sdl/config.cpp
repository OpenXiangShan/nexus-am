#include "main.h"
#include "throttle.h"
#include "config.h"

#include "input.h"
#include "dface.h"

#include "sdl.h"
#include "sdl-video.h"

/**
 * Read a custom pallete from a file and load it into the core.
 */
int
LoadCPalette(const std::string &file)
{
	uint8 tmpp[192];
	FILE *fp;

	if(!(fp = FCEUD_UTF8fopen(file.c_str(), "rb"))) {
		char errorMsg[256];
		strcpy(errorMsg, "Error loading custom palette from file: ");
		strcat(errorMsg, file.c_str());
		FCEUD_PrintError(errorMsg);
		return 0;
	}
	size_t result = fread(tmpp, 1, 192, fp);
	if(result != 192) {
		char errorMsg[256];
		strcpy(errorMsg, "Error loading custom palette from file: ");
		strcat(errorMsg, file.c_str());
		FCEUD_PrintError(errorMsg);
		return 0;
	}
	FCEUI_SetUserPalette(tmpp, result/3);
	fclose(fp);
	return 1;
}

/**
 * Attempts to locate FCEU's application directory.  This will
 * hopefully become obsolete once the new configuration system is in
 * place.
 */
static void
GetBaseDirectory(std::string &dir)
{
	char *home = getenv("HOME");
	if(home) {
		dir = std::string(home) + "/.fceux";
	} else {
		dir = "";
	}
}

// returns a config structure with default options
// also creates config base directory (ie: /home/user/.fceux as well as subdirs
Config *
InitConfig()
{
	std::string dir, prefix;
	Config *config;

	GetBaseDirectory(dir);

	config = new Config(dir);

	// sound options
	config->addOption('s', "sound", "SDL.Sound", 1);
	config->addOption("volume", "SDL.Sound.Volume", 150);
	config->addOption("trianglevol", "SDL.Sound.TriangleVolume", 256);
	config->addOption("square1vol", "SDL.Sound.Square1Volume", 256);
	config->addOption("square2vol", "SDL.Sound.Square2Volume", 256);
	config->addOption("noisevol", "SDL.Sound.NoiseVolume", 256);
	config->addOption("pcmvol", "SDL.Sound.PCMVolume", 256);
	config->addOption("soundrate", "SDL.Sound.Rate", 44100);
	config->addOption("soundq", "SDL.Sound.Quality", 1);
	config->addOption("soundrecord", "SDL.Sound.RecordFile", "");
	config->addOption("soundbufsize", "SDL.Sound.BufSize", 128);
	config->addOption("lowpass", "SDL.Sound.LowPass", 0);
    
	config->addOption('g', "gamegenie", "SDL.GameGenie", 0);
	config->addOption("pal", "SDL.PAL", 0);
	config->addOption("frameskip", "SDL.Frameskip", 0);
	config->addOption("clipsides", "SDL.ClipSides", 0);
	config->addOption("nospritelim", "SDL.DisableSpriteLimit", 1);
	config->addOption("swapduty", "SDL.SwapDuty", 0);

	// color control
	config->addOption('p', "palette", "SDL.Palette", "");
	config->addOption("tint", "SDL.Tint", 56);
	config->addOption("hue", "SDL.Hue", 72);
	config->addOption("ntsccolor", "SDL.NTSCpalette", 0);

	// scanline settings
	config->addOption("slstart", "SDL.ScanLineStart", 0);
	config->addOption("slend", "SDL.ScanLineEnd", 239);

	// video controls
	config->addOption('f', "fullscreen", "SDL.Fullscreen", 0);

	// set x/y res to 0 for automatic fullscreen resolution detection (no change)
	config->addOption('x', "xres", "SDL.XResolution", 0);
	config->addOption('y', "yres", "SDL.YResolution", 0);
	config->addOption("SDL.LastXRes", 0);
	config->addOption("SDL.LastYRes", 0);
	config->addOption('b', "bpp", "SDL.BitsPerPixel", 32);
	config->addOption("doublebuf", "SDL.DoubleBuffering", 0);
	config->addOption("autoscale", "SDL.AutoScale", 1);
	config->addOption("keepratio", "SDL.KeepRatio", 1);
	config->addOption("xscale", "SDL.XScale", 1.0);
	config->addOption("yscale", "SDL.YScale", 1.0);
	config->addOption("xstretch", "SDL.XStretch", 0);
	config->addOption("ystretch", "SDL.YStretch", 0);
	config->addOption("noframe", "SDL.NoFrame", 0);
	config->addOption("special", "SDL.SpecialFilter", 0);
	config->addOption("showfps", "SDL.ShowFPS", 0);
	config->addOption("togglemenu", "SDL.ToggleMenu", 0);

	// OpenGL options
	config->addOption("opengl", "SDL.OpenGL", 0);
	config->addOption("openglip", "SDL.OpenGLip", 0);
	config->addOption("SDL.SpecialFilter", 0);
	config->addOption("SDL.SpecialFX", 0);
	config->addOption("SDL.Vsync", 1);

	// network play options - netplay is broken
	config->addOption("server", "SDL.NetworkIsServer", 0);
	config->addOption('n', "net", "SDL.NetworkIP", "");
	config->addOption('u', "user", "SDL.NetworkUsername", "");
	config->addOption('w', "pass", "SDL.NetworkPassword", "");
	config->addOption('k', "netkey", "SDL.NetworkGameKey", "");
	config->addOption("port", "SDL.NetworkPort", 4046);
	config->addOption("players", "SDL.NetworkPlayers", 1);
     
	// input configuration options
	config->addOption("input1", "SDL.Input.0", "GamePad.0");
	config->addOption("input2", "SDL.Input.1", "GamePad.1");
	config->addOption("input3", "SDL.Input.2", "Gamepad.2");
	config->addOption("input4", "SDL.Input.3", "Gamepad.3");

	// allow for input configuration
	config->addOption('i', "inputcfg", "SDL.InputCfg", InputCfg);
    
	// display input
	config->addOption("inputdisplay", "SDL.InputDisplay", 0);

	// enable / disable opposite directionals (left + right or up + down simultaneously)
	config->addOption("opposite-directionals", "SDL.Input.EnableOppositeDirectionals", 1);
    
	// pause movie playback at frame x
	config->addOption("pauseframe", "SDL.PauseFrame", 0);
	config->addOption("recordhud", "SDL.RecordHUD", 1);
	config->addOption("moviemsg", "SDL.MovieMsg", 1);
    
	// overwrite the config file?
	config->addOption("no-config", "SDL.NoConfig", 0);

	config->addOption("autoresume", "SDL.AutoResume", 0);
    
	// video playback
	config->addOption("playmov", "SDL.Movie", "");
	config->addOption("subtitles", "SDL.SubtitleDisplay", 1);
	config->addOption("movielength", "SDL.MovieLength", 0);
	
	config->addOption("fourscore", "SDL.FourScore", 0);

	config->addOption("nofscursor", "SDL.NoFullscreenCursor", 1);
    
    // auto load/save on gameload/close
	config->addOption("loadstate", "SDL.AutoLoadState", INVALID_STATE);
	config->addOption("savestate", "SDL.AutoSaveState", INVALID_STATE);

    //TODO implement this
    config->addOption("periodicsaves", "SDL.PeriodicSaves", 0);

	// GamePad 0 - 3
	for(unsigned int i = 0; i < GAMEPAD_NUM_DEVICES; i++) {
		char buf[64];
		snprintf(buf, 20, "SDL.Input.GamePad.%d.", i);
		prefix = buf;

		config->addOption(prefix + "DeviceType", DefaultGamePadDevice[i]);
		config->addOption(prefix + "DeviceNum",  0);
		for(unsigned int j = 0; j < GAMEPAD_NUM_BUTTONS; j++) {
			config->addOption(prefix + GamePadNames[j], DefaultGamePad[i][j]);
		}
	}
    
	return config;
}

void
UpdateEMUCore(Config *config)
{
	int ntsccol, ntsctint, ntschue, flag, region, start, end;
	std::string cpalette;

	config->getOption("SDL.NTSCpalette", &ntsccol);
	config->getOption("SDL.Tint", &ntsctint);
	config->getOption("SDL.Hue", &ntschue);
	FCEUI_SetNTSCTH(ntsccol, ntsctint, ntschue);

	config->getOption("SDL.Palette", &cpalette);
	if(cpalette.size()) {
		LoadCPalette(cpalette);
	}

	config->getOption("SDL.PAL", &region);
	FCEUI_SetRegion(region);

	config->getOption("SDL.DisableSpriteLimit", &flag);
	FCEUI_DisableSpriteLimitation(flag ? 1 : 0);

	config->getOption("SDL.ScanLineStart", &start);
	config->getOption("SDL.ScanLineEnd", &end);

#if DOING_SCANLINE_CHECKS
	for(int i = 0; i < 2; x++) {
		if(srendlinev[x]<0 || srendlinev[x]>239) srendlinev[x]=0;
		if(erendlinev[x]<srendlinev[x] || erendlinev[x]>239) erendlinev[x]=239;
	}
#endif

	FCEUI_SetRenderedLines(start + 8, end - 8, start, end);
}
