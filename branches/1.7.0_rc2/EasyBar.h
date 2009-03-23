#ifndef EASYBAR_H
#define EASYBAR_H

#ifndef _WINDOWS_
#include <windows.h>
#endif

//Unicode-константы
#define APP_DEV_STAGE				L"RC2"

#define APP_NAME					L"EasyBar"
#define APP_COPYRIGHT				L"Copyright © Boris Vorontsov & Co., 2007 - 2009"
#define APP_EMAIL					L"borisvorontsov@gmail.com"

#define APP_LICENSE \
	L"This program comes with NO WARRANTY, to the extent permitted by law. " \
	L"You may redistribute it under the terms of the GNU General Public License. " \
	L"See the file named '%s' for details."

#define APP_LICENSE_FILE			L"License.txt"

#define APP_PLAYLIST_FILE			L"EasyBar.ebl"

#define APP_HELP_FILE				L"EasyBar.chm"
#define APP_HELP_PAGE_MAIN			L"Index.htm"

#define APP_CL_KEY_ADD				L"/add"

#define APP_MAIN_WND_CLASS			L"EASYBAR_"

//ANSI-константы
#define APP_NAME_A					"EasyBar"
#define APP_HELP_FILE_A				"EasyBar.chm"

//Максимальный размер буфера файла для общего диалога открытия файла(ов) с
//возможностью выбора множества файлов (MultiSelect)
#define APP_OD_MS_MAX_FILE			65535
//Максимальный размер буфера файла для общего диалога открытия файла(ов)
#define APP_OD_MAX_FILE				MAX_PATH
//Максимальный размер буфера фильтра для общего диалога открытия файла(ов)
#define APP_OD_MAX_FILTER			3072

#define APP_SFT_UBOUND				0x00000064

//Перечень типов файлов, которые проигрыватель может открыть
//----------------------------------------------------------------------------------
__declspec(selectany) LPCWSTR APP_SFT[APP_SFT_UBOUND] =
	{
		//---------------------------
		L"[VIDEO]",
		//---------------------------
		L"avi",
		L"qt",
		L"mov",
		L"mp4",
		L"3gp",
		L"mpg",
		L"mpeg",
		L"m1v",
		L"m2v",
		L"mp2",
		L"mpa",
		L"mpe",
		L"wmv",
		L"wvx",
		L"rv",
		L"vob",
		L"dat",
		L"mkv",
		L"divx",
		L"ogm",
		L"flv",
		L"ivf",
		//---------------------------
		L"[AUDIO]",
		//---------------------------
		L"aac",
		L"m4a",
		L"wav",
		L"mpa",
		L"mp3",
		L"m1a",
		L"m2a",
		L"mod",
		L"stm",
		L"s3m",
		L"xm",
		L"it",
		L"au",
		L"aif",
		L"aifc",
		L"aiff",
		L"ac3",
		L"snd",
		L"mka",
		L"ogg",
		L"wma",
		L"wax",
		L"ra",
		//---------------------------
		L"[MIDI]",
		//---------------------------
		L"mid",
		L"midi",
		L"rmi",
		L"kar",
		//---------------------------
		L"[IMAGE]",
		//---------------------------
		L"jpg",
		L"jpeg",
		L"bmp",
		L"gif",
		L"tga",
		//---------------------------
		L"[PLAYLIST]",
		//---------------------------
		L"ebl",
		L"asx",
		L"xspf",
		L"m3u",
		L"m3u8",
		L"pls",
		L"mpp",
		L"mpcpl",
		L"mls",
		L"lst",
		L"lap",
		L"bsl",
		L"wpl",
		//---------------------------
		L"[OTHER]",
		//---------------------------
		L"asf",
		L"wm",
		L"wmx",
		L"wmd",
		L"rm",
		L"ram",
		L"ogm",
		//---------------------------
		L"[RESERVED]",
		//---------------------------
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L""
	};

typedef enum _SFTCATEGORY
{
	SFTC_NONE = 0,
	SFTC_VIDEO = 1,
	SFTC_AUDIO = 2,
	SFTC_MIDI = 3,
	SFTC_IMAGE = 4,
	SFTC_PLAYLIST = 5,
	SFTC_OTHER = 6,
	SFTC_RESERVED = 7
} SFTCATEGORY;

__inline SFTCATEGORY SFT_IsCategory(LPCWSTR lpwItem)
{
	if (_wcsicmp(lpwItem, L"[VIDEO]") == 0) return SFTC_VIDEO;
	if (_wcsicmp(lpwItem, L"[AUDIO]") == 0) return SFTC_AUDIO;
	if (_wcsicmp(lpwItem, L"[MIDI]") == 0) return SFTC_MIDI;
	if (_wcsicmp(lpwItem, L"[IMAGE]") == 0) return SFTC_IMAGE;
	if (_wcsicmp(lpwItem, L"[PLAYLIST]") == 0) return SFTC_PLAYLIST;
	if (_wcsicmp(lpwItem, L"[OTHER]") == 0) return SFTC_OTHER;
	if (_wcsicmp(lpwItem, L"[RESERVED]") == 0) return SFTC_RESERVED;
	return SFTC_NONE;
}

__inline BOOL SFT_IsMemberOfCategory(LPCWSTR lpwItem, SFTCATEGORY cCategory)
{
	for (ULONG i = 0; i < APP_SFT_UBOUND; i++)
	{
		if (SFT_IsCategory(APP_SFT[i]) == cCategory)
		{
			for (ULONG j = i + 1; wcslen(APP_SFT[j]) &&
				(SFT_IsCategory(APP_SFT[j]) == SFTC_NONE); j++)
			{
				if (_wcsicmp(lpwItem, APP_SFT[j]) == 0) return TRUE;
			}
			break;
		}
	}
	return FALSE;
}
//----------------------------------------------------------------------------------

//Максимальная размерность для массива строк/структур со строками
//Используется модулем для работы с настройками
#define APP_MAX_STRINGS				1024

typedef struct _FAVFILE
{
	LPWSTR lpwPath;
	LPWSTR lpwDisplayName;
	DWORD dwReserved;
} FAVFILE, *LPFAVFILE;

typedef enum _PAUSESOURCE
{
	PS_ACTIVATE = 2,
	PS_SIZE_MINIMIZED = 4,
	PS_OTHER = 6
} PAUSESOURCE;

typedef struct _PAUSEINFO
{
	PAUSESOURCE psSource;
	DWORD dwReserved;
} PAUSEINFO, *LPPAUSEINFO;

//Служит для уведомления остальных экземпляров о факте инициализации
//		wParam - пока не используется
//		lParam - Thread Id
#define EBM_INITIALIZED				RegisterWindowMessage(L"EB_Message_Initialized")
//Служит для уведомления остальных экземпляров о факте активации главного окна программы
//		wParam - пока не используется
//		lParam - Thread Id
#define EBM_ACTIVATED				RegisterWindowMessage(L"EB_Message_Activated")
//Служит для групового завершения работы (меню 'Exit')
//		wParam - пока не используется
//		lParam - пока не используется
#define EBM_QUIT					RegisterWindowMessage(L"EB_Message_Quit")

//Цвета интерфейса (тема по умолчанию)
#define EB_BACKGROUND_COLOR			RGB(254, 191, 188)
#define EB_GRADIENT_COLOR_1			RGB(132, 27, 0)
#define EB_GRADIENT_COLOR_2			RGB(255, 52, 47)
#define EB_BORDER_COLOR_1			RGB(130, 26, 0)
#define EB_BORDER_COLOR_2			RGB(255, 135, 132)
#define EB_TEXT_COLOR				RGB(255, 238, 238)
#define EB_TEXT_SHADOW_COLOR		RGB(149, 14, 0)
#define EB_ACTIVE_ITEM_TEXT_COLOR	RGB(240, 60, 60)

//Интервалы времени для поиска
#define EB_SEEK_VALUE_1				0x0000000A
#define EB_SEEK_VALUE_2				0x0000003C
#define EB_SEEK_VALUE_3				0x0000012C

typedef struct _CURRENTINFO //Меняет информацию на дисплее
{
	DWORD dwInfoIndex;
	WCHAR lpwBuffer[128];
	DWORD dwTimer;
	DWORD dwTimeout;
} CURRENTINFO, *LPCURRENTINFO;

#define II_TIME						0x00000000
#define II_STATE					0x00000001
#define II_TITLE					0x00000002
#define II_POSITION					0x00000014
#define II_MUTE						0x00000015
#define II_VOLUME					0x00000016
#define II_BALANCE					0x00000017

#define MAKEINT64(v1, v2)			((__int64)(((__int32)(v1)) | ((__int64)((__int32)(v2))) << 32))
#define LOINT64(v)					((__int32)(v))
#define HIINT64(v)					((__int32)(((__int64)(v) >> 32) & 0xFFFFFFFF))

LONG ExceptionFilter(EXCEPTION_POINTERS *pEP);
DWORD ReadCommandLine(LPCWSTR lpwCmdLine, LPBOOL pAdd);
INT_PTR CALLBACK PlayerDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK EnumThreadWndsProc(HWND hWnd, LPARAM lParam);
void ApplySettings();
void UpdateToolTips(BOOL bRemove = FALSE);
void UpdateBorderStyle(HWND hTarget = 0);
void UpdateMainControlsState();
COLORREF GetMenuBarColor();
void UpdateEBColors();
void UpdatePosition();
void UpdateOnTopState();
void UpdateOpacityState(HWND hTarget = 0);
void UpdateMuteButtonState();
LONG InitTrack(DWORD dwFCFlag, DWORD dwFCIndex = 0);
void CloseTrack();
DWORD LoadDirectory(LPCWSTR lpwPath, BOOL bIncImages = FALSE);
void InitMainWnd(BOOL bCreate = TRUE);
void PPSetDefFileInfo(HWND hPPWnd, LPWSTR lpwFileName);
void UpdateCFTitle(LPCWSTR lpwFileName, BOOL bMainWndTitleOnly = FALSE);
void GetTitle(LPCWSTR lpwFileName, LPWSTR lpwResult);

//Глобальные переменные настроек из модуля 'Settings.cpp'
extern DWORD dwMultipleInstances;
extern LPWSTR lpwRecentDir;
extern LPWSTR lpwRecentURL;
extern DWORD dwRememberPlaylist;
extern DWORD dwSelectedFileIndex;
extern DWORD dwSFPosition;
extern DWORD dwSFState;
extern DWORD dwWindowBorderIndex;
extern DWORD dwMainControls;
extern DWORD dwPlaylist;
extern DWORD dwTitleBarIndex;
extern DWORD dwTBDoNotChangeTitle;
extern DWORD dwUseSystemColors;
extern DWORD dwBackgroundColor;
extern DWORD dwGradientColor1;
extern DWORD dwGradientColor2;
extern DWORD dwBorderColor1;
extern DWORD dwBorderColor2;
extern DWORD dwTextColor;
extern DWORD dwTextShadowColor;
extern DWORD dwActiveItemTextColor;
extern DWORD dwOnTopIndex;
extern POINT ptMainWindowPos;
extern RECT rcPlaylistPos;
extern DWORD dwPositionAtStartupIndex;
extern DWORD dwOpacityLevel;
extern DWORD dwOpaqueOnFocus;
extern DWORD dwTrayIcon;
extern LPFAVFILE pFavorites[APP_MAX_STRINGS];
extern DWORD dwASResumePlayback;
extern DWORD dwRepeatIndex;
extern DWORD dwShuffle;
extern DWORD dwSeekValue1;
extern DWORD dwSeekValue2;
extern DWORD dwSeekValue3;
extern DWORD dwSeekByKeyFrames;
extern DWORD dwMute;
extern DWORD dwVolume;
extern DWORD dwBalance;
extern DWORD dwKeepAspectRatio;
extern DWORD dwAspectRatioIndex;
extern DWORD dwChangeFSVideoMode;
extern DWORD dwFSVideoModeWidth;
extern DWORD dwFSVideoModeHeight;
extern DWORD dwFSVideoModeBPP;
extern DWORD dwFSVideoModeDF;
extern DWORD dwAddGraphToROT;
extern DWORD dwUseDMO;
extern DWORD dwDMOAEParamEq;
extern DWORD dwDMOAEWavesReverb;
extern DWORD dwDMOAEGargle;
extern DWORD dwDMOAEDistortion;
extern DWORD dwDMOAECompressor;
extern DWORD dwDMOAEEcho;
extern DWORD dwDMOAEFlanger;
extern DWORD dwDMOAEChorus;
extern DWORD dwDMOAEI3DL2Reverb;
extern DWORD dwUseExternalFilters;
extern LPWSTR lpwExternalFilters[APP_MAX_STRINGS];
extern DWORD dwAfterPlaybackIndex;

//Опциональные настройки
extern DWORD dwNoOwnerDrawMenu;
extern DWORD dwNoTransitionEffects;
extern DWORD dwNoAdvancedBackground;
extern DWORD dwNoBalloonTips;
extern DWORD dwNoAutomaticPause;

#endif