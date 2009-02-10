 #include <windows.h>

#include "resource.h"
#include "engine.h"
#include "common.h"
#include "easybar.h"
#include "ppadjustmentsdlg.h"
#include "ppfiltersdlg.h"

static HWND hPPFilters;

INT_PTR CALLBACK PPFiltersDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			hPPFilters = hWnd;

			PPSetDefFileInfo(hWnd, pEngine->m_lpwFileName);
			GetFGFilters();
			GetFGStreams();
			GetFGStreams(TRUE);
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_LSTFILTERS, LBN_SELCHANGE), 0);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LSTFILTERS:
				{
					switch (HIWORD(wParam))
					{
						case LBN_SELCHANGE:
						{
							WCHAR lpwText[MAX_PATH] = { 0 };
							DWORD dwSelItem = SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETCURSEL, 0, 0);
							SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETTEXT, dwSelItem, (LPARAM)lpwText);
							EnableWindow(GetDlgItem(hWnd, IDC_BTNPROP), (pEngine->FGFiltersPropertyPages(lpwText, TRUE) >= 0));
							break;
						}
						case LBN_DBLCLK:
							if (IsWindowEnabled(GetDlgItem(hWnd, IDC_BTNPROP)))
								PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTNPROP, 0), 0);
							break;
					}
					break;
				}
				case IDC_BTNREFR:
					GetFGFilters();
					GetFGStreams();
					GetFGStreams(TRUE);
					break;
				case IDC_BTNPROP:
				{
					WCHAR lpwText[MAX_PATH] = { 0 };
					DWORD dwSelItem = SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETTEXT, dwSelItem, (LPARAM)lpwText);
					pEngine->FGFiltersPropertyPages(lpwText, FALSE);
					break;
				}
			}
			return TRUE;
	}
	return FALSE;
}

void GetFGFilters()
{
	ULONG i;
	DWORD dwFASize = 0;
	pEngine->UpdateFGFiltersArray();
	pEngine->GetFGFiltersNames(0, &dwFASize, 0);
	LPWSTR *lpwFGFilters = new LPWSTR[dwFASize];
	for (i = 0; i < dwFASize; i++)
		lpwFGFilters[i] = new WCHAR[MAX_PATH];
	if (pEngine->GetFGFiltersNames(&lpwFGFilters[0], &dwFASize, MAX_PATH) >= 0)
	{
		SendDlgItemMessage(hPPFilters, IDC_LSTFILTERS, LB_RESETCONTENT, 0, 0);
		for (i = 0; i < dwFASize; i++)
		{
			SendDlgItemMessage(hPPFilters, IDC_LSTFILTERS, LB_ADDSTRING, 0,
				(LPARAM)lpwFGFilters[i]);
		}
		SendDlgItemMessage(hPPFilters, IDC_LSTFILTERS, LB_SETCURSEL, 0, 0);
		SetLBHorizontalExtent(GetDlgItem(hPPFilters, IDC_LSTFILTERS));
	}
	for (i = 0; i < dwFASize; i++)
		delete[] lpwFGFilters[i];
	delete[] lpwFGFilters;
}