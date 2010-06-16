//Дополнительный модуль для 'EasyBar.cpp'
//Загрузка и сохранение списков воспроизведения

#include <windows.h>
#include <stdio.h>

#include <msxml2.h>

#pragma comment (lib, "msxml2.lib")

#include "resource.h"
#include "strparser.h"
#include "common.h"
#include "filecollection.h"
#include "playlistdlg.h"
#include "easybar.h"
#include "playlist.h"

extern WCHAR lpAppVersionMM[10];

//Общая функция загрузки списков
//В зависимости от расширения файла функция вызывает соответствующие подфункции
DWORD LoadPlaylist(LPCWSTR lpFileName)
{
	WCHAR lpExt[64] = {};
	SP_ExtractRightPart(lpFileName, lpExt, '.');
	if (_wcsicmp(lpExt, L"ebl") == 0)
	{
		return LoadPlaylist_EBL(lpFileName);
	}
	else if ((_wcsicmp(lpExt, L"m3u") == 0) || (_wcsicmp(lpExt, L"m3u8") == 0))
	{
		return LoadPlaylist_M3U(lpFileName);
	}
	else if (_wcsicmp(lpExt, L"wpl") == 0)
	{
		return LoadPlaylist_WPL(lpFileName);
	}
	else if (_wcsicmp(lpExt, L"asx") == 0)
	{
		return LoadPlaylist_ASX(lpFileName);
	}
	else if (_wcsicmp(lpExt, L"xspf") == 0)
	{
		return LoadPlaylist_XSPF(lpFileName);
	}
	else if (_wcsicmp(lpExt, L"pls") == 0)
	{
		return LoadPlaylist_PLS(lpFileName);
	}
	else if (_wcsicmp(lpExt, L"mpcpl") == 0)
	{
		return LoadPlaylist_MPCPL(lpFileName);
	}
	else
	{
		return LoadPlaylist_Common(lpFileName);
	}
}

//Общая функция сохранения списков
//В зависимости от расширения файла функция вызывает соответствующие подфункции
void SavePlaylist(LPCWSTR lpFileName)
{
	WCHAR lpExt[64] = {};
	SP_ExtractRightPart(lpFileName, lpExt, '.');
	if (_wcsicmp(lpExt, L"ebl") == 0)
	{
		SavePlaylist_EBL(lpFileName);
	}
	else if ((_wcsicmp(lpExt, L"m3u") == 0) || (_wcsicmp(lpExt, L"m3u8") == 0))
	{
		SavePlaylist_M3U(lpFileName);
	}
	else if (_wcsicmp(lpExt, L"asx") == 0)
	{
		SavePlaylist_ASX(lpFileName);
	}
}

//Загрузка списков, в которых путь к файлу занимает отдельную строку
//По умолчанию читаются файлы в кодировке ANSI. Для файлов с BOM UTF-8
//выбирается кодировка UTF-8
DWORD LoadPlaylist_Common(LPCWSTR lpFileName)
{
	FILE *pPlaylist;
	SIZE_T szLineSize;
	ULONG lLineCnt = 0, lFileCnt = 0;
	BOOL bEncInUTF8 = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = {};
	WCHAR lpUnicLine[MAX_PATH] = {};
	WCHAR lpExt[64] = {};
	SP_ExtractRightPart(lpFileName, lpExt, '.');
	pPlaylist = _wfopen(lpFileName, L"rt");
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpLine, sizeof(lpLine));
		fgets(lpLine, sizeof(lpLine), pPlaylist);
		if (lLineCnt == 0)
		{
			//Ищем BOM (Byte Order Mark) UTF-8...
			if (strstr(lpLine, BOM_UTF8_TEXT) == lpLine)
			{
				CopyMemory(lpLine, lpLine + 3, sizeof(lpLine) - 3);
				bEncInUTF8 = TRUE;
			}
		}
		MultiByteToWideChar((bEncInUTF8)?CP_UTF8:CP_ACP, 0, lpLine, -1, lpUnicLine,
			sizeof(lpLine) / sizeof(WCHAR));
		szLineSize = TrimNLChr(lpUnicLine);
		CheckPath(lpFileName, lpUnicLine, lpUnicLine);
		if (IsFile(lpUnicLine) || IsURL(lpUnicLine))
		{
			//Для BSL (и подобных) - удаление кавычек
			if ((lpLine[0] == '\"') && (lpLine[szLineSize - 1] == '\"'))
			{
				SP_TrimEx(lpUnicLine, lpUnicLine, '\"');
			}
			pFileCollection->AppendFile(lpUnicLine);
			lFileCnt++;
		}
		lLineCnt++;
	}
	fclose(pPlaylist);
	return lFileCnt;
}

//Загрузка списка в форматах M3U и M3U8, кодировка ANSI и UTF-8 соответственно
DWORD LoadPlaylist_M3U(LPCWSTR lpFileName)
{
	FILE *pPlaylist;
	SIZE_T szLineSize;
	ULONG lLineCnt = 0, lArgsCnt, lFileCnt = 0;
	BOOL bEncInUTF8 = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = {};
	WCHAR lpUnicLine[MAX_PATH] = {};
	WCHAR lpExt[64] = {};
	WCHAR lpKeyword[8] = {};
	LPWSTR lpExtInf[2];
	LPPLITEMDESC pPLID = 0;
	SP_ExtractRightPart(lpFileName, lpExt, '.');
	pPlaylist = _wfopen(lpFileName, L"rt");
	if (_wcsicmp(lpExt, L"m3u8") == 0) bEncInUTF8 = TRUE;
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpLine, sizeof(lpLine));
		fgets(lpLine, sizeof(lpLine), pPlaylist);
		if (lLineCnt == 0)
		{
			if (strstr(lpLine, BOM_UTF8_TEXT) == lpLine)
				CopyMemory(lpLine, lpLine + 3, sizeof(lpLine) - 3);
		}
		MultiByteToWideChar((bEncInUTF8)?CP_UTF8:CP_ACP, 0, lpLine, -1, lpUnicLine,
			sizeof(lpLine) / sizeof(WCHAR));
		szLineSize = TrimNLChr(lpUnicLine);
		if (lLineCnt == 0)
		{
			if (lpLine[0] != '#' || (szLineSize != 7)) return 0;
			wcsncpy(lpKeyword, lpUnicLine, 7);
			if (_wcsicmp(lpKeyword, L"#EXTM3U") != 0) return 0;
		}
		if ((lpLine[0] == '#') && (szLineSize > 7))
		{
			wcsncpy(lpKeyword, lpUnicLine, 7);
			if (_wcsicmp(lpKeyword, L"#EXTINF") == 0)
			{
				//"#EXTINF:_p1_,_p2_\n"
				lArgsCnt = SP_Split(&lpUnicLine[8], lpExtInf, ',', 2);
				if (lArgsCnt >= 2)
				{
					pPLID = new PLITEMDESC;
					SP_TrimEx(lpExtInf[0], lpExtInf[0], ' ');
					pPLID->uDuration = _wtoi(lpExtInf[0]) * 1000;
					SP_TrimEx(lpExtInf[1], lpExtInf[1], ' ');
					wcsncpy(pPLID->lpTitle, lpExtInf[1], 127);
				}
				if (lpExtInf[0]) delete[] lpExtInf[0];
				if (lpExtInf[1]) delete[] lpExtInf[1];
			}
			ZeroMemory(lpLine, sizeof(lpLine));
			ZeroMemory(lpLine, sizeof(lpLine));
			fgets(lpLine, sizeof(lpLine), pPlaylist);
			MultiByteToWideChar((bEncInUTF8)?CP_UTF8:CP_ACP, 0, lpLine, -1, lpUnicLine,
				sizeof(lpLine) / sizeof(WCHAR));
			szLineSize = TrimNLChr(lpUnicLine);
			CheckPath(lpFileName, lpUnicLine, lpUnicLine);
			if (IsFile(lpUnicLine) || IsURL(lpUnicLine))
			{
				if (pPLID)
				{
					pFileCollection->AppendFile(lpUnicLine, (LONG_PTR)pPLID);
					pPLID = 0;
				}
				else
					pFileCollection->AppendFile(lpUnicLine);
			}
			else
			{
				if (pPLID)
				{
					delete pPLID;
					pPLID = 0;
				}
			}
			lFileCnt++;
			lLineCnt += 2;
		}
		else lLineCnt++;
	}
	fclose(pPlaylist);
	return lFileCnt;
}

//Загрузка списка в формате EBL (EasyBar [Play]List), кодировка UTF-8
DWORD LoadPlaylist_EBL(LPCWSTR lpFileName)
{
	FILE *pPlaylist;
	SIZE_T szLineSize;
	ULONG i, lLineCnt = 0, lArgsCnt, lFileCnt = 0;
	BOOL SignatureOK = FALSE, EncodingOK = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = {};
	WCHAR lpUnicLine[MAX_PATH] = {};
	LPWSTR pArgs[4];
	LPPLITEMDESC pPLID = 0;
	pPlaylist = _wfopen(lpFileName, L"rt");
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpLine, sizeof(lpLine));
		fgets(lpLine, sizeof(lpLine), pPlaylist);
		MultiByteToWideChar(CP_UTF8, 0, lpLine, -1, lpUnicLine, sizeof(lpLine) / sizeof(WCHAR));
		szLineSize = TrimNLChr(lpUnicLine);
		ZeroMemory(pArgs, sizeof(pArgs));
		lArgsCnt = SP_Split(lpUnicLine, &pArgs[0], ',', 4);
		if (lArgsCnt >= 2)
		{
			if (!SignatureOK)
			{
				if (_wcsicmp(pArgs[0], EBL_KEYWORD_SIGNATURE) == 0)
				{
					SP_TrimEx(pArgs[1], pArgs[1], ' ');
					if (_wcsicmp(pArgs[1], EBL_SIGNATURE) != 0) goto ExitFunction;
					SP_TrimEx(pArgs[2], pArgs[2], ' ');
					for (i = 0; EBL_SUPPORTED_VERSIONS[i] != 0; i++)
					{
						if (_wcsicmp(pArgs[2], EBL_SUPPORTED_VERSIONS[i]) == 0) break;
						if (EBL_SUPPORTED_VERSIONS[i + 1] == 0) goto ExitFunction;
					}
					SignatureOK = TRUE;
					continue;
				}
			}
			if (!EncodingOK)
			{
				if (_wcsicmp(pArgs[0], EBL_KEYWORD_ENCODING) == 0)
				{
					SP_TrimEx(pArgs[1], pArgs[1], ' ');
					for (i = 0; ; i++)
					{
						if (_wcsicmp(pArgs[1], EBL_SUPPORTED_ENCODINGS[i]) == 0) break;
						if (EBL_SUPPORTED_ENCODINGS[i + 1] == 0) goto ExitFunction;
					}
					EncodingOK = TRUE;
					continue;
				}
			}
			if (SignatureOK && EncodingOK)
			{
				SP_TrimEx(pArgs[0], pArgs[0], ' ');
				if (IsNumeric(pArgs[0]))
				{
					SP_TrimEx(pArgs[1], pArgs[1], ' ');
					CheckPath(lpFileName, pArgs[1], lpUnicLine);
					if (lArgsCnt == 4)
					{
						pPLID = new PLITEMDESC;
						SP_TrimEx(pArgs[2], pArgs[2], ' ');
						wcsncpy(pPLID->lpTitle, pArgs[2], 127);
						SP_TrimEx(pArgs[3], pArgs[3], ' ');
						pPLID->uDuration = _wtoi(pArgs[3]);
					}
					if (IsFile(lpUnicLine) || IsURL(lpUnicLine))
					{
						if (pPLID)
						{
							pFileCollection->AppendFile(lpUnicLine, (LONG_PTR)pPLID);
							pPLID = 0;
						}
						else
							pFileCollection->AppendFile(lpUnicLine);
						lFileCnt++;
					}
					else
					{
						if (pPLID)
						{
							delete pPLID;
							pPLID = 0;
						}
					}
				}
			}
		}
		else
		{
			if (lLineCnt == 0) goto ExitFunction;
		}
		if (pArgs[0]) delete[] pArgs[0];
		if (pArgs[1]) delete[] pArgs[1];
		if (pArgs[2]) delete[] pArgs[2];
		if (pArgs[3]) delete[] pArgs[3];
		lLineCnt++;
	}
ExitFunction:
#ifndef _DEBUG
	if (pArgs[0]) delete[] pArgs[0];
	if (pArgs[1]) delete[] pArgs[1];
	if (pArgs[2]) delete[] pArgs[2];
	if (pArgs[3]) delete[] pArgs[3];
#endif
	fclose(pPlaylist);
	return lFileCnt;
}

//Загрузка списка в формате WPL (Windows Media Playlist), кодировка неизвестна
DWORD LoadPlaylist_WPL(LPCWSTR lpFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMNodeList *pMediaList = 0;
	IXMLDOMNode *pMedia = 0, *pMediaSource = 0;
	IXMLDOMNamedNodeMap *pMediaAttributes = 0;
	VARIANT varFileName = {};
	VARIANT_BOOL varResult;
	BSTR bstrTmp;
	ULONG lFileCnt = 0;
	WCHAR lpLine[MAX_PATH] = {};
	if (FAILED(CoCreateInstance(__uuidof(DOMDocument), 0, CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument), (LPVOID *)&pXMLDOMDocument))) return 0;
	pXMLDOMDocument->put_async(VARIANT_FALSE);
	pXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
	pXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
	varFileName.vt = VT_BSTR;
	varFileName.bstrVal = SysAllocString(lpFileName);
	pXMLDOMDocument->load(varFileName, &varResult);
	if (varResult == VARIANT_FALSE)
	{
#ifdef _DEBUG
		IXMLDOMParseError *pXMLError = 0;
		pXMLDOMDocument->get_parseError(&pXMLError);
		pXMLError->get_reason(&bstrTmp);
		MessageBox(0, bstrTmp, L"Parse error", 0);
		SysFreeString(bstrTmp);
#endif
		VariantClear(&varFileName);
		pXMLDOMDocument->Release();
		return 0;
	}
	pXMLDOMDocument->selectNodes(L"smil/body/seq/media", &pMediaList);
	pMediaList->nextNode(&pMedia);
	while (pMedia)
	{
		pMedia->get_attributes(&pMediaAttributes);
		pMediaAttributes->getNamedItem(L"src", &pMediaSource);
		pMediaSource->get_text(&bstrTmp);
		CheckPath(lpFileName, bstrTmp, lpLine);
		if (IsFile(lpLine) || IsURL(lpLine))
		{
			pFileCollection->AppendFile(lpLine);
			lFileCnt++;
		}
		SysFreeString(bstrTmp);
		pMediaSource->Release();
		pMediaAttributes->Release();
		pMedia->Release();
		pMediaList->nextNode(&pMedia);
	}
	pMediaList->Release();
	VariantClear(&varFileName);
	pXMLDOMDocument->Release();
	return lFileCnt;
}

//Загрузка списка в формате ASX (Advanced Streaming Index), кодировка неизвестна
DWORD LoadPlaylist_ASX(LPCWSTR lpFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMNodeList *pEntryList = 0;
	IXMLDOMNode *pEntry = 0, *pRef = 0, *pHRef = 0;
	IXMLDOMNamedNodeMap *pRefAttributes = 0;
	VARIANT varFileName = {};
	VARIANT_BOOL varResult;
	BSTR bstrTmp;
	ULONG lFileCnt = 0;
	WCHAR lpLine[MAX_PATH] = {};
	if (FAILED(CoCreateInstance(__uuidof(DOMDocument), 0, CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument), (LPVOID *)&pXMLDOMDocument))) return 0;
	pXMLDOMDocument->put_async(VARIANT_FALSE);
	pXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
	pXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
	varFileName.vt = VT_BSTR;
	varFileName.bstrVal = SysAllocString(lpFileName);
	pXMLDOMDocument->load(varFileName, &varResult);
	if (varResult == VARIANT_FALSE)
	{
#ifdef _DEBUG
		IXMLDOMParseError *pXMLError = 0;
		pXMLDOMDocument->get_parseError(&pXMLError);
		pXMLError->get_reason(&bstrTmp);
		MessageBox(0, bstrTmp, L"Parse error", 0);
		SysFreeString(bstrTmp);
#endif
		VariantClear(&varFileName);
		pXMLDOMDocument->Release();
		return 0;
	}
	pXMLDOMDocument->selectNodes(L"asx/entry", &pEntryList);
	pEntryList->nextNode(&pEntry);
	while (pEntry)
	{
		pEntry->selectSingleNode(L"ref", &pRef);
		pRef->get_attributes(&pRefAttributes);
		pRefAttributes->getNamedItem(L"href", &pHRef);
		pHRef->get_text(&bstrTmp);
		CheckPath(lpFileName, bstrTmp, lpLine);
		if (IsFile(lpLine) || IsURL(lpLine))
		{
			pFileCollection->AppendFile(lpLine);
			lFileCnt++;
		}
		SysFreeString(bstrTmp);
		pHRef->Release();
		pRefAttributes->Release();
		pRef->Release();
		pEntry->Release();
		pEntryList->nextNode(&pEntry);
	}
	pEntryList->Release();
	VariantClear(&varFileName);
	pXMLDOMDocument->Release();
	return lFileCnt;
}

//Загрузка списка в формате XSPF (VLC media player playlist), кодировка UTF-8
DWORD LoadPlaylist_XSPF(LPCWSTR lpFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMNodeList *pTrackList = 0;
	IXMLDOMNode *pTrack = 0, *pLocation = 0;
	VARIANT varFileName = {};
	VARIANT_BOOL varResult;
	BSTR bstrTmp;
	ULONG lFileCnt = 0;
	WCHAR lpLine[MAX_PATH] = {};
	if (FAILED(CoCreateInstance(__uuidof(DOMDocument), 0, CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument), (LPVOID *)&pXMLDOMDocument))) return 0;
	pXMLDOMDocument->put_async(VARIANT_FALSE);
	pXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
	pXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
	varFileName.vt = VT_BSTR;
	varFileName.bstrVal = SysAllocString(lpFileName);
	pXMLDOMDocument->load(varFileName, &varResult);
	if (varResult == VARIANT_FALSE)
	{
#ifdef _DEBUG
		IXMLDOMParseError *pXMLError = 0;
		pXMLDOMDocument->get_parseError(&pXMLError);
		pXMLError->get_reason(&bstrTmp);
		MessageBox(0, bstrTmp, L"Parse error", 0);
		SysFreeString(bstrTmp);
#endif
		VariantClear(&varFileName);
		pXMLDOMDocument->Release();
		return 0;
	}
	pXMLDOMDocument->selectNodes(L"playlist/trackList/track", &pTrackList);
	pTrackList->nextNode(&pTrack);
	while (pTrack)
	{
		pTrack->selectSingleNode(L"location", &pLocation);
		pLocation->get_text(&bstrTmp);
		CheckPath(lpFileName, bstrTmp, lpLine);
		if (IsFile(lpLine) || IsURL(lpLine))
		{
			pFileCollection->AppendFile(lpLine);
			lFileCnt++;
		}
		SysFreeString(bstrTmp);
		pLocation->Release();
		pTrack->Release();
		pTrackList->nextNode(&pTrack);
	}
	pTrackList->Release();
	VariantClear(&varFileName);
	pXMLDOMDocument->Release();
	return lFileCnt;
}

//Загрузка списка в формате PLS (? Playlist), кодировка ANSI
DWORD LoadPlaylist_PLS(LPCWSTR lpFileName)
{
	ULONG i = 0, lNumOfEntries, lFileCnt = 0;
	WCHAR lpKey[32] = {};
	WCHAR lpLine[MAX_PATH] = {};
	lNumOfEntries = GetPrivateProfileInt(L"playlist", L"numberofentries", 0, lpFileName);
	if (!lNumOfEntries) return 0;
	for (; i <= lNumOfEntries; i++)
	{
		swprintf(lpKey, L"file%i", i);
		GetPrivateProfileString(L"playlist", lpKey, 0, lpLine, MAX_PATH, lpFileName);
		CheckPath(lpFileName, lpLine, lpLine);
		if (IsFile(lpLine))
		{
			pFileCollection->AppendFile(lpLine);
			lFileCnt++;
		}
	}
	return lFileCnt;
}

//Загрузка списка в формате MPCPL (Media Player Classic Playlist), кодировка неизвестна
DWORD LoadPlaylist_MPCPL(LPCWSTR lpFileName)
{
	FILE *pPlaylist;
	SIZE_T szLineSize;
	ULONG lLineCnt = 0, lArgsCnt, lFileCnt = 0;
	BOOL bEncInUTF8 = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = {};
	WCHAR lpUnicLine[MAX_PATH] = {};
	LPWSTR pArgs[3];
	pPlaylist = _wfopen(lpFileName, L"rt");
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpLine, sizeof(lpLine));
		fgets(lpLine, sizeof(lpLine), pPlaylist);
		if (lLineCnt == 0)
		{
			if (strstr(lpLine, BOM_UTF8_TEXT) == lpLine)
			{
				CopyMemory(lpLine, lpLine + 3, sizeof(lpLine) - 3);
				bEncInUTF8 = TRUE;
			}
		}
		if (bEncInUTF8)
		{
			MultiByteToWideChar(CP_UTF8, 0, lpLine, -1, lpUnicLine, sizeof(lpLine) / sizeof(WCHAR));
		}
		else
		{
			MultiByteToWideChar(CP_ACP, 0, lpLine, -1, lpUnicLine, sizeof(lpLine) / sizeof(WCHAR));
		}
		szLineSize = TrimNLChr(lpUnicLine);
		ZeroMemory(pArgs, sizeof(pArgs));
		lArgsCnt = SP_Split(lpUnicLine, &pArgs[0], ',', 3);
		if (lArgsCnt >= 3)
		{
			if (wcscmp(pArgs[1], L"filename") == 0)
			{
				CheckPath(lpFileName, pArgs[2], lpUnicLine);
				if (IsFile(lpUnicLine))
				{
					
					pFileCollection->AppendFile(lpUnicLine);
					lFileCnt++;
				}
			}
		}
		if (pArgs[0]) delete[] pArgs[0];
		if (pArgs[1]) delete[] pArgs[1];
		if (pArgs[2]) delete[] pArgs[2];
		lLineCnt++;
	}
	fclose(pPlaylist);
	return lFileCnt;
}

//Сохранение списка в формате EBL, кодировка UTF-8
void SavePlaylist_EBL(LPCWSTR lpFileName)
{
	FILE *pPlaylist;
	ULONG i, lTime, lFCFileCnt = 0;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = {};
	char lpTmp1[256] = {}, lpTmp2[256] = {}, lpTmp3[/*64*/256] = {};
	WCHAR lpTitle[128] = {};
	WCHAR lpName[128] = {};
	WCHAR lpUnicLine[MAX_PATH] = {};
	LPPLITEMDESC pPLID = 0;
	lFCFileCnt = pFileCollection->FileCount();
	pPlaylist = _wfopen(lpFileName, L"wt");
	WideCharToMultiByte(CP_UTF8, 0, EBL_KEYWORD_SIGNATURE, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
	WideCharToMultiByte(CP_UTF8, 0, EBL_SIGNATURE, -1, lpTmp2, sizeof(lpTmp2), 0, 0);
	for (i = 0; ; i++)
	{
		if (EBL_SUPPORTED_VERSIONS[i + 1] == 0)
		{
			WideCharToMultiByte(CP_UTF8, 0, EBL_SUPPORTED_VERSIONS[i], -1, lpTmp3, sizeof(lpTmp3), 0, 0);
			fprintf(pPlaylist, "%s, %s, %s\n", lpTmp1, lpTmp2, lpTmp3);
			break;
		}
	}
	WideCharToMultiByte(CP_UTF8, 0, EBL_KEYWORD_ENCODING, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
	for (i = 0; ; i++)
	{
		if (EBL_SUPPORTED_ENCODINGS[i + 1] == 0)
		{
			WideCharToMultiByte(CP_UTF8, 0, EBL_SUPPORTED_ENCODINGS[i], -1, lpTmp2, sizeof(lpTmp2), 0, 0);
			fprintf(pPlaylist, "%s, %s\n", lpTmp1, lpTmp2);
			break;
		}
	}
	WideCharToMultiByte(CP_UTF8, 0, EBL_KEYWORD_GENERATOR, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
	WideCharToMultiByte(CP_UTF8, 0, APP_NAME, -1, lpTmp2, sizeof(lpTmp2), 0, 0);
	WideCharToMultiByte(CP_UTF8, 0, lpAppVersionMM, -1, lpTmp3, sizeof(lpTmp3), 0, 0);
	fprintf(pPlaylist, "%s, %s, %s\n", lpTmp1, lpTmp2, lpTmp3);
	WideCharToMultiByte(CP_UTF8, 0, EBL_KEYWORD_TITLE, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
	SP_ExtractName(lpFileName, lpTitle);
	SP_ExtractLeftPart(lpTitle, lpTitle, '.');
	WideCharToMultiByte(CP_UTF8, 0, lpTitle, -1, lpTmp2, sizeof(lpTmp2), 0, 0);
	fprintf(pPlaylist, "%s, %s\n\n", lpTmp1, lpTmp2);
	WideCharToMultiByte(CP_UTF8, 0, EBL_KEYWORD_MEDIASOURCES, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
	fprintf(pPlaylist, "%s, %i\n", lpTmp1, lFCFileCnt);
	for (; i < lFCFileCnt; i++)
	{
		pPLID = 0;
		pFileCollection->GetUserData(0, i, FCF_BYINDEX, (LONG_PTR&)pPLID);
		pFileCollection->GetFile(lpUnicLine, i, FCF_BYINDEX);
		if (pPLID)
		{
			lTime = pPLID->uDuration;
			wcscpy(lpName, pPLID->lpTitle);
		}
		else
		{
			lTime = 0;
			GetTitle(lpUnicLine, lpName);
		}
		WideCharToMultiByte(CP_UTF8, 0, lpUnicLine, -1, lpLine, sizeof(lpLine), 0, 0);
		WideCharToMultiByte(CP_UTF8, 0, lpName, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
		fprintf(pPlaylist, "%i, %s, %s, %i\n", i + 1, lpLine, lpTmp1, lTime);
	}
	fclose(pPlaylist);
}

//Сохранение списка в формате M3U, кодировка ANSI
/*void SavePlaylist_M3U(LPCWSTR lpFileName)
{
	FILE *pPlaylist;
	ULONG i = 0, lFCFileCnt = 0;
	char lpName[128] = {};
	WCHAR lpName[128] = {};
	char lpLine[MAX_PATH] = {};
	WCHAR lpLine[MAX_PATH] = {};
	pPlaylist = _wfopen(lpFileName, L"wt");
	fputs("#EXTM3U\n", pPlaylist);
	lFCFileCnt = pFileCollection->FileCount();
	for (; i < lFCFileCnt; i++)
	{
		pFileCollection->GetFile(lpLine, i, FCF_BYINDEX);
		SP_ExtractName(lpLine, lpName);
		WideCharToMultiByte(CP_ACP, 0, lpName, -1, lpName, sizeof(lpName), 0, 0);
		WideCharToMultiByte(CP_ACP, 0, lpLine, -1, lpLine, sizeof(lpLine), 0, 0);
		fprintf(pPlaylist, "#EXTINF: 0, %s\n", lpName);
		fprintf(pPlaylist, "%s\n", lpLine);
	}
	fclose(pPlaylist);
}*/

//Сохранение списка в форматах M3U и M3U8, кодировка ANSI и UTF-8 соответственно
void SavePlaylist_M3U(LPCWSTR lpFileName)
{
	FILE *pPlaylist;
	ULONG i = 0, lTime, lFCFileCnt = 0;
	char lpName[256] = {};
	WCHAR lpUnicName[128] = {};
	char lpLine[MAX_PATH * sizeof(WCHAR)] = {};
	WCHAR lpUnicLine[MAX_PATH] = {};
	LPPLITEMDESC pPLID = 0;
	pPlaylist = _wfopen(lpFileName, L"wt");
	fputs(BOM_UTF8_TEXT, pPlaylist);
	fputs("#EXTM3U\n", pPlaylist);
	lFCFileCnt = pFileCollection->FileCount();
	for (; i < lFCFileCnt; i++)
	{
		pPLID = 0;
		pFileCollection->GetUserData(0, i, FCF_BYINDEX, (LONG_PTR&)pPLID);
		pFileCollection->GetFile(lpUnicLine, i, FCF_BYINDEX);
		if (pPLID)
		{
			lTime = pPLID->uDuration / 1000;
			wcscpy(lpUnicName, pPLID->lpTitle);
		}
		else
		{
			lTime = 0;
			GetTitle(lpUnicLine, lpUnicName);
		}
		WideCharToMultiByte(CP_UTF8, 0, lpUnicName, -1, lpName, sizeof(lpName), 0, 0);
		WideCharToMultiByte(CP_UTF8, 0, lpUnicLine, -1, lpLine, sizeof(lpLine), 0, 0);
		fprintf(pPlaylist, "#EXTINF:%i,%s\n", lTime, lpName);
		fprintf(pPlaylist, "%s\n", lpLine);
	}
	fclose(pPlaylist);
}

//Сохранение списка в формате ASX, кодировка ?
void SavePlaylist_ASX(LPCWSTR lpFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMElement *pAsxAttributes = 0, *pRefAttributes = 0;
	IXMLDOMNode *pAsx = 0, *pTitle = 0, *pEntry = 0, *pInsertedNode = 0, *pRef = 0;
	IXMLDOMText *pWhiteSpace = 0;
	VARIANT varAsxVersion = {}, varNodeType = {}, varLine = {},
		varFileName = {};
	ULONG i = 0, lFCFileCnt = 0;
	WCHAR lpTitle[128] = {};
	WCHAR lpLine[MAX_PATH] = {};
	varNodeType.vt = VT_INT;
	varNodeType.intVal = NODE_ELEMENT;
	if (FAILED(CoCreateInstance(__uuidof(DOMDocument), 0, CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument), (LPVOID *)&pXMLDOMDocument))) return;
	pXMLDOMDocument->put_async(VARIANT_FALSE);
	pXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
	pXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
	pXMLDOMDocument->createNode(varNodeType, L"asx", 0, &pAsx);
	pAsx->QueryInterface(__uuidof(IXMLDOMElement), (LPVOID *)&pAsxAttributes);
	varAsxVersion.vt = VT_BSTR;
	varAsxVersion.bstrVal = SysAllocString(L"3.0");
	pAsxAttributes->setAttribute(L"version", varAsxVersion);
	pXMLDOMDocument->appendChild(pAsx, &pInsertedNode);
	pXMLDOMDocument->createTextNode(L"\n\t", &pWhiteSpace);
	pAsx->appendChild(pWhiteSpace, &pInsertedNode);
	pWhiteSpace->Release();
	pXMLDOMDocument->createNode(varNodeType, L"title", 0, &pTitle);
	SP_ExtractName(lpFileName, lpTitle);
	SP_ExtractLeftPart(lpTitle, lpTitle, '.');
	pTitle->put_text(lpTitle);
	pAsx->appendChild(pTitle, &pInsertedNode);
	pTitle->Release();
	lFCFileCnt = pFileCollection->FileCount();
	for (; i < lFCFileCnt; i++)
	{
		pFileCollection->GetFile(lpLine, i, FCF_BYINDEX);
		pXMLDOMDocument->createTextNode(L"\n\t", &pWhiteSpace);
		pAsx->appendChild(pWhiteSpace, &pInsertedNode);
		pWhiteSpace->Release();
		pXMLDOMDocument->createNode(varNodeType, L"entry", 0, &pEntry);
		pAsx->appendChild(pEntry, &pInsertedNode);
		pXMLDOMDocument->createTextNode(L"\n\t\t", &pWhiteSpace);
		pEntry->appendChild(pWhiteSpace, &pInsertedNode);
		pWhiteSpace->Release();
		pXMLDOMDocument->createNode(varNodeType, L"title", 0, &pTitle);
		SP_ExtractName(lpLine, lpTitle);
		SP_ExtractLeftPart(lpTitle, lpTitle, '.');
		pTitle->put_text(lpTitle);
		pEntry->appendChild(pTitle, &pInsertedNode);
		pTitle->Release();
		pXMLDOMDocument->createTextNode(L"\n\t\t", &pWhiteSpace);
		pEntry->appendChild(pWhiteSpace, &pInsertedNode);
		pWhiteSpace->Release();
		pXMLDOMDocument->createNode(varNodeType, L"ref", 0, &pRef);
		pRef->QueryInterface(__uuidof(IXMLDOMElement), (LPVOID *)&pRefAttributes);
		varLine.vt = VT_BSTR;
		varLine.bstrVal = SysAllocString(lpLine);
		pRefAttributes->setAttribute(L"href", varLine);
		pEntry->appendChild(pRef, &pInsertedNode);
		pXMLDOMDocument->createTextNode(L"\n\t", &pWhiteSpace);
		pEntry->appendChild(pWhiteSpace, &pInsertedNode);
		pWhiteSpace->Release();
		VariantClear(&varLine);
		pRefAttributes->Release();
		pRef->Release();
		pEntry->Release();
	}
	pXMLDOMDocument->createTextNode(L"\n", &pWhiteSpace);
	pAsx->appendChild(pWhiteSpace, &pInsertedNode);
	pWhiteSpace->Release();
	varFileName.vt = VT_BSTR;
	varFileName.bstrVal = SysAllocString(lpFileName);
	pXMLDOMDocument->save(varFileName);
	VariantClear(&varNodeType);
	VariantClear(&varAsxVersion);
	VariantClear(&varFileName);
	pAsxAttributes->Release();
	pAsx->Release();
	pXMLDOMDocument->Release();
}

//Создание абсолютного пути к файлу из относительного
void CheckPath(LPCWSTR lpPlaylist, LPCWSTR lpFile, LPWSTR lpResult)
{
	WCHAR lpPLDir[MAX_PATH] = {};
	WCHAR lpFullPath[MAX_PATH] = {};
	LPWSTR lpFilePart;
	SP_ExtractDirectory(lpPlaylist, lpPLDir);
	SetCurrentDirectory(lpPLDir);
	GetFullPathName(lpFile, MAX_PATH, lpFullPath, &lpFilePart);
	if (IsFile(lpFullPath))
	{
		wcscpy(lpResult, lpFullPath);
	}
	else
	{
		wcscpy(lpResult, lpFile);
	}
}

//Удаление символа новой строки
SIZE_T TrimNLChr(LPWSTR lpLine)
{
	SIZE_T szLineSize = wcslen(lpLine);
	if (lpLine[szLineSize - 1] == '\n')
	{
		lpLine[szLineSize - 1] = '\0';
		szLineSize--;
	}
	return szLineSize;
}
