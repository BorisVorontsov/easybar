//Здесь происходит инициализация, чтение и запись всех настроек программы

#include <windows.h>
#include <stdio.h>

#include "engine.h"
#include "easybar.h"
#include "settings.h"

DWORD dwMultipleInstances;
LPWSTR lpwRecentURL;
DWORD dwRememberPlaylist;
DWORD dwRememberPosition;
DWORD dwSelectedFileIndex;
DWORD dwSFPosition;
DWORD dwSFState;
DWORD dwWindowBorderIndex;
DWORD dwMainControls;
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
DWORD dwOnTopIndex;
DWORD dwMainWindowLeft;
DWORD dwMainWindowTop;
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
LPWSTR lpwExternalFilters[APP_MAX_STRINGS];
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
	lpwRecentURL = 0;
	dwRememberPlaylist = TRUE;
	dwSelectedFileIndex = 0;
	dwSFPosition = 0;
	dwSFState = E_STATE_STOPPED;
	dwWindowBorderIndex = 0;
	dwMainControls = TRUE;
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
	dwOnTopIndex = 0;
	dwMainWindowLeft = 0;
	dwMainWindowTop = 0;
	dwPositionAtStartupIndex = 0;
	dwOpacityLevel = 100;
	dwOpaqueOnFocus = FALSE;
	dwTrayIcon = TRUE;
	for (i = 0; i < APP_MAX_STRINGS; i++)
		pFavorites[i] = 0;
	dwASResumePlayback = FALSE;
	dwRepeatIndex = FALSE;
	dwShuffle = FALSE;
	dwSeekValue1 = EB_SEEK_VALUE_1;
	dwSeekValue2 = EB_SEEK_VALUE_2;
	dwSeekValue3 = EB_SEEK_VALUE_3;
	dwSeekByKeyFrames = TRUE;
	dwMute = FALSE;
	dwVolume = 9000;
	dwBalance = 10000;
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
		lpwExternalFilters[i] = 0;
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
	WCHAR lpwKeyName[64] = { 0 };
	WCHAR lpwData[MAX_PATH] = { 0 };
	DWORD dwDWORDSIZE = 4; //sizeof(DWORD)
	DWORD dwSZSIZE;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_GENERAL_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hGSKey, L"MultipleInstances", 0, 0, (LPBYTE)&dwMultipleInstances, &dwDWORDSIZE);
		ZeroMemory(lpwData, sizeof(lpwData));
		dwSZSIZE = sizeof(lpwData);
		RegQueryValueEx(hGSKey, L"RecentURL", 0, 0, (LPBYTE)lpwData, &dwSZSIZE);
		lpwRecentURL = new WCHAR[MAX_PATH];
		wcscpy(lpwRecentURL, lpwData);
		RegQueryValueEx(hGSKey, L"RememberPlaylist", 0, 0, (LPBYTE)&dwRememberPlaylist, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SelectedFile", 0, 0, (LPBYTE)&dwSelectedFileIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SFPosition", 0, 0, (LPBYTE)&dwSFPosition, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SFState", 0, 0, (LPBYTE)&dwSFState, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"WindowBorderIndex", 0, 0, (LPBYTE)&dwWindowBorderIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"MainControls", 0, 0, (LPBYTE)&dwMainControls, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TitleBar", 0, 0, (LPBYTE)&dwTitleBarIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TBDoNotChangeTitle", 0, 0, (LPBYTE)&dwTBDoNotChangeTitle, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"UseSystemColors", 0, 0, (LPBYTE)&dwUseSystemColors, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"BackgroundColor", 0, 0, (LPBYTE)&dwBackgroundColor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"GradientColor1", 0, 0, (LPBYTE)&dwGradientColor1, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"GradientColor2", 0, 0, (LPBYTE)&dwGradientColor2, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"BorderColor1", 0, 0, (LPBYTE)&dwBorderColor1, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"BorderColor2", 0, 0, (LPBYTE)&dwBorderColor2, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TextColor", 0, 0, (LPBYTE)&dwTextColor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TextShadowColor", 0, 0, (LPBYTE)&dwTextShadowColor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"OnTop", 0, 0, (LPBYTE)&dwOnTopIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"MainWindowLeft", 0, 0, (LPBYTE)&dwMainWindowLeft, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"MainWindowTop", 0, 0, (LPBYTE)&dwMainWindowTop, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"PositionAtStartup", 0, 0, (LPBYTE)&dwPositionAtStartupIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Opacity", 0, 0, (LPBYTE)&dwOpacityLevel, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"OpaqueOnFocus", 0, 0, (LPBYTE)&dwOpaqueOnFocus, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"TrayIcon", 0, 0, (LPBYTE)&dwTrayIcon, &dwDWORDSIZE);
		RegCloseKey(hGSKey);
	}
	else
	{
		bNoSettings = TRUE;
		goto ExitFunction;
	}
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_FAVORITES_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryInfoKey(hGSKey, 0, 0, 0, &lSubKeys, 0, 0, 0, 0, 0, 0, 0);
		for (i = 0; i < lSubKeys; i++)
		{
			swprintf(lpwKeyName, L"File_%i", i);
			if (RegOpenKeyEx(hGSKey, lpwKeyName, 0, KEY_ALL_ACCESS, &hGSSubKey) == ERROR_SUCCESS)
			{
				for (j = 0; j < APP_MAX_STRINGS; j++)
				{
					if (pFavorites[j] == 0)
					{
						pFavorites[j] = new FAVFILE;
						ZeroMemory(lpwData, sizeof(lpwData));
						dwSZSIZE = sizeof(lpwData);
						RegQueryValueEx(hGSSubKey, L"Path", 0, 0, (LPBYTE)lpwData, &dwSZSIZE);
						pFavorites[j]->lpwPath = new WCHAR[MAX_PATH];
						wcscpy(pFavorites[j]->lpwPath, lpwData);
						ZeroMemory(lpwData, sizeof(lpwData));
						dwSZSIZE = sizeof(lpwData);
						RegQueryValueEx(hGSSubKey, L"DisplayName", 0, 0, (LPBYTE)lpwData, &dwSZSIZE);
						pFavorites[j]->lpwDisplayName = new WCHAR[128];
						wcscpy(pFavorites[j]->lpwDisplayName, lpwData);
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
		RegQueryValueEx(hGSKey, L"ASResumePlayback", 0, 0, (LPBYTE)&dwASResumePlayback, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Repeat", 0, 0, (LPBYTE)&dwRepeatIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Shuffle", 0, 0, (LPBYTE)&dwShuffle, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekValue1", 0, 0, (LPBYTE)&dwSeekValue1, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekValue2", 0, 0, (LPBYTE)&dwSeekValue2, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekValue3", 0, 0, (LPBYTE)&dwSeekValue3, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"SeekByKeyFrames", 0, 0, (LPBYTE)&dwSeekByKeyFrames, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Mute", 0, 0, (LPBYTE)&dwMute, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Volume", 0, 0, (LPBYTE)&dwVolume, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"Balance", 0, 0, (LPBYTE)&dwBalance, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"KeepAspectRatio", 0, 0, (LPBYTE)&dwKeepAspectRatio, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"AspectRatio", 0, 0, (LPBYTE)&dwAspectRatioIndex, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"ChangeFSVideoMode", 0, 0, (LPBYTE)&dwChangeFSVideoMode, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_Width", 0, 0, (LPBYTE)&dwFSVideoModeWidth, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_Height", 0, 0, (LPBYTE)&dwFSVideoModeHeight, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_BPP", 0, 0, (LPBYTE)&dwFSVideoModeBPP, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"FSVideoMode_DF", 0, 0, (LPBYTE)&dwFSVideoModeDF, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"AddGraphToROT", 0, 0, (LPBYTE)&dwAddGraphToROT, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"AfterPlayback", 0, 0, (LPBYTE)&dwAfterPlaybackIndex, &dwDWORDSIZE);
		RegCloseKey(hGSKey);
	}
	else
	{
		bNoSettings = TRUE;
		goto ExitFunction;
	}
	if (RegOpenKeyEx(HKEY_CURRENT_USER, EB_REG_DMO_PATH, 0, KEY_ALL_ACCESS, &hGSKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hGSKey, L"UseDMO", 0, 0, (LPBYTE)&dwUseDMO, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_ParamEq", 0, 0, (LPBYTE)&dwDMOAEParamEq, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_WavesReverb", 0, 0, (LPBYTE)&dwDMOAEWavesReverb, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Gargle", 0, 0, (LPBYTE)&dwDMOAEGargle, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Distortion", 0, 0, (LPBYTE)&dwDMOAEDistortion, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Compressor", 0, 0, (LPBYTE)&dwDMOAECompressor, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Echo", 0, 0, (LPBYTE)&dwDMOAEEcho, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Flanger", 0, 0, (LPBYTE)&dwDMOAEFlanger, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_Chorus", 0, 0, (LPBYTE)&dwDMOAEChorus, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"DMO_AE_I3DL2Reverb", 0, 0, (LPBYTE)&dwDMOAEI3DL2Reverb, &dwDWORDSIZE);
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
		RegQueryInfoKey(hGSKey, 0, 0, 0, 0, 0, 0, &lValues, 0, 0, 0, 0);
		for (i = 0; i < lValues; i++)
		{
			swprintf(lpwKeyName, L"ExternalFilter_%i", i);
			ZeroMemory(lpwData, sizeof(lpwData));
			dwSZSIZE = sizeof(lpwData);
			RegQueryValueEx(hGSKey, lpwKeyName, 0, 0, (LPBYTE)lpwData, &dwSZSIZE);
			if (wcslen(lpwData))
			{
				for (j = 0; j < APP_MAX_STRINGS; j++)
				{
					if (lpwExternalFilters[j] == 0)
					{
						lpwExternalFilters[j] = new WCHAR[MAX_PATH];
						wcscpy(lpwExternalFilters[j], lpwData);
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
		RegQueryValueEx(hGSKey, L"NoOwnerDrawMenu", 0, 0, (LPBYTE)&dwNoOwnerDrawMenu, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoTransitionEffects", 0, 0, (LPBYTE)&dwNoTransitionEffects, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoAdvancedBackground", 0, 0, (LPBYTE)&dwNoAdvancedBackground, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoBalloonTips", 0, 0, (LPBYTE)&dwNoBalloonTips, &dwDWORDSIZE);
		RegQueryValueEx(hGSKey, L"NoAutomaticPause", 0, 0, (LPBYTE)&dwNoAutomaticPause, &dwDWORDSIZE);
		RegCloseKey(hGSKey);
	}
	//----------------------------------------------------------------------
ExitFunction:
	if (bNoSettings)
	{
		SetDefaultValues();
		SaveSettings();
	}
}

void SaveSettings()
{
	HKEY hSSKey, hSSSubKey;
	ULONG i;
	WCHAR lpwKeyName[64] = { 0 };
	DWORD dwDisposition;
	RegCreateKeyEx(HKEY_CURRENT_USER, EB_REG_GENERAL_PATH, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hSSKey, &dwDisposition);
	RegSetValueEx(hSSKey, L"MultipleInstances", 0, REG_DWORD, (CONST LPBYTE)&dwMultipleInstances, 4);
	if (lpwRecentURL)
	{
		RegSetValueEx(hSSKey, L"RecentURL", 0, REG_SZ, (CONST LPBYTE)lpwRecentURL, (wcslen(lpwRecentURL)
			+ 1) * sizeof(WCHAR));
	}
	RegSetValueEx(hSSKey, L"RememberPlaylist", 0, REG_DWORD, (CONST LPBYTE)&dwRememberPlaylist, 4);
	RegSetValueEx(hSSKey, L"SelectedFile", 0, REG_DWORD, (CONST LPBYTE)&dwSelectedFileIndex, 4);
	RegSetValueEx(hSSKey, L"SFPosition", 0, REG_DWORD, (CONST LPBYTE)&dwSFPosition, 4);
	RegSetValueEx(hSSKey, L"SFState", 0, REG_DWORD, (CONST LPBYTE)&dwSFState, 4);
	RegSetValueEx(hSSKey, L"WindowBorderIndex", 0, REG_DWORD, (CONST LPBYTE)&dwWindowBorderIndex, 4);
	RegSetValueEx(hSSKey, L"MainControls", 0, REG_DWORD, (CONST LPBYTE)&dwMainControls, 4);
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
	RegSetValueEx(hSSKey, L"OnTop", 0, REG_DWORD, (CONST LPBYTE)&dwOnTopIndex, 4);
	RegSetValueEx(hSSKey, L"MainWindowLeft", 0, REG_DWORD, (CONST LPBYTE)&dwMainWindowLeft, 4);
	RegSetValueEx(hSSKey, L"MainWindowTop", 0, REG_DWORD, (CONST LPBYTE)&dwMainWindowTop, 4);
	RegSetValueEx(hSSKey, L"PositionAtStartup", 0, REG_DWORD, (CONST LPBYTE)&dwPositionAtStartupIndex, 4);
	RegSetValueEx(hSSKey, L"Opacity", 0, REG_DWORD, (CONST LPBYTE)&dwOpacityLevel, 4);
	RegSetValueEx(hSSKey, L"OpaqueOnFocus", 0, REG_DWORD, (CONST LPBYTE)&dwOpaqueOnFocus, 4);
	RegSetValueEx(hSSKey, L"TrayIcon", 0, REG_DWORD, (CONST LPBYTE)&dwTrayIcon, 4);
	RegCloseKey(hSSKey);
	RegCreateKeyEx(HKEY_CURRENT_USER, EB_REG_FAVORITES_PATH, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hSSKey, &dwDisposition);
	for (i = 0; i < APP_MAX_STRINGS; i++)
	{
		swprintf(lpwKeyName, L"File_%i", i);
		RegDeleteKey(hSSKey, lpwKeyName);
	}
	for (i = 0; i < APP_MAX_STRINGS; i++)
	{
		if (!pFavorites[i]) break;
		swprintf(lpwKeyName, L"File_%i", i);
		RegCreateKeyEx(hSSKey, lpwKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
			&hSSSubKey, &dwDisposition);
		RegSetValueEx(hSSSubKey, L"Path", 0, REG_SZ, (CONST LPBYTE)pFavorites[i]->lpwPath,
			(wcslen(pFavorites[i]->lpwPath) + 1) * sizeof(WCHAR));
		RegSetValueEx(hSSSubKey, L"DisplayName", 0, REG_SZ, (CONST LPBYTE)pFavorites[i]->lpwDisplayName,
			(wcslen(pFavorites[i]->lpwDisplayName) + 1) * sizeof(WCHAR));
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
		swprintf(lpwKeyName, L"ExternalFilter_%i", i);
		RegDeleteValue(hSSKey, lpwKeyName);
	}
	for (i = 0; i < APP_MAX_STRINGS; i++)
	{
		if (!lpwExternalFilters[i]) break;
		swprintf(lpwKeyName, L"ExternalFilter_%i", i);
		RegSetValueEx(hSSKey, lpwKeyName, 0, REG_SZ, (CONST LPBYTE)lpwExternalFilters[i],
			(wcslen(lpwExternalFilters[i]) + 1) * sizeof(WCHAR));
	}
	RegCloseKey(hSSKey);
}