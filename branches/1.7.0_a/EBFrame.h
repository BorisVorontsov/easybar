#ifndef EBFRAME_H
#define EBFRAME_H

typedef struct _EBFPROPERTIES
{
	//-----------------------
	RECT CRC;
	HDC hDC;
	HDC hDBDC;
	HBITMAP hDBBitmap;
	HBITMAP hOldDBBitmap;
	//-----------------------
	COLORREF crBrColorOne;
	COLORREF crBrColorTwo;
	LONG lMode;
} EBFPROPERTIES, *LPEBFPROPERTIES;

#define EBFRAME_CLASS		TEXT("EBFRAME")

//Свойства EBFrame
//-----------------------------------------------------
#define EBFM_SETBRCOLORS	WM_USER + 0x0000086B	//wParam: COLORREF, lParam: COLORREF, return: 0
#define EBFM_GETBRCOLORS	WM_USER + 0x0000086C	//wParam: LPCOLORREF, lParam: LPCOLORREF, return: 0

#define EBFM_SETMODE		WM_USER + 0x0000086D	//wParam: 0, lParam: (LONG) Mode, return: 0
#define EBFM_GETMODE		WM_USER + 0x0000086E	//wParam: 0, lParam: 0, return: (LONG) Mode
//-----------------------------------------------------

//Внутренние методы EBFrame
//-----------------------------------------------------
#define EBFM_DRAWCONTROL	WM_USER + 0x00000200	//wParam: 0, lParam: 0, return: 0
//-----------------------------------------------------

#define EBF_MODE_SINGLELINE	0x00000000
#define EBF_MODE_NORMAL		0x00000001

BOOL InitEBFrame(HINSTANCE hInstance);
static LRESULT CALLBACK EBFrameProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif