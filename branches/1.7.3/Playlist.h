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

DWORD LoadPlaylist(LPCWSTR lpFileName);
void SavePlaylist(LPCWSTR lpFileName);

static DWORD LoadPlaylist_Common(LPCWSTR lpFileName);
static DWORD LoadPlaylist_M3U(LPCWSTR lpFileName);
static DWORD LoadPlaylist_EBL(LPCWSTR lpFileName);
static DWORD LoadPlaylist_WPL(LPCWSTR lpFileName);
static DWORD LoadPlaylist_ASX(LPCWSTR lpFileName);
static DWORD LoadPlaylist_XSPF(LPCWSTR lpFileName);
static DWORD LoadPlaylist_PLS(LPCWSTR lpFileName);
static DWORD LoadPlaylist_MPCPL(LPCWSTR lpFileName);
static void SavePlaylist_EBL(LPCWSTR lpFileName);
static void SavePlaylist_M3U(LPCWSTR lpFileName);
static void SavePlaylist_ASX(LPCWSTR lpFileName);

static void CheckPath(LPCWSTR lpPlaylist, LPCWSTR lpFile, LPWSTR lpResult);
static SIZE_T TrimNLChr(LPWSTR lpLine);

#endif