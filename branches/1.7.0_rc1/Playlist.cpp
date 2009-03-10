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

extern WCHAR lpwAppVersionMM[10];

//Общая функция загрузки списков
//В зависимости от расширения файла функция вызывает соответствующие подфункции
DWORD LoadPlaylist(LPCWSTR lpwFileName)
{
	WCHAR lpwExt[64] = { 0 };
	SP_ExtractRightPart(lpwFileName, lpwExt, '.');
	if (_wcsicmp(lpwExt, L"ebl") == 0)
	{
		return LoadPlaylist_EBL(lpwFileName);
	}
	else if ((_wcsicmp(lpwExt, L"m3u") == 0) || (_wcsicmp(lpwExt, L"m3u8") == 0))
	{
		return LoadPlaylist_M3U(lpwFileName);
	}
	else if (_wcsicmp(lpwExt, L"wpl") == 0)
	{
		return LoadPlaylist_WPL(lpwFileName);
	}
	else if (_wcsicmp(lpwExt, L"asx") == 0)
	{
		return LoadPlaylist_ASX(lpwFileName);
	}
	else if (_wcsicmp(lpwExt, L"xspf") == 0)
	{
		return LoadPlaylist_XSPF(lpwFileName);
	}
	else if (_wcsicmp(lpwExt, L"pls") == 0)
	{
		return LoadPlaylist_PLS(lpwFileName);
	}
	else if (_wcsicmp(lpwExt, L"mpcpl") == 0)
	{
		return LoadPlaylist_MPCPL(lpwFileName);
	}
	else
	{
		return LoadPlaylist_Common(lpwFileName);
	}
}

//Общая функция сохранения списков
//В зависимости от расширения файла функция вызывает соответствующие подфункции
void SavePlaylist(LPCWSTR lpwFileName)
{
	WCHAR lpwExt[64] = { 0 };
	SP_ExtractRightPart(lpwFileName, lpwExt, '.');
	if (_wcsicmp(lpwExt, L"ebl") == 0)
	{
		SavePlaylist_EBL(lpwFileName);
	}
	else if ((_wcsicmp(lpwExt, L"m3u") == 0) || (_wcsicmp(lpwExt, L"m3u8") == 0))
	{
		SavePlaylist_M3U(lpwFileName);
	}
	else if (_wcsicmp(lpwExt, L"asx") == 0)
	{
		SavePlaylist_ASX(lpwFileName);
	}
}

//Загрузка списков, в которых путь к файлу занимает отдельную строку
//По умолчанию читаются файлы в кодировке ANSI. Для файлов с BOM UTF-8
//выбирается кодировка UTF-8
DWORD LoadPlaylist_Common(LPCWSTR lpwFileName)
{
	FILE *pPlaylist;
	ULONG lLineSize, lLineCnt = 0, lFileCnt = 0;
	BOOL bEncInUTF8 = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	WCHAR lpwExt[64] = { 0 };
	SP_ExtractRightPart(lpwFileName, lpwExt, '.');
	pPlaylist = _wfopen(lpwFileName, L"rt");
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpwLine, sizeof(lpwLine));
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
		MultiByteToWideChar((bEncInUTF8)?CP_UTF8:CP_ACP, 0, lpLine, -1, lpwLine,
			sizeof(lpwLine) / sizeof(WCHAR));
		lLineSize = TrimNLChr(lpwLine);
		CheckPath(lpwFileName, lpwLine, lpwLine);
		if (IsFile(lpwLine) || IsURL(lpwLine))
		{
			//Для BSL (и подобных) - удаление кавычек
			if ((lpwLine[0] == '\"') && (lpwLine[lLineSize - 1] == '\"'))
			{
				SP_TrimEx(lpwLine, lpwLine, '\"');
			}
			pFileCollection->AppendFile(lpwLine);
			lFileCnt++;
		}
		lLineCnt++;
	}
	fclose(pPlaylist);
	return lFileCnt;
}

//Загрузка списка в форматах M3U и M3U8, кодировка ANSI и UTF-8 соответственно
DWORD LoadPlaylist_M3U(LPCWSTR lpwFileName)
{
	FILE *pPlaylist;
	ULONG lLineSize, lLineCnt = 0, lArgsCnt, lFileCnt = 0;
	BOOL bEncInUTF8 = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	WCHAR lpwExt[64] = { 0 };
	WCHAR lpwKeyword[8] = { 0 };
	LPWSTR lpwExtInf[2];
	LPPLITEMDESC pPLID = 0;
	SP_ExtractRightPart(lpwFileName, lpwExt, '.');
	pPlaylist = _wfopen(lpwFileName, L"rt");
	if (_wcsicmp(lpwExt, L"m3u8") == 0) bEncInUTF8 = TRUE;
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpwLine, sizeof(lpwLine));
		fgets(lpLine, sizeof(lpLine), pPlaylist);
		if (lLineCnt == 0)
		{
			if (strstr(lpLine, BOM_UTF8_TEXT) == lpLine)
				CopyMemory(lpLine, lpLine + 3, sizeof(lpLine) - 3);
		}
		MultiByteToWideChar((bEncInUTF8)?CP_UTF8:CP_ACP, 0, lpLine, -1, lpwLine,
			sizeof(lpwLine) / sizeof(WCHAR));
		lLineSize = TrimNLChr(lpwLine);
		if (lLineCnt == 0)
		{
			if (lpwLine[0] != '#' || (lLineSize != 7)) return 0;
			wcsncpy(lpwKeyword, lpwLine, 7);
			if (_wcsicmp(lpwKeyword, L"#EXTM3U") != 0) return 0;
		}
		if ((lpwLine[0] == '#') && (lLineSize > 7))
		{
			wcsncpy(lpwKeyword, lpwLine, 7);
			if (_wcsicmp(lpwKeyword, L"#EXTINF") == 0)
			{
				//"#EXTINF:_p1_,_p2_\n"
				lArgsCnt = SP_Split(&lpwLine[8], lpwExtInf, ',', 2);
				if (lArgsCnt >= 2)
				{
					pPLID = new PLITEMDESC;
					SP_TrimEx(lpwExtInf[0], lpwExtInf[0], ' ');
					pPLID->uDuration = _wtoi(lpwExtInf[0]) * 1000;
					SP_TrimEx(lpwExtInf[0], lpwExtInf[1], ' ');
					wcsncpy(pPLID->lpwTitle, lpwExtInf[1], 127);
				}
				if (lpwExtInf[0]) delete[] lpwExtInf[0];
				if (lpwExtInf[1]) delete[] lpwExtInf[1];
			}
			ZeroMemory(lpLine, sizeof(lpLine));
			ZeroMemory(lpwLine, sizeof(lpwLine));
			fgets(lpLine, sizeof(lpLine), pPlaylist);
			MultiByteToWideChar((bEncInUTF8)?CP_UTF8:CP_ACP, 0, lpLine, -1, lpwLine,
				sizeof(lpwLine) / sizeof(WCHAR));
			lLineSize = TrimNLChr(lpwLine);
			CheckPath(lpwFileName, lpwLine, lpwLine);
			if (IsFile(lpwLine) || IsURL(lpwLine))
			{
				if (pPLID)
				{
					pFileCollection->AppendFile(lpwLine, (LONG_PTR)pPLID);
					pPLID = 0;
				}
				else
					pFileCollection->AppendFile(lpwLine);
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
DWORD LoadPlaylist_EBL(LPCWSTR lpwFileName)
{
	FILE *pPlaylist;
	ULONG i, lLineSize, lLineCnt = 0, lArgsCnt, lFileCnt = 0;
	BOOL SignatureOK = FALSE, EncodingOK = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	LPWSTR pArgs[4];
	LPPLITEMDESC pPLID = 0;
	pPlaylist = _wfopen(lpwFileName, L"rt");
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpwLine, sizeof(lpwLine));
		fgets(lpLine, sizeof(lpLine), pPlaylist);
		MultiByteToWideChar(CP_UTF8, 0, lpLine, -1, lpwLine, sizeof(lpwLine) / sizeof(WCHAR));
		lLineSize = TrimNLChr(lpwLine);
		ZeroMemory(pArgs, sizeof(pArgs));
		lArgsCnt = SP_Split(lpwLine, &pArgs[0], ',', 4);
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
					CheckPath(lpwFileName, pArgs[1], lpwLine);
					if (lArgsCnt == 4)
					{
						pPLID = new PLITEMDESC;
						SP_TrimEx(pArgs[2], pArgs[2], ' ');
						wcsncpy(pPLID->lpwTitle, pArgs[2], 127);
						SP_TrimEx(pArgs[3], pArgs[3], ' ');
						pPLID->uDuration = _wtoi(pArgs[3]);
					}
					if (IsFile(lpwLine) || IsURL(lpwLine))
					{
						if (pPLID)
						{
							pFileCollection->AppendFile(lpwLine, (LONG_PTR)pPLID);
							pPLID = 0;
						}
						else
							pFileCollection->AppendFile(lpwLine);
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
DWORD LoadPlaylist_WPL(LPCWSTR lpwFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMNodeList *pMediaList = 0;
	IXMLDOMNode *pMedia = 0, *pMediaSource = 0;
	IXMLDOMNamedNodeMap *pMediaAttributes = 0;
	VARIANT varFileName = { 0 };
	VARIANT_BOOL varResult;
	BSTR bstrTmp;
	ULONG lFileCnt = 0;
	WCHAR lpwLine[MAX_PATH] = { 0 };
	if (FAILED(CoCreateInstance(__uuidof(DOMDocument), 0, CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument), (LPVOID *)&pXMLDOMDocument))) return 0;
	pXMLDOMDocument->put_async(VARIANT_FALSE);
	pXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
	pXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
	varFileName.vt = VT_BSTR;
	varFileName.bstrVal = SysAllocString(lpwFileName);
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
		CheckPath(lpwFileName, bstrTmp, lpwLine);
		if (IsFile(lpwLine) || IsURL(lpwLine))
		{
			pFileCollection->AppendFile(lpwLine);
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
DWORD LoadPlaylist_ASX(LPCWSTR lpwFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMNodeList *pEntryList = 0;
	IXMLDOMNode *pEntry = 0, *pRef = 0, *pHRef = 0;
	IXMLDOMNamedNodeMap *pRefAttributes = 0;
	VARIANT varFileName = { 0 };
	VARIANT_BOOL varResult;
	BSTR bstrTmp;
	ULONG lFileCnt = 0;
	WCHAR lpwLine[MAX_PATH] = { 0 };
	if (FAILED(CoCreateInstance(__uuidof(DOMDocument), 0, CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument), (LPVOID *)&pXMLDOMDocument))) return 0;
	pXMLDOMDocument->put_async(VARIANT_FALSE);
	pXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
	pXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
	varFileName.vt = VT_BSTR;
	varFileName.bstrVal = SysAllocString(lpwFileName);
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
		CheckPath(lpwFileName, bstrTmp, lpwLine);
		if (IsFile(lpwLine) || IsURL(lpwLine))
		{
			pFileCollection->AppendFile(lpwLine);
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
DWORD LoadPlaylist_XSPF(LPCWSTR lpwFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMNodeList *pTrackList = 0;
	IXMLDOMNode *pTrack = 0, *pLocation = 0;
	VARIANT varFileName = { 0 };
	VARIANT_BOOL varResult;
	BSTR bstrTmp;
	ULONG lFileCnt = 0;
	WCHAR lpwLine[MAX_PATH] = { 0 };
	if (FAILED(CoCreateInstance(__uuidof(DOMDocument), 0, CLSCTX_INPROC_SERVER,
		__uuidof(IXMLDOMDocument), (LPVOID *)&pXMLDOMDocument))) return 0;
	pXMLDOMDocument->put_async(VARIANT_FALSE);
	pXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
	pXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
	varFileName.vt = VT_BSTR;
	varFileName.bstrVal = SysAllocString(lpwFileName);
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
		CheckPath(lpwFileName, bstrTmp, lpwLine);
		if (IsFile(lpwLine) || IsURL(lpwLine))
		{
			pFileCollection->AppendFile(lpwLine);
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
DWORD LoadPlaylist_PLS(LPCWSTR lpwFileName)
{
	ULONG i = 0, lNumOfEntries, lFileCnt = 0;
	WCHAR lpwKey[32] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	lNumOfEntries = GetPrivateProfileInt(L"playlist", L"numberofentries", 0, lpwFileName);
	if (!lNumOfEntries) return 0;
	for (; i <= lNumOfEntries; i++)
	{
		swprintf(lpwKey, L"file%i", i);
		GetPrivateProfileString(L"playlist", lpwKey, 0, lpwLine, MAX_PATH, lpwFileName);
		CheckPath(lpwFileName, lpwLine, lpwLine);
		if (IsFile(lpwLine))
		{
			pFileCollection->AppendFile(lpwLine);
			lFileCnt++;
		}
	}
	return lFileCnt;
}

//Загрузка списка в формате MPCPL (Media Player Classic Playlist), кодировка неизвестна
DWORD LoadPlaylist_MPCPL(LPCWSTR lpwFileName)
{
	FILE *pPlaylist;
	ULONG lLineSize, lLineCnt = 0, lArgsCnt, lFileCnt = 0;
	BOOL bEncInUTF8 = FALSE;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	LPWSTR pArgs[3];
	pPlaylist = _wfopen(lpwFileName, L"rt");
	while (!feof(pPlaylist))
	{
		ZeroMemory(lpLine, sizeof(lpLine));
		ZeroMemory(lpwLine, sizeof(lpwLine));
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
			MultiByteToWideChar(CP_UTF8, 0, lpLine, -1, lpwLine, sizeof(lpwLine) / sizeof(WCHAR));
		}
		else
		{
			MultiByteToWideChar(CP_ACP, 0, lpLine, -1, lpwLine, sizeof(lpwLine) / sizeof(WCHAR));
		}
		lLineSize = TrimNLChr(lpwLine);
		ZeroMemory(pArgs, sizeof(pArgs));
		lArgsCnt = SP_Split(lpwLine, &pArgs[0], ',', 3);
		if (lArgsCnt >= 3)
		{
			if (wcscmp(pArgs[1], L"filename") == 0)
			{
				CheckPath(lpwFileName, pArgs[2], lpwLine);
				if (IsFile(lpwLine))
				{
					
					pFileCollection->AppendFile(lpwLine);
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
void SavePlaylist_EBL(LPCWSTR lpwFileName)
{
	FILE *pPlaylist;
	ULONG i, lTime, lFCFileCnt = 0;
	char lpLine[MAX_PATH * sizeof(WCHAR)] = { 0 };
	char lpTmp1[256] = { 0 }, lpTmp2[256] = { 0 }, lpTmp3[/*64*/256] = { 0 };
	WCHAR lpwTitle[128] = { 0 };
	WCHAR lpwName[128] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	LPPLITEMDESC pPLID = 0;
	lFCFileCnt = pFileCollection->FileCount();
	pPlaylist = _wfopen(lpwFileName, L"wt");
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
	WideCharToMultiByte(CP_UTF8, 0, lpwAppVersionMM, -1, lpTmp3, sizeof(lpTmp3), 0, 0);
	fprintf(pPlaylist, "%s, %s, %s\n", lpTmp1, lpTmp2, lpTmp3);
	WideCharToMultiByte(CP_UTF8, 0, EBL_KEYWORD_TITLE, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
	SP_ExtractName(lpwFileName, lpwTitle);
	SP_ExtractLeftPart(lpwTitle, lpwTitle, '.');
	WideCharToMultiByte(CP_UTF8, 0, lpwTitle, -1, lpTmp2, sizeof(lpTmp2), 0, 0);
	fprintf(pPlaylist, "%s, %s\n\n", lpTmp1, lpTmp2);
	WideCharToMultiByte(CP_UTF8, 0, EBL_KEYWORD_MEDIASOURCES, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
	fprintf(pPlaylist, "%s, %i\n", lpTmp1, lFCFileCnt);
	for (; i < lFCFileCnt; i++)
	{
		pPLID = 0;
		pFileCollection->GetUserData(0, i, FCF_BYINDEX, (LONG_PTR&)pPLID);
		pFileCollection->GetFile(lpwLine, i, FCF_BYINDEX);
		if (pPLID)
		{
			lTime = pPLID->uDuration;
			wcscpy(lpwName, pPLID->lpwTitle);
		}
		else
		{
			lTime = 0;
			GetTitle(lpwLine, lpwName);
		}
		WideCharToMultiByte(CP_UTF8, 0, lpwLine, -1, lpLine, sizeof(lpLine), 0, 0);
		WideCharToMultiByte(CP_UTF8, 0, lpwName, -1, lpTmp1, sizeof(lpTmp1), 0, 0);
		fprintf(pPlaylist, "%i, %s, %s, %i\n", i + 1, lpLine, lpTmp1, lTime);
	}
	fclose(pPlaylist);
}

//Сохранение списка в формате M3U, кодировка ANSI
/*void SavePlaylist_M3U(LPCWSTR lpwFileName)
{
	FILE *pPlaylist;
	ULONG i = 0, lFCFileCnt = 0;
	char lpName[128] = { 0 };
	WCHAR lpwName[128] = { 0 };
	char lpLine[MAX_PATH] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	pPlaylist = _wfopen(lpwFileName, L"wt");
	fputs("#EXTM3U\n", pPlaylist);
	lFCFileCnt = pFileCollection->FileCount();
	for (; i < lFCFileCnt; i++)
	{
		pFileCollection->GetFile(lpwLine, i, FCF_BYINDEX);
		SP_ExtractName(lpwLine, lpwName);
		WideCharToMultiByte(CP_ACP, 0, lpwName, -1, lpName, sizeof(lpName), 0, 0);
		WideCharToMultiByte(CP_ACP, 0, lpwLine, -1, lpLine, sizeof(lpLine), 0, 0);
		fprintf(pPlaylist, "#EXTINF: 0, %s\n", lpName);
		fprintf(pPlaylist, "%s\n", lpLine);
	}
	fclose(pPlaylist);
}*/

//Сохранение списка в форматах M3U и M3U8, кодировка ANSI и UTF-8 соответственно
void SavePlaylist_M3U(LPCWSTR lpwFileName)
{
	FILE *pPlaylist;
	ULONG i = 0, lTime, lFCFileCnt = 0;
	char lpName[256] = { 0 };
	WCHAR lpwName[128] = { 0 };
	char lpLine[MAX_PATH * sizeof(WCHAR)] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
	LPPLITEMDESC pPLID = 0;
	pPlaylist = _wfopen(lpwFileName, L"wt");
	fputs(BOM_UTF8_TEXT, pPlaylist);
	fputs("#EXTM3U\n", pPlaylist);
	lFCFileCnt = pFileCollection->FileCount();
	for (; i < lFCFileCnt; i++)
	{
		pPLID = 0;
		pFileCollection->GetUserData(0, i, FCF_BYINDEX, (LONG_PTR&)pPLID);
		pFileCollection->GetFile(lpwLine, i, FCF_BYINDEX);
		if (pPLID)
		{
			lTime = pPLID->uDuration / 1000;
			wcscpy(lpwName, pPLID->lpwTitle);
		}
		else
		{
			lTime = 0;
			GetTitle(lpwLine, lpwName);
		}
		WideCharToMultiByte(CP_UTF8, 0, lpwName, -1, lpName, sizeof(lpName), 0, 0);
		WideCharToMultiByte(CP_UTF8, 0, lpwLine, -1, lpLine, sizeof(lpLine), 0, 0);
		fprintf(pPlaylist, "#EXTINF:%i,%s\n", lTime, lpName);
		fprintf(pPlaylist, "%s\n", lpLine);
	}
	fclose(pPlaylist);
}

//Сохранение списка в формате ASX, кодировка ?
void SavePlaylist_ASX(LPCWSTR lpwFileName)
{
	IXMLDOMDocument *pXMLDOMDocument = 0;
	IXMLDOMElement *pAsxAttributes = 0, *pRefAttributes = 0;
	IXMLDOMNode *pAsx = 0, *pTitle = 0, *pEntry = 0, *pInsertedNode = 0, *pRef = 0;
	IXMLDOMText *pWhiteSpace = 0;
	VARIANT varAsxVersion = { 0 }, varNodeType = { 0 }, varLine = { 0 },
		varFileName = { 0 };
	ULONG i = 0, lFCFileCnt = 0;
	WCHAR lpwTitle[128] = { 0 };
	WCHAR lpwLine[MAX_PATH] = { 0 };
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
	SP_ExtractName(lpwFileName, lpwTitle);
	SP_ExtractLeftPart(lpwTitle, lpwTitle, '.');
	pTitle->put_text(lpwTitle);
	pAsx->appendChild(pTitle, &pInsertedNode);
	pTitle->Release();
	lFCFileCnt = pFileCollection->FileCount();
	for (; i < lFCFileCnt; i++)
	{
		pFileCollection->GetFile(lpwLine, i, FCF_BYINDEX);
		pXMLDOMDocument->createTextNode(L"\n\t", &pWhiteSpace);
		pAsx->appendChild(pWhiteSpace, &pInsertedNode);
		pWhiteSpace->Release();
		pXMLDOMDocument->createNode(varNodeType, L"entry", 0, &pEntry);
		pAsx->appendChild(pEntry, &pInsertedNode);
		pXMLDOMDocument->createTextNode(L"\n\t\t", &pWhiteSpace);
		pEntry->appendChild(pWhiteSpace, &pInsertedNode);
		pWhiteSpace->Release();
		pXMLDOMDocument->createNode(varNodeType, L"title", 0, &pTitle);
		SP_ExtractName(lpwLine, lpwTitle);
		SP_ExtractLeftPart(lpwTitle, lpwTitle, '.');
		pTitle->put_text(lpwTitle);
		pEntry->appendChild(pTitle, &pInsertedNode);
		pTitle->Release();
		pXMLDOMDocument->createTextNode(L"\n\t\t", &pWhiteSpace);
		pEntry->appendChild(pWhiteSpace, &pInsertedNode);
		pWhiteSpace->Release();
		pXMLDOMDocument->createNode(varNodeType, L"ref", 0, &pRef);
		pRef->QueryInterface(__uuidof(IXMLDOMElement), (LPVOID *)&pRefAttributes);
		varLine.vt = VT_BSTR;
		varLine.bstrVal = SysAllocString(lpwLine);
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
	varFileName.bstrVal = SysAllocString(lpwFileName);
	pXMLDOMDocument->save(varFileName);
	VariantClear(&varNodeType);
	VariantClear(&varAsxVersion);
	VariantClear(&varFileName);
	pAsxAttributes->Release();
	pAsx->Release();
	pXMLDOMDocument->Release();
}

//Создание абсолютного пути к файлу из относительного
void CheckPath(LPCWSTR lpwPlaylist, LPCWSTR lpwFile, LPWSTR lpwResult)
{
	WCHAR lpwPLDir[MAX_PATH] = { 0 };
	WCHAR lpwFullPath[MAX_PATH] = { 0 };
	LPWSTR lpwFilePart;
	SP_ExtractDirectory(lpwPlaylist, lpwPLDir);
	SetCurrentDirectory(lpwPLDir);
	GetFullPathName(lpwFile, MAX_PATH, lpwFullPath, &lpwFilePart);
	if (IsFile(lpwFullPath))
	{
		wcscpy(lpwResult, lpwFullPath);
	}
	else
	{
		wcscpy(lpwResult, lpwFile);
	}
}

//Удаление символа новой строки
DWORD TrimNLChr(LPWSTR lpwLine)
{
	ULONG lLineSize = wcslen(lpwLine);
	if (lpwLine[lLineSize - 1] == '\n')
	{
		lpwLine[lLineSize - 1] = '\0';
		lLineSize--;
	}
	return lLineSize;
}
