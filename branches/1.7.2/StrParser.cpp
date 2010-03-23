//Модуль разбора строк

#include <windows.h>

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include <tchar.h>
#include <locale.h>

#include "strparser.h"

/* Можно использовать _tsplitpath как альтернативу некоторым ниже приведенным функциям */

//Функция извлекает путь к папке из полного пути
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_ExtractDirectory(LPCTSTR lpFullPath, LPTSTR lpResult)
{
    SIZE_T szPathSize = _tcslen(lpFullPath);
	BOOL bS = FALSE;
	if (szPathSize <= 1) return 0;
	if ((_tcschr(lpFullPath, '\\') == 0) && (_tcschr(lpFullPath, '/') == 0)) return 0;
	if (_tcsrchr(lpFullPath, '/') > _tcsrchr(lpFullPath, '\\')) bS = TRUE;
	LPTSTR lpDir = new TCHAR[szPathSize + 1];
	ZeroMemory(lpDir, (szPathSize + 1) * sizeof(TCHAR));
	_tcscpy(lpDir, lpFullPath);
	lpDir[_tcsrchr(lpDir, (bS)?'/':'\\') - lpDir] = '\0';
	_tcscpy(lpResult, lpDir);
	delete[] lpDir;
	return _tcslen(lpResult);
}

//Функция извлекает имя файла из полного пути
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_ExtractName(LPCTSTR lpPath, LPTSTR lpResult)
{
    SIZE_T szPathSize = _tcslen(lpPath);
	BOOL bS = FALSE;
	if (szPathSize <= 1) return 0;
	if ((_tcschr(lpPath, '\\') == 0) && (_tcschr(lpPath, '/') == 0)) return 0;
	if (_tcsrchr(lpPath, '/') > _tcsrchr(lpPath, '\\')) bS = TRUE;
	LPTSTR lpOrigName = new TCHAR[szPathSize + 1];
	ZeroMemory(lpOrigName, (szPathSize + 1) * sizeof(TCHAR));
	LPTSTR lpName = lpOrigName;
	_tcscpy(lpName, lpPath);
	lpName = _tcsrchr(lpName, (bS)?'/':'\\');
	lpName++;
	_tcscpy(lpResult, lpName);
	delete[] lpOrigName;
	return _tcslen(lpResult);
}

//Функция извлекает левую часть строки (до указанного символа)
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_ExtractLeftPart(LPCTSTR lpString, LPTSTR lpResult, TCHAR ch)
{
	SIZE_T szStrSize = _tcslen(lpString);
	if (szStrSize <= 1) return 0;
	if (_tcschr(lpString, ch) == 0) return 0;
	LPTSTR lpLeftPart = new TCHAR[szStrSize + 1];
	ZeroMemory(lpLeftPart, (szStrSize + 1) * sizeof(TCHAR));
	_tcscpy(lpLeftPart, lpString);
	lpLeftPart[_tcsrchr(lpLeftPart, ch) - lpLeftPart] = '\0';
	_tcscpy(lpResult, lpLeftPart);
	delete[] lpLeftPart;
	return _tcslen(lpResult);
}

//Функция извлекает правую часть строки (после указанного символа)
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_ExtractRightPart(LPCTSTR lpString, LPTSTR lpResult, TCHAR ch)
{
	SIZE_T szStrSize = _tcslen(lpString);
	if (szStrSize <= 1) return 0;
	if (_tcschr(lpString, ch) == 0) return 0;
	LPTSTR lpOrigRightPart = new TCHAR[szStrSize + 1];
	ZeroMemory(lpOrigRightPart, (szStrSize + 1) * sizeof(TCHAR));
	LPTSTR lpRightPart = lpOrigRightPart;
	_tcscpy(lpRightPart, lpString);
	lpRightPart = _tcsrchr(lpRightPart, ch);
	lpRightPart++;
	_tcscpy(lpResult, lpRightPart);
	delete[] lpOrigRightPart;
	return _tcslen(lpResult);
}

//Функция вырезает с границ строки указанный символ
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_TrimEx(LPCTSTR lpString, LPTSTR lpResult, TCHAR ch)
{
	SIZE_T szStrSize = _tcslen(lpString);
	if (szStrSize <= 1) return 0;
	LPTSTR lpOrigTemp = new TCHAR[szStrSize + 1];
	ZeroMemory(lpOrigTemp, (szStrSize + 1) * sizeof(TCHAR));
	LPTSTR lpTemp = lpOrigTemp;
	_tcscpy(lpTemp, lpString);
	SIZE_T i = 0;
	for (; lpTemp[0] == ch && i < szStrSize; i++)
		lpTemp++;
	for (i = (_tcslen(lpTemp) - 1); lpTemp[i] == ch && i > 0; i--)
		lpTemp[i] = '\0';
	_tcscpy(lpResult, lpTemp);
	delete[] lpOrigTemp;
	return _tcslen(lpResult);
}

//Функция добавляет к концу пути разделитель (если его там нет)
//Разделителем для пути служат косая и обратная косая
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_AddDirSep(LPCTSTR lpDir, LPTSTR lpResult)
{
	SIZE_T szStrSize = _tcslen(lpDir);
	if (szStrSize <= 1) return 0;
	_tcscpy(lpResult, lpDir);
	if ((lpDir[szStrSize - 1] != '\\') && (lpDir[szStrSize - 1] != '/'))
	{
		_tcscat(lpResult, TEXT("\\"));
	}
	return _tcslen(lpResult);
}

//Функция убирает с конца пути разделитель
//Разделителем для пути служат косая и обратная косая
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_RemDirSep(LPCTSTR lpDir, LPTSTR lpResult)
{
	SIZE_T szStrSize = _tcslen(lpDir);
	if (szStrSize <= 1) return 0;
	_tcscpy(lpResult, lpDir);
	if ((lpDir[szStrSize - 1] == '\\') || (lpDir[szStrSize - 1] == '/'))
	{
		lpResult[szStrSize - 1] = '\0';
	}
	return _tcslen(lpResult);
}

//Функция преобразовывает подстроки в строке в элементы массива
//Предполагается, что строка завершена нулевым символом, и нулевой символ не является
//разделителем для подстрок в строке
//Вызывающий ответственен за освобождение элементов массива (путем вызова delete[] %elem%)
//Возвращаемое значение - количество заполненых элементов в массиве
DWORD SP_Split(LPCTSTR lpString, LPTSTR *pArray, TCHAR chDelimiter, DWORD dwMaxNumOfElem)
{
	DWORD dwArgNum = 0;
	SIZE_T i = 0, szPrevPos = 0, szStrSize = (_tcslen(lpString) + 1), szElemSize, szElemSizeB;
	if (szStrSize <= 1) return 0;
	for (; i <= szStrSize; i++)
	{
		//Не будем использовать _tcstok, так как она изменяет обрабатываемую строку, а следовательно
		//потребуется выделение временного буфера...
		if ((lpString[i] == chDelimiter) || (lpString[i] == '\0'))
		{
			szElemSize = i - szPrevPos;
			szElemSizeB = szElemSize * sizeof(TCHAR);
			pArray[dwArgNum] = new TCHAR[szElemSize + 1];
			ZeroMemory(pArray[dwArgNum], szElemSizeB + (1 * sizeof(TCHAR)));
			CopyMemory(pArray[dwArgNum], lpString + szPrevPos, szElemSizeB);
			dwArgNum++;
			if (dwArgNum == dwMaxNumOfElem) return dwMaxNumOfElem;
			if (lpString[i] == '\0') break;
			szPrevPos = i + 1;
		}
	}
	return dwArgNum;
}

//Функция производит поиск подстроки в строке начиная с указанной позиции
//Возвращаемое значение - позиция подстроки в строке, -1 в случае неудачи
SIZE_T SP_Find(LPCTSTR lpString, LPCTSTR lpFind, SIZE_T szStartPos, BOOL bMatchCase,
			   LPCTSTR lpLocale)
{
	SIZE_T szStrSize = _tcslen(lpString), szFindSize = _tcslen(lpFind),
		szResult = -1;
	if ((szStrSize <= 1) || (szFindSize < 1)) return -1;
	TCHAR lpOldLoc[64] = {0};
	if (lpLocale)
	{
		_tcsncpy(lpOldLoc, _tsetlocale(LC_CTYPE, NULL), 63);
		_tsetlocale(LC_CTYPE, lpLocale);
	}
	LPTSTR lpOrigStringTemp = new TCHAR[szStrSize + 1];
	LPTSTR lpStringTemp = lpOrigStringTemp;
	LPTSTR lpFindTemp = new TCHAR[szFindSize + 1];
	BOOL bMatch;
	//ZeroMemory(lpStringTemp, (szStrSize + 1) * sizeof(TCHAR));
	_tcscpy(lpStringTemp, lpString);
	lpStringTemp += szStartPos;
	do
	{
		//ZeroMemory(lpFindTemp, (szFindSize + 1) * sizeof(TCHAR));
		_tcsncpy(lpFindTemp, lpStringTemp, szFindSize);
		lpFindTemp[szFindSize] = '\0';
		bMatch = (bMatchCase)?(_tcscmp(lpFindTemp, lpFind) == 0):
			(_tcsicmp(lpFindTemp, lpFind) == 0);
		if (bMatch)
		{
			szResult = (lpStringTemp - lpOrigStringTemp);
			break;
		}
		else lpStringTemp++;
	}
	while (lpStringTemp[0] != '\0');
	delete[] lpOrigStringTemp;
	delete[] lpFindTemp;
	if (lpLocale)
		_tsetlocale(LC_CTYPE, lpOldLoc);
	return szResult;
}

//Функция производит в строке замену одной подстроки на другую
//Если заменяющая подстрока длиннее заменяемой, то результирующая строка будет
//длиннее исходной. Вызывающий должен это учесть перед вызовом функции
//Возвращаемое значение - длина обработанной строки
SIZE_T SP_Replace(LPCTSTR lpString, LPTSTR lpResult, LPCTSTR lpFind, LPCTSTR lpReplace,
				  BOOL bMatchCase, LPCTSTR lpLocale)
{
	SIZE_T szStrSize = _tcslen(lpString), szResultSize = _tcslen(lpResult),
		szFindSize = _tcslen(lpFind), szReplaceSize = _tcslen(lpReplace);
	if ((szStrSize <= 1) || (szFindSize < 1) || (szReplaceSize < 1)) return 0;
	TCHAR lpOldLoc[64] = {0};
	if (lpLocale)
	{
		_tcsncpy(lpOldLoc, _tsetlocale(LC_CTYPE, NULL), 63);
		_tsetlocale(LC_CTYPE, lpLocale);
	}
	LPTSTR lpOrigStringTemp = new TCHAR[szStrSize + 1];
	LPTSTR lpStringTemp = lpOrigStringTemp;
	LPTSTR lpFindTemp = new TCHAR[szFindSize + 1];
	BOOL bMatch;
	//ZeroMemory(lpStringTemp, (szStrSize + 1) * sizeof(TCHAR));
	_tcscpy(lpStringTemp, lpString);
	if (szResultSize)
	{
		//ZeroMemory(lpResult, (szResultSize + 1) * sizeof(TCHAR));
		lpResult[0] = '\0';
	}
	do
	{
		//ZeroMemory(lpFindTemp, (szFindSize + 1) * sizeof(TCHAR));
		_tcsncpy(lpFindTemp, lpStringTemp, szFindSize);
		lpFindTemp[szFindSize] = '\0';
		bMatch = (bMatchCase)?(_tcscmp(lpFindTemp, lpFind) == 0):
			(_tcsicmp(lpFindTemp, lpFind) == 0);
		if (bMatch)
		{
			_tcscat(lpResult, lpReplace);
			lpStringTemp += szFindSize;
		}
		else
		{
			_tcsncat(lpResult, lpStringTemp, 1);
			lpStringTemp++;
		}
	}
	while (lpStringTemp[0] != '\0');
	delete[] lpOrigStringTemp;
	delete[] lpFindTemp;
	if (lpLocale)
		_tsetlocale(LC_CTYPE, lpOldLoc);
	return _tcslen(lpResult);
}
