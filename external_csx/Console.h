#ifndef __CONSOLE_H__
#define __CONSOLE_H__
//
//	Program Console style & input functions
//	Chris De Pasquale
//

// Console size, in characters
#define CONSOLE_W 80
#define CONSOLE_H 40

// Width of the loader bar, in chars
#define CONSOLE_LOADER_W 5
// character spacing on each side of the loader
#define CONSOLE_LOADER_SPACING 2

// Console color list
#define COLOR_BLACK		0
#define COLOR_WHITE		7
#define COLOR_GREEN		0xa
#define COLOR_LBLUE		0xb
#define COLOR_RED		0xc
#define COLOR_PINK		0xd
#define COLOR_YELLOW	0xe
#define COLOR_BWHITE	0xf

// color helpers
#define TITLE_COLOR() ConsoleColor(COLOR_BLACK, COLOR_BWHITE)
#define DEFAULT_COLOR() ConsoleColor(COLOR_BWHITE, COLOR_BLACK) 
#define ERROR_COLOR() ConsoleColor(COLOR_BWHITE, COLOR_RED)

// Keys
#define EXIT_KEY VK_F6
#define EXIT_KEY_STR "F6"

#define CHEAT_KEY VK_CAPITAL
#define CHEAT_KEY_STR "CapsLock"

#define FORCE_GLOW_KEY VK_F7
#define FORCE_GLOW_KEY_STR "F7"

// Helper for outer title spacing 
#define PUT_TITLE_SPC(); DEFAULT_COLOR(); printf("  "); TITLE_COLOR();

// Print error and exit
#define ERROR_EXIT(error) do { ERROR_COLOR(); printf(error); exit(-1); } while (false)

// Returns TRUE if key is down or not
#define KeyDown(VKEY) ((GetAsyncKeyState(VKEY) & 0x8000) != 0)
// Returns TRUE if key was just pressed
#define KeyPress(VKEY) ((GetAsyncKeyState(VKEY) & 1) != 0)

// Helper - Click the mouse, sleeps 25ms
void Click();

// Change text and background for console output
#define ConsoleColor(textColor, backColor) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), backColor * 16 + textColor)

// Print an error in console style
void ErrorPrint(const char *format, ...);

// Set console size (in characters), optionally size buffer to remove scrollbar
bool SetConsoleSizeChars(int w, int h, bool bResizeBuffer);

// Enable/disable maximising and resizing
bool SetConsoleStyle(bool bAllowMaximise, bool bAllowResize);

/* Displays a looping loading bar, must be called when on a newline. each call 
   moves the loading bar one character across, call ClearLoader() to clear */
void RenderLoaderStep();

// Removes the loading bar 
void ClearLoader();

// Waits given time in ms. returns true and stops early on exit keypress
bool WaitAllowExit(unsigned int ms);

#endif