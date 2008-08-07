#ifndef LABELEX_H
#define LABELEX_H

typedef struct _LEMOUSEFLAG
{
	DWORD dwFlag;
	DWORD dwReserved;
} LEMOUSEFLAG, *LPLEMOUSEFLAG;

#define LEMF_LEAVE			0x00000000
#define LEMF_HOVER			0x00000001
#define LEMF_DOWN			0x00000002

typedef struct _LEPROPERTIES
{
	//-----------------------
	RECT CRC;
	LEMOUSEFLAG LEMF;
	NMHDR NM;
	HDC hDC;
	HDC hDBDC;
	HBITMAP hDBBitmap;
	HBITMAP hOldDBBitmap;
	//-----------------------
	HFONT hNormalFont;
	HFONT hHotFont;
	COLORREF crNormalFontColor;
	COLORREF crHotFontColor;
	COLORREF crBkColor;
	BOOL bAutoSize;
	TCHAR lpCurrentText[MAX_PATH];
} LEPROPERTIES, *LPLEPROPERTIES;

#define LABELEX_CLASS		TEXT("LABELEX")

//Свойства LabelEx
//-----------------------------------------------------
#define LEM_SETNFONT		WM_USER + 0x00000613	//wParam: HFONT, lParam: MAKELPARAM(BOOL, 0), return: 0
#define LEM_GETNFONT		WM_USER + 0x00000614	//wParam: 0, lParam: 0, return: HFONT
#define LEM_SETHFONT		WM_USER + 0x00000615	//wParam: HFONT, lParam: MAKELPARAM(BOOL, 0), return: 0
#define LEM_GETHFONT		WM_USER + 0x00000616	//wParam: 0, lParam: 0, return: HFONT

#define LEM_SETNFONTCOLOR	WM_USER + 0x00000617	//wParam: COLORREF, lParam: 0, return: 0
#define LEM_GETNFONTCOLOR	WM_USER + 0x00000618	//wParam: 0, lParam: 0, return: COLORREF
#define LEM_SETHFONTCOLOR	WM_USER + 0x00000619	//wParam: COLORREF, lParam: 0, return: 0
#define LEM_GETHFONTCOLOR	WM_USER + 0x0000061A	//wParam: 0, lParam: 0, return: COLORREF

#define LEM_SETBKCOLOR		WM_USER + 0x0000061B	//wParam: COLORREF, lParam: 0, return: 0
#define LEM_GETBKCOLOR		WM_USER + 0x0000061C	//wParam: 0, lParam: 0, return: COLORREF

#define LEM_SETAUTOSIZE		WM_USER + 0x0000061D	//wParam: 0, lParam: MAKELPARAM(BOOL, 0), return 0
#define LEM_GETAUTOSIZE		WM_USER + 0x0000061E	//wParam: 0, lParam: 0, return: BOOL
//-----------------------------------------------------

//События LabelEx
//-----------------------------------------------------
#define LEN_CLICKED			WM_USER + 0x00000708	//WM_NOTIFY, (lParam)pNMHDR->code
//-----------------------------------------------------

//Внутренние методы LabelEx
//-----------------------------------------------------
#define LEM_DRAWCONTROL		WM_USER + 0x00000200	//wParam: 0, lParam: 0, return: 0
//-----------------------------------------------------

BOOL InitLabelEx(HINSTANCE hInstance);
static LRESULT CALLBACK LabelExProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif