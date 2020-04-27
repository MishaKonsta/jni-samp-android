#pragma once

#include "RW/RenderWare.h"
#include "gui/gui.h"
#include "rgba.h"

class CFont
{
public:
	//
	static void SetWrapx(float fVal);
	static void SetCentreSize(float fVal);
	static void SetRightJustifyWrap(float fVal);
	static void SetAlphaFade(float fVal);
	static void SetDropColor(uint32_t* dwColor);
	static void SetDropShadowPosition(signed char iPos);
	static void SetEdge(signed char iPos);
	static void SetBackgroundColor(uint32_t* dwColor);
	static void SetJustify(signed char iPos);
	//
	static void SetScale(float fScaleX, float fScaleY);
	static void SetBackground(uint8_t unk0, uint8_t unk1);
	static void SetProportional(uint8_t iProp);
	static void SetOrientation(uint8_t iOrient);
	static void SetColor(uint32_t* dwColor); /// lol
	static void SetFontStyle(uint8_t iStyle);
	static void AsciiToGxtChar(const char* ascii, uint16_t* gxt);
	static void PrintString(float posX, float posY, char* string );
};