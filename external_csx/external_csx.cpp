#include "stdafx.h"
#include "Funcs.h"
#include "Game.h"
#include "Console.h"

int main(int argc, char* argv[]) {
	SetConsoleSizeChars(CONSOLE_W, CONSOLE_H, true);
	SetConsoleStyle(false, false);
	SetConsoleTitleA("CSX v3.6");

	DEFAULT_COLOR();
	printf("                                                                            \n");
	PUT_TITLE_SPC(); printf("                                                                            \n");
	PUT_TITLE_SPC(); printf("                                                                            \n");
	PUT_TITLE_SPC(); printf("                    .d8888b.     .d8888b.   Y88b   d88P                     \n");
	PUT_TITLE_SPC(); printf("                   d88P  Y88b   d88P  Y88b   Y88b d88P                      \n");
	PUT_TITLE_SPC(); printf("                   888    888   Y88b.         Y88o88P                       \n");
	PUT_TITLE_SPC(); printf("                   888           'Y888b.       Y888P                        \n");
	PUT_TITLE_SPC(); printf("                   888              'Y88b.     d888b                        \n");
	PUT_TITLE_SPC(); printf("                   888    888         '888    d88888b                       \n");
	PUT_TITLE_SPC(); printf("                   Y88b  d88P   Y88b  d88P   d88P Y88b                      \n");
	PUT_TITLE_SPC(); printf("                    'Y8888P'     'Y8888P'   d88P   Y88b                     \n");
	PUT_TITLE_SPC(); printf("                                Version 3.6                                 \n");
	PUT_TITLE_SPC(); printf("                                                                            \n");
	PUT_TITLE_SPC(); printf("                                                                            \n");
	DEFAULT_COLOR(); 
	printf("\n  ");
	
	ERROR_COLOR();
	printf("                               Key Bindings                                 \n");
	DEFAULT_COLOR();
	printf("\n\t%-8s\tTriggerBot \n\t%-8s\tExit \n\t%-8s\tEnable wallhack while Dead\n\n  ", CHEAT_KEY_STR, EXIT_KEY_STR, FORCE_GLOW_KEY_STR);


	// Get handle to game
	printf("  Waiting for CS:GO to launch\n");
	HANDLE hGame;
	while (GetProcessHandleFromFileName("csgo.exe", hGame) == MODULE_NOT_FOUND) {
		if (WaitAllowExit(500))
			break;
		RenderLoaderStep();
	}
	ClearLoader();
	if (hGame == NULL)
		ERROR_EXIT("  Fatal Error - Failed to find csgo.exe\n");

	CGame game;
	if (!game.Init(hGame))
		ERROR_EXIT("  Fatal Error - Failed to initiate game\n");

	CLocalPlayer local = game.GetLocalPlayer();
	while (!KeyDown(EXIT_KEY) && game.GameIsOpen()) {

		if (local.ShouldRunCheats()) {
			if (KeyDown(CHEAT_KEY))
				game.DoTriggerbot(&local);

			local.ApplyNoFlash();
		}

		game.DoEntityGlow(&local, KeyDown(FORCE_GLOW_KEY));
	}
	local.RemoveNoFlash();
	CloseHandle(hGame);

	return 0;
}

