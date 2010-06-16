#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

#include "resource.h"
#include "ppgeneraldlg.h"
#include "ppcontentdlg.h"
#include "ppfiltersdlg.h"
#include "ppadjustmentsdlg.h"
#include "common.h"
#include "easybar.h"
#include "filepropdlg.h"

static HWND hPPGeneral		= 0; //Property Page: 'General'
static HWND hPPContent		= 0; //Property Page: '[Media] Content'
static HWND hPPFilters		= 0; //Property Page: '[DirectShow] Filters'
static HWND hPPAdjustments	= 0; //Property Page: 'Adjustments'

extern OSVERSIONINFO OSVI;
extern HINSTANCE hAppInstance;

INT_PTR CALLBACK FilePropDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			TC_ITEM TCI = {};
			RECT RCC = {};
			POINT PTLT = {};
			//Делаем пункт 'Close' у системного меню окна недоступным
			HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
			EnableMenuItem(hSysMenu, SC_CLOSE, MF_DISABLED | MF_GRAYED);
			TCI.mask = TCIF_TEXT;
			TCI.pszText = L"General";
			TabCtrl_InsertItem(GetDlgItem(hWnd, IDC_TABPROP), 0, &TCI);
			TCI.pszText = L"Media Content";
			TabCtrl_InsertItem(GetDlgItem(hWnd, IDC_TABPROP), 1, &TCI);
			TCI.pszText = L"DirectShow Filters";
			TabCtrl_InsertItem(GetDlgItem(hWnd, IDC_TABPROP), 2, &TCI);
			TCI.pszText = L"Adjustments";
			TabCtrl_InsertItem(GetDlgItem(hWnd, IDC_TABPROP), 3, &TCI);
			MoveToCenter(hWnd, -100, 0);
			hPPGeneral = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PP_GENERAL),
				hWnd, PPGeneralDlgProc, 0);
			hPPContent = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PP_CONTENT),
				hWnd, PPContentDlgProc, 0);
			hPPFilters = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PP_FILTERS),
				hWnd, PPFiltersDlgProc, 0);
			hPPAdjustments = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PP_ADJUSTMENTS),
				hWnd, PPAdjustmentsDlgProc, 0);
			GetWindowRect(GetDlgItem(hWnd, IDC_TABPROP), &RCC);
			TabCtrl_AdjustRect(GetDlgItem(hWnd, IDC_TABPROP), FALSE, &RCC);
			PTLT.x = RCC.left;
			PTLT.y = RCC.top;
			ScreenToClient(hWnd, &PTLT);
			MoveWindow(hPPGeneral, PTLT.x, PTLT.y, RCC.right - RCC.left,
				RCC.bottom - RCC.top, FALSE);
			MoveWindow(hPPContent, PTLT.x, PTLT.y, RCC.right - RCC.left,
				RCC.bottom - RCC.top, FALSE);
			MoveWindow(hPPFilters, PTLT.x, PTLT.y, RCC.right - RCC.left,
				RCC.bottom - RCC.top, FALSE);
			MoveWindow(hPPAdjustments, PTLT.x, PTLT.y, RCC.right - RCC.left,
				RCC.bottom - RCC.top, FALSE);
			if ((OSVI.dwMajorVersion > 5) || ((OSVI.dwMajorVersion == 5) &&
				(OSVI.dwMinorVersion >= 1)))
			{
				EnableThemeDialogTexture(hPPGeneral, ETDT_ENABLETAB);
				EnableThemeDialogTexture(hPPContent, ETDT_ENABLETAB);
				EnableThemeDialogTexture(hPPFilters, ETDT_ENABLETAB);
				EnableThemeDialogTexture(hPPAdjustments, ETDT_ENABLETAB);
			}
			//Выбираем по умолчанию 'General'
			TabCtrl_SetCurSel(GetDlgItem(hWnd, IDC_TABPROP), 0);
			ShowWindow(hPPGeneral, SW_SHOW);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTNCLOSE:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
			}
			return TRUE;
		case WM_NOTIFY:
			switch (LOWORD(wParam))
			{
				case IDC_TABPROP:
				{
					LPNMHDR pNM = (LPNMHDR)lParam;
					switch (pNM->code)
					{
						case TCN_SELCHANGING:
							//
							break;
						case TCN_SELCHANGE:
							ShowWindow(hPPGeneral, SW_HIDE);
							ShowWindow(hPPContent, SW_HIDE);
							ShowWindow(hPPFilters, SW_HIDE);
							ShowWindow(hPPAdjustments, SW_HIDE);
							switch (TabCtrl_GetCurSel(pNM->hwndFrom))
							{
								case 0:
									ShowWindow(hPPGeneral, SW_SHOW);
									break;
								case 1:
									ShowWindow(hPPContent, SW_SHOW);
									break;
								case 2:
									ShowWindow(hPPFilters, SW_SHOW);
									break;
								case 3:
									ShowWindow(hPPAdjustments, SW_SHOW);
									break;
							}
							break;
					}
					break;
				}
			}
			return TRUE;
		case WM_CLOSE:
			//DestroyWindow(hPPGeneral);
			//DestroyWindow(hPPContent);
			//DestroyWindow(hPPFilters);
			//DestroyWindow(hPPAdjustments);
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}