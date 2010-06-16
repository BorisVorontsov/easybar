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
			WIN32_FILE_ATTRIBUTE_DATA FAD = {};
			FILETIME LFT = {};
			SYSTEMTIME ST = {};
			WCHAR lpOut[MAX_PATH] = {};
			__int64 intSizeB = 0;
			double dblSizeMB_GB = 0;
			int intLen = 0;
			SIZE SZ = {};
			PPSetDefFileInfo(hWnd, pEngine->m_lpFileName);
			GetFileAttributesEx(pEngine->m_lpFileName, GetFileExInfoStandard, (LPVOID)&FAD);
			SP_ExtractDirectory(pEngine->m_lpFileName, lpOut);
			SP_AddDirSep(lpOut, lpOut);
			SendDlgItemMessage(hWnd, IDC_EDTLOC, WM_SETTEXT, 0, (LPARAM)lpOut);
			SP_ExtractRightPart(pEngine->m_lpFileName, lpOut, '.');
			_wcsupr(lpOut);
			SendDlgItemMessage(hWnd, IDC_EDTEXT, WM_SETTEXT, 0, (LPARAM)lpOut);
			if (IsURL(pEngine->m_lpFileName))
			{
				wcscpy(lpOut, L"--");
			}
			else
			{
				intSizeB = ((__int64)FAD.nFileSizeHigh << 32) + FAD.nFileSizeLow;
				if ((intSizeB > 0) && (intSizeB < (__int64)pow(2.0, 32.0)))
				{
					dblSizeMB_GB = (((double)intSizeB / 1024) / 1024);
					swprintf(lpOut, L"%.1f MB (%i bytes)", dblSizeMB_GB, intSizeB);
				}
				else
				{
					dblSizeMB_GB = ((((double)intSizeB / 1024) / 1024) / 1024); 
					swprintf(lpOut, L"%.1f GB", dblSizeMB_GB);
				}
			}
			SendDlgItemMessage(hWnd, IDC_EDTFS, WM_SETTEXT, 0, (LPARAM)lpOut);
			if (pEngine->HasVideo())
			{
				wcscpy(lpOut, L"Video");
				if (pEngine->HasAudio())
					wcscat(lpOut, L" (with audio)");
			}
			else if (pEngine->HasAudio())
			{
				wcscpy(lpOut, L"Audio");
			}
			else
			{
				wcscpy(lpOut, L"Unknown");
			}
			SendDlgItemMessage(hWnd, IDC_EDTMT, WM_SETTEXT, 0, (LPARAM)lpOut);
			intLen = (int)pEngine->GetLength();
			if (((intLen / 1000) / 3600))
			{
				swprintf(lpOut, L"%02i:%02i:%02i", (intLen / 1000) / 3600,
					((intLen / 1000) / 60) % 60, ((intLen / 1000) % 60));
			}
			else
			{
				swprintf(lpOut, L"%02i:%02i", ((intLen / 1000) / 60) % 60,
					((intLen / 1000) % 60));
			}
			SendDlgItemMessage(hWnd, IDC_EDTML, WM_SETTEXT, 0, (LPARAM)lpOut);
			if (pEngine->HasVideo())
			{
				pEngine->GetOriginalVideoSize(&SZ);
				swprintf(lpOut, L"%i x %i", SZ.cx, SZ.cy);
				SendDlgItemMessage(hWnd, IDC_EDTVS, WM_SETTEXT, 0, (LPARAM)lpOut);
			}
			else
			{
				SendDlgItemMessage(hWnd, IDC_EDTVS, WM_SETTEXT, 0, (LPARAM)L"Not available");
			}
			if (IsURL(pEngine->m_lpFileName))
			{
				wcscpy(lpOut, L"--");
			}
			else
			{
				WCHAR lpTmp[128] = {};
				FileTimeToLocalFileTime(&FAD.ftCreationTime, &LFT);
				FileTimeToSystemTime(&LFT, &ST);
				//ќтображение даты и времени с учетом региональных настроек пользовател€
				//------------------------------------------------------------------------------
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &ST, 0, lpTmp, (sizeof(lpTmp) / sizeof(WCHAR)));
				wcscpy(lpOut, lpTmp);
				wcscat(lpOut, L" ");
				GetTimeFormat(LOCALE_USER_DEFAULT, 0, &ST, 0, lpTmp, (sizeof(lpTmp) / sizeof(WCHAR)));
				wcscat(lpOut, lpTmp);
				//------------------------------------------------------------------------------
				//swprintf(lpOut, L"%i/%i/%i, %i:%02i:%02i", ST.wMonth, ST.wDay,
				//	ST.wYear, ST.wHour, ST.wMinute, ST.wSecond);
			}
			SendDlgItemMessage(hWnd, IDC_EDTCR, WM_SETTEXT, 0, (LPARAM)lpOut);
			swprintf(lpOut, L"%s%s%s%s", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ==
				FILE_ATTRIBUTE_ARCHIVE)?L"<Read Only>":L"", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ==
				FILE_ATTRIBUTE_HIDDEN)?L"<Hidden>":L"", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ==
				FILE_ATTRIBUTE_ARCHIVE)?L"<Archive>":L"", ((FAD.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ==
				FILE_ATTRIBUTE_SYSTEM)?L"<System>":L"");
				SendDlgItemMessage(hWnd, IDC_EDTAT, WM_SETTEXT, 0, (LPARAM)((wcslen(lpOut))?lpOut:L"<None>"));
			return TRUE;
		}
	}
	return FALSE;
}