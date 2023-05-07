#include <windows.h>
#include <shlwapi.h>

#include "resource.h"
#include "strparser.h"
#include "engine.h"
#include "playlist.h"
#include "common.h"
#include "filecollection.h"
#include "easybar.h"
#include "urldlg.h"

#pragma comment (lib, "urlmon.lib")
#pragma comment (lib, "shlwapi.lib")

INT_PTR CALLBACK URLDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			CoInitialize(NULL);
			SetDlgItemText(hWnd, IDC_EDTURL, lpRecentURL);
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_EDTURL, EN_CHANGE), 0);
			SHAutoComplete(GetDlgItem(hWnd, IDC_EDTURL), SHACF_URLALL);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_EDTURL:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						WCHAR lpText[MAX_PATH] = {};
						GetDlgItemText(hWnd, IDC_EDTURL, lpText, MAX_PATH);
						EnableWindow(GetDlgItem(hWnd, IDC_BTNOK), IsURL(lpText));
					}
					break;
				}
				case IDC_BTNOK:
				{
					WCHAR lpText[MAX_PATH] = {};
					WCHAR lpExt[64] = {};
					ULONG lFileCnt = 0;
					GetDlgItemText(hWnd, IDC_EDTURL, lpText, MAX_PATH);
					SP_ExtractRightPart(lpText, lpExt, '.');
					if (SFT_IsMemberOfCategory(lpExt, SFTC_PLAYLIST))
					{
						WCHAR lpPath[MAX_PATH] = {};
						ExpandEnvironmentStrings(L"%tmp%\\playlist.tmp", lpPath, MAX_PATH);
						if (SUCCEEDED(URLDownloadToFile(0, lpText, lpPath, 0, NULL)))
						{
							lFileCnt += LoadPlaylist(lpPath);
						}
						DeleteFile(lpPath);
					}
					else
					{
						pFileCollection->AppendFile(lpText);
						lFileCnt++;
					}
					if (lFileCnt)
					{
						if (!pEngine->m_lpFileName)
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
							pFileCollection->SetRecentFile(pEngine->m_lpFileName);
						}
						wcscpy(lpRecentURL, lpText);
					}
					EndDialog(hWnd, 0);
					break;
				}
				case IDC_BTNCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
		case WM_DESTROY:
			CoUninitialize();
			return TRUE;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
	}
	return FALSE;
}