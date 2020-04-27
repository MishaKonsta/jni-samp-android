#pragma once

#include "common.h"

#define PADDING(x,y) uint8_t x[y]

#define MAX_TEXT_DRAW_LINE 256

//-----------------------------------------------------------

typedef struct _TEXT_DRAW_DATA
{
	float fLetterWidth;		// -4
	float fLetterHeight;	// 0
	uint32_t dwLetterColor;	// 4
	uint8_t byteUnk12;			// 8
	uint8_t byteCentered;		// 9
	uint8_t byteBox;			// 10
	float fLineWidth;		// 12
	float fLineHeight;		// 16
	uint32_t dwBoxColor;		// 20
	uint8_t byteProportional;	// 24
	uint32_t dwBackgroundColor;// 25-29
	uint8_t byteShadow;		// 29
	uint8_t byteOutline;		// 30
	uint8_t byteAlignLeft;		// 31
	uint8_t byteAlignRight;	// 32
	uint32_t dwStyle;			// 36
	float fX;				// 40
	float fY;				// 44
	char cGXT[8];			// 48
	uint32_t dwParam1;			// 60-64
	uint32_t dwParam2;			// 64-68
    int iTextureSlot;
	uint8_t byteSelectable;
	uint16_t wModelID;
	float fRotX;
    float fRotY;
    float fRotZ;
	float fZoom;
	uint16_t wColor1;
	uint16_t wColor2;
	bool bHasRectArea;
} TEXT_DRAW_DATA;

//-----------------------------------------------------------

#pragma pack(1)
typedef struct _TEXT_DRAW_TRANSMIT
{
	union
	{
		unsigned char byteFlags;
		struct
		{
			unsigned char byteBox : 1;
			unsigned char byteLeft : 1;
			unsigned char byteRight : 1;
			unsigned char byteCenter : 1;
			unsigned char byteProportional : 1;
			unsigned char bytePadding : 3;
		};
	};
	float fLetterWidth;
	float fLetterHeight;
	unsigned long dwLetterColor;
	float fLineWidth;
	float fLineHeight;
	unsigned long dwBoxColor;
	unsigned char byteShadow;
	unsigned char byteOutline;
	unsigned long dwBackgroundColor;
	unsigned char byteStyle;
	unsigned char byteSelectable;
	float fX;
	float fY;
	unsigned short wModelID;
	float fRotX;
	float fRotY;
	float fRotZ;
	float fZoom;
	unsigned short wColor1;
	unsigned short wColor2;
} TEXT_DRAW_TRANSMIT;

class CTextDraw
{
private:

	char			m_szText[MAX_TEXT_DRAW_LINE];
	char			m_szString[MAX_TEXT_DRAW_LINE*4];

public:
	CTextDraw(TEXT_DRAW_TRANSMIT *TextDrawTransmit, char* szText);
	~CTextDraw(){};

	char* GetText() { return m_szText; };
	void SetText(char* szText);

	void Draw();

    TEXT_DRAW_DATA  m_TextDrawData;
    RECT m_rectArea;
	bool m_bHovered;
	uint32_t m_dwHoverColor;
};


//----------------------------------------------------
