#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "common.h"
#include "strparser.h"
#include "engine.h"
#include "filecollection.h"
#include "ebmenu.h"
#include "eblistbox.h"
#include "easybar.h"
#include "playlistdlg.h"

extern HINSTANCE hAppInstance;
extern PAUSEINFO PI;
extern HWND hMainWnd;
extern HWND hPlaylistWnd;

static CEBMenu *pEBMenuPL = new CEBMenu;
static CEBListBox *pEBListBox = new CEBListBox;

HWND hFindTextWnd;
static FINDREPLACE FR;
static int intOldIndex;

INT_PTR CALLBACK PlaylistDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_FINDMSG)
	{
		LPFINDREPLACE pFR = (LPFINDREPLACE)lParam;
		LPWSTR lpwLocale;
		LCID lcSD = GetSystemDefaultLCID();
		UINT uLocNameSize = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, 0, 0);
		lpwLocale = new WCHAR[uLocNameSize];
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, lpwLocale, uLocNameSize);
		if  ((pFR->Flags & FR_FINDNEXT) == FR_FINDNEXT)
		{
			WCHAR lpwText[160] = { 0 };
			int intItemsCnt = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCOUNT, 0, 0);
Find_Begin:
			for (UINT i = intOldIndex; i < (UINT)intItemsCnt; i++)
			{
				SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETTEXT, i, (LPARAM)lpwText);
				if (SP_Find(lpwText, pFR->lpstrFindWhat, 0, ((pFR->Flags & FR_MATCHCASE) == FR_MATCHCASE),
					lpwLocale) != -1)
				{
					SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_SETCURSEL, i, 0);
					if (i < (UINT)(intItemsCnt - 1))
						intOldIndex = ++i;
					else
						intOldIndex = 0;
					break;
				}
				if (i == (UINT)(intItemsCnt - 1))
				{
					if (intOldIndex)
					{
						intOldIndex = 0;
						goto Find_Begin;
					}
					else MessageBeep(-1);
				}
			}
		}
		else if ((pFR->Flags & FR_DIALOGTERM) == FR_DIALOGTERM)
		{
			delete[] pFR->lpstrFindWhat;
			hFindTextWnd = 0;
		}
		delete[] lpwLocale;
		return TRUE;
	}
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			if (!dwNoOwnerDrawMenu)
			{
				pEBMenuPL->hBmpCheck = (HBITMAP)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_CHECKMARK),
					IMAGE_BITMAP, 16, 16, 0);
				pEBMenuPL->hBmpRadioCheck = (HBITMAP)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_RADIOCHECKMARK),
					IMAGE_BITMAP, 16, 16, 0);
				pEBMenuPL->InitEBMenu(hWnd, FALSE);
			}
			pEBListBox->hBmpMark = (HBITMAP)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_ACTIVEITEM), IMAGE_BITMAP, 16, 16, 0);
			pEBListBox->InitEBListBox(GetDlgItem(hWnd, IDC_LSTPL));
			UpdatePlaylistColors();
			return TRUE;
		case WM_INITMENUPOPUP:
			if (!HIWORD(lParam))
			{
				int intItemsCnt = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCOUNT, 0, 0);
				int intCurSel = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCURSEL, 0, 0);
				HMENU hPlaylistMenu = GetMenu(hWnd);
				EnableMenuItem(hPlaylistMenu, IDM_SELECTION_MOVEUP,
					(((intItemsCnt != LB_ERR) && intItemsCnt) && ((intCurSel != LB_ERR) && (intCurSel > 0)))?MF_ENABLED:MF_DISABLED
					| MF_GRAYED);
				EnableMenuItem(hPlaylistMenu, IDM_SELECTION_MOVEDOWN,
					(((intItemsCnt != LB_ERR) && intItemsCnt) && ((intCurSel != LB_ERR) && (intCurSel < (intItemsCnt - 1))))?MF_ENABLED:
					MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hPlaylistMenu, IDM_SELECTION_CROP,
					(((intItemsCnt != LB_ERR) && (intItemsCnt > 1)) && (intCurSel != LB_ERR))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hPlaylistMenu, IDM_SELECTION_DELETE,
					(((intItemsCnt != LB_ERR) && intItemsCnt) && (intCurSel != LB_ERR))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hPlaylistMenu, IDM_MISC_FIND,
					((intItemsCnt != LB_ERR) && intItemsCnt)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hPlaylistMenu, IDM_SORT_SORTBYTITLE,
					((intItemsCnt != LB_ERR) && (intItemsCnt > 1))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hPlaylistMenu, IDM_SORT_REVERSESORTBYTITLE,
					((intItemsCnt != LB_ERR) && (intItemsCnt > 1))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hPlaylistMenu, IDM_MISC_CLEARPLAYLIST,
					((intItemsCnt != LB_ERR) && intItemsCnt)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
			}
			return TRUE;
		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT pMIS = (LPMEASUREITEMSTRUCT)lParam;
			switch (pMIS->CtlType)
			{
				case ODT_MENU:
					if (!dwNoOwnerDrawMenu)
					{
						return pEBMenuPL->MeasureItem(wParam, lParam);
					}
					break;
				case ODT_LISTBOX:
					return pEBListBox->MeasureItem(wParam, lParam);
			}
			return FALSE;
		}
		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
			switch (pDIS->CtlType)
			{
				case ODT_MENU:
					if (!dwNoOwnerDrawMenu)
					{
						return pEBMenuPL->DrawItem(wParam, lParam);
					}
					break;
				case ODT_LISTBOX:
					return pEBListBox->DrawItem(wParam, lParam);
			}
			return FALSE;
		}
		case WM_FCNOTIFICATION:
			switch (HIWORD(wParam))
			{
				case FCN_FILEADDED:
				{
					WCHAR lpwText[160] = { 0 };
					WCHAR lpwTime[32] = { 0 };
					LPPLITEMDESC pPLID = 0;
					pFileCollection->GetUserData((LPCWSTR)lParam, 0, FCF_BYFILENAME, (LONG_PTR &)pPLID);
					if (pPLID)
					{
						wcscpy(lpwText, pPLID->lpwTitle);
						if (((pPLID->uDuration / 1000) / 3600)) //Больше часа
						{
							swprintf(lpwTime, L"\t%02i:%02i:%02i",(pPLID->uDuration / 1000) / 3600, ((pPLID->uDuration / 1000)
								/ 60) % 60, ((pPLID->uDuration / 1000) % 60));
						}
						else
						{
							swprintf(lpwTime, L"\t%02i:%02i",((pPLID->uDuration / 1000) / 60) % 60, ((pPLID->uDuration / 1000)
								% 60));
						}
						wcscat(lpwText, lpwTime);
					}
					else
					{
						SP_ExtractName((LPCWSTR)lParam, lpwText);
						SP_ExtractLeftPart(lpwText, lpwText, '.');
						wcscat(lpwText, L"\t00:00");
					}
					SendDlgItemMessage(hWnd, IDC_LSTPL, LB_ADDSTRING, 0, (LPARAM)lpwText);
					break;
				}
				case FCN_FILEDELETED:
					SendDlgItemMessage(hWnd, IDC_LSTPL, LB_DELETESTRING, lParam, 0);
					break;
				case FCN_CURFILECHANGED:
					//
					break;
				case FCN_COLCLEARED:
					SendDlgItemMessage(hWnd, IDC_LSTPL, LB_RESETCONTENT, 0, 0);
					break;
			}
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LSTPL:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						int intCurSel = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCURSEL, 0, 0);
						ENGINESTATE eOldState = pEngine->GetState();
						InitTrack(FCF_BYINDEX, intCurSel);
						switch (eOldState)
						{
							case E_STATE_STOPPED:
								//
								break;
							case E_STATE_PAUSED:
								PI.psSource = PS_OTHER;
								pEngine->Pause();
								break;
							case E_STATE_PLAYING:
								pEngine->Play();
								break;
						}
					}
					break;
				case IDM_FILE_OPENFILES:
				case IDM_OD_OPENDIRECTORY:
				case IDM_OD_OPENDIRECTORY_II:
				case IDM_FILE_ADDFILES:
				case IDM_AD_ADDDIRECTORY:
				case IDM_AD_ADDDIRECTORY_II:
				case IDM_FILE_ADDURL:
					PostMessage(hMainWnd, uMsg, wParam, lParam);
					break;
				case IDM_SELECTION_MOVEUP:
				case IDM_SELECTION_MOVEDOWN:
				{
					BOOL bUp = (LOWORD(wParam) == IDM_SELECTION_MOVEUP);
					WCHAR lpwText[160] = { 0 };
					int intNewIndex;
					int intCurSel = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCURSEL, 0, 0);
					BOOL bHighlighted = pEBListBox->IsItemHighlighted(intCurSel);
					SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_GETTEXT, intCurSel, (LPARAM)lpwText);
					SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_DELETESTRING, intCurSel, 0);
					intNewIndex = (bUp)?intCurSel - 1:intCurSel + 1;
					pFileCollection->MoveFile(0, intCurSel, intNewIndex, FCF_BYINDEX);
					SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_INSERTSTRING, intNewIndex, (LPARAM)lpwText);
					if (bHighlighted)
						pEBListBox->HighlightItem((UINT)intNewIndex);
					SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_SETCURSEL, intNewIndex, 0);
					break;
				}
				case IDM_SELECTION_CROP:
				{
					UINT i;
					int intCurSel = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCURSEL, 0, 0);
					int intItemsCnt = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCOUNT, 0, 0);
					for (i = (intItemsCnt - 1); i > (UINT)intCurSel; i--)
						DeleteItem(i);
					for (i = 0; i < (UINT)intCurSel; i++)
						DeleteItem(0);
					break;
				}
				case IDM_SELECTION_DELETE:
				{
					int intCurSel = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCURSEL, 0, 0);
					DeleteItem(intCurSel);
					break;
				}
				case IDM_MISC_FIND:
					ZeroMemory(&FR, sizeof(FINDREPLACE));
					FR.lStructSize = sizeof(FINDREPLACE);
					FR.hwndOwner = hWnd;
					FR.Flags = FR_HIDEUPDOWN | FR_HIDEWHOLEWORD;
					FR.lpstrFindWhat = new WCHAR[128];
					FR.wFindWhatLen = 128;
					ZeroMemory(FR.lpstrFindWhat, 128 * sizeof(WCHAR));
					intOldIndex = 0;
					hFindTextWnd = FindText(&FR);
					break;
				case IDM_SORT_SORTBYTITLE:
				case IDM_SORT_REVERSESORTBYTITLE:
				{
					BOOL bReverse = (LOWORD(wParam) == IDM_SORT_REVERSESORTBYTITLE);
					UINT i;
					WCHAR lpwFile[MAX_PATH] = { 0 };
					int intItemsCnt = SendDlgItemMessage(hWnd, IDC_LSTPL, LB_GETCOUNT, 0, 0);
					for (i = 0; i < (UINT)intItemsCnt; i++)
					{
						pFileCollection->GetFile(lpwFile, i, FCF_BYINDEX);
						pEBListBox->SetItemTag(i, (LPCBYTE)lpwFile, MAX_PATH);
					}
					pEBListBox->Sort(bReverse);
					pFileCollection->Clear(FALSE);
					for (i = 0; i < (UINT)intItemsCnt; i++)
					{
						pEBListBox->GetItemTag(i, (LPBYTE)lpwFile);
						pFileCollection->AppendFile(lpwFile, FALSE);
						pEBListBox->DeleteItemTag(i);
					}
					pEBListBox->GetItemTag(pEBListBox->GetHighlightedItemIndex(), (LPBYTE)lpwFile);
					pFileCollection->SetRecentFile(lpwFile);
					break;
				}
				case IDM_MISC_CLEARPLAYLIST:
					LPPLITEMDESC pPLID;
					if (pEngine->m_lpwFileName)
						SendMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSE, 0), 0);
					for (UINT i = 0; i < (UINT)pFileCollection->FileCount(); i++)
					{
						pPLID = 0;							
						pFileCollection->GetUserData(0, i, FCF_BYINDEX, (LONG_PTR &)pPLID);
						if (pPLID)
							delete pPLID;
					}
					pFileCollection->Clear();
					break;
			}
			return TRUE;
		case WM_DROPFILES:
			SendMessage(hMainWnd, uMsg, wParam, lParam);
			return TRUE;
		case WM_MOVING:
			StickyWindow(hWnd, (LPRECT)lParam);
			return TRUE;
		case WM_WINDOWPOSCHANGED:
		{
			RECT RCC = { 0 };
			GetClientRect(hWnd, &RCC);
			SetWindowPos(GetDlgItem(hWnd, IDC_LSTPL), 0, 0, 0, RCC.right, RCC.bottom, SWP_NOMOVE | SWP_NOZORDER
				| SWP_NOSENDCHANGING);
			return TRUE;
		}
		case WM_DESTROY:
		{
			WINDOWPLACEMENT WP = { 0 };
			GetWindowPlacement(hWnd, &WP);
			if (WP.showCmd == SW_NORMAL)
				GetWindowRect(hWnd, &rcPlaylistPos);
			return TRUE;
		}
	}
	return FALSE;
}

void SetActiveItem(LPCWSTR lpwFileName)
{
	WCHAR lpwText[160] = { 0 };
	WCHAR lpwTime[32] = { 0 };
	LPPLITEMDESC pPLID = 0;
	int intIndex = pFileCollection->GetFileIndex(lpwFileName, FCF_BYFILENAME);
	LPBYTE pData = new BYTE[MAX_PATH * sizeof(WCHAR)];
	ZeroMemory(pData, MAX_PATH * sizeof(WCHAR));
	if (pEBListBox->GetItemTag(intIndex, pData))
		pData = pData;
	SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_DELETESTRING, intIndex, 0);
	pFileCollection->GetUserData(0, intIndex, FCF_BYINDEX, (LONG_PTR &)pPLID);
	wcscpy(lpwText, pPLID->lpwTitle);
	if (((pPLID->uDuration / 1000) / 3600))
	{
		swprintf(lpwTime, L"\t%02i:%02i:%02i",(pPLID->uDuration / 1000) / 3600, ((pPLID->uDuration / 1000)
			/ 60) % 60, ((pPLID->uDuration / 1000) % 60));
	}
	else
	{
		swprintf(lpwTime, L"\t%02i:%02i",((pPLID->uDuration / 1000) / 60) % 60, ((pPLID->uDuration / 1000)
			% 60));
	}
	wcscat(lpwText, lpwTime);
	SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_INSERTSTRING, intIndex, (LPARAM)lpwText);
	pEBListBox->HighlightItem((UINT)intIndex);
	SendDlgItemMessage(hPlaylistWnd, IDC_LSTPL, LB_SETCURSEL, intIndex, 0);
}

void UpdatePlaylistColors()
{
	if (dwUseSystemColors)
	{
		if (!dwNoOwnerDrawMenu)
		{
			pEBMenuPL->crFontColorOne = GetSysColor(COLOR_HIGHLIGHTTEXT);
			pEBMenuPL->crFontColorTwo = Blend(GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_HIGHLIGHT), 0.6);
			pEBMenuPL->crFontColorThree = GetSysColor(COLOR_MENUTEXT);
			pEBMenuPL->crBkColorOne = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.2);
			pEBMenuPL->crBkColorTwo = GetSysColor(COLOR_MENU);
			pEBMenuPL->crBkColorThree = GetMenuBarColor();
			pEBMenuPL->crSelColorOne = GetSysColor(COLOR_HIGHLIGHT);
			pEBMenuPL->crSelColorTwo = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1);
			pEBMenuPL->crBrColorOne = Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1);
			pEBMenuPL->crBrColorTwo = Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1);
			pEBMenuPL->UpdateMenuBar();
			DrawMenuBar(hPlaylistWnd);
		}
		pEBListBox->crFontColorOne = GetSysColor(COLOR_HIGHLIGHTTEXT);
		pEBListBox->crFontColorTwo = Blend(GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_HIGHLIGHT), 0.6);
		pEBListBox->crFontColorThree = GetSysColor(COLOR_BTNTEXT);
		pEBListBox->crFontColorFour = Blend(GetSysColor(COLOR_HIGHLIGHTTEXT), GetSysColor(COLOR_BTNTEXT), 0.7);
		pEBListBox->crBkColorOne = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.2);
		pEBListBox->crBkColorTwo = GetSysColor(COLOR_WINDOW);
		pEBListBox->crBkColorThree = Blend(pEBListBox->crBkColorOne, GetSysColor(COLOR_WINDOW), 0.5);
		pEBListBox->crSelColorOne = GetSysColor(COLOR_HIGHLIGHT);
		pEBListBox->crSelColorTwo = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1);
		pEBListBox->crBrColorOne = Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1);
		pEBListBox->crBrColorTwo = Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1);
		pEBListBox->Refresh();
	}
	else
	{
		if (!dwNoOwnerDrawMenu)
		{
			pEBMenuPL->crFontColorOne = dwTextColor;
			pEBMenuPL->crFontColorTwo = dwTextShadowColor;
			pEBMenuPL->crFontColorThree = GetSysColor(COLOR_MENUTEXT);
			pEBMenuPL->crBkColorOne = dwBackgroundColor;
			pEBMenuPL->crBkColorTwo = GetSysColor(COLOR_MENU);
			pEBMenuPL->crBkColorThree = GetMenuBarColor();
			pEBMenuPL->crSelColorOne = dwGradientColor1;
			pEBMenuPL->crSelColorTwo = dwGradientColor2;
			pEBMenuPL->crBrColorOne = dwBorderColor1;
			pEBMenuPL->crBrColorTwo = dwBorderColor2;
			pEBMenuPL->UpdateMenuBar();
			DrawMenuBar(hPlaylistWnd);
		}
		pEBListBox->crFontColorOne = dwTextColor;
		pEBListBox->crFontColorTwo = dwTextShadowColor;
		pEBListBox->crFontColorThree = GetSysColor(COLOR_BTNTEXT);
		pEBListBox->crFontColorFour = dwActiveItemTextColor;
		pEBListBox->crBkColorOne = dwBackgroundColor;
		pEBListBox->crBkColorTwo = GetSysColor(COLOR_WINDOW);
		pEBListBox->crBkColorThree = Blend(dwBackgroundColor, GetSysColor(COLOR_WINDOW), 0.5);
		pEBListBox->crSelColorOne = dwGradientColor1;
		pEBListBox->crSelColorTwo = dwGradientColor2;
		pEBListBox->crBrColorOne = dwBorderColor1;
		pEBListBox->crBrColorTwo = dwBorderColor2;
		pEBListBox->Refresh();
	}
}

void DeleteItem(int intItemIndex)
{
	LPPLITEMDESC pPLID = 0;
	if (pEBListBox->IsItemHighlighted(intItemIndex) && pEngine->m_lpwFileName)
		SendMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSE, 0), 0);
	pFileCollection->GetUserData(0, intItemIndex, FCF_BYINDEX, (LONG_PTR &)pPLID);
	if (pPLID)
		delete pPLID;
	pFileCollection->DeleteFile(0, intItemIndex, FCF_BYINDEX);
}
