#include "../main.h"
#include "font.h"


// 53381C ; CFont::SetWrapx(float)
void CFont::SetWrapx(float fVal)
{
	return (( void (*)(float))(g_libGTASA+0x53381C+1))(fVal);
}

// 533834 ; CFont::SetCentreSize(float)
void CFont::SetCentreSize(float fVal)
{
	return (( void (*)(float))(g_libGTASA+0x533834+1))(fVal);
}

// 53384C ; CFont::SetRightJustifyWrap(float)
void CFont::SetRightJustifyWrap(float fVal)
{
	return (( void (*)(float))(g_libGTASA+0x53384C+1))(fVal);
}

// 533864 ; CFont::SetAlphaFade(float)
void CFont::SetAlphaFade(float fVal)
{
	return (( void (*)(float))(g_libGTASA+0x533864+1))(fVal);
}

// 53387C ; CFont::SetDropColor(CRGBA)
void CFont::SetDropColor(uint32_t* dwColor)
{
	return (( void (*)(uint32_t*))(g_libGTASA+0x53387C+1))(dwColor);
}

// 5338DC ; CFont::SetDropShadowPosition(signed char)
void CFont::SetDropShadowPosition(signed char iPos)
{
	return (( void (*)(signed char))(g_libGTASA+0x5338DC+1))(iPos);
}

// 53394C ; CFont::SetEdge(signed char)
void CFont::SetEdge(signed char iPos) // outline
{
	return (( void (*)(signed char))(g_libGTASA+0x53394C+1))(iPos);
}

// 5339A4 ; CFont::SetBackgroundColor(CRGBA)
void CFont::SetBackgroundColor(uint32_t* dwColor)
{
	return (( void (*)(uint32_t*))(g_libGTASA+0x5339A4+1))(dwColor);
}

// 5339D0 ; CFont::SetJustify(unsigned char)
void CFont::SetJustify(signed char iPos)
{
	return (( void (*)(signed char))(g_libGTASA+0x5339D0+1))(iPos);
}

void CFont::SetScale(float fScaleX, float fScaleY)
{
	*(float*)(g_libGTASA + 0x99D750) = fScaleX;
	*(float*)(g_libGTASA + 0x99D750+0x4) = fScaleY;
 	///return (( void (*)(float))(g_libGTASA+0x533694+1))(fScaleX/*, fScaleY*/);
}

void CFont::SetBackground(uint8_t unk0, uint8_t unk1)
{
	return (( void (*)(uint8_t, uint8_t))(g_libGTASA+0x533988+1))(unk0, unk1);
}
// 0x533970
void CFont::SetProportional(uint8_t iProp)
{
	return (( void (*)(uint8_t))(g_libGTASA+0x533970+1))(iProp);
}

void CFont::SetOrientation(uint8_t iOrient)
{
	return (( void (*)(uint8_t))(g_libGTASA+0x5339E8+1))(iOrient);
}

void CFont::SetColor(uint32_t* dwColor)
{
	return (( void (*)(uint32_t*))(g_libGTASA+0x5336F4+1))(dwColor);
}

void CFont::SetFontStyle(uint8_t iStyle)
{
	return (( void (*)(uint8_t))(g_libGTASA+0x533748+1))(iStyle);
}


void CFont::AsciiToGxtChar(const char* ascii, uint16_t* gxt)
{
	return (( void (*)(const char*, uint16_t*))(g_libGTASA+0x532D00+1))(ascii, gxt);
}

void CFont::PrintString(float posX, float posY, char *string )
{
	uint16_t *gxt_string = new uint16_t[0xFF];
	CFont::AsciiToGxtChar(string, gxt_string);
	(( void (*)(float, float, uint16_t*))(g_libGTASA+0x5353B4+1))(posX, posY, gxt_string);
	delete gxt_string;
	(( void (*)())(g_libGTASA+0x53411C+1))();//53411C ; _DWORD CFont::RenderFontBuffer(CFont *__hidden this)

}