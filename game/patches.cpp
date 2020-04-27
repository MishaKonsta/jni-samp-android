#include "../main.h"
#include "../util/armhook.h"
#include "common.h"

char* PLAYERS_REALLOC = nullptr;

void ApplyFPSPatch(uint8_t fps)
{
	char fpsch[15];
	sprintf(fpsch, "\\x%x", fps);
	WriteMemory(g_libGTASA+0x463FE8, (uintptr_t)fpsch, 1);
	WriteMemory(g_libGTASA+0x56C1F6, (uintptr_t)fpsch, 1);
	WriteMemory(g_libGTASA+0x56C126, (uintptr_t)fpsch, 1);
	WriteMemory(g_libGTASA+0x95B074, (uintptr_t)fpsch, 1);
}


void ApplyCrashFixPatches()
{
	// reallocate CPools::ms_pEntryInfoNodePool
	WriteMemory(g_libGTASA + 0x003AF27A, (uintptr_t)"\x4f\xf4\x20\x40", 4); // MOV.W	R0, #0xA000 | size = 0x14
	WriteMemory(g_libGTASA + 0x003AF284, (uintptr_t)"\x4f\xf4\x90\x50", 4); // MOV.W R0, #0x1200
	WriteMemory(g_libGTASA + 0x003AF28C, (uintptr_t)"\x4f\xf4\x90\x52", 4); // MOV.W R2, #0x1200
	WriteMemory(g_libGTASA + 0x003AF2BA, (uintptr_t)"\xb3\xf5\x90\x5f", 4); // CMP.W R3, #0x1200
	
	// reallocate CPools::ms_pPtrNodeDoubleLinkPool
	WriteMemory(g_libGTASA + 0x003AF21C, (uintptr_t)"\x4F\xF4\x00\x30", 4); // MOV.W R0, #0x20000
}

void ApplyPatches_level0()
{
	// reallocate CWorld::Players[]
	//PLAYERS_REALLOC = new char[404*PLAYER_PED_SLOTS];
	PLAYERS_REALLOC = (( char* (*)(int))(g_libGTASA+0x179B40))(404*MAX_PLAYERS);
	UnFuck(g_libGTASA+0x5D021C);
	*(char**)(g_libGTASA+0x5D021C) = PLAYERS_REALLOC;
	Log("CWorld::Player new address = 0x%X", PLAYERS_REALLOC);

	// CdStreamInit(6);
	WriteMemory(g_libGTASA+0x3981EC, (uintptr_t)"\x06\x20", 2);

	// Apply crash fix patches
	#ifdef RELEASE_BETA
	ApplyCrashFixPatches();
	#endif
}

struct _ATOMIC_MODEL
{
	uintptr_t func_tbl;
	char data[56];
};
struct _ATOMIC_MODEL *ATOMIC_MODELS;

void ApplyPatches()
{
	Log("Installing patches..");

	ApplyFPSPatch(60); // default 60fps
	// allocate Atomic models pool
	ATOMIC_MODELS = new _ATOMIC_MODEL[20000];
	for (int i = 0; i < 20000; i++) 
	{
		// CBaseModelInfo::CBaseModelInfo
		((void(*)(_ATOMIC_MODEL*))(g_libGTASA + 0x33559C + 1))(&ATOMIC_MODELS[i]);
		ATOMIC_MODELS[i].func_tbl = g_libGTASA + 0x5C6C68;
		memset(ATOMIC_MODELS[i].data, 0, sizeof(ATOMIC_MODELS->data));
	}

	// CAudioEngine::StartLoadingTune
	NOP(g_libGTASA+0x56C150, 2);

	// DefaultPCSaveFileName
	char* DefaultPCSaveFileName = (char*)(g_libGTASA+0x60EAE8);
	memcpy((char*)DefaultPCSaveFileName, "GTASAMP", 8);

	// CWidgetRegionSteeringSelection::Draw
	WriteMemory(g_libGTASA+0x284BB8, (uintptr_t)"\xF7\x46", 2);

	// CHud::SetHelpMessage
	WriteMemory(g_libGTASA+0x3D4244, (uintptr_t)"\xF7\x46", 2);
	// CHud::SetHelpMessageStatUpdate
	WriteMemory(g_libGTASA+0x3D42A8, (uintptr_t)"\xF7\x46", 2);
	// CVehicleRecording::Load
	WriteMemory(g_libGTASA+0x2DC8E0, (uintptr_t)"\xF7\x46", 2);

	// nop calling CRealTimeShadowManager::ReturnRealTimeShadow from ~CPhysical
	NOP(g_libGTASA+0x3A019C, 2);

    // Increase matrix count in CPlaceable::InitMatrixArray 
 	WriteMemory(g_libGTASA+0x3ABB0A, (uintptr_t)"\x4F\xF4\x7A\x61", 4); // MOV.W R1, #4000
}

void ApplyInGamePatches()
{
	Log("Installing patches (ingame)..");

	/* ������������� ����� */
	// CTheZones::ZonesVisited[100]
	memset((void*)(g_libGTASA+0x8EA7B0), 1, 100);
	// CTheZones::ZonesRevealed
	*(uint32_t*)(g_libGTASA+0x8EA7A8) = 100;

	// CTaskSimplePlayerOnFoot::PlayIdleAnimations 
	WriteMemory(g_libGTASA+0x4BDB18, (uintptr_t)"\x70\x47", 2);

	// CarCtl::GenerateRandomCars nulled from CGame::Process
	UnFuck(g_libGTASA+0x398A3A);
	NOP(g_libGTASA+0x398A3A, 2);

	// CTheCarGenerators::Process nulled from CGame::Process
	UnFuck(g_libGTASA+0x398A34);
	NOP(g_libGTASA+0x398A34, 2);

	// ��������� ��� MaxHealth
	UnFuck(g_libGTASA+0x3BAC68);
	*(float*)(g_libGTASA+0x3BAC68) = 176.0f;
	// ��������� ��� Armour
	UnFuck(g_libGTASA+0x27D884);
	*(float*)(g_libGTASA+0x27D884) = 176.0;

	// CEntryExit::GenerateAmbientPeds nulled from CEntryExit::TransitionFinished
	UnFuck(g_libGTASA+0x2C2C22);
	NOP(g_libGTASA+0x2C2C22, 4);

	// ���������
	WriteMemory(g_libGTASA+0x3D62FC, (uintptr_t)"\xF7\x46", 2);

	// CPlayerPed::CPlayerPed task fix
	WriteMemory(g_libGTASA+0x458ED1, (uintptr_t)"\xE0", 1);

	// ReapplyPlayerAnimation (�� �����)
	NOP(g_libGTASA+0x45477E, 5);

	// radar draw blips
    UnFuck(g_libGTASA+0x3DCA90);
    NOP(g_libGTASA+0x3DCA90, 2);
    UnFuck(g_libGTASA+0x3DD4A4);
    NOP(g_libGTASA+0x3DD4A4, 2);
    // CCamera::CamShake from CExplosion::AddExplosion
    NOP(g_libGTASA+0x55EFB8, 2);
    NOP(g_libGTASA+0x55F8F8, 2);

    // camera_on_actor path
    UnFuck(g_libGTASA+0x2F7B68);
    *(uint8_t*)(g_libGTASA+0x2F7B6B) = 0xBE;

    // CPed::RemoveWeaponWhenEnteringVehicle (GetPlayerInfoForThisPlayerPed)
    UnFuck(g_libGTASA+0x434D94);
    NOP(g_libGTASA+0x434D94, 6);
	
    // CBike::ProcessAI
    UnFuck(g_libGTASA+0x4EE200);
    *(uint8_t*)(g_libGTASA+0x4EE200) = 0x9B;

    // CWidgetPlayerInfo::DrawWanted
    WriteMemory(g_libGTASA+0x27D8D0, (uintptr_t)"\x4F\xF0\x00\x08", 4);

    // no vehicle audio processing
    UnFuck(g_libGTASA+0x4E31A6);
    NOP(g_libGTASA+0x4E31A6, 2);
    UnFuck(g_libGTASA+0x4EE7D2);
    NOP(g_libGTASA+0x4EE7D2, 2);
    UnFuck(g_libGTASA+0x4F741E);
    NOP(g_libGTASA+0x4F741E, 2);
    UnFuck(g_libGTASA+0x50AB4A);
    NOP(g_libGTASA+0x50AB4A, 2);
}