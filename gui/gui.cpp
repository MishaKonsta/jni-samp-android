#include "main.h"
#include "gui.h"
#include "game/game.h"
#include "net/netgame.h"
#include "game/RW/RenderWare.h"
#include "chatwindow.h"
#include "spawnscreen.h"
#include "playertags.h"
#include "dialog.h"
#include "keyboard.h"
#include "debug.h"
#include "settings.h"
#include "scoreboard.h"
#include "consolegui.h"
#include "button.h"

extern CChatWindow *pChatWindow;
extern CSpawnScreen *pSpawnScreen;
extern CPlayerTags *pPlayerTags;
extern CDialogWindow *pDialogWindow;
extern CDebug *pDebug;
extern CSettings *pSettings;
extern CKeyBoard *pKeyBoard;
extern CNetGame *pNetGame;
extern CScoreBoard *pScoreBoard;
extern CConsoleGUI *pConsoleGUI;

/* imgui_impl_renderware.h */
void ImGui_ImplRenderWare_RenderDrawData(ImDrawData* draw_data);
bool ImGui_ImplRenderWare_Init();
void ImGui_ImplRenderWare_NewFrame();
void ImGui_ImplRenderWare_ShutDown();

/*
	Все координаты GUI-элементов задаются
	относительно разрешения 1920x1080
*/
#define MULT_X	0.00052083333f	// 1/1920
#define MULT_Y	0.00092592592f 	// 1/1080

CGUI::CGUI()
{
	Log("Initializing GUI..");

	// setup ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	ImGui_ImplRenderWare_Init();

	// scale
	m_vecScale.x = io.DisplaySize.x * MULT_X;
	m_vecScale.y = io.DisplaySize.y * MULT_Y;
	// font Size
	m_fFontSize = ScaleY( pSettings->Get().fFontSize );

	// mouse/touch
	m_bMousePressed = false;
	m_vecMousePos = ImVec2(0, 0);

	Log("GUI | Scale factor: %f, %f Font size: %f", m_vecScale.x, m_vecScale.y, m_fFontSize);

	// setup style
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScrollbarSize = ScaleY(35.0f);
	style.WindowBorderSize = 0.0f;
	ImGui::StyleColorsClassic();
	//ImGui::StyleColorsDark();

	// load fonts
	char path[0xFF];
	sprintf(path, "%sSAMP/fonts/%s", g_pszStorage, pSettings->Get().szFont);
	// cp1251 ranges
	static const ImWchar ranges[] = 
	{
		0x0020, 0x0080,
		0x00A0, 0x00C0,
		0x0400, 0x0460,
		0x0490, 0x04A0,
		0x2010, 0x2040,
		0x20A0, 0x20B0,
		0x2110, 0x2130,
		0
	};
	Log("GUI | Loading font: %s", pSettings->Get().szFont);
	m_pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
	Log("GUI | ImFont pointer = 0x%X", m_pFont);
}

CGUI::~CGUI()
{
	ImGui_ImplRenderWare_ShutDown();
	ImGui::DestroyContext();
}

void CGUI::Render()
{
	ImGuiIO& io = ImGui::GetIO();

	if(pNetGame && pNetGame->GetTextDrawPool())
	{
		pNetGame->GetTextDrawPool()->Draw();
	}

	ImGui_ImplRenderWare_NewFrame();
	ImGui::NewFrame();

	RenderVersion();
	//RenderRakNetStatistics();
	if(pPlayerTags) pPlayerTags->Render();
	
	if(pNetGame && pNetGame->GetLabelPool())
	{
		pNetGame->GetLabelPool()->Draw();
	}

	/*if(pNetGame && pNetGame->GetTextDrawPool())
	{
		pNetGame->GetTextDrawPool()->Draw();
	}*/
	
	if(pChatWindow) pChatWindow->Render();

	if(pDialogWindow) pDialogWindow->Render();

	if(pSpawnScreen) pSpawnScreen->Render();
	if(pKeyBoard) pKeyBoard->Render();

	if(pScoreBoard) pScoreBoard->Draw();
	
	if(pConsoleGUI) pConsoleGUI->Draw();

	ImVec2 vecButSize = ImVec2(ImGui::GetFontSize() * 3.5, ImGui::GetFontSize() * 2.5);

	ImGui::SetNextWindowPos(ImVec2(2.0f, io.DisplaySize.y/3-vecButSize.x/2));
  	ImGui::Begin("###keys", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::Button(m_bKeysStatus ? "<<" : ">>", vecButSize))
	{
		if(m_bKeysStatus)
			m_bKeysStatus = false;
		else
			m_bKeysStatus = true;
	}
	ImGui::SameLine();
	if(pNetGame)
	{
		CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
		if(pVehiclePool)
		{
			VEHICLEID ClosetVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();
			if(ClosetVehicleID < MAX_VEHICLES && pVehiclePool->GetSlotState(ClosetVehicleID))
			{
				CVehicle* pVehicle = pVehiclePool->GetAt(ClosetVehicleID);
				if(pVehicle)
				{
					if(pVehicle->GetDistanceFromLocalPlayerPed() < 4.0f)
					{
						if (ImGui::Button("G", vecButSize))
						{
							CPlayerPool *pPlayerPool;
							CLocalPlayer *pLocalPlayer;

							if(pNetGame)
							{
								pPlayerPool = pNetGame->GetPlayerPool();
								if(pPlayerPool)
								{
									pLocalPlayer = pPlayerPool->GetLocalPlayer();
									if(pLocalPlayer)
									{
										pLocalPlayer->HandlePassengerEntryEx();
									}
								}
							}
						}
						ImGui::SameLine();
					}
				}			
			}
		}
	}
	if (ImGui::Button("Enter", vecButSize))
	{
		//pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->
		LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;
	}
	if(m_bKeysStatus)
	{
		ImGui::SameLine();
		if (ImGui::Button("Alt", vecButSize))
			LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
		ImGui::SameLine();
		if (ImGui::Button("H", vecButSize))
			LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;
		ImGui::SameLine();
		if (ImGui::Button("Y", vecButSize))
			LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;
		ImGui::SameLine();
		if (ImGui::Button("N", vecButSize))
			LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
	}
  	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplRenderWare_RenderDrawData(ImGui::GetDrawData());

	if(m_bNeedClearMousePos)
	{
		io.MousePos = ImVec2(-1, -1);
		m_bNeedClearMousePos = false;
	}
}

bool CGUI::OnTouchEvent(int type, bool multi, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();

	if(pNetGame)
		pNetGame->GetTextDrawPool()->OnTouchEvent(type, multi, x, y);
	if(!pChatWindow->OnTouchEvent(type, multi, x, y))
	{
		return false;
	}
	if(!pKeyBoard->OnTouchEvent(type, multi, x, y))
	{
		return false;
	}
	if(!pScoreBoard->OnTouchEvent(type, multi, x, y)) return false;

	switch(type)
	{
		case TOUCH_PUSH:
		io.MousePos = ImVec2(x, y);
		io.MouseDown[0] = true;
		break;

		case TOUCH_POP:
		io.MouseDown[0] = false;
		m_bNeedClearMousePos = true;
		break;

		case TOUCH_MOVE:
		io.MousePos = ImVec2(x, y);
		break;
	}

	return true;
}

void CGUI::RenderVersion()
{
	ImVec2 verpos = ImVec2(ScaleX(10), ScaleY(10));
	RenderText(verpos, ImColor(0xa039b3FF), true, "SAMP-SERVERS.RU Beta 0.3.7");
}

void CGUI::RenderRakNetStatistics()
{
		//StatisticsToString(rss, message, 0);

		/*ImGui::GetOverlayDrawList()->AddText(
			ImVec2(ScaleX(10), ScaleY(400)),
			ImColor(IM_COL32_BLACK), message);*/
}

void CGUI::RenderText(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end)
{
	int iOffset = pSettings->Get().iFontOutline;

	if(bOutline)
	{
		posCur.x -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x += iOffset;
		// right 
		posCur.x += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x -= iOffset;
		// above
		posCur.y -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y += iOffset;
		// below
		posCur.y += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y -= iOffset;
	}

	ImGui::GetBackgroundDrawList()->AddText(posCur, col, text_begin, text_end);
}

void CGUI::RenderOverlayText(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end)
{
	int iOffset = pSettings->Get().iFontOutline;

	if(bOutline)
	{
		posCur.x -= iOffset;
		ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x += iOffset;
		// right 
		posCur.x += iOffset;
		ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x -= iOffset;
		// above
		posCur.y -= iOffset;
		ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y += iOffset;
		// below
		posCur.y += iOffset;
		ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y -= iOffset;
	}

	ImGui::GetOverlayDrawList()->AddText(posCur, col, text_begin, text_end);
}