#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "strparser.h"
#include "common.h"
#include "engine.h"
#include "easybar.h"
#include "ppgeneraldlg.h"

INT_PTR CALLBACK PPGeneralDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//»нициализаци€ диалога
			//--------------------------------------------------------------------
			WIN32_FILE_ATTRIBUTE_DATA FAD = { 0 };
			FILETIME LFT = { 0 };
			SYSTEMTIME ST = { 0 };
			WCHAR lpwOut[MAX_PATH] = { 0 };
			__int64 intSizeB = 0;
			double dblSizeMB = 0;
			int intLen = 0;
			SIZE SZ = { 0 };
			PPSetDefFileInfo(hWnd, pEngine->m_lpwFileName);
			GetFileAttributesEx(pEngine->m_lpwFileName, GetFileExInfoStandard, (LPVOID)&FAD);
			SP_ExtractDirectory(pEngine->m_lpwFileName, lpwOut);
			SP_AddDirSep(lpwOut, lpwOut);
			SendDlgItemMessage(hWnd, IDC_EDTLOC, WM_SETTEXT, 0, (LPARAM)lpwOut);
			SP_ExtractRightPart(pEngine->m_lpwFileName, lpwOut, '.');
			_wcsupr(lpwOut);
			SendDlgItemMessage(hWnd, IDC_EDTEXT, WM_SETTEXT, 0, (LPARAM)lpwOut);
			if (IsURL(pEngine->m_lpwFileName))
			{
				wcscpy(lpwOut, L"--");
			}
			else
			{
				intSizeB = MAKEINT64(FAD.nFileSizeLow, FAD.nFileSizeHigh);
				dblSizeMB = (((double)intSizeB / 1024) / 1024);
				swprintf(lpwOut, L"%.1f MB (%i bytes)", dblSizeMB, (int)intSizeB);
			}
			SendDlgItemMessage(hWnd, IDC_EDTFS, WM_SETTEXT, 0, (LPARAM)lpwOut);
			SendDlgItemMessage(hWnd, IDC_EDTMT, WM_SETTEXT, 0, 
				(pEngine->IsVideo())?(LPARAM)L"Video":(LPARAM)L"Audio");
			intLen = pEngine->GetLength();
			swprintf(lpwOut, L"%02i:%02i:%02i", (intLen / 1000) / 3600,
				((intLen / 1000) / 60) % 60, ((intLen / 1000) % 60));
			SendDlgItemMessage(hWnd, IDC_EDTML, WM_SETTEXT, 0, (LPARAM)lpwOut);
			if (pEngine->IsVideo())
			{
				pEngine->GetOriginalVideoSize(&SZ);
				swprintf(lpwOut, L"%i x %i", SZ.cx, SZ.cy);
				SendDlgItemMessage(hWnd, IDC_EDTVS, WM_SETTEXT, 0, (LPARAM)lpwOut);
			}
			else
			{
				SendDlgItemMessage(hWnd, IDC_EDTVS, WM_SETTEXT, 0, (LPARAM)L"Not available");
			}
			if (IsURL(pEngine->m_lpwFileName))
			{
				wcscpy(lpwOut, L"--");
			}
			else
			{
				WCHAR lpwTmp[128] = { 0 };
				FileTimeToLocalFileTime(&FAD.ftCreationTime, &LFT);
				FileTimeToSystemTime(&LFT, &ST);
				//ќтображение даты и времени с учетом региональных настроек пользовател€
				//------------------------------------------------------------------------------
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &ST, 0, lpwTmp, (sizeof(lpwTmp) / sizeof(WCHAR)));
				wcscpy(lpwOut, lpwTmp);
				wcscat(lpwOut, L" ");
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &ST, 0, lpwTmp, (sizeof(lpwTmp) / sizeof(WCHAR)));
				wcscat(lpwOut, lpwTmp);
				//------------------------------------------------------------------------------
				//swprintf(lpwOut, L"%i/%i/%i, %i:%02i:%02i", ST.wMonth, ST.wDay,
				//	ST.wYear, ST.wHour, ST.wMinute, ST.wSecond);
			}
			SendDlgItemMessage(hWnd, IDC_EDTCR, WM_SETTEXT, 0, (LPARAM)lpwOut);
			swprintf(lpwOut, L"%s%s%s%s", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ==
				FILE_ATTRIBUTE_ARCHIVE)?L"<Read Only>":L"", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ==
				FILE_ATTRIBUTE_HIDDEN)?L"<Hidden>":L"", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ==
				FILE_ATTRIBUTE_ARCHIVE)?L"<Archive>":L"", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ==
				FILE_ATTRIBUTE_SYSTEM)?L"<System>":L"");
				SendDlgItemMessage(hWnd, IDC_EDTAT, WM_SETTEXT, 0, (LPARAM)((wcslen(lpwOut))?lpwOut:L"<None>"));
			return TRUE;
		}
	}
	return FALSE;
}