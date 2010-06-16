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

			PPSetDefFileInfo(hWnd, pEngine->m_lpFileName);
			GetFGFilters();
			GetFGAudioStreams();
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
							WCHAR lpText[MAX_PATH] = {};
							DWORD dwSelItem = (DWORD)SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETCURSEL, 0, 0);
							SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETTEXT, dwSelItem, (LPARAM)lpText);
							EnableWindow(GetDlgItem(hWnd, IDC_BTNPROP), (pEngine->FGFiltersPropertyPages(lpText, TRUE) >= 0));
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
					GetFGAudioStreams();
					break;
				case IDC_BTNPROP:
				{
					WCHAR lpText[MAX_PATH] = {};
					DWORD dwSelItem = (DWORD)SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hWnd, IDC_LSTFILTERS, LB_GETTEXT, dwSelItem, (LPARAM)lpText);
					pEngine->FGFiltersPropertyPages(lpText, FALSE);
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
	LPWSTR *lpFGFilters = new LPWSTR[dwFASize];
	for (i = 0; i < dwFASize; i++)
		lpFGFilters[i] = new WCHAR[MAX_PATH];
	if (pEngine->GetFGFiltersNames(&lpFGFilters[0], &dwFASize, MAX_PATH) >= 0)
	{
		SendDlgItemMessage(hPPFilters, IDC_LSTFILTERS, LB_RESETCONTENT, 0, 0);
		for (i = 0; i < dwFASize; i++)
		{
			SendDlgItemMessage(hPPFilters, IDC_LSTFILTERS, LB_ADDSTRING, 0,
				(LPARAM)lpFGFilters[i]);
		}
		SendDlgItemMessage(hPPFilters, IDC_LSTFILTERS, LB_SETCURSEL, 0, 0);
		SetLBHorizontalExtent(GetDlgItem(hPPFilters, IDC_LSTFILTERS));
	}
	for (i = 0; i < dwFASize; i++)
		delete[] lpFGFilters[i];
	delete[] lpFGFilters;
}