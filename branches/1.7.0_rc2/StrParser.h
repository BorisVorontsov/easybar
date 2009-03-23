#ifndef STRPARSER_H
#define STRPARSER_H

SIZE_T SP_ExtractDirectory(LPCTSTR lpFullPath, LPTSTR lpResult);
SIZE_T SP_ExtractName(LPCTSTR lpPath, LPTSTR lpResult);
SIZE_T SP_ExtractLeftPart(LPCTSTR lpText, LPTSTR lpResult, TCHAR ch);
SIZE_T SP_ExtractRightPart(LPCTSTR lpText, LPTSTR lpResult, TCHAR ch);
SIZE_T SP_TrimEx(LPCTSTR lpString, LPTSTR lpResult, TCHAR ch);
SIZE_T SP_AddDirSep(LPCTSTR lpDir, LPTSTR lpResult);
SIZE_T SP_RemDirSep(LPCTSTR lpDir, LPTSTR lpResult);
DWORD SP_Split(LPCTSTR lpString, LPTSTR *pArray, TCHAR chDelimiter, DWORD dwMaxNumOfElem);
SIZE_T SP_Find(LPCTSTR lpString, LPCTSTR lpFind, SIZE_T szStartPos = 0, BOOL bMatchCase = TRUE
			   , LPCTSTR lpLocale = NULL);
SIZE_T SP_Replace(LPCTSTR lpString, LPTSTR lpResult, LPCTSTR lpFind, LPCTSTR lpReplace,
				  BOOL bMatchCase = TRUE, LPCTSTR lpLocale = NULL);

#endif