/////////////////////////////////////////////////////////////////////////////
//		Проект: EasyBar - media player
//		Автор(ы): Борис Воронцов и участники проекта
//		Последнее обновление: 13.02.2009
/////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT	0x0501
#define WINVER			0x0501

#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>
#include <stdio.h>
#include <htmlhelp.h>
#include <math.h>

#include <uxtheme.h>

#include "resource.h"
#include "urldlg.h"
#include "favoritesdlg.h"
#include "aboutdlg.h"
#include "associationsdlg.h"
#include "filepropdlg.h"
#include "ppgeneraldlg.h"
#include "ppcontentdlg.h"
#include "ppfiltersdlg.h"
#include "playlistdlg.h"
#include "videodlg.h"
#include "fsvideomodedlg.h"
#include "colorsdlg.h"
#include "opacitydlg.h"
#include "timeintervalsdlg.h"
#include "extfltdlg.h"
#include "tooltips.h"
#include "filecollection.h"
#include "playlist.h"
#include "strparser.h"
#include "common.h"
#include "trayicon.h"
#include "settings.h"
#include "ebmenu.h"
#include "ebbutton.h"
#include "ebframe.h"
#include "ebdisplay.h"
#include "ebslider.h"
#include "labelex.h"
#include "engine.h"
#include "videomode.h"
#include "easybar.h"

extern HWND hFindTextWnd;

static HANDLE hMutex;

HINSTANCE hAppInstance;
WCHAR lpwAppPath[MAX_PATH] = { 0 };
WCHAR lpwAppVersion[20] = { 0 };
WCHAR lpwAppVersionMM[10] = { 0 };

OSVERSIONINFO OSVI = { 0 };

HWND hMainWnd = 0;
HWND hPlaylistWnd = 0;
HWND hVideoWnd = 0;

WCHAR lpwStdWndTitle[64] = { 0 };
WCHAR lpwCurFileTitle[128] = { 0 };

WCHAR lpwPlPath[MAX_PATH] = { 0 };

static BOOL bSeekFlag = FALSE; //Временно отключает обновление полосы поиска

CURRENTINFO CI = { 0 };
PAUSEINFO PI = { PS_OTHER, 0 };

static CEBMenu *pEBMenuMain = new CEBMenu;
static CToolTips *pToolTipsMain = new CToolTips;
CFileCollection *pFileCollection = new CFileCollection;
CDirectShow *pEngine = new CDirectShow;
CVideoMode *pVideoMode = new CVideoMode;

VWDATA VWD = { 0 };

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpwCmdLine, int nShowCmd)
{
	ULONG lCmdLnLen = 0, lPlFileCnt = 0, lCmdLnFileCnt = 0;
	BOOL bCmdLnAdd = FALSE;
	HACCEL hMainAccel, hPLAccel;
	MSG Msg = { 0 };
	LPITEMIDLIST pIIDL = 0;
	LPEXCEPTION_POINTERS pGEP = 0;
	OSVI.dwOSVersionInfoSize = sizeof(OSVI);
	GetVersionEx(&OSVI);
	//На всякий случай...
	if (OSVI.dwMajorVersion < 5)
		return 0;
#ifndef _DEBUG
	__try
	{
#endif
		GetSettings();
		if (!dwMultipleInstances)
		{
			hMutex = CreateMutex(0, TRUE, APP_NAME);
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				HWND hAppWnd = FindWindow(APP_MAIN_WND_CLASS, 0);
				if (hAppWnd)
				{
					lCmdLnLen = wcslen(lpwCmdLine);
					if (lCmdLnLen)
					{
						COPYDATASTRUCT CDS = { 0 };
						CDS.cbData = lCmdLnLen * sizeof(WCHAR);
						CDS.lpData = lpwCmdLine;
						SendMessage(hAppWnd, WM_COPYDATA, 0, (LPARAM)&CDS);
						return 0;
					}
					else return 0;
				}
			}
		}
		hAppInstance = hInstance;
		InitCommonControls();
		AdjustPrivilege(SE_SHUTDOWN_NAME);
		InitEBButton(hAppInstance);
		InitEBFrame(hAppInstance);
		InitEBDisplay(hAppInstance);
		InitEBSlider(hAppInstance);
		InitLabelEx(hAppInstance);
		GetAppPath(hAppInstance, lpwAppPath, MAX_PATH, FALSE);
		ReadFileVersion(lpwAppPath, lpwAppVersion, sizeof(lpwAppVersion), RFV_MAJOR | RFV_MINOR | RFV_RELEASE);
		ReadFileVersion(lpwAppPath, lpwAppVersionMM, sizeof(lpwAppVersionMM), RFV_MAJOR | RFV_MINOR);
		swprintf(lpwStdWndTitle, L"%s %s", APP_NAME, lpwAppVersionMM);
		hMainAccel = LoadAccelerators(hAppInstance, MAKEINTRESOURCE(IDR_MAINACCEL));
		hPLAccel = LoadAccelerators(hAppInstance, MAKEINTRESOURCE(IDR_PLACCEL));
		InitMainWnd();
		hPlaylistWnd = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PLAYLIST), hMainWnd, PlaylistDlgProc, 0);
		UpdateCFTitle(0);
		if (!dwNoOwnerDrawMenu)
		{
			pEBMenuMain->hBmpCheck = (HBITMAP)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_CHECKMARK),
				IMAGE_BITMAP, 16, 16, 0);
			pEBMenuMain->hBmpRadioCheck = (HBITMAP)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_RADIOCHECKMARK),
				IMAGE_BITMAP, 16, 16, 0);
			pEBMenuMain->InitEBMenu(hMainWnd, FALSE);
		}
		pToolTipsMain->m_hInstance = hAppInstance;
		pToolTipsMain->m_hOwner = hMainWnd;
		pToolTipsMain->Initialize();
		UpdateToolTips();
		pFileCollection->SetCallbackWnd(hPlaylistWnd);
		pEngine->m_lpwFileName = 0;
		pEngine->m_lpwAppName = APP_NAME;
		pEngine->m_hAppWnd = hMainWnd;
		pEngine->UpdateDMOArray(DMO_CATEGORY_AUDIO_EFFECT);
		ApplySettings();
		SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pIIDL);
		SHGetPathFromIDList(pIIDL, lpwPlPath);
		CoTaskMemFree(pIIDL);
		SP_AddDirSep(lpwPlPath, lpwPlPath);
		wcscat(lpwPlPath, APP_NAME);
		SP_AddDirSep(lpwPlPath, lpwPlPath);
		if (!IsDirectory(lpwPlPath)) CreateDirectory(lpwPlPath, 0);
		wcscat(lpwPlPath, APP_PLAYLIST_FILE);
		if (IsFile(lpwPlPath))
		{
			if (dwRememberPlaylist)
			{
				lPlFileCnt = LoadPlaylist(lpwPlPath);
			}
			else
			{
				DeleteFile(lpwPlPath);
			}
		}
		lCmdLnLen = wcslen(lpwCmdLine);
		if (lCmdLnLen)
		{
			lCmdLnFileCnt = ReadCommandLine(lpwCmdLine, &bCmdLnAdd);
		}
		if (lPlFileCnt && !lCmdLnFileCnt)
		{
			if (dwShuffle)
			{
				InitTrack(FCF_RANDOM);
			}
			else
			{
				if (InitTrack(FCF_BYINDEX, dwSelectedFileIndex) < 0)
				{
					InitTrack(FCF_FIRST);
				}
				else
				{
					if (dwASResumePlayback)
					{
						if (dwSFState != E_STATE_STOPPED)
							SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, dwSFPosition);
						switch (dwSFState)
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
				}
			}
		}
		else if (lPlFileCnt || lCmdLnFileCnt)
		{
			if (!bCmdLnAdd)
			{
				InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
			}
			else
			{
				//if (!pEngine->m_lpwFileName)
				//{
					if (dwShuffle)
					{
						if (InitTrack(FCF_RANDOM) == -1) InitTrack(FCF_RANDOM);
					}
					else
					{
						InitTrack(FCF_RECENT);
					}
				//}
				//else
				//{
				//	pFileCollection->SetRecentFile(pEngine->m_lpwFileName);
				//}
			}
			if (lCmdLnFileCnt && !bCmdLnAdd)
			{
				pEngine->Play();
			}
		}
		ShowWindow(hMainWnd, nShowCmd);
		if (dwPlaylist)
			ShowWindow(hPlaylistWnd, SW_SHOW);
#ifndef _DEBUG
	}
	__except(ExceptionFilter(pGEP = GetExceptionInformation()))
	{
		WCHAR lpwMsg[MAX_PATH] = { 0 };
		swprintf(lpwMsg, L"Unexpected error in the main function!\n\nError code: 0x%08x"
			L"\nError address: 0x%08x", pGEP->ExceptionRecord->ExceptionCode,
			pGEP->ExceptionRecord->ExceptionAddress);
		if (IsDebuggerPresent())
		{
			OutputDebugString(L"\n********************************\n");
			OutputDebugString(lpwMsg);
			OutputDebugString(L"\n********************************\n");
			Sleep(10000);
		}
		else
		{
			MessageBox(0, lpwMsg, APP_NAME, MB_ICONSTOP);
		}
		goto ExitFunction;
	}
#endif
	while (GetMessage(&Msg, 0, 0, 0))
	{
		if (hFindTextWnd)
			if (IsDialogMessage(hFindTextWnd, &Msg)) continue;
		if (TranslateAccelerator(hMainWnd, hMainAccel, &Msg)) continue;
		if (TranslateAccelerator(hPlaylistWnd, hPLAccel, &Msg)) continue;
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
#ifndef _DEBUG
ExitFunction:
#endif
	SaveSettings();
	ReleaseMutex(hMutex);
	SDO(pEBMenuMain);
	SDO(pToolTipsMain);
	SDO(pFileCollection);
	SDO(pEngine);
	SDO(pVideoMode);
	return Msg.wParam;
}

LONG ExceptionFilter(EXCEPTION_POINTERS *pEP)
{
	LONG lResult;
	switch(pEP->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_BREAKPOINT:
			lResult = EXCEPTION_CONTINUE_EXECUTION;
			break;
		default:
			if (pEP->ExceptionRecord->ExceptionFlags == 0)
			{
				lResult = EXCEPTION_CONTINUE_EXECUTION;
			}
			else
			{
				lResult = EXCEPTION_EXECUTE_HANDLER;
			}
			break;
	}
	return lResult;
}

DWORD ReadCommandLine(LPCWSTR lpwCmdLine, LPBOOL pAdd)
{
	WCHAR lpwExt[64] = { 0 };
	LPWSTR *lpwCmdLineArgs = 0;
	ULONG i = 0, lArgsCnt = 0, lFileCnt = 0;
	lpwCmdLineArgs = CommandLineToArgvW(lpwCmdLine, (int *)&lArgsCnt);
	if (lpwCmdLineArgs)
	{
		for (i = 0; i < lArgsCnt; i++)
		{
			if (i == 0)
			{
				if (!*pAdd)
				{
					if (wcscmp(lpwCmdLineArgs[i], APP_CL_KEY_ADD) != 0)
					{
						pFileCollection->Clear();
					}
					else
					{
						*pAdd = TRUE;
						continue;
					}
				}
			}
			SP_ExtractRightPart(lpwCmdLineArgs[i], lpwExt, '.');
			if (SFT_IsMemberOfCategory(lpwExt, SFTC_PLAYLIST))
			{
				lFileCnt += LoadPlaylist(lpwCmdLineArgs[i]);
			}
			else if (IsDirectory(lpwCmdLineArgs[i]))
			{
				lFileCnt += LoadDirectory(lpwCmdLineArgs[i]);
			}
			else
			{
				pFileCollection->AppendFile(lpwCmdLineArgs[i]);
				lFileCnt++;
			}
		}
		for (i = 0; i < (ULONG)lArgsCnt; i++)
			LocalFree(lpwCmdLineArgs[i]);
		LocalFree(lpwCmdLineArgs);
	}
	return lFileCnt;
}

INT_PTR CALLBACK PlayerDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == EBM_INITIALIZED)
	{
		if (lParam != GetCurrentThreadId())
		{
			//
		}
		return TRUE;
	}
	else if (uMsg == EBM_ACTIVATED)
	{
		//----------------------------------------------------------------
		if (!dwNoAutomaticPause)
		{
			if (lParam != GetCurrentThreadId())
			{
				if (pEngine->GetState() == E_STATE_PLAYING)
				{
					PI.psSource = PS_ACTIVATE;
					pEngine->Pause();
				}
			}
		}
		return TRUE;
		//----------------------------------------------------------------
	}
	else if (uMsg == EBM_QUIT)
	{
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	}
	switch (uMsg)
	{
		case WM_INITDIALOG:
			//Инициализация диалога
			//--------------------------------------------------------------------
			SendMessage(hWnd, WM_SETICON, ICON_BIG,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 32, 32, 0));
			SendMessage(hWnd, WM_SETICON, ICON_SMALL,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, 0));
			//Инициализация элементов управления
			//--------------------------------------------------------------------
			SendDlgItemMessage(hWnd, IDC_EBBPF, EBBM_SETBITMAP, 0,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_PREVIOUSFILE), IMAGE_BITMAP, 16, 15, 0));
			SendDlgItemMessage(hWnd, IDC_EBBSB, EBBM_SETBITMAP, 0,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_STEPBACKWARD), IMAGE_BITMAP, 16, 15, 0));
			SendDlgItemMessage(hWnd, IDC_EBBPP, EBBM_SETBITMAP, 0,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_PLAY), IMAGE_BITMAP, 16, 15, 0));
			//Это свойство требуется для экономии ресурсов ЦП и отсутствия эффекта мерцания кнопки
			//Оно поможет в таймере 1 не присваивать по нескольку раз одну и ту же иконку
			SetProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_", (HANDLE)L"play");
			SendDlgItemMessage(hWnd, IDC_EBBSF, EBBM_SETBITMAP, 0,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_STEPFORWARD), IMAGE_BITMAP, 16, 15, 0));
			SendDlgItemMessage(hWnd, IDC_EBBNF, EBBM_SETBITMAP, 0,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_NEXTFILE), IMAGE_BITMAP, 16, 15, 0));
			SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETRANGE, 0, 0);
			//SendDlgItemMessage(hWnd, IDC_EBBMUT, EBBM_SETBITMAP, 0,
			//	(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_SOUND...), IMAGE_BITMAP, 10, 10, 0));
			SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_SETRANGE, 0, 100);
			SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_SETPOS, 0, dwVolume);
			SendDlgItemMessage(hWnd, IDC_EBBBN, EBBM_SETBITMAP, 0,
				(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_BALANCENORMAL), IMAGE_BITMAP, 10, 10, 0));
			SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_SETRANGE, -100, 100);
			SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_SETPOS, 0, (dwBalance - 100));
			//Прочее...
			//--------------------------------------------------------------------
			CI.dwInfoIndex = II_TITLE;
			CI.dwTimer = GetTickCount();
			CI.dwTimeout = 10000;
			SetTimer(hWnd, 1, 200, 0);
			SendMessage(hWnd, WM_TIMER, 1, 0);
			//--------------------------------------------------------------------
			if (dwMultipleInstances)
			{
				PostMessage(HWND_BROADCAST, EBM_INITIALIZED, 0, (LPARAM)GetCurrentThreadId());
			}
			return TRUE;
		case WM_ACTIVATEAPP:
		{
			if (dwOpaqueOnFocus && (dwOpacityLevel < 100))
			{
				BYTE bAlpha1 = (BYTE)(2.55 * dwOpacityLevel), bAlpha2 = 255, i;
				if (wParam)
				{
					if (!dwNoTransitionEffects)
					{
						for (i = bAlpha1; i < bAlpha2; i ++)
						{
							SetLayeredWindowAttributes(hWnd, 0, i, LWA_ALPHA);
							SetLayeredWindowAttributes(hPlaylistWnd, 0, i, LWA_ALPHA);
							/*if (hVideoWnd)
								SetLayeredWindowAttributes(hVideoWnd, 0, i, LWA_ALPHA);*/
							ProcessMessages();
						}
					}
					else
					{
						SetLayeredWindowAttributes(hWnd, 0, bAlpha2, LWA_ALPHA);
					}
				}
				else
				{
					if (!dwNoTransitionEffects)
					{
						for (i = bAlpha2; i > bAlpha1; i --)
						{
							SetLayeredWindowAttributes(hWnd, 0, i, LWA_ALPHA);
							SetLayeredWindowAttributes(hPlaylistWnd, 0, i, LWA_ALPHA);
							/*if (hVideoWnd)
								SetLayeredWindowAttributes(hVideoWnd, 0, i, LWA_ALPHA);*/
							ProcessMessages();
						}
					}
					else
					{
						SetLayeredWindowAttributes(hWnd, 0, bAlpha1, LWA_ALPHA);
					}
				}
			}
			//---------------------------------------------------------------
			if (dwMultipleInstances)
			{
				if (wParam)
				{
					//BOOL bIsEBThread = FALSE;
					//EnumThreadWindows(lParam, EnumThreadWndsProc, (LPARAM)&bIsEBThread);
					//if (bIsEBThread)
					//{
						if (!dwNoAutomaticPause)
						{
							HANDLE hMtx = CreateMutex(0, TRUE, APP_NAME);
							if (GetLastError() == ERROR_ALREADY_EXISTS)
							{
								if (pEngine->GetState() == E_STATE_PAUSED)
								{
									if (PI.psSource == PS_ACTIVATE)
									{
										pEngine->Play();
									}
								}
							}
							ReleaseMutex(hMtx);
						}
					//}
					PostMessage(HWND_BROADCAST, EBM_ACTIVATED, 0, (LPARAM)GetCurrentThreadId());
				}
			}
			return TRUE;
		}
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_VOLUME_DOWN:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_DECREASE, 0), 0);
					break;
				case VK_VOLUME_UP:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_INCREASE, 0), 0);
					break;
			}
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_MEDIA_PREV_TRACK:
					if (((dwShuffle)?(pFileCollection->FileCount() > 1):
						(pFileCollection->IsFileAvailable(FCF_BACKWARD))))
					{
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_PREVIOUSFILE, 0), 0);
					}
					break;
				case VK_MEDIA_PLAY_PAUSE:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_PLAYPAUSE, 0), 0);
					break;
				case VK_MEDIA_STOP:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_STOP, 0), 0);
					break;
				case VK_MEDIA_NEXT_TRACK:
					if (((dwShuffle)?(pFileCollection->FileCount() > 1):
						(pFileCollection->IsFileAvailable(FCF_FORWARD))))
					{
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_NEXTFILE, 0), 0);
					}
					break;
				case VK_VOLUME_MUTE:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_MUTE, 0), 0);
					break;
			}
		case WM_XBUTTONUP:
			switch (HIWORD(wParam))
			{
				case XBUTTON1:
					if (((dwShuffle)?(pFileCollection->FileCount() > 1):
						(pFileCollection->IsFileAvailable(FCF_BACKWARD))))
					{
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_PREVIOUSFILE, 0), 0);
					}
					break;
				case XBUTTON2:
					if (((dwShuffle)?(pFileCollection->FileCount() > 1):
						(pFileCollection->IsFileAvailable(FCF_FORWARD))))
					{
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_NEXTFILE, 0), 0);
					}
					break;
			}
			break;
		case WM_MOUSEWHEEL:
			if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
			{
				PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_DECREASE, 0), 0);
			}
			else
			{
				PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_INCREASE, 0), 0);
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_EBBPF:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_PREVIOUSFILE, 0), 0);
					break;
				case IDC_EBBSB:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_SEEK_STEPBACKWARD, 0), 0);
					break;
				case IDC_EBBPP:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_PLAYPAUSE, 0), 0);
					break;
				case IDC_EBBSF:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_SEEK_STEPFORWARD, 0), 0);
					break;
				case IDC_EBBNF:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_NEXTFILE, 0), 0);
					break;
				case IDC_EBBMUT:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_MUTE, 0), 0);
					break;
				case IDC_EBBBN:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_BALANCE_NORMAL, 0), 0);
					break;
				case IDM_FILE_MULTIPLEINSTANCES:
					dwMultipleInstances = !dwMultipleInstances;
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_MULTIPLEINSTANCES, MF_BYCOMMAND |
						(dwMultipleInstances)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_FILE_NEWPLAYER:
					ShellExecute(0, L"Open", lpwAppPath, 0, 0, SW_NORMAL);
					break;
				case IDM_FILE_OPENFILES:
				case IDM_FILE_ADDFILES:
				{
					BOOL bOpen = (LOWORD(wParam) == IDM_FILE_OPENFILES);
					WCHAR lpwODFile[APP_OD_MS_MAX_FILE] = { 0 };
					WCHAR lpwODFilter[APP_OD_MAX_FILTER] = { 0 };
					WCHAR lpwExt[64] = { 0 };
					LPWSTR pFiles[FC_MAX_FILES];
					ULONG i, lODFileCnt = 0, lFileCnt = 0;
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
					if (GetOpenDialog(hAppInstance, hWnd, (bOpen)?L"Open File(s)":L"Add File(s)",
						lpwODFile, APP_OD_MS_MAX_FILE - 1, lpwODFilter, 1, TRUE, lpwRecentDir))
					{
						if (bOpen) pFileCollection->Clear();
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
							wcscpy(lpwRecentDir, lpwDir);
							for (i = 1; i < lODFileCnt; i++)
							{
								wcscpy(lpwFile, lpwDir);
								wcscat(lpwFile, pFiles[i]);
								SP_ExtractRightPart(lpwFile, lpwExt, '.');
								if (SFT_IsMemberOfCategory(lpwExt, SFTC_PLAYLIST))
								{
									lFileCnt += LoadPlaylist(lpwFile);
								}
								else
								{
									pFileCollection->AppendFile(lpwFile);
									lFileCnt++;
								}
							}
							for (i = 0; i < lODFileCnt; i++)
								delete[] pFiles[i];
						}
						else
						{
							SP_ExtractDirectory(pFiles[0], lpwRecentDir);
							SP_AddDirSep(lpwRecentDir, lpwRecentDir);
							SP_ExtractRightPart(pFiles[0], lpwExt, '.');
							if (SFT_IsMemberOfCategory(lpwExt, SFTC_PLAYLIST))
							{
								lFileCnt += LoadPlaylist(pFiles[0]);
							}
							else
							{
								pFileCollection->AppendFile(pFiles[0]);
								lFileCnt++;
							}
							delete[] pFiles[0];
						}
						if (lFileCnt)
						{
							if (bOpen)
							{
								InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
							}
							else
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
							}
						}
						else
						{
							if (bOpen)
							{
								if (pEngine->m_lpwFileName) CloseTrack();
							}
						}
					}
					break;
				}
				case IDM_OD_OPENDIRECTORY:
				case IDM_OD_OPENDIRECTORY_II:
				case IDM_AD_ADDDIRECTORY:
				case IDM_AD_ADDDIRECTORY_II:
				{
					BOOL bOpen = ((LOWORD(wParam) == IDM_OD_OPENDIRECTORY) || (LOWORD(wParam) == IDM_OD_OPENDIRECTORY_II));
					BOOL bIncImages = ((LOWORD(wParam) == IDM_OD_OPENDIRECTORY_II) || (LOWORD(wParam) == IDM_AD_ADDDIRECTORY_II));
					WCHAR lpwBFFPath[MAX_PATH] = { 0 };
					if (GetBrowseForFolderDialog(hWnd, lpwBFFPath, (bOpen)?L"Select directory to open"
						:L"Select directory to add", lpwRecentDir))
					{
						wcscpy(lpwRecentDir, lpwBFFPath);
						if (bOpen) pFileCollection->Clear();
						if(LoadDirectory(lpwBFFPath, bIncImages))
						{
							if (bOpen)
							{
								InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
							}
							else
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
							}
						}
						else 
						{
							if (bOpen) 
							{
								if (pEngine->m_lpwFileName) CloseTrack();
							}
						}
					}
					break;
				}
				case IDM_FILE_ADDURL:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_URL), hWnd,
						URLDlgProc, 0);
					break;
				case IDM_FILE_FAVORITES:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_FAVORITES), hWnd,
						FavoritesDlgProc, 0);
					break;
				case IDM_FILE_REOPENCURRENT:
					int intPos;
					ENGINESTATE eState;
					if (pEngine->m_lpwFileName)
					{
						intPos = (int)pEngine->GetPosition();
						eState = pEngine->GetState();
						SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSE, 0), 0);
					}
					else eState = E_STATE_STOPPED;
					InitTrack(FCF_RECENT);
					if (eState != E_STATE_STOPPED)
						SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, intPos);
					switch (eState)
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
					break;
				case IDM_FILE_CLOSE:
					CloseTrack();
					UpdateOnTopState();
					break;
				case IDM_FILE_DELETEFROMPLAYLIST:
				{
					WCHAR lpwFile[MAX_PATH] = { 0 };
					LPPLITEMDESC pPLID = 0;
					pFileCollection->GetUserData(pEngine->m_lpwFileName, 0, FCF_BYFILENAME, (LONG_PTR &)pPLID);
					if (pPLID)
						delete pPLID;
					pFileCollection->DeleteFile(pEngine->m_lpwFileName, 0, FCF_BYFILENAME);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSE, 0), 0);
					if (pFileCollection->GetFile(lpwFile, 0, FCF_RECENT))
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
					break;
				}
				case IDM_FILE_SAVEFILEAS:
				{
					WCHAR lpwSDFile[MAX_PATH] = { 0 };
					DWORD dwFilterIndex = 1;
					wcscpy(lpwSDFile, pEngine->m_lpwFileName);
					if (GetSaveDialog(hAppInstance, hWnd, L"Save File As", lpwSDFile,
						MAX_PATH - 1, L"Any file\0*.*;\0", &dwFilterIndex, 0, lpwRecentDir))
					{
						SP_ExtractDirectory(lpwSDFile, lpwRecentDir);
						SP_AddDirSep(lpwRecentDir, lpwRecentDir);
						if (_wcsicmp(pEngine->m_lpwFileName, lpwSDFile) != 0)
							CopyFile(pEngine->m_lpwFileName, lpwSDFile, FALSE);
					}
					break;
				}
				case IDM_FILE_SAVEPLAYLISTAS:
				{
					WCHAR lpwSDFile[MAX_PATH] = { 0 };
					WCHAR lpwExt[64] = { 0 };
					DWORD dwFilterIndex = 1;
					if (GetSaveDialog(hAppInstance, hWnd, L"Save Playlist As", lpwSDFile,
						MAX_PATH - 1, L"EBL Playlist\0*.ebl;\0M3U/M3U8 Playlist\0*.m3u; *.m3u8;\0"
						L"ASX Playlist\0*.asx;\0", &dwFilterIndex, 0, lpwRecentDir))
					{
						SP_ExtractDirectory(lpwSDFile, lpwRecentDir);
						SP_AddDirSep(lpwRecentDir, lpwRecentDir);
						SP_ExtractRightPart(lpwSDFile, lpwExt, '.');
						switch (dwFilterIndex)
						{
							case 1:
								if (_wcsicmp(lpwExt, L"ebl") != 0)
									wcscat(lpwSDFile, L".ebl");
								break;

							case 2:
								if (_wcsicmp(lpwExt, L"m3u8") != 0)
								{
									if (_wcsicmp(lpwExt, L"m3u") != 0)
										wcscat(lpwSDFile, L".m3u");
								}
								break;
							case 3:
								if (_wcsicmp(lpwExt, L"asx") != 0)
									wcscat(lpwSDFile, L".asx");
								break;
						}
						SavePlaylist(lpwSDFile);
					}
					break;
				}
				case IDM_FILE_PROPERTIES:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_PROPERTIES), hWnd,
						FilePropDlgProc, 0);
					break;
				case IDM_FILE_ASSOCIATIONS:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_ASSOCIATIONS), hWnd,
						AssociationsDlgProc, 0);
					break;
				case IDM_FILE_REMEMBERPLAYLIST:
					dwRememberPlaylist = !dwRememberPlaylist;
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_REMEMBERPLAYLIST, MF_BYCOMMAND |
						(dwRememberPlaylist)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_FILE_CLOSEWINDOW:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				case IDM_FILE_EXIT:
					PostMessage(HWND_BROADCAST, EBM_QUIT, 0, 0);
					break;
				case IDM_WB_NORMALWINDOW:
					dwWindowBorderIndex = 0;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_WB_NORMALWINDOW,
						IDM_WB_TOOLWINDOW, IDM_WB_NORMALWINDOW, MF_BYCOMMAND);
					UpdateBorderStyle();
					UpdateMainControlsState();
					UpdateBorderStyle(hPlaylistWnd);
					if (hVideoWnd/*pEngine->IsVideo()*/)
						UpdateBorderStyle(hVideoWnd);
					break;
				case IDM_WB_TOOLWINDOW:
					dwWindowBorderIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_WB_NORMALWINDOW,
						IDM_WB_TOOLWINDOW, IDM_WB_TOOLWINDOW, MF_BYCOMMAND);
					UpdateBorderStyle();
					UpdateMainControlsState();
					UpdateBorderStyle(hPlaylistWnd);
					if (hVideoWnd/*pEngine->IsVideo()*/)
						UpdateBorderStyle(hVideoWnd);
					break;
				case IDM_VIEW_MAINCONTROLS:
					dwMainControls = !dwMainControls;
					CheckMenuItem(GetMenu(hWnd), IDM_VIEW_MAINCONTROLS, MF_BYCOMMAND |
						(dwMainControls)?MF_CHECKED:MF_UNCHECKED);
					UpdateMainControlsState();
					break;
				case IDM_VIEW_PLAYLIST:
				{
					dwPlaylist = !dwPlaylist;
					CheckMenuItem(GetMenu(hWnd), IDM_VIEW_PLAYLIST, MF_BYCOMMAND |
						(dwPlaylist)?MF_CHECKED:MF_UNCHECKED);
					ShowWindow(hPlaylistWnd, (dwPlaylist)?SW_SHOW:SW_HIDE);
					break;
				}
				case IDM_TB_DISPLAYFULLPATH:
					dwTitleBarIndex = 0;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_TB_DISPLAYFULLPATH,
						IDM_TB_DISPLAYFILENAMEONLY, IDM_TB_DISPLAYFULLPATH, MF_BYCOMMAND);
					if (pEngine->m_lpwFileName)
						UpdateCFTitle(pEngine->m_lpwFileName, TRUE);
					break;
				case IDM_TB_DISPLAYFILENAMEONLY:
					dwTitleBarIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_TB_DISPLAYFULLPATH,
						IDM_TB_DISPLAYFILENAMEONLY, IDM_TB_DISPLAYFILENAMEONLY, MF_BYCOMMAND);
					if (pEngine->m_lpwFileName)
						UpdateCFTitle(pEngine->m_lpwFileName, TRUE);
					break;
				case IDM_TB_DONOTCHANGETITLE:
					dwTBDoNotChangeTitle = !dwTBDoNotChangeTitle;
					CheckMenuItem(GetMenu(hWnd), IDM_TB_DONOTCHANGETITLE, MF_BYCOMMAND |
						(dwTBDoNotChangeTitle)?MF_CHECKED:MF_UNCHECKED);
					if (pEngine->m_lpwFileName)
						UpdateCFTitle(pEngine->m_lpwFileName, TRUE);
					break;
				case IDM_INTERFACE_USESYSTEMCOLORS:
					dwUseSystemColors = !dwUseSystemColors;
					CheckMenuItem(GetMenu(hWnd), IDM_INTERFACE_USESYSTEMCOLORS, MF_BYCOMMAND |
						(dwUseSystemColors)?MF_CHECKED:MF_UNCHECKED);
					UpdateEBColors();
					break;
				case IDM_INTERFACE_DEFINECOLORS:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_COLORS), hWnd,
						ColorsDlgProc, 0);
					UpdateEBColors();
					break;
				case IDM_ONTOP_NEVER:
					dwOnTopIndex = 0;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_ONTOP_NEVER,
						IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_NEVER, MF_BYCOMMAND);
					UpdateOnTopState();
					break;
				case IDM_ONTOP_ALWAYS:
					dwOnTopIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_ONTOP_NEVER,
						IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_ALWAYS, MF_BYCOMMAND);
					UpdateOnTopState();
					break;
				case IDM_ONTOP_WHILEPLAYING:
					dwOnTopIndex = 2;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_ONTOP_NEVER,
						IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_WHILEPLAYING, MF_BYCOMMAND);
					UpdateOnTopState();
					break;
				case IDM_PAS_SCREENCENTER:
					dwPositionAtStartupIndex = 0;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_PAS_SCREENCENTER,
						IDM_PAS_RANDOM, IDM_PAS_SCREENCENTER, MF_BYCOMMAND);
					break;
				case IDM_PAS_RESTOREPREVIOUS:
					dwPositionAtStartupIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_PAS_SCREENCENTER,
						IDM_PAS_RANDOM, IDM_PAS_RESTOREPREVIOUS, MF_BYCOMMAND);
					break;
				case IDM_PAS_RANDOM:
					dwPositionAtStartupIndex = 2;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_PAS_SCREENCENTER,
						IDM_PAS_RANDOM, IDM_PAS_RANDOM, MF_BYCOMMAND);
					break;
				case IDM_OPACITY_100:
					dwOpacityLevel = 100;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_OPACITY_100,
						IDM_OPACITY_CUSTOM, IDM_OPACITY_100, MF_BYCOMMAND);
					UpdateOpacityState();
					UpdateOpacityState(hPlaylistWnd);
					/*if (hVideoWnd)
						UpdateOpacityState(hVideoWnd);*/
					break;
				case IDM_OPACITY_75:
					dwOpacityLevel = 75;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_OPACITY_100,
						IDM_OPACITY_CUSTOM, IDM_OPACITY_75, MF_BYCOMMAND);
					UpdateOpacityState();
					UpdateOpacityState(hPlaylistWnd);
					/*if (hVideoWnd)
						UpdateOpacityState(hVideoWnd);*/
					break;
				case IDM_OPACITY_50:
					dwOpacityLevel = 50;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_OPACITY_100,
						IDM_OPACITY_CUSTOM, IDM_OPACITY_50, MF_BYCOMMAND);
					UpdateOpacityState();
					UpdateOpacityState(hPlaylistWnd);
					/*if (hVideoWnd)
						UpdateOpacityState(hVideoWnd);*/
					break;
				case IDM_OPACITY_25:
					dwOpacityLevel = 25;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_OPACITY_100,
						IDM_OPACITY_CUSTOM, IDM_OPACITY_25, MF_BYCOMMAND);
					UpdateOpacityState();
					UpdateOpacityState(hPlaylistWnd);
					/*if (hVideoWnd)
						UpdateOpacityState(hVideoWnd);*/
					break;
				case IDM_OPACITY_CUSTOM:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_OPACITY),
						hWnd, OpacityDlgProc, 0);
					switch (dwOpacityLevel)
					{
						case 100:
							PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_OPACITY_100, 0), 0);
							break;
						case 75:
							PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_OPACITY_75, 0), 0);
							break;
						case 50:
							PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_OPACITY_50, 0), 0);
							break;
						case 25:
							PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_OPACITY_25, 0), 0);
							break;
						default:
							CheckMenuRadioItem(GetMenu(hWnd), IDM_OPACITY_100,
								IDM_OPACITY_CUSTOM, IDM_OPACITY_CUSTOM, MF_BYCOMMAND);
							UpdateOpacityState();
							UpdateOpacityState(hPlaylistWnd);
							/*if (hVideoWnd)
								UpdateOpacityState(hVideoWnd);*/
							break;
					}
					break;
				case IDM_OPACITY_OPAQUEONFOCUS:
					dwOpaqueOnFocus = !dwOpaqueOnFocus;
					CheckMenuItem(GetMenu(hMainWnd), IDM_OPACITY_OPAQUEONFOCUS, MF_BYCOMMAND |
						(dwOpaqueOnFocus)?MF_CHECKED:MF_UNCHECKED);
					UpdateOpacityState();
					UpdateOpacityState(hPlaylistWnd);
					/*if (hVideoWnd)
						UpdateOpacityState(hVideoWnd);*/
					break;
				case IDM_VIEW_TRAYICON:
					dwTrayIcon = !dwTrayIcon;
					CheckMenuItem(GetMenu(hWnd), IDM_VIEW_TRAYICON, MF_BYCOMMAND |
						(dwTrayIcon)?MF_CHECKED:MF_UNCHECKED);
					if (dwTrayIcon) InitTrayIcon(); else RemoveTrayIcon();
					break;
				case IDM_AS_RESUMEPLAYBACK:
					dwASResumePlayback = !dwASResumePlayback;
					CheckMenuItem(GetMenu(hMainWnd), IDM_AS_RESUMEPLAYBACK, MF_BYCOMMAND |
						(dwASResumePlayback)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_REPEAT_REPEATOFF:
					dwRepeatIndex = 0;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_REPEAT_REPEATOFF,
						IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATOFF, MF_BYCOMMAND);
					break;
				case IDM_REPEAT_REPEATALL:
					dwRepeatIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_REPEAT_REPEATOFF,
						IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATALL, MF_BYCOMMAND);
					break;
				case IDM_REPEAT_REPEATONE:
					dwRepeatIndex = 2;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_REPEAT_REPEATOFF,
						IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATONE, MF_BYCOMMAND);
					break;
				case IDM_PLAYBACK_SHUFFLE:
					dwShuffle = !dwShuffle;
					CheckMenuItem(GetMenu(hWnd), IDM_PLAYBACK_SHUFFLE, MF_BYCOMMAND |
						(dwShuffle)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_PLAYBACK_PREVIOUSFILE:
				{
					ENGINESTATE eOldState = pEngine->GetState();
					if (dwShuffle)
					{
						if (InitTrack(FCF_RANDOM) == -1) InitTrack(FCF_RANDOM);
					}
					else
					{
						InitTrack(FCF_BACKWARD);
					}
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
					break;
				}
				case IDM_PLAYBACK_NEXTFILE:
				{
					ENGINESTATE eOldState = pEngine->GetState();
					if (dwShuffle)
					{
						if (InitTrack(FCF_RANDOM) == -1) InitTrack(FCF_RANDOM);
					}
					else
					{
						InitTrack(FCF_FORWARD);
					}
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
					break;
				}
				case IDM_PLAYBACK_PLAYPAUSE:
					if (pEngine->GetState() == E_STATE_STOPPED || pEngine->GetState() == E_STATE_PAUSED)
					{
						if (pEngine->GetState() == E_STATE_PAUSED)
						{
							if (PI.psSource != PS_OTHER)
							{
								MessageBeep(0);
								break;
							}
						}
						pEngine->Play();
					}
					else
					{
						PI.psSource = PS_OTHER;
						pEngine->Pause();
					}
					break;
				case IDM_PLAYBACK_STOP:
					pEngine->Stop();
					break;
				case IDM_PLAYBACK_FRAMESTEP:
					pEngine->FrameStep(1);
					break;
				case IDM_RATE_NORMAL:
					pEngine->SetRate(1.0);
					break;
				case IDM_RATE_DECREASE:
					pEngine->SetRate(pEngine->GetRate() - 0.1);
					break;
				case IDM_RATE_INCREASE:
					pEngine->SetRate(pEngine->GetRate() + 0.1);
					break;
				case IDM_SEEK_STEPBACKWARD:
				{
					int intPos = (int)pEngine->GetPosition();
					int intLen = (int)pEngine->GetLength();
					int intNewPos = (intPos - (dwSeekValue1 * 1000));
					CheckBounds((long *)&intNewPos, 0, (long)intLen);
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, intNewPos);
					CI.dwInfoIndex = II_POSITION;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					break;
				}
				case IDM_SEEK_STEPFORWARD:
				{
					int intPos = (int)pEngine->GetPosition();
					int intLen = (int)pEngine->GetLength();
					int intNewPos = (intPos + (dwSeekValue1 * 1000));
					CheckBounds((long *)&intNewPos, 0, (long)intLen);
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, intNewPos);
					CI.dwInfoIndex = II_POSITION;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					break;
				}
				case IDM_SEEK_JUMPBACKWARD:
				{
					int intPos = (int)pEngine->GetPosition();
					int intLen = (int)pEngine->GetLength();
					int intNewPos = (intPos - (dwSeekValue2 * 1000));
					CheckBounds((long *)&intNewPos, 0, (long)intLen);
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, intNewPos);
					CI.dwInfoIndex = II_POSITION;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					break;
				}
				case IDM_SEEK_JUMPFORWARD:
				{
					int intPos = (int)pEngine->GetPosition();
					int intLen = (int)pEngine->GetLength();
					int intNewPos = (intPos + (dwSeekValue2 * 1000));
					CheckBounds((long *)&intNewPos, 0, (long)intLen);
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, intNewPos);
					CI.dwInfoIndex = II_POSITION;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					break;
				}
				case IDM_SEEK_LONGJUMPBACKWARD:
				{
					int intPos = (int)pEngine->GetPosition();
					int intLen = (int)pEngine->GetLength();
					int intNewPos = (intPos - (dwSeekValue3 * 1000));
					CheckBounds((long *)&intNewPos, 0, (long)intLen);
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, intNewPos);
					CI.dwInfoIndex = II_POSITION;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					break;
				}
				case IDM_SEEK_LONGJUMPFORWARD:
				{
					int intPos = (int)pEngine->GetPosition();
					int intLen = (int)pEngine->GetLength();
					int intNewPos = (intPos + (dwSeekValue3 * 1000));
					CheckBounds((long *)&intNewPos, 0, (long)intLen);
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, intNewPos);
					CI.dwInfoIndex = II_POSITION;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					break;
				}
				case IDM_SEEK_DEFINETIMEINTERVALS:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_TIMEINTERVALS),
						hWnd, TimeIntervalsDlgProc, 0);
					break;
				case IDM_SEEK_REWIND:
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, 0);
					CI.dwInfoIndex = II_POSITION;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					break;
				case IDM_SEEK_SEEKBYKEYFRAMES:
					dwSeekByKeyFrames = !dwSeekByKeyFrames;
					CheckMenuItem(GetMenu(hWnd), IDM_SEEK_SEEKBYKEYFRAMES, MF_BYCOMMAND |
						(dwSeekByKeyFrames)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_VOLUME_MUTE:
					dwMute = !dwMute;
					CI.dwInfoIndex = II_MUTE;
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 1000;
					CheckMenuItem(GetMenu(hWnd), IDM_VOLUME_MUTE, MF_BYCOMMAND |
						(dwMute)?MF_CHECKED:MF_UNCHECKED);
					pEngine->SetMute(dwMute);
					UpdateMuteButtonState();
					break;
				case IDM_VOLUME_DECREASE:
				{
					int intVol, intMin, intMax;
					intVol = SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
					SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETRANGE, (WPARAM)&intMin,
						(LPARAM)&intMax);
					intVol--;
					CheckBounds((long *)&intVol, (long)intMin, (long)intMax);
					SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_SETPOS, TRUE, intVol);
					break;
				}
				case IDM_VOLUME_INCREASE:
				{
					int intVol, intMin, intMax;
					intVol = SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
					SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETRANGE, (WPARAM)&intMin,
						(LPARAM)&intMax);
					intVol++;
					CheckBounds((long *)&intVol, (long)intMin, (long)intMax);
					SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_SETPOS, TRUE, intVol);
					break;
				}
				case IDM_BALANCE_NORMAL:
					SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_SETPOS, TRUE, 0);
					break;
				case IDM_BALANCE_LEFT:
				{
					int intBal, intMin, intMax;
					intBal = SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
					SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETRANGE, (WPARAM)&intMin,
						(LPARAM)&intMax);
					intBal--;
					CheckBounds((long *)&intBal, (long)intMin, (long)intMax);
					SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_SETPOS, TRUE, intBal);
					break;
				}
				case IDM_BALANCE_RIGHT:
				{
					int intBal, intMin, intMax;
					intBal = SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
					SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETRANGE, (WPARAM)&intMin,
						(LPARAM)&intMax);
					intBal++;
					CheckBounds((long *)&intBal, (long)intMin, (long)intMax);
					SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_SETPOS, TRUE, intBal);
					break;
				}
				case IDM_PLAYBACK_SAVECURRENTFRAME:
				{
					WCHAR lpwSDFile[MAX_PATH] = { 0 };
					WCHAR lpwExt[64] = { 0 };
					DWORD dwFilterIndex = 1;
					ENGINESTATE eState = pEngine->GetState();
					if (eState != E_STATE_PAUSED)
					{
						PI.psSource = PS_OTHER;
						pEngine->Pause();
					}
					if (GetSaveDialog(hAppInstance, hWnd, L"Save Frame As", lpwSDFile,
						MAX_PATH - 1, L"Windows Bitmap\0*.bmp;\0", &dwFilterIndex, 0, lpwRecentDir))
					{
						SP_ExtractDirectory(lpwSDFile, lpwRecentDir);
						SP_AddDirSep(lpwRecentDir, lpwRecentDir);
						SP_ExtractRightPart(lpwSDFile, lpwExt, '.');
						if (_wcsicmp(lpwExt, L"bmp") != 0)
							wcscat(lpwSDFile, L".bmp");
						pEngine->SaveCurrentFrame(lpwSDFile);
					}
					if (eState == E_STATE_PLAYING)
						pEngine->Play();
					break;
				}
				case IDM_AR_KEEPASPECTRATIO:
					dwKeepAspectRatio = !dwKeepAspectRatio;
					CheckMenuItem(GetMenu(hWnd), IDM_AR_KEEPASPECTRATIO, MF_BYCOMMAND |
						(dwKeepAspectRatio)?MF_CHECKED:MF_UNCHECKED);
					SendMessage(hVideoWnd, VWM_UPDATEASPECTRATIO, 0, 0);
					break;
				case IDM_AR_DEFAULT:
					dwAspectRatioIndex = 0;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AR_DEFAULT,
						IDM_AR_169, IDM_AR_DEFAULT, MF_BYCOMMAND);
					SendMessage(hVideoWnd, VWM_UPDATEASPECTRATIO, 0, 0);
					break;
				case IDM_AR_43:
					dwAspectRatioIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AR_DEFAULT,
						IDM_AR_169, IDM_AR_43, MF_BYCOMMAND);
					SendMessage(hVideoWnd, VWM_UPDATEASPECTRATIO, 0, 0);
					break;
				case IDM_AR_54:
					dwAspectRatioIndex = 2;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AR_DEFAULT,
						IDM_AR_169, IDM_AR_54, MF_BYCOMMAND);
					SendMessage(hVideoWnd, VWM_UPDATEASPECTRATIO, 0, 0);
					break;
				case IDM_AR_169:
					dwAspectRatioIndex = 3;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AR_DEFAULT,
						IDM_AR_169, IDM_AR_169, MF_BYCOMMAND);
					SendMessage(hVideoWnd, VWM_UPDATEASPECTRATIO, 0, 0);
					break;
				case IDM_ZOOM_HALFSIZE:
					ScaleVideoWindow(hVideoWnd, 0);
					break;
				case IDM_ZOOM_NORMALSIZE:
					ScaleVideoWindow(hVideoWnd, 1);
					break;
				case IDM_ZOOM_DOUBLESIZE:
					ScaleVideoWindow(hVideoWnd, 2);
					break;
				case IDM_FULLSCREEN_FULLSCREENNORMAL:
				{
					if (VWD.dwVWPosFlag == VWPF_FULLSCREEN)
					{
						if (dwChangeFSVideoMode)
						{
							pVideoMode->RestoreOriginalVideoMode();
						}
						SendMessage(hVideoWnd, VWM_CHANGESTATE, VWS_NORMAL, 0);
					}
					else
					{
						SendMessage(hVideoWnd, VWM_SAVENORMALSTATE, 0, 0);
						if (dwChangeFSVideoMode)
						{
							VIDEOMODE VM = { 0 };
							VM.dwPelsWidth = dwFSVideoModeWidth;
							VM.dwPelsHeight = dwFSVideoModeHeight;
							VM.dwBitsPerPel = dwFSVideoModeBPP;
							VM.dwDisplayFrequency = dwFSVideoModeDF;
							pVideoMode->ChangeVideoMode(VM, 0);
						}
						SendMessage(hVideoWnd, VWM_CHANGESTATE, VWS_FULLSCREEN, 0);
					}
					break;
				}
				case IDM_FULLSCREEN_VIDEOMODE:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_FULLSCREENVIDEOMODE),
						hWnd, FSVideoModeDlgProc, 0);
					break;
				case IDM_PB_ADV_AGTROT:
					dwAddGraphToROT = !dwAddGraphToROT;
					CheckMenuItem(GetMenu(hWnd), IDM_PB_ADV_AGTROT, MF_BYCOMMAND |
						(dwAddGraphToROT)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_DMO_USEDMO:
					dwUseDMO = !dwUseDMO;
					CheckMenuItem(GetMenu(hWnd), IDM_DMO_USEDMO, MF_BYCOMMAND |
						(dwUseDMO)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_PARAMEQ:
					dwDMOAEParamEq = !dwDMOAEParamEq;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_PARAMEQ, MF_BYCOMMAND |
						(dwDMOAEParamEq)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_WAVESREVERB:
					dwDMOAEWavesReverb = !dwDMOAEWavesReverb;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_WAVESREVERB, MF_BYCOMMAND |
						(dwDMOAEWavesReverb)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_GARGLE:
					dwDMOAEGargle = !dwDMOAEGargle;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_GARGLE, MF_BYCOMMAND |
						(dwDMOAEGargle)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_DISTORTION:
					dwDMOAEDistortion = !dwDMOAEDistortion;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_DISTORTION, MF_BYCOMMAND |
						(dwDMOAEDistortion)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_COMPRESSOR:
					dwDMOAECompressor = !dwDMOAECompressor;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_COMPRESSOR, MF_BYCOMMAND |
						(dwDMOAECompressor)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_ECHO:
					dwDMOAEEcho = !dwDMOAEEcho;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_ECHO, MF_BYCOMMAND |
						(dwDMOAEEcho)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_FLANGER:
					dwDMOAEFlanger = !dwDMOAEFlanger;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_FLANGER, MF_BYCOMMAND |
						(dwDMOAEFlanger)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_CHORUS:
					dwDMOAEChorus = !dwDMOAEChorus;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_CHORUS, MF_BYCOMMAND |
						(dwDMOAEChorus)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_SAE_I3DL2REVERB:
					dwDMOAEI3DL2Reverb = !dwDMOAEI3DL2Reverb;
					CheckMenuItem(GetMenu(hWnd), IDM_SAE_I3DL2REVERB, MF_BYCOMMAND |
						(dwDMOAEI3DL2Reverb)?MF_CHECKED:MF_UNCHECKED);
					break;
				case IDM_PB_ADV_EXTFLT:
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_EXTERNALFILTERS), hWnd,
						ExtFltDlgProc, 0);
					break;
				case IDM_AP_CLOSEWINDOW:
					dwAfterPlaybackIndex = 0;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
						IDM_AP_CLOSEWINDOW, MF_BYCOMMAND);
					break;
				case IDM_AP_EXIT:
					dwAfterPlaybackIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
						IDM_AP_EXIT, MF_BYCOMMAND);
					break;
				case IDM_AP_STANDBY:
					dwAfterPlaybackIndex = 2;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
						IDM_AP_STANDBY, MF_BYCOMMAND);
					break;
				case IDM_AP_HIBERNATE:
					dwAfterPlaybackIndex = 3;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
						IDM_AP_HIBERNATE, MF_BYCOMMAND);
					break;
				case IDM_AP_SHUTDOWN:
					dwAfterPlaybackIndex = 4;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
						IDM_AP_SHUTDOWN, MF_BYCOMMAND);
					break;
				case IDM_AP_LOGOFF:
					dwAfterPlaybackIndex = 5;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
						IDM_AP_LOGOFF, MF_BYCOMMAND);
					break;
				case IDM_AP_DONOTHING:
					dwAfterPlaybackIndex = 6;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
						IDM_AP_DONOTHING, MF_BYCOMMAND);
					break;
				case IDM_HELP_HELPTOPICS:
				{
					WCHAR lpwHHStr[360] = { 0 };
					WCHAR lpwAppDir[MAX_PATH] = { 0 };
					SP_ExtractDirectory(lpwAppPath, lpwAppDir);
					SP_AddDirSep(lpwAppDir, lpwAppDir);
					swprintf(lpwHHStr, L"%s%s::/%s", lpwAppDir, APP_HELP_FILE,
						APP_HELP_PAGE_MAIN);
					if (HtmlHelp(hWnd, lpwHHStr, HH_DISPLAY_TOPIC, 0) == 0)
					{
						LPWSTR lpwMsg = new WCHAR[128];
						swprintf(lpwMsg, L"Unable to open \"%s\"!", APP_HELP_FILE);
						MessageBox(hWnd, lpwMsg, APP_NAME, MB_ICONEXCLAMATION);
					}
					break;
				}
				case IDM_HELP_ABOUT:
				{
					DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd,
						AboutDlgProc, 0);
					break;
				}
			}
			return TRUE;
		case WM_INITMENUPOPUP:
			//Если это не системное меню, устанавливаем доступность/недоступность для пунктов
			if (!HIWORD(lParam))
			{
				BOOL bMFFlag1 = (pEngine->m_lpwFileName != 0);
				BOOL bMFFlag2 = (pEngine->IsSeekable() != 0);
				BOOL bMFFlag3 = (pEngine->CanStep(1) != 0);
				BOOL bMFFlag4 = (pEngine->IsVideo() != 0);
				BOOL bMFFlag5 = IsWindowVisible(hWnd);
				BOOL bMFFlag6 = (pEngine->GetState() != E_STATE_STOPPED);
				BOOL bMFFlag7 = (VWD.dwVWPosFlag == VWPF_NORMAL);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_NEWPLAYER,
					(dwMultipleInstances)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_REOPENCURRENT,
					(pFileCollection->FileCount())?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_CLOSE,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_DELETEFROMPLAYLIST,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_SAVEFILEAS,
					(bMFFlag1 && !IsURL(pEngine->m_lpwFileName))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_SAVEPLAYLISTAS,
					(bMFFlag1 || (pFileCollection->FileCount()))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_PROPERTIES,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FILE_EXIT,
					(dwMultipleInstances)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_INTERFACE_DEFINECOLORS,
					(!dwUseSystemColors)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_WB_NORMALWINDOW,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_WB_TOOLWINDOW,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_VIEW_MAINCONTROLS,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_TB_DISPLAYFULLPATH,
					(bMFFlag5 && !dwTBDoNotChangeTitle)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_TB_DISPLAYFILENAMEONLY,
					(bMFFlag5 && !dwTBDoNotChangeTitle)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_TB_DONOTCHANGETITLE,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_INTERFACE_USESYSTEMCOLORS,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_INTERFACE_DEFINECOLORS,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_ONTOP_NEVER,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_ONTOP_ALWAYS,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_ONTOP_WHILEPLAYING,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PAS_SCREENCENTER,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PAS_RESTOREPREVIOUS,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PAS_RANDOM,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_OPACITY_100,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_OPACITY_75,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_OPACITY_50,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_OPACITY_25,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_OPACITY_CUSTOM,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_OPACITY_OPAQUEONFOCUS,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_VIEW_TRAYICON,
					(bMFFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AS_RESUMEPLAYBACK,
					(dwRememberPlaylist && !dwShuffle)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PLAYBACK_PREVIOUSFILE,
					((dwShuffle)?(pFileCollection->FileCount() > 1):
					(pFileCollection->IsFileAvailable(FCF_BACKWARD)))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PLAYBACK_NEXTFILE,
					((dwShuffle)?(pFileCollection->FileCount() > 1):
					(pFileCollection->IsFileAvailable(FCF_FORWARD)))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PLAYBACK_PLAYPAUSE,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PLAYBACK_STOP,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PLAYBACK_FRAMESTEP,
					(bMFFlag3 && bMFFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_RATE_NORMAL,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_RATE_DECREASE,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_RATE_INCREASE,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SEEK_STEPBACKWARD,
					((bMFFlag1 && bMFFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SEEK_STEPFORWARD,
					((bMFFlag1 && bMFFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SEEK_JUMPBACKWARD,
					((bMFFlag1 && bMFFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SEEK_JUMPFORWARD,
					((bMFFlag1 && bMFFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SEEK_LONGJUMPBACKWARD,
					((bMFFlag1 && bMFFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SEEK_LONGJUMPFORWARD,
					((bMFFlag1 && bMFFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SEEK_REWIND,
					((bMFFlag1 && bMFFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_VOLUME_MUTE,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_VOLUME_DECREASE,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_VOLUME_INCREASE,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_BALANCE_NORMAL,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_BALANCE_LEFT,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_BALANCE_RIGHT,
					(bMFFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_PLAYBACK_SAVECURRENTFRAME,
					(bMFFlag4 && bMFFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AR_KEEPASPECTRATIO,
					(bMFFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AR_DEFAULT,
					(bMFFlag4 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AR_43,
					(bMFFlag4 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AR_54,
					(bMFFlag4 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AR_169,
					(bMFFlag4 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_ZOOM_HALFSIZE,
					(bMFFlag4 && bMFFlag7)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_ZOOM_NORMALSIZE,
					(bMFFlag4 && bMFFlag7)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_ZOOM_DOUBLESIZE,
					(bMFFlag4 && bMFFlag7)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_FULLSCREEN_FULLSCREENNORMAL,
					(bMFFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_PARAMEQ,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_WAVESREVERB,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_GARGLE,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_DISTORTION,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_COMPRESSOR,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_ECHO,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_FLANGER,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_CHORUS,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_SAE_I3DL2REVERB,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AP_CLOSEWINDOW,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AP_EXIT,
					((dwRepeatIndex == 0) && dwMultipleInstances)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AP_STANDBY,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AP_HIBERNATE,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AP_SHUTDOWN,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AP_LOGOFF,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(GetMenu(hWnd), IDM_AP_DONOTHING,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
			}
			return TRUE;
		case WM_COPYDATA:
		{
			COPYDATASTRUCT *pCDS = (COPYDATASTRUCT *)lParam;
			LPWSTR lpwCmdLine = new WCHAR[(pCDS->cbData / sizeof(WCHAR)) + 1];
			ULONG lCmdLnFileCnt;
			BOOL bCmdLnAdd = FALSE;
			ZeroMemory(lpwCmdLine, pCDS->cbData + sizeof(WCHAR));
			wcsncpy(lpwCmdLine, (LPWSTR)pCDS->lpData, pCDS->cbData / sizeof(WCHAR));
			lCmdLnFileCnt = ReadCommandLine(lpwCmdLine, &bCmdLnAdd);
			if (lCmdLnFileCnt)
			{
				if (!bCmdLnAdd)
				{
					InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
				}
				else
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
				}
				if (!bCmdLnAdd)
				{
					pEngine->Play();
				}
			}
			else
			{
				if (!bCmdLnAdd) 
				{
					if (pEngine->m_lpwFileName) CloseTrack();
				}
			}
			return TRUE;
		}
		case WM_DROPFILES:
		{
			WCHAR lpwDFile[MAX_PATH] = { 0 };
			WCHAR lpwExt[64] = { 0 };
			ULONG lDrFileCnt = 0, lFileCnt = 0;
			HDROP hDrop = (HDROP)wParam;
			lDrFileCnt = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);
			for (ULONG i = 0; i < lDrFileCnt; i++)
			{
				DragQueryFile(hDrop, i, lpwDFile, MAX_PATH);
				if (i == 0)
				{
					SP_ExtractDirectory(lpwDFile, lpwRecentDir);
					SP_AddDirSep(lpwRecentDir, lpwRecentDir);
				}
				SP_ExtractRightPart(lpwDFile, lpwExt, '.');
				if (SFT_IsMemberOfCategory(lpwExt, SFTC_PLAYLIST))
				{
					lFileCnt += LoadPlaylist(lpwDFile);
				}
				else if (IsDirectory(lpwDFile))
				{
					lFileCnt += LoadDirectory(lpwDFile);
				}
				else
				{
					pFileCollection->AppendFile(lpwDFile);
					lFileCnt++;
				}
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
			}
			DragFinish(hDrop);
			return TRUE;
		}
		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT pMIS = (LPMEASUREITEMSTRUCT)lParam;
			if (pMIS->CtlType == ODT_MENU)
			{
				if (!dwNoOwnerDrawMenu)
				{
					return pEBMenuMain->MeasureItem(wParam, lParam);
				}
			}
			return FALSE;
		}
		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
			if (pDIS->CtlType == ODT_MENU)
			{			
				if (!dwNoOwnerDrawMenu)
				{
					return pEBMenuMain->DrawItem(wParam, lParam);
				}
			}
			return FALSE;
		}
		case WM_SYSCOLORCHANGE:
		{
			BOOL bShowWnd = FALSE;
			if (IsWindowVisible(hWnd))
			{
				ShowWindow(hWnd, SW_HIDE);
				bShowWnd = TRUE;
			}
			UpdateEBColors();
			if (bShowWnd) ShowWindow(hWnd, SW_SHOW);
			return TRUE;
		}
		case WM_ERASEBKGND:
		{
			if (!dwNoAdvancedBackground)
			{
				if ((OSVI.dwMajorVersion > 5) || ((OSVI.dwMajorVersion == 5) && (OSVI.dwMinorVersion >= 1)))
				{
					HDC hDC = (HDC)wParam;
					RECT RCW = { 0 };
					GetClientRect(hWnd, &RCW);
					HTHEME hTheme = OpenThemeData(hWnd, L"REBAR");
					if (hTheme)
					{
						DrawThemeBackground(hTheme, hDC, 0, 0, &RCW, 0);
						CloseThemeData(hTheme);
						return TRUE;
					}
				}
			}
			return FALSE;
		}
		case WM_NOTIFY:
		{
			LPNMHDR pNM = (LPNMHDR)lParam;
			switch (wParam)
			{
				case IDC_EBDMAIN:
					switch (pNM->code)
					{
						case EBDN_CLICKED:
							switch (CI.dwInfoIndex)
							{
								case II_TIME:
									CI.dwInfoIndex = II_STATE;
									break;
								case II_STATE:
									CI.dwInfoIndex = II_TITLE;
									break;
								case II_TITLE:
								default:
									CI.dwInfoIndex = II_TIME;
									break;
							}
							CI.dwTimer = GetTickCount();
							CI.dwTimeout = 10000;
							break;
						case EBDN_DBLCLK:
							PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_PROPERTIES, 0), 0);
							break;
					}
					break;
				case IDC_EBSLDSEEK:
					switch (pNM->code)
					{
						case EBSN_SCROLL:
						{
							bSeekFlag = TRUE;
							CI.dwInfoIndex = II_POSITION;
							CI.dwTimer = GetTickCount();
							CI.dwTimeout = 1000;
							break;
						}
						case EBSN_CHANGE:
						{
							bSeekFlag = FALSE;
Seek_SetPosition:
							int intPos = SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_GETPOS, 0, 0);
							if (pEngine->IsVideo())
							{
								pEngine->SetPosition(intPos, TRUE, dwSeekByKeyFrames);
							}
							else
							{
								pEngine->SetPosition(intPos);
							}
							break;
						}
						case EBSN_CHANGE_SP:
							goto Seek_SetPosition;
							break;
					}
					break;
				case IDC_EBSLDVOL:
					switch (pNM->code)
					{
						case EBSN_SCROLL:
						case EBSN_CHANGE:
						case EBSN_CHANGE_SP:
						{
							int intVol = 0, intVoldB = 0;
							CI.dwInfoIndex = II_VOLUME;
							CI.dwTimer = GetTickCount();
							CI.dwTimeout = 1000;
							intVol = SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
							dwVolume = (DWORD)intVol;
							intVoldB = PercentsTodB_LogScale(intVol);
							pEngine->SetVolume(intVoldB);
							break;
						}
					}
					break;
				case IDC_EBSLDBAL:
					switch (pNM->code)
					{
						case EBSN_SCROLL:
						case EBSN_CHANGE:
						case EBSN_CHANGE_SP:
						{
							int intBal = 0, intBaldB = 0;
							CI.dwInfoIndex = II_BALANCE;
							CI.dwTimer = GetTickCount();
							CI.dwTimeout = 1000;
							intBal = SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
							dwBalance = (intBal + 100);
							intBaldB = PercentsTodB_LogScale((intBal >= 0)?100 - intBal:100 - abs(intBal));
							intBaldB = (intBal >= 0)?abs(intBaldB):intBaldB;
							pEngine->SetBalance(intBaldB);
							break;
						}
					}
					break;
			}
			return TRUE;
		}
		case WM_SIZE:
			switch (wParam)
			{
				case SIZE_MINIMIZED:
					if (dwTrayIcon)
					{
						ShowWindow(hWnd, SW_HIDE);
					}
					if (!dwNoAutomaticPause)
					{
						if (pEngine->IsVideo())
						{
							if (pEngine->GetState() == E_STATE_PLAYING)
							{
								PI.psSource = PS_SIZE_MINIMIZED;
								pEngine->Pause();
							}
						}
					}
					break;
				case SIZE_RESTORED:
					RedrawWindow(hWnd, 0, 0, RDW_ERASE | RDW_INVALIDATE);
					if (!dwNoAutomaticPause)
					{
						if (pEngine->IsVideo())
						{
							if (pEngine->GetState() == E_STATE_PAUSED)
							{
								if (PI.psSource == PS_SIZE_MINIMIZED)
								{
									pEngine->Play();
								}
							}
						}
					}
					break;
			}
			return TRUE;
		case WM_MOVING:
			StickyWindow(hWnd, (LPRECT)lParam);
			return TRUE;
		case WM_TIMER:
		{
			if (wParam == 1)
			{
				int intPos = 0, intLen = 0;
				int intMin = 0, intMax = 0, intCur = 0, intTmp = 0;
				BOOL bCtlsFlag1 = (pEngine->m_lpwFileName != 0);
				BOOL bCtlsFlag2 = (pEngine->IsSeekable() != 0);
				BOOL bSSActive, bEndOfPlayback;
				EnableWindow(GetDlgItem(hWnd, IDC_EBFMAIN), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_EBDMAIN), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_EBBPF), ((dwShuffle)?(pFileCollection->FileCount() > 1):
					(pFileCollection->IsFileAvailable(FCF_BACKWARD))));
				EnableWindow(GetDlgItem(hWnd, IDC_EBBSB), (bCtlsFlag1 && bCtlsFlag2));
				EnableWindow(GetDlgItem(hWnd, IDC_EBBPP), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_EBBSF),(bCtlsFlag1 && bCtlsFlag2));
				EnableWindow(GetDlgItem(hWnd, IDC_EBBNF), ((dwShuffle)?(pFileCollection->FileCount() > 1):
					(pFileCollection->IsFileAvailable(FCF_FORWARD))));
				EnableWindow(GetDlgItem(hWnd, IDC_EBSLDSEEK), (bCtlsFlag1 && bCtlsFlag2));
				EnableWindow(GetDlgItem(hWnd, IDC_EBBMUT), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_EBSLDVOL), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_EBBBN), bCtlsFlag1);
				EnableWindow(GetDlgItem(hWnd, IDC_EBSLDBAL), bCtlsFlag1);
				//----------------------------------------------------------------
				if (pEngine->GetState() == E_STATE_PLAYING)
				{
					if (_wcsicmp((LPWSTR)GetProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_"), L"pause") != 0)
					{
						SendDlgItemMessage(hWnd, IDC_EBBPP, EBBM_SETBITMAP, 0,
							(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_PAUSE), IMAGE_BITMAP, 16, 15, 0));
						SetProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_", (HANDLE)L"pause");
					}
					SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &bSSActive, 0);
					if (bSSActive)
					{
						SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0);
					}
				}
				else
				{
					if (_wcsicmp((LPWSTR)GetProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_"), L"play") != 0)
					{
						SendDlgItemMessage(hWnd, IDC_EBBPP, EBBM_SETBITMAP, 0,
							(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_PLAY), IMAGE_BITMAP, 16, 15, 0));
						SetProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_", (HANDLE)L"play");
					}
					SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &bSSActive, 0);
					if (!bSSActive)
					{
						SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0);
					}
				}
				//Если задано 'On Top->While Playing' - обновляем
				if (dwOnTopIndex == 2) UpdateOnTopState();
				//----------------------------------------------------------------
				if (bCtlsFlag1 && bCtlsFlag2)
				{
					intPos = (int)pEngine->GetPosition();
					intLen = (int)pEngine->GetLength();
					if (!bSeekFlag)
					{
						SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, 0, intPos);
					}
					//Если дошли до конца трека, то...
					if (intPos == intLen)
					{
						if (pFileCollection->IsFileAvailable(FCF_FORWARD) && (dwRepeatIndex != 2) && !dwShuffle)
						{
							InitTrack(FCF_FORWARD);
							pEngine->Play();
							bEndOfPlayback = FALSE;
						}
						else
						{
							bEndOfPlayback = TRUE;
							switch (dwRepeatIndex)
							{
								case 0:
									if (dwShuffle && (pFileCollection->FileCount() > 1))
									{
										if (InitTrack(FCF_RANDOM) == 0)
										{
											pEngine->Play();
											bEndOfPlayback = FALSE;
										}
									}
									break;
								case 1:
									if (dwShuffle && (pFileCollection->FileCount() > 1))
									{
										if (InitTrack(FCF_RANDOM) == -1) InitTrack(FCF_RANDOM);
										pEngine->Play();
									}
									else
									{
										if (pFileCollection->IsFileAvailable(FCF_BACKWARD))
										{
											InitTrack(FCF_FIRST);
											pEngine->Play();
										}
										else
										{
											pEngine->SetPosition(0);
										}
									}
									bEndOfPlayback = FALSE;
									break;
								case 2:
									pEngine->SetPosition(0);
									bEndOfPlayback = FALSE;
									break;
							}
							if (bEndOfPlayback)
							{
								pEngine->SetPosition(0);
								if (pEngine->GetState() == E_STATE_PLAYING)
								{
									SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_STOP, 0), 0);
								}
								switch (dwAfterPlaybackIndex)
								{
									case 0:
										PostMessage(hWnd, WM_CLOSE, 0, 0);
										break;
									case 1:
										PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSEWINDOW, 0), 0);
										break;
									case 2:
										ShowWindow(hWnd, SW_MINIMIZE);
										SetSystemPowerState(TRUE, FALSE);
										break;
									case 3:
										ShowWindow(hWnd, SW_MINIMIZE);
										SetSystemPowerState(FALSE, FALSE);
										break;
									case 4:
										ExitWindowsEx(EWX_SHUTDOWN, 0);
										break;
									case 5:
										ExitWindowsEx(EWX_LOGOFF, 0);
										break;
									case 6:
										//Ничего не делать
										break;
								}
							}
						}
					}
				}
				else
				{
					SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_SETPOS, TRUE, 0);
				}
				//----------------------------------------------------------------
				switch (CI.dwInfoIndex)
				{
					case II_TIME:
						if (bCtlsFlag1 && bCtlsFlag2)
						{
							if (((intPos / 1000) / 3600))
							{
								swprintf(CI.lpwBuffer, L"Position: %02i:%02i:%02i, Length: %02i:%02i:%02i, Rate: %.1f",
									(intPos / 1000) / 3600, ((intPos / 1000) / 60) % 60, ((intPos / 1000) % 60),
									(intLen / 1000) / 3600, ((intLen / 1000) / 60) % 60, ((intLen / 1000) % 60),
									pEngine->GetRate());
							}
							else
							{
								swprintf(CI.lpwBuffer, L"Position: %02i:%02i, Length: %02i:%02i, Rate: %.1f", ((intPos / 1000)
									/ 60) % 60, ((intPos / 1000) % 60), ((intLen / 1000) / 60) % 60, ((intLen / 1000) % 60),
									pEngine->GetRate());
							}
						}
						else
						{
							wcscpy(CI.lpwBuffer, L"Position: 00:00, Length: 00:00, Rate: 0.0");
						}
						break;
					case II_STATE:
						switch (pEngine->GetState())
						{
							case E_STATE_PLAYING:
								wcscpy(CI.lpwBuffer, L"State: Playing");
								break;
							case E_STATE_PAUSED:
								wcscpy(CI.lpwBuffer, L"State: Paused");
								break;
							case E_STATE_STOPPED:
								wcscpy(CI.lpwBuffer, L"State: Stopped");
								break;
						}
						break;
					case II_TITLE:
						wcscpy(CI.lpwBuffer, lpwCurFileTitle);
						break;
					case II_POSITION:
						if (bCtlsFlag1 && bCtlsFlag2)
						{
							SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_GETRANGE, (WPARAM)&intMin,
								(LPARAM)&intMax);
							intCur = SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_GETPOS, 0, 0);
							intTmp = (int)(((double)intCur / (double)intMax) * 100);
							swprintf(CI.lpwBuffer, L"Position: %i%c", intTmp, '%');
						}
						else
						{
							wcscpy(CI.lpwBuffer, L"Position: None");
						}
						break;
					case II_MUTE:
						if (bCtlsFlag1)
						{
							swprintf(CI.lpwBuffer, L"Mute: %s", (dwMute)?L"On":L"Off");
						}
						else
						{
							wcscpy(CI.lpwBuffer, L"Mute: None");
						}
						break;
					case II_VOLUME:
						if (bCtlsFlag1)
						{
							SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETRANGE, (WPARAM)&intMin,
								(LPARAM)&intMax);
							intCur = SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
							intTmp = (int)(((double)intCur / (double)intMax) * 100);
							swprintf(CI.lpwBuffer, L"Volume: %i%c", intTmp, '%');
						}
						else
						{
							wcscpy(CI.lpwBuffer, L"Volume: None");
						}
						break;
					case II_BALANCE:
						if (bCtlsFlag1)
						{
							SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETRANGE, (WPARAM)&intMin,
								(LPARAM)&intMax);
							intCur = SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
							intTmp = (int)(((double)intCur / (double)intMax) * 100);
							swprintf(CI.lpwBuffer, L"Balance: %i%c", intTmp, '%');
						}
						else
						{
							wcscpy(CI.lpwBuffer, L"Balance: None");
						}
						break;
				}
				SetDlgItemText(hWnd, IDC_EBDMAIN, CI.lpwBuffer);
				if ((GetTickCount() - CI.dwTimer) >= CI.dwTimeout)
				{
					switch (CI.dwInfoIndex)
					{
						case II_TIME:
							CI.dwInfoIndex = II_STATE;
							break;
						case II_STATE:
							CI.dwInfoIndex = II_TITLE;
							break;
						case II_TITLE:
						default:
							CI.dwInfoIndex = II_TIME;
							break;
					}
					CI.dwTimer = GetTickCount();
					CI.dwTimeout = 10000;
				}
			}
			return TRUE;
		}
		case WM_CLOSE:
		{
			WINDOWPLACEMENT WP = { 0 };
			GetWindowPlacement(hWnd, &WP);
			if (WP.showCmd == SW_NORMAL)
			{
				RECT RCW = { 0 };
				GetWindowRect(hWnd, &RCW);
				ptMainWindowPos.x = RCW.left;
				ptMainWindowPos.y = RCW.top;
			}
			RemoveProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_");
			KillTimer(hWnd, 1);

			if (dwRememberPlaylist)
			{
				if ((pEngine->m_lpwFileName) || (pFileCollection->FileCount()))
				{
					SavePlaylist(lpwPlPath);
					int intSelFile = pFileCollection->GetFileIndex(0, FCF_RECENT);
					dwSelectedFileIndex = (intSelFile >= 0)?intSelFile:0;
					//if (dwASResumePlayback)
					//{
						dwSFPosition = (pEngine->m_lpwFileName)?(DWORD)pEngine->GetPosition():0;
						dwSFState = pEngine->GetState();
					//}
				}
				else
				{
					DeleteFile(lpwPlPath);
					dwSelectedFileIndex = 0;
					dwSFPosition = 0;
					dwSFState = E_STATE_STOPPED;
				}
			}
			else
			{
				if (IsFile(lpwPlPath))
				{
					DeleteFile(lpwPlPath);
				}
				dwSelectedFileIndex = 0;
				dwSFPosition = 0;
				dwSFState = E_STATE_STOPPED;
			}
			if (pEngine->m_lpwFileName) CloseTrack();

			pToolTipsMain->Destroy();
			if (!dwNoOwnerDrawMenu) pEBMenuMain->InitEBMenu(0);
			DestroyWindow(hPlaylistWnd);
			InitMainWnd(FALSE);
			if (dwTrayIcon) RemoveTrayIcon();

			PostQuitMessage(0);
			return TRUE;
		}
	}
	return FALSE;
}

//Поиск окна программы в потоке
/*BOOL CALLBACK EnumThreadWndsProc(HWND hWnd, LPARAM lParam)
{
	WCHAR lpwWndClass[64] = { 0 };
	GetClassName(hWnd, lpwWndClass, 64);
	if (_wcsicmp(lpwWndClass, APP_MAIN_WND_CLASS) == 0)
	{
		*((BOOL*)lParam) = TRUE;
		return FALSE;
	}
	return TRUE;
}*/

void ApplySettings()
{
	CheckMenuItem(GetMenu(hMainWnd), IDM_FILE_MULTIPLEINSTANCES, MF_BYCOMMAND |
		(dwMultipleInstances)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_FILE_REMEMBERPLAYLIST, MF_BYCOMMAND |
		(dwRememberPlaylist)?MF_CHECKED:MF_UNCHECKED);
	switch (dwWindowBorderIndex)
	{
		case 0:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_WB_NORMALWINDOW,
				IDM_WB_TOOLWINDOW, IDM_WB_NORMALWINDOW, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_WB_NORMALWINDOW,
				IDM_WB_TOOLWINDOW, IDM_WB_TOOLWINDOW, MF_BYCOMMAND);
			break;
	}
	UpdateBorderStyle();
	UpdateBorderStyle(hPlaylistWnd);
	if (hVideoWnd)
		UpdateBorderStyle(hVideoWnd);
	CheckMenuItem(GetMenu(hMainWnd), IDM_VIEW_MAINCONTROLS, MF_BYCOMMAND |
		(dwMainControls)?MF_CHECKED:MF_UNCHECKED);
	UpdateMainControlsState();
	CheckMenuItem(GetMenu(hMainWnd), IDM_VIEW_PLAYLIST, MF_BYCOMMAND |
		(dwPlaylist)?MF_CHECKED:MF_UNCHECKED);
	//if (dwPlaylist)
	//	ShowWindow(hPlaylistWnd, SW_SHOW);
	switch (dwTitleBarIndex)
	{
		case 0:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_TB_DISPLAYFULLPATH,
				IDM_TB_DISPLAYFILENAMEONLY, IDM_TB_DISPLAYFULLPATH, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_TB_DISPLAYFULLPATH,
				IDM_TB_DISPLAYFILENAMEONLY, IDM_TB_DISPLAYFILENAMEONLY, MF_BYCOMMAND);
			break;
	}
	CheckMenuItem(GetMenu(hMainWnd), IDM_TB_DONOTCHANGETITLE, MF_BYCOMMAND |
		(dwTBDoNotChangeTitle)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_INTERFACE_USESYSTEMCOLORS, MF_BYCOMMAND |
		(dwUseSystemColors)?MF_CHECKED:MF_UNCHECKED);
	UpdateEBColors();
	switch (dwOnTopIndex)
	{
		case 0:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_ONTOP_NEVER,
				IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_NEVER, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_ONTOP_NEVER,
				IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_ALWAYS, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_ONTOP_NEVER,
				IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_WHILEPLAYING, MF_BYCOMMAND);
			break;
	}
	UpdateOnTopState();
	switch (dwPositionAtStartupIndex)
	{
		case 0:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_PAS_SCREENCENTER,
				IDM_PAS_RANDOM, IDM_PAS_SCREENCENTER, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_PAS_SCREENCENTER,
				IDM_PAS_RANDOM, IDM_PAS_RESTOREPREVIOUS, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_PAS_SCREENCENTER,
				IDM_PAS_RANDOM, IDM_PAS_RANDOM, MF_BYCOMMAND);
			break;
	}
	UpdatePosition();
	switch (dwOpacityLevel)
	{
		case 100:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_100, MF_BYCOMMAND);
			break;
		case 75:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_75, MF_BYCOMMAND);
			break;
		case 50:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_50, MF_BYCOMMAND);
			break;
		case 25:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_25, MF_BYCOMMAND);
			break;
		default:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_CUSTOM, MF_BYCOMMAND);
			break;
	}
	UpdateOpacityState();
	UpdateOpacityState(hPlaylistWnd);
	/*if (hVideoWnd)
		UpdateOpacityState(hVideoWnd);*/
	CheckMenuItem(GetMenu(hMainWnd), IDM_OPACITY_OPAQUEONFOCUS, MF_BYCOMMAND |
		(dwOpaqueOnFocus)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_VIEW_TRAYICON, MF_BYCOMMAND |
		(dwTrayIcon)?MF_CHECKED:MF_UNCHECKED);
	if (dwTrayIcon) InitTrayIcon(); else RemoveTrayIcon();
	CheckMenuItem(GetMenu(hMainWnd), IDM_AS_RESUMEPLAYBACK, MF_BYCOMMAND |
		(dwASResumePlayback)?MF_CHECKED:MF_UNCHECKED);
	switch (dwRepeatIndex)
	{
		case 0:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_REPEAT_REPEATOFF,
				IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATOFF, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_REPEAT_REPEATOFF,
				IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATALL, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_REPEAT_REPEATOFF,
				IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATONE, MF_BYCOMMAND);
			break;
	}
	CheckMenuItem(GetMenu(hMainWnd), IDM_PLAYBACK_SHUFFLE, MF_BYCOMMAND |
		(dwShuffle)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SEEK_SEEKBYKEYFRAMES, MF_BYCOMMAND |
		(dwSeekByKeyFrames)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_VOLUME_MUTE, MF_BYCOMMAND |
		(dwMute)?MF_CHECKED:MF_UNCHECKED);
	UpdateMuteButtonState();
	CheckMenuItem(GetMenu(hMainWnd), IDM_AR_KEEPASPECTRATIO, MF_BYCOMMAND |
		(dwKeepAspectRatio)?MF_CHECKED:MF_UNCHECKED);
	switch (dwAspectRatioIndex)
	{
		case 0:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_DEFAULT, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_43, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_54, MF_BYCOMMAND);
			break;
		case 3:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_169, MF_BYCOMMAND);
			break;
	}
	CheckMenuItem(GetMenu(hMainWnd), IDM_PB_ADV_AGTROT, MF_BYCOMMAND |
		(dwAddGraphToROT)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_DMO_USEDMO, MF_BYCOMMAND |
		(dwUseDMO)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_PARAMEQ, MF_BYCOMMAND |
		(dwDMOAEParamEq)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_WAVESREVERB, MF_BYCOMMAND |
		(dwDMOAEWavesReverb)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_GARGLE, MF_BYCOMMAND |
		(dwDMOAEGargle)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_DISTORTION, MF_BYCOMMAND |
		(dwDMOAEDistortion)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_COMPRESSOR, MF_BYCOMMAND |
		(dwDMOAECompressor)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_ECHO, MF_BYCOMMAND |
		(dwDMOAEEcho)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_FLANGER, MF_BYCOMMAND |
		(dwDMOAEFlanger)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_CHORUS, MF_BYCOMMAND |
		(dwDMOAEChorus)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMainWnd), IDM_SAE_I3DL2REVERB, MF_BYCOMMAND |
		(dwDMOAEI3DL2Reverb)?MF_CHECKED:MF_UNCHECKED);
	switch (dwAfterPlaybackIndex)
	{
		case 0:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_CLOSEWINDOW, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_EXIT, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_STANDBY, MF_BYCOMMAND);
			break;
		case 3:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_HIBERNATE, MF_BYCOMMAND);
			break;
		case 4:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_SHUTDOWN, MF_BYCOMMAND);
			break;
		case 5:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_LOGOFF, MF_BYCOMMAND);
			break;
		case 6:
			CheckMenuRadioItem(GetMenu(hMainWnd), IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_DONOTHING, MF_BYCOMMAND);
			break;
	}
}

//Открытие и инициализация трека
//В случае ошибки функция вернет значение меньше нуля
LONG InitTrack(DWORD dwFCFlag, DWORD dwFCIndex)
{
	WCHAR lpwFileName[MAX_PATH] = { 0 };
	ULONG i = 0;
	int intTmp1 = 0, intTmp2 = 0;
	LPPLITEMDESC pPLID = 0;
	if (!pFileCollection->NextFile(lpwFileName, dwFCIndex, dwFCFlag)) return -1;
	//Если у нас уже что-то открыто, вызываем File->Close
	if (pEngine->m_lpwFileName)
	{
		SendMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSE, 0), 0);
	}
	//Далее подготавливаем файл к воспроизведению...
	//-----------------------------------------------------
	pEngine->m_lpwFileName = new WCHAR[MAX_PATH];
	wcscpy(pEngine->m_lpwFileName, lpwFileName);
	pEngine->Initialize();
	//Добавление стандартных аудио эффектов (DMO)
	if (dwUseDMO)
	{
		if (dwDMOAEParamEq) pEngine->AddDMOToFilterGraph(L"ParamEq");
		if (dwDMOAEWavesReverb) pEngine->AddDMOToFilterGraph(L"WavesReverb");
		if (dwDMOAEGargle) pEngine->AddDMOToFilterGraph(L"Gargle");
		if (dwDMOAEDistortion) pEngine->AddDMOToFilterGraph(L"Distortion");
		if (dwDMOAECompressor) pEngine->AddDMOToFilterGraph(L"Compressor");
		if (dwDMOAEEcho) pEngine->AddDMOToFilterGraph(L"Echo");
		if (dwDMOAEFlanger) pEngine->AddDMOToFilterGraph(L"Flanger");
		if (dwDMOAEChorus) pEngine->AddDMOToFilterGraph(L"Chorus");
		if (dwDMOAEI3DL2Reverb) pEngine->AddDMOToFilterGraph(L"I3DL2Reverb");
	}
	//Добавление внешних фильтров
	if (dwUseExternalFilters)
	{
		pEngine->UpdateDSFiltersArray(DSF_CATEGORY_LEGACY_AM_FILTER);
		for (; i < APP_MAX_STRINGS; i++)
		{
			if (!lpwExternalFilters[i]) break;
			pEngine->AddDSFilterToFilterGraph(lpwExternalFilters[i]);
		}
	}
	//Добавление графа в 'ROT'
	if (dwAddGraphToROT) pEngine->AddFGToROT();
	pEngine->Open();
	if (pEngine->IsVideo())
	{
		if (dwTrayIcon && !IsWindowVisible(hMainWnd))
		{
			SendMessage(hTrayCBWnd, WM_COMMAND, MAKEWPARAM(IDM_TRAY_HIDESHOW, 0), 0);
		}
		hVideoWnd = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_VIDEO),
			hMainWnd, VideoDlgProc, 0);
		pEngine->SetVideoStyles(WS_CHILDWINDOW | WS_CLIPSIBLINGS, 0);
		pEngine->SetVideoOwner(hVideoWnd);
		UpdateBorderStyle(hVideoWnd);
		//UpdateOpacityState(hVideoWnd);
		AutoMoveVideoDlg(hVideoWnd);
		ShowWindow(hVideoWnd, SW_SHOW);
	}
	SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETRANGE, 0, (LPARAM)pEngine->GetLength());
	pEngine->SetMute(dwMute);
	intTmp1 = SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
	intTmp2 = PercentsTodB_LogScale(intTmp1);
	pEngine->SetVolume(intTmp2);
	intTmp1 = SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
	intTmp2 = PercentsTodB_LogScale((intTmp1 >= 0)?100 - intTmp1:100 - abs(intTmp1));
	intTmp2 = (intTmp1 >= 0)?abs(intTmp2):intTmp2;
	pEngine->SetBalance(intTmp2);
	pFileCollection->GetUserData(0, 0, FCF_RECENT, (LONG_PTR &)pPLID);
	if (!pPLID)
	{
		pPLID = new PLITEMDESC;
		ZeroMemory(pPLID, sizeof(PLITEMDESC));
		GetTitle(lpwFileName, pPLID->lpwTitle);
		pPLID->uDuration = (UINT)pEngine->GetLength();
		pFileCollection->SetUserData(0, 0, FCF_RECENT, (LONG_PTR)pPLID);
	}
	SetActiveItem(lpwFileName);
	UpdateCFTitle(lpwFileName);
	return 0;
}

//Закрытие трека
void CloseTrack()
{
	if (pEngine->IsVideo())
	{
		if (VWD.dwVWPosFlag == VWPF_FULLSCREEN)
		{
			SendMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FULLSCREEN_FULLSCREENNORMAL, 0), 0);
		}
		pEngine->SetVideoVisible(FALSE);
		pEngine->SetVideoOwner(0);
		DestroyWindow(hVideoWnd);
		hVideoWnd = 0;
	}
	if (pEngine->GetState() != E_STATE_STOPPED) pEngine->Stop();
	pEngine->Close();
	delete[] pEngine->m_lpwFileName;
	pEngine->m_lpwFileName = 0;
	SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETRANGE, 0, 0);
	UpdateCFTitle(0);
}

//Загрузка файлов из каталога, включая все его подкаталоги
DWORD LoadDirectory(LPCWSTR lpwPath, BOOL bIncImages)
{
	WCHAR lpwFFPath[MAX_PATH] = { 0 };
	WCHAR lpwFFMask[MAX_PATH] = { 0 };
	WCHAR lpwTmp[MAX_PATH] = { 0 };
	WCHAR lpwExt[64] = { 0 };
	WIN32_FIND_DATA WFD = { 0 };
	HANDLE hFindFile;
	BOOL bNext;
	ULONG i, lFileCnt = 0, lDirCnt = 0;
	SP_AddDirSep(lpwPath, lpwFFPath);
	//Сначала пытаемся найти поддерживаемые файлы
	for (i = 0; i < APP_SFT_UBOUND; i++)
	{
		if (wcslen(APP_SFT[i]) && (SFT_IsCategory(APP_SFT[i]) == SFTC_NONE))
		{
			swprintf(lpwFFMask, L"%s*.%s", lpwFFPath, APP_SFT[i]);
			hFindFile = FindFirstFile(lpwFFMask, &WFD);
			if (hFindFile != INVALID_HANDLE_VALUE)
			{
				bNext = TRUE;
				while (bNext)
				{
					if (wcscmp(WFD.cFileName, L".") != 0 && wcscmp(WFD.cFileName, L"..") != 0)
					{
						swprintf(lpwTmp, L"%s%s", lpwFFPath, WFD.cFileName);
						SP_ExtractRightPart(lpwTmp, lpwExt, '.');
						if (SFT_IsMemberOfCategory(lpwExt, SFTC_PLAYLIST))
						{
							lFileCnt += LoadPlaylist(lpwTmp);
						}
						else
						{
							if (!bIncImages)
								if (SFT_IsMemberOfCategory(lpwExt, SFTC_IMAGE))
									goto LD_NextFile;
							pFileCollection->AppendFile(lpwTmp);
							lFileCnt++;
						}
					}
LD_NextFile:
					bNext = FindNextFile(hFindFile, &WFD);
				}
			}
			FindClose(hFindFile);
		}
	}
	//Далее ищем подкаталоги, где так же сможем найти файлы
	swprintf(lpwFFMask, L"%s*", lpwFFPath);
	hFindFile = FindFirstFile(lpwFFMask, &WFD);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		bNext = TRUE;
		while (bNext)
		{
			if (wcscmp(WFD.cFileName, L".") != 0 && wcscmp(WFD.cFileName, L"..") != 0)
			{
				swprintf(lpwTmp, L"%s%s", lpwFFPath, WFD.cFileName);
				if (IsDirectory(lpwTmp))
				{
					lFileCnt += LoadDirectory(lpwTmp);
				}
			}
			bNext = FindNextFile(hFindFile, &WFD);
		}
	}
	FindClose(hFindFile);
	return lFileCnt;
}

//Основное окно программы
void InitMainWnd(BOOL bCreate)
{
	if (bCreate)
	{
		WNDCLASSEX WCEX = { 0 };
		WCEX.cbSize = sizeof(WNDCLASSEX); 
		WCEX.lpfnWndProc = DefDlgProc;
		WCEX.hInstance = hAppInstance;
		WCEX.lpszClassName = APP_MAIN_WND_CLASS;
		WCEX.cbWndExtra = DLGWINDOWEXTRA;
		RegisterClassEx(&WCEX);
		hMainWnd = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PLAYER), 0,
			PlayerDlgProc, 0);
	}
	else DestroyWindow(hMainWnd);
}

void PPSetDefFileInfo(HWND hPPWnd, LPWSTR lpwFileName)
{
	if (lpwFileName)
	{
		WORD wIndex = 0;
		WCHAR lpwIconPath[MAX_PATH] = { 0 };
		WCHAR lpwName[128] = { 0 };
		SP_ExtractName(lpwFileName, lpwName);
		SP_ExtractLeftPart(lpwName, lpwName, '.');
		wcscpy(lpwIconPath, lpwFileName);
		HICON hFile = ExtractAssociatedIcon(hAppInstance, lpwIconPath, &wIndex);
		if (hFile)
		{
			SendDlgItemMessage(hPPWnd, IDC_STCFILE, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hFile);
		}
		SendDlgItemMessage(hPPWnd, IDC_EDTFN, WM_SETTEXT, 0, (LPARAM)lpwName);
	}
	else
	{
		SendDlgItemMessage(hPPWnd, IDC_EDTFN, WM_SETTEXT, 0, (LPARAM)L"None");
	}
}

//Добавляет/удаляет элементам управления всплывающие подсказки
void UpdateToolTips(BOOL bRemove)
{
	if (bRemove)
	{
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBSLDSEEK));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBDMAIN));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBBMUT));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBSLDVOL));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBBPF));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBBSB));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBBPP));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBBSF));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBBNF));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBSLDBAL));
		pToolTipsMain->RemoveToolTip(GetDlgItem(hMainWnd, IDC_EBBBN));
	}
	else
	{
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBSLDSEEK), L"Seek");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBDMAIN), L"Display");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBBMUT), L"Mute");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBSLDVOL), L"Volume");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBBPF), L"Previous File");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBBSB), L"Step Backward");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBBPP), L"Play/Pause");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBBSF), L"Step Forward");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBBNF), L"Next File");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBSLDBAL), L"Balance");
		pToolTipsMain->AddToolTip(GetDlgItem(hMainWnd, IDC_EBBBN), L"Normal");
	}
}

void UpdateBorderStyle(HWND hTarget)
{
	HWND hWnd = (hTarget)?hTarget:hMainWnd;
	LONG lWndExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	switch (dwWindowBorderIndex)
	{
		case 0:
			if ((lWndExStyle & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW)
			{
				lWndExStyle ^= WS_EX_TOOLWINDOW;
			}
			break;
		case 1:
			if ((lWndExStyle & WS_EX_TOOLWINDOW) != WS_EX_TOOLWINDOW)
			{
				lWndExStyle |= WS_EX_TOOLWINDOW;
			}
			break;
	}
	SetWindowLong(hWnd, GWL_EXSTYLE, lWndExStyle);
	SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE |
		SWP_NOMOVE | SWP_NOZORDER);
}

//Обновляет состояние видимости основных элементов управления
void UpdateMainControlsState()
{
	RECT RCW = { 0 }, RCC = { 0 };
	GetWindowRect(hMainWnd, &RCW);
	if (dwMainControls)
	{
		//За метку для высоты основного окна берем кнопку 'Play/Pause'
		GetWindowRect(GetDlgItem(hMainWnd, IDC_EBBPP), &RCC);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBMUT), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBSLDVOL), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBPF), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBSB), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBPP), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBSF), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBNF), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBSLDBAL), SW_SHOW);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBBN), SW_SHOW);
		MoveWindow(hMainWnd, RCW.left, RCW.top, RCW.right - RCW.left,
			(RCC.bottom - RCW.top) + 10, TRUE);
	}
	else
	{
		//За метку для высоты основного окна берем основную рамку
		GetWindowRect(GetDlgItem(hMainWnd, IDC_EBFMAIN), &RCC);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBMUT), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBSLDVOL), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBPF), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBSB), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBPP), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBSF), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBNF), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBSLDBAL), SW_HIDE);
		ShowWindow(GetDlgItem(hMainWnd, IDC_EBBBN), SW_HIDE);
		MoveWindow(hMainWnd, RCW.left, RCW.top, RCW.right - RCW.left,
			(RCC.bottom - RCW.top) + 10, TRUE);
	}
}

COLORREF GetMenuBarColor()
{
	if ((OSVI.dwMajorVersion > 5) || ((OSVI.dwMajorVersion == 5) && (OSVI.dwMinorVersion >= 1)))
	{
		BOOL bFlatMenu = FALSE;
		SystemParametersInfo(SPI_GETFLATMENU, 0, (LPVOID)&bFlatMenu, 0);
		if (bFlatMenu)
		{
			return GetSysColor(COLOR_MENUBAR);
		}
	}
	return GetSysColor(COLOR_MENU);
}

void UpdateEBColors()
{
	if (dwUseSystemColors)
	{
		//Рассчитано на стандартные темы оформления
		if (!dwNoOwnerDrawMenu)
		{
			//Цвета меню
			pEBMenuMain->crFontColorOne = GetSysColor(COLOR_HIGHLIGHTTEXT);
			pEBMenuMain->crFontColorTwo = Blend(GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_HIGHLIGHT), 0.6);
			pEBMenuMain->crFontColorThree = GetSysColor(COLOR_MENUTEXT);
			pEBMenuMain->crBkColorOne = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.2);
			pEBMenuMain->crBkColorTwo = GetSysColor(COLOR_MENU);
			pEBMenuMain->crBkColorThree = GetMenuBarColor();
			pEBMenuMain->crSelColorOne = GetSysColor(COLOR_HIGHLIGHT);
			pEBMenuMain->crSelColorTwo = Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1);
			pEBMenuMain->crBrColorOne = Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1);
			pEBMenuMain->crBrColorTwo = Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1);
			pEBMenuMain->UpdateMenuBar();
			DrawMenuBar(hMainWnd);
			UpdateTrayMenuColors();
		}
		//Цвета элементов управления
		SendDlgItemMessage(hMainWnd, IDC_EBBMUT, EBBM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBMUT, EBBM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBPF, EBBM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBPF, EBBM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBSB, EBBM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBSB, EBBM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBPP, EBBM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBPP, EBBM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBSF, EBBM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBSF, EBBM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBNF, EBBM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBNF, EBBM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBBN, EBBM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBBBN, EBBM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBFMAIN, EBFM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		//Тень текста должна быть чуть светлее
		SendDlgItemMessage(hMainWnd, IDC_EBDMAIN, EBDM_SETFONTCOLORS, GetSysColor(COLOR_HIGHLIGHTTEXT),
			Blend(GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_HIGHLIGHT), 0.6));
		SendDlgItemMessage(hMainWnd, IDC_EBDMAIN, EBDM_SETBKCOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBDMAIN, EBDM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETLINECOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETBKCOLOR, 0, Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.2));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_SETLINECOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_SETBKCOLOR, 0, Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.2));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_SETLINECOLORS, GetSysColor(COLOR_HIGHLIGHT),
			Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_SETBKCOLOR, 0, Blend(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 0.2));
		SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_SETBRCOLORS, Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_BTNTEXT), 0.1),
			Blend(GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_HIGHLIGHT), 0.1));
		UpdatePlaylistColors();
	}
	else
	{
		if (!dwNoOwnerDrawMenu)
		{
			pEBMenuMain->crFontColorOne = dwTextColor;
			pEBMenuMain->crFontColorTwo = dwTextShadowColor;
			pEBMenuMain->crFontColorThree = GetSysColor(COLOR_MENUTEXT);
			pEBMenuMain->crBkColorOne = dwBackgroundColor;
			pEBMenuMain->crBkColorTwo = GetSysColor(COLOR_MENU);
			pEBMenuMain->crBkColorThree = GetMenuBarColor();
			pEBMenuMain->crSelColorOne = dwGradientColor1;
			pEBMenuMain->crSelColorTwo = dwGradientColor2;
			pEBMenuMain->crBrColorOne = dwBorderColor1;
			pEBMenuMain->crBrColorTwo = dwBorderColor2;
			pEBMenuMain->UpdateMenuBar();
			DrawMenuBar(hMainWnd);
			UpdateTrayMenuColors();
		}
		SendDlgItemMessage(hMainWnd, IDC_EBBMUT, EBBM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBMUT, EBBM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBPF, EBBM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBPF, EBBM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBSB, EBBM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBSB, EBBM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBPP, EBBM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBPP, EBBM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBSF, EBBM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBSF, EBBM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBNF, EBBM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBNF, EBBM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBBN, EBBM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBBBN, EBBM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBFMAIN, EBFM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBDMAIN, EBDM_SETFONTCOLORS, dwTextColor, dwTextShadowColor);
		SendDlgItemMessage(hMainWnd, IDC_EBDMAIN, EBDM_SETBKCOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBDMAIN, EBDM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETLINECOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETBKCOLOR, 0, dwBackgroundColor);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_SETLINECOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_SETBKCOLOR, 0, dwBackgroundColor);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_SETLINECOLORS, dwGradientColor1, dwGradientColor2);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_SETBKCOLOR, 0, dwBackgroundColor);
		SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_SETBRCOLORS, dwBorderColor1, dwBorderColor2);
		UpdatePlaylistColors();
	}
}

void UpdatePosition()
{
	switch (dwPositionAtStartupIndex)
	{
		case 0:
			MoveToCenter(hMainWnd, 0, 0);
			MoveToCenter(hPlaylistWnd, 0, 0);
			break;
		case 1:
			SetWindowPos(hMainWnd, 0, ptMainWindowPos.x, ptMainWindowPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			SetWindowPos(hPlaylistWnd, 0, rcPlaylistPos.left, rcPlaylistPos.top, rcPlaylistPos.right - rcPlaylistPos.left,
				rcPlaylistPos.bottom - rcPlaylistPos.top, SWP_NOZORDER);
			break;
		case 2:
			Randomize();
			MoveToCenter(hMainWnd, 100 - Random(200), 100 - Random(200));
			MoveToCenter(hPlaylistWnd, 100 - Random(200), 100 - Random(200));
			break;
	}
}

//Обновляет стиль окна "поверх всех" в зависимости от настройки меню 'On Top'
void UpdateOnTopState()
{
	LONG lWndStyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
	switch (dwOnTopIndex)
	{
		case 0:
			if ((lWndStyle & WS_EX_TOPMOST) == WS_EX_TOPMOST)
			{
				SetWindowPos(hMainWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			break;
		case 1:
			if ((lWndStyle & WS_EX_TOPMOST) != WS_EX_TOPMOST)
			{
				SetWindowPos(hMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			break;
		case 2:
			if (pEngine->GetState() == E_STATE_STOPPED)
			{
				if ((lWndStyle & WS_EX_TOPMOST) == WS_EX_TOPMOST)
				{
					SetWindowPos(hMainWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
			else
			{
				if ((lWndStyle & WS_EX_TOPMOST) != WS_EX_TOPMOST)
				{
					SetWindowPos(hMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
			break;
	}
}

void UpdateOpacityState(HWND hTarget)
{
	HWND hWnd = (hTarget)?hTarget:hMainWnd;
	LONG lWndStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	if (dwOpacityLevel < 100)
	{
		if ((lWndStyle & WS_EX_LAYERED) != WS_EX_LAYERED)
		{
			lWndStyle |= WS_EX_LAYERED;
			SetWindowLong(hWnd, GWL_EXSTYLE, lWndStyle);
		}
	}
	else
	{
		if ((lWndStyle & WS_EX_LAYERED) == WS_EX_LAYERED)
		{
			lWndStyle ^= WS_EX_LAYERED;
			SetWindowLong(hWnd, GWL_EXSTYLE, lWndStyle);
		}
		return;
	}
	BYTE bAlpha1 = (BYTE)(2.55 * dwOpacityLevel), bAlpha2 = 255;
	if (dwOpaqueOnFocus)
	{
		if (GetForegroundWindow() == hMainWnd)
		{
			SetLayeredWindowAttributes(hWnd, 0, bAlpha2, LWA_ALPHA);
			return;
		}
	}
	SetLayeredWindowAttributes(hWnd, 0, bAlpha1, LWA_ALPHA);
}

void UpdateMuteButtonState()
{
	if (dwMute)
	{
		SendDlgItemMessage(hMainWnd, IDC_EBBMUT, EBBM_SETBITMAP, 0,
			(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_SOUNDOFF), IMAGE_BITMAP, 10, 10, 0));
	}
	else
	{
		SendDlgItemMessage(hMainWnd, IDC_EBBMUT, EBBM_SETBITMAP, 0,
			(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_SOUNDON), IMAGE_BITMAP, 10, 10, 0));
	}
}

void UpdateCFTitle(LPCWSTR lpwFileName, BOOL bMainWndTitleOnly)
{
	if (!lpwFileName)
	{
		if (!bMainWndTitleOnly)
		{
			wcscpy(lpwCurFileTitle, L"None");
		}
		SetWindowText(hMainWnd, lpwStdWndTitle);
	}
	else
	{
		WCHAR lpwPath[MAX_PATH] = { 0 };
		WCHAR lpwName[128] = { 0 };
		wcscpy(lpwPath, lpwFileName);
		if (!bMainWndTitleOnly)
		{
			GetTitle(lpwPath, lpwName);
			wcscpy(lpwCurFileTitle, lpwName);
		}
		if (dwTBDoNotChangeTitle)
		{
			SetWindowText(hMainWnd, lpwStdWndTitle);
		}
		else
		{
			SHELLFLAGSTATE SFS = { 0 };
			SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
			if (!SFS.fShowExtensions)
			{
				SP_ExtractLeftPart(lpwPath, lpwPath, '.');
			}
			switch (dwTitleBarIndex)
			{
				case 0:
					SetWindowText(hMainWnd, lpwPath);
					break;
				case 1:
					SP_ExtractName(lpwPath, lpwName);
					SetWindowText(hMainWnd, lpwName);
					break;
			}
		}
	}
	if (dwTrayIcon)
	{
		WCHAR lpwTip[256] = { 0 };
		swprintf(lpwTip, L"%s\n%s", lpwStdWndTitle, lpwCurFileTitle);
		UpdateTrayIcon(lpwTip);
		if (!dwNoBalloonTips)
		{
			if (IsIconic(hMainWnd))
			{
				if (lpwFileName)
				{
					WCHAR lpwBalloonText[256] = { 0 };
					swprintf(lpwBalloonText, L"%s\n%s", L"Now playing:", lpwCurFileTitle);
					ShowBalloon(lpwStdWndTitle, lpwBalloonText, BI_INFORMATION, 15000);
				}
			}
		}
	}
}

void GetTitle(LPCWSTR lpwFileName, LPWSTR lpwResult)
{
	WCHAR lpwName[128] = { 0 };
	if ((pEngine->m_lpwFileName) && (_wcsicmp(lpwFileName, pEngine->m_lpwFileName) == 0))
	{
		MEDIACONTENT MC = { 0 };
		pEngine->UpdateFGFiltersArray();
		if (pEngine->GetMediaContent(&MC) < 0)
		{
			goto GetTitle_CreateFromFile;
		}
		else
		{
			int intAL = wcslen(MC.Author);
			int intTL = wcslen(MC.Title);
			if ((intAL == 0) && (intTL == 0))
			{
				goto GetTitle_CreateFromFile;
			}
			if (intAL)
			{
				swprintf(lpwName, L"%s - %s", MC.Author, (intTL)?MC.Title:L"None");
			}
			else
			{
				wcscpy(lpwName, MC.Title);
			}
		}
	}
	else
	{
GetTitle_CreateFromFile:
		SP_ExtractName(lpwFileName, lpwName);
		SP_ExtractLeftPart(lpwName, lpwName, '.');
	}
	wcscpy(lpwResult, lpwName);
}