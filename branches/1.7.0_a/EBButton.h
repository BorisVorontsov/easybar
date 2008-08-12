#ifndef EBBUTTON_H
#define EBBUTTON_H

typedef struct _EBBMOUSEFLAG
{
	DWORD dwFlag;
	DWORD dwReserved;
} EBBMOUSEFLAG, *LPEBBMOUSEFLAG;

#define EBBMF_DOWN			0x00000001

typedef struct _EBBPROPERTIES
{
	//-----------------------
	RECT CRC;
	EBBMOUSEFLAG EBBMF;
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
	COLORREF crTrColor;
	TCHAR lpCurrentText[MAX_PATH];
	HBITMAP hBitmap;
} EBBPROPERTIES, *LPEBBPROPERTIES;

#define EBBUTTON_CLASS		TEXT("EBBUTTON")

//Свойства EBButton
//-----------------------------------------------------
#define EBBM_SETFONT			WM_USER + 0x00000997	//wParam: HFONT, lParam: MAKELPARAM(BOOL, 0), return: 0
#define EBBM_GETFONT			WM_USER + 0x00000998	//wParam: 0, lParam: 0, return: HFONT

#define EBBM_SETFONTCOLORS		WM_USER + 0x00000999	//wParam: COLORREF, lParam: COLORREF, return: 0
#define EBBM_GETFONTCOLORS		WM_USER + 0x0000099A	//wParam: LPCOLORREF, lParam: LPCOLORREF, return: 0

#define EBBM_SETBKCOLORS		WM_USER + 0x0000099B	//wParam: COLORREF, lParam: COLORREF, return: 0
#define EBBM_GETBKCOLORS		WM_USER + 0x0000099C	//wParam: LPCOLORREF, lParam: LPCOLORREF, return: 0

#define EBBM_SETBRCOLORS		WM_USER + 0x0000099D	//wParam: COLORREF, lParam: COLORREF, return: 0
#define EBBM_GETBRCOLORS		WM_USER + 0x0000099E	//wParam: LPCOLORREF, lParam: LPCOLORREF, return: 0

#define EBBM_SETTRCOLOR			WM_USER + 0x0000099F	//wParam: 0, lParam: (COLORREF), return: 0
#define EBBM_GETTRCOLOR			WM_USER + 0x000009A0	//wParam: 0, lParam: 0, return: (COLORREF)

#define EBBM_SETBITMAP			WM_USER + 0x000009A1	//wParam: 0, lParam: (HBITMAP), return: 0
#define EBBM_GETBITMAP			WM_USER + 0x000009A2	//wParam: 0, lParam: 0, return: (HBITMAP)
//-----------------------------------------------------

//События EBButton
//-----------------------------------------------------
#define EBBN_CLICKED			WM_USER + 0x000009C4	//WM_COMMAND, HIWORD(lParam)
//-----------------------------------------------------

//Внутренние методы EBButton
//-----------------------------------------------------
#define EBBM_DRAWCONTROL		WM_USER + 0x00000200	//wParam: 0, lParam: 0, return: 0
//-----------------------------------------------------

BOOL InitEBButton(HINSTANCE hInstance);
static LRESULT CALLBACK EBButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif