#include "stdafx.h"
#include "Game.h"
#include "Funcs.h"
#include "Console.h" 

void* CLocalPlayer::GetPlayerBase() {
	DWORD* base = 0;
	if (ReadProcessMemory(m_pGame->m_hGame, MakePointer(m_pGame->m_hClient, m_pGame->m_dwLocalPlayerOffset), &base, sizeof(DWORD*), 0) != 0)
		return NULL;
	return base;
}

CLocalPlayer::CLocalPlayer(CGame* pGame) {
	m_pGame = pGame;
	m_bValid = pGame != NULL;
}

bool CLocalPlayer::GetEyePosition(vec3& result) {
	if (!m_bValid)
		return false;

	void* pBase = GetPlayerBase();
	if (!pBase)
		return false;

	// Get player character world origin
	vec3 baseOrigin;
	if (ReadProcessMemory(m_pGame->m_hGame, MakePointer(pBase, VECORIGINOFFSET), &baseOrigin, sizeof(vec3), 0) == 0)
		return false;

	// Get player eye offset vector
	vec3 viewOffset;
	if (ReadProcessMemory(m_pGame->m_hGame, MakePointer(pBase, VECVIEWOFFSETOFFSET), &viewOffset, sizeof(vec3), 0) == 0)
		return false;

	result = baseOrigin + viewOffset;
	return true;
}

bool CLocalPlayer::ApplyNoFlash() {
	if (!m_bValid)
		return false;

	void* pBase = GetPlayerBase();
	if (pBase == NULL)
		return false;

	float f = FLASH_MAX_ALPHA_NOFLASH;
	return WriteProcessMemory(m_pGame->m_hGame, MakePointer(pBase, FLASHMAXALPHAOFFSET), &f, sizeof(float), 0) != 0;
}

bool CLocalPlayer::RemoveNoFlash() {
	if (!m_bValid)
		return false;

	void* pBase = GetPlayerBase();
	if (pBase == NULL)
		return false;

	float f = FLASH_MAX_ALPHA_DEFAULT;
	return WriteProcessMemory(m_pGame->m_hGame, MakePointer(pBase, FLASHMAXALPHAOFFSET), &f, sizeof(float), 0) != 0;
}

bool CLocalPlayer::GetTeam(int& __out_team) {
	if (!m_bValid)
		return 0;

	void* pBase = GetPlayerBase();
	if (!pBase)
		return false;

	return ReadProcessMemory(m_pGame->m_hGame, MakePointer(pBase, TEAMOFFSET), &__out_team, sizeof(int), 0) != 0;
}

int CLocalPlayer::EntIndexInCrosshair() {
	if (!m_bValid)
		return 0;

	void* pBase = GetPlayerBase();
	if (pBase == NULL)
		return 0;

	int entIndex = 0;
	if (ReadProcessMemory(m_pGame->m_hGame, MakePointer(pBase, m_pGame->m_dwInCrossOffset), &entIndex, sizeof(int), 0) == 0)
		return false;

	// inCross value is entity ID + 1
	return (entIndex <= MIN_VALID_PLAYER_INDEX || entIndex >= MAX_VALID_PLAYER_INDEX) ? 0 : entIndex - 1; 
}

bool CLocalPlayer::IsAlive() {
	if (!m_bValid)
		return false;

	void* pBase = GetPlayerBase();
	if (!pBase)
		return 0;

	char alive = 0;
	if (ReadProcessMemory(m_pGame->m_hGame, MakePointer(pBase, ALIVEOFFSET), &alive, sizeof(char), 0) == 0)
		return false;
	return alive == ISALIVE_VALUE_ALIVE;
}

bool CLocalPlayer::ShouldRunCheats() {
	// Local player and Game Ptr must be valid, and either cheat key is pressed, or local player is alive
	return m_bValid && GetPlayerBase() && (KeyDown(CHEAT_KEY) || IsAlive());
}

CGame::~CGame() {
	if (m_hGame != NULL) {
		CloseHandle(m_hGame);
	}
}

// Helper - Get module, while allowing exit & rendering loading bar
HMODULE GetModule(HANDLE hProcess, char* szModule) {
	HMODULE hModule;
	int result;

	while ((result = GetExternalModule(hProcess, "engine.dll", hModule)) == MODULE_NOT_FOUND) {
		if (WaitAllowExit(500))
			return NULL;
		RenderLoaderStep();
	}
	ClearLoader();

	if (result == MODULE_ERROR)
		return NULL;
	return hModule;
}

bool CGame::Init(HANDLE hGameHandle) {
	m_hGame = hGameHandle;
	printf("  Waiting for Game modules to load\n");

	// Get handle (Base Address) of required modules
	m_hEngine = GetModule(m_hGame, "engine.dll");
	if (m_hEngine == NULL)
		return false;

	m_hClient = GetModule(m_hGame, "client.dll");
	if (m_hClient == NULL)
		return false;

	printf("\tClient: 0x%x\n\tEngine: 0x%x\n", m_hClient, m_hEngine);

	// Load dynamic offsets by scanning for byte patterns
	printf("  Starting offset signature scan");

	// Incross Offset = *(InCrossOffsetSig + 2)
	void* pCrossSig = SignatureScan(m_hGame, m_hClient, CROSSHAIR_ENT_ID_OFFSET_BYTES, CROSSHAIR_ENT_ID_OFFSET_MASK);
	if (!pCrossSig) {
		ErrorPrint("ERROR! Unable to find Crosshair ID offset!\n");
		return false;
	}
	ReadProcessMemory(m_hGame, MakePointer(pCrossSig, 2), &m_dwInCrossOffset, sizeof(DWORD), 0);

	// LocalPlayerOffset = (*(LocalPlayerSig + 1) + 0x10) - m_hClient
	void* dwLocalPlayerSig = SignatureScan(m_hGame, m_hClient, LOCAL_PLAYER_OFFSET_BYTES, LOCAL_PLAYER_OFFSET_MASK);
	if (!dwLocalPlayerSig) {
		ErrorPrint("ERROR! Unable to find Local Player offset!\n");
		return false;
	}
	ReadProcessMemory(m_hGame, MakePointer(dwLocalPlayerSig, 1), &m_dwLocalPlayerOffset, sizeof(DWORD), 0);
	m_dwLocalPlayerOffset += 0x10;
	m_dwLocalPlayerOffset -= (DWORD)m_hClient;

	// GlowObjectsOffset = *(GlowObjectsSig + 0x58) - m_hClient
	void* dwGlowObjectsSig = SignatureScan(m_hGame, m_hClient, GLOW_OBJECTS_OFFSET_BYTES, GLOW_OBJECTS_OFFSET_MASK);
	if (!dwGlowObjectsSig) {
		ErrorPrint("ERROR! Unable to find Glow Objects offset!\n");
		return false;
	}
	ReadProcessMemory(m_hGame, MakePointer(dwGlowObjectsSig, 0x58), &m_dwGlowObjectsOffset, sizeof(DWORD), 0);
	m_dwGlowObjectsOffset -= (DWORD)m_hClient;

	// ClientStateOffset = *(ClientStateSig + 1) - m_hEngine
	void* dwClientStateSig = SignatureScan(m_hGame, m_hEngine, CLIENT_STATE_OFFSET_BYTES, CLIENT_STATE_OFFSET_MASK);
	if (!dwClientStateSig) {
		ErrorPrint("ERROR! Unable to find Client State Offset!\n");
		return false;
	}
	ReadProcessMemory(m_hGame, MakePointer(dwClientStateSig, 0x1), &m_dwClientStateOffset, sizeof(DWORD), 0);
	m_dwClientStateOffset -= (DWORD)m_hEngine;

	// EntityListOffset = *(EntityListSig + 1) - m_hClient
	void* dwEntListOffsetSig = SignatureScan(m_hGame, m_hClient, ENTITY_LIST_OFFSET_BYTES, ENTITY_LIST_OFFSET_MASK);
	if (!dwEntListOffsetSig) {
		ErrorPrint("ERROR! Unable to find Entity Table Offset!\n");
		return false;
	}
	ReadProcessMemory(m_hGame, MakePointer(dwEntListOffsetSig, 0x1), &m_dwEntityListOffset, sizeof(DWORD), 0);
	m_dwEntityListOffset -= (DWORD)m_hClient;

	printf("\tCrosshair ID: 0x%x\n", m_dwInCrossOffset);
	printf("\tLocal Player: 0x%x\n", m_dwLocalPlayerOffset);
	printf("\tGlow Objects: 0x%x\n", m_dwGlowObjectsOffset);
	printf("\tClient State: 0x%x\n", m_dwClientStateOffset);
	printf("\tEntity Table: 0x%x\n", m_dwEntityListOffset);

	return true;
}

CLocalPlayer CGame::GetLocalPlayer() {
	 return CLocalPlayer(this);
}

void* CGame::GetPlayerBaseByIndex(int index) {
	DWORD* pBase = 0;
	if (ReadProcessMemory(m_hGame, MakePointer(m_hClient, m_dwEntityListOffset + (index * 0x10)), &pBase, sizeof(DWORD*), 0) == 0)
		return NULL;
	return pBase;
}

void* CGame::GetEngine() {
	DWORD* dwEngine;
	if (ReadProcessMemory(m_hGame, MakePointer(m_hEngine, m_dwClientStateOffset), &dwEngine, sizeof(DWORD*), 0) == 0)
		return NULL;
	return dwEngine;
}

void CGame::DoTriggerbot(CLocalPlayer* pLocal) {
	if (!pLocal)
		return;

	int target = pLocal->EntIndexInCrosshair();
	if (!target)
		return;

	void* pTargetBase = GetPlayerBaseByIndex(target);
	int targetTeam = 0;
	if (ReadProcessMemory(m_hGame, MakePointer(pTargetBase, TEAMOFFSET), &targetTeam, sizeof(int), 0) == 0)
		return;

	int playerTeam;
	if (!pLocal->GetTeam(playerTeam) || targetTeam == playerTeam)
		return;

	Click();
}

void CGame::DoEntityGlow(CLocalPlayer* pLocalPlayer, bool bForceRun) {
	if (!pLocalPlayer->ShouldRunCheats())
		return;

	DWORD* ppGlowArray = MakePointer(m_hClient, m_dwGlowObjectsOffset);
	GlowObjectDefinition_t glowObject;

	DWORD pGlowArray = 0;
	if (ReadProcessMemory(m_hGame, ppGlowArray, &pGlowArray, sizeof(DWORD), 0) == 0 || !pGlowArray)
		return;

	int count = 0;
	if (ReadProcessMemory(m_hGame, MakePointer(ppGlowArray, GLOW_ARRAY_SIZE_OFFSET), &count, sizeof(int), 0) == 0 || count < 0)
		return;

	int localTeam;
	if (!pLocalPlayer->GetTeam(localTeam))
		return;

	for (int i = 0; i < count; i++)	{
		// Read glow object
		if (ReadProcessMemory(m_hGame, (void*)(pGlowArray + i*sizeof(GlowObjectDefinition_t)), &glowObject, sizeof(GlowObjectDefinition_t), 0) == 0)
			continue;

		// Get & validate properties for associated entity
		BYTE bDormant = 1;
		if (ReadProcessMemory(m_hGame, MakePointer(glowObject.pEntity, DORMANTOFFSET), &bDormant, sizeof(BYTE), 0) == 0 || bDormant)
			continue;
		BYTE alive = 1;
		if (ReadProcessMemory(m_hGame, MakePointer(glowObject.pEntity, ALIVEOFFSET), &alive, sizeof(BYTE), 0) == 0 || alive != 0)
			continue;
		int targetTeam;
		if (ReadProcessMemory(m_hGame, MakePointer(glowObject.pEntity, TEAMOFFSET), &targetTeam, sizeof(int), 0) == 0)
			continue;
		int targetIndex;
		if (ReadProcessMemory(m_hGame, MakePointer(glowObject.pEntity, INDEXOFFSET), &targetIndex, sizeof(int), 0) == 0 || targetIndex > MAX_VALID_PLAYER_INDEX)
			continue;

		// Modify to display through walls
		glowObject.SetRenderValues(targetTeam == localTeam ? 0.0f : 0.5f, // Enemies are red
			targetTeam == localTeam ? 0.5f : 0.0f, // Friends are green
			0.0f, 1.0f, true, false);

		// Write to memory
		WriteProcessMemory(m_hGame, (void*)(pGlowArray + i*sizeof(GlowObjectDefinition_t)), &glowObject, sizeof(GlowObjectDefinition_t), 0);
	}
}

bool CGame::GameIsOpen() {
	DWORD exitCode;
	if (!GetExitCodeProcess(m_hGame, &exitCode) || exitCode != STILL_ACTIVE)
		return false;
	return true;
}