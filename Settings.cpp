//Здесь происходит инициализация, чтение и запись всех настроек программы

#include <windows.h>
#include <stdio.h>

#include "engine.h"
#include "easybar.h"
#include "settings.h"

DWORD dwMultipleInstances;
LPWSTR lpRecentDir;
LPWSTR lpRecentURL;
DWORD dwRememberPlaylist;
DWORD dwRememberPosition;
DWORD dwSelectedFileIndex;
DWORD dwSFPosition;
DWORD dwSFState;
DWORD dwWindowBorderIndex;
DWORD dwMainControls;
DWORD dwPlaylist;
DWORD dwTitleBarIndex;
DWORD dwTBDoNotChangeTitle;
DWORD dwUseSystemColors;
DWORD dwBackgroundColor;
DWORD dwGradientColor1;
DWORD dwGradientColor2;
DWORD dwBorderColor1;
DWORD dwBorderColor2;
DWORD dwTextColor;
DWORD dwTextShadowColor;
DWORD dwActiveItemTextColor;
DWORD dwOnTopIndex;
POINT ptMainWindowPos;
RECT rcPlaylistPos;
DWORD dwPositionAtStartupIndex;
DWORD dwOpacityLevel;
DWORD dwOpaqueOnFocus;
DWORD dwTrayIcon;
LPFAVFILE pFavorites[APP_MAX_STRINGS];
DWORD dwASResumePlayback;
DWORD dwRepeatIndex;
DWORD dwShuffle;
DWORD dwSeekValue1;
DWORD dwSeekValue2;
DWORD dwSeekValue3;
DWORD dwSeekByKeyFrames;
DWORD dwMute;
DWORD dwVolume;
DWORD dwBalance;
DWORD dwKeepAspectRatio;
DWORD dwAspectRatioIndex;
DWORD dwChangeFSVideoMode;
DWORD dwFSVideoModeWidth;
DWORD dwFSVideoModeHeight;
DWORD dwFSVideoModeBPP;
DWORD dwFSVideoModeDF;
DWORD dwAddGraphToROT;
DWORD dwUseDMO;
DWORD dwDMOAEParamEq;
DWORD dwDMOAEWavesReverb;
DWORD dwDMOAEGargle;
DWORD dwDMOAEDistortion;
DWORD dwDMOAECompressor;
DWORD dwDMOAEEcho;
DWORD dwDMOAEFlanger;
DWORD dwDMOAEChorus;
DWORD dwDMOAEI3DL2Reverb;
DWORD dwUseExternalFilters;
LPWSTR lpExternalFilters[APP_MAX_STRINGS];
DWORD dwAfterPlaybackIndex;

//Опциональные настройки
DWORD dwNoOwnerDrawMenu;
DWORD dwNoTransitionEffects;
DWORD dwNoAdvancedBackground;
DWORD dwNoBalloonTips;
DWORD dwNoAutomaticPause;

void SetDefaultValues()
{
	ULONG i;
	dwMultipleInstances = TRUE;
	lpRecentDir = new WCHAR[MAX_PATH];
	lpRecentURL = new WCHAR[MAX_PATH];
	dwRememberPlaylist = TRUE;
	dwSelectedFileIndex = 0;
	dwSFPosition = 0;
	dwSFState = E_STATE_STOPPED;
	dwWindowBorderIndex = 0;
	dwMainControls = TRUE;
	dwPlaylist = FALSE;
	dwTitleBarIndex = 1;
	dwTBDoNotChangeTitle = TRUE;
	dwUseSystemColors = FALSE;
	dwBackgroundColor = EB_BACKGROUND_COLOR;
	dwGradientColor1 = EB_GRADIENT_COLOR_1;
	dwGradientColor2 = EB_GRADIENT_COLOR_2;
	dwBorderColor1 = EB_BORDER_COLOR_1;
	dwBorderColor2 = EB_BORDER_COLOR_2;
	dwTextColor = EB_TEXT_COLOR;
	dwTextShadowColor = EB_TEXT_SHADOW_COLOR;
	dwActiveItemTextColor = EB_ACTIVE_ITEM_TEXT_COLOR;
	dwOnTopIndex = 0;
	ptMainWindowPos.x = 0;
	ptMainWindowPos.y = 0;
	SetRect(&rcPlaylistPos, 0, 0, 200, 560);
	dwPositionAtStartupIndex = 0;
	dwOpacityLevel = 100;
	dwOpaqueOnFocus = FALSE;
	dwTrayIcon = TRUE;
	for (i = 0; i < APP_MAX_STRINGS; i++)
		pFavorites[i] = NULL;
	dwASResumePlayback = FALSE;
	dwRepeatIndex = FALSE;
	dwShuffle = FALSE;
	dwSeekValue1 = EB_SEEK_VALUE_1;
	dwSeekValue2 = EB_SEEK_VALUE_2;
	dwSeekValue3 = EB_SEEK_VALUE_3;
	dwSeekByKeyFrames = TRUE;
	dwMute = FALSE;
	dwVolume = 90;
	dwBalance = 100;
	dwKeepAspectRatio = TRUE;
	dwAspectRatioIndex = 0;
	dwChangeFSVideoMode = FALSE;
	dwFSVideoModeWidth = 0;
	dwFSVideoModeHeight = 0;
	dwFSVideoModeBPP = 0;
	dwFSVideoModeDF = 0;
	dwAddGraphToROT = FALSE;
	dwUseDMO = TRUE;
	dwDMOAEParamEq = TRUE;
	dwDMOAEWavesReverb = FALSE;
	dwDMOAEGargle = FALSE;
	dwDMOAEDistortion = FALSE;
	dwDMOAECompressor = FALSE;
	dwDMOAEEcho = FALSE;
	dwDMOAEFlanger = FALSE;
	dwDMOAEChorus = FALSE;
	dwDMOAEI3DL2Reverb = FALSE;
	dwUseExternalFilters = FALSE;
	for (i = 0; i < APP_MAX_STRINGS; i++)
		lpExternalFilters[i] = NULL;
	dwAfterPlaybackIndex = 6;
	//-------------------------
	dwNoOwnerDrawMenu = FALSE;
	dwNoTransitionEffects = FALSE;
	dwNoAdvancedBackground = FALSE;
	dwNoBalloonTips = FALSE;
	dwNoAutomaticPause = FALSE;
}

void GetSettings()
{
	BOOL bNoSettings = FALSE;
	HKEY hGSKey, hGSSubKey;
	ULONG i, j, lSubKeys, lValues;
	WCHAR lpKeyName[64] = {};
	WCHAR lpData[MAX_PATH] = {};
	DWORD dwDWORDSIZE = 4/*sizeof(DWORD)*/, dwSZSIZE, dwBINSIZE;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_GENERAL_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hGSKey, L"MultipleInstances", 0, NULL, (LPBYTE)&dwMultipleInstances, &dwDWORDSIZE);
		ZeroMemory(lpData, sizeof(lpData));
		dwSZSIZE = sizeof(lpData);
		RegQueryValueEx(hGSKey, L"RecentDir", 0, NULL, (LPBYTE)lpData, &dwSZSIZE);
		lpRecentDir = new WCHAR[MAX_PATH];
		wcscpy(lpRecentDir, lpData);
		ZeroMemory(lpData, sizeof(lpData));
		dwSZSIZE = sizeof(lpData);
		RegQueryValueEx(hGSKey, L"RecentURL", 0, NULL, (LPBYTE)lpData, &dwSZSIZE);
		lpRecentURL = new WCHAR[MAX_PATH];
		wcscpy(lpRecentURL, lpData);
		RegQueryValueEx(hGSKey, L"RememberPlaylist", 0, NULL, (LPBYTE)&dwRememberPlaylist, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SelectedFile", 0, NULL, (LPBYTE)&dwSelectedFileIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SFPosition", 0, NULL, (LPBYTE)&dwSFPosition, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SFState", 0, NULL, (LPBYTE)&dwSFState, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"WindowBorderIndex", 0, NULL, (LPBYTE)&dwWindowBorderIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"MainControls", 0, NULL, (LPBYTE)&dwMainControls, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Playlist", 0, NULL, (LPBYTE)&dwPlaylist, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TitleBar", 0, NULL, (LPBYTE)&dwTitleBarIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TBDoNotChangeTitle", 0, NULL, (LPBYTE)&dwTBDoNotChangeTitle, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"UseSystemColors", 0, NULL, (LPBYTE)&dwUseSystemColors, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"BackgroundColor", 0, NULL, (LPBYTE)&dwBackgroundColor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"GradientColor1", 0, NULL, (LPBYTE)&dwGradientColor1, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"GradientColor2", 0, NULL, (LPBYTE)&dwGradientColor2, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"BorderColor1", 0, NULL, (LPBYTE)&dwBorderColor1, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"BorderColor2", 0, NULL, (LPBYTE)&dwBorderColor2, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TextColor", 0, NULL, (LPBYTE)&dwTextColor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TextShadowColor", 0, NULL, (LPBYTE)&dwTextShadowColor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"ActiveItemTextColor", 0, NULL, (LPBYTE)&dwActiveItemTextColor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"OnTop", 0, NULL, (LPBYTE)&dwOnTopIndex, &dwDWORDSIZE);
		dwBINSIZE = sizeof(POINT);
		RegQueryValueEx(hGSKey, L"MainWindowPos", 0, NULL, (LPBYTE)&ptMainWindowPos, &dwBINSIZE);
		dwBINSIZE = sizeof(RECT);
		RegQueryValueEx(hGSKey, L"PlaylistPos", 0, NULL, (LPBYTE)&rcPlaylistPos, &dwBINSIZE);
		RegQueryValueEx(hGSKey, L"PositionAtStartup", 0, NULL, (LPBYTE)&dwPositionAtStartupIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Opacity", 0, NULL, (LPBYTE)&dwOpacityLevel, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"OpaqueOnFocus", 0, NULL, (LPBYTE)&dwOpaqueOnFocus, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TrayIcon", 0, NULL, (LPBYTE)&dwTrayIcon, &dwDWORDSIZE);
		RegCloseKey(hGSKey);
	}
	else
	{
		bNoSettings = TRUE;
		goto ExitFunction;
	}
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_FAVORITES_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryInfoKey(hGSKey, NULL, NULL, NULL, &lSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		for (i = 0; i < lSubKeys; i++)
		{
			swprintf(lpKeyName, L"File_%i", i);
			if (RegOpenKeyEx(hGSKey, lpKeyName, 0, KEY_ALL_ACCESS, &hGSSubKey) == ERROR_SUCCESS)
			{
				for (j = 0; j < APP_MAX_STRINGS; j++)
				{
					if (pFavorites[j] == NULL)
					{
						pFavorites[j] = new FAVFILE;
						ZeroMemory(lpData, sizeof(lpData));
						dwSZSIZE = sizeof(lpData);
						RegQueryValueEx(hGSSubKey, L"Path", 0, NULL, (LPBYTE)lpData, &dwSZSIZE);
						pFavorites[j]->lpPath = new WCHAR[MAX_PATH];
						wcscpy(pFavorites[j]->lpPath, lpData);
						ZeroMemory(lpData, sizeof(lpData));
						dwSZSIZE = sizeof(lpData);
						RegQueryValueEx(hGSSubKey, L"DisplayName", 0, NULL, (LPBYTE)lpData, &dwSZSIZE);
						pFavorites[j]->lpDisplayName = new WCHAR[128];
						wcscpy(pFavorites[j]->lpDisplayName, lpData);
						break;
					}
				}
				RegCloseKey(hGSSubKey);
			}
		}
		RegCloseKey(hGSKey);
	}
	else
	{
		bNoSettings = TRUE;
		goto ExitFunction;
	}
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_PLAYBACK_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hGSKey, L"ASResumePlayback", 0, NULL, (LPBYTE)&dwASResumePlayback, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Repeat", 0, NULL, (LPBYTE)&dwRepeatIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Shuffle", 0, NULL, (LPBYTE)&dwShuffle, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekValue1", 0, NULL, (LPBYTE)&dwSeekValue1, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekValue2", 0, NULL, (LPBYTE)&dwSeekValue2, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekValue3", 0, NULL, (LPBYTE)&dwSeekValue3, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekByKeyFrames", 0, NULL, (LPBYTE)&dwSeekByKeyFrames, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Mute", 0, NULL, (LPBYTE)&dwMute, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Volume", 0, NULL, (LPBYTE)&dwVolume, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Balance", 0, NULL, (LPBYTE)&dwBalance, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"KeepAspectRatio", 0, NULL, (LPBYTE)&dwKeepAspectRatio, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"AspectRatio", 0, NULL, (LPBYTE)&dwAspectRatioIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"ChangeFSVideoMode", 0, NULL, (LPBYTE)&dwChangeFSVideoMode, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_Width", 0, NULL, (LPBYTE)&dwFSVideoModeWidth, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_Height", 0, NULL, (LPBYTE)&dwFSVideoModeHeight, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_BPP", 0, NULL, (LPBYTE)&dwFSVideoModeBPP, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_DF", 0, NULL, (LPBYTE)&dwFSVideoModeDF, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"AddGraphToROT", 0, NULL, (LPBYTE)&dwAddGraphToROT, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"AfterPlayback", 0, NULL, (LPBYTE)&dwAfterPlaybackIndex, &dwDWORDSIZE);
		RegCloseKey(hGSKey);
	}
	else
	{
		bNoSettings = TRUE;
		goto ExitFunction;
	}
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_DMO_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hGSKey, L"UseDMO", 0, NULL, (LPBYTE)&dwUseDMO, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_ParamEq", 0, NULL, (LPBYTE)&dwDMOAEParamEq, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_WavesReverb", 0, NULL, (LPBYTE)&dwDMOAEWavesReverb, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Gargle", 0, NULL, (LPBYTE)&dwDMOAEGargle, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Distortion", 0, NULL, (LPBYTE)&dwDMOAEDistortion, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Compressor", 0, NULL, (LPBYTE)&dwDMOAECompressor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Echo", 0, NULL, (LPBYTE)&dwDMOAEEcho, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Flanger", 0, NULL, (LPBYTE)&dwDMOAEFlanger, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Chorus", 0, NULL, (LPBYTE)&dwDMOAEChorus, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_I3DL2Reverb", 0, NULL, (LPBYTE)&dwDMOAEI3DL2Reverb, &dwDWORDSIZE);
		RegCloseKey(hGSKey);
	}
	else
	{
		bNoSettings = TRUE;
		goto ExitFunction;
	}
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_EF_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hGSKey, L"UseExternalFilters", 0, 0, (LPBYTE)&dwUseExternalFilters, &dwDWORDSIZE);
		RegQueryInfoKey(hGSKey, NULL, NULL, NULL, NULL, NULL, NULL, &lValues, NULL, NULL, NULL, NULL);
		for (i = 0; i < lValues; i++)
		{
			swprintf(lpKeyName, L"ExternalFilter_%i", i);
			ZeroMemory(lpData, sizeof(lpData));
			dwSZSIZE = sizeof(lpData);
			RegQueryValueEx(hGSKey, lpKeyName, 0, NULL, (LPBYTE)lpData, &dwSZSIZE);
			if (wcslen(lpData))
			{
				for (j = 0; j < APP_MAX_STRINGS; j++)
				{
					if (lpExternalFilters[j] == NULL)
					{
						lpExternalFilters[j] = new WCHAR[MAX_PATH];
						wcscpy(lpExternalFilters[j], lpData);
						break;
					}
				}
			}
		}
		RegCloseKey(hGSKey);
	}
	else
	{
		bNoSettings = TRUE;
		goto ExitFunction;
	}
	//----------------------------------------------------------------------
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_RESTRICTIONS_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hGSKey, L"NoOwnerDrawMenu", 0, NULL, (LPBYTE)&dwNoOwnerDrawMenu, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoTransitionEffects", 0, NULL, (LPBYTE)&dwNoTransitionEffects, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoAdvancedBackground", 0, NULL, (LPBYTE)&dwNoAdvancedBackground, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoBalloonTips", 0, NULL, (LPBYTE)&dwNoBalloonTips, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoAutomaticPause", 0, NULL, (LPBYTE)&dwNoAutomaticPause, &dwDWORDSIZE);
		RegCloseKey(hGSKey);
	}
	//----------------------------------------------------------------------
ExitFunction:
	if (bNoSettings)
	{
		
		if (lpRecentDir)
			delete[] lpRecentDir;
		if (lpRecentURL)
			delete[] lpRecentURL;

		int n;

		n = 0;
		while (pFavorites[n])
		{
			delete[] pFavorites[n]->lpPath;
			delete[] pFavorites[n]->lpDisplayName;

			delete pFavorites[n];

			pFavorites[n] = NULL;
			n++;
		};

		n = 0;
		while (lpExternalFilters[n])
		{
			delete[] lpExternalFilters[n];

			lpExternalFilters[n] = NULL;
			n++;
		};

		SetDefaultValues();
		SaveSettings();
	}
}

void SaveSettings()
{
	HKEY hSSKey, hSSSubKey;
	ULONG i;
	WCHAR lpKeyName[64] = {};
	DWORD dwDisposition;
	RegCreateKeyEx(HKEY_CURRENT_USER, EB_REG_GENERAL_PATH, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hSSKey, &dwDisposition);
	RegSetValueEx(hSSKey, L"MultipleInstances", 0, REG_DWORD, (CONST LPBYTE)&dwMultipleInstances, 4);
	RegSetValueEx(hSSKey, L"RecentDir", 0, REG_SZ, (CONST LPBYTE)lpRecentDir, (DWORD)(wcslen(lpRecentDir)
			+ 1) * sizeof(WCHAR));
	RegSetValueEx(hSSKey, L"RecentURL", 0, REG_SZ, (CONST LPBYTE)lpRecentURL, (DWORD)(wcslen(lpRecentURL)
		+ 1) * sizeof(WCHAR));
	RegSetValueEx(hSSKey, L"RememberPlaylist", 0, REG_DWORD, (CONST LPBYTE)&dwRememberPlaylist, 4);
	RegSetValueEx(hSSKey, L"SelectedFile", 0, REG_DWORD, (CONST LPBYTE)&dwSelectedFileIndex, 4);
	RegSetValueEx(hSSKey, L"SFPosition", 0, REG_DWORD, (CONST LPBYTE)&dwSFPosition, 4);
	RegSetValueEx(hSSKey, L"SFState", 0, REG_DWORD, (CONST LPBYTE)&dwSFState, 4);
	RegSetValueEx(hSSKey, L"WindowBorderIndex", 0, REG_DWORD, (CONST LPBYTE)&dwWindowBorderIndex, 4);
	RegSetValueEx(hSSKey, L"MainControls", 0, REG_DWORD, (CONST LPBYTE)&dwMainControls, 4);
	RegSetValueEx(hSSKey, L"Playlist", 0, REG_DWORD, (CONST LPBYTE)&dwPlaylist, 4);
	RegSetValueEx(hSSKey, L"TitleBar", 0, REG_DWORD, (CONST LPBYTE)&dwTitleBarIndex, 4);
	RegSetValueEx(hSSKey, L"TBDoNotChangeTitle", 0, REG_DWORD, (CONST LPBYTE)&dwTBDoNotChangeTitle, 4);
	RegSetValueEx(hSSKey, L"UseSystemColors", 0, REG_DWORD, (CONST LPBYTE)&dwUseSystemColors, 4);
	RegSetValueEx(hSSKey, L"BackgroundColor", 0, REG_DWORD, (CONST LPBYTE)&dwBackgroundColor, 4);
	RegSetValueEx(hSSKey, L"GradientColor1", 0, REG_DWORD, (CONST LPBYTE)&dwGradientColor1, 4);
	RegSetValueEx(hSSKey, L"GradientColor2", 0, REG_DWORD, (CONST LPBYTE)&dwGradientColor2, 4);
	RegSetValueEx(hSSKey, L"BorderColor1", 0, REG_DWORD, (CONST LPBYTE)&dwBorderColor1, 4);
	RegSetValueEx(hSSKey, L"BorderColor2", 0, REG_DWORD, (CONST LPBYTE)&dwBorderColor2, 4);
	RegSetValueEx(hSSKey, L"TextColor", 0, REG_DWORD, (CONST LPBYTE)&dwTextColor, 4);
	RegSetValueEx(hSSKey, L"TextShadowColor", 0, REG_DWORD, (CONST LPBYTE)&dwTextShadowColor, 4);
	RegSetValueEx(hSSKey, L"ActiveItemTextColor", 0, REG_DWORD, (CONST LPBYTE)&dwActiveItemTextColor, 4);
	RegSetValueEx(hSSKey, L"OnTop", 0, REG_DWORD, (CONST LPBYTE)&dwOnTopIndex, 4);
	RegSetValueEx(hSSKey, L"MainWindowPos", 0, REG_BINARY, (CONST LPBYTE)&ptMainWindowPos, sizeof(POINT));
	RegSetValueEx(hSSKey, L"PlaylistPos", 0, REG_BINARY, (CONST LPBYTE)&rcPlaylistPos, sizeof(RECT));
	RegSetValueEx(hSSKey, L"PositionAtStartup", 0, REG_DWORD, (CONST LPBYTE)&dwPositionAtStartupIndex, 4);
	RegSetValueEx(hSSKey, L"Opacity", 0, REG_DWORD, (CONST LPBYTE)&dwOpacityLevel, 4);
	RegSetValueEx(hSSKey, L"OpaqueOnFocus", 0, REG_DWORD, (CONST LPBYTE)&dwOpaqueOnFocus, 4);
	RegSetValueEx(hSSKey, L"TrayIcon", 0, REG_DWORD, (CONST LPBYTE)&dwTrayIcon, 4);
	RegCloseKey(hSSKey);
	RegCreateKeyEx(HKEY_CURRENT_USER, EB_REG_FAVORITES_PATH, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hSSKey, &dwDisposition);
	for (i = 0; i < APP_MAX_STRINGS; i++)
	{
		swprintf(lpKeyName, L"File_%i", i);
		RegDeleteKey(hSSKey, lpKeyName);
	}
	for (i = 0; i < APP_MAX_STRINGS; i++)
	{
		if (!pFavorites[i]) break;
		swprintf(lpKeyName, L"File_%i", i);
		RegCreateKeyEx(hSSKey, lpKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
			&hSSSubKey, &dwDisposition);
		RegSetValueEx(hSSSubKey, L"Path", 0, REG_SZ, (CONST LPBYTE)pFavorites[i]->lpPath,
			(DWORD)(wcslen(pFavorites[i]->lpPath) + 1) * sizeof(WCHAR));
		RegSetValueEx(hSSSubKey, L"DisplayName", 0, REG_SZ, (CONST LPBYTE)pFavorites[i]->lpDisplayName,
			(DWORD)(wcslen(pFavorites[i]->lpDisplayName) + 1) * sizeof(WCHAR));
		RegCloseKey(hSSSubKey);
	}
	RegCloseKey(hSSKey);
	RegCreateKeyEx(HKEY_CURRENT_USER, EB_REG_PLAYBACK_PATH, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hSSKey, &dwDisposition);
	RegSetValueEx(hSSKey, L"ASResumePlayback", 0, REG_DWORD, (CONST LPBYTE)&dwASResumePlayback, 4);
	RegSetValueEx(hSSKey, L"Repeat", 0, REG_DWORD, (CONST LPBYTE)&dwRepeatIndex, 4);
	RegSetValueEx(hSSKey, L"Shuffle", 0, REG_DWORD, (CONST LPBYTE)&dwShuffle, 4);
	RegSetValueEx(hSSKey, L"SeekValue1", 0, REG_DWORD, (CONST LPBYTE)&dwSeekValue1, 4);
	RegSetValueEx(hSSKey, L"SeekValue2", 0, REG_DWORD, (CONST LPBYTE)&dwSeekValue2, 4);
	RegSetValueEx(hSSKey, L"SeekValue3", 0, REG_DWORD, (CONST LPBYTE)&dwSeekValue3, 4);
	RegSetValueEx(hSSKey, L"SeekByKeyFrames", 0, REG_DWORD, (CONST LPBYTE)&dwSeekByKeyFrames, 4);
	RegSetValueEx(hSSKey, L"Mute", 0, REG_DWORD, (CONST LPBYTE)&dwMute, 4);
	RegSetValueEx(hSSKey, L"Volume", 0, REG_DWORD, (CONST LPBYTE)&dwVolume, 4);
	RegSetValueEx(hSSKey, L"Balance", 0, REG_DWORD, (CONST LPBYTE)&dwBalance, 4);
	RegSetValueEx(hSSKey, L"KeepAspectRatio", 0, REG_DWORD, (CONST LPBYTE)&dwKeepAspectRatio, 4);
	RegSetValueEx(hSSKey, L"AspectRatio", 0, REG_DWORD, (CONST LPBYTE)&dwAspectRatioIndex, 4);
	RegSetValueEx(hSSKey, L"ChangeFSVideoMode", 0, REG_DWORD, (CONST LPBYTE)&dwChangeFSVideoMode, 4);
	RegSetValueEx(hSSKey, L"FSVideoMode_Width", 0, REG_DWORD, (CONST LPBYTE)&dwFSVideoModeWidth, 4);
	RegSetValueEx(hSSKey, L"FSVideoMode_Height", 0, REG_DWORD, (CONST LPBYTE)&dwFSVideoModeHeight, 4);
	RegSetValueEx(hSSKey, L"FSVideoMode_BPP", 0, REG_DWORD, (CONST LPBYTE)&dwFSVideoModeBPP, 4);
	RegSetValueEx(hSSKey, L"FSVideoMode_DF", 0, REG_DWORD, (CONST LPBYTE)&dwFSVideoModeDF, 4);
	RegSetValueEx(hSSKey, L"AddGraphToROT", 0, REG_DWORD, (CONST LPBYTE)&dwAddGraphToROT, 4);
	RegSetValueEx(hSSKey, L"AfterPlayback", 0, REG_DWORD, (CONST LPBYTE)&dwAfterPlaybackIndex, 4);
	RegCloseKey(hSSKey);
	RegCreateKeyEx(HKEY_CURRENT_USER, EB_REG_DMO_PATH, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hSSKey, &dwDisposition);
	RegSetValueEx(hSSKey, L"UseDMO", 0, REG_DWORD, (CONST LPBYTE)&dwUseDMO, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_ParamEq", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEParamEq, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_WavesReverb", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEWavesReverb, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_Gargle", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEGargle, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_Distortion", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEDistortion, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_Compressor", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAECompressor, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_Echo", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEEcho, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_Flanger", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEFlanger, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_Chorus", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEChorus, 4);
	RegSetValueEx(hSSKey, L"DMO_AE_I3DL2Reverb", 0, REG_DWORD, (CONST LPBYTE)&dwDMOAEI3DL2Reverb, 4);
	RegCloseKey(hSSKey);
	RegCreateKeyEx(HKEY_CURRENT_USER, EB_REG_EF_PATH, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hSSKey, &dwDisposition);
	RegSetValueEx(hSSKey, L"UseExternalFilters", 0, REG_DWORD, (CONST LPBYTE)&dwUseExternalFilters, 4);
	for (i = 0; i < APP_MAX_STRINGS; i++)
	{
		swprintf(lpKeyName, L"ExternalFilter_%i", i);
		RegDeleteValue(hSSKey, lpKeyName);
	}
	for (i = 0; i < APP_MAX_STRINGS; i++)
	{
		if (!lpExternalFilters[i]) break;
		swprintf(lpKeyName, L"ExternalFilter_%i", i);
		RegSetValueEx(hSSKey, lpKeyName, 0, REG_SZ, (CONST LPBYTE)lpExternalFilters[i],
			(DWORD)(wcslen(lpExternalFilters[i]) + 1) * sizeof(WCHAR));
	}
	RegCloseKey(hSSKey);
}