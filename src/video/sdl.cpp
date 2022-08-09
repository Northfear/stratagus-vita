//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name sdl.cpp - SDL video support. */
//
//      (c) Copyright 1999-2011 by Lutz Sammer, Jimmy Salmon, Nehal Mistry and
//                                 Pali Rohár
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"
#include "online_service.h"

#ifdef DEBUG
#include <signal.h>
#endif

#include <map>
#include <string>
#include <vector>


#include <limits.h>
#include <math.h>

#ifndef USE_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "SDL.h"
#include "SDL_syswm.h"

#ifdef USE_BEOS
#include <sys/socket.h>
#endif

#ifdef USE_WIN32
#include <shellapi.h>
#endif

#include "editor.h"
#include "font.h"
#include "game.h"
#include "interface.h"
#include "minimap.h"
#include "network.h"
#include "parameters.h"
#include "sound.h"
#include "sound_server.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "video.h"
#include "widgets.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

SDL_Window *TheWindow; /// Internal screen
SDL_Renderer *TheRenderer = NULL; /// Internal screen
SDL_Texture *TheTexture; /// Internal screen
SDL_Surface *TheScreen; /// Internal screen

static SDL_Rect Rects[100];
static int NumRects;

static std::map<int, std::string> Key2Str;
static std::map<std::string, int> Str2Key;

double FrameTicks;     /// Frame length in ms

const EventCallback *Callbacks;

bool IsSDLWindowVisible = true;

/// Just a counter to cache window data on in other places when the size changes
uint8_t SizeChangeCounter = 0;

static bool dummyRenderer = false;

uint32_t SDL_CUSTOM_KEY_UP;

#ifdef __vita__
// used to convert user-friendly pointer speed values into more useable ones
const double CONTROLLER_SPEED_MOD = 2000000.0;
// bigger value correndsponds to faster pointer movement speed with bigger stick axis values
const double CONTROLLER_AXIS_SPEEDUP = 1.03;

enum
{
	CONTROLLER_L_DEADZONE = 3000,
	CONTROLLER_R_DEADZONE = 25000
};

SDL_GameController* gameController = nullptr;
int16_t controllerLeftXAxis = 0;
int16_t controllerLeftYAxis = 0;
int16_t controllerRightXAxis = 0;
int16_t controllerRightYAxis = 0;
uint32_t lastControllerTime = 0;
float emulatedPointerPosX;
float emulatedPointerPosY;
SDL_FingerID firstFingerId = 0;
int16_t numTouches = 0;
bool leftScrollActive = false;
bool rightScrollActive = false;
bool upScrollActive = false;
bool downScrollActive = false;
float cursorSpeedup = 1.0f;

void OpenController()
{
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            gameController = SDL_GameControllerOpen(i);
        }
    }
}

void CloseController()
{
    if (SDL_GameControllerGetAttached(gameController)) {
        SDL_GameControllerClose(gameController);
        gameController = nullptr;
    }
}
#endif

/*----------------------------------------------------------------------------
--  Sync
----------------------------------------------------------------------------*/

/**
**  Initialise video sync.
**  Calculate the length of video frame and any simulation skips.
**
**  @see VideoSyncSpeed @see SkipFrames @see FrameTicks
*/
void SetVideoSync()
{
	double ms;

	if (VideoSyncSpeed) {
		ms = (1000.0 * 1000.0 / CYCLES_PER_SECOND) / VideoSyncSpeed;
	} else {
		ms = (double)INT_MAX;
	}
	SkipFrames = ms / 400;
	while (SkipFrames && ms / SkipFrames < 200) {
		--SkipFrames;
	}
	ms /= SkipFrames + 1;

	FrameTicks = ms / 10;
	DebugPrint("frames %d - %5.2fms\n" _C_ SkipFrames _C_ ms / 10);
}

/*----------------------------------------------------------------------------
--  Video
----------------------------------------------------------------------------*/

#if defined(DEBUG) && !defined(USE_WIN32)
static void CleanExit(int)
{
	// Clean SDL
	SDL_Quit();
	// Reestablish normal behaviour for next abort call
	signal(SIGABRT, SIG_DFL);
	// Generates a core dump
	abort();
}
#endif

/**
**  Initialize SDLKey to string map
*/
static void InitKey2Str()
{
	Str2Key[_("esc")] = SDLK_ESCAPE;

	if (!Key2Str.empty()) {
		return;
	}

	int i;
	char str[20];

	Key2Str[SDLK_BACKSPACE] = "backspace";
	Key2Str[SDLK_TAB] = "tab";
	Key2Str[SDLK_CLEAR] = "clear";
	Key2Str[SDLK_RETURN] = "return";
	Key2Str[SDLK_PAUSE] = "pause";
	Key2Str[SDLK_ESCAPE] = "escape";
	Key2Str[SDLK_SPACE] = " ";
	Key2Str[SDLK_EXCLAIM] = "!";
	Key2Str[SDLK_QUOTEDBL] = "\"";
	Key2Str[SDLK_HASH] = "#";
	Key2Str[SDLK_DOLLAR] = "$";
	Key2Str[SDLK_AMPERSAND] = "&";
	Key2Str[SDLK_QUOTE] = "'";
	Key2Str[SDLK_LEFTPAREN] = "(";
	Key2Str[SDLK_RIGHTPAREN] = ")";
	Key2Str[SDLK_ASTERISK] = "*";
	Key2Str[SDLK_PLUS] = "+";
	Key2Str[SDLK_COMMA] = ",";
	Key2Str[SDLK_MINUS] = "-";
	Key2Str[SDLK_PERIOD] = ".";
	Key2Str[SDLK_SLASH] = "/";

	str[1] = '\0';
	for (i = SDLK_0; i <= SDLK_9; ++i) {
		str[0] = i;
		Key2Str[i] = str;
	}

	Key2Str[SDLK_COLON] = ":";
	Key2Str[SDLK_SEMICOLON] = ";";
	Key2Str[SDLK_LESS] = "<";
	Key2Str[SDLK_EQUALS] = "=";
	Key2Str[SDLK_GREATER] = ">";
	Key2Str[SDLK_QUESTION] = "?";
	Key2Str[SDLK_AT] = "@";
	Key2Str[SDLK_LEFTBRACKET] = "[";
	Key2Str[SDLK_BACKSLASH] = "\\";
	Key2Str[SDLK_RIGHTBRACKET] = "]";
	Key2Str[SDLK_BACKQUOTE] = "`";

	str[1] = '\0';
	for (i = SDLK_a; i <= SDLK_z; ++i) {
		str[0] = i;
		Key2Str[i] = str;
	}

	Key2Str[SDLK_DELETE] = "delete";

	for (i = SDLK_KP_0; i <= SDLK_KP_9; ++i) {
		snprintf(str, sizeof(str), "kp_%d", i - SDLK_KP_0);
		Key2Str[i] = str;
	}

	Key2Str[SDLK_KP_PERIOD] = "kp_period";
	Key2Str[SDLK_KP_DIVIDE] = "kp_divide";
	Key2Str[SDLK_KP_MULTIPLY] = "kp_multiply";
	Key2Str[SDLK_KP_MINUS] = "kp_minus";
	Key2Str[SDLK_KP_PLUS] = "kp_plus";
	Key2Str[SDLK_KP_ENTER] = "kp_enter";
	Key2Str[SDLK_KP_EQUALS] = "kp_equals";
	Key2Str[SDLK_UP] = "up";
	Key2Str[SDLK_DOWN] = "down";
	Key2Str[SDLK_RIGHT] = "right";
	Key2Str[SDLK_LEFT] = "left";
	Key2Str[SDLK_INSERT] = "insert";
	Key2Str[SDLK_HOME] = "home";
	Key2Str[SDLK_END] = "end";
	Key2Str[SDLK_PAGEUP] = "pageup";
	Key2Str[SDLK_PAGEDOWN] = "pagedown";

	for (i = SDLK_F1; i <= SDLK_F15; ++i) {
		snprintf(str, sizeof(str), "f%d", i - SDLK_F1 + 1);
		Key2Str[i] = str;
		snprintf(str, sizeof(str), "F%d", i - SDLK_F1 + 1);
		Str2Key[str] = i;
	}

	Key2Str[SDLK_HELP] = "help";
	Key2Str[SDLK_PRINTSCREEN] = "print";
	Key2Str[SDLK_SYSREQ] = "sysreq";
	Key2Str[SDLK_PAUSE] = "break";
	Key2Str[SDLK_MENU] = "menu";
	Key2Str[SDLK_POWER] = "power";
	//Key2Str[SDLK_EURO] = "euro";
	Key2Str[SDLK_UNDO] = "undo";
}

#ifdef USE_WIN32
typedef enum PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

static void setDpiAware() {
	void* userDLL;
	BOOL(WINAPI *SetProcessDPIAware)(void); // Vista and later
	void* shcoreDLL;
	HRESULT(WINAPI *SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS dpiAwareness); // Windows 8.1 and later

	userDLL = SDL_LoadObject("USER32.DLL");
	if (userDLL) {
		SetProcessDPIAware = (BOOL(WINAPI *)(void)) SDL_LoadFunction(userDLL, "SetProcessDPIAware");
	} else {
		SetProcessDPIAware = NULL;
	}

	shcoreDLL = SDL_LoadObject("SHCORE.DLL");
	if (shcoreDLL) {
		SetProcessDpiAwareness = (HRESULT(WINAPI *)(PROCESS_DPI_AWARENESS)) SDL_LoadFunction(shcoreDLL, "SetProcessDpiAwareness");
	} else {
		SetProcessDpiAwareness = NULL;
	}

	if (SetProcessDpiAwareness) {
		/* Try Windows 8.1+ version */
		HRESULT result = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		DebugPrint("called SetProcessDpiAwareness: %d" _C_ (result == S_OK) ? 1 : 0);
	} else {
		if (SetProcessDPIAware) {
			/* Try Vista - Windows 8 version.
			   This has a constant scale factor for all monitors.
			*/
			BOOL success = SetProcessDPIAware();
			DebugPrint("called SetProcessDPIAware: %d" _C_ (int)success);
		}
		// In any case, on these old Windows versions we have to do a bit of
		// compatibility hacking. Windows 7 and below don't play well with
		// opengl rendering and (for some odd reason) fullscreen.
		fprintf(stdout, "\n!!! Detected old Windows version - forcing software renderer and windowed mode !!!\n\n");
		SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE);
		VideoForceFullScreen = 1;
		Video.FullScreen = 0;
	}

}
#else
static void setDpiAware() {
}
#endif

/**
**  Initialize the video part for SDL.
*/
void InitVideoSdl()
{
	Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI;

	if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
		// Fix tablet input in full-screen mode
		SDL_setenv("SDL_MOUSE_RELATIVE", "0", 1);
		int res = SDL_Init(
					  SDL_INIT_AUDIO | SDL_INIT_VIDEO |
					  SDL_INIT_EVENTS | SDL_INIT_TIMER);
		if (res < 0) {
			fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
			exit(1);
		}

		SDL_CUSTOM_KEY_UP = SDL_RegisterEvents(1);
		SDL_StartTextInput();

		// Clean up on exit
		atexit(SDL_Quit);

		// If debug is enabled, Stratagus disable SDL Parachute.
		// So we need gracefully handle segfaults and aborts.
#if defined(DEBUG) && !defined(USE_WIN32)
		signal(SIGSEGV, CleanExit);
		signal(SIGABRT, CleanExit);
#endif
#ifdef __vita__
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
	SDL_Init(SDL_INIT_GAMECONTROLLER);
	OpenController();
#endif
	}

	// Initialize the display

	setDpiAware();

	// Sam said: better for windows.
	/* SDL_HWSURFACE|SDL_HWPALETTE | */
	if (Video.FullScreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else {
		flags |= SDL_WINDOW_RESIZABLE;
	}

	if (!Video.Width || !Video.Height) {
		Video.Width = 640;
		Video.Height = 480;
	}

	if (!Video.WindowWidth || !Video.WindowHeight) {
		Video.WindowWidth = Video.Width;
		Video.WindowHeight = Video.Height;
	}

	if (!Video.Depth) {
		Video.Depth = 32;
	}

	const char *win_title = "Stratagus";
	// Set WindowManager Title
	if (!FullGameName.empty()) {
		win_title = FullGameName.c_str();
	} else if (!Parameters::Instance.applicationName.empty()) {
		win_title = Parameters::Instance.applicationName.c_str();
	}

	TheWindow = SDL_CreateWindow(win_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                             Video.WindowWidth, Video.WindowHeight, flags);
	if (TheWindow == NULL) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
				Video.Width, Video.Height, Video.Depth, SDL_GetError());
		exit(1);
	}
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	int rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
	if (!Parameters::Instance.benchmark) {
		rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
	}
	if (!TheRenderer) {
		TheRenderer = SDL_CreateRenderer(TheWindow, -1, rendererFlags);
	}
	SDL_RendererInfo rendererInfo;
	if (!SDL_GetRendererInfo(TheRenderer, &rendererInfo)) {
		printf("[Renderer] %s\n", rendererInfo.name);
		if (strlen(rendererInfo.name) == 0) {
			dummyRenderer = true;
		}
		if(!strncmp(rendererInfo.name, "opengl", 6)) {
			LoadShaderExtensions();
		}
	}
	SDL_SetRenderDrawColor(TheRenderer, 0, 0, 0, 255);
	Video.ResizeScreen(Video.Width, Video.Height);

// #ifdef USE_WIN32
// 	HWND hwnd = NULL;
// 	HICON hicon = NULL;
// 	SDL_SysWMinfo info;
// 	SDL_VERSION(&info.version);

// 	if (SDL_GetWindowWMInfo(TheWindow, &info)) {
// 		hwnd = info.win.window;
// 	}

// 	if (hwnd) {
// 		hicon = ExtractIcon(GetModuleHandle(NULL), Parameters::Instance.applicationName.c_str(), 0);
// 	}

// 	if (hicon) {
// 		SendMessage(hwnd, (UINT)WM_SETICON, ICON_SMALL, (LPARAM)hicon);
// 		SendMessage(hwnd, (UINT)WM_SETICON, ICON_BIG, (LPARAM)hicon);
// 	}
// #endif

#if ! defined(USE_WIN32) && ! defined(USE_MAEMO)

		SDL_Surface *icon = NULL;
		CGraphic *g = NULL;
		struct stat st;

		std::string FullGameNameL = FullGameName;
		for (size_t i = 0; i < FullGameNameL.size(); ++i) {
			FullGameNameL[i] = tolower(FullGameNameL[i]);
		}

		std::string ApplicationName = Parameters::Instance.applicationName;
		std::string ApplicationNameL = ApplicationName;
		for (size_t i = 0; i < ApplicationNameL.size(); ++i) {
			ApplicationNameL[i] = tolower(ApplicationNameL[i]);
		}

		std::vector <std::string> pixmaps;
		pixmaps.push_back(std::string() + PIXMAPS + "/" + FullGameName + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + FullGameNameL + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + FullGameName + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + FullGameNameL + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + ApplicationName + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + ApplicationNameL + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + ApplicationName + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + ApplicationNameL + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + "Stratagus" + ".png");
		pixmaps.push_back(std::string() + PIXMAPS + "/" + "stratagus" + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + "Stratagus" + ".png");
		pixmaps.push_back(std::string() + "/usr/share/pixmaps" + "/" + "stratagus" + ".png");

		for (size_t i = 0; i < pixmaps.size(); ++i) {
			if (stat(pixmaps[i].c_str(), &st) == 0) {
				if (g) { CGraphic::Free(g); }
				g = CGraphic::New(pixmaps[i].c_str());
				g->Load();
				icon = g->Surface;
				if (icon) { break; }
			}
		}

		if (icon) {
			SDL_SetWindowIcon(TheWindow, icon);
		}

		if (g) {
			CGraphic::Free(g);
		}

#endif
	Video.FullScreen = (SDL_GetWindowFlags(TheWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 1 : 0;
	Video.Depth = TheScreen->format->BitsPerPixel;

	// Must not allow SDL to switch to relative mouse coordinates when going
	// fullscreen. So we don't hide the cursor, but instead set a transparent
	// 1px cursor
	Uint8 emptyCursor[] = {'\0'};
	Video.blankCursor = SDL_CreateCursor(emptyCursor, emptyCursor, 1, 1, 0, 0);
	SDL_SetCursor(Video.blankCursor);

	InitKey2Str();

	ColorBlack = Video.MapRGB(TheScreen->format, 0, 0, 0);
	ColorDarkGreen = Video.MapRGB(TheScreen->format, 48, 100, 4);
	ColorLightBlue = Video.MapRGB(TheScreen->format, 52, 113, 166);
	ColorBlue = Video.MapRGB(TheScreen->format, 0, 0, 252);
	ColorOrange = Video.MapRGB(TheScreen->format, 248, 140, 20);
	ColorWhite = Video.MapRGB(TheScreen->format, 252, 248, 240);
	ColorLightGray = Video.MapRGB(TheScreen->format, 192, 192, 192);
	ColorGray = Video.MapRGB(TheScreen->format, 128, 128, 128);
	ColorDarkGray = Video.MapRGB(TheScreen->format, 64, 64, 64);
	ColorRed = Video.MapRGB(TheScreen->format, 252, 0, 0);
	ColorGreen = Video.MapRGB(TheScreen->format, 0, 252, 0);
	ColorYellow = Video.MapRGB(TheScreen->format, 252, 252, 0);

	UI.MouseWarpPos.x = UI.MouseWarpPos.y = -1;
}

/**
**  Check if a resolution is valid
**
**  @param w  Width
**  @param h  Height
*/
int VideoValidResolution(int w, int h)
{
	return 1;
}

/**
**  Invalidate some area
**
**  @param x  screen pixel X position.
**  @param y  screen pixel Y position.
**  @param w  width of rectangle in pixels.
**  @param h  height of rectangle in pixels.
*/
void InvalidateArea(int x, int y, int w, int h)
{
	Assert(NumRects != sizeof(Rects) / sizeof(*Rects));
	Assert(x >= 0 && y >= 0 && x + w <= Video.Width && y + h <= Video.Height);
	Rects[NumRects].x = x;
	Rects[NumRects].y = y;
	Rects[NumRects].w = w;
	Rects[NumRects].h = h;
	++NumRects;
}

/**
**  Invalidate whole window
*/
void Invalidate()
{
	Rects[0].x = 0;
	Rects[0].y = 0;
	Rects[0].w = Video.Width;
	Rects[0].h = Video.Height;
	NumRects = 1;
}

static bool isTextInput(int key) {
	return key >= 32 && key <= 128 && !(KeyModifiers & (ModifierAlt | ModifierControl | ModifierSuper));
}

#ifdef __vita__
void HandleTouchEvent(const EventCallback &callbacks, const SDL_TouchFingerEvent& event)
{
	// ignore back touchpad
	if (event.touchId != 0)
		return;

	if (event.type == SDL_FINGERDOWN) {
		++numTouches;
		if (numTouches == 1) {
			firstFingerId = event.fingerId;
		}
	} else if (event.type == SDL_FINGERUP) {
		--numTouches;
	}

	if (firstFingerId == event.fingerId) {
		emulatedPointerPosX =
			static_cast<float>(VITA_FULLSCREEN_WIDTH * event.x - Video.RenderRect.x) * (static_cast<float>(Video.Width) / Video.RenderRect.w);
		emulatedPointerPosY = static_cast<float>(VITA_FULLSCREEN_HEIGHT * event.y - Video.RenderRect.y)
							  * (static_cast<float>(Video.Height) / Video.RenderRect.h);

		if (emulatedPointerPosX < 0)
			emulatedPointerPosX = 0;
		else if (emulatedPointerPosX >= Video.Width)
			emulatedPointerPosX = Video.Width - 1;

		if (emulatedPointerPosY < 0)
			emulatedPointerPosY = 0;
		else if (emulatedPointerPosY >= Video.Height)
			emulatedPointerPosY = Video.Height - 1;

		InputMouseMove(callbacks, SDL_GetTicks(), emulatedPointerPosX, emulatedPointerPosY);

		if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERUP) {
			SDL_Event ev;
			ev.type = (event.type == SDL_FINGERDOWN) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
			ev.button.button = SDL_BUTTON_LEFT;
			ev.button.x = emulatedPointerPosX;
			ev.button.y = emulatedPointerPosY;
			SDL_PushEvent(&ev);
		}
	}
}

void ProcessControllerAxisMotion()
{
    const uint32_t currentTime = SDL_GetTicks();
    const double deltaTime = currentTime - lastControllerTime;
    lastControllerTime = currentTime;

    if (controllerLeftXAxis != 0 || controllerLeftYAxis != 0) {
        const int16_t xSign = (controllerLeftXAxis > 0) - (controllerLeftXAxis < 0);
        const int16_t ySign = (controllerLeftYAxis > 0) - (controllerLeftYAxis < 0);
		float resolutionSpeedMod = static_cast<float>(Video.Height) / 480;

        emulatedPointerPosX += std::pow(std::abs(controllerLeftXAxis), CONTROLLER_AXIS_SPEEDUP) * xSign * deltaTime
                               * Video.ControllerPointerSpeed / CONTROLLER_SPEED_MOD * resolutionSpeedMod * cursorSpeedup;
        emulatedPointerPosY += std::pow(std::abs(controllerLeftYAxis), CONTROLLER_AXIS_SPEEDUP) * ySign * deltaTime
                               * Video.ControllerPointerSpeed / CONTROLLER_SPEED_MOD * resolutionSpeedMod * cursorSpeedup;

        if (emulatedPointerPosX < 0)
            emulatedPointerPosX = 0;
        else if (emulatedPointerPosX >= Video.Width)
            emulatedPointerPosX = Video.Width - 1;

        if (emulatedPointerPosY < 0)
            emulatedPointerPosY = 0;
        else if (emulatedPointerPosY >= Video.Height)
            emulatedPointerPosY = Video.Height - 1;

        InputMouseMove(*GetCallbacks(), SDL_GetTicks(), emulatedPointerPosX, emulatedPointerPosY);
    }
}

void HandleControllerAxisEvent(const SDL_ControllerAxisEvent& motion)
{
    if (motion.axis == SDL_CONTROLLER_AXIS_LEFTX) {
        if (std::abs(motion.value) > CONTROLLER_L_DEADZONE)
            controllerLeftXAxis = motion.value;
        else
            controllerLeftXAxis = 0;
    } else if (motion.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        if (std::abs(motion.value) > CONTROLLER_L_DEADZONE)
            controllerLeftYAxis = motion.value;
        else
            controllerLeftYAxis = 0;
    } else if (motion.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
        if (std::abs(motion.value) > CONTROLLER_R_DEADZONE)
            controllerRightXAxis = motion.value;
        else
            controllerRightXAxis = 0;
    } else if (motion.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
        if (std::abs(motion.value) > CONTROLLER_R_DEADZONE)
            controllerRightYAxis = motion.value;
        else
            controllerRightYAxis = 0;
    }

	//map scroll
	if (controllerRightXAxis > CONTROLLER_R_DEADZONE)
	{
		if (!rightScrollActive)
		{
			rightScrollActive = true;
			SDL_Event ev;
			ev.type = SDL_KEYDOWN;
			ev.key.state = SDL_PRESSED;
			ev.key.keysym.mod = KMOD_NONE;
			ev.key.keysym.sym = SDLK_RIGHT;
			SDL_PushEvent(&ev);
		}
	}
	else if (rightScrollActive)
	{
		rightScrollActive = false;
		SDL_Event ev;
		ev.type = SDL_KEYUP;
		ev.key.state = SDL_RELEASED;
		ev.key.keysym.mod = KMOD_NONE;
		ev.key.keysym.sym = SDLK_RIGHT;
		SDL_PushEvent(&ev);
	}

	if (controllerRightXAxis < -CONTROLLER_R_DEADZONE)
	{
		if (!leftScrollActive)
		{
			leftScrollActive = true;
			SDL_Event ev;
			ev.type = SDL_KEYDOWN;
			ev.key.state = SDL_PRESSED;
			ev.key.keysym.mod = KMOD_NONE;
			ev.key.keysym.sym = SDLK_LEFT;
			SDL_PushEvent(&ev);
		}
	}
	else if (leftScrollActive)
	{
		leftScrollActive = false;
		SDL_Event ev;
		ev.type = SDL_KEYUP;
		ev.key.state = SDL_RELEASED;
		ev.key.keysym.mod = KMOD_NONE;
		ev.key.keysym.sym = SDLK_LEFT;
		SDL_PushEvent(&ev);
	}

	if (controllerRightYAxis > CONTROLLER_R_DEADZONE)
	{
		if (!upScrollActive)
		{
			upScrollActive = true;
			SDL_Event ev;
			ev.type = SDL_KEYDOWN;
			ev.key.state = SDL_PRESSED;
			ev.key.keysym.mod = KMOD_NONE;
			ev.key.keysym.sym = SDLK_DOWN;
			SDL_PushEvent(&ev);
		}
	}
	else if (upScrollActive)
	{
		upScrollActive = false;
		SDL_Event ev;
		ev.type = SDL_KEYUP;
		ev.key.state = SDL_RELEASED;
		ev.key.keysym.mod = KMOD_NONE;
		ev.key.keysym.sym = SDLK_DOWN;
		SDL_PushEvent(&ev);
	}

	if (controllerRightYAxis < -CONTROLLER_R_DEADZONE)
	{
		if (!downScrollActive)
		{
			downScrollActive = true;
			SDL_Event ev;
			ev.type = SDL_KEYDOWN;
			ev.key.state = SDL_PRESSED;
			ev.key.keysym.mod = KMOD_NONE;
			ev.key.keysym.sym = SDLK_UP;
			SDL_PushEvent(&ev);
		}
	}
	else if (downScrollActive)
	{
		downScrollActive = false;
		SDL_Event ev;
		ev.type = SDL_KEYUP;
		ev.key.state = SDL_RELEASED;
		ev.key.keysym.mod = KMOD_NONE;
		ev.key.keysym.sym = SDLK_UP;
		SDL_PushEvent(&ev);
	}
}

void HandleControllerButtonEvent(const EventCallback &callbacks, const SDL_ControllerButtonEvent& button)
{
    bool keyboardPress = false;
    bool mousePress = false;
    Uint8 mouseBtn;
    SDL_Scancode scancode;
	SDL_Keycode keycode;

    switch (button.button) {
    case SDL_CONTROLLER_BUTTON_A:
        mousePress = true;
        mouseBtn = SDL_BUTTON_LEFT;
        break;
    case SDL_CONTROLLER_BUTTON_B:
        mousePress = true;
        mouseBtn = SDL_BUTTON_RIGHT;
        break;
    case SDL_CONTROLLER_BUTTON_X:
        keyboardPress = true;
        scancode = SDL_SCANCODE_A;
		keycode = SDLK_a;
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        keyboardPress = true;
        scancode = SDL_SCANCODE_S;
		keycode = SDLK_s;
        break;
    case SDL_CONTROLLER_BUTTON_BACK:
        keyboardPress = true;
        scancode = SDL_SCANCODE_F10;
		keycode = SDLK_F10;
        break;
    case SDL_CONTROLLER_BUTTON_START:
        keyboardPress = true;
        scancode = SDL_SCANCODE_ESCAPE;
		keycode = SDLK_ESCAPE;
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        keyboardPress = true;
        scancode = SDL_SCANCODE_LCTRL;
		keycode = SDLK_LCTRL;
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        keyboardPress = true;
        scancode = SDL_SCANCODE_RSHIFT;
		keycode = SDLK_RSHIFT;
		if (button.type == SDL_CONTROLLERBUTTONDOWN) {
			cursorSpeedup = 2.0f;
		} else {
			cursorSpeedup = 1.0f;
		}
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        keyboardPress = true;
        scancode = SDL_SCANCODE_1;
		keycode = SDLK_1;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        keyboardPress = true;
        scancode = SDL_SCANCODE_2;
		keycode = SDLK_2;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        keyboardPress = true;
        scancode = SDL_SCANCODE_3;
		keycode = SDLK_3;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        keyboardPress = true;
		scancode = SDL_SCANCODE_4;
		keycode = SDLK_4;
        break;
    default:
        break;
    }

    if (keyboardPress) {
		if (button.type == SDL_CONTROLLERBUTTONDOWN) {
			InputKeyButtonPress(callbacks, SDL_GetTicks(), keycode, keycode);
		} else {
			InputKeyButtonRelease(callbacks, SDL_GetTicks(), keycode, keycode);
		}

		if (&callbacks == GetCallbacks()) {
			SDL_Event ev;
			ev.type = (button.type == SDL_CONTROLLERBUTTONDOWN) ? SDL_KEYDOWN : SDL_KEYUP;
			ev.key.state = (button.type == SDL_CONTROLLERBUTTONDOWN) ? SDL_PRESSED : SDL_RELEASED;
			ev.key.keysym.mod = KMOD_NONE;
			ev.key.keysym.scancode = scancode;
			ev.key.keysym.sym = keycode;
			SDL_PushEvent(&ev);
			handleInput(&ev);
		}
    } else if (mousePress) {
		SDL_Event ev;
		ev.type = (button.type == SDL_CONTROLLERBUTTONDOWN) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
		ev.button.button = mouseBtn;
		ev.button.x = emulatedPointerPosX;
		ev.button.y = emulatedPointerPosY;
		SDL_PushEvent(&ev);
    }
}
#endif

/**
**  Handle interactive input event.
**
**  @param callbacks  Callback structure for events.
**  @param event      SDL event structure pointer.
*/
static void SdlDoEvent(const EventCallback &callbacks, SDL_Event &event)
{
	unsigned int keysym = 0;

	switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
			event.button.y = static_cast<int>(std::floor(event.button.y / Video.VerticalPixelSize + 0.5));
			InputMouseButtonPress(callbacks, SDL_GetTicks(), event.button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			event.button.y = static_cast<int>(std::floor(event.button.y / Video.VerticalPixelSize + 0.5));
			InputMouseButtonRelease(callbacks, SDL_GetTicks(), event.button.button);
			break;

		case SDL_MOUSEMOTION:
			event.motion.y = static_cast<int>(std::floor(event.button.y / Video.VerticalPixelSize + 0.5));
			InputMouseMove(callbacks, SDL_GetTicks(), event.motion.x, event.motion.y);
			break;

		case SDL_MOUSEWHEEL:
			{   // similar to Squeak, we fabricate Ctrl+Alt+PageUp/Down for wheel events
				SDL_Keycode key = event.wheel.y > 0 ? SDLK_PAGEUP : SDLK_PAGEDOWN;
				SDL_Event event;
				SDL_zero(event);
				event.type = SDL_KEYDOWN;
				event.key.keysym.sym = SDLK_LCTRL;
				SDL_PushEvent(&event);
				SDL_zero(event);
				event.type = SDL_KEYDOWN;
				event.key.keysym.sym = SDLK_LALT;
				SDL_PushEvent(&event);
				SDL_zero(event);
				event.type = SDL_KEYDOWN;
				event.key.keysym.sym = key;
				SDL_PushEvent(&event);
				SDL_zero(event);
				event.type = SDL_KEYUP;
				event.key.keysym.sym = key;
				SDL_PushEvent(&event);
				SDL_zero(event);
				event.type = SDL_KEYUP;
				event.key.keysym.sym = SDLK_LALT;
				SDL_PushEvent(&event);
				SDL_zero(event);
				event.type = SDL_KEYUP;
				event.key.keysym.sym = SDLK_LCTRL;
				SDL_PushEvent(&event);
			}
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				SizeChangeCounter++;
				break;

				case SDL_WINDOWEVENT_ENTER:
				case SDL_WINDOWEVENT_LEAVE:
				{
					static bool InMainWindow = true;

					if (InMainWindow && (event.window.event == SDL_WINDOWEVENT_LEAVE)) {
						InputMouseExit(callbacks, SDL_GetTicks());
					}
					InMainWindow = (event.window.event == SDL_WINDOWEVENT_ENTER);
				}
				break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				case SDL_WINDOWEVENT_FOCUS_LOST:
				{
				if (!IsNetworkGame() && Preference.PauseOnLeave /*(SDL_GetWindowFlags(TheWindow) & SDL_WINDOW_INPUT_FOCUS)*/) {
					static bool DoTogglePause = false;

					if (IsSDLWindowVisible && (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)) {
						IsSDLWindowVisible = false;
						if (!GamePaused) {
							DoTogglePause = !GamePaused;
							GamePaused = true;
						}
					} else if (!IsSDLWindowVisible && (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)) {
						IsSDLWindowVisible = true;
						if (GamePaused && DoTogglePause) {
							DoTogglePause = false;
							GamePaused = false;
						}
					}
				}
				}
				break;
			}
			break;

		case SDL_TEXTINPUT:
			{
				char* text = event.text.text;
				if (isTextInput((uint8_t)text[0])) {
					// we only accept US-ascii chars for now
					char lastKey = text[0];
					InputKeyButtonPress(callbacks, SDL_GetTicks(), lastKey, lastKey);
					// fabricate a keyup event for later
					SDL_Event event;
					SDL_zero(event);
					event.type = SDL_CUSTOM_KEY_UP;
					event.user.code = lastKey;
					SDL_PeepEvents(&event, 1, SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
				}
			}
			break;

		case SDL_KEYDOWN:
			keysym = event.key.keysym.sym;
			if (!isTextInput(keysym)) {
				// only report non-printing keys here, the characters will be reported with the textinput event
				InputKeyButtonPress(callbacks, SDL_GetTicks(), keysym, keysym < 128 ? keysym : 0);
			}
			break;

		case SDL_KEYUP:
			keysym = event.key.keysym.sym;
			if (!isTextInput(keysym)) {
				// only report non-printing keys here, the characters will be reported with the textinput event
				InputKeyButtonRelease(callbacks, SDL_GetTicks(), keysym, keysym < 128 ? keysym : 0);
			}
			break;

#ifdef __vita__
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			HandleTouchEvent(callbacks, event.tfinger);
			break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (gameController != nullptr) {
                const SDL_GameController* removedController = SDL_GameControllerFromInstanceID(event.jdevice.which);
                if (removedController == gameController) {
                    SDL_GameControllerClose(gameController);
                    gameController = nullptr;
                }
            }
            break;
        case SDL_CONTROLLERDEVICEADDED:
            if (gameController == nullptr) {
                gameController = SDL_GameControllerOpen(event.jdevice.which);
            }
            break;
        case SDL_CONTROLLERAXISMOTION:
            HandleControllerAxisEvent(event.caxis);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            HandleControllerButtonEvent(callbacks, event.cbutton);
            break;
#endif
		case SDL_QUIT:
			Exit(0);
			break;

		default:
			if (event.type == SDL_SOUND_FINISHED) {
				HandleSoundEvent(event);
			} else if (event.type == SDL_CUSTOM_KEY_UP) {
				char key = static_cast<char>(event.user.code);
				InputKeyButtonRelease(callbacks, SDL_GetTicks(), key, key);
			}
			break;
	}

	if (&callbacks == GetCallbacks()) {
		handleInput(&event);
	}
}

/**
**  Set the current callbacks
*/
void SetCallbacks(const EventCallback *callbacks)
{
	Callbacks = callbacks;
}

/**
**  Get the current callbacks
*/
const EventCallback *GetCallbacks()
{
	return Callbacks;
}

/**
**  Wait for interactive input event for one frame.
**
**  Handles system events, joystick, keyboard, mouse.
**  Handles the network messages.
**  Handles the sound queue.
**
**  All events available are fetched. Sound and network only if available.
**  Returns if the time for one frame is over.
*/
void WaitEventsOneFrame()
{
	if (dummyRenderer) {
		return;
	}

	Uint32 ticks = SDL_GetTicks();
	if (ticks > NextFrameTicks) { // We are too slow :(
		++SlowFrameCounter;
	}

	InputMouseTimeout(*GetCallbacks(), ticks);
	InputKeyTimeout(*GetCallbacks(), ticks);
	CursorAnimate(ticks);

	int interrupts = Parameters::Instance.benchmark;

	for (;;) {
		// Time of frame over? This makes the CPU happy. :(
		ticks = SDL_GetTicks();
		if (!interrupts && ticks < NextFrameTicks) {
			SDL_Delay(NextFrameTicks - ticks);
			ticks = SDL_GetTicks();
		}
		while (ticks >= (unsigned long)(NextFrameTicks)) {
			++interrupts;
			NextFrameTicks += FrameTicks;
		}

		SDL_Event event[1];
		const int i = SDL_PollEvent(event);
		if (i) { // Handle SDL event
			SdlDoEvent(*GetCallbacks(), *event);
		}

		// Network
		int s = 0;
		if (IsNetworkGame()) {
			s = NetworkFildes.HasDataToRead(0);
			if (s > 0) {
				if (GetCallbacks()->NetworkEvent) {
					GetCallbacks()->NetworkEvent();
				}
			}
		}

		// Online session
		OnlineContextHandler->doOneStep();

		// No more input and time for frame over: return
		if (!i && s <= 0 && interrupts) {
			break;
		}
	}
	handleInput(NULL);

	if (!SkipGameCycle--) {
		SkipGameCycle = SkipFrames;
	}

#ifdef __vita__
	ProcessControllerAxisMotion();
#endif
}

/**
**  Realize video memory.
*/

static Uint32 LastTick = 0;
static int RefreshRate = 0;

static void RenderBenchmarkOverlay()
{
	if (!RefreshRate) {
		int displayCount = SDL_GetNumVideoDisplays();
		SDL_DisplayMode mode;
		for (int i = 0; i < displayCount; i++) {
			SDL_GetDesktopDisplayMode(0, &mode);
			if (mode.refresh_rate > RefreshRate) {
				RefreshRate = mode.refresh_rate;
			}
		}
	}

	// show a bar representing fps, where the entire bar is the max refresh rate of attached displays
	Uint32 nextTick = SDL_GetTicks();
	Uint32 frameTime = nextTick - LastTick;
	int fps = std::min(RefreshRate, static_cast<int>(frameTime > 0 ? (1000.0 / frameTime) : 0));
	LastTick = nextTick;

	// draw the full bar
	SDL_SetRenderDrawColor(TheRenderer, 255, 0, 0, 255);
	SDL_Rect frame = { Video.Width - 10, 2, 8, RefreshRate };
	SDL_RenderDrawRect(TheRenderer, &frame);

	// draw the inner fps gage
	SDL_SetRenderDrawColor(TheRenderer, 0, 255, 0, 255);
	SDL_Rect bar = { Video.Width - 8, 2 + RefreshRate - fps, 4, fps };
	SDL_RenderFillRect(TheRenderer, &bar);

	SDL_SetRenderDrawColor(TheRenderer, 0, 0, 0, 255);
}

void RealizeVideoMemory()
{
	++FrameCounter;
	if (dummyRenderer) {
		return;
	}
	if (Preference.FrameSkip && (FrameCounter & Preference.FrameSkip)) {
		return;
	}
	if (NumRects) {
		//SDL_UpdateWindowSurfaceRects(TheWindow, Rects, NumRects);
		SDL_UpdateTexture(TheTexture, NULL, TheScreen->pixels, TheScreen->pitch);
		if (!RenderWithShader(TheRenderer, TheWindow, TheTexture)) {
			SDL_RenderClear(TheRenderer);
			//for (int i = 0; i < NumRects; i++)
			//    SDL_UpdateTexture(TheTexture, &Rects[i], TheScreen->pixels, TheScreen->pitch);
#ifdef __vita__
			SDL_RenderCopy(TheRenderer, TheTexture, NULL, &Video.RenderRect);
#else
			SDL_RenderCopy(TheRenderer, TheTexture, NULL, NULL);
#endif
		}
		if (Parameters::Instance.benchmark) {
			RenderBenchmarkOverlay();
		}
		SDL_RenderPresent(TheRenderer);
		NumRects = 0;
	}
	if (!Preference.HardwareCursor) {
		HideCursor();
	}
}

/**
**  Lock the screen for write access.
*/
void SdlLockScreen()
{
	if (SDL_MUSTLOCK(TheScreen)) {
		SDL_LockSurface(TheScreen);
	}
}

/**
**  Unlock the screen for write access.
*/
void SdlUnlockScreen()
{
	if (SDL_MUSTLOCK(TheScreen)) {
		SDL_UnlockSurface(TheScreen);
	}
}

/**
**  Convert a SDLKey to a string
*/
const char *SdlKey2Str(int key)
{
	return Key2Str[key].c_str();
}

/**
**  Convert a string to SDLKey
*/
int Str2SdlKey(const char *str)
{
	InitKey2Str();

	std::map<int, std::string>::iterator i;
	for (i = Key2Str.begin(); i != Key2Str.end(); ++i) {
		if (!strcasecmp(str, (*i).second.c_str())) {
			return (*i).first;
		}
	}
	std::map<std::string, int>::iterator i2;
	for (i2 = Str2Key.begin(); i2 != Str2Key.end(); ++i2) {
		if (!strcasecmp(str, (*i2).first.c_str())) {
			return (*i2).second;
		}
	}
	return 0;
}

/**
**  Check if the mouse is grabbed
*/
bool SdlGetGrabMouse()
{
	return SDL_GetWindowGrab(TheWindow);
}

/**
**  Toggle grab mouse.
**
**  @param mode  Wanted mode, 1 grab, -1 not grab, 0 toggle.
*/
void ToggleGrabMouse(int mode)
{
	bool grabbed = SdlGetGrabMouse();

	if (mode <= 0 && grabbed) {
		SDL_SetWindowGrab(TheWindow, SDL_FALSE);
	} else if (mode >= 0 && !grabbed) {
		SDL_SetWindowGrab(TheWindow, SDL_TRUE);
	}
}

/**
**  Toggle full screen mode.
*/
void ToggleFullScreen()
{
	Uint32 flags;
	flags = SDL_GetWindowFlags(TheWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef USE_WIN32

	if (!TheWindow) { // don't bother if there's no surface.
		return;
	}
	SDL_SetWindowFullscreen(TheWindow, flags ^ SDL_WINDOW_FULLSCREEN_DESKTOP);

	Invalidate(); // Update display
#else // !USE_WIN32
	SDL_SetWindowFullscreen(TheWindow, flags ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif

	Video.FullScreen = (flags ^ SDL_WINDOW_FULLSCREEN_DESKTOP) ? 1 : 0;
#ifdef __vita__
	Video.SetVitaRenderArea();
#endif
}

//@}
