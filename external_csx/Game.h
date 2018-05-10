#pragma once
//
//	CSGO Cheat Functions / offsets / signatures
//	Chris De Pasquale
//

#include "Funcs.h"

#define FLASH_MAX_ALPHA_NOFLASH 80.0f
#define FLASH_MAX_ALPHA_DEFAULT 255.0f

// Static Offsets - not likely to change
static const DWORD DORMANTOFFSET = 0x000000e9;
static const DWORD ALIVEOFFSET = 0x0000025b;
static const DWORD TEAMOFFSET = 0x000000f0;
static const DWORD FLASHMAXALPHAOFFSET = 0x0000BEF4;
static const DWORD VECORIGINOFFSET = 0x00000134;
static const DWORD VECVIEWOFFSETOFFSET = 0x00000104; 
static const DWORD INDEXOFFSET = 0x00000064;// Entity index 

static const DWORD GLOW_ARRAY_SIZE_OFFSET = 4;

static const int ISALIVE_VALUE_ALIVE = 0;

// Valid player index range, not inclusive
// index 1 is World, >64 is non-player objects 
static const int MAX_VALID_PLAYER_INDEX = 64;
static const int MIN_VALID_PLAYER_INDEX = 1;

// Dynamic offset signatures 
#define CROSSHAIR_ENT_ID_OFFSET_BYTES "\x8B\x81\x00\x00\x00\x00\x85\xC0\x75\x15"
#define CROSSHAIR_ENT_ID_OFFSET_MASK "xx????xxxx"

#define LOCAL_PLAYER_OFFSET_BYTES "\xA3\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x59\xC3\x6A\x00"
#define LOCAL_PLAYER_OFFSET_MASK "x????xx????????x????xxxx"

#define GLOW_OBJECTS_OFFSET_BYTES "\xA1\x00\x00\x00\x00\xA8\x01\x75\x00\x0F\x57\xC0\xC7\x05"
#define GLOW_OBJECTS_OFFSET_MASK "x????xxx?xxxxx"

#define CLIENT_STATE_OFFSET_BYTES "\xA1\x00\x00\x00\x00\xF3\x0F\x11\x80\x00\x00\x00\x00\xD9\x46\x04\xD9\x05"
#define CLIENT_STATE_OFFSET_MASK "x????xxxx????xxxxx"

#define ENTITY_LIST_OFFSET_BYTES "\xBB\x00\x00\x00\x00\x83\xFF\x01\x0F\x8C\x00\x00\x00\x00\x3B\xF8"
#define ENTITY_LIST_OFFSET_MASK "x????xxxxx????xx"

class CGame;  
 
struct GlowObjectDefinition_t {
    DWORD* pEntity; 
    float r; 
    float g;
    float b;
    float a; 
    unsigned char unk1[16];
    bool m_bRenderWhenOccluded;
    bool m_bRenderWhenUnoccluded; 
    bool m_bFullBloom;
    unsigned char unk2[13];

	GlowObjectDefinition_t(){}

	void SetRenderValues(float _r, float _g, float _b, float _a, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded) {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
		m_bRenderWhenOccluded = bRenderWhenOccluded;
		m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
	}
};  

class CPlayer {

protected: 
	CGame* m_pGame;

public:
	CPlayer();
	void Update( int index );
};

class CLocalPlayer {
	friend class CGame;
	bool m_bValid;
	
protected:
	CGame* m_pGame;
	CLocalPlayer(CGame* pGame);

public:
	CLocalPlayer() { m_bValid = false; }
	void Update();

	bool GetEyePosition(vec3& result);

	bool ApplyNoFlash();
	bool RemoveNoFlash();

	int EntIndexInCrosshair();
	bool GetTeam(int& __out_team);
	bool IsAlive();
	bool ShouldRunCheats();

	void* GetPlayerBase();
};

class CGame {
	friend class CLocalPlayer;

protected:
	HANDLE m_hGame = NULL;
	HMODULE m_hEngine;
	HMODULE m_hClient;

	DWORD m_dwClientStateOffset;
	DWORD m_dwLocalPlayerOffset;
	DWORD m_dwGlowObjectsOffset;
	DWORD m_dwEntityListOffset;
	DWORD m_dwInCrossOffset;

public:
	~CGame();
	bool Init(HANDLE hGameHandle);
	bool GameIsOpen();

	// Helpers
	CLocalPlayer GetLocalPlayer();
	void* GetPlayerBaseByIndex(int i);
	void* GetEngine();

	// Game Cheats
	void DoEntityGlow(CLocalPlayer* pLocalPlayer, bool bForceRun);
	void DoTriggerbot(CLocalPlayer* pLocalPlayer);
};