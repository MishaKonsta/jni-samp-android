#include <iostream>
#include "../main.h"
#include "game.h"
#include "font.h"
#include "gui/gui.h"
#include "textdraw.h"

extern CGUI *pGUI;
extern CGame *pGame;

CTextDraw::CTextDraw(TEXT_DRAW_TRANSMIT *TextDrawTransmit, char* szText)
{
	Log("CTextDraw..");
	memset(&m_TextDrawData,0,sizeof(TEXT_DRAW_DATA));

	m_TextDrawData.fLetterWidth = TextDrawTransmit->fLetterWidth;
	m_TextDrawData.fLetterHeight = TextDrawTransmit->fLetterHeight;
	m_TextDrawData.dwLetterColor = TextDrawTransmit->dwLetterColor;
	m_TextDrawData.byteUnk12 = 0;
	m_TextDrawData.byteCentered = TextDrawTransmit->byteCenter;
	m_TextDrawData.byteBox = TextDrawTransmit->byteBox;
	m_TextDrawData.fLineWidth = TextDrawTransmit->fLineWidth;
	m_TextDrawData.fLineHeight = TextDrawTransmit->fLineHeight;
	m_TextDrawData.dwBoxColor = TextDrawTransmit->dwBoxColor;
	m_TextDrawData.byteProportional = TextDrawTransmit->byteProportional;
	m_TextDrawData.dwBackgroundColor = TextDrawTransmit->dwBackgroundColor;
	m_TextDrawData.byteShadow = TextDrawTransmit->byteShadow;
	m_TextDrawData.byteOutline = TextDrawTransmit->byteOutline;
	m_TextDrawData.byteAlignLeft = TextDrawTransmit->byteLeft;
	m_TextDrawData.byteAlignRight = TextDrawTransmit->byteRight;
	m_TextDrawData.dwStyle = TextDrawTransmit->byteStyle;
	m_TextDrawData.fX = TextDrawTransmit->fX;
	m_TextDrawData.fY = TextDrawTransmit->fY;
	m_TextDrawData.dwParam1 = 0xFFFFFFFF;
	m_TextDrawData.dwParam2 = 0xFFFFFFFF;
	m_TextDrawData.byteSelectable = TextDrawTransmit->byteSelectable;

	strncpy(m_szText, szText, MAX_TEXT_DRAW_LINE);
	m_szText[MAX_TEXT_DRAW_LINE - 1] = '\0';

	m_TextDrawData.bHasRectArea = false;
	m_rectArea.fLeft = 0.0f;
	m_rectArea.fRight = 0.0f;
	m_rectArea.fBottom = 0.0f;
	m_rectArea.fTop = 0.0f;
	m_bHovered = false;
	m_dwHoverColor = 0;
	Log("CTextDrawEnd..");
}

void CTextDraw::SetText(char* szText)
{
	Log("SetText..");
	strncpy(m_szText, szText, MAX_TEXT_DRAW_LINE);
	m_szText[MAX_TEXT_DRAW_LINE - 1] = '\0';
	Log("SetTextEnd..");
}

void CTextDraw::Draw()
{
	Log("Draw..");
    if(!m_szText || !strlen(m_szText)) return;
    if(m_TextDrawData.dwStyle > 3)
		return;
    strcpy(m_szString, m_szText);

    int iScreenWidth = RsGlobal->maximumWidth;
	int iScreenHeight = RsGlobal->maximumHeight;
	float fHorizHudScale = 0.0015625;
	float fVertHudScale = 0.00223214;

	float fScaleY = (float)iScreenHeight * fVertHudScale * m_TextDrawData.fLetterHeight * 0.5f;
	float fScaleX = (float)iScreenWidth * fHorizHudScale * m_TextDrawData.fLetterWidth;

	CFont::SetScale(fScaleX, fScaleY);
	
	if (m_bHovered)
	{
		CFont::SetColor( &m_dwHoverColor);
	}
	else
	{
		CFont::SetColor( &m_TextDrawData.dwLetterColor );
	}

	CFont::SetJustify(0);

	if (m_TextDrawData.byteAlignRight) CFont::SetOrientation(2);
	else if (m_TextDrawData.byteCentered) CFont::SetOrientation(0);
	else CFont::SetOrientation(1);

	float fLineWidth = iScreenWidth * m_TextDrawData.fLineWidth * fHorizHudScale;
	CFont::SetWrapx(fLineWidth);
	
	float fLineHeight = iScreenWidth * m_TextDrawData.fLineHeight * fHorizHudScale;
	CFont::SetCentreSize(fLineHeight);
	
	CFont::SetBackground(m_TextDrawData.byteBox, 0);
	
	CFont::SetBackgroundColor( &m_TextDrawData.dwBoxColor );
	CFont::SetProportional(m_TextDrawData.byteProportional);
	
	CFont::SetDropColor( &m_TextDrawData.dwBackgroundColor );

	if (m_TextDrawData.byteOutline) {
		CFont::SetEdge(m_TextDrawData.byteOutline);
	}
	else {
		CFont::SetDropShadowPosition(m_TextDrawData.byteShadow);
	}

	CFont::SetFontStyle(m_TextDrawData.dwStyle);

	float fUseY = iScreenHeight - ((448.0 - m_TextDrawData.fY) * (iScreenHeight * fVertHudScale));
	float fUseX = iScreenWidth - ((640.0 - m_TextDrawData.fX) * (iScreenWidth * fHorizHudScale));

	//533864 ; _DWORD __fastcall CFont::SetAlphaFade(CFont *__hidden this, float)
	//CFont::SetAlphaFade(256.0);

	CFont::PrintString(fUseX, fUseY, m_szString);
	
	CFont::SetEdge(0);

	if (m_TextDrawData.byteAlignRight)
	{
		m_rectArea.fLeft = fUseX - (fLineWidth - fUseX);
		m_rectArea.fRight = fUseX;
		m_rectArea.fBottom = fUseY;
		m_rectArea.fTop = fUseY + fLineHeight;
	}
	else if (m_TextDrawData.byteCentered)
	{
		m_rectArea.fLeft = fUseX - (fLineHeight * 0.5f);
		m_rectArea.fRight = m_rectArea.fLeft + fLineHeight;
		m_rectArea.fBottom = fUseY;
		m_rectArea.fTop = fUseY + fLineWidth;
	}
	else
	{
		m_rectArea.fLeft = fUseX;
		m_rectArea.fRight = fLineWidth - fUseX + fUseX;
		m_rectArea.fBottom = fUseY;
		m_rectArea.fTop = fUseY + fLineHeight;
	}

	m_TextDrawData.bHasRectArea = true;
	Log("DrawEnd..");
}