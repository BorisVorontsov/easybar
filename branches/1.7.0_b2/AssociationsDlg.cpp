#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "strparser.h"
#include "fileassociations.h"
#include "easybar.h"
#include "associationsdlg.h"

extern WCHAR lpwAppPath[MAX_PATH];

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
			WCHAR lpwText[64] = { 0 }, lpwCACmd[MAX_PATH] = { 0 };
			int intItemsCnt;
			wcscpy(pFileAssociations->m_lpwAppPath, lpwAppPath);
			pFileAssociations->m_dwIconIndex = 1;
			swprintf(pFileAssociations->m_lpwRegAppKey, L"%s.File", APP_NAME);
			swprintf(pFileAssociations->m_lpwFileDesc, L"%s media file", APP_NAME);
			swprintf(lpwCACmd, L"\"%s\" \"%%1\"", lpwAppPath);
			pFileAssociations->AddCustomAction(L"play", L"&Play", lpwCACmd, TRUE);
			swprintf(lpwCACmd, L"\"%s\" %s \"%%1\"", lpwAppPath, APP_CL_KEY_ADD);
			pFileAssociations->AddCustomAction(L"add", L"&Add to Playlist", lpwCACmd);
			SendDlgItemMessage(hWnd, IDC_LSTFT, LB_RESETCONTENT, 0, 0);
			for (i = 0; i < APP_SFT_UBOUND; i++)
			{
				if (wcslen(APP_SFT[i]) && (SFT_IsCategory(APP_SFT[i]) == SFTC_NONE))
				{
					swprintf(lpwText, L"*.%s", APP_SFT[i]);
					SendDlgItemMessage(hWnd, IDC_LSTFT, LB_ADDSTRING, 0, (LPARAM)lpwText);
					intItemsCnt = SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETCOUNT, 0, 0);
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
					WCHAR lpwText[64] = { 0 };
					SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, FALSE, -1);
					int intItemsCnt = SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETCOUNT, 0, 0);
					for (i = 0; i < (UINT)intItemsCnt; i++)
					{
						SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETTEXT, i, (LPARAM)lpwText);
						SP_ExtractRightPart(lpwText, lpwText, '.');
						switch (LOWORD(wParam))
						{
							case IDC_BTNVIDEO:
								if(SFT_IsMemberOfCategory(lpwText, SFTC_VIDEO))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
							case IDC_BTNAUDIO:
								if(SFT_IsMemberOfCategory(lpwText, SFTC_AUDIO) || SFT_IsMemberOfCategory(lpwText, SFTC_MIDI))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
							case IDC_BTNIMGS:
								if(SFT_IsMemberOfCategory(lpwText, SFTC_IMAGE))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
							case IDC_BTNPLS:
								if(SFT_IsMemberOfCategory(lpwText, SFTC_PLAYLIST))
									SendDlgItemMessage(hWnd, IDC_LSTFT, LB_SETSEL, TRUE, i);
								break;
						}
					}
					break;
				}
				case IDC_BTNOK:
				{
					ULONG i;
					WCHAR lpwText[64] = { 0 };
					BOOL bChanges = FALSE;
					int intItemsCnt = SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETCOUNT, 0, 0);
					for (i = 0; i < (UINT)intItemsCnt; i++)
					{
						SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETTEXT, i, (LPARAM)lpwText);
						SP_ExtractRightPart(lpwText, lpwText, '.');
						if(SendDlgItemMessage(hWnd, IDC_LSTFT, LB_GETSEL, i, 0))
						{
							if (!pFileAssociations->IsAssociated(lpwText))
							{
								bChanges = TRUE;
								pFileAssociations->AddAssociation(lpwText, FALSE);
							}
						}
						else
						{
							if (pFileAssociations->IsAssociated(lpwText))
							{
								bChanges = TRUE;
								pFileAssociations->RemoveAssociation(lpwText);
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