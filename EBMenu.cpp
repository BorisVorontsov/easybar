#if(WINVER < 0x0500)
#define WINVER 0x0500
#endif

#include <windows.h>

#ifdef UNICODE
#define _UNICODE
#endif

#include <tchar.h>

#include "ebmenu.h"

#pragma comment (lib, "msimg32.lib")

CEBMenu::CEBMenu()
{
	HDC hDC = GetDC(0);
	crFontColorOne = RGB(255, 255, 255);
	crFontColorTwo = RGB(128, 128, 128);
	crFontColorThree = RGB(0, 0, 0);
	crBkColorOne = RGB(220, 220, 220);
	crBkColorTwo = RGB(192, 192, 192);
	crBkColorThree = RGB(200, 200, 200);
	crSelColorOne = RGB(255, 0, 0);
	crSelColorTwo = RGB(0, 0, 255);
	crBrColorOne = RGB(0, 0, 0);
	crBrColorTwo = RGB(255, 255, 255);
	crTrColor = RGB(255, 0, 255);
	hFntStandardItem = CreateFont(-MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72)
		, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Tahoma"));
	hFntDefaultItem = CreateFont(-MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72)
		, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Tahoma"));
	hBmpCheck = 0;
	hBmpRadioCheck = 0;
	m_hMenuOwner = 0;
	ReleaseDC(0, hDC);
}

CEBMenu::~CEBMenu()
{
	DeleteObject(hFntStandardItem);
	DeleteObject(hFntDefaultItem);
	DeleteObject(hBmpCheck);
	DeleteObject(hBmpRadioCheck);
}

BOOL CEBMenu::InitEBMenu(HWND hMenuOwner, BOOL bPopupMenu)
{
	if (hMenuOwner)
	{
		if (!CreateODMenu(GetMenu(hMenuOwner), !bPopupMenu)) return FALSE;
		m_hMenuOwner = hMenuOwner;
		m_bPopupMenu = bPopupMenu;
	}
	else
	{
		m_hMenuOwner = 0;
	}
	return TRUE;
}

DWORD CEBMenu::MeasureItem(WPARAM wParam, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pMIS = (LPMEASUREITEMSTRUCT)lParam;
	MENUITEMINFO MII = { 0 };
	MII.cbSize = sizeof(MII);
	MII.fMask = MIIM_TYPE;
	HMENU hMenu = GetMenu(m_hMenuOwner);

	GetMenuItemInfo(hMenu, pMIS->itemID, FALSE, &MII);

	if ((MII.fType & MFT_SEPARATOR) == MFT_SEPARATOR)
	{
		pMIS->itemWidth = 1;
		pMIS->itemHeight = EBM_METRICS_SEPARATOR_HEIGHT;
	}
	else if ((MII.fType & MFT_STRING) == MFT_STRING)
	{
		TCHAR lpText[MAX_PATH] = { 0 };
		TCHAR lpItem[MAX_PATH] = { 0 }, lpAccel[64] = { 0 };
		LONG lItemWidth, lAccelWidth;
		MII.fMask = MIIM_STRING;
		MII.dwTypeData = lpText;
		MII.cch = MAX_PATH;
		GetMenuItemInfo(hMenu, pMIS->itemID, FALSE, &MII);
		RECT RCI = { 0 }, RCA = { 0 };
		LPVOID pTabPos = _tcschr(lpText, '\t');
		if (pTabPos)
		{
			_tcsncpy(lpItem, lpText, ((LPTSTR)pTabPos - lpText));
			_tcscpy(lpAccel, (LPTSTR)pTabPos + 1);
		}
		else
		{
			_tcscpy(lpItem, lpText);
		}
		HDC hTDC = CreateCompatibleDC(0);
		HBITMAP hTBitmap = CreateCompatibleBitmap(hTDC, 512, 32);
		SelectObject(hTDC, hTBitmap);
		MII.fMask = MIIM_STATE;
		GetMenuItemInfo(hMenu, pMIS->itemID, FALSE, &MII);
		if ((MII.fState & MFS_DEFAULT) == MFS_DEFAULT)
		{
			SelectObject(hTDC, hFntDefaultItem);
		}
		else
		{
			SelectObject(hTDC, hFntStandardItem);
		}
		DrawText(hTDC, lpItem, (_tcschr(lpItem, '&'))?_tcslen(lpItem) - 1:-1, &RCI, DT_SINGLELINE | DT_CALCRECT);
		DrawText(hTDC, lpAccel, -1, &RCA, DT_SINGLELINE | DT_CALCRECT);
		lItemWidth = RCI.right;
		lAccelWidth = RCA.right;
		MII.fMask = MIIM_DATA;
		GetMenuItemInfo(hMenu, pMIS->itemID, FALSE, &MII);
		if ((MII.dwItemData & EBM_ITEM_DATA_TLMENU) == EBM_ITEM_DATA_TLMENU)
		{
			pMIS->itemWidth = EBM_METRICS_TLMENU_LEFT_TEXT_INDENT + lItemWidth +
				EBM_METRICS_TLMENU_RIGHT_TEXT_INDENT;
			pMIS->itemHeight = EBM_METRICS_STD_INDENT + EBM_METRICS_TLMENU_HEIGHT +
				EBM_METRICS_STD_INDENT;
		}
		else
		{
			pMIS->itemWidth = EBM_METRICS_STD_INDENT + EBM_METRICS_PA_WIDTH +
				EBM_METRICS_STD_INDENT + EBM_METRICS_LEFT_TEXT_INDENT + lItemWidth +
				((lAccelWidth)?(EBM_MI_IA_INDENT + lAccelWidth):0) + EBM_METRICS_RIGHT_TEXT_INDENT;
			pMIS->itemHeight = EBM_METRICS_STD_INDENT + EBM_METRICS_PA_HEIGHT +
				EBM_METRICS_STD_INDENT;
		}
		DeleteDC(hTDC);
		DeleteObject(hTBitmap);
	}
	return TRUE;
}

DWORD CEBMenu::DrawItem(WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
	TRIVERTEX TV[2] = { 0 };
	GRADIENT_RECT GR = { 0 };
	RECT RCB = { 0 }, RCP = { 0 };
	MENUITEMINFO MII = { 0 };
	BOOL bSeparator = FALSE;
	TCHAR lpText[MAX_PATH] = { 0 };
	MII.cbSize = sizeof(MII);
	MII.fMask = MIIM_TYPE;
	HMENU hMenu = GetMenu(m_hMenuOwner);

	GetMenuItemInfo(hMenu, pDIS->itemID, FALSE, &MII);
	if ((MII.fType & MFT_SEPARATOR) == MFT_SEPARATOR)
	{
		bSeparator = TRUE;
	}
	else if ((MII.fType & MFT_STRING) == MFT_STRING)
	{
		MII.fMask = MIIM_STRING;
		MII.dwTypeData = lpText;
		MII.cch = MAX_PATH;
		GetMenuItemInfo(hMenu, pDIS->itemID, FALSE, &MII);
	}
	MII.fMask = MIIM_DATA;
	GetMenuItemInfo(hMenu, pDIS->itemID, FALSE, &MII);

	if ((MII.dwItemData & EBM_ITEM_DATA_TLMENU) == EBM_ITEM_DATA_TLMENU)
	{
		HBRUSH hBk3Brush;
		CopyRect(&RCP, &pDIS->rcItem);
		hBk3Brush = CreateSolidBrush(crBkColorThree);
		FillRect(pDIS->hDC, &RCP, hBk3Brush);
		RCP.left += EBM_METRICS_STD_INDENT;
		RCP.right -= EBM_METRICS_STD_INDENT;
		DrawMenuString(pDIS->hDC, lpText, RCP, ((pDIS->itemState & ODS_GRAYED) ==
			ODS_GRAYED), FALSE, ((pDIS->itemState & ODS_DEFAULT) == ODS_DEFAULT), 
			((pDIS->itemState & ODS_INACTIVE) == ODS_INACTIVE), TRUE);
		DeleteObject(hBk3Brush);
	}
	else
	{
		HBRUSH hBk1Brush, hBk2Brush;
		HPEN hOldVSepPen, hVSepPen;
		hBk1Brush = CreateSolidBrush(crBkColorOne);
		hBk2Brush = CreateSolidBrush(crBkColorTwo);
		CopyRect(&RCP, &pDIS->rcItem);
		RCP.right = EBM_METRICS_STD_INDENT + EBM_METRICS_PA_WIDTH +
			EBM_METRICS_STD_INDENT;
		FillRect(pDIS->hDC, &RCP, hBk1Brush);
		hVSepPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));
		hOldVSepPen = (HPEN)SelectObject(pDIS->hDC, hVSepPen);
		MoveToEx(pDIS->hDC, RCP.right, RCP.top, NULL);
		LineTo(pDIS->hDC, RCP.right, RCP.bottom);
		SelectObject(pDIS->hDC, hOldVSepPen);
		DeleteObject(hVSepPen);
		CopyRect(&RCP, &pDIS->rcItem);
		RCP.left += EBM_METRICS_STD_INDENT + EBM_METRICS_PA_WIDTH +
			EBM_METRICS_STD_INDENT + 1;
		FillRect(pDIS->hDC, &RCP, hBk2Brush);
		if (!bSeparator)
		{
			if ((pDIS->itemState & ODS_CHECKED) == ODS_CHECKED)
			{
				MII.fMask = MIIM_TYPE;
				GetMenuItemInfo(hMenu, pDIS->itemID, FALSE, &MII);
				CopyRect(&RCP, &pDIS->rcItem);
				DrawCheckMark(pDIS->hDC, RCP.left + EBM_METRICS_STD_INDENT, RCP.top +
					EBM_METRICS_STD_INDENT, ((pDIS->itemState & ODS_GRAYED) == ODS_GRAYED),
					FALSE, ((MII.fType & MFT_RADIOCHECK) == MFT_RADIOCHECK));
			}
			CopyRect(&RCP, &pDIS->rcItem);
			RCP.left += EBM_METRICS_STD_INDENT + EBM_METRICS_PA_WIDTH +
				EBM_METRICS_STD_INDENT + EBM_METRICS_LEFT_TEXT_INDENT;
			RCP.right -= EBM_METRICS_RIGHT_TEXT_INDENT;
			DrawMenuString(pDIS->hDC, lpText, RCP, ((pDIS->itemState & ODS_GRAYED) ==
				ODS_GRAYED), FALSE, ((pDIS->itemState & ODS_DEFAULT) == ODS_DEFAULT),
				((pDIS->itemState & ODS_INACTIVE) == ODS_INACTIVE), FALSE);
		}
		else
		{
			HPEN hOldHSepPen, hHSepPen;
			hHSepPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));
			hOldHSepPen = (HPEN)SelectObject(pDIS->hDC, hHSepPen);
			MoveToEx(pDIS->hDC, EBM_METRICS_STD_INDENT + EBM_METRICS_PA_WIDTH +
				EBM_METRICS_STD_INDENT + EBM_METRICS_LEFT_TEXT_INDENT, RCP.top +
				EBM_METRICS_SEPARATOR_HEIGHT / 2, 0);
			LineTo(pDIS->hDC, RCP.right - EBM_METRICS_STD_INDENT, RCP.top +
				EBM_METRICS_SEPARATOR_HEIGHT / 2);
			SelectObject(pDIS->hDC, hOldHSepPen);
			DeleteObject(hHSepPen);
		}
		DeleteObject(hBk1Brush);
		DeleteObject(hBk2Brush);
	}

	if ((((pDIS->itemAction & ODA_DRAWENTIRE) == ODA_DRAWENTIRE) || ((pDIS->itemAction & ODA_SELECT) == ODA_SELECT)) &&
		(((pDIS->itemState & ODS_SELECTED) == ODS_SELECTED) || (((pDIS->itemState & ODS_HOTLIGHT) == ODS_HOTLIGHT) &&
		((pDIS->itemState & ODS_INACTIVE) != ODS_INACTIVE))))
	{
		if (!bSeparator)
		{
			CopyRect(&RCP, &pDIS->rcItem);
			//Заливаем часть без градиента
			//--------------------------------------------------------------------
			CopyRect(&RCB, &pDIS->rcItem);
			RCB.bottom -= (((RCP.bottom - RCP.top) / 3) * 2);
			if ((MII.dwItemData & EBM_ITEM_DATA_TLMENU) == EBM_ITEM_DATA_TLMENU)
			{
				RCB.left += EBM_METRICS_TLMENU_LEFT_SEL_INDENT;
				RCB.right -= EBM_METRICS_TLMENU_RIGHT_SEL_INDENT;
			}
			else
			{
				RCB.left += EBM_METRICS_LEFT_SEL_INDENT;
				RCB.right -= EBM_METRICS_RIGHT_SEL_INDENT;
			}
			if ((pDIS->itemState & ODS_GRAYED) != ODS_GRAYED)
			{
				HBRUSH hBkBrush = CreateSolidBrush(crSelColorOne);
				FillRect(pDIS->hDC, &RCB, hBkBrush);
				DeleteObject(hBkBrush);
			}
			else
			{
				FillRect(pDIS->hDC, &RCB, GetSysColorBrush(COLOR_MENU));
			}
			//--------------------------------------------------------------------
			if ((MII.dwItemData & EBM_ITEM_DATA_TLMENU) == EBM_ITEM_DATA_TLMENU)
			{
				TV[0].x = RCP.left + EBM_METRICS_TLMENU_LEFT_SEL_INDENT;
			}
			else
			{
				TV[0].x = RCP.left + EBM_METRICS_LEFT_SEL_INDENT;
			}
			TV[0].y = RCP.top + (RCB.bottom - RCB.top);
			if ((pDIS->itemState & ODS_GRAYED) != ODS_GRAYED)
			{
				TV[0].Red = GetRValue(crSelColorOne) << 8;
				TV[0].Green = GetGValue(crSelColorOne) << 8;
				TV[0].Blue = GetBValue(crSelColorOne) << 8;
			}
			else
			{
				TV[0].Red = GetRValue(GetSysColor(COLOR_MENU)) << 8;
				TV[0].Green = GetGValue(GetSysColor(COLOR_MENU)) << 8;
				TV[0].Blue = GetBValue(GetSysColor(COLOR_MENU)) << 8;
			}
			TV[0].Alpha = 0;
			if ((MII.dwItemData & EBM_ITEM_DATA_TLMENU) == EBM_ITEM_DATA_TLMENU)
			{
				TV[1].x = RCP.right - EBM_METRICS_TLMENU_RIGHT_SEL_INDENT;
			}
			else
			{
				TV[1].x = RCP.right - EBM_METRICS_RIGHT_SEL_INDENT;
			}
			TV[1].y = RCP.bottom;
			if ((pDIS->itemState & ODS_GRAYED) != ODS_GRAYED)
			{
				TV[1].Red = GetRValue(crSelColorTwo) << 8;
				TV[1].Green = GetGValue(crSelColorTwo) << 8;
				TV[1].Blue = GetBValue(crSelColorTwo) << 8;
			}
			else
			{
				TV[1].Red = GetRValue(GetSysColor(COLOR_MENU)) << 8;
				TV[1].Green = GetGValue(GetSysColor(COLOR_MENU)) << 8;
				TV[1].Blue = GetBValue(GetSysColor(COLOR_MENU)) << 8;
			}
			TV[1].Alpha = 0;
			GR.UpperLeft = 0;
			GR.LowerRight = 1;
			GradientFill(pDIS->hDC, &TV[0], 2, (PVOID)&GR, 1, GRADIENT_FILL_RECT_V);
			MII.fMask = MIIM_DATA;
			GetMenuItemInfo(hMenu, pDIS->itemID, FALSE, &MII);
			if ((MII.dwItemData & EBM_ITEM_DATA_TLMENU) == EBM_ITEM_DATA_TLMENU)
			{
				RCP.left += EBM_METRICS_STD_INDENT;
				RCP.right -= EBM_METRICS_STD_INDENT;
				DrawMenuString(pDIS->hDC, lpText, RCP, ((pDIS->itemState & ODS_GRAYED) ==
					ODS_GRAYED), TRUE, ((pDIS->itemState & ODS_DEFAULT) == ODS_DEFAULT),
					((pDIS->itemState & ODS_INACTIVE) == ODS_INACTIVE), TRUE);
			}
			else
			{
				if ((pDIS->itemState & ODS_CHECKED) == ODS_CHECKED)
				{
					MII.fMask = MIIM_TYPE;
					GetMenuItemInfo(hMenu, pDIS->itemID, FALSE, &MII);
					CopyRect(&RCP, &pDIS->rcItem);
					DrawCheckMark(pDIS->hDC, RCP.left + EBM_METRICS_STD_INDENT, RCP.top +
						EBM_METRICS_STD_INDENT, ((pDIS->itemState & ODS_GRAYED) == ODS_GRAYED),
						TRUE, ((MII.fType & MFT_RADIOCHECK) == MFT_RADIOCHECK));
				}
				CopyRect(&RCP, &pDIS->rcItem);
				RCP.left += EBM_METRICS_STD_INDENT + EBM_METRICS_PA_WIDTH +
					EBM_METRICS_STD_INDENT + EBM_METRICS_LEFT_TEXT_INDENT;
				RCP.right -= EBM_METRICS_RIGHT_TEXT_INDENT;
				DrawMenuString(pDIS->hDC, lpText, RCP, ((pDIS->itemState & ODS_GRAYED) ==
					ODS_GRAYED), TRUE, ((pDIS->itemState & ODS_DEFAULT) == ODS_DEFAULT),
					((pDIS->itemState & ODS_INACTIVE) == ODS_INACTIVE), FALSE);
			}
			CopyRect(&RCP, &pDIS->rcItem);
			if ((MII.dwItemData & EBM_ITEM_DATA_TLMENU) == EBM_ITEM_DATA_TLMENU)
			{
				RCP.left += EBM_METRICS_TLMENU_LEFT_SEL_INDENT;
				RCP.right -= EBM_METRICS_TLMENU_RIGHT_SEL_INDENT;
			}
			else
			{
				RCP.left += EBM_METRICS_LEFT_SEL_INDENT;
				RCP.right -= EBM_METRICS_RIGHT_SEL_INDENT;
			}
			if ((pDIS->itemState & ODS_GRAYED) != ODS_GRAYED)
			{
				HBRUSH hBr1Brush, hBr2Brush;
				hBr1Brush = CreateSolidBrush(crBrColorOne);
				FrameRect(pDIS->hDC, &RCP, hBr1Brush);
				hBr2Brush = CreateSolidBrush(crBrColorTwo);
				InflateRect(&RCP, -1, -1);
				FrameRect(pDIS->hDC, &RCP, hBr2Brush);
				InflateRect(&RCP, 1, 1);
				DeleteObject(hBr1Brush);
				DeleteObject(hBr2Brush);
			}
			else
			{
				FrameRect(pDIS->hDC, &RCP, GetSysColorBrush(COLOR_3DDKSHADOW));
				InflateRect(&RCP, -1, -1);
				FrameRect(pDIS->hDC, &RCP, GetSysColorBrush(COLOR_3DHIGHLIGHT));
				InflateRect(&RCP, 1, 1);
			}
			//--------------------------------------------------------------------
			if ((pDIS->itemState & ODS_GRAYED) != ODS_GRAYED)
			{
				HDC hLDC;
				HBRUSH hGlBrush;
				HBITMAP hTmpBitmap, hOldBitmap;
				RECT RCGE = { 0 };
				BLENDFUNCTION BF = { 0 };
				CopyRect(&RCGE, &pDIS->rcItem);
				RCGE.right -= RCGE.left;
				RCGE.bottom -= RCGE.top;
				RCGE.bottom >>= 1;
				RCGE.left = 0;
				RCGE.top = 0;
				hLDC = CreateCompatibleDC(pDIS->hDC);
				hTmpBitmap = CreateCompatibleBitmap(pDIS->hDC, RCGE.right, RCGE.bottom);
				hOldBitmap = (HBITMAP)SelectObject(hLDC, hTmpBitmap);
				hGlBrush = CreateSolidBrush(RGB(255, 255, 255));
				FillRect(hLDC, &RCGE, hGlBrush);
				DeleteObject(hGlBrush);
				BF.SourceConstantAlpha = 80;
				BF.BlendOp = AC_SRC_OVER;
				AlphaBlend(pDIS->hDC, RCP.left, RCP.top, RCGE.right, RCGE.bottom, hLDC, 0, 0, RCGE.right, RCGE.bottom, BF);
				SelectObject(hLDC, hOldBitmap);
				DeleteObject(hTmpBitmap);
				DeleteDC(hLDC);
			}
			//--------------------------------------------------------------------
		}
	}
	return TRUE;
}

HMENU CEBMenu::GetCurrentMenu()
{
	return GetMenu(m_hMenuOwner);
}

BOOL CEBMenu::UpdateMenuBar()
{
	if (m_bPopupMenu) return FALSE;

	MENUINFO MI = { 0 };
	MI.cbSize = sizeof(MI);
	MI.fMask = MIM_BACKGROUND;
	MI.hbrBack = CreateSolidBrush(crBkColorThree);
	return SetMenuInfo(GetMenu(m_hMenuOwner), &MI);
}

DWORD CEBMenu::CreateODMenu(HMENU hMenu, BOOL bTLMenu)
{
	MENUITEMINFO MII = { 0 };
	ULONG i, lODItemCnt = 0;

	MII.cbSize = sizeof(MII);
	int intItemCnt = GetMenuItemCount(hMenu);
	if (intItemCnt == -1) return lODItemCnt;

	for (i = 0; i < (ULONG)intItemCnt; i++)
	{
		MII.fMask = MIIM_TYPE;
		GetMenuItemInfo(hMenu, i, TRUE, &MII);
		MII.fType |= MFT_OWNERDRAW;
		SetMenuItemInfo(hMenu, i, TRUE, &MII);
		if (bTLMenu)
		{
			MII.fMask = MIIM_DATA;
			MII.dwItemData = EBM_ITEM_DATA_TLMENU;
			SetMenuItemInfo(hMenu, i, TRUE, &MII);
		}
		lODItemCnt++;
		MII.fMask = MIIM_SUBMENU;
		GetMenuItemInfo(hMenu, i, TRUE, &MII);
		if (MII.hSubMenu)
		{
			lODItemCnt += CreateODMenu(MII.hSubMenu);
		}
	}

	return lODItemCnt;
}

BOOL CEBMenu::DrawCheckMark(HDC hDC, LONG lX, LONG lY, BOOL bDisabled, BOOL bSelected, BOOL bRadioCheck)
{
	if (hBmpCheck)
	{
		HBITMAP hCheckMark;
		BITMAPINFO BMI = { 0 }, EBMI = { 0 };
		HDC hTDC = CreateCompatibleDC(hDC);
		if (bRadioCheck)
		{
			if (hBmpRadioCheck)
				hCheckMark = hBmpRadioCheck; else hCheckMark = hBmpCheck;
		}
		else
		{
			hCheckMark = hBmpCheck;
		}
		SelectObject(hTDC, hCheckMark);
		GetObject(hCheckMark, sizeof(BMI), &BMI);
		if (bDisabled || bSelected)
		{
			ULONG i, lR, lG, lB;
			LPBYTE pPixels;
			HDC hEDC = CreateCompatibleDC(hDC);
			HBITMAP hEBitmap = CreateCompatibleBitmap(hDC, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight);
			SelectObject(hEDC, hEBitmap);
			BitBlt(hEDC, 0, 0, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight, hTDC, 0, 0, SRCCOPY);
			EBMI.bmiHeader.biSize = sizeof(EBMI.bmiHeader);
			EBMI.bmiHeader.biWidth = BMI.bmiHeader.biWidth;
			EBMI.bmiHeader.biHeight = BMI.bmiHeader.biHeight;
			EBMI.bmiHeader.biPlanes = 1;
			EBMI.bmiHeader.biBitCount = 32;
			EBMI.bmiHeader.biCompression = BI_RGB;
			pPixels = new BYTE[EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight * 4];
			ZeroMemory(pPixels, EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight * 4);
			GetDIBits(hEDC, hCheckMark, 0, EBMI.bmiHeader.biHeight, pPixels, &EBMI, DIB_RGB_COLORS);
			for (i = 0; i < (ULONG)(EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight); i++)
			{
				lR = pPixels[2];
				lG = pPixels[1];
				lB = pPixels[0];
				if (RGB(lR, lG, lB) != crTrColor)
				{
					if (bDisabled)
					{
						pPixels[2] = (BYTE)GetRValue(GetSysColor(COLOR_GRAYTEXT));
						pPixels[1] = (BYTE)GetGValue(GetSysColor(COLOR_GRAYTEXT));
						pPixels[0] = (BYTE)GetBValue(GetSysColor(COLOR_GRAYTEXT));
					}
					else if (bSelected)
					{
						pPixels[2] = (BYTE)GetRValue(GetSysColor(COLOR_HIGHLIGHTTEXT));
						pPixels[1] = (BYTE)GetGValue(GetSysColor(COLOR_HIGHLIGHTTEXT));
						pPixels[0] = (BYTE)GetBValue(GetSysColor(COLOR_HIGHLIGHTTEXT));
					}
					else
					{
						pPixels[2] = (BYTE)GetRValue(GetSysColor(COLOR_MENUTEXT));
						pPixels[1] = (BYTE)GetGValue(GetSysColor(COLOR_MENUTEXT));
						pPixels[0] = (BYTE)GetBValue(GetSysColor(COLOR_MENUTEXT));
					}
				}
				pPixels += 4;
			}
			pPixels -= (EBMI.bmiHeader.biWidth * EBMI.bmiHeader.biHeight * 4);
			SetDIBitsToDevice(hEDC, 0, 0, EBMI.bmiHeader.biWidth, EBMI.bmiHeader.biHeight, 0, 
				0, 0, EBMI.bmiHeader.biHeight, pPixels, &EBMI, DIB_RGB_COLORS);
			TransparentBlt(hDC, lX, lY, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight,
				hEDC, 0, 0, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight, crTrColor);
			delete[] pPixels;
			DeleteDC(hEDC);
			DeleteObject(hEBitmap);
		}
		else
		{
			TransparentBlt(hDC, lX, lY, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight,
				hTDC, 0, 0, BMI.bmiHeader.biWidth, BMI.bmiHeader.biHeight, crTrColor);
		}
		DeleteDC(hTDC);
		return TRUE;
	}
	else return FALSE;
}

BOOL CEBMenu::DrawMenuString(HDC hDC, LPCTSTR lpString, RECT RCT, BOOL bDisabled, BOOL bSelected, BOOL bDefault, BOOL bInactive, BOOL bTLMenu)
{
	if (_tcslen(lpString))
	{
		HFONT hOldFont;
		TCHAR lpItem[MAX_PATH] = { 0 }, lpAccel[64] = { 0 };
		LPVOID pTabPos = (LPTSTR)_tcschr(lpString, '\t');
		if (pTabPos && !bTLMenu)
		{
			_tcsncpy(lpItem, lpString, ((LPTSTR)pTabPos - lpString));
			_tcscpy(lpAccel, (LPTSTR)pTabPos + 1);
		}
		else
		{
			_tcscpy(lpItem, lpString);
		}
		SetBkMode(hDC, TRANSPARENT);
		if (bDefault)
		{
			hOldFont = (HFONT)SelectObject(hDC, hFntDefaultItem);
		}
		else
		{
			hOldFont = (HFONT)SelectObject(hDC, hFntStandardItem);
		}
		if (bSelected)
		{
			if (!bInactive)
			{
				if (bDisabled)
				{
					SetTextColor(hDC, GetSysColor(COLOR_3DHIGHLIGHT));
				}
				else
				{
					SetTextColor(hDC, crFontColorTwo);
				}
				OffsetRect(&RCT, 1, 1);
				DrawText(hDC, lpItem, -1, &RCT, DT_VCENTER | ((bTLMenu)?DT_CENTER:DT_LEFT) | DT_SINGLELINE);
				if (_tcslen(lpAccel))
				{
					DrawText(hDC, lpAccel, -1, &RCT, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);
				}
				if (bDisabled)
				{
					SetTextColor(hDC, GetSysColor(COLOR_3DSHADOW));
				}
				else
				{
					SetTextColor(hDC, crFontColorOne);
				}
				OffsetRect(&RCT, -1, -1);
				DrawText(hDC, lpItem, -1, &RCT, DT_VCENTER | ((bTLMenu)?DT_CENTER:DT_LEFT) | DT_SINGLELINE);
				if (_tcslen(lpAccel))
				{
					DrawText(hDC, lpAccel, -1, &RCT, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);
				}
			}
			else
			{
				SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
				DrawText(hDC, lpItem, -1, &RCT, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			}
		}
		else
		{
			if (!bInactive)
			{
				if (bDisabled)
				{
					SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
					DrawText(hDC, lpItem, -1, &RCT, DT_VCENTER | ((bTLMenu)?DT_CENTER:DT_LEFT) | DT_SINGLELINE);
					if (_tcslen(lpAccel))
					{
						DrawText(hDC, lpAccel, -1, &RCT, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);
					}
				}
				else
				{
					SetTextColor(hDC, crFontColorThree);
					DrawText(hDC, lpItem, -1, &RCT, DT_VCENTER | ((bTLMenu)?DT_CENTER:DT_LEFT) | DT_SINGLELINE);
					if (_tcslen(lpAccel))
					{
						DrawText(hDC, lpAccel, -1, &RCT, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);
					}
				}
			}
			else
			{
				SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
				DrawText(hDC, lpItem, -1, &RCT, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			}
		}
		SelectObject(hDC, hOldFont);
		return TRUE;
	}
	else return FALSE;
}