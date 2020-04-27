#pragma once

#include "aimstuff.h"
#include "object.h"

class CPlayerPed
	: public CEntity
{
public:
	CPlayerPed();	// local
	CPlayerPed(uint8_t bytePlayerNumber, int iSkin, float fX, float fY, float fZ, float fRotation); // remote
	~CPlayerPed();

	void Destroy();

	CAMERA_AIM * GetCurrentAim();
	void SetCurrentAim(CAMERA_AIM *pAim);

	uint16_t GetCameraMode();

	void SetCameraMode(uint16_t byteCamMode);

	float GetCameraExtendedZoom();

	void SetCameraExtendedZoom(float fZoom);

	// 0.3.7
	bool IsInVehicle();
	// 0.3.7
	bool IsAPassenger();
	// 0.3.7
	VEHICLE_TYPE* GetGtaVehicle();
	// 0.3.7
	void RemoveFromVehicleAndPutAt(float fX, float fY, float fZ);
	// 0.3.7
	void SetInitialState();
	// 0.3.7
	void SetHealth(float fHealth);
	void SetArmour(float fArmour);
	// 0.3.7
	float GetHealth();
	float GetArmour();
	// 0.3.7
	void TogglePlayerControllable(bool bToggle);
	void TogglePlayerControllableWithoutLock(bool bToggle);
	// 0.3.7
	void SetModelIndex(unsigned int uiModel);

	void SetInterior(uint8_t byteID);

	void PutDirectlyInVehicle(int iVehicleID, int iSeat);
	void EnterVehicle(int iVehicleID, bool bPassenger);
	// 0.3.7
	void ExitCurrentVehicle();
	// 0.3.7
	int GetCurrentVehicleID();
	int GetVehicleSeatID();

	ENTITY_TYPE* GetEntityUnderPlayer();

	void GiveWeapon(int iWeaponID, int iAmmo);
	uint8_t GetCurrentWeapon();
	void SetArmedWeapon(int iWeaponID);

	void SetPlayerAimState();
	void ClearPlayerAimState();
	void SetAimZ(float fAimZ);
	float GetAimZ();
	WEAPON_SLOT_TYPE * GetCurrentWeaponSlot();
	//CAMERA_AIM* GetCurrentAim();
	// ��������

	void ClearAllWeapons();
	// ��������
	void DestroyFollowPedTask();
	// ��������
	void ResetDamageEntity();

	void SetDead();
	void ExtinguishFire();
		
	// 0.3.7
	void RestartIfWastedAt(VECTOR *vecRestart, float fRotation);
	// 0.3.7
	void ForceTargetRotation(float fRotation);
	// 0.3.7
	uint8_t GetActionTrigger();
	// 0.3.7
	bool IsDead();
	uint16_t GetKeys(uint16_t *lrAnalog, uint16_t *udAnalog);
	void SetKeys(uint16_t wKeys, uint16_t lrAnalog, uint16_t udAnalog);
	// 0.3.7
	void SetMoney(int iAmount);
	// 0.3.7
	void ShowMarker(uint32_t iMarkerColorID);
	// 0.3.7
	void HideMarker();
	// 0.3.7
	void SetFightingStyle(int iStyle);
	// 0.3.7
	void SetRotation(float fRotation);
	// 0.3.7
	void ApplyAnimation( char *szAnimName, char *szAnimFile, float fT, int opt1, int opt2, int opt3, int opt4, int iUnk );
	// 0.3.7
	void GetBonePosition(int iBoneID, VECTOR* vecOut);
	// roflan
	unsigned char FindDeathReasonAndResponsiblePlayer(PLAYERID *nPlayer);
	void SetActionTrigger(uint8_t action);
	PED_TYPE * GetGtaActor() { return m_pPed; };

	void StartDancing(int iStyle);
	void StopDancing();
	bool IsDancing();

	void SetAmmo(unsigned char byteWeapon, unsigned short wordAmmo);
	WEAPON_SLOT_TYPE *FindWeaponSlot(uint32_t dwWeapon);

	void ApplyCrouch();
	void ResetCrouch();
	bool IsCrouching();

	void GetBonePosition(VECTOR &out, unsigned int boneid, bool update);
	bool SetPlayerAttachedObject(int index, int model, unsigned int bone, VECTOR offset, VECTOR rot);
	void DestroyPlayerAttachedObject(int index);
	uint8_t GetExtendedKeys();

	void DisableAutoAim();

	void ApplyCommandTask(char *szTaskName, int p1, int p2, int p3, VECTOR *p4, int p5, int p6, int p7, int p8, int p9);
	
public:
	PED_TYPE*	m_pPed;
	uint8_t		m_bytePlayerNumber;
	uint32_t	m_dwArrow;
	int 		m_iDancingState;
	int 		m_iDancingStyle;
	// Attached objects
	CObject*	m_pAttachedObjects[10];
	bool		m_bAOSlotState[10];
};