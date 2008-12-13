#ifndef EBDISPLAY_H
#define EBDISPLAY_H

typedef struct _EBDPROPERTIES
{
	//-----------------------
	RECT CRC;
	NMHDR NM;
	HDC hDC;
	HDC hDBDC;
	HBITMAP hDBBitmap;
	HBITMAP hOldDBBitmap;
	//-----------------------
	HFONT hFont;
	COLORREF crFontColorOne;
	COLORREF crFontColorTwo;
	COLORREF crBkColorOne;
	COLORREF crBkColorTwo;
	COLORREF crBrColorOne;
	COLORREF crBrColorTwo;
	TCHAR lpCurrentText[MAX_PATH];
} EBDPROPERTIES, *LPEBDPROPERTIES;

#define EBDISPLAY_CLASS		TEXT("EBDISPLAY")

//Свойства EBDisplay
//-----------------------------------------------------
#define EBDM_SETFONT			WM_USER + 0x00000613	//wParam: HFONT, lParam: MAKELPARAM(BOOL, 0), return: 0
#define EBDM_GETFONT			WM_USER + 0x00000614	//wParam: 0, lParam: 0, return: HFONT

#define EBDM_SETFONTCOLORS		WM_USER + 0x00000615	//wParam: COLORREF, lParam: COLORREF, return: 0
#define EBDM_GETFONTCOLORS		WM_USER + 0x00000616	//wParam: LPCOLORREF, lParam: LPCOLORREF, return: 0

#define EBDM_SETBKCOLORS		WM_USER + 0x00000617	//wParam: COLORREF, lParam: COLORREF, return: 0
#define EBDM_GETBKCOLORS		WM_USER + 0x00000618	//wParam: LPCOLORREF, lParam: LPCOLORREF, return: 0

#define EBDM_SETBRCOLORS		WM_USER + 0x00000619	//wParam: COLORREF, lParam: COLORREF, return: 0
#define EBDM_GETBRCOLORS		WM_USER + 0x00000620	//wParam: LPCOLORREF, lParam: LPCOLORREF, return: 0
//-----------------------------------------------------

//События EBDisplay
//-----------------------------------------------------
#define EBDN_CLICKED			WM_USER + 0x00000708	//WM_NOTIFY, (lParam)pNMHDR->code
#define EBDN_DBLCLK				WM_USER + 0x00000709	//WM_NOTIFY, (lParam)pNMHDR->code
//-----------------------------------------------------

//Внутренние методы EBDisplay
//-----------------------------------------------------
#define EBDM_DRAWCONTROL		WM_USER + 0x00000200	//wParam: 0, lParam: 0, return: 0
//-----------------------------------------------------

BOOL InitEBDisplay(HINSTANCE hInstance);
static LRESULT CALLBACK EBDisplayProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif