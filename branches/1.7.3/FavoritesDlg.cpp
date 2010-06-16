#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>
#include <stdio.h>

#include "resource.h"
#include "strparser.h"
#include "engine.h"
#include "playlist.h"
#include "common.h"
#include "filecollection.h"
#include "easybar.h"
#include "favoritesdlg.h"

extern HINSTANCE hAppInstance;

INT_PTR CALLBACK FavoritesDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			ULONG i = 0;
			LV_COLUMN LVC = {};
			LV_ITEM LVI = {};
			HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
			EnableMenuItem(hSysMenu, SC_CLOSE, MF_DISABLED | MF_GRAYED);
			LVC.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
			LVC.pszText = L"Name";
			LVC.fmt = LVCFMT_LEFT;
			LVC.cx = 220;
			ListView_InsertColumn(GetDlgItem(hWnd, IDC_LVFILES), 0, &LVC);
			LVC.pszText = L"Path";
			LVC.fmt = LVCFMT_LEFT;
			LVC.cx = 420;
			ListView_InsertColumn(GetDlgItem(hWnd, IDC_LVFILES), 1, &LVC);
			for (; i < APP_MAX_STRINGS; i++)
			{
				if (!pFavorites[i]) break;
				LVI.mask = LVIF_TEXT;
				LVI.pszText = pFavorites[i]->lpDisplayName;
				LVI.iItem = i;
				ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
				ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1,
					pFavorites[i]->lpPath);
			}
			SetTimer(hWnd, 1, 200, 0);
			SendMessage(hWnd, WM_TIMER, 1, 0);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTNACF:
				{
					ULONG lIndex;
					LV_ITEM LVI = {};
					SHELLFLAGSTATE SFS = {};
					WCHAR lpName[128] = {};
					lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
					SP_ExtractName(pEngine->m_lpFileName, lpName);
					SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
					if (!SFS.fShowExtensions)
					{
						SP_ExtractLeftPart(lpName, lpName, '.');
					}
					LVI.mask = LVIF_TEXT;
					LVI.pszText = lpName;
					LVI.iItem = lIndex;
					ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
					ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), lIndex, 1, pEngine->m_lpFileName);
					break;
				}
				case IDC_BTNADD:
				{
					WCHAR lpODFile[APP_OD_MS_MAX_FILE] = {};
					WCHAR lpODFilter[APP_OD_MAX_FILTER] = {};
					WCHAR lpExt[64] = {};
					LPWSTR pFiles[FC_MAX_FILES];
					ULONG i, lIndex, lODFileCnt = 0;
					LV_ITEM LVI = {};
					SHELLFLAGSTATE SFS = {};
					WCHAR lpName[128] = {};
					ZeroMemory(pFiles, sizeof(pFiles));
					wcscpy(lpODFilter, L"Media files (all types)|");
					for (i = 0; i < APP_SFT_UBOUND; i++)
					{
						if (wcslen(APP_SFT[i]) && (SFT_IsCategory(APP_SFT[i]) == SFTC_NONE))
						{
							swprintf(lpExt, L"*.%s;", APP_SFT[i]);
							wcscat(lpODFilter, lpExt);
						}
					}
					for (i = 0; i < APP_SFT_UBOUND; i++)
					{
						if (wcslen(APP_SFT[i]))
						{
							switch(SFT_IsCategory(APP_SFT[i]))
							{
								case SFTC_NONE:
									swprintf(lpExt, L"*.%s;", APP_SFT[i]);
									wcscat(lpODFilter, lpExt);
									break;
								case SFTC_VIDEO:
									wcscat(lpODFilter, L"|Video file|");
									break;
								case SFTC_AUDIO:
									wcscat(lpODFilter, L"|Audio file|");
									break;
								case SFTC_MIDI:
									wcscat(lpODFilter, L"|MIDI file|");
									break;
								case SFTC_IMAGE:
									wcscat(lpODFilter, L"|Image file|");
									break;
								case SFTC_PLAYLIST:
									wcscat(lpODFilter, L"|Playlist file|");
									break;
								case SFTC_OTHER:
									wcscat(lpODFilter, L"|Other|");
									break;
							}
						}
					}
					wcscat(lpODFilter, L"|Any file|*.*;||");
					for (i = 0; i < (APP_OD_MAX_FILTER - 1); i++)
					{
						if (lpODFilter[i] == '|')
						{
							if (lpODFilter[i + 1] != '|')
							{
								lpODFilter[i] = '\0';
							}
							else break;
						}
					}
					if (GetOpenDialog(hAppInstance, hWnd, L"Add File(s)", lpODFile, APP_OD_MS_MAX_FILE - 1,
						lpODFilter, 1, TRUE, lpRecentDir))
					{
						for (i = 0; i < (APP_OD_MS_MAX_FILE - 1); i++)
						{
							if ((lpODFile[i] == '\0') && (lpODFile[i + 1] != '\0'))
							{
								lpODFile[i] = '|';
							}
						}
						lODFileCnt = SP_Split(lpODFile, &pFiles[0], '|', FC_MAX_FILES);
						if (lODFileCnt > 1)
						{
							WCHAR lpDir[MAX_PATH] = {}, lpFile[MAX_PATH] = {};
							wcscpy(lpDir, pFiles[0]);
							SP_AddDirSep(lpDir, lpDir);
							wcscpy(lpRecentDir, lpDir);
							for (i = 1; i < lODFileCnt; i++)
							{
								wcscpy(lpFile, lpDir);
								wcscat(lpFile, pFiles[i]);
								lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
								wcscpy(lpName, pFiles[i]);
								SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
								if (!SFS.fShowExtensions)
								{
									SP_ExtractLeftPart(lpName, lpName, '.');
								}
								LVI.mask = LVIF_TEXT;
								LVI.pszText = lpName;
								LVI.iItem = lIndex;
								ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
								ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), lIndex, 1, lpFile);
							}
							for (i = 0; i < lODFileCnt; i++)
								delete[] pFiles[i];
						}
						else
						{
							SP_ExtractDirectory(pFiles[0], lpRecentDir);
							SP_AddDirSep(lpRecentDir, lpRecentDir);
							lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
							SP_ExtractName(pFiles[0], lpName);
							SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
							if (!SFS.fShowExtensions)
							{
								SP_ExtractLeftPart(lpName, lpName, '.');
							}
							LVI.mask = LVIF_TEXT;
							LVI.pszText = lpName;
							LVI.iItem = lIndex;
							ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), lIndex, 1, pFiles[0]);
							delete[] pFiles[0];
						}
					}
					break;
				}
				case IDC_BTNREM:
				{
					ULONG i, lItemCnt;
Removing_NextItem:
					lItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
					for (i = 0; i < lItemCnt; i++)
					{
						if (ListView_GetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							ListView_DeleteItem(GetDlgItem(hWnd, IDC_LVFILES), i);
							goto Removing_NextItem;
						}
					}
					break;
				}
				case IDC_BTNCA:
					if (MessageBox(hWnd, L"Are you sure you want to clear all?", APP_NAME,
						MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDYES)
					{
						ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LVFILES));
					}
					break;
				case IDC_BTNMU:
				{
					ULONG i, lItemCnt;
					WCHAR lpName1[128] = {}, lpName2[128] = {};
					WCHAR lpPath1[MAX_PATH] = {}, lpPath2[MAX_PATH] = {};
					lItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
					if (lItemCnt <= 1) break;
					for (i = 1; i < lItemCnt; i++)
					{
						if (ListView_GetItemState(GetDlgItem(hWnd, IDC_LVFILES), i - 1, LVIS_SELECTED)
							== LVIS_SELECTED)
						{
							continue;
						}
						if (ListView_GetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 0, lpName1, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 1, lpPath1, MAX_PATH);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpName2, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpPath2, MAX_PATH);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 0, lpName2);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 1, lpPath2);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i - 1, LVIS_SELECTED, LVIS_SELECTED);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpName1);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpPath1);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, 0, LVIS_SELECTED);
						}
					}
					break;
				}
				case IDC_BTNMD:
				{
					ULONG lItemCnt;
					WCHAR lpName1[128] = {}, lpName2[128] = {};
					WCHAR lpPath1[MAX_PATH] = {}, lpPath2[MAX_PATH] = {};
					lItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
					if (lItemCnt <= 1) break;
					for (LONG i = (lItemCnt - 2); i >= 0; i--)
					{
						if (ListView_GetItemState(GetDlgItem(hWnd, IDC_LVFILES), i + 1, LVIS_SELECTED)
							== LVIS_SELECTED)
						{
							continue;
						}
						if (ListView_GetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 0, lpName1, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 1, lpPath1, MAX_PATH);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpName2, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpPath2, MAX_PATH);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 0, lpName2);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 1, lpPath2);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i + 1, LVIS_SELECTED, LVIS_SELECTED);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpName1);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpPath1);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, 0, LVIS_SELECTED);
						}
					}
					break;
				}
				case IDC_BTNSEL:
				{
					ULONG i = 0, lItemCnt = 0, lFileCnt = 0;
					WCHAR lpPath[MAX_PATH] = {};
					WCHAR lpExt[64] = {};
					lItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
					pFileCollection->Clear();
					for (; i < lItemCnt; i++)
					{
						if (ListView_GetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpPath, MAX_PATH);
							SP_ExtractRightPart(lpPath, lpExt, '.');
							if (SFT_IsMemberOfCategory(lpExt, SFTC_PLAYLIST))
							{
								lFileCnt += LoadPlaylist(lpPath);
							}
							else if (IsDirectory(lpPath))
							{
								lFileCnt += LoadDirectory(lpPath);
							}
							else
							{
								pFileCollection->AppendFile(lpPath);
								lFileCnt++;
							}
						}
					}
					if (lFileCnt)
					{
						InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
						if (!(GetAsyncKeyState(VK_SHIFT) >> 15))
							pEngine->Play(); //Auto-play
					}
					else
					{
						if (pEngine->m_lpFileName) CloseTrack();
					}
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				}
				case IDC_BTNCLOSE:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
			}
			return TRUE;
		case WM_DROPFILES:
		{
			WCHAR lpDFile[MAX_PATH] = {};
			ULONG i = 0, lIndex, lDrFileCnt = 0;
			LV_ITEM LVI = {};
			SHELLFLAGSTATE SFS = {};
			WCHAR lpName[128] = {};
			HDROP hDrop = (HDROP)wParam;
			lDrFileCnt = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);
			for (; i < lDrFileCnt; i++)
			{
				DragQueryFile(hDrop, i, lpDFile, MAX_PATH);
				lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
				if (IsDirectory(lpDFile))
				{
					SP_RemDirSep(lpDFile, lpDFile);
					SP_ExtractName(lpDFile, lpName);
				}
				else
				{
					SP_ExtractName(lpDFile, lpName);
					SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
					if (!SFS.fShowExtensions)
					{
						SP_ExtractLeftPart(lpName, lpName, '.');
					}
				}
				LVI.mask = LVIF_TEXT;
				LVI.pszText = lpName;
				LVI.iItem = lIndex;
				ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
				ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), lIndex, 1, lpDFile);
			}
			DragFinish(hDrop);
			return TRUE;
		}
		case WM_NOTIFY:
		{
			LPNMHDR pNM = (LPNMHDR)lParam;
			switch (wParam)
			{
				case IDC_LVFILES:
					switch (pNM->code)
					{
						case LVN_ENDLABELEDIT:
						{
							LV_DISPINFO *pLVDI = (LV_DISPINFO *)lParam;
							if (pLVDI->item.pszText)
							{
								ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), pLVDI->item.iItem,
									0, pLVDI->item.pszText);
							}
							break;
						}
						case LVN_KEYDOWN:
						{
							LV_KEYDOWN *pLVKD = (LV_KEYDOWN *)lParam;
							if (pLVKD->wVKey == VK_DELETE)
							{
								PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTNREM, 0), 0);
							}
							break;
						}
						case NM_DBLCLK:
							PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTNSEL, 0), 0);
							break;
					}
					return TRUE;
				default:
					//
					break;
			}
			return TRUE;
		}
		case WM_TIMER:
		{
			if (wParam == 1)
			{
				BOOL bCtlsFlag1 = (pEngine->m_lpFileName != 0);
				BOOL bCtlsFlag2 = (ListView_GetSelectedCount(GetDlgItem(hWnd, IDC_LVFILES)) != 0);
				BOOL bCtlsFlag3 = (ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES)) != 0);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNACF), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNREM), bCtlsFlag2);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNCA), bCtlsFlag3);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNMU), bCtlsFlag2);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNMD), bCtlsFlag2);
				EnableWindow(GetDlgItem(hWnd, IDC_BTNSEL), bCtlsFlag2);
			}
			return TRUE;
		}
		case WM_CLOSE:
		{
			ULONG i = 0, lItemCnt = 0;
			WCHAR lpName[128] = {}, lpPath[MAX_PATH] = {};
			for (; (i < APP_MAX_STRINGS) && (pFavorites[i]); i++)
			{
				delete[] pFavorites[i]->lpPath;
				delete[] pFavorites[i]->lpDisplayName;
				delete pFavorites[i];
				pFavorites[i] = 0;
			}
			lItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
			for (i = 0; i < lItemCnt; i++)
			{
				ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpPath, MAX_PATH);
				pFavorites[i] = new FAVFILE;
				pFavorites[i]->lpPath = new WCHAR[MAX_PATH];
				wcscpy(pFavorites[i]->lpPath, lpPath);
				ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpName, 128);
				pFavorites[i]->lpDisplayName = new WCHAR[128];
				wcscpy(pFavorites[i]->lpDisplayName, lpName);
			}
			KillTimer(hWnd, 1);
			EndDialog(hWnd, 0);
			return TRUE;
		}
	}
	return FALSE;
}