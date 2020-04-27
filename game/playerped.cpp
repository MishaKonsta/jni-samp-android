#include "../main.h"
#include "game.h"
#include "net/netgame.h"
#include "util/armhook.h"
#include "common.h"
#include "../chatwindow.h"

#define IS_CROUCHING(x) ((x->dwStateFlags >> 26) & 1)

extern CGame* pGame;
extern CNetGame *pNetGame;
extern CChatWindow *pChatWindow;

CPlayerPed::CPlayerPed()
{
	m_dwGTAId = 1;
	m_pPed = (PED_TYPE*)GamePool_FindPlayerPed();
	m_pEntity = (ENTITY_TYPE*)GamePool_FindPlayerPed();

	m_bytePlayerNumber = 0;
	SetPlayerPedPtrRecord(m_bytePlayerNumber,(uintptr_t)m_pPed);
	ScriptCommand(&set_actor_weapon_droppable, m_dwGTAId, 1);
	ScriptCommand(&set_actor_can_be_decapitated, m_dwGTAId, 0);

	DisableAutoAim();

	m_dwArrow = 0;

	m_iDancingState = 0;
	m_iDancingStyle = 0;

	for(int i = 0; i < 10; i++)
	{
		m_pAttachedObjects[i] = nullptr;
		m_bAOSlotState[i] = false;
	}
}

CPlayerPed::CPlayerPed(uint8_t bytePlayerNumber, int iSkin, float fX, float fY, float fZ, float fRotation)
{
	uint32_t dwPlayerActorID = 0;
	int iPlayerNum = bytePlayerNumber;

	m_pPed = nullptr;
	m_dwGTAId = 0;

	ScriptCommand(&create_player, &iPlayerNum, fX, fY, fZ, &dwPlayerActorID);
	ScriptCommand(&create_actor_from_player, &iPlayerNum, &dwPlayerActorID);

	m_dwGTAId = dwPlayerActorID;
	m_pPed = GamePool_Ped_GetAt(m_dwGTAId);
	m_pEntity = (ENTITY_TYPE*)GamePool_Ped_GetAt(m_dwGTAId);

	DisableAutoAim();

	m_bytePlayerNumber = bytePlayerNumber;
	SetPlayerPedPtrRecord(m_bytePlayerNumber, (uintptr_t)m_pPed);
	ScriptCommand(&set_actor_weapon_droppable, m_dwGTAId, 1);
	ScriptCommand(&set_actor_immunities, m_dwGTAId, 0, 0, 1, 0, 0);
	ScriptCommand(&set_actor_can_be_decapitated, m_dwGTAId, 0);

	m_dwArrow = 0;

	if(pNetGame)
		SetMoney(pNetGame->m_iDeathDropMoney);

	SetModelIndex(iSkin);
	ForceTargetRotation(fRotation);

	MATRIX4X4 mat;
	GetMatrix(&mat);
	mat.pos.X = fX;
	mat.pos.Y = fY;
	mat.pos.Z = fZ + 0.15f;
	SetMatrix(mat);

	memset(&RemotePlayerKeys[m_bytePlayerNumber], 0, sizeof(PAD_KEYS));

	m_iDancingState = 0;
	m_iDancingStyle = 0;

	for(int i = 0; i < 10; i++)
	{
		m_pAttachedObjects[i] = nullptr;
		m_bAOSlotState[i] = false;
	}
}

CPlayerPed::~CPlayerPed()
{
	Destroy();
}

void CPlayerPed::Destroy()
{
	memset(&RemotePlayerKeys[m_bytePlayerNumber], 0, sizeof(PAD_KEYS));
	SetPlayerPedPtrRecord(m_bytePlayerNumber, 0);

	if(!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId) || m_pPed->entity.vtable == 0x5C7358)
	{
		Log("CPlayerPed::Destroy: invalid pointer/vtable");
		m_pPed = nullptr;
		m_pEntity = nullptr;
		m_dwGTAId = 0;
		return;
	}


	//if(IsHaveAttachedObject())
	//	RemoveAllAttachedObjects();

	/*
		if(m_dwParachute) ... (��������)
	*/

	Log("Removing from vehicle..");
	if(IN_VEHICLE(m_pPed))
		RemoveFromVehicleAndPutAt(100.0f, 100.0f, 10.0f);

	Log("Setting flag state..");
	uintptr_t dwPedPtr = (uintptr_t)m_pPed;
	*(uint32_t*)(*(uintptr_t*)(dwPedPtr + 1088) + 76) = 0;
	// CPlayerPed::Destructor
	Log("Calling destructor..");
	(( void (*)(PED_TYPE*))(*(void**)(m_pPed->entity.vtable+0x4)))(m_pPed);

	m_pPed = nullptr;
	m_pEntity = nullptr;

	for(int i = 0; i < 10; i++)
	{
		if(m_bAOSlotState[i] == true)
		{
			delete m_pAttachedObjects[i];
			m_pAttachedObjects[i] = nullptr;
			m_bAOSlotState[i] = false;
		}
	}
}

CAMERA_AIM * CPlayerPed::GetCurrentAim()
{
	return GameGetInternalAim();
}

void CPlayerPed::SetCurrentAim(CAMERA_AIM * pAim)
{
	GameStoreRemotePlayerAim(m_bytePlayerNumber, pAim);
}

uint16_t CPlayerPed::GetCameraMode()
{
	return GameGetLocalPlayerCameraMode();
}

void CPlayerPed::SetCameraMode(uint16_t byteCamMode)
{
	GameSetPlayerCameraMode(byteCamMode, m_bytePlayerNumber);
}

float CPlayerPed::GetCameraExtendedZoom()
{
	return GameGetLocalPlayerCameraExtZoom();
}

void CPlayerPed::SetCameraExtendedZoom(float fZoom)
{
	GameSetPlayerCameraExtZoom(m_bytePlayerNumber, fZoom);
}

// 0.3.7
bool CPlayerPed::IsInVehicle()
{
	if(!m_pPed) return false;

	if(IN_VEHICLE(m_pPed))
		return true;

	return false;
}
int GameGetWeaponModelIDFromWeaponID(int iWeaponID)
{
	switch (iWeaponID)
	{
	case WEAPON_BRASSKNUCKLE:
		return WEAPON_MODEL_BRASSKNUCKLE;

	case WEAPON_GOLFCLUB:
		return WEAPON_MODEL_GOLFCLUB;

	case WEAPON_NITESTICK:
		return WEAPON_MODEL_NITESTICK;

	case WEAPON_KNIFE:
		return WEAPON_MODEL_KNIFE;

	case WEAPON_BAT:
		return WEAPON_MODEL_BAT;

	case WEAPON_SHOVEL:
		return WEAPON_MODEL_SHOVEL;

	case WEAPON_POOLSTICK:
		return WEAPON_MODEL_POOLSTICK;

	case WEAPON_KATANA:
		return WEAPON_MODEL_KATANA;

	case WEAPON_CHAINSAW:
		return WEAPON_MODEL_CHAINSAW;

	case WEAPON_DILDO:
		return WEAPON_MODEL_DILDO;

	case WEAPON_DILDO2:
		return WEAPON_MODEL_DILDO2;

	case WEAPON_VIBRATOR:
		return WEAPON_MODEL_VIBRATOR;

	case WEAPON_VIBRATOR2:
		return WEAPON_MODEL_VIBRATOR2;

	case WEAPON_FLOWER:
		return WEAPON_MODEL_FLOWER;

	case WEAPON_CANE:
		return WEAPON_MODEL_CANE;

	case WEAPON_GRENADE:
		return WEAPON_MODEL_GRENADE;

	case WEAPON_TEARGAS:
		return WEAPON_MODEL_TEARGAS;

	case WEAPON_MOLTOV:
		return -1;

	case WEAPON_COLT45:
		return WEAPON_MODEL_COLT45;

	case WEAPON_SILENCED:
		return WEAPON_MODEL_SILENCED;

	case WEAPON_DEAGLE:
		return WEAPON_MODEL_DEAGLE;

	case WEAPON_SHOTGUN:
		return WEAPON_MODEL_SHOTGUN;

	case WEAPON_SAWEDOFF:
		return WEAPON_MODEL_SAWEDOFF;

	case WEAPON_SHOTGSPA:
		return WEAPON_MODEL_SHOTGSPA;

	case WEAPON_UZI:
		return WEAPON_MODEL_UZI;

	case WEAPON_MP5:
		return WEAPON_MODEL_MP5;

	case WEAPON_AK47:
		return WEAPON_MODEL_AK47;

	case WEAPON_M4:
		return WEAPON_MODEL_M4;

	case WEAPON_TEC9:
		return WEAPON_MODEL_TEC9;

	case WEAPON_RIFLE:
		return WEAPON_MODEL_RIFLE;

	case WEAPON_SNIPER:
		return WEAPON_MODEL_SNIPER;

	case WEAPON_ROCKETLAUNCHER:
		return WEAPON_MODEL_ROCKETLAUNCHER;

	case WEAPON_HEATSEEKER:
		return WEAPON_MODEL_HEATSEEKER;

	case WEAPON_FLAMETHROWER:
		return WEAPON_MODEL_FLAMETHROWER;

	case WEAPON_MINIGUN:
		return WEAPON_MODEL_MINIGUN;

	case WEAPON_SATCHEL:
		return WEAPON_MODEL_SATCHEL;

	case WEAPON_BOMB:
		return WEAPON_MODEL_BOMB;

	case WEAPON_SPRAYCAN:
		return WEAPON_MODEL_SPRAYCAN;

	case WEAPON_FIREEXTINGUISHER:
		return WEAPON_MODEL_FIREEXTINGUISHER;

	case WEAPON_CAMERA:
		return WEAPON_MODEL_CAMERA;

	case -1:
		return WEAPON_MODEL_NIGHTVISION;

	case -2:
		return WEAPON_MODEL_INFRARED;

	case WEAPON_PARACHUTE:
		return WEAPON_MODEL_PARACHUTE;

	}

	return -1;
}

void CPlayerPed::GiveWeapon(int iWeaponID, int iAmmo)
{
	int iModelID = 0;
	iModelID = GameGetWeaponModelIDFromWeaponID(iWeaponID);
	
	if (iModelID == -1) return;
	
	if (!pGame->IsModelLoaded(iModelID)) 
	{
		pGame->RequestModel(iModelID);
		pGame->LoadRequestedModels();
		while (!pGame->IsModelLoaded(iModelID)) sleep(1);
	}

	((int(*)(uintptr_t, unsigned int, int))(g_libGTASA + 0x0043429C + 1))((uintptr_t)m_pPed, iWeaponID, iAmmo); // CPed::GiveWeapon(thisptr, weapoid, ammo)
	((int(*)(uintptr_t, unsigned int))(g_libGTASA + 0x00434528 + 1))((uintptr_t)m_pPed, iWeaponID);	// CPed::SetCurrentWeapon(thisptr, weapid)
}

void CPlayerPed::SetArmedWeapon(int iWeaponID)
{
	((int(*)(uintptr_t, unsigned int))(g_libGTASA + 0x00434528 + 1))((uintptr_t)m_pPed, iWeaponID);	// CPed::SetCurrentWeapon(thisptr, weapid)
}

void CPlayerPed::SetPlayerAimState()
{
	uintptr_t ped = (uintptr_t)m_pPed;
	uint8_t old = *(uint8_t*)(g_libGTASA + 0x008E864C); // CWorld::PlayerInFocus - 0x008E864C
	*(uint8_t*)(g_libGTASA + 0x008E864C) = m_bytePlayerNumber;

	((uint32_t(*)(uintptr_t, int, int, int))(g_libGTASA + 0x00454A6C + 1))(ped, 1, 1, 1); // CPlayerPed::ClearWeaponTarget
	*(uint8_t *)(*(uint32_t *)(ped + 1088) + 52) = *(uint8_t *)(*(uint32_t *)(ped + 1088) + 52) & 0xF7 | 8 * (1 & 1); // magic 

	*(uint8_t*)(g_libGTASA + 0x008E864C) = old;
}

void CPlayerPed::ClearPlayerAimState()
{
	uintptr_t ped = (uintptr_t)m_pPed;
	uint8_t old = *(uint8_t*)(g_libGTASA + 0x008E864C);	// CWorld::PlayerInFocus - 0x008E864C
	*(uint8_t*)(g_libGTASA + 0x008E864C) = m_bytePlayerNumber;

	*(uint32_t *)(ped + 1432) = 0;	// unk
	((uint32_t(*)(uintptr_t, int, int, int))(g_libGTASA + 0x00454A6C + 1))(ped, 0, 0, 0);	// CPlayerPed::ClearWeaponTarget
	*(uint8_t *)(*(uint32_t *)(ped + 1088) + 52) = *(uint8_t *)(*(uint32_t *)(ped + 1088) + 52) & 0xF7 | 8 * (0 & 1);	// magic...

	*(uint8_t*)(g_libGTASA + 0x008E864C) = old;
}

uint8_t CPlayerPed::GetCurrentWeapon()
{
	if (!m_pPed) return 0;
	if (GamePool_Ped_GetAt(m_dwGTAId) == 0) return 0;

	uint32_t dwRetVal;
	ScriptCommand(&get_actor_armed_weapon, m_dwGTAId, &dwRetVal);
	return (uint8_t)dwRetVal;
}

// 0.3.7
bool CPlayerPed::IsAPassenger()
{
	if(m_pPed->pVehicle && IN_VEHICLE(m_pPed))
	{
		VEHICLE_TYPE *pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle;

		if(	pVehicle->pDriver != m_pPed ||
			pVehicle->entity.nModelIndex == TRAIN_PASSENGER ||
			pVehicle->entity.nModelIndex == TRAIN_FREIGHT )
			return true;
	}

	return false;
}

// 0.3.7
VEHICLE_TYPE* CPlayerPed::GetGtaVehicle()
{
	return (VEHICLE_TYPE*)m_pPed->pVehicle;
}

// 0.3.7
void CPlayerPed::RemoveFromVehicleAndPutAt(float fX, float fY, float fZ)
{
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(m_pPed && IN_VEHICLE(m_pPed))
		ScriptCommand(&remove_actor_from_car_and_put_at, m_dwGTAId, fX, fY, fZ);
}

// 0.3.7
void CPlayerPed::SetInitialState()
{
	(( void (*)(PED_TYPE*))(g_libGTASA+0x458D1C+1))(m_pPed);
}

// 0.3.7
void CPlayerPed::SetHealth(float fHealth)
{
	if(!m_pPed) return;
	m_pPed->fHealth = fHealth;
}

// 0.3.7
float CPlayerPed::GetHealth()
{
	if(!m_pPed) return 0.0f;
	return m_pPed->fHealth;
}

// 0.3.7
void CPlayerPed::SetArmour(float fArmour)
{
	if(!m_pPed) return;
	m_pPed->fArmour = fArmour;
}

float CPlayerPed::GetArmour()
{
	if(!m_pPed) return 0.0f;
	return m_pPed->fArmour;
}

void CPlayerPed::SetInterior(uint8_t byteID)
{
	if(!m_pPed) return;

	ScriptCommand(&select_interior, byteID);
	ScriptCommand(&link_actor_to_interior, m_dwGTAId, byteID);

	MATRIX4X4 mat;
	GetMatrix(&mat);
	ScriptCommand(&refresh_streaming_at, mat.pos.X, mat.pos.Y);
}

void CPlayerPed::PutDirectlyInVehicle(int iVehicleID, int iSeat)
{
	if(!m_pPed) return;
	if(!GamePool_Vehicle_GetAt(iVehicleID)) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	/* ��������
	if(GetCurrentWeapon() == WEAPON_PARACHUTE) {
		SetArmedWeapon(0);
	}*/

	VEHICLE_TYPE *pVehicle = GamePool_Vehicle_GetAt(iVehicleID);

	if(pVehicle->fHealth == 0.0f) return;
	// check is cplaceable
	if (pVehicle->entity.vtable == g_libGTASA+0x5C7358) return;
	// check seatid (��������)

	if(iSeat == 0)
	{
		if(pVehicle->pDriver && IN_VEHICLE(pVehicle->pDriver)) return;
		ScriptCommand(&put_actor_in_car, m_dwGTAId, iVehicleID);
	}
	else
	{
		iSeat--;
		ScriptCommand(&put_actor_in_car2, m_dwGTAId, iVehicleID, iSeat);
	}

	if(m_pPed == GamePool_FindPlayerPed() && IN_VEHICLE(m_pPed))
		pGame->GetCamera()->SetBehindPlayer();

	if(pNetGame)
	{
		// �������� (��������)
	}
}

void CPlayerPed::EnterVehicle(int iVehicleID, bool bPassenger)
{
	if(!m_pPed) return;
	VEHICLE_TYPE* ThisVehicleType;
	if((ThisVehicleType = GamePool_Vehicle_GetAt(iVehicleID)) == 0) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(bPassenger)
	{
		if(ThisVehicleType->entity.nModelIndex == TRAIN_PASSENGER &&
			(m_pPed == GamePool_FindPlayerPed()))
		{
			ScriptCommand(&put_actor_in_car2, m_dwGTAId, iVehicleID, -1);
		}
		else
		{
			ScriptCommand(&send_actor_to_car_passenger,m_dwGTAId,iVehicleID, 3000, -1);
		}
	}
	else
		ScriptCommand(&send_actor_to_car_driverseat, m_dwGTAId, iVehicleID, 3000);
}

// 0.3.7
void CPlayerPed::ExitCurrentVehicle()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	VEHICLE_TYPE* ThisVehicleType = 0;

	if(IN_VEHICLE(m_pPed))
	{
		if(GamePool_Vehicle_GetIndex((VEHICLE_TYPE*)m_pPed->pVehicle))
		{
			int index = GamePool_Vehicle_GetIndex((VEHICLE_TYPE*)m_pPed->pVehicle);
			ThisVehicleType = GamePool_Vehicle_GetAt(index);
			if(ThisVehicleType)
			{
				if(	ThisVehicleType->entity.nModelIndex != TRAIN_PASSENGER &&
					ThisVehicleType->entity.nModelIndex != TRAIN_PASSENGER_LOCO)
				{
					ScriptCommand(&make_actor_leave_car, m_dwGTAId, GetCurrentVehicleID());
				}
			}
		}
	}
}

// 0.3.7
int CPlayerPed::GetCurrentVehicleID()
{
	if(!m_pPed) return 0;

	VEHICLE_TYPE *pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle;
	return GamePool_Vehicle_GetIndex(pVehicle);
}

int CPlayerPed::GetVehicleSeatID()
{
	VEHICLE_TYPE *pVehicle;

	if( GetActionTrigger() == ACTION_INCAR && (pVehicle = (VEHICLE_TYPE *)m_pPed->pVehicle) != 0 ) 
	{
		if(pVehicle->pDriver == m_pPed) return 0;
		if(pVehicle->pPassengers[0] == m_pPed) return 1;
		if(pVehicle->pPassengers[1] == m_pPed) return 2;
		if(pVehicle->pPassengers[2] == m_pPed) return 3;
		if(pVehicle->pPassengers[3] == m_pPed) return 4;
		if(pVehicle->pPassengers[4] == m_pPed) return 5;
		if(pVehicle->pPassengers[5] == m_pPed) return 6;
		if(pVehicle->pPassengers[6] == m_pPed) return 7;
	}

	return (-1);
}

/*bool CPlayerPed::SetPlayerControl(bool bFlag)
{
	m_bPlayerControl = bFlag;
	return true;
}*/
// 0.3.7
void CPlayerPed::TogglePlayerControllable(bool bToggle)
{
	MATRIX4X4 mat;

	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!bToggle)
	{
		ScriptCommand(&toggle_player_controllable, m_bytePlayerNumber, 0);
		ScriptCommand(&lock_actor, m_dwGTAId, 1);
	}
	else
	{
		ScriptCommand(&toggle_player_controllable, m_bytePlayerNumber, 1);
		ScriptCommand(&lock_actor, m_dwGTAId, 0);
		if(!IsInVehicle()) 
		{
			GetMatrix(&mat);
			TeleportTo(mat.pos.X, mat.pos.Y, mat.pos.Z);
		}
	}
}

// 0.3.7
void CPlayerPed::TogglePlayerControllableWithoutLock(bool bToggle)
{
	MATRIX4X4 mat;

	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!bToggle)
	{
		ScriptCommand(&toggle_player_controllable, m_bytePlayerNumber, 0);
	}
	else
	{
		ScriptCommand(&toggle_player_controllable, m_bytePlayerNumber, 1);
		if(!IsInVehicle()) 
		{
			GetMatrix(&mat);
			TeleportTo(mat.pos.X, mat.pos.Y, mat.pos.Z);
		}
	}
}

// 0.3.7
void CPlayerPed::SetModelIndex(unsigned int uiModel)
{
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;
	if(!IsPedModel(uiModel))
		uiModel = 0;

	if(m_pPed)
	{
		// CClothes::RebuildPlayer nulled
		WriteMemory(g_libGTASA+0x3F1030, (uintptr_t)"\x70\x47", 2);
		DestroyFollowPedTask();
		CEntity::SetModelIndex(uiModel);

		// reset the Ped Audio Attributes
		(( void (*)(uintptr_t, uintptr_t))(g_libGTASA+0x34B2A8+1))(((uintptr_t)m_pPed+660), (uintptr_t)m_pPed);
	}
}

// ��������
void CPlayerPed::DestroyFollowPedTask()
{

}

// ��������
void CPlayerPed::ClearAllWeapons()
{
	uintptr_t dwPedPtr = (uintptr_t)m_pPed;

	*(uint8_t*)(g_libGTASA + 0x008E864C) = m_bytePlayerNumber;

	reinterpret_cast<int(*)(uintptr_t)>(g_libGTASA+0x4345AC+1)(dwPedPtr);

	*(uint8_t*)(g_libGTASA + 0x008E864C) = 0;
}

// ��������
void CPlayerPed::ResetDamageEntity()
{

}

// 0.3.7
void CPlayerPed::RestartIfWastedAt(VECTOR *vecRestart, float fRotation)
{	
	ScriptCommand(&restart_if_wasted_at, vecRestart->X, vecRestart->Y, vecRestart->Z, fRotation, 0);
}

// 0.3.7
void CPlayerPed::ForceTargetRotation(float fRotation)
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->fRotation1 = DegToRad(fRotation);
	m_pPed->fRotation2 = DegToRad(fRotation);

	ScriptCommand(&set_actor_z_angle,m_dwGTAId,fRotation);
}

void CPlayerPed::SetRotation(float fRotation)
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->fRotation1 = DegToRad(fRotation);
	m_pPed->fRotation2 = DegToRad(fRotation);
}

// 0.3.7
uint8_t CPlayerPed::GetActionTrigger()
{
	return (uint8_t)m_pPed->dwAction;
}

void CPlayerPed::SetActionTrigger(uint8_t action)
{
	m_pPed->dwAction = (uint32_t)action;
}


// 0.3.7
bool CPlayerPed::IsDead()
{
	if(!m_pPed) return true;
	if(m_pPed->fHealth > 0.0f) return false;
	return true;
}

void CPlayerPed::SetMoney(int iAmount)
{
	ScriptCommand(&set_actor_money, m_dwGTAId, 0);
	ScriptCommand(&set_actor_money, m_dwGTAId, iAmount);
}

// 0.3.7
void CPlayerPed::ShowMarker(uint32_t iMarkerColorID)
{
	if(m_dwArrow) HideMarker();
	ScriptCommand(&create_arrow_above_actor, m_dwGTAId, &m_dwArrow);
	ScriptCommand(&set_marker_color, m_dwArrow, iMarkerColorID);
	ScriptCommand(&show_on_radar2, m_dwArrow, 2);
}

// 0.3.7
void CPlayerPed::HideMarker()
{
	if(m_dwArrow) ScriptCommand(&disable_marker, m_dwArrow);
	m_dwArrow = 0;
}

// 0.3.7
void CPlayerPed::SetFightingStyle(int iStyle)
{
	if(!m_pPed) return;
	ScriptCommand( &set_fighting_style, m_dwGTAId, iStyle, 6 );
}

// 0.3.7
void CPlayerPed::ApplyAnimation( char *szAnimName, char *szAnimFile, float fT,
								 int opt1, int opt2, int opt3, int opt4, int iUnk )
{
	int iWaitAnimLoad = 0;

	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	if(!strcasecmp(szAnimFile,"SEX")) return;

	if(!pGame->IsAnimationLoaded(szAnimFile))
	{
		pGame->RequestAnimation(szAnimFile);
		while(!pGame->IsAnimationLoaded(szAnimFile))
		{
			usleep(1000);
			iWaitAnimLoad++;
			if(iWaitAnimLoad > 15) return;
		}
	}

	ScriptCommand(&apply_animation, m_dwGTAId, szAnimName, szAnimFile, fT, opt1, opt2, opt3, opt4, iUnk);
}

unsigned char CPlayerPed::FindDeathReasonAndResponsiblePlayer(PLAYERID *nPlayer)
{
	unsigned char byteDeathReason;
	PLAYERID bytePlayerIDWhoKilled;
	CVehiclePool *pVehiclePool;
	CPlayerPool *pPlayerPool;

	// grab the vehicle/player pool now anyway, even though we may not need it.
	if(pNetGame) {
		pVehiclePool = pNetGame->GetVehiclePool();
		pPlayerPool = pNetGame->GetPlayerPool();
	}
	else { // just leave if there's no netgame.
		*nPlayer = INVALID_PLAYER_ID;
		return WEAPON_UNKNOWN;
	}

	if(m_pPed) 
	{
		byteDeathReason = (unsigned char)m_pPed->dwWeaponUsed;
		if(byteDeathReason < WEAPON_CAMERA || byteDeathReason == WEAPON_HELIBLADES || byteDeathReason == WEAPON_EXPLOSION) { // It's a weapon of some sort.

			if(m_pPed->pdwDamageEntity) { // check for a player pointer.
				
				bytePlayerIDWhoKilled = pPlayerPool->
					FindRemotePlayerIDFromGtaPtr((PED_TYPE *)m_pPed->pdwDamageEntity);

				if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
					// killed by another player with a weapon, this is all easy.
					*nPlayer = bytePlayerIDWhoKilled;
					return byteDeathReason;
				} else { // could be a vehicle
					if(pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)m_pPed->pdwDamageEntity) != INVALID_VEHICLE_ID) {
						VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pdwDamageEntity;
						bytePlayerIDWhoKilled = pPlayerPool->
							FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pGtaVehicle->pDriver);
												
						if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
							*nPlayer = bytePlayerIDWhoKilled;
							return byteDeathReason;
						}
					}
				}
			}
			//else { // weapon was used but who_killed is 0 (?)
			*nPlayer = INVALID_PLAYER_ID;
			return WEAPON_UNKNOWN;
			//}
		}
		else if(byteDeathReason == WEAPON_DROWN) {
			*nPlayer = INVALID_PLAYER_ID;
			return WEAPON_DROWN;
		}
		else if(byteDeathReason == WEAPON_VEHICLE) {

			if(m_pPed->pdwDamageEntity) {
				// now, if we can find the vehicle
				// we can probably derive the responsible player.
				// Look in the vehicle pool for this vehicle.
				if(pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)m_pPed->pdwDamageEntity) != INVALID_VEHICLE_ID)
				{
					VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pdwDamageEntity;

					bytePlayerIDWhoKilled = pPlayerPool->
						FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pGtaVehicle->pDriver);
											
					if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
						*nPlayer = bytePlayerIDWhoKilled;
						return WEAPON_VEHICLE;
					}
				}									
			}
		}
		else if(byteDeathReason == WEAPON_COLLISION) {

			if(m_pPed->pdwDamageEntity) {
				if(pVehiclePool->FindIDFromGtaPtr((VEHICLE_TYPE *)m_pPed->pdwDamageEntity) != INVALID_VEHICLE_ID)
				{
					VEHICLE_TYPE *pGtaVehicle = (VEHICLE_TYPE *)m_pPed->pdwDamageEntity;
										
					bytePlayerIDWhoKilled = pPlayerPool->
						FindRemotePlayerIDFromGtaPtr((PED_TYPE *)pGtaVehicle->pDriver);
						
					if(bytePlayerIDWhoKilled != INVALID_PLAYER_ID) {
						*nPlayer = bytePlayerIDWhoKilled;
						return WEAPON_COLLISION;
					}
				}
				else {
					*nPlayer = INVALID_PLAYER_ID;
					return WEAPON_COLLISION;
				}
			}
		}
	}

	// Unhandled death type.
	*nPlayer = INVALID_PLAYER_ID;
	return WEAPON_UNKNOWN;
}

// 0.3.7
void CPlayerPed::GetBonePosition(int iBoneID, VECTOR* vecOut)
{
	if(!m_pPed) return;
	if(m_pEntity->vtable == g_libGTASA+0x5C7358) return;

	(( void (*)(PED_TYPE*, VECTOR*, int, int))(g_libGTASA+0x436590+1))(m_pPed, vecOut, iBoneID, 0);
}

ENTITY_TYPE* CPlayerPed::GetEntityUnderPlayer()
{
	uintptr_t entity;
	VECTOR vecStart;
	VECTOR vecEnd;
	char buf[100];

	if(!m_pPed) return nullptr;
	if( IN_VEHICLE(m_pPed) || !GamePool_Ped_GetAt(m_dwGTAId))
		return 0;

	vecStart.X = m_pPed->entity.mat->pos.X;
	vecStart.Y = m_pPed->entity.mat->pos.Y;
	vecStart.Z = m_pPed->entity.mat->pos.Z - 0.25f;

	vecEnd.X = m_pPed->entity.mat->pos.X;
	vecEnd.Y = m_pPed->entity.mat->pos.Y;
	vecEnd.Z = vecStart.Z - 1.75f;

	LineOfSight(&vecStart, &vecEnd, (void*)buf, (uintptr_t)&entity,
		0, 1, 0, 1, 0, 0, 0, 0);

	return (ENTITY_TYPE*)entity;
}

// ��������
uint16_t CPlayerPed::GetKeys(uint16_t *lrAnalog, uint16_t *udAnalog)
{
	*lrAnalog = LocalPlayerKeys.wKeyLR;
	*udAnalog = LocalPlayerKeys.wKeyUD;
	uint16_t wRet = 0;

	// KEY_ANALOG_RIGHT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_RIGHT]) wRet |= 1;
	wRet <<= 1;
	// KEY_ANALOG_LEFT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_LEFT]) wRet |= 1;
	wRet <<= 1;
	// KEY_ANALOG_DOWN
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_DOWN]) wRet |= 1;
	wRet <<= 1;
	// KEY_ANALOG_UP
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ANALOG_UP]) wRet |= 1;
	wRet <<= 1;
	// KEY_WALK
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK]) wRet |= 1;
	wRet <<= 1;
	// KEY_SUBMISSION
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION]) wRet |= 1;
	wRet <<= 1;
	// KEY_WALK
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK]) wRet |= 1;
	wRet <<= 1;
	// KEY_SUBMISSION
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION]) wRet |= 1;
	wRet <<= 1;
	// KEY_LOOK_LEFT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_LOOK_LEFT]) wRet |= 1;
	wRet <<= 1;

	if (GetCameraMode() == 0x35)
	{
		LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE] = 1;
	}
	else
	{
		LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE] = 0;
	}

	// KEY_HANDBRAKE
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE]/*true*/) wRet |= 1;
	wRet <<= 1;
	// KEY_LOOK_RIGHT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_LOOK_RIGHT]) wRet |= 1;
	wRet <<= 1;
	// KEY_JUMP
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_JUMP]) wRet |= 1;
	wRet <<= 1;
	// KEY_SECONDARY_ATTACK
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK]) wRet |= 1;
	wRet <<= 1;
	// KEY_SPRINT
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT]) wRet |= 1;
	wRet <<= 1;
	// KEY_FIRE
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE]) wRet |= 1;
	wRet <<= 1;
	// KEY_CROUCH
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH]) wRet |= 1;
	wRet <<= 1;
	// KEY_ACTION
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_ACTION]) wRet |= 1;

	memset(LocalPlayerKeys.bKeys, 0, ePadKeys::SIZE);

	return wRet;
}

WEAPON_SLOT_TYPE * CPlayerPed::GetCurrentWeaponSlot()
{
	if (m_pPed) 
	{
		return &m_pPed->WeaponSlots[m_pPed->byteCurWeaponSlot];
	}
	return NULL;
}

uint8_t CPlayerPed::GetExtendedKeys()
{
	uint8_t result = 0;
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_YES])
	{
		LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = false;
		result = 1;
	}
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_NO])
	{
		LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = false;
		result = 2;
	}
	if(LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK])
	{
		LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = false;
		result = 3;
	}
	
	return result;
}

void CPlayerPed::SetKeys(uint16_t wKeys, uint16_t lrAnalog, uint16_t udAnalog)
{
	PAD_KEYS *pad = &RemotePlayerKeys[m_bytePlayerNumber];

	// LEFT/RIGHT
	pad->wKeyLR = lrAnalog;
	// UP/DOWN
	pad->wKeyUD = udAnalog;

	// KEY_ACTION
	pad->bKeys[ePadKeys::KEY_ACTION] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_CROUCH
	pad->bKeys[ePadKeys::KEY_CROUCH] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_FIRE
	pad->bKeys[ePadKeys::KEY_FIRE] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_SPRINT
	pad->bKeys[ePadKeys::KEY_SPRINT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_SECONDARY_ATTACK
	pad->bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_JUMP
	pad->bKeys[ePadKeys::KEY_JUMP] = (wKeys & 1);
	if(!pad->bKeys[ePadKeys::KEY_JUMP]) pad->bIgnoreJump = false;
	wKeys >>= 1;
	// KEY_LOOK_RIGHT
	pad->bKeys[ePadKeys::KEY_LOOK_RIGHT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_HANDBRAKE
	pad->bKeys[ePadKeys::KEY_HANDBRAKE] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_LOOK_LEFT
	pad->bKeys[ePadKeys::KEY_LOOK_LEFT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_SUBMISSION
	pad->bKeys[ePadKeys::KEY_SUBMISSION] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_WALK
	pad->bKeys[ePadKeys::KEY_WALK] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_UP
	pad->bKeys[ePadKeys::KEY_ANALOG_UP] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_DOWN
	pad->bKeys[ePadKeys::KEY_ANALOG_DOWN] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_LEFT
	pad->bKeys[ePadKeys::KEY_ANALOG_LEFT] = (wKeys & 1);
	wKeys >>= 1;
	// KEY_ANALOG_RIGHT
	pad->bKeys[ePadKeys::KEY_ANALOG_RIGHT] = (wKeys & 1);

	return;
}

void CPlayerPed::SetAimZ(float fAimZ)
{
	*(float*)(*((uintptr_t*)m_pPed + 272) + 84) = fAimZ;
	//m_pPed + 272 - dwPlayerInfo
}

float CPlayerPed::GetAimZ()
{
	return *(float*)(*((uintptr_t*)m_pPed + 272) + 84);
}

void CPlayerPed::ExtinguishFire()
{
	if (m_pPed)
	{
		// Remove ped's fire if any
		uintptr_t pFireObject = m_pPed->pFireObject;
		if (pFireObject != NULL)
		{
			reinterpret_cast<int(*)(uintptr_t)>(g_libGTASA + 0x39670C + 1)(pFireObject);
		}

		// Remove ped's vehicle's fire if any
		/*if (IN_VEHICLE(m_pPed))
		{
			VEHICLE_TYPE *pVeh = (VEHICLE_TYPE *)m_pPed->pVehicle;
			pFireObject = pVeh->pFireObject;

			if (pFireObject)
			{
				reinterpret_cast<int(*)(uintptr_t)>(g_libGTASA + 0x39670C + 1)(pFireObject);
			}
		}*/
	}
}

void CPlayerPed::SetDead()
{
	if(m_dwGTAId && m_pPed) {
		ExtinguishFire();

		MATRIX4X4 mat;
		GetMatrix(&mat);
		// will reset the tasks
		TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
        m_pPed->fHealth = 0.0f;
		*(uint8_t*)(g_libGTASA + 0x008E864C) = m_bytePlayerNumber;
		ScriptCommand(&kill_actor,m_dwGTAId);
		*(uint8_t*)(g_libGTASA + 0x008E864C) = 0;
	}
}

char DanceStyleLibs[4][16] = {"WOP","GFUNK","RUNNINGMAN","STRIP"};
char DanceIdleLoops[4][16] = {"DANCE_LOOP","DANCE_LOOP","DANCE_LOOP","STR_Loop_B"};

char szDanceAnimNamesFemale[16][16] = {
	"DANCE_G1","DANCE_G2","DANCE_G3","DANCE_G4",
	"DANCE_G5","DANCE_G6","DANCE_G7","DANCE_G8",
	"DANCE_G9","DANCE_G10","DANCE_G11","DANCE_G12",
	"DANCE_G13","DANCE_G14","DANCE_G15","DANCE_G16"
};

char szDanceAnimNamesMale[16][16] = {
	"DANCE_B1","DANCE_B2","DANCE_B3","DANCE_B4",
	"DANCE_B5","DANCE_B6","DANCE_B7","DANCE_B8",
	"DANCE_B9","DANCE_B10","DANCE_B11","DANCE_B12",
	"DANCE_B13","DANCE_B14","DANCE_B15","DANCE_B16"
};

char szStripAnims[16][16] = {
	"strip_A","strip_B","strip_C","strip_D",
	"strip_E","strip_F","strip_G","STR_A2B",
	"STR_B2A","STR_B2C","STR_C1","STR_C2",
	"STR_C2B","STR_A2B","STR_B2C","STR_C2"
};

void CPlayerPed::StartDancing(int iStyle)
{
	if(iStyle < 0 || iStyle > 3) return;

	/* if(iStyle == 3 && GetPedStat() != 5 && GetPedStat() != 22) {
		// stops male stripping
		return;
	}*/

	m_iDancingState = 1;
	m_iDancingStyle = iStyle;
	
	ApplyAnimation(DanceIdleLoops[m_iDancingStyle],DanceStyleLibs[m_iDancingStyle],16.0,1,0,0,0,-1);
}

void CPlayerPed::StopDancing()
{
	m_iDancingState = 0;
	MATRIX4X4 mat;
	GetMatrix(&mat);
	TeleportTo(mat.pos.X,mat.pos.Y,mat.pos.Z);
}

bool CPlayerPed::IsDancing()
{
	if(m_iDancingState) return true;
	return false;
}

void CPlayerPed::SetAmmo(unsigned char byteWeapon, unsigned short wordAmmo)
{
	if(m_pPed)
	{
		WEAPON_SLOT_TYPE *WeaponSlot = FindWeaponSlot((uint32_t)byteWeapon);
		if(!WeaponSlot) return;
		WeaponSlot->dwAmmo = (uint32_t)wordAmmo;
	}
}

WEAPON_SLOT_TYPE *CPlayerPed::FindWeaponSlot(uint32_t dwWeapon)
{
	if(m_pPed)
	{
		uint8_t i;
		for(i = 0; i < 13; i++)
		{
			if(m_pPed->WeaponSlots[i].dwType == dwWeapon) return &m_pPed->WeaponSlots[i];
		}
	}
	return NULL;
}

void CPlayerPed::ApplyCrouch()
{
	uintptr_t ped = (uintptr_t)m_pPed;
	reinterpret_cast<int(*)(uintptr_t, uint16_t)>(g_libGTASA+0x44E0F4+1)(*((uintptr_t*)ped + 271), 0);
}

void CPlayerPed::ResetCrouch()
{
	m_pPed->dwStateFlags &= 0xFBFFFFFF;
}

bool CPlayerPed::IsCrouching()
{
	return IS_CROUCHING(m_pPed);
}

// 0.3.7
void CPlayerPed::GetBonePosition(VECTOR &out, unsigned int boneid, bool update)
{
	if(m_pPed)
	{
		((void(*)(uintptr_t, VECTOR &, unsigned int, bool))(g_libGTASA+0x436590+1))((uintptr_t)m_pPed, out, boneid, update);
	}
}

int GameBoneOffsets[] = {
	BONE_SPINE1, // spine
	BONE_HEAD, // head
	BONE_LEFTSHOULDER, // lupper
	BONE_RIGHTSHOULDER, // rupper
	BONE_LEFTHAND, // left hand
	BONE_RIGHTHAND, // right hand
	BONE_LEFTHIP, // left thigh
	BONE_RIGHTHIP, // right thigh
	BONE_LEFTFOOT, // left foot
	BONE_RIGHTFOOT, // right foot
	BONE_RIGHTANKLE, // right calf
	BONE_LEFTANKLE, // left calf
	BONE_LEFTELBOW, // left forearm
	BONE_RIGHTELBOW, // right forearm
	BONE_LEFTUPPERTORSO, // left shoulder
	BONE_RIGHTUPPERTORSO, // right shoulder
	BONE_NECK, // neck
	BONE_HEAD1 // I don't know if this right bone, but in samp this is "JAW"
};

bool CPlayerPed::SetPlayerAttachedObject(int index, int model, unsigned int bone, VECTOR offset, VECTOR rot)
{
	if(index < 0 || index > 9) return false;
	
	/*if(m_pAttachedObjects[index] != 0)
	{
		Log("13");
		// Replace attached object to ours
		delete m_pAttachedObjects[index];
		Log("14");
		m_pAttachedObjects[index] = 0; 
		Log("15");
	}*/

	// Change attached object to ours
	if(m_bAOSlotState[index] == true)
	{
		delete m_pAttachedObjects[index];
		m_pAttachedObjects[index] = 0;
		m_bAOSlotState[index] = false;
	}

	// Get bone position
	/*if(!bone || bone >= 11 && bone <= 14 || bone > 18)
	{
		#ifdef RELEASE_BETA
		if(pChatWindow)	pChatWindow->AddDebugMessage("Can't determite bone position.");
		#endif
		return false;
	}*/

	bone--;
	if(GameBoneOffsets[bone] == -1)
		return false;

	VECTOR vecBonePos;
	GetBonePosition(vecBonePos, GameBoneOffsets[bone], true);
	/*switch(bone)
	{
		// Spine
		case 1:
		GetBonePosition(BONE_SPINE1, &vecBonePos);
		break;
		// Head
		case 2:
		GetBonePosition(BONE_HEAD, &vecBonePos);
		break;
		// Left upper arm
		case 3:
		GetBonePosition(BONE_LEFTUPPERTORSO, &vecBonePos);
		break;
		// Right upper arm
		case 4:
		GetBonePosition(BONE_RIGHTUPPERTORSO, &vecBonePos);
		break;
		// Left hand
		case 5:
		GetBonePosition(BONE_LEFTHAND, &vecBonePos);
		break;
		// Right hand
		case 6:
		GetBonePosition(BONE_RIGHTHAND, &vecBonePos);
		break;
		// Left thigh
		case 7:
		GetBonePosition(BONE_LEFTHIP, &vecBonePos);
		break;
		// Right thigh
		case 8:
		GetBonePosition(BONE_RIGHTHIP, &vecBonePos);
		break;
		// Left foot
		case 9:
		GetBonePosition(BONE_LEFTFOOT, &vecBonePos);
		break;
		// Right foot
		case 10:
		GetBonePosition(BONE_RIGHTFOOT, &vecBonePos);
		break;
		// Right calf
		case 11:
		
		break;
		// Left calf
		case 12:

		break;
		// Left forearm
		case 13:
		
		break;
		// Right forearm
		case 14:

		break;
		// Left shoulder
		case 15:
		GetBonePosition(BONE_LEFTSHOULDER, &vecBonePos);
		break;
		// Right shoulder
		case 16:
		GetBonePosition(BONE_RIGHTSHOULDER, &vecBonePos);
		break;
		// Neck
		case 17:
		GetBonePosition(BONE_NECK, &vecBonePos);
		break;
		// Jaw
		case 18:
		GetBonePosition(BONE_HEAD2, &vecBonePos);
		break;
		default: return false;
	}*/

	// Create the object
	m_pAttachedObjects[index] = new CObject(model, vecBonePos.X, vecBonePos.Y, vecBonePos.Z, rot, 100.0f);
	if(!m_pAttachedObjects[index]) return false;
	m_bAOSlotState[index] = true;

	// Remove the object collision
	ScriptCommand(&set_object_collision, m_pAttachedObjects[index]->m_dwGTAId, 0);

	// Calculate offsets
	MATRIX4X4 mat;
	GetMatrix(&mat);

	VECTOR offs;
	/*offs.X = FloatOffset(vecBonePos.X, mat.pos.X);
	offs.Y = FloatOffset(vecBonePos.Y, mat.pos.Y);
	offs.Z = FloatOffset(vecBonePos.Z, mat.pos.Z);*/

	// We should get the offset for bone, but the problem of FloatOffset that's we can't get the NEGATIVE offset
	// For example left hand < player position, but we'll get right hand cause FloatOffset will return positive offset
	offs.X = vecBonePos.X - mat.pos.X;
	offs.Y = vecBonePos.Y - mat.pos.Y;
	offs.Z = vecBonePos.Z - mat.pos.Z;

	// We don't care about signs due to 300 + (-300) = 0
	offs.X += offset.X;
	offs.Y += offset.Y;
	offs.Z += offset.Z;

	// Reverse the rotation offsets
	/*rot.X *= (-1.0f);
	rot.Y *= (-1.0f);
	rot.Z *= (-1.0f);*/

	/*rot.X /= 180.0f;

	rot.Y /= 180.0f;

	rot.Z /= 180.0f;*/

	if(offs.X != 0.0f)
	{
		RwMatrixRotate(&mat, 0, offs.X);		}
	if(offs.Y != 0.0f)
	{
		RwMatrixRotate(&mat, 1, offs.Y);
	}
	if(offs.Z != 0.0f)
	{
		RwMatrixRotate(&mat, 2, offs.Z);
	}

	
	// Attach object to player
	try
	{
		ScriptCommand( &attach_object_to_actor, m_pAttachedObjects[index]->m_dwGTAId, m_dwGTAId,
					offs.X,
					offs.Y,
					offs.Z, 
					rot.X,
					rot.Y,
					rot.Z);
	} catch(...) {
		#ifdef RELEASE_BETA
		pChatWindow->AddDebugMessage("unhandled exception when tried to attach the object");
		#endif
		return false;
	}

	// Let's try to use attach function directly instead of using scm
	// CEntity *CPhysical::AttachEntityToEntity(CEntity *entity, float offsetX, float offsetY,
	// float offsetZ, float rotX, float rotY, float rotZ);
	//((void(*)(uintptr_t, uintptr_t, VECTOR, VECTOR))(g_libGTASA+0x3A9EFC+1))((uintptr_t)m_pPed, (uintptr_t)m_pAttachedObjects[index]->m_pEntity, offs, rot);
	return true;
}

void CPlayerPed::DestroyPlayerAttachedObject(int index)
{
	if(index < 0 || index > 9) return;
	if(m_bAOSlotState[index] == true)
	{
		delete m_pAttachedObjects[index];
		m_pAttachedObjects[index] = nullptr;
		m_bAOSlotState[index] = true;
	}
}

void CPlayerPed::ApplyCommandTask(char *szTaskName, int p1, int p2, int p3, VECTOR *p4, int p5, int p6, int p7, int p8, int p9)
{
	((int(*)(uintptr_t, char*, uintptr_t, int,int,int,VECTOR*,int,int,int,int,int))(g_libGTASA+0x463188+1))(g_libGTASA+0x958484, szTaskName, (uintptr_t)m_pPed, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

void CPlayerPed::DisableAutoAim()
{
	if(!m_pPed) return;
	if(!GamePool_Ped_GetAt(m_dwGTAId)) return;

	ScriptCommand(&set_char_never_targetted, m_dwGTAId, 1);
}