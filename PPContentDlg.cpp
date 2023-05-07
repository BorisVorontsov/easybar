#include <windows.h>

#include "resource.h"
#include "engine.h"
#include "easybar.h"
#include "ppcontentdlg.h"

INT_PTR CALLBACK PPContentDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			MEDIACONTENT MC = {};
			LPWSTR lpOut = new WCHAR[MAX_PATH];
			PPSetDefFileInfo(hWnd, pEngine->m_lpFileName);
			pEngine->UpdateFGFiltersArray();
			pEngine->GetMediaContent(&MC);
			wcscpy(lpOut, (wcslen(MC.Title) > 0)?MC.Title:L"None");
			SendDlgItemMessage(hWnd, IDC_EDTCLIP, WM_SETTEXT, 0, (LPARAM)lpOut);
			wcscpy(lpOut, (wcslen(MC.Author) > 0)?MC.Author:L"None");
			SendDlgItemMessage(hWnd, IDC_EDTAUTH, WM_SETTEXT, 0, (LPARAM)lpOut);
			wcscpy(lpOut, (wcslen(MC.Copyright) > 0)?MC.Copyright:L"None");
			SendDlgItemMessage(hWnd, IDC_EDTCOP, WM_SETTEXT, 0, (LPARAM)lpOut);
			wcscpy(lpOut, (wcslen(MC.Rating) > 0)?MC.Rating:L"None");
			SendDlgItemMessage(hWnd, IDC_EDTRAT, WM_SETTEXT, 0, (LPARAM)lpOut);
			wcscpy(lpOut, (wcslen(MC.Description) > 0)?MC.Description:L"None");
			SendDlgItemMessage(hWnd, IDC_EDTDESC, WM_SETTEXT, 0, (LPARAM)lpOut);
			wcscpy(lpOut, (wcslen(MC.MoreInfo) > 0)?MC.MoreInfo:L"None");
			SendDlgItemMessage(hWnd, IDC_EDTMI, WM_SETTEXT, 0, (LPARAM)lpOut);
			delete[] lpOut;
			return TRUE;
		}
	}
	return FALSE;
}