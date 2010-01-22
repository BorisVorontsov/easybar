#ifndef PLAYLIST_H
#define PLAYLIST_H

//#define BOM_UTF8_WORD				0xEFBB
//#define BOM_UTF8_BYTE				0xBF

#define BOM_UTF8_TEXT				"﻿"

#define EBL_KEYWORD_SIGNATURE		L"#SIGNATURE"
#define EBL_KEYWORD_ENCODING		L"#ENCODING"
#define EBL_KEYWORD_GENERATOR		L"#GENERATOR"
#define EBL_KEYWORD_TITLE			L"#TITLE"
#define EBL_KEYWORD_MEDIASOURCES	L"#MEDIA SOURCES"

#define EBL_SIGNATURE				L"EBL"

__declspec(selectany) LPCWSTR EBL_SUPPORTED_VERSIONS[] = { L"1.0", L"1.1", L"1.2", 0 };
__declspec(selectany) LPCWSTR EBL_SUPPORTED_ENCODINGS[] = { L"UTF-8", 0 };

DWORD LoadPlaylist(LPCWSTR lpwFileName);
void SavePlaylist(LPCWSTR lpwFileName);

static DWORD LoadPlaylist_Common(LPCWSTR lpwFileName);
static DWORD LoadPlaylist_M3U(LPCWSTR lpwFileName);
static DWORD LoadPlaylist_EBL(LPCWSTR lpwFileName);
static DWORD LoadPlaylist_WPL(LPCWSTR lpwFileName);
static DWORD LoadPlaylist_ASX(LPCWSTR lpwFileName);
static DWORD LoadPlaylist_XSPF(LPCWSTR lpwFileName);
static DWORD LoadPlaylist_PLS(LPCWSTR lpwFileName);
static DWORD LoadPlaylist_MPCPL(LPCWSTR lpwFileName);
static void SavePlaylist_EBL(LPCWSTR lpwFileName);
static void SavePlaylist_M3U(LPCWSTR lpwFileName);
static void SavePlaylist_ASX(LPCWSTR lpwFileName);

static void CheckPath(LPCWSTR lpwPlaylist, LPCWSTR lpwFile, LPWSTR lpwResult);
static DWORD TrimNLChr(LPWSTR lpwLine);

#endif