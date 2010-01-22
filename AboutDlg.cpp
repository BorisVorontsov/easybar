#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "labelex.h"
#include "easybar.h"
#include "aboutdlg.h"

#pragma comment (lib, "winmm.lib")

extern HINSTANCE hAppInstance;
extern WCHAR lpwAppVersion[20];

INT_PTR CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//������������� �������
			//--------------------------------------------------------------------
			WCHAR lpwTitle[64] = { 0 };
			WCHAR lpwAbout[256] = { 0 };
			HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
			EnableMenuItem(hSysMenu, SC_CLOSE, MF_DISABLED | MF_GRAYED);
			swprintf(lpwTitle, L"About %s", APP_NAME);
			SetWindowText(hWnd, lpwTitle);
			swprintf(lpwAbout, L"%s v. %s", APP_NAME,
				lpwAppVersion);
			SetDlgItemText(hWnd, IDC_STCNAME, lpwAbout);
			SetDlgItemText(hWnd, IDC_STCCOP, APP_COPYRIGHT);
			SendDlgItemMessage(hWnd, IDC_STCEM, LEM_SETNFONTCOLOR,
				(WPARAM)RGB(20, 60, 145), 0);
			SendDlgItemMessage(hWnd, IDC_STCEM, LEM_SETHFONTCOLOR,
				(WPARAM)RGB(90, 138, 230), 0);
			swprintf(lpwAbout, L"E-mail: %s", APP_EMAIL);
			SetDlgItemText(hWnd, IDC_STCEM, lpwAbout);
			swprintf(lpwAbout, APP_LICENSE, APP_LICENSE_FILE);
			SetDlgItemText(hWnd, IDC_STCLIC, lpwAbout);
			LoadString(hAppInstance, IDS_CREDITS, lpwAbout, sizeof(lpwAbout) / sizeof(WCHAR));
			SetDlgItemText(hWnd, IDC_EDTCRED, lpwAbout);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_STCICON:
				{
					//������, � ������� :)
					if (HIWORD(wParam) == STN_DBLCLK)
					{
						HRSRC hEERes;
						HGLOBAL hEEData;
						LPBYTE pEEData;
						LONG i = 0, lEESize = 0;
						hEERes = FindResource(0, L"#1001", RT_RCDATA);
						lEESize = SizeofResource(0, hEERes);
						hEEData = LoadResource(0, hEERes);
						pEEData = (LPBYTE)LockResource(hEEData);
						for (; i < lEESize; i++)
							pEEData[i] ^= EASTEREGG_XOR_KEY;
						PlaySound((LPWSTR)pEEData, 0, SND_MEMORY | SND_ASYNC);
					}
					break;
				}
				case IDC_BTNOK:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				case IDC_BTNCRED:
					if (IsWindowVisible(GetDlgItem(hWnd, IDC_EDTCRED)))
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCNAME), SW_SHOW);
						ShowWindow(GetDlgItem(hWnd, IDC_STCCOP), SW_SHOW);
						ShowWindow(GetDlgItem(hWnd, IDC_STCEM), SW_SHOW);
						ShowWindow(GetDlgItem(hWnd, IDC_STCLIC), SW_SHOW);
						ShowWindow(GetDlgItem(hWnd, IDC_EDTCRED), SW_HIDE);
						SetDlgItemText(hWnd, IDC_BTNCRED, L"&Credits >");
					}
					else
					{
						ShowWindow(GetDlgItem(hWnd, IDC_STCNAME), SW_HIDE);
						ShowWindow(GetDlgItem(hWnd, IDC_STCCOP), SW_HIDE);
						ShowWindow(GetDlgItem(hWnd, IDC_STCEM), SW_HIDE);
						ShowWindow(GetDlgItem(hWnd, IDC_STCLIC), SW_HIDE);
						ShowWindow(GetDlgItem(hWnd, IDC_EDTCRED), SW_SHOW);
						SetDlgItemText(hWnd, IDC_BTNCRED, L"< &About");
					}
					break;
			}
			return TRUE;
		case WM_NOTIFY:
		{
			LPNMHDR pNM = (LPNMHDR)lParam;
			switch (wParam)
			{
				case IDC_STCEM:
					if (pNM->code == LEN_CLICKED)
					{
						WCHAR lpwTmp[128] = { 0 };
						swprintf(lpwTmp, L"mailto:%s?subject=%s %s", APP_EMAIL, APP_NAME, lpwAppVersion);
						ShellExecute(0, L"Open", lpwTmp, 0, 0, SW_NORMAL);
					}
					break;
				default:
					//
					break;
			}
			return TRUE;
		}
		case WM_CLOSE:
			PlaySound(0, 0, 0);
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}
