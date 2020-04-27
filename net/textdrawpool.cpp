#include "../main.h"
#include "textdrawpool.h"
#include "../game/game.h"
#include "netgame.h"
extern CGame* pGame;
extern CNetGame* pNetGame;

CTextDrawPool::CTextDrawPool()
{
    m_bSelectState = false;
	m_dwHoverColor = 0;
	m_wClickedTextDrawID = 0xFFFF;

    for(int i = 0; i < MAX_TEXT_DRAWS; i++)
    {
        m_pTextDraw[i] = 0;
        m_bSlotState[i] = false;
    }
}

CTextDrawPool::~CTextDrawPool()
{
    for(int i = 0; i < MAX_TEXT_DRAWS; i++)
    {
        if(!m_pTextDraw[i]) continue;
        delete m_pTextDraw[i];
        m_pTextDraw[i] = 0;
        m_bSlotState[i] = false;
    }
}

CTextDraw *CTextDrawPool::New(unsigned short wText, TEXT_DRAW_TRANSMIT *TextDrawTransmit, char *szText)
{
    if(m_pTextDraw[wText]) Delete(wText);

    CTextDraw *pTextDraw = new CTextDraw(TextDrawTransmit, szText);

    if(pTextDraw)
    {
        m_pTextDraw[wText] = pTextDraw;
        m_bSlotState[wText] = true;
        return pTextDraw;
    }
    return 0;
}

void CTextDrawPool::Delete(unsigned short wText)
{
    if(m_pTextDraw[wText] == 0) return;
    delete m_pTextDraw[wText];
    m_pTextDraw[wText] = 0;
    m_bSlotState[wText] = false;
}

void CTextDrawPool::Draw()
{
    for(int i = 0; i < MAX_TEXT_DRAWS; i++)
    {
        if(m_bSlotState[i]) m_pTextDraw[i]->Draw();
    }
}

void CTextDrawPool::SetSelectState(bool bState, uint32_t dwColor)
{
	if (bState) 
    {
		m_bSelectState = true;
		m_dwHoverColor = dwColor;
        m_wClickedTextDrawID = 0xFFFF;
		pGame->DisplayHUD(false);
		pGame->FindPlayerPed()->TogglePlayerControllable(false);
	}
	else {
		m_bSelectState = false;
		m_dwHoverColor = 0;
		m_wClickedTextDrawID = 0xFFFF;
		pGame->DisplayHUD(true);
		pGame->FindPlayerPed()->TogglePlayerControllable(true);

		SendClick();

		for (int i = 0; i < MAX_TEXT_DRAWS; i++)
		{
			if (m_bSlotState[i] && m_pTextDraw[i]) {
				CTextDraw* pTextDraw = m_pTextDraw[i];
				pTextDraw->m_bHovered = false;
				pTextDraw->m_dwHoverColor = 0;
			}
		}
	}
}

void CTextDrawPool::SendClick()
{
	RakNet::BitStream bsClick;
	bsClick.Write(m_wClickedTextDrawID);
	pNetGame->GetRakClient()->RPC(&RPC_ClickTextDraw, &bsClick, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
}

bool CTextDrawPool::OnTouchEvent(int type, bool multi, int x, int y)
{
	if (m_bSelectState == false) return true;

	m_wClickedTextDrawID = 0xFFFF;


	for (int i = 0; i < MAX_TEXT_DRAWS; i++)
	{
		if (m_bSlotState[i] && m_pTextDraw[i])
		{
			CTextDraw* pTextDraw = m_pTextDraw[i];
			pTextDraw->m_bHovered = false;
			pTextDraw->m_dwHoverColor = 0;

			if (pTextDraw->m_TextDrawData.byteSelectable &&	pTextDraw->m_TextDrawData.bHasRectArea)
			{
				RECT* rect = &pTextDraw->m_rectArea;
				if (rect)
				{
					if (IsPointInRect(x, y, rect))
					{
						switch (type)
						{
						case TOUCH_PUSH:
						case TOUCH_MOVE:
							m_wClickedTextDrawID = 0xFFFF;
							pTextDraw->m_bHovered = true;
							pTextDraw->m_dwHoverColor = m_dwHoverColor;
							break;

						case TOUCH_POP:
							m_wClickedTextDrawID = i;
							SendClick();
							pTextDraw->m_bHovered = false;
							pTextDraw->m_dwHoverColor = 0;
							break;
						}
					}
				}
			}
		}
	}

	return false;
}