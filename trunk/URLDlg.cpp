#include <windows.h>

#include "resource.h"
#include "strparser.h"
#include "engine.h"
#include "playlist.h"
#include "common.h"
#include "filecollection.h"
#include "easybar.h"
#include "urldlg.h"

#pragma comment (lib, "urlmon.lib")

INT_PTR CALLBACK URLDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			SetDlgItemText(hWnd, IDC_EDTURL, lpwRecentURL);
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_EDTURL, EN_CHANGE), 0);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_EDTURL:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						WCHAR lpwText[MAX_PATH] = { 0 };
						GetDlgItemText(hWnd, IDC_EDTURL, lpwText, MAX_PATH);
						EnableWindow(GetDlgItem(hWnd, IDC_BTNOK), IsURL(lpwText));
					}
					break;
				}
				case IDC_BTNOK:
				{
					WCHAR lpwText[MAX_PATH] = { 0 };
					WCHAR lpwExt[64] = { 0 };
					ULONG lFileCnt = 0;
					GetDlgItemText(hWnd, IDC_EDTURL, lpwText, MAX_PATH);
					SP_ExtractRightPart(lpwText, lpwExt, '.');
					if (SFT_IsMemberOfCategory(lpwExt, SFTC_PLAYLIST))
					{
						WCHAR lpwPath[MAX_PATH] = { 0 };
						ExpandEnvironmentStrings(L"%tmp%\\playlist.tmp", lpwPath, MAX_PATH);
						if (SUCCEEDED(URLDownloadToFile(0, lpwText, lpwPath, 0, 0)))
						{
							lFileCnt += LoadPlaylist(lpwPath);
						}
						DeleteFile(lpwPath);
					}
					else
					{
						pFileCollection->AppendFile(lpwText);
						lFileCnt++;
					}
					if (lFileCnt)
					{
						if (!pEngine->m_lpwFileName)
						{
							if (dwShuffle)
							{
								if (InitTrack(FCF_RANDOM) == -1) InitTrack(FCF_RANDOM);
							}
							else
							{
								InitTrack(FCF_RECENT);
							}
						}
						else
						{
							pFileCollection->SetRecentFile(pEngine->m_lpwFileName);
						}
						wcscpy(lpwRecentURL, lpwText);
					}
					EndDialog(hWnd, 0);
					break;
				}
				case IDC_BTNCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}