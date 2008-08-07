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

INT_PTR CALLBACK FavoritesDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			//Инициализация диалога
			//--------------------------------------------------------------------
			ULONG i = 0;
			LV_COLUMN LVC = { 0 };
			LV_ITEM LVI = { 0 };
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
				LVI.pszText = pFavorites[i]->lpwDisplayName;
				LVI.iItem = i;
				ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
				ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1,
					pFavorites[i]->lpwPath);
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
					LV_ITEM LVI = { 0 };
					SHELLFLAGSTATE SFS = { 0 };
					WCHAR lpwName[128] = { 0 };
					lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
					SP_ExtractName(pEngine->m_lpwFileName, lpwName);
					SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
					if (!SFS.fShowExtensions)
					{
						SP_ExtractLeftPart(lpwName, lpwName, '.');
					}
					LVI.mask = LVIF_TEXT;
					LVI.pszText = lpwName;
					LVI.iItem = lIndex;
					ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
					ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), lIndex, 1, pEngine->m_lpwFileName);
					break;
				}
				case IDC_BTNADD:
				{
					WCHAR lpwODFile[APP_OD_MS_MAX_FILE] = { 0 };
					WCHAR lpwODFilter[APP_OD_MAX_FILTER] = { 0 };
					WCHAR lpwExt[64] = { 0 };
					LPWSTR pFiles[FC_MAX_FILES];
					ULONG i, lIndex, lODFileCnt = 0;
					LV_ITEM LVI = { 0 };
					SHELLFLAGSTATE SFS = { 0 };
					WCHAR lpwName[128] = { 0 };
					ZeroMemory(pFiles, sizeof(pFiles));
					wcscpy(lpwODFilter, L"Media files (all types)|");
					for (i = 0; i < APP_SFT_UBOUND; i++)
					{
						if (wcslen(APP_SFT[i]) && (SFT_IsCategory(APP_SFT[i]) == SFTC_NONE))
						{
							swprintf(lpwExt, L"*.%s;", APP_SFT[i]);
							wcscat(lpwODFilter, lpwExt);
						}
					}
					for (i = 0; i < APP_SFT_UBOUND; i++)
					{
						if (wcslen(APP_SFT[i]))
						{
							switch(SFT_IsCategory(APP_SFT[i]))
							{
								case SFTC_NONE:
									swprintf(lpwExt, L"*.%s;", APP_SFT[i]);
									wcscat(lpwODFilter, lpwExt);
									break;
								case SFTC_VIDEO:
									wcscat(lpwODFilter, L"|Video file|");
									break;
								case SFTC_AUDIO:
									wcscat(lpwODFilter, L"|Audio file|");
									break;
								case SFTC_MIDI:
									wcscat(lpwODFilter, L"|MIDI file|");
									break;
								case SFTC_IMAGE:
									wcscat(lpwODFilter, L"|Image file|");
									break;
								case SFTC_PLAYLIST:
									wcscat(lpwODFilter, L"|Playlist file|");
									break;
								case SFTC_OTHER:
									wcscat(lpwODFilter, L"|Other|");
									break;
							}
						}
					}
					wcscat(lpwODFilter, L"|Any file|*.*;||");
					for (i = 0; i < (APP_OD_MAX_FILTER - 1); i++)
					{
						if (lpwODFilter[i] == '|')
						{
							if (lpwODFilter[i + 1] != '|')
							{
								lpwODFilter[i] = '\0';
							}
							else break;
						}
					}
					if (GetOpenDialog(hAppInstance, hWnd, L"Add File(s)", lpwODFile, APP_OD_MS_MAX_FILE - 1,
						lpwODFilter, 1, TRUE))
					{
						for (i = 0; i < (APP_OD_MS_MAX_FILE - 1); i++)
						{
							if ((lpwODFile[i] == '\0') && (lpwODFile[i + 1] != '\0'))
							{
								lpwODFile[i] = '|';
							}
						}
						lODFileCnt = SP_Split(lpwODFile, &pFiles[0], '|', FC_MAX_FILES);
						if (lODFileCnt > 1)
						{
							WCHAR lpwDir[MAX_PATH] = { 0 }, lpwFile[MAX_PATH] = { 0 };
							wcscpy(lpwDir, pFiles[0]);
							SP_AddDirSep(lpwDir, lpwDir);
							for (i = 1; i < lODFileCnt; i++)
							{
								wcscpy(lpwFile, lpwDir);
								wcscat(lpwFile, pFiles[i]);
								lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
								wcscpy(lpwName, pFiles[i]);
								SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
								if (!SFS.fShowExtensions)
								{
									SP_ExtractLeftPart(lpwName, lpwName, '.');
								}
								LVI.mask = LVIF_TEXT;
								LVI.pszText = lpwName;
								LVI.iItem = lIndex;
								ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
								ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), lIndex, 1, lpwFile);
							}
							for (i = 0; i < lODFileCnt; i++)
								delete[] pFiles[i];
						}
						else
						{
							lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
							SP_ExtractName(pFiles[0], lpwName);
							SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
							if (!SFS.fShowExtensions)
							{
								SP_ExtractLeftPart(lpwName, lpwName, '.');
							}
							LVI.mask = LVIF_TEXT;
							LVI.pszText = lpwName;
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
					WCHAR lpwName1[128] = { 0 }, lpwName2[128] = { 0 };
					WCHAR lpwPath1[MAX_PATH] = { 0 }, lpwPath2[MAX_PATH] = { 0 };
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
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 0, lpwName1, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 1, lpwPath1, MAX_PATH);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpwName2, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpwPath2, MAX_PATH);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 0, lpwName2);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i - 1, 1, lpwPath2);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i - 1, LVIS_SELECTED, LVIS_SELECTED);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpwName1);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpwPath1);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, 0, LVIS_SELECTED);
						}
					}
					break;
				}
				case IDC_BTNMD:
				{
					ULONG lItemCnt;
					WCHAR lpwName1[128] = { 0 }, lpwName2[128] = { 0 };
					WCHAR lpwPath1[MAX_PATH] = { 0 }, lpwPath2[MAX_PATH] = { 0 };
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
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 0, lpwName1, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 1, lpwPath1, MAX_PATH);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpwName2, 128);
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpwPath2, MAX_PATH);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 0, lpwName2);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i + 1, 1, lpwPath2);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i + 1, LVIS_SELECTED, LVIS_SELECTED);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpwName1);
							ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpwPath1);
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, 0, LVIS_SELECTED);
						}
					}
					break;
				}
				case IDC_BTNSEL:
				{
					ULONG i = 0, lItemCnt = 0, lFileCnt = 0;
					WCHAR lpwPath[MAX_PATH] = { 0 };
					WCHAR lpwExt[64] = { 0 };
					lItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
					pFileCollection->Clear();
					for (; i < lItemCnt; i++)
					{
						if (ListView_GetItemState(GetDlgItem(hWnd, IDC_LVFILES), i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpwPath, MAX_PATH);
							SP_ExtractRightPart(lpwPath, lpwExt, '.');
							if (SFT_IsMemberOfCategory(lpwExt, SFTC_PLAYLIST))
							{
								lFileCnt += LoadPlaylist(lpwPath);
							}
							else if (IsDirectory(lpwPath))
							{
								lFileCnt += LoadDirectory(lpwPath);
							}
							else
							{
								pFileCollection->AppendFile(lpwPath);
								lFileCnt++;
							}
						}
					}
					if (lFileCnt)
					{
						InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
					}
					else
					{
						if (pEngine->m_lpwFileName) CloseTrack();
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
			WCHAR lpwDFile[MAX_PATH] = { 0 };
			ULONG i = 0, lIndex, lDrFileCnt = 0;
			LV_ITEM LVI = { 0 };
			SHELLFLAGSTATE SFS = { 0 };
			WCHAR lpwName[128] = { 0 };
			HDROP hDrop = (HDROP)wParam;
			lDrFileCnt = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);
			for (; i < lDrFileCnt; i++)
			{
				DragQueryFile(hDrop, i, lpwDFile, MAX_PATH);
				lIndex = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
				if (IsDirectory(lpwDFile))
				{
					SP_RemDirSep(lpwDFile, lpwDFile);
					SP_ExtractName(lpwDFile, lpwName);
				}
				else
				{
					SP_ExtractName(lpwDFile, lpwName);
					SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
					if (!SFS.fShowExtensions)
					{
						SP_ExtractLeftPart(lpwName, lpwName, '.');
					}
				}
				LVI.mask = LVIF_TEXT;
				LVI.pszText = lpwName;
				LVI.iItem = lIndex;
				ListView_InsertItem(GetDlgItem(hWnd, IDC_LVFILES), &LVI);
				ListView_SetItemText(GetDlgItem(hWnd, IDC_LVFILES), lIndex, 1, lpwDFile);
			}
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
				BOOL bCtlsFlag1 = (pEngine->m_lpwFileName != 0);
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
			WCHAR lpwName[128] = { 0 }, lpwPath[MAX_PATH] = { 0 };
			for (; (i < APP_MAX_STRINGS) && (pFavorites[i]); i++)
			{
				delete[] pFavorites[i]->lpwPath;
				delete[] pFavorites[i]->lpwDisplayName;
				delete pFavorites[i];
				pFavorites[i] = 0;
			}
			lItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LVFILES));
			for (i = 0; i < lItemCnt; i++)
			{
				ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 1, lpwPath, MAX_PATH);
				pFavorites[i] = new FAVFILE;
				pFavorites[i]->lpwPath = new WCHAR[MAX_PATH];
				wcscpy(pFavorites[i]->lpwPath, lpwPath);
				ListView_GetItemText(GetDlgItem(hWnd, IDC_LVFILES), i, 0, lpwName, 128);
				pFavorites[i]->lpwDisplayName = new WCHAR[128];
				wcscpy(pFavorites[i]->lpwDisplayName, lpwName);
			}
			KillTimer(hWnd, 1);
			EndDialog(hWnd, 0);
			return TRUE;
		}
	}
	return FALSE;
}