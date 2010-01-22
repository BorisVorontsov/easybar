#ifndef EBSLIDER_H
#define EBSLIDER_H

typedef struct _EBSPROPERTIES
{
	//-----------------------
	RECT CRC;
	NMHDR NM;
	HDC hDC;
	HDC hDBDC;
	HBITMAP hDBBitmap;
	HBITMAP hOldDBBitmap;
	//-----------------------
	LONG lMin;
	LONG lMax;
	LONG lPos;
	COLORREF crLineColorOne;
	COLORREF crLineColorTwo;
	COLORREF crBkColor;
	COLORREF crBrColorOne;
	COLORREF crBrColorTwo;
	LONG lMode;
} EBSPROPERTIES, *LPEBSPROPERTIES;

#define EBSLIDER_CLASS		TEXT("EBSLIDER")

//Свойства EBSlider
//-----------------------------------------------------
#define EBSM_SETRANGE		WM_USER + 0x0000073F	//wParam: (LONG) Min, lParam: (LONG) Max, return: 0
#define EBSM_GETRANGE		WM_USER + 0x00000740	//wParam: (PLONG) Min, lParam: (PLONG) Max, return: 0

#define EBSM_SETPOS			WM_USER + 0x00000741	//wParam: (BOOL) Notify, lParam: (LONG) Pos, return: 0
#define EBSM_GETPOS			WM_USER + 0x00000742	//wParam: 0, lParam: 0, return: Pos

#define EBSM_SETLINECOLORS	WM_USER + 0x00000743	//wParam: (COLORREF) First, lParam: (COLORREF) Second, return: 0
#define EBSM_GETLINECOLORS	WM_USER + 0x00000744	//wParam: (LPCOLORREF) First, lParam: (LPCOLORREF) Second, return: 0

#define EBSM_SETBKCOLOR		WM_USER + 0x00000745	//wParam: 0, lParam: (COLORREF) Color, return: 0
#define EBSM_GETBKCOLOR		WM_USER + 0x00000746	//wParam: 0, lParam: 0, return: (COLORREF) Color

#define EBSM_SETBRCOLORS	WM_USER + 0x00000747	//wParam: (COLORREF) First, lParam: (COLORREF) Second, return: 0
#define EBSM_GETBRCOLORS	WM_USER + 0x00000748	//wParam: (LPCOLORREF) First, lParam: (LPCOLORREF) Second, return: 0

#define EBSM_SETMODE		WM_USER + 0x00000749	//wParam: 0, lParam: (LONG) Mode, return: 0
#define EBSM_GETMODE		WM_USER + 0x00000750	//wParam: 0, lParam: 0, return: (LONG) Mode
//-----------------------------------------------------

//События EBSlider
//-----------------------------------------------------
#define EBSN_SCROLL			WM_USER + 0x00000747	//WM_NOTIFY, (lParam)pNMHDR->code
#define EBSN_CHANGE			WM_USER + 0x00000748	//WM_NOTIFY, (lParam)pNMHDR->code
#define EBSN_CHANGE_SP		WM_USER + 0x00000749	//WM_NOTIFY, (lParam)pNMHDR->code
//-----------------------------------------------------

//Внутренние методы EBSlider
//-----------------------------------------------------
#define EBSM_DRAWCONTROL	WM_USER + 0x00000200	//wParam: 0, lParam: 0, return: 0
//-----------------------------------------------------

#define EBS_MODE_HORIZONTAL	0x00000000
#define EBS_MODE_VERTICAL	0x00000001

BOOL InitEBSlider(HINSTANCE hInstance);
static LRESULT CALLBACK EBSliderProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif