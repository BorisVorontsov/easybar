#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "strparser.h"
#include "fileassociations.h"
#include "easybar.h"
#include "associationsdlg.h"

extern WCHAR lpAppPath[MAX_PATH];

static CFileAssociations *pFileAssociations = new CFileAssociations;

INT_PTR CALLBACK AssociationsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			ULONG i;
			WCHAR lpText[64] = {}, lpCACmd[MAX_PATH] = {};
			int intItemsCnt;
			wcscpy(pFileAssociations->m_lpAppPath, lpAppPath);
			pFileAssociations->m_dwIconIndex = 1;
			swprintf(pFileAssociations->m_lpRegAppKey, L"%s.File", APP_NAME);
			swprintf(pFileAssociations->m_lpFileDesc, L"%s media file", APP_NAME);
			swprintf(lpCACmd, L"\"%s\" \"%%1\"", lpAppPath);
			pFileAssociations->AddCustomAction(L"play", L"&Play", lpCACmd, TRUE);
			swprintf(lpCACmd, L"\"%s\" %s \"%%1\"", lpAppPath, APP_CL_KEY_ADD);
			pFileAssociations->AddCustomAction(L"add", L"&Add to Playlist", lpCACmd);
			SendDlgItemMessage(hWnd, IDC_LSTFT, LB_RESETCONTENT, 0, 0);
			for (i = 0; i < APP_SFT_UBOUND; i++)
			{
				if (wcslen(APP_SFT[i]) && (SFT_IsCategory(APP_SFT[i]) == SFTC_NONE))
				{
					swprintf(lpText, L"*.%s", APP_SFT[i]);
					SendDlgItemMessage(hWnd, IDC_LSTFT, LB_ADDSTRING, 0, (LPARAM)lpText);
					intItemsCnt = (int)SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETCOUNT, 0, 0);
					if (pFileAssociations->IsAssociated(APP_SFT[i]))
					{
						SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, intItemsCnt - 1);
					}
				}
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTNVIDEO:
				case IDC_BTNAUDIO:
				case IDC_BTNIMGS:
				case IDC_BTNPLS:
				{
					ULONG i;
					WCHAR lpText[64] = {};
					int intItemsCnt = (int)SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETCOUNT, 0, 0);
					for (i = 0; i < (UINT)intItemsCnt; i++)
					{
						SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETTEXT, i, (LPARAM)lpText);
						SP_ExtractRightPart(lpText, lpText, '.');
						switch (LOWORD(wParam))
						{
							case IDC_BTNVIDEO:
								if(SFT_IsMemberOfCategory(lpText, SFTC_VIDEO))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
							case IDC_BTNAUDIO:
								if(SFT_IsMemberOfCategory(lpText, SFTC_AUDIO) || SFT_IsMemberOfCategory(lpText, SFTC_MIDI))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
							case IDC_BTNIMGS:
								if(SFT_IsMemberOfCategory(lpText, SFTC_IMAGE))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
							case IDC_BTNPLS:
								if(SFT_IsMemberOfCategory(lpText, SFTC_PLAYLIST))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
						}
					}
					break;
				}
				case IDC_BTNRESET:
					SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, FALSE, -1);
					break;
				case IDC_BTNOK:
				{
					ULONG i;
					WCHAR lpText[64] = {};
					BOOL bChanges = FALSE;
					int intItemsCnt = (int)SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETCOUNT, 0, 0);
					for (i = 0; i < (UINT)intItemsCnt; i++)
					{
						SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETTEXT, i, (LPARAM)lpText);
						SP_ExtractRightPart(lpText, lpText, '.');
						if(SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETSEL, i, 0))
						{
							if (!pFileAssociations->IsAssociated(lpText))
							{
								bChanges = TRUE;
								pFileAssociations->AddAssociation(lpText, FALSE);
							}
						}
						else
						{
							if (pFileAssociations->IsAssociated(lpText))
							{
								bChanges = TRUE;
								pFileAssociations->RemoveAssociation(lpText);
							}
						}
					}
					if (bChanges)
					{
						pFileAssociations->UpdateShell();
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