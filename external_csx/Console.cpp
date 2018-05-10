#include "stdafx.h"
#include "Console.h"

#define LOADER_MAX (CONSOLE_W - (CONSOLE_LOADER_SPACING * 2) - 2 - CONSOLE_LOADER_W)
void RenderLoaderStep() {
	static int loaderStep = 0;
	static int direction = 1;
	if (loaderStep > LOADER_MAX) {
		direction *= -1;
		loaderStep = LOADER_MAX - 1;
	} else if (loaderStep < 0) {
		direction *= -1;
		loaderStep = 1;
	}

	printf("\r  [");
	for (int i = 0; i < loaderStep; i++)
		putc(' ', stdout);
	for (int i = loaderStep; i < loaderStep + CONSOLE_LOADER_W ; i++)
		putc('#', stdout);
	for (int i = loaderStep + CONSOLE_LOADER_W; i < LOADER_MAX + CONSOLE_LOADER_W; i++)
		putc(' ', stdout);
	putc(']', stdout);

	loaderStep += direction;
}

void ClearLoader() {
	putc('\r', stdout);
}

bool SetConsoleStyle(bool bAllowMaximise, bool bAllowResize) {
	HWND hConsole = GetConsoleWindow();

	// Retrieve and modify style flags
	long newStyle = GetWindowLong(hConsole, GWL_STYLE);
	newStyle = bAllowMaximise ? newStyle | WS_MAXIMIZEBOX : newStyle & ~WS_MAXIMIZEBOX;
	newStyle = bAllowResize ? newStyle | WS_SIZEBOX : newStyle & ~WS_SIZEBOX;

	return SetWindowLong(hConsole, GWL_STYLE, newStyle) != 0;
}

bool GetWindowPosAndBorderSize(HWND hWnd, int& __out_borderW, int& __out_borderH, PRECT __out_wndRect) {
	// Get negative of inner window size
	if (!GetClientRect(hWnd, __out_wndRect))
		return false;
	__out_borderW = -1 * __out_wndRect->right;
	__out_borderH = -1 * __out_wndRect->bottom;

	// Add full window size.  We do this second to get a valid rect for
	// __out_wndRect, ClientRect values are relative to the WindowRect
	if (!GetWindowRect(hWnd, __out_wndRect))
		return false;
	__out_borderW += __out_wndRect->right - __out_wndRect->left;
	__out_borderH += __out_wndRect->bottom - __out_wndRect->top;

	return true;
}

bool SetConsoleSizeChars(int w, int h, bool bResizeBuffer) {
	// Get font data to retrieve font size in pixels
	CONSOLE_FONT_INFO fontInfo;
	HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetCurrentConsoleFont(hConsoleOut, false, &fontInfo);

	// Get window position & border size
	RECT windowRect;
	HWND hConsole = GetConsoleWindow();
	int borderW, borderH;
	if (!GetWindowPosAndBorderSize(hConsole, borderW, borderH, &windowRect))
		return false;

	// Setting console screen buffer to it's size removes the scrollbar 
	if (bResizeBuffer) {
		COORD bufSize;
		bufSize.X = w;
		bufSize.Y = h;
		SetConsoleScreenBufferSize(hConsoleOut, bufSize);
	}

	// Multiply w,h by font width/height to convert characters to pixels
	// Add border size, as moveWindow size includes outer borders
	return MoveWindow(hConsole, windowRect.left, windowRect.top, 
		              w * fontInfo.dwFontSize.X + borderW, 
					  h * fontInfo.dwFontSize.Y + borderH, TRUE) != 0;
}

void Click() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
	Sleep(15);
	mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
	Sleep(10);
}

void ErrorPrint(const char *format, ...) {
	DEFAULT_COLOR();
	printf("  ");
	ERROR_COLOR();

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	DEFAULT_COLOR();
}

bool WaitAllowExit(unsigned int ms) {
	unsigned int totalLoops = ms / 100;

	if (totalLoops == 0) {
		// Handle ms < 100
		Sleep(ms);
		if (KeyDown(EXIT_KEY))
			return true;
		return false;
	}

	// Wait given time, checking for exit key every 100ms
	for (unsigned waitTime = 0; waitTime < totalLoops; waitTime++) {		
		if (KeyDown(EXIT_KEY))
			return true;
		Sleep(100);
	}
	return false;
}