//Обертка над стандартным э.у. ListBox
//Превращает ListBox в функциональный список, пригодный для отображения списка воспроизведения

#include <windows.h>

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include <tchar.h>

#include "eblistbox.h"

#pragma comment (lib, "msimg32.lib")

CEBListBox::CEBListBox()
{
	HDC hDC = GetDC(NULL);
	crFontColorOne = RGB(255, 255, 255);
	crFontColorTwo = RGB(128, 128, 128);
	crFontColorThree = RGB(0, 0, 0);
	crFontColorFour = RGB(92, 92, 92);
	crBkColorOne = RGB(220, 220, 220);
	crBkColorTwo = RGB(192, 192, 192);
	crBkColorThree = RGB(205, 205, 205);
	crSelColorOne = RGB(255, 0, 0);
	crSelColorTwo = RGB(0, 0, 255);
	crBrColorOne = RGB(0, 0, 0);
	crBrColorTwo = RGB(255, 255, 255);
	crTrColor = RGB(255, 0, 255);
	hFntStandardItem = CreateFont(-MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72)
		, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Tahoma"));
	hFntHLItem = CreateFont(-MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72)
		, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Tahoma"));
	hBmpMark = NULL;
	m_lOldLBProc = 0;
	m_hListBox = NULL;
	ReleaseDC(NULL, hDC);

	hLPDC = NULL;
	hLPIMDC = NULL;
	hLPAMDC = NULL;
	hLPDMDC = NULL;
	hNI1DC = NULL;
	hNI2DC = NULL;
	hBDC = NULL;
	hSIDC = NULL;

	hLPBmp = NULL; hOldLPBmp = NULL;
	hLPIMBmp = NULL; hOldLPIMBmp = NULL;
	hLPAMBmp = NULL; hOldLPAMBmp = NULL;
	hLPDMBmp = NULL; hOldLPDMBmp = NULL;
	hNI1Bmp = NULL; hOldNI1Bmp = NULL;
	hNI2Bmp = NULL; hOldNI2Bmp = NULL;
	hBBmp = NULL; hOldBBmp = NULL;
	hSIBmp = NULL; hOldSIBmp = NULL;
}

CEBListBox::~CEBListBox()
{
	if (m_hListBox)
		InitEBListBox(NULL);
	DeleteObject(hFntStandardItem);
	DeleteObject(hFntHLItem);
	DeleteObject(hBmpMark);

	DeleteObject(SelectObject(hLPDC, hOldLPBmp));
	DeleteDC(hLPDC);
	DeleteObject(SelectObject(hLPIMDC, hOldLPIMBmp));
	DeleteDC(hLPIMDC);
	DeleteObject(SelectObject(hLPAMDC, hOldLPAMBmp));
	DeleteDC(hLPAMDC);
	DeleteObject(SelectObject(hLPDMDC, hOldLPDMBmp));
	DeleteDC(hLPDMDC);
	DeleteObject(SelectObject(hNI1DC, hOldNI1Bmp));
	DeleteDC(hNI1DC);
	DeleteObject(SelectObject(hNI2DC, hOldNI2Bmp));
	DeleteDC(hNI2DC);
	DeleteObject(SelectObject(hBDC, hOldBBmp));
	DeleteDC(hBDC);
	DeleteObject(SelectObject(hSIDC, hOldSIBmp));
	DeleteDC(hSIDC);
}

BOOL CEBListBox::InitEBListBox(HWND hListBox)
{
	if (hListBox)
	{
		if ((GetWindowLongPtr(hListBox, GWL_STYLE) & LBS_OWNERDRAWFIXED) != LBS_OWNERDRAWFIXED)
			return FALSE;
		int intItemsCnt = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
		if (intItemsCnt != LB_ERR)
		{
			LPLBITEMDESC pLBID;
			for (UINT i = 0; i < (UINT)intItemsCnt; i++)
			{
				pLBID = new LBITEMDESC;
				ZeroMemory(pLBID, sizeof(LBITEMDESC));
				SendMessage(hListBox, LB_SETITEMDATA, i, (LPARAM)pLBID);
			}
		}
		SetProp(hListBox, _T("_this_"), (HANDLE)this);
		m_lOldLBProc = SetWindowLongPtr(hListBox, GWL_WNDPROC, (LONG_PTR)ListBoxProc);
		m_hListBox = hListBox;
		Refresh();
	}
	else
	{
		if (!m_hListBox) return FALSE;
		int intItemsCnt = SendMessage(m_hListBox, LB_GETCOUNT, 0, 0);
		if (intItemsCnt != LB_ERR)
		{
			LPLBITEMDESC pLBID;
			for (UINT i = 0; i < (UINT)intItemsCnt; i++)
			{
				pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, i, 0);
				if ((int)pLBID != LB_ERR && pLBID)
					delete pLBID;
			}
		}
		SetWindowLongPtr(m_hListBox, GWL_WNDPROC, m_lOldLBProc);
		m_lOldLBProc = 0;
		RemoveProp(m_hListBox, _T("_this_"));
		m_hListBox = NULL;
	}
	return TRUE;
}

void CEBListBox::Refresh()
{
	RECT RCLB = { 0 }, RCP = { 0 }, RCB = { 0 };

	GetClientRect(m_hListBox, &RCLB);
	if (hLPDC)
	{
		DeleteObject(SelectObject(hLPDC, hOldLPBmp));
		DeleteDC(hLPDC);
	}
	if (hLPIMDC)
	{
		DeleteObject(SelectObject(hLPIMDC, hOldLPIMBmp));
		DeleteDC(hLPIMDC);
	}
	if (hLPAMDC)
	{
		DeleteObject(SelectObject(hLPAMDC, hOldLPAMBmp));
		DeleteDC(hLPAMDC);
	}
	if (hLPDMDC)
	{
		DeleteObject(SelectObject(hLPDMDC, hOldLPDMBmp));
		DeleteDC(hLPDMDC);
	}
	if (hNI1DC)
	{
		DeleteObject(SelectObject(hNI1DC, hOldNI1Bmp));
		DeleteDC(hNI1DC);
	}
	if (hNI2DC)
	{
		DeleteObject(SelectObject(hNI2DC, hOldNI2Bmp));
		DeleteDC(hNI2DC);
	}
	if (hBDC)
	{
		DeleteObject(SelectObject(hBDC, hOldBBmp));
		DeleteDC(hBDC);
	}
	if (hSIDC)
	{
		DeleteObject(SelectObject(hSIDC, hOldSIBmp));
		DeleteDC(hSIDC);
	}

	HDC hLBDC = GetDC(m_hListBox);

	SetRect(&RCP, 0, 0, EBLB_METRICS_STD_INDENT + EBLB_METRICS_PA_WIDTH +
		EBLB_METRICS_STD_INDENT + 1, EBLB_METRICS_STD_HEIGHT);

	hLPDC = CreateCompatibleDC(hLBDC);
	hLPBmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldLPBmp = (HBITMAP)SelectObject(hLPDC, hLPBmp);

	//Левая панель
	//----------------------------------------------------------------
	{
		HPEN hOldVSepPen, hVSepPen;
		HBRUSH hBkBrush = CreateSolidBrush(crBkColorOne);
		
		hVSepPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));
		hOldVSepPen = (HPEN)SelectObject(hLPDC, hVSepPen);

		FillRect(hLPDC, &RCP, hBkBrush);
		
		MoveToEx(hLPDC, RCP.right - 1, RCP.top, NULL);
		LineTo(hLPDC, RCP.right - 1, RCP.bottom);
		SelectObject(hLPDC, hOldVSepPen);
		DeleteObject(hVSepPen);
		DeleteObject(hBkBrush);
	}
	//---------------------------------------------------------------

	hLPIMDC = CreateCompatibleDC(hLBDC);
	hLPIMBmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldLPIMBmp = (HBITMAP)SelectObject(hLPIMDC, hLPIMBmp);

	//Левая панель + неактивная пометка
	//----------------------------------------------------------------
	{
		BitBlt(hLPIMDC, RCP.left, RCP.top, RCP.right, RCP.bottom, hLPDC, RCP.left, RCP.top, SRCCOPY);
		DrawMark(hLPIMDC, RCP.left + EBLB_METRICS_STD_INDENT, RCP.top + EBLB_METRICS_STD_INDENT, FALSE, FALSE);
	}
	//----------------------------------------------------------------

	hLPAMDC = CreateCompatibleDC(hLBDC);
	hLPAMBmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldLPAMBmp = (HBITMAP)SelectObject(hLPAMDC, hLPAMBmp);

	//Левая панель + активная пометка
	//----------------------------------------------------------------
	{
		BitBlt(hLPAMDC, RCP.left, RCP.top, RCP.right, RCP.bottom, hLPDC, RCP.left, RCP.top, SRCCOPY);
		DrawMark(hLPAMDC, RCP.left + EBLB_METRICS_STD_INDENT, RCP.top + EBLB_METRICS_STD_INDENT, FALSE, TRUE);
	}
	//----------------------------------------------------------------

	hLPDMDC = CreateCompatibleDC(hLBDC);
	hLPDMBmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldLPDMBmp = (HBITMAP)SelectObject(hLPDMDC, hLPDMBmp);

	//Левая панель + недоступная пометка
	//----------------------------------------------------------------
	{
		BitBlt(hLPDMDC, RCP.left, RCP.top, RCP.right, RCP.bottom, hLPDC, RCP.left, RCP.top, SRCCOPY);
		DrawMark(hLPDMDC, RCP.left + EBLB_METRICS_STD_INDENT, RCP.top + EBLB_METRICS_STD_INDENT, TRUE, FALSE);
	}
	//----------------------------------------------------------------

	SetRect(&RCP, 0, 0, RCLB.right - (EBLB_METRICS_STD_INDENT + EBLB_METRICS_PA_WIDTH +
		EBLB_METRICS_STD_INDENT + 1), EBLB_METRICS_STD_HEIGHT);

	hNI1DC = CreateCompatibleDC(hLBDC);
	hNI1Bmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldNI1Bmp = (HBITMAP)SelectObject(hNI1DC, hNI1Bmp);

	//Обычный пункт (цвет 1)
	//----------------------------------------------------------------
	{
		HBRUSH hBkBrush = CreateSolidBrush(crBkColorTwo);
		FillRect(hNI1DC, &RCP, hBkBrush);
		DeleteObject(hBkBrush);
	}
	//----------------------------------------------------------------
	
	hNI2DC = CreateCompatibleDC(hLBDC);
	hNI2Bmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldNI2Bmp = (HBITMAP)SelectObject(hNI2DC, hNI2Bmp);

	//Обычный пункт (цвет 2)
	//----------------------------------------------------------------
	{
		HBRUSH hBkBrush = CreateSolidBrush(crBkColorThree);
		FillRect(hNI2DC, &RCP, hBkBrush);
		DeleteObject(hBkBrush);
	}
	//----------------------------------------------------------------

	hBDC = CreateCompatibleDC(hLBDC);
	hBBmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldBBmp = (HBITMAP)SelectObject(hBDC, hBBmp);

	//Фон
	//----------------------------------------------------------------
	{
		FillRect(hBDC, &RCP, GetSysColorBrush(COLOR_WINDOW));
	}
	//----------------------------------------------------------------

	SetRect(&RCP, 0, 0, RCLB.right - (EBLB_METRICS_LEFT_SEL_INDENT + EBLB_METRICS_RIGHT_SEL_INDENT),
		EBLB_METRICS_STD_HEIGHT);

	hSIDC = CreateCompatibleDC(hLBDC);
	hSIBmp = CreateCompatibleBitmap(hLBDC, RCP.right, RCP.bottom);
	hOldSIBmp = (HBITMAP)SelectObject(hSIDC, hSIBmp);

	//Выделенный пункт
	//----------------------------------------------------------------
	{
		TRIVERTEX TV[2] = { 0 };
		GRADIENT_RECT GR = { 0 };
		
		//Заливаем часть без градиента
		//--------------------------------------------------------------------
		CopyRect(&RCB, &RCP);
		RCB.bottom -= (((RCP.bottom - RCP.top) / 3) * 2);

		HBRUSH hBkBrush = CreateSolidBrush(crSelColorOne);
		FillRect(hSIDC, &RCB, hBkBrush);
		DeleteObject(hBkBrush);
		//--------------------------------------------------------------------
		TV[0].x = RCP.left;
		TV[0].y = RCP.top + (RCB.bottom - RCB.top);
		TV[0].Red = GetRValue(crSelColorOne) << 8;
		TV[0].Green = GetGValue(crSelColorOne) << 8;
		TV[0].Blue = GetBValue(crSelColorOne) << 8;
		TV[0].Alpha = 0;

		TV[1].x = RCP.right;
		TV[1].y = RCP.bottom;
		TV[1].Red = GetRValue(crSelColorTwo) << 8;
		TV[1].Green = GetGValue(crSelColorTwo) << 8;
		TV[1].Blue = GetBValue(crSelColorTwo) << 8;
		TV[1].Alpha = 0;
		GR.UpperLeft = 0;
		GR.LowerRight = 1;
		GradientFill(hSIDC, &TV[0], 2, (PVOID)&GR, 1, GRADIENT_FILL_RECT_V);
	}
	//----------------------------------------------------------------

	ReleaseDC(m_hListBox, hLBDC);
	RedrawWindow(m_hListBox, 0, 0, RDW_ERASE | RDW_INVALIDATE);
}

LRESULT CALLBACK CEBListBox::ListBoxProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CEBListBox *pThis = (CEBListBox*)GetProp(hWnd, _T("_this_"));
	switch (uMsg)
	{
		case WM_ERASEBKGND:
		{
			int intItemsCnt;
			RECT RCLB = { 0 }, RCI = { 0 };
			BITMAP BMLP = { 0 }, BMB = { 0 };
			
			GetClientRect(hWnd, &RCLB);
			intItemsCnt = SendMessage(hWnd, LB_GETCOUNT, 0, 0);

			if ((intItemsCnt != LB_ERR) && intItemsCnt)
				SendMessage(hWnd, LB_GETITEMRECT, (intItemsCnt - 1), (LPARAM)&RCI);

			GetObject(pThis->hLPBmp, sizeof(BITMAP), &BMLP);
			GetObject(pThis->hBBmp, sizeof(BITMAP), &BMB);

			if (RCI.bottom < RCLB.bottom)
			{
				for (UINT i = RCI.bottom; i < (UINT)RCLB.bottom; i += BMLP.bmHeight)
				{
					BitBlt((HDC)wParam, 0, i, BMLP.bmWidth, BMLP.bmHeight, pThis->hLPDC, 0, 0, SRCCOPY);
					BitBlt((HDC)wParam, BMLP.bmWidth, i, BMB.bmWidth, BMB.bmHeight, pThis->hBDC, 0, 0, SRCCOPY);
				}
			}
			return 1;
		}
		case WM_PAINT:
		{
			int intItemsCnt = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
			if ((intItemsCnt == LB_ERR) || !intItemsCnt)
			{
				PAINTSTRUCT PS = { 0 };
				HDC hDC = BeginPaint(hWnd, &PS);
				//
				EndPaint(hWnd, &PS);
				return 0;
			}
			else break;
		}
		case WM_WINDOWPOSCHANGED:
			pThis->Refresh();
			break;
		case LB_DELETESTRING:
		{
			LPLBITEMDESC pLBID = (LPLBITEMDESC)SendMessage(hWnd, LB_GETITEMDATA,
				wParam, 0);
			if ((int)pLBID != LB_ERR && pLBID)
				delete pLBID;
			break;
		}
		case LB_RESETCONTENT:
		{
			int intItemsCnt = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
			if ((intItemsCnt != LB_ERR) && intItemsCnt)
			{
				LPLBITEMDESC pLBID;
				for (UINT i = 0; i < (UINT)intItemsCnt; i++)
				{
					pLBID = (LPLBITEMDESC)SendMessage(hWnd, LB_GETITEMDATA, i, 0);
					if ((int)pLBID != LB_ERR && pLBID)
						delete pLBID;
				}
			}
			break;
		}
		default:
			//
			break;
	}
	return CallWindowProc((WNDPROC)pThis->GetOldListBoxProc(), hWnd, uMsg, wParam, lParam);
}

DWORD CEBListBox::MeasureItem(WPARAM wParam, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pMIS = (LPMEASUREITEMSTRUCT)lParam;
	if (pMIS->CtlType != ODT_LISTBOX) return FALSE;
	
	RECT RCT = { 0 };
	HDC hTDC = CreateCompatibleDC(NULL);
	HBITMAP hTBitmap = CreateCompatibleBitmap(hTDC, 512, 32);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hTDC, hTBitmap);
	HFONT hOldFont = (HFONT)SelectObject(hTDC, hFntStandardItem);
	DrawText(hTDC, _T("A"), -1, &RCT, DT_SINGLELINE | DT_CALCRECT);
	if (RCT.bottom > (int)EBLB_METRICS_STD_HEIGHT)
	{
		pMIS->itemHeight = RCT.bottom;
		EBLB_METRICS_STD_HEIGHT = RCT.bottom;
	}
	else
	{
		pMIS->itemHeight = EBLB_METRICS_STD_HEIGHT;
	}
	SelectObject(hTDC, hOldFont);
	SelectObject(hTDC, hOldBitmap);
	DeleteObject(hTBitmap);
	DeleteDC(hTDC);
	return TRUE;
}

DWORD CEBListBox::DrawItem(WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
	if (pDIS->CtlType != ODT_LISTBOX) return FALSE;

	LPLBITEMDESC pLBID;
	TCHAR lpText[MAX_PATH + 64] = { 0 };
	RECT RCP = { 0 }, RCT = { 0 };
	BITMAP BMLP = { 0 }, BMNI = { 0 }, BMSI = { 0 };
	HDC hTmpDC;
	HBITMAP hTmpBmp, hOldBmp;

	hTmpDC = CreateCompatibleDC(pDIS->hDC);
	hTmpBmp = CreateCompatibleBitmap(pDIS->hDC, pDIS->rcItem.right - pDIS->rcItem.left,
		pDIS->rcItem.bottom - pDIS->rcItem.top);
	hOldBmp = (HBITMAP)SelectObject(hTmpDC, hTmpBmp);

	//Получаем информацию о пункте
	pLBID = (LPLBITEMDESC)SendMessage(pDIS->hwndItem, LB_GETITEMDATA, pDIS->itemID, 0);
	if (((int)pLBID == LB_ERR) || !pLBID)
	{
		pLBID = new LBITEMDESC;
		ZeroMemory(pLBID, sizeof(LBITEMDESC));
		SendMessage(pDIS->hwndItem, LB_SETITEMDATA, pDIS->itemID, (LPARAM)pLBID);
	}

	SendMessage(pDIS->hwndItem, LB_GETTEXT, pDIS->itemID, (LPARAM)lpText);

	//Левая панель
	//-------------------------------------------------------------------------
	GetObject(hLPBmp, sizeof(BITMAP), &BMLP);
	if ((pLBID->dwStyle & LBIS_HIGHLIGHTED) == LBIS_HIGHLIGHTED)
	{
		if (((pDIS->itemState & ODS_SELECTED) == ODS_SELECTED) || ((pDIS->itemState & ODS_FOCUS) == ODS_FOCUS))
		{
			//GetObject(hLPAMBmp, sizeof(BITMAP), &BMx);
			BitBlt(hTmpDC, 0, 0, BMLP.bmWidth, BMLP.bmHeight, hLPAMDC, 0, 0, SRCCOPY);
		}
		else if (((pDIS->itemState & ODS_DISABLED) == ODS_DISABLED))
		{
			//GetObject(hLPDMBmp, sizeof(BITMAP), &BMx);
			BitBlt(hTmpDC, 0, 0, BMLP.bmWidth, BMLP.bmHeight, hLPDMDC, 0, 0, SRCCOPY);
		}
		else
		{
			//GetObject(hLPIMBmp, sizeof(BITMIP), &BMx);
			BitBlt(hTmpDC, 0, 0, BMLP.bmWidth, BMLP.bmHeight, hLPIMDC, 0, 0, SRCCOPY);
		}
	}
	else
	{
		//GetObject(hLPBmp, sizeof(BITMAP), &BMx);
		BitBlt(hTmpDC, 0, 0, BMLP.bmWidth, BMLP.bmHeight, hLPDC, 0, 0, SRCCOPY);
	}
	//-------------------------------------------------------------------------
	
	//Обычный пункт
	//-------------------------------------------------------------------------
	GetObject(hNI1Bmp, sizeof(BITMAP), &BMNI);

	if ((pDIS->itemID == LB_ERR) || (((pDIS->itemAction & ODA_DRAWENTIRE) == ODA_DRAWENTIRE) || ((pDIS->itemAction &
		ODA_SELECT) == ODA_SELECT)))
	{
		if (pDIS->itemID == LB_ERR)
		{
			//GetObject(hBBmp, sizeof(BITMAP), &BMx);
			BitBlt(hTmpDC, BMLP.bmWidth, 0, BMNI.bmWidth, BMNI.bmHeight, hBDC, 0, 0, SRCCOPY);
		}
		else
		{
			if ((pDIS->itemID % 2) == 0)
			{
				//GetObject(hNI1Bmp, sizeof(BITMAP), &BMx);
				BitBlt(hTmpDC, BMLP.bmWidth, 0, BMNI.bmWidth, BMNI.bmHeight, hNI1DC, 0, 0, SRCCOPY);
			}
			else
			{
				//GetObject(hNI2Bmp, sizeof(BITMAP), &BMx);
				BitBlt(hTmpDC, BMLP.bmWidth, 0, BMNI.bmWidth, BMNI.bmHeight, hNI2DC, 0, 0, SRCCOPY);
			}
		}

		SetRect(&RCT, /*pDIS->rcItem.left + */EBLB_METRICS_LEFT_SEL_INDENT + EBLB_METRICS_LEFT_TEXT_INDENT,
			0, pDIS->rcItem.right - (EBLB_METRICS_RIGHT_SEL_INDENT + EBLB_METRICS_RIGHT_TEXT_INDENT), BMNI.bmHeight);

		DrawLBString(hTmpDC, lpText, RCT, ((pDIS->itemState & ODS_DISABLED) == ODS_DISABLED), FALSE,
			((pLBID->dwStyle & LBIS_HIGHLIGHTED) == LBIS_HIGHLIGHTED));
	}
	//-------------------------------------------------------------------------

	//Выделенный пункт
	//-------------------------------------------------------------------------
	if ((((pDIS->itemAction & ODA_DRAWENTIRE) == ODA_DRAWENTIRE) || ((pDIS->itemAction & ODA_SELECT) == ODA_SELECT)) &&
		((pDIS->itemState & ODS_SELECTED) == ODS_SELECTED))
	{
		GetObject(hSIBmp, sizeof(BITMAP), &BMSI);

		BitBlt(hTmpDC, EBLB_METRICS_LEFT_SEL_INDENT, 0, BMSI.bmWidth,
			BMSI.bmHeight, hSIDC, 0, 0, SRCCOPY);

		SetRect(&RCT, /*pDIS->rcItem.left + */EBLB_METRICS_LEFT_SEL_INDENT + EBLB_METRICS_LEFT_TEXT_INDENT,
			0, pDIS->rcItem.right - (EBLB_METRICS_RIGHT_SEL_INDENT + EBLB_METRICS_RIGHT_TEXT_INDENT), BMNI.bmHeight);

		DrawLBString(hTmpDC, lpText, RCT, ((pDIS->itemState & ODS_DISABLED) == ODS_DISABLED), TRUE,
			((pLBID->dwStyle & LBIS_HIGHLIGHTED) == LBIS_HIGHLIGHTED));

		SetRect(&RCP, EBLB_METRICS_LEFT_SEL_INDENT, 0, EBLB_METRICS_LEFT_SEL_INDENT + BMSI.bmWidth, BMSI.bmHeight);

		HBRUSH hBr1Brush, hBr2Brush;
		hBr1Brush = CreateSolidBrush(crBrColorOne);
		FrameRect(hTmpDC, &RCP, hBr1Brush);
		hBr2Brush = CreateSolidBrush(crBrColorTwo);
		InflateRect(&RCP, -1, -1);
		FrameRect(hTmpDC, &RCP, hBr2Brush);
		InflateRect(&RCP, 1, 1);
		DeleteObject(hBr1Brush);
		DeleteObject(hBr2Brush);

		//--------------------------------------------------------------------
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
		AlphaBlend(hTmpDC, RCP.left, RCP.top, RCGE.right, RCGE.bottom, hLDC, 0, 0, RCGE.right, RCGE.bottom, BF);
		SelectObject(hLDC, hOldBitmap);
		DeleteObject(hTmpBitmap);
		DeleteDC(hLDC);
		//--------------------------------------------------------------------
	}
	//-------------------------------------------------------------------------

	if ((pDIS->itemID == LB_ERR) || ((pDIS->itemAction & ODA_FOCUS) != ODA_FOCUS))
	{
		BitBlt(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, pDIS->rcItem.right - pDIS->rcItem.left,
			pDIS->rcItem.bottom - pDIS->rcItem.top, hTmpDC, 0, 0, SRCCOPY);
	}

	SelectObject(hTmpDC, hOldBmp);
	DeleteObject(hTmpBmp);
	DeleteDC(hTmpDC);

	return TRUE;
}

BOOL CEBListBox::HighlightItem(UINT uItemIndex)
{
	int intItemsCnt = SendMessage(m_hListBox, LB_GETCOUNT, 0, 0);
	if ((intItemsCnt == LB_ERR) || !intItemsCnt) return FALSE;
	//if (uItemIndex > (intItemsCnt - 1)) return FALSE;
	LPLBITEMDESC pLBID;
	for (UINT i = 0; i < (UINT)intItemsCnt; i++)
	{
		pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, i, 0);
		if (((int)pLBID == LB_ERR) || !pLBID)
		{
			pLBID = new LBITEMDESC;
			ZeroMemory(pLBID, sizeof(LBITEMDESC));
			SendMessage(m_hListBox, LB_SETITEMDATA, i, (LPARAM)pLBID);
		}
		if (i == uItemIndex)
		{
			if ((pLBID->dwStyle & LBIS_HIGHLIGHTED) != LBIS_HIGHLIGHTED)
				pLBID->dwStyle |= LBIS_HIGHLIGHTED;
		}
		else
		{
			if ((pLBID->dwStyle & LBIS_HIGHLIGHTED) == LBIS_HIGHLIGHTED)
				pLBID->dwStyle ^= LBIS_HIGHLIGHTED;
		}
	}
	RedrawWindow(m_hListBox, 0, 0, RDW_ERASE | RDW_INVALIDATE);
	return TRUE;
}

BOOL CEBListBox::IsItemHighlighted(UINT uItemIndex)
{
	//int intItemsCnt = SendMessage(m_hListBox, LB_GETCOUNT, 0, 0);
	//if ((intItemsCnt == LB_ERR) || !intItemsCnt) return FALSE;
	//if (uItemIndex > (intItemsCnt - 1)) return FALSE;
	LPLBITEMDESC pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, uItemIndex, 0);
	if (((int)pLBID == LB_ERR) || !pLBID) return FALSE;
	return ((pLBID->dwStyle & LBIS_HIGHLIGHTED) == LBIS_HIGHLIGHTED);
}

UINT CEBListBox::GetHighlightedItemIndex()
{
	int intItemsCnt = SendMessage(m_hListBox, LB_GETCOUNT, 0, 0);
	if ((intItemsCnt == LB_ERR) || !intItemsCnt) return -1;
	LPLBITEMDESC pLBID;
	for (UINT i = 0; i < (UINT)intItemsCnt; i++)
	{
		pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, i, 0);
		if (((int)pLBID == LB_ERR) || !pLBID) return -1;
		if ((pLBID->dwStyle & LBIS_HIGHLIGHTED) == LBIS_HIGHLIGHTED)
			return i;
	}
	return -1;
}

BOOL CEBListBox::SetItemTag(UINT uItemIndex, LPCBYTE pTag, SIZE_T szTagSize)
{
	//int intItemsCnt = SendMessage(m_hListBox, LB_GETCOUNT, 0, 0);
	//if ((intItemsCnt == LB_ERR) || !intItemsCnt) return FALSE;
	//if (uItemIndex > (intItemsCnt - 1)) return FALSE;
	LPLBITEMDESC pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, uItemIndex, 0);
	if (((int)pLBID == LB_ERR) || !pLBID)
	{
		pLBID = new LBITEMDESC;
		ZeroMemory(pLBID, sizeof(LBITEMDESC));
		SendMessage(m_hListBox, LB_SETITEMDATA, uItemIndex, (LPARAM)pLBID);
	}
	if (pLBID->pTag != NULL) delete[] pLBID->pTag;
	pLBID->pTag = new BYTE[szTagSize];
	CopyMemory(pLBID->pTag, pTag, szTagSize);
	pLBID->szTagSize = szTagSize;
	return TRUE;
}

SIZE_T CEBListBox::GetItemTagSize(UINT uItemIndex)
{
	LPLBITEMDESC pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, uItemIndex, 0);
	if (((int)pLBID == LB_ERR) || !pLBID) return -1;
	return pLBID->szTagSize;
}

BOOL CEBListBox::GetItemTag(UINT uItemIndex, LPBYTE pTag)
{
	LPLBITEMDESC pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, uItemIndex, 0);
	if (((int)pLBID == LB_ERR) || !pLBID) return FALSE;
	if (pLBID->pTag == NULL) return FALSE;
	CopyMemory(pTag, pLBID->pTag, pLBID->szTagSize);
	return TRUE;
}

BOOL CEBListBox::DeleteItemTag(UINT uItemIndex)
{
	LPLBITEMDESC pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, uItemIndex, 0);
	if (((int)pLBID == LB_ERR) || !pLBID) return FALSE;
	if (pLBID->pTag != NULL)
	{
		delete[] pLBID->pTag;
		pLBID->pTag = NULL;
	}
	return TRUE;
}

BOOL CEBListBox::Sort(BOOL bReverse)
{
	int intItemsCnt = SendMessage(m_hListBox, LB_GETCOUNT, 0, 0);
	if ((intItemsCnt == LB_ERR) || !intItemsCnt) return FALSE;
	UINT i, j, uTextSize;
	LPWSTR *lpwItems = new LPWSTR[intItemsCnt];

	struct ItemData
	{
		LPLBITEMDESC pLBID;
		LPWSTR lpwItem;
	};
	ItemData **pIDArr = new ItemData*[intItemsCnt];

	LPLBITEMDESC pLBID;
	for (i = 0; i < (UINT)intItemsCnt; i++)
	{
		uTextSize = SendMessage(m_hListBox, LB_GETTEXTLEN, i, 0);
		lpwItems[i] = new WCHAR[uTextSize + 1];
		ZeroMemory(lpwItems[i], (uTextSize + 1) * sizeof(WCHAR));
		SendMessage(m_hListBox, LB_GETTEXT, i, (LPARAM)lpwItems[i]);
		pIDArr[i] = new ItemData;
		pIDArr[i]->pLBID = NULL;
		pIDArr[i]->lpwItem = lpwItems[i];
		pLBID = (LPLBITEMDESC)SendMessage(m_hListBox, LB_GETITEMDATA, i, 0);
		if (((int)pLBID != LB_ERR) && pLBID)
		{
			pIDArr[i]->pLBID = new LBITEMDESC;
			ZeroMemory(pIDArr[i]->pLBID, sizeof(LBITEMDESC));
			pIDArr[i]->pLBID->dwStyle = pLBID->dwStyle;
			if (pLBID->pTag)
			{
				pIDArr[i]->pLBID->pTag = new BYTE[pLBID->szTagSize];
				CopyMemory(pIDArr[i]->pLBID->pTag, pLBID->pTag, pLBID->szTagSize);
				pIDArr[i]->pLBID->szTagSize = pLBID->szTagSize;
			}
		}
		else pIDArr[i]->pLBID = NULL;
	}

	SendMessage(m_hListBox, LB_RESETCONTENT, 0, 0);
	qsort(lpwItems, intItemsCnt, sizeof(LPWSTR), (bReverse)?CompareReverse:Compare);
	
	for (i = 0; i < (UINT)intItemsCnt; i++)
	{
		SendMessage(m_hListBox, LB_ADDSTRING, 0, (LPARAM)lpwItems[i]);
		for (j = 0; j < (UINT)intItemsCnt; j++)
		{
			if (!pIDArr[j] || !pIDArr[j]->pLBID) continue;
			if (lpwItems[i] == pIDArr[j]->lpwItem)
			{
				SendMessage(m_hListBox, LB_SETITEMDATA, i, (LPARAM)pIDArr[j]->pLBID);
				delete pIDArr[j];
				pIDArr[j] = NULL;
				break;
			}
		}
		delete[] lpwItems[i];
	}
	delete[] lpwItems;
	delete[] pIDArr;

	Refresh();
	return TRUE;
}

int CEBListBox::Compare(LPCVOID pArg1, LPCVOID pArg2)
{
	return _wcsicmp(*((LPCWSTR*)pArg1), *((LPCWSTR*)pArg2));
}

int CEBListBox::CompareReverse(LPCVOID pArg1, LPCVOID pArg2)
{
	return -(_wcsicmp(*((LPCWSTR*)pArg1), *((LPCWSTR*)pArg2)));
}

HWND CEBListBox::GetCurrentListBox()
{
	return m_hListBox;
}

LONG_PTR CEBListBox::GetOldListBoxProc()
{
	return m_lOldLBProc;
}

BOOL CEBListBox::DrawMark(HDC hDC, LONG lX, LONG lY, BOOL bDisabled, BOOL bSelected)
{
	if (hBmpMark)
	{
		BITMAPINFO BMI = { 0 }, EBMI = { 0 };
		HDC hTDC = CreateCompatibleDC(hDC);
		SelectObject(hTDC, hBmpMark);
		GetObject(hBmpMark, sizeof(BMI), &BMI);
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
			GetDIBits(hEDC, hBmpMark, 0, EBMI.bmiHeader.biHeight, pPixels, &EBMI, DIB_RGB_COLORS);
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

BOOL CEBListBox::DrawLBString(HDC hDC, LPCTSTR lpString, RECT RCT,
                      BOOL bDisabled, BOOL bSelected, BOOL bHighlighted)
{
	if (_tcslen(lpString))
	{
		HFONT hOldFont;
		TCHAR lpCol1[MAX_PATH] = { 0 }, lpCol2[64] = { 0 };
		SIZE_T szCol1Size, szCol2Size;
		RECT RCC1(RCT), RCC2(RCT);
		LPVOID pTabPos = (LPTSTR)_tcschr(lpString, '\t');
		if (pTabPos)
		{
			_tcsncpy(lpCol1, lpString, ((LPTSTR)pTabPos - lpString));
			_tcscpy(lpCol2, (LPTSTR)pTabPos + 1);
		}
		else
		{
			_tcscpy(lpCol1, lpString);
		}
		szCol1Size = _tcslen(lpCol1);
		szCol2Size = _tcslen(lpCol2);
		if (szCol2Size)
		{
			RCC1.right -= EBLB_METRICS_COL2_WIDTH;
			RCC2.left = RCC1.right;
		}
		SetBkMode(hDC, TRANSPARENT);
		if (bHighlighted)
		{
			hOldFont = (HFONT)SelectObject(hDC, hFntHLItem);
		}
		else
		{
			hOldFont = (HFONT)SelectObject(hDC, hFntStandardItem);
		}
		if (bSelected)
		{
			if (bDisabled)
			{
				SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
				DrawText(hDC, lpCol1, -1, &RCC1, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
				if (szCol2Size)
				{
					DrawText(hDC, lpCol2, -1, &RCC2, DT_VCENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
				}
			}
			else
			{
				SetTextColor(hDC, crFontColorTwo);
				OffsetRect(&RCC1, 1, 1);
				DrawText(hDC, lpCol1, -1, &RCC1, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
				if (szCol2Size)
				{
					OffsetRect(&RCC2, 1, 1);
					DrawText(hDC, lpCol2, -1, &RCC2, DT_VCENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
				}
				SetTextColor(hDC, crFontColorOne);
				OffsetRect(&RCC1, -1, -1);
				DrawText(hDC, lpCol1, -1, &RCC1, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
				if (szCol2Size)
				{
					OffsetRect(&RCC2, -1, -1);
					DrawText(hDC, lpCol2, -1, &RCC2, DT_VCENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
				}
			}
		}
		else
		{
			if (bDisabled)
			{
				SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
			}
			else if (bHighlighted)
			{
				SetTextColor(hDC, crFontColorFour);
			}
			else
			{
				SetTextColor(hDC, crFontColorThree);
			}
			DrawText(hDC, lpCol1, -1, &RCC1, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
			if (szCol2Size)
			{
				DrawText(hDC, lpCol2, -1, &RCC2, DT_VCENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
			}
		}
		SelectObject(hDC, hOldFont);
		return TRUE;
	}
	else return FALSE;
}