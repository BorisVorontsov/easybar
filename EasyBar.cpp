/////////////////////////////////////////////////////////////////////////////
//		Проект: EasyBar - media player
//		Автор(ы): Борис Воронцов и участники проекта
//		Последнее обновление: 07.05.2023
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

WCHAR lpMutexName[128] = {};
HANDLE hMutex = 0;

HINSTANCE hAppInstance;
WCHAR lpAppPath[MAX_PATH] = {};
WCHAR lpAppVersion[20] = {};

OSVERSIONINFO OSVI = {};

HWND hMainWnd = 0;
HWND hPlaylistWnd = 0;
HWND hVideoWnd = 0;

WCHAR lpStdWndTitle[64] = {};
WCHAR lpCurFileTitle[128] = {};

WCHAR lpPlPath[MAX_PATH] = {};

static BYTE bCurAlpha;

static BOOL bSeekFlag = FALSE; //Временно отключает обновление полосы поиска

CURRENTINFO CI = {};
PAUSEINFO PI = { PS_OTHER, 0 };

static CEBMenu *pEBMenuMain = new CEBMenu;
static CToolTips *pToolTipsMain = new CToolTips;
CFileCollection *pFileCollection = new CFileCollection;
CDirectShow *pEngine = new CDirectShow;
CVideoMode *pVideoMode = new CVideoMode;

VWDATA VWD = {};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nShowCmd)
{
	SIZE_T szCmdLnLen = 0;
	ULONG lPlFileCnt = 0, lCmdLnFileCnt = 0;
	BOOL bCmdLnAdd = FALSE;
	HACCEL hMainAccel, hPLAccel;
	MSG Msg = {};
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
		wcscpy(lpMutexName, APP_NAME);
		LPWSTR lpUN = CreateUniqueName();
		if (wcslen(lpUN) < (128 - (wcslen(APP_NAME) + 1)))
		{
			wcscat(lpMutexName, L"-");
			wcscat(lpMutexName, CreateUniqueName());
		}
		delete[] lpUN;
		hMutex = CreateMutex(0, TRUE, lpMutexName);
		if (!dwMultipleInstances)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				HWND hAppWnd = FindWindow(APP_MAIN_WND_CLASS, 0);
				if (hAppWnd)
				{
					szCmdLnLen = wcslen(lpCmdLine);
					if (szCmdLnLen)
					{
						COPYDATASTRUCT CDS = {};
						CDS.cbData = (DWORD)(szCmdLnLen * sizeof(WCHAR));
						CDS.lpData = lpCmdLine;
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
		GetAppPath(hAppInstance, lpAppPath, MAX_PATH, FALSE);
		ReadFileVersion(lpAppPath, lpAppVersion, sizeof(lpAppVersion), RFV_MAJOR | RFV_MINOR | RFV_RELEASE);
		if (wcslen(APP_DEV_STAGE))
		{
			wcscat(lpAppVersion, L" ");
			wcscat(lpAppVersion, APP_DEV_STAGE);
		}
		swprintf(lpStdWndTitle, L"%s %s", APP_NAME, lpAppVersion);
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
		pEngine->m_lpFileName = 0;
		pEngine->m_lpAppName = APP_NAME;
		pEngine->m_hAppWnd = hMainWnd;
		pEngine->UpdateDMOArray(DMO_CATEGORY_AUDIO_EFFECT);
		ApplySettings();
		SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pIIDL);
		SHGetPathFromIDList(pIIDL, lpPlPath);
		CoTaskMemFree(pIIDL);
		SP_AddDirSep(lpPlPath, lpPlPath);
		wcscat(lpPlPath, APP_NAME);
		SP_AddDirSep(lpPlPath, lpPlPath);
		if (!IsDirectory(lpPlPath)) CreateDirectory(lpPlPath, 0);
		wcscat(lpPlPath, APP_PLAYLIST_FILE);
		if (IsFile(lpPlPath))
		{
			if (dwRememberPlaylist)
			{
				lPlFileCnt = LoadPlaylist(lpPlPath);
			}
			else
			{
				DeleteFile(lpPlPath);
			}
		}
		szCmdLnLen = wcslen(lpCmdLine);
		if (szCmdLnLen)
		{
			lCmdLnFileCnt = ReadCommandLine(lpCmdLine, &bCmdLnAdd);
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
				//if (!pEngine->m_lpFileName)
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
				//	pFileCollection->SetRecentFile(pEngine->m_lpFileName);
				//}
			}
			if (lCmdLnFileCnt && !bCmdLnAdd)
			{
				pEngine->Play();
			}
		}
		if ((nShowCmd == SW_MAXIMIZE) || (nShowCmd == SW_SHOWMAXIMIZED))
			nShowCmd = SW_SHOWDEFAULT;
		ShowWindow(hMainWnd, nShowCmd);
		if (dwPlaylist)
			ShowWindow(hPlaylistWnd, SW_SHOW);
#ifndef _DEBUG
	}
	__except(ExceptionFilter(pGEP = GetExceptionInformation()))
	{
		WCHAR lpMsg[MAX_PATH] = {};
		swprintf(lpMsg, L"Unexpected error in the main function!\n\nError code: 0x%08x"
			L"\nError address: 0x%08x", pGEP->ExceptionRecord->ExceptionCode,
			pGEP->ExceptionRecord->ExceptionAddress);
		if (IsDebuggerPresent())
		{
			OutputDebugString(L"\n********************************\n");
			OutputDebugString(lpMsg);
			OutputDebugString(L"\n********************************\n");
			Sleep(10000);
		}
		else
		{
			MessageBox(0, lpMsg, APP_NAME, MB_ICONSTOP);
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
	if (hMutex)
		CloseHandle(hMutex);
	SDO(pEBMenuMain);
	SDO(pToolTipsMain);
	SDO(pFileCollection);
	SDO(pEngine);
	SDO(pVideoMode);
	return (int)Msg.wParam;
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

DWORD ReadCommandLine(LPCWSTR lpCmdLine, LPBOOL pAdd)
{
	WCHAR lpExt[64] = {};
	LPWSTR *ppCmdLineArgs = 0;
	ULONG lArgsCnt = 0, lFileCnt = 0;
	ppCmdLineArgs = CommandLineToArgvW(lpCmdLine, (int *)&lArgsCnt);
	if (ppCmdLineArgs)
	{
		for (ULONG i = 0; i < lArgsCnt; i++)
		{
			if (i == 0)
			{
				if (!*pAdd)
				{
					if (wcscmp(ppCmdLineArgs[i], APP_CL_KEY_ADD) != 0)
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
			SP_ExtractRightPart(ppCmdLineArgs[i], lpExt, '.');
			if (SFT_IsMemberOfCategory(lpExt, SFTC_PLAYLIST))
			{
				lFileCnt += LoadPlaylist(ppCmdLineArgs[i]);
			}
			else if (IsDirectory(ppCmdLineArgs[i]))
			{
				lFileCnt += LoadDirectory(ppCmdLineArgs[i]);
			}
			else
			{
				pFileCollection->AppendFile(ppCmdLineArgs[i]);
				lFileCnt++;
			}
		}

		LocalFree(ppCmdLineArgs);
	}
	return lFileCnt;
}

INT_PTR CALLBACK PlayerDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == EBM_INITIALIZED)
	{
		if (lParam != GetCurrentProcessId())
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
				PostMessage(HWND_BROADCAST, EBM_INITIALIZED, 0, (LPARAM)GetCurrentProcessId());
			}
			return TRUE;
		case WM_ACTIVATEAPP:
		{
			if (dwOpaqueOnFocus && (dwOpacityLevel < 100))
			{
				if (wParam)
				{
					if (!dwNoTransitionEffects)
					{
						bCurAlpha = (BYTE)(2.55 * dwOpacityLevel);
						SetTimer(hWnd, 2, 10, 0);
					}
					else
					{
						SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
					}
				}
				else
				{
					if (!dwNoTransitionEffects)
					{
						bCurAlpha = 255;
						SetTimer(hWnd, 3, 10, 0);
					}
					else
					{
						SetLayeredWindowAttributes(hWnd, 0, (BYTE)(2.55 * dwOpacityLevel), LWA_ALPHA);
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
							HANDLE hMutex2 = CreateMutex(0, TRUE, lpMutexName);
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
							CloseHandle(hMutex2);
						}
					//}
					PostMessage(HWND_BROADCAST, EBM_ACTIVATED, 0, (LPARAM)GetCurrentThreadId());
				}
			}
			return TRUE;
		}
		case WM_APPCOMMAND:
			switch (GET_APPCOMMAND_LPARAM(lParam))
			{
				case APPCOMMAND_MEDIA_PREVIOUSTRACK:
					if (((dwShuffle)?(pFileCollection->FileCount() > 1):
						(pFileCollection->IsFileAvailable(FCF_BACKWARD))))
					{
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_PREVIOUSFILE, 0), 0);
					}
					return TRUE;
				case APPCOMMAND_MEDIA_PLAY_PAUSE:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_PLAYPAUSE, 0), 0);
					return TRUE;
				case APPCOMMAND_MEDIA_STOP:
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_STOP, 0), 0);
					return TRUE;
				case APPCOMMAND_MEDIA_NEXTTRACK:
					if (((dwShuffle)?(pFileCollection->FileCount() > 1):
						(pFileCollection->IsFileAvailable(FCF_FORWARD))))
					{
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_PLAYBACK_NEXTFILE, 0), 0);
					}
					return TRUE;
				case APPCOMMAND_VOLUME_DOWN:
					if (pEngine->HasAudio())
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_DECREASE, 0), 0);
					return TRUE;
				case APPCOMMAND_VOLUME_UP:
					if (pEngine->HasAudio())
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_INCREASE, 0), 0);
					return TRUE;
				case APPCOMMAND_VOLUME_MUTE:
					if (pEngine->HasAudio())
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_MUTE, 0), 0);
					return TRUE;
			}
			return FALSE;
		case WM_MOUSEMOVE:
			if ((wParam & MK_LBUTTON) == MK_LBUTTON)
			{
				ReleaseCapture();
				SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
			}
			return TRUE;
		case WM_XBUTTONDOWN:
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
			return TRUE;
		case WM_MOUSEWHEEL:
			if (pEngine->HasAudio())
			{
				if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
				{
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_DECREASE, 0), 0);
				}
				else
				{
					PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_VOLUME_INCREASE, 0), 0);
				}
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
					ShellExecute(0, L"Open", lpAppPath, 0, 0, SW_NORMAL);
					break;
				case IDM_FILE_OPENFILES:
				case IDM_FILE_ADDFILES:
				{
					BOOL bOpen = (LOWORD(wParam) == IDM_FILE_OPENFILES);
					WCHAR lpODFile[APP_OD_MS_MAX_FILE] = {};
					WCHAR lpODFilter[APP_OD_MAX_FILTER] = {};
					WCHAR lpExt[64] = {};
					LPWSTR pFiles[FC_MAX_FILES];
					ULONG i, lODFileCnt = 0, lFileCnt = 0;
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
					if (GetOpenDialog(hAppInstance, hWnd, (bOpen)?L"Open File(s)":L"Add File(s)",
						lpODFile, APP_OD_MS_MAX_FILE - 1, lpODFilter, 1, TRUE, lpRecentDir))
					{
						if (bOpen) pFileCollection->Clear();
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
								SP_ExtractRightPart(lpFile, lpExt, '.');
								if (SFT_IsMemberOfCategory(lpExt, SFTC_PLAYLIST))
								{
									lFileCnt += LoadPlaylist(lpFile);
								}
								else
								{
									pFileCollection->AppendFile(lpFile);
									lFileCnt++;
								}
							}
							for (i = 0; i < lODFileCnt; i++)
								delete[] pFiles[i];
						}
						else
						{
							SP_ExtractDirectory(pFiles[0], lpRecentDir);
							SP_AddDirSep(lpRecentDir, lpRecentDir);
							SP_ExtractRightPart(pFiles[0], lpExt, '.');
							if (SFT_IsMemberOfCategory(lpExt, SFTC_PLAYLIST))
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
								if (!(GetAsyncKeyState(VK_SHIFT) >> 15))
									pEngine->Play(); //Auto-play
							}
							else
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
							}
						}
						else
						{
							if (bOpen)
							{
								if (pEngine->m_lpFileName) CloseTrack();
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
					WCHAR lpBFFPath[MAX_PATH] = {};
					if (GetBrowseForFolderDialog(hWnd, lpBFFPath, (bOpen)?L"Select directory to open"
						:L"Select directory to add", lpRecentDir))
					{
						wcscpy(lpRecentDir, lpBFFPath);
						if (bOpen) pFileCollection->Clear();
						if(LoadDirectory(lpBFFPath, bIncImages))
						{
							if (bOpen)
							{
								InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
								if (!(GetAsyncKeyState(VK_SHIFT) >> 15))
									pEngine->Play(); //Auto-play
							}
							else
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
							}
						}
						else 
						{
							if (bOpen) 
							{
								if (pEngine->m_lpFileName) CloseTrack();
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
					if (pEngine->m_lpFileName)
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
					WCHAR lpFile[MAX_PATH] = {};
					LPPLITEMDESC pPLID = 0;
					pFileCollection->GetUserData(pEngine->m_lpFileName, 0, FCF_BYFILENAME, (LONG_PTR &)pPLID);
					if (pPLID)
						delete pPLID;
					pFileCollection->DeleteFile(pEngine->m_lpFileName, 0, FCF_BYFILENAME);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSE, 0), 0);
					if (pFileCollection->GetFile(lpFile, 0, FCF_RECENT))
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
					WCHAR lpSDFile[MAX_PATH] = {};
					DWORD dwFilterIndex = 1;
					wcscpy(lpSDFile, pEngine->m_lpFileName);
					if (GetSaveDialog(hAppInstance, hWnd, L"Save File As", lpSDFile,
						MAX_PATH - 1, L"Any file\0*.*;\0", &dwFilterIndex, 0, lpRecentDir))
					{
						SP_ExtractDirectory(lpSDFile, lpRecentDir);
						SP_AddDirSep(lpRecentDir, lpRecentDir);
						if (_wcsicmp(pEngine->m_lpFileName, lpSDFile) != 0)
							CopyFile(pEngine->m_lpFileName, lpSDFile, FALSE);
					}
					break;
				}
				case IDM_FILE_SAVEPLAYLISTAS:
				{
					WCHAR lpSDFile[MAX_PATH] = {};
					WCHAR lpExt[64] = {};
					DWORD dwFilterIndex = 1;
					if (GetSaveDialog(hAppInstance, hWnd, L"Save Playlist As", lpSDFile,
						MAX_PATH - 1, L"EBL Playlist\0*.ebl;\0M3U/M3U8 Playlist\0*.m3u; *.m3u8;\0"
						L"ASX Playlist\0*.asx;\0", &dwFilterIndex, 0, lpRecentDir))
					{
						SP_ExtractDirectory(lpSDFile, lpRecentDir);
						SP_AddDirSep(lpRecentDir, lpRecentDir);
						SP_ExtractRightPart(lpSDFile, lpExt, '.');
						switch (dwFilterIndex)
						{
							case 1:
								if (_wcsicmp(lpExt, L"ebl") != 0)
									wcscat(lpSDFile, L".ebl");
								break;

							case 2:
								if (_wcsicmp(lpExt, L"m3u8") != 0)
								{
									if (_wcsicmp(lpExt, L"m3u") != 0)
										wcscat(lpSDFile, L".m3u");
								}
								break;
							case 3:
								if (_wcsicmp(lpExt, L"asx") != 0)
									wcscat(lpSDFile, L".asx");
								break;
						}
						SavePlaylist(lpSDFile);
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
					/*if (hVideoWnd)
						UpdateBorderStyle(hVideoWnd);*/
					break;
				case IDM_WB_TOOLWINDOW:
					dwWindowBorderIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_WB_NORMALWINDOW,
						IDM_WB_TOOLWINDOW, IDM_WB_TOOLWINDOW, MF_BYCOMMAND);
					UpdateBorderStyle();
					UpdateMainControlsState();
					UpdateBorderStyle(hPlaylistWnd);
					/*if (hVideoWnd)
						UpdateBorderStyle(hVideoWnd);*/
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
					if (pEngine->m_lpFileName)
						UpdateCFTitle(pEngine->m_lpFileName, TRUE);
					break;
				case IDM_TB_DISPLAYFILENAMEONLY:
					dwTitleBarIndex = 1;
					CheckMenuRadioItem(GetMenu(hWnd), IDM_TB_DISPLAYFULLPATH,
						IDM_TB_DISPLAYFILENAMEONLY, IDM_TB_DISPLAYFILENAMEONLY, MF_BYCOMMAND);
					if (pEngine->m_lpFileName)
						UpdateCFTitle(pEngine->m_lpFileName, TRUE);
					break;
				case IDM_TB_DONOTCHANGETITLE:
					dwTBDoNotChangeTitle = !dwTBDoNotChangeTitle;
					CheckMenuItem(GetMenu(hWnd), IDM_TB_DONOTCHANGETITLE, MF_BYCOMMAND |
						(dwTBDoNotChangeTitle)?MF_CHECKED:MF_UNCHECKED);
					if (pEngine->m_lpFileName)
						UpdateCFTitle(pEngine->m_lpFileName, TRUE);
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
					CheckMenuItem(GetMenu(hWnd), IDM_OPACITY_OPAQUEONFOCUS, MF_BYCOMMAND |
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
					CheckMenuItem(GetMenu(hWnd), IDM_AS_RESUMEPLAYBACK, MF_BYCOMMAND |
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
								MessageBeep(-1);
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
					intVol = (int)SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
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
					intVol = (int)SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
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
					intBal = (int)SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
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
					intBal = (int)SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
					SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETRANGE, (WPARAM)&intMin,
						(LPARAM)&intMax);
					intBal++;
					CheckBounds((long *)&intBal, (long)intMin, (long)intMax);
					SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_SETPOS, TRUE, intBal);
					break;
				}
				case IDM_PLAYBACK_SAVECURRENTFRAME:
				{
					WCHAR lpSDFile[MAX_PATH] = {};
					WCHAR lpName[128] = {};
					WCHAR lpExt[64] = {};
					DWORD dwFilterIndex = 1;
					int intPos;
					ENGINESTATE eState = pEngine->GetState();
					if (eState != E_STATE_PAUSED)
					{
						PI.psSource = PS_OTHER;
						pEngine->Pause();
					}
					SP_ExtractName(pEngine->m_lpFileName, lpName);
					SP_ExtractLeftPart(lpName, lpName, '.');
					intPos = (int)pEngine->GetPosition();
					if (((intPos / 1000) / 3600))
					{
						swprintf(lpSDFile, L"%s_%02i%02i%02i.bmp", lpName, (intPos / 1000) / 3600,
							((intPos / 1000) / 60) % 60, ((intPos / 1000) % 60));
					}
					else
					{
						swprintf(lpSDFile, L"%s_%02i%02i.bmp", lpName, ((intPos / 1000) / 60) % 60,
							((intPos / 1000) % 60));
					}
					if (GetSaveDialog(hAppInstance, hWnd, L"Save Frame As", lpSDFile,
						MAX_PATH - 1, L"Windows Bitmap\0*.bmp;\0", &dwFilterIndex, 0, lpRecentDir))
					{
						SP_ExtractDirectory(lpSDFile, lpRecentDir);
						SP_AddDirSep(lpRecentDir, lpRecentDir);
						SP_ExtractRightPart(lpSDFile, lpExt, '.');
						if (_wcsicmp(lpExt, L"bmp") != 0)
							wcscat(lpSDFile, L".bmp");
						pEngine->SaveCurrentFrame(lpSDFile);
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
							VIDEOMODE VM = {};
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
					WCHAR lpHHStr[360] = {};
					WCHAR lpAppDir[MAX_PATH] = {};
					SP_ExtractDirectory(lpAppPath, lpAppDir);
					SP_AddDirSep(lpAppDir, lpAppDir);
					swprintf(lpHHStr, L"%s%s::/%s", lpAppDir, APP_HELP_FILE,
						APP_HELP_PAGE_MAIN);
					if (HtmlHelp(hWnd, lpHHStr, HH_DISPLAY_TOPIC, 0) == 0)
					{
						LPWSTR lpMsg = new WCHAR[128];
						swprintf(lpMsg, L"Unable to open \"%s\"!", APP_HELP_FILE);
						MessageBox(hWnd, lpMsg, APP_NAME, MB_ICONEXCLAMATION);
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
				BOOL bMenuFlag1 = (pEngine->m_lpFileName != 0);
				BOOL bMenuFlag2 = (pEngine->IsSeekable() != 0);
				BOOL bMenuFlag3 = (pEngine->CanStep(1) != 0);
				BOOL bMenuFlag4 = (pEngine->HasAudio() != 0);
				BOOL bMenuFlag5 = (pEngine->HasVideo() != 0);
				BOOL bMenuFlag6 = IsWindowVisible(hWnd);
				BOOL bMenuFlag7 = (pEngine->GetState() != E_STATE_STOPPED);
				BOOL bMenuFlag8 = (VWD.dwVWPosFlag == VWPF_NORMAL);
				HMENU hMainMenu = GetMenu(hWnd);
				EnableMenuItem(hMainMenu, IDM_FILE_NEWPLAYER,
					(dwMultipleInstances)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FILE_REOPENCURRENT,
					(pFileCollection->FileCount())?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FILE_CLOSE,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FILE_DELETEFROMPLAYLIST,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FILE_SAVEFILEAS,
					(bMenuFlag1 && !IsURL(pEngine->m_lpFileName))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FILE_SAVEPLAYLISTAS,
					(bMenuFlag1 || (pFileCollection->FileCount()))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FILE_PROPERTIES,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FILE_EXIT,
					(dwMultipleInstances)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_INTERFACE_DEFINECOLORS,
					(!dwUseSystemColors)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_WB_NORMALWINDOW,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_WB_TOOLWINDOW,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_VIEW_MAINCONTROLS,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_TB_DISPLAYFULLPATH,
					(bMenuFlag6 && !dwTBDoNotChangeTitle)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_TB_DISPLAYFILENAMEONLY,
					(bMenuFlag6 && !dwTBDoNotChangeTitle)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_TB_DONOTCHANGETITLE,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_INTERFACE_USESYSTEMCOLORS,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_INTERFACE_DEFINECOLORS,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_ONTOP_NEVER,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_ONTOP_ALWAYS,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_ONTOP_WHILEPLAYING,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PAS_SCREENCENTER,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PAS_RESTOREPREVIOUS,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PAS_RANDOM,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_OPACITY_100,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_OPACITY_75,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_OPACITY_50,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_OPACITY_25,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_OPACITY_CUSTOM,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_OPACITY_OPAQUEONFOCUS,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_VIEW_TRAYICON,
					(bMenuFlag6)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AS_RESUMEPLAYBACK,
					(dwRememberPlaylist && !dwShuffle)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PLAYBACK_PREVIOUSFILE,
					((dwShuffle)?(pFileCollection->FileCount() > 1):
					(pFileCollection->IsFileAvailable(FCF_BACKWARD)))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PLAYBACK_NEXTFILE,
					((dwShuffle)?(pFileCollection->FileCount() > 1):
					(pFileCollection->IsFileAvailable(FCF_FORWARD)))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PLAYBACK_PLAYPAUSE,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PLAYBACK_STOP,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PLAYBACK_FRAMESTEP,
					(bMenuFlag3 && bMenuFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_RATE_NORMAL,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_RATE_DECREASE,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_RATE_INCREASE,
					(bMenuFlag1)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SEEK_STEPBACKWARD,
					((bMenuFlag1 && bMenuFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SEEK_STEPFORWARD,
					((bMenuFlag1 && bMenuFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SEEK_JUMPBACKWARD,
					((bMenuFlag1 && bMenuFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SEEK_JUMPFORWARD,
					((bMenuFlag1 && bMenuFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SEEK_LONGJUMPBACKWARD,
					((bMenuFlag1 && bMenuFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SEEK_LONGJUMPFORWARD,
					((bMenuFlag1 && bMenuFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SEEK_REWIND,
					((bMenuFlag1 && bMenuFlag2))?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_VOLUME_MUTE,
					(bMenuFlag1 && bMenuFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_VOLUME_DECREASE,
					(bMenuFlag1 && bMenuFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_VOLUME_INCREASE,
					(bMenuFlag1 && bMenuFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_BALANCE_NORMAL,
					(bMenuFlag1 && bMenuFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_BALANCE_LEFT,
					(bMenuFlag1 && bMenuFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_BALANCE_RIGHT,
					(bMenuFlag1 && bMenuFlag4)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_PLAYBACK_SAVECURRENTFRAME,
					(bMenuFlag5 && bMenuFlag7)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AR_KEEPASPECTRATIO,
					(bMenuFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AR_DEFAULT,
					(bMenuFlag5 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AR_43,
					(bMenuFlag5 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AR_54,
					(bMenuFlag5 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AR_169,
					(bMenuFlag5 && dwKeepAspectRatio)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_ZOOM_HALFSIZE,
					(bMenuFlag5 && bMenuFlag8)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_ZOOM_NORMALSIZE,
					(bMenuFlag5 && bMenuFlag8)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_ZOOM_DOUBLESIZE,
					(bMenuFlag5 && bMenuFlag8)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_FULLSCREEN_FULLSCREENNORMAL,
					(bMenuFlag5)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_PARAMEQ,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_WAVESREVERB,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_GARGLE,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_DISTORTION,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_COMPRESSOR,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_ECHO,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_FLANGER,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_CHORUS,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_SAE_I3DL2REVERB,
					(dwUseDMO)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AP_CLOSEWINDOW,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AP_EXIT,
					((dwRepeatIndex == 0) && dwMultipleInstances)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AP_STANDBY,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AP_HIBERNATE,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AP_SHUTDOWN,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AP_LOGOFF,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
				EnableMenuItem(hMainMenu, IDM_AP_DONOTHING,
					(dwRepeatIndex == 0)?MF_ENABLED:MF_DISABLED | MF_GRAYED);
			}
			return TRUE;
		case WM_COPYDATA:
		{
			COPYDATASTRUCT *pCDS = (COPYDATASTRUCT *)lParam;
			LPWSTR lpCmdLine = new WCHAR[(pCDS->cbData / sizeof(WCHAR)) + 1];
			ULONG lCmdLnFileCnt;
			BOOL bCmdLnAdd = FALSE;
			if (IsWindowVisible(hWnd) && IsIconic(hWnd))
			{
				ShowWindow(hWnd, SW_RESTORE);
			}
			else if (dwTrayIcon && !IsWindowVisible(hWnd))
			{
				SendMessage(hTrayCBWnd, WM_COMMAND, MAKEWPARAM(IDM_TRAY_HIDESHOW, 0), 0);
			}
			ZeroMemory(lpCmdLine, pCDS->cbData + sizeof(WCHAR));
			wcsncpy(lpCmdLine, (LPWSTR)pCDS->lpData, pCDS->cbData / sizeof(WCHAR));
			lCmdLnFileCnt = ReadCommandLine(lpCmdLine, &bCmdLnAdd);
			if (lCmdLnFileCnt)
			{
				if (!bCmdLnAdd)
				{
					InitTrack((dwShuffle)?FCF_RANDOM:FCF_FIRST);
				}
				else
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
					if (pEngine->m_lpFileName) CloseTrack();
				}
			}
			delete[] lpCmdLine;
			return TRUE;
		}
		case WM_DROPFILES:
		{
			WCHAR lpDFile[MAX_PATH] = {};
			WCHAR lpExt[64] = {};
			ULONG lDrFileCnt = 0, lFileCnt = 0;
			HDROP hDrop = (HDROP)wParam;
			lDrFileCnt = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);
			for (ULONG i = 0; i < lDrFileCnt; i++)
			{
				DragQueryFile(hDrop, i, lpDFile, MAX_PATH);
				if (i == 0)
				{
					SP_ExtractDirectory(lpDFile, lpRecentDir);
					SP_AddDirSep(lpRecentDir, lpRecentDir);
				}
				SP_ExtractRightPart(lpDFile, lpExt, '.');
				if (SFT_IsMemberOfCategory(lpExt, SFTC_PLAYLIST))
				{
					lFileCnt += LoadPlaylist(lpDFile);
				}
				else if (IsDirectory(lpDFile))
				{
					lFileCnt += LoadDirectory(lpDFile);
				}
				else
				{
					pFileCollection->AppendFile(lpDFile);
					lFileCnt++;
				}
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
			}
			DragFinish(hDrop);
			return TRUE;
		}
		/*case WM_SYSCOMMAND:
		{
			WPARAM wP = (wParam & 0xFFF0);
			if ((wP == SC_MONITORPOWER) || (wP == SC_SCREENSAVE))
			{
				if (pEngine->HasVideo())
					return 0;
			}
			break;
		}*/
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
					RECT RCW = {};
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
							int intPos = (int)SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_GETPOS, 0, 0);
							if (pEngine->HasVideo())
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
							intVol = (int)SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
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
							intBal = (int)SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
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
						if (pEngine->HasVideo())
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
						if (pEngine->HasVideo())
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
			switch (wParam)
			{
				case 1:
				{
					int intPos = 0, intLen = 0;
					int intMin = 0, intMax = 0, intCur = 0, intTmp = 0;
					BOOL bCtlsFlag1 = (pEngine->m_lpFileName != 0);
					BOOL bCtlsFlag2 = (pEngine->IsSeekable() != 0);
					BOOL bCtlsFlag3 = (pEngine->HasAudio() != 0);
					BOOL bEndOfPlayback;
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
					EnableWindow(GetDlgItem(hWnd, IDC_EBBMUT), bCtlsFlag1 && bCtlsFlag3);
					EnableWindow(GetDlgItem(hWnd, IDC_EBSLDVOL), bCtlsFlag1 && bCtlsFlag3);
					EnableWindow(GetDlgItem(hWnd, IDC_EBBBN), bCtlsFlag1 && bCtlsFlag3);
					EnableWindow(GetDlgItem(hWnd, IDC_EBSLDBAL), bCtlsFlag1 && bCtlsFlag3);
					//----------------------------------------------------------------
					if (pEngine->GetState() == E_STATE_PLAYING)
					{
						if (_wcsicmp((LPWSTR)GetProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_"), L"pause") != 0)
						{
							SendDlgItemMessage(hWnd, IDC_EBBPP, EBBM_SETBITMAP, 0,
								(LPARAM)LoadImage(hAppInstance, MAKEINTRESOURCE(IDB_PAUSE), IMAGE_BITMAP, 16, 15, 0));
							SetProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_", (HANDLE)L"pause");
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
					}
					ModifyActiveItemState();
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
								if (((intLen / 1000) / 3600))
								{
									swprintf(CI.lpBuffer, L"Position: %02i:%02i:%02i, Length: %02i:%02i:%02i, Rate: %.1f",
										(intPos / 1000) / 3600, ((intPos / 1000) / 60) % 60, ((intPos / 1000) % 60),
										(intLen / 1000) / 3600, ((intLen / 1000) / 60) % 60, ((intLen / 1000) % 60),
										pEngine->GetRate());
								}
								else
								{
									swprintf(CI.lpBuffer, L"Position: %02i:%02i, Length: %02i:%02i, Rate: %.1f", ((intPos / 1000)
										/ 60) % 60, ((intPos / 1000) % 60), ((intLen / 1000) / 60) % 60, ((intLen / 1000) % 60),
										pEngine->GetRate());
								}
							}
							else
							{
								wcscpy(CI.lpBuffer, L"Position: 00:00, Length: 00:00, Rate: 0.0");
							}
							break;
						case II_STATE:
							switch (pEngine->GetState())
							{
								case E_STATE_PLAYING:
									wcscpy(CI.lpBuffer, L"State: Playing");
									break;
								case E_STATE_PAUSED:
									wcscpy(CI.lpBuffer, L"State: Paused");
									break;
								case E_STATE_STOPPED:
									wcscpy(CI.lpBuffer, L"State: Stopped");
									break;
							}
							break;
						case II_TITLE:
							wcscpy(CI.lpBuffer, lpCurFileTitle);
							break;
						case II_POSITION:
							if (bCtlsFlag1 && bCtlsFlag2)
							{
								SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_GETRANGE, (WPARAM)&intMin,
									(LPARAM)&intMax);
								intCur = (int)SendDlgItemMessage(hWnd, IDC_EBSLDSEEK, EBSM_GETPOS, 0, 0);
								intTmp = (int)(((double)intCur / (double)intMax) * 100);
								swprintf(CI.lpBuffer, L"Position: %i%c", intTmp, '%');
							}
							else
							{
								wcscpy(CI.lpBuffer, L"Position: None");
							}
							break;
						case II_MUTE:
							if (bCtlsFlag1)
							{
								swprintf(CI.lpBuffer, L"Mute: %s", (dwMute)?L"On":L"Off");
							}
							else
							{
								wcscpy(CI.lpBuffer, L"Mute: None");
							}
							break;
						case II_VOLUME:
							if (bCtlsFlag1)
							{
								SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETRANGE, (WPARAM)&intMin,
									(LPARAM)&intMax);
								intCur = (int)SendDlgItemMessage(hWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
								intTmp = (int)(((double)intCur / (double)intMax) * 100);
								swprintf(CI.lpBuffer, L"Volume: %i%c", intTmp, '%');
							}
							else
							{
								wcscpy(CI.lpBuffer, L"Volume: None");
							}
							break;
						case II_BALANCE:
							if (bCtlsFlag1)
							{
								SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETRANGE, (WPARAM)&intMin,
									(LPARAM)&intMax);
								intCur = (int)SendDlgItemMessage(hWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
								intTmp = (int)(((double)intCur / (double)intMax) * 100);
								swprintf(CI.lpBuffer, L"Balance: %i%c", intTmp, '%');
							}
							else
							{
								wcscpy(CI.lpBuffer, L"Balance: None");
							}
							break;
					}
					SetDlgItemText(hWnd, IDC_EBDMAIN, CI.lpBuffer);
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
					break;
				}
				case 2:
					if (bCurAlpha < 255)
						bCurAlpha++;
					else
					{
						KillTimer(hWnd, 2);
						break;
					}
					SetLayeredWindowAttributes(hWnd, 0, bCurAlpha, LWA_ALPHA);
					SetLayeredWindowAttributes(hPlaylistWnd, 0, bCurAlpha, LWA_ALPHA);
					/*if (hVideoWnd)
						SetLayeredWindowAttributes(hVideoWnd, 0, bCurAlpha, LWA_ALPHA);*/
					break;
				case 3:
					if (bCurAlpha > (BYTE)(2.55 * dwOpacityLevel))
						bCurAlpha--;
					else
					{
						KillTimer(hWnd, 3);
						break;
					}
					SetLayeredWindowAttributes(hWnd, 0, bCurAlpha, LWA_ALPHA);
					SetLayeredWindowAttributes(hPlaylistWnd, 0, bCurAlpha, LWA_ALPHA);
					/*if (hVideoWnd)
						SetLayeredWindowAttributes(hVideoWnd, 0, bCurAlpha, LWA_ALPHA);*/
					break;
				case 4:
					if (pEngine->HasVideo())
					{
						EXECUTION_STATE esFlags = ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED;
						if (OSVI.dwMajorVersion >= 6)
							esFlags |= ES_AWAYMODE_REQUIRED;
						SetThreadExecutionState(esFlags);
					}
					break;
			}
			return TRUE;
		}
		case WM_CLOSE:
		{
			WINDOWPLACEMENT WP = {};
			GetWindowPlacement(hWnd, &WP);
			if (WP.showCmd == SW_NORMAL)
			{
				RECT RCW = {};
				GetWindowRect(hWnd, &RCW);
				ptMainWindowPos.x = RCW.left;
				ptMainWindowPos.y = RCW.top;
			}
			RemoveProp(GetDlgItem(hWnd, IDC_EBBPP), L"_icon_");
			KillTimer(hWnd, 1);

			if (dwRememberPlaylist)
			{
				if ((pEngine->m_lpFileName) || (pFileCollection->FileCount()))
				{
					SavePlaylist(lpPlPath);
					int intSelFile = pFileCollection->GetFileIndex(0, FCF_RECENT);
					dwSelectedFileIndex = (intSelFile >= 0)?intSelFile:0;
					//if (dwASResumePlayback)
					//{
						dwSFPosition = (pEngine->m_lpFileName)?(DWORD)pEngine->GetPosition():0;
						dwSFState = pEngine->GetState();
					//}
				}
				else
				{
					DeleteFile(lpPlPath);
					dwSelectedFileIndex = 0;
					dwSFPosition = 0;
					dwSFState = E_STATE_STOPPED;
				}
			}
			else
			{
				if (IsFile(lpPlPath))
				{
					DeleteFile(lpPlPath);
				}
				dwSelectedFileIndex = 0;
				dwSFPosition = 0;
				dwSFState = E_STATE_STOPPED;
			}
			if (pEngine->m_lpFileName) CloseTrack();

			if (dwTrayIcon) RemoveTrayIcon();
			pToolTipsMain->Destroy();
			if (!dwNoOwnerDrawMenu) pEBMenuMain->InitEBMenu(0);
			DestroyWindow(hPlaylistWnd);
			InitMainWnd(FALSE);

			return TRUE;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
	}
	return FALSE;
}

//Поиск окна программы в потоке
/*BOOL CALLBACK EnumThreadWndsProc(HWND hWnd, LPARAM lParam)
{
	WCHAR lpWndClass[64] = {};
	GetClassName(hWnd, lpWndClass, 64);
	if (_wcsicmp(lpWndClass, APP_MAIN_WND_CLASS) == 0)
	{
		*((BOOL*)lParam) = TRUE;
		return FALSE;
	}
	return TRUE;
}*/

void ApplySettings()
{
	HMENU hMainMenu = GetMenu(hMainWnd);
	CheckMenuItem(hMainMenu, IDM_FILE_MULTIPLEINSTANCES, MF_BYCOMMAND |
		(dwMultipleInstances)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_FILE_REMEMBERPLAYLIST, MF_BYCOMMAND |
		(dwRememberPlaylist)?MF_CHECKED:MF_UNCHECKED);
	switch (dwWindowBorderIndex)
	{
		case 0:
			CheckMenuRadioItem(hMainMenu, IDM_WB_NORMALWINDOW,
				IDM_WB_TOOLWINDOW, IDM_WB_NORMALWINDOW, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(hMainMenu, IDM_WB_NORMALWINDOW,
				IDM_WB_TOOLWINDOW, IDM_WB_TOOLWINDOW, MF_BYCOMMAND);
			break;
	}
	UpdateBorderStyle();
	UpdateBorderStyle(hPlaylistWnd);
	/*if (hVideoWnd)
		UpdateBorderStyle(hVideoWnd);*/
	CheckMenuItem(hMainMenu, IDM_VIEW_MAINCONTROLS, MF_BYCOMMAND |
		(dwMainControls)?MF_CHECKED:MF_UNCHECKED);
	UpdateMainControlsState();
	CheckMenuItem(hMainMenu, IDM_VIEW_PLAYLIST, MF_BYCOMMAND |
		(dwPlaylist)?MF_CHECKED:MF_UNCHECKED);
	//if (dwPlaylist)
	//	ShowWindow(hPlaylistWnd, SW_SHOW);
	switch (dwTitleBarIndex)
	{
		case 0:
			CheckMenuRadioItem(hMainMenu, IDM_TB_DISPLAYFULLPATH,
				IDM_TB_DISPLAYFILENAMEONLY, IDM_TB_DISPLAYFULLPATH, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(hMainMenu, IDM_TB_DISPLAYFULLPATH,
				IDM_TB_DISPLAYFILENAMEONLY, IDM_TB_DISPLAYFILENAMEONLY, MF_BYCOMMAND);
			break;
	}
	CheckMenuItem(hMainMenu, IDM_TB_DONOTCHANGETITLE, MF_BYCOMMAND |
		(dwTBDoNotChangeTitle)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_INTERFACE_USESYSTEMCOLORS, MF_BYCOMMAND |
		(dwUseSystemColors)?MF_CHECKED:MF_UNCHECKED);
	UpdateEBColors();
	switch (dwOnTopIndex)
	{
		case 0:
			CheckMenuRadioItem(hMainMenu, IDM_ONTOP_NEVER,
				IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_NEVER, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(hMainMenu, IDM_ONTOP_NEVER,
				IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_ALWAYS, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(hMainMenu, IDM_ONTOP_NEVER,
				IDM_ONTOP_WHILEPLAYING, IDM_ONTOP_WHILEPLAYING, MF_BYCOMMAND);
			break;
	}
	UpdateOnTopState();
	switch (dwPositionAtStartupIndex)
	{
		case 0:
			CheckMenuRadioItem(hMainMenu, IDM_PAS_SCREENCENTER,
				IDM_PAS_RANDOM, IDM_PAS_SCREENCENTER, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(hMainMenu, IDM_PAS_SCREENCENTER,
				IDM_PAS_RANDOM, IDM_PAS_RESTOREPREVIOUS, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(hMainMenu, IDM_PAS_SCREENCENTER,
				IDM_PAS_RANDOM, IDM_PAS_RANDOM, MF_BYCOMMAND);
			break;
	}
	UpdatePosition();
	switch (dwOpacityLevel)
	{
		case 100:
			CheckMenuRadioItem(hMainMenu, IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_100, MF_BYCOMMAND);
			break;
		case 75:
			CheckMenuRadioItem(hMainMenu, IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_75, MF_BYCOMMAND);
			break;
		case 50:
			CheckMenuRadioItem(hMainMenu, IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_50, MF_BYCOMMAND);
			break;
		case 25:
			CheckMenuRadioItem(hMainMenu, IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_25, MF_BYCOMMAND);
			break;
		default:
			CheckMenuRadioItem(hMainMenu, IDM_OPACITY_100,
				IDM_OPACITY_CUSTOM, IDM_OPACITY_CUSTOM, MF_BYCOMMAND);
			break;
	}
	UpdateOpacityState();
	UpdateOpacityState(hPlaylistWnd);
	/*if (hVideoWnd)
		UpdateOpacityState(hVideoWnd);*/
	CheckMenuItem(hMainMenu, IDM_OPACITY_OPAQUEONFOCUS, MF_BYCOMMAND |
		(dwOpaqueOnFocus)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_VIEW_TRAYICON, MF_BYCOMMAND |
		(dwTrayIcon)?MF_CHECKED:MF_UNCHECKED);
	if (dwTrayIcon) InitTrayIcon(); else RemoveTrayIcon();
	CheckMenuItem(hMainMenu, IDM_AS_RESUMEPLAYBACK, MF_BYCOMMAND |
		(dwASResumePlayback)?MF_CHECKED:MF_UNCHECKED);
	switch (dwRepeatIndex)
	{
		case 0:
			CheckMenuRadioItem(hMainMenu, IDM_REPEAT_REPEATOFF,
				IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATOFF, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(hMainMenu, IDM_REPEAT_REPEATOFF,
				IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATALL, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(hMainMenu, IDM_REPEAT_REPEATOFF,
				IDM_REPEAT_REPEATONE, IDM_REPEAT_REPEATONE, MF_BYCOMMAND);
			break;
	}
	CheckMenuItem(hMainMenu, IDM_PLAYBACK_SHUFFLE, MF_BYCOMMAND |
		(dwShuffle)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SEEK_SEEKBYKEYFRAMES, MF_BYCOMMAND |
		(dwSeekByKeyFrames)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_VOLUME_MUTE, MF_BYCOMMAND |
		(dwMute)?MF_CHECKED:MF_UNCHECKED);
	UpdateMuteButtonState();
	CheckMenuItem(hMainMenu, IDM_AR_KEEPASPECTRATIO, MF_BYCOMMAND |
		(dwKeepAspectRatio)?MF_CHECKED:MF_UNCHECKED);
	switch (dwAspectRatioIndex)
	{
		case 0:
			CheckMenuRadioItem(hMainMenu, IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_DEFAULT, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(hMainMenu, IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_43, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(hMainMenu, IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_54, MF_BYCOMMAND);
			break;
		case 3:
			CheckMenuRadioItem(hMainMenu, IDM_AR_DEFAULT,
				IDM_AR_169, IDM_AR_169, MF_BYCOMMAND);
			break;
	}
	CheckMenuItem(hMainMenu, IDM_PB_ADV_AGTROT, MF_BYCOMMAND |
		(dwAddGraphToROT)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_DMO_USEDMO, MF_BYCOMMAND |
		(dwUseDMO)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_PARAMEQ, MF_BYCOMMAND |
		(dwDMOAEParamEq)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_WAVESREVERB, MF_BYCOMMAND |
		(dwDMOAEWavesReverb)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_GARGLE, MF_BYCOMMAND |
		(dwDMOAEGargle)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_DISTORTION, MF_BYCOMMAND |
		(dwDMOAEDistortion)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_COMPRESSOR, MF_BYCOMMAND |
		(dwDMOAECompressor)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_ECHO, MF_BYCOMMAND |
		(dwDMOAEEcho)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_FLANGER, MF_BYCOMMAND |
		(dwDMOAEFlanger)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_CHORUS, MF_BYCOMMAND |
		(dwDMOAEChorus)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMainMenu, IDM_SAE_I3DL2REVERB, MF_BYCOMMAND |
		(dwDMOAEI3DL2Reverb)?MF_CHECKED:MF_UNCHECKED);
	switch (dwAfterPlaybackIndex)
	{
		case 0:
			CheckMenuRadioItem(hMainMenu, IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_CLOSEWINDOW, MF_BYCOMMAND);
			break;
		case 1:
			CheckMenuRadioItem(hMainMenu, IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_EXIT, MF_BYCOMMAND);
			break;
		case 2:
			CheckMenuRadioItem(hMainMenu, IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_STANDBY, MF_BYCOMMAND);
			break;
		case 3:
			CheckMenuRadioItem(hMainMenu, IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_HIBERNATE, MF_BYCOMMAND);
			break;
		case 4:
			CheckMenuRadioItem(hMainMenu, IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_SHUTDOWN, MF_BYCOMMAND);
			break;
		case 5:
			CheckMenuRadioItem(hMainMenu, IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_LOGOFF, MF_BYCOMMAND);
			break;
		case 6:
			CheckMenuRadioItem(hMainMenu, IDM_AP_CLOSEWINDOW, IDM_AP_DONOTHING,
				IDM_AP_DONOTHING, MF_BYCOMMAND);
			break;
	}
}

//Открытие и инициализация трека
//В случае ошибки функция вернет значение меньше нуля
LONG InitTrack(DWORD dwFCFlag, DWORD dwFCIndex)
{
	WCHAR lpFileName[MAX_PATH] = {};
	ULONG i = 0;
	LPPLITEMDESC pPLID = 0;
	if (!pFileCollection->NextFile(lpFileName, dwFCIndex, dwFCFlag)) return -1;
	//Если у нас уже что-то открыто, вызываем File->Close
	if (pEngine->m_lpFileName)
	{
		SendMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_CLOSE, 0), 0);
	}
	//Далее подготавливаем файл к воспроизведению...
	//-----------------------------------------------------
	pEngine->m_lpFileName = new WCHAR[MAX_PATH];
	wcscpy(pEngine->m_lpFileName, lpFileName);
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
			if (!lpExternalFilters[i]) break;
			pEngine->AddDSFilterToFilterGraph(lpExternalFilters[i]);
		}
	}
	//Добавление графа в 'ROT'
	if (dwAddGraphToROT) pEngine->AddFGToROT();
	pEngine->Open();
	if (pEngine->HasVideo())
	{
		if (dwTrayIcon && !IsWindowVisible(hMainWnd))
		{
			SendMessage(hTrayCBWnd, WM_COMMAND, MAKEWPARAM(IDM_TRAY_HIDESHOW, 0), 0);
		}
		hVideoWnd = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_VIDEO),
			hMainWnd, VideoDlgProc, 0);
		pEngine->SetVideoStyles(WS_CHILDWINDOW | WS_CLIPSIBLINGS, 0);
		pEngine->SetVideoOwner(hVideoWnd);
		//UpdateBorderStyle(hVideoWnd);
		//UpdateOpacityState(hVideoWnd);
		AutoMoveVideoDlg(hVideoWnd);
		ShowWindow(hVideoWnd, SW_SHOW);

		//Для предотвращения запуска хранителя экрана или перехода системы в спящий режим
		SetTimer(hMainWnd, 4, 30000, NULL);
	}
	SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETRANGE, 0, (LPARAM)pEngine->GetLength());
	if (pEngine->HasAudio())
	{
		int intTmp1 = 0, intTmp2 = 0;
		pEngine->SetMute(dwMute);
		intTmp1 = (int)SendDlgItemMessage(hMainWnd, IDC_EBSLDVOL, EBSM_GETPOS, 0, 0);
		intTmp2 = PercentsTodB_LogScale(intTmp1);
		pEngine->SetVolume(intTmp2);
		intTmp1 = (int)SendDlgItemMessage(hMainWnd, IDC_EBSLDBAL, EBSM_GETPOS, 0, 0);
		intTmp2 = PercentsTodB_LogScale((intTmp1 >= 0)?100 - intTmp1:100 - abs(intTmp1));
		intTmp2 = (intTmp1 >= 0)?abs(intTmp2):intTmp2;
		pEngine->SetBalance(intTmp2);
	}
	pFileCollection->GetUserData(0, 0, FCF_RECENT, (LONG_PTR &)pPLID);
	if (pPLID)
		delete pPLID;
	pPLID = new PLITEMDESC;
	ZeroMemory(pPLID, sizeof(PLITEMDESC));
	GetTitle(lpFileName, pPLID->lpTitle);
	pPLID->uDuration = (UINT)pEngine->GetLength();
	pFileCollection->SetUserData(0, 0, FCF_RECENT, (LONG_PTR)pPLID);
	SetActiveItem(lpFileName);
	UpdateCFTitle(lpFileName);
	return 0;
}

//Закрытие трека
void CloseTrack()
{
	if (pEngine->HasVideo())
	{
		if (VWD.dwVWPosFlag == VWPF_FULLSCREEN)
		{
			SendMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_FULLSCREEN_FULLSCREENNORMAL, 0), 0);
		}
		pEngine->SetVideoVisible(FALSE);
		pEngine->SetVideoOwner(0);
		DestroyWindow(hVideoWnd);
		hVideoWnd = 0;

		KillTimer(hMainWnd, 4);
	}
	if (pEngine->GetState() != E_STATE_STOPPED) pEngine->Stop();
	pEngine->Close();
	delete[] pEngine->m_lpFileName;
	pEngine->m_lpFileName = 0;
	SendDlgItemMessage(hMainWnd, IDC_EBSLDSEEK, EBSM_SETRANGE, 0, 0);
	UpdateCFTitle(0);
}

//Загрузка файлов из каталога, включая все его подкаталоги
DWORD LoadDirectory(LPCWSTR lpPath, BOOL bIncImages)
{
	WCHAR lpFFPath[MAX_PATH] = {};
	WCHAR lpFFMask[MAX_PATH] = {};
	WCHAR lpTmp[MAX_PATH] = {};
	WCHAR lpExt[64] = {};
	WIN32_FIND_DATA WFD = {};
	HANDLE hFindFile;
	BOOL bNext;
	ULONG i, lFileCnt = 0, lDirCnt = 0;
	SP_AddDirSep(lpPath, lpFFPath);
	//Сначала пытаемся найти поддерживаемые файлы
	for (i = 0; i < APP_SFT_UBOUND; i++)
	{
		if (wcslen(APP_SFT[i]) && (SFT_IsCategory(APP_SFT[i]) == SFTC_NONE))
		{
			swprintf(lpFFMask, L"%s*.%s", lpFFPath, APP_SFT[i]);
			hFindFile = FindFirstFile(lpFFMask, &WFD);
			if (hFindFile != INVALID_HANDLE_VALUE)
			{
				bNext = TRUE;
				while (bNext)
				{
					if (wcscmp(WFD.cFileName, L".") != 0 && wcscmp(WFD.cFileName, L"..") != 0)
					{
						swprintf(lpTmp, L"%s%s", lpFFPath, WFD.cFileName);
						SP_ExtractRightPart(lpTmp, lpExt, '.');
						if (SFT_IsMemberOfCategory(lpExt, SFTC_PLAYLIST))
						{
							lFileCnt += LoadPlaylist(lpTmp);
						}
						else
						{
							if (!bIncImages)
								if (SFT_IsMemberOfCategory(lpExt, SFTC_IMAGE))
									goto LD_NextFile;
							pFileCollection->AppendFile(lpTmp);
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
	swprintf(lpFFMask, L"%s*", lpFFPath);
	hFindFile = FindFirstFile(lpFFMask, &WFD);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		bNext = TRUE;
		while (bNext)
		{
			if (wcscmp(WFD.cFileName, L".") != 0 && wcscmp(WFD.cFileName, L"..") != 0)
			{
				swprintf(lpTmp, L"%s%s", lpFFPath, WFD.cFileName);
				if (IsDirectory(lpTmp))
				{
					lFileCnt += LoadDirectory(lpTmp);
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
		WNDCLASSEX WCEX = {};
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

void PPSetDefFileInfo(HWND hPPWnd, LPWSTR lpFileName)
{
	if (lpFileName)
	{
		WORD wIndex = 0;
		WCHAR lpIconPath[MAX_PATH] = {};
		WCHAR lpName[128] = {};
		SP_ExtractName(lpFileName, lpName);
		SP_ExtractLeftPart(lpName, lpName, '.');
		wcscpy(lpIconPath, lpFileName);
		HICON hFile = ExtractAssociatedIcon(hAppInstance, lpIconPath, &wIndex);
		if (hFile)
		{
			SendDlgItemMessage(hPPWnd, IDC_STCFILE, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hFile);
		}
		SendDlgItemMessage(hPPWnd, IDC_EDTFN, WM_SETTEXT, 0, (LPARAM)lpName);
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
	LONG_PTR lWndExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
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
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, lWndExStyle);
	SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE |
		SWP_NOMOVE | SWP_NOZORDER);
}

//Обновляет состояние видимости основных элементов управления
void UpdateMainControlsState()
{
	RECT RCW = {}, RCC = {};
	DWORD dwNewHeight;
	GetWindowRect(hMainWnd, &RCW);
	GetWindowRect(GetDlgItem(hMainWnd, IDC_EBBPP), &RCC);

	ShowWindow(GetDlgItem(hMainWnd, IDC_EBBMUT), (dwMainControls)?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBSLDVOL), (dwMainControls) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBBPF), (dwMainControls) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBBSB), (dwMainControls) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBBPP), (dwMainControls) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBBSF), (dwMainControls) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBBNF), (dwMainControls) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBSLDBAL), (dwMainControls) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMainWnd, IDC_EBBBN), (dwMainControls) ? SW_SHOW : SW_HIDE);

	ClientToScreen(hMainWnd, (POINT*)&RCC.left);
	ClientToScreen(hMainWnd, (POINT*)&RCC.right);

	if (dwMainControls)
		dwNewHeight = (RCW.bottom - RCW.top) + (RCC.bottom - RCC.top);
	else
		dwNewHeight = (RCW.bottom - RCW.top) - (RCC.bottom - RCC.top);

	SetWindowPos(hMainWnd, NULL, 0, 0, RCW.right - RCW.left, dwNewHeight, SWP_NOZORDER | SWP_NOMOVE);
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
	LONG_PTR lWndStyle = GetWindowLongPtr(hMainWnd, GWL_EXSTYLE);
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
	LONG_PTR lWndStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	if (dwOpacityLevel < 100)
	{
		if ((lWndStyle & WS_EX_LAYERED) != WS_EX_LAYERED)
		{
			lWndStyle |= WS_EX_LAYERED;
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, lWndStyle);
		}
	}
	else
	{
		if ((lWndStyle & WS_EX_LAYERED) == WS_EX_LAYERED)
		{
			lWndStyle ^= WS_EX_LAYERED;
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, lWndStyle);
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

void UpdateCFTitle(LPCWSTR lpFileName, BOOL bMainWndTitleOnly)
{
	if (!lpFileName)
	{
		if (!bMainWndTitleOnly)
		{
			wcscpy(lpCurFileTitle, L"None");
		}
		SetWindowText(hMainWnd, lpStdWndTitle);
	}
	else
	{
		WCHAR lpPath[MAX_PATH] = {};
		WCHAR lpName[128] = {};
		wcscpy(lpPath, lpFileName);
		if (!bMainWndTitleOnly)
		{
			GetTitle(lpPath, lpName);
			wcscpy(lpCurFileTitle, lpName);
		}
		if (dwTBDoNotChangeTitle)
		{
			SetWindowText(hMainWnd, lpStdWndTitle);
		}
		else
		{
			SHELLFLAGSTATE SFS = {};
			SHGetSettings(&SFS, SSF_SHOWEXTENSIONS);
			if (!SFS.fShowExtensions)
			{
				SP_ExtractLeftPart(lpPath, lpPath, '.');
			}
			switch (dwTitleBarIndex)
			{
				case 0:
					SetWindowText(hMainWnd, lpPath);
					break;
				case 1:
					SP_ExtractName(lpPath, lpName);
					SetWindowText(hMainWnd, lpName);
					break;
			}
		}
	}
	if (dwTrayIcon)
	{
		WCHAR lpTip[256] = {};
		swprintf(lpTip, L"%s\n%s", lpStdWndTitle, lpCurFileTitle);
		UpdateTrayIcon(lpTip);
		if (!dwNoBalloonTips)
		{
			if (IsIconic(hMainWnd))
			{
				if (lpFileName)
				{
					WCHAR lpBalloonText[256] = {};
					swprintf(lpBalloonText, L"%s\n%s", L"Now playing:", lpCurFileTitle);
					ShowBalloon(lpStdWndTitle, lpBalloonText, BI_INFORMATION, 15000);
				}
			}
		}
	}
}

void GetTitle(LPCWSTR lpFileName, LPWSTR lpResult)
{
	WCHAR lpName[128] = {};
	if ((pEngine->m_lpFileName) && (_wcsicmp(lpFileName, pEngine->m_lpFileName) == 0))
	{
		MEDIACONTENT MC = {};
		pEngine->UpdateFGFiltersArray();
		if (pEngine->GetMediaContent(&MC) < 0)
		{
			goto GetTitle_CreateFromFile;
		}
		else
		{
			SIZE_T szAL = wcslen(MC.Author);
			SIZE_T szTL = wcslen(MC.Title);
			if ((szAL == 0) && (szTL == 0))
			{
				goto GetTitle_CreateFromFile;
			}
			if (szAL)
			{
				swprintf(lpName, L"%s - %s", MC.Author, (szTL)?MC.Title:L"None");
			}
			else
			{
				wcscpy(lpName, MC.Title);
			}
		}
	}
	else
	{
GetTitle_CreateFromFile:
		if (IsURL(lpFileName))
		{
			wcsncpy(lpName, lpFileName, 124);
			wcscat(lpName, L"...");
		}
		else
		{
			SP_ExtractName(lpFileName, lpName);
			SP_ExtractLeftPart(lpName, lpName, '.');
		}
	}
	wcscpy(lpResult, lpName);
}