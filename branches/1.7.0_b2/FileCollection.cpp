//Работа со списком файлов

#include <windows.h>

#include "filecollection.h"

CFileCollection::CFileCollection()
{
	ULONG i = 0;
	m_lpwCurrentFile = NULL;
	for (; i < FC_MAX_FILES; i++)
		m_pFileCollection[i] = NULL;
	m_hCBWnd = NULL;
}

CFileCollection::~CFileCollection()
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			delete[] m_pFileCollection[i]->lpwPath;
			delete m_pFileCollection[i];
		}
	}
}

//Функция возвращает количество файлов в коллекции (счет с единицы)
int CFileCollection::FileCount()
{
	ULONG i = 0;
	if (m_pFileCollection[0] == NULL) return 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i] == NULL)
			return i;
	}
	return FC_MAX_FILES;
}

//Функция добавляет файл к коллекции
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::AppendFile(LPWSTR lpwFileName)
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i] == NULL)
		{
			m_pFileCollection[i] = new FCSTRUCT;
			m_pFileCollection[i]->lpwPath = new WCHAR[MAX_PATH];
			wcscpy(m_pFileCollection[i]->lpwPath, lpwFileName);
			m_pFileCollection[i]->dwRndIndex = 0;
			m_pFileCollection[i]->lpUserData = NULL;
			if (m_hCBWnd)
				SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEADDED), (LPARAM)lpwFileName);
			SetCurrentFile(lpwFileName);
			break;
		}
		else
		{
			if (i == (FC_MAX_FILES - 1)) return 0;
			if (_wcsicmp(m_pFileCollection[i]->lpwPath, lpwFileName) == 0) return 0;
		}
	}
	return 1;
}

//Функция возвращает в lpwFileName текущий или соответствующий указанному
//индексу (счет с нуля) файл (без инициализации)
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::GetFile(LPWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag)
{
	switch (dwFlag)
	{
		case FCF_RECENT:
			if (m_lpwCurrentFile == NULL) return 0;
			wcscpy(lpwFileName, m_lpwCurrentFile);
			break;
		case FCF_BYINDEX:
			if (dwIndex >= FC_MAX_FILES) return 0;
			if (m_pFileCollection[dwIndex] == NULL) return 0;
			wcscpy(lpwFileName, m_pFileCollection[dwIndex]->lpwPath);
			break;
		default:
			return 0;
	}
	return 1;
}

//Функция возвращает индекс указанного или текущего файла (счет с нуля)
//В случае ошибки функция возвращает значение меньше нуля
int CFileCollection::GetFileIndex(LPCWSTR lpwFileName, DWORD dwFlag)
{
	ULONG i = 0;
	if (m_pFileCollection[0] == NULL) return -1;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_RECENT)
			{
				if (m_lpwCurrentFile == NULL) break;
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
				{
					return i;
				}
			}
			else if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, lpwFileName) == 0)
				{
					return i;
				}
			}
			else break;
		}
		else break;
	}
	return -1;
}

//Функция инициализирует и возвращает в lpwFileName следующий, по индексу (счет с нуля) или
//предыдущий файл в коллекции
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::NextFile(LPWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag)
{
	ULONG i;
	if (m_pFileCollection[0] == NULL) return 0;
	//Если требуется первый - возвращаем первый...
	if (dwFlag == FCF_FIRST)
	{
		wcscpy(lpwFileName, m_pFileCollection[0]->lpwPath);
		goto ExitFunction;
	}
	//Пытаемся вернуть файл по индексу
	if (dwFlag == FCF_BYINDEX)
	{
		if (dwIndex >= FC_MAX_FILES) return 0;
		if (m_pFileCollection[dwIndex])
		{
			wcscpy(lpwFileName, m_pFileCollection[dwIndex]->lpwPath);
		}
		else return 0;
		goto ExitFunction;
	}
	//Пытаемся вернуть случайный файл из коллекции
	if (dwFlag == FCF_RANDOM)
	{
		ULONG lFileCnt = 0, lIndex = 0;
		for (i = 0; i < FC_MAX_FILES; i++)
		{
			if (m_pFileCollection[i] == NULL)
			{
				lFileCnt = i;
				break;
			}
			else
			{
				if (i == (FC_MAX_FILES - 1)) return 0;
			}
		}
		Randomize();
Random_NextTry:
		lIndex = Random((int)lFileCnt);
		if (m_pFileCollection[lIndex]->dwRndIndex == 0)
		{
			wcscpy(lpwFileName, m_pFileCollection[lIndex]->lpwPath);
			m_pFileCollection[lIndex]->dwRndIndex++;
			goto ExitFunction;
		}
		else
		{
			for (i = 0; i < lFileCnt; i++)
			{
				if (m_pFileCollection[i]->dwRndIndex == 0)
				{
					goto Random_NextTry;
				}
			}
			for (i = 0; i < lFileCnt; i++)
			{
				m_pFileCollection[i]->dwRndIndex = 0;
			}
			return 0;
		}
	}
	if (m_lpwCurrentFile == NULL) return 0;
	//Ищем текущий файл в коллекции...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
			{
				if (dwFlag == FCF_FORWARD)
				{
					if (i == (FC_MAX_FILES - 1)) return 0;
					//Если следующего нет, возвращаем текущий
					if (!m_pFileCollection[i + 1])
					{
						wcscpy(lpwFileName, m_pFileCollection[i]->lpwPath);
					}
					else
					{
						wcscpy(lpwFileName, m_pFileCollection[i + 1]->lpwPath);
					}
					break;
				}
				else if (dwFlag == FCF_BACKWARD)
				{
					if (i == 0) return 0;
					//Если предыдущего нет, возвращаем текущий
					if (!m_pFileCollection[i - 1])
					{
						wcscpy(lpwFileName, m_pFileCollection[i]->lpwPath);
					}
					else
					{
						wcscpy(lpwFileName, m_pFileCollection[i - 1]->lpwPath);
					}
					break;
				}
				else if (dwFlag == FCF_RECENT)
				{
					wcscpy(lpwFileName, m_pFileCollection[i]->lpwPath);
					break;
				}
				else return 0;
			}
		}
		else return 0; //Каким-то чудом найти не удалось
	}
ExitFunction:
	SetCurrentFile(lpwFileName);
	return 1;
}

//Функция перемещает в коллекции указанный или по индексу файл на указанную позицию (индекс)
//Нужно учитывать, что коллекция не может быть с разрывами, т.е. переместить файл далее, чем
//_индекс_последнего_файла_ нельзя
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::MoveFile(LPCWSTR lpwFileName, DWORD dwIndex, DWORD dwNewIndex, DWORD dwFlag)
{
	UINT i, uStartIndex;
	if (m_pFileCollection[0] == NULL) return 0;
	if (m_pFileCollection[dwNewIndex] == NULL) return 0;
	if (dwFlag == FCF_BYINDEX)
	{
		if (dwIndex == dwNewIndex) return 0;
		if (m_pFileCollection[dwIndex])
		{
			uStartIndex = dwIndex;
			goto MoveFile_Begin;
		} else return 0;
	}
	else
	{
		for (i = 0; i < FC_MAX_FILES; i++)
		{
			if (m_pFileCollection[i])
			{
				if (dwFlag == FCF_BYFILENAME)
				{
					if (_wcsicmp(m_pFileCollection[i]->lpwPath, lpwFileName) == 0)
					{
						if (i == dwNewIndex) return 0;
						uStartIndex = i;
						goto MoveFile_Begin;
					}
				}
				else if (dwFlag == FCF_RECENT)
				{
					if (m_lpwCurrentFile == NULL) return 0;
					if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
					{
						if (i == dwNewIndex) return 0;
						uStartIndex = i;
						goto MoveFile_Begin;
					}
				}
				else break;
			}
			else break;
		}
		return 0;
	}
MoveFile_Begin:
	//У нас есть индекс...
	LPFCSTRUCT pFCST = m_pFileCollection[uStartIndex];
	if (dwNewIndex > uStartIndex)
	{
		for (i = uStartIndex; i < dwNewIndex; i++)
			m_pFileCollection[i] = m_pFileCollection[i + 1];
	}
	else
	{
		for (i = uStartIndex; i > dwNewIndex; i--)
			m_pFileCollection[i] = m_pFileCollection[i - 1];
	}
	m_pFileCollection[dwNewIndex] = pFCST;
	return 1;
}

//Функция удаляет из коллекции указанный или по индексу файл
//Пожалуй, это самая сложная функция класса...
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::DeleteFile(LPCWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag)
{
	ULONG i, j;
	LPFCSTRUCT pFCST;
	if (m_pFileCollection[0] == NULL) return 0;
	//Сдвигаем в коллекции файлы, начиная с указанного индекса
	//Последний заполненный элемент удаляем...
	if (dwFlag == FCF_BYINDEX)
	{
		if (m_pFileCollection[dwIndex] == NULL) return 0;
		WCHAR lpwFile[MAX_PATH] = { 0 };
		wcscpy(lpwFile, m_pFileCollection[dwIndex]->lpwPath);
		pFCST = m_pFileCollection[dwIndex];
		for (i = dwIndex; i < FC_MAX_FILES; i++)
		{
			if (m_pFileCollection[i])
			{
				if (i < (FC_MAX_FILES - 1))
				{
					if (m_pFileCollection[i + 1])
					{
						m_pFileCollection[i] = m_pFileCollection[i + 1];
					}
					else
					{
						m_pFileCollection[i] = NULL;
						delete[] pFCST->lpwPath;
						delete pFCST;
						break;
					}
				}
				else
				{
					m_pFileCollection[i] = NULL;
					delete[] pFCST->lpwPath;
					delete pFCST;
				}
			}
			else return 0;
		}
		if (m_hCBWnd)
			SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEDELETED), dwIndex);
		if (_wcsicmp(lpwFile, m_lpwCurrentFile) == 0)
		{
			if (dwIndex > 0)
			{
				SetCurrentFile((m_pFileCollection[dwIndex])?
					m_pFileCollection[dwIndex]->lpwPath:m_pFileCollection[dwIndex - 1]->lpwPath);
			}
			else
			{
				SetCurrentFile((m_pFileCollection[dwIndex])?m_pFileCollection[dwIndex]->lpwPath:NULL);
			}
		}
		return 1;
	}
	//Для получения индекса требуется найти файл...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, lpwFileName) == 0)
				{
					pFCST = m_pFileCollection[i];
					for (j = i; j < FC_MAX_FILES; j++)
					{
						if (j < (FC_MAX_FILES - 1))
						{
							if (m_pFileCollection[j + 1])
							{
								m_pFileCollection[j] = m_pFileCollection[j + 1];
							}
							else
							{
								m_pFileCollection[j] = NULL;
								delete[] pFCST->lpwPath;
								delete pFCST;
								break;
							}
						}
						else
						{
							m_pFileCollection[j] = NULL;
							delete[] pFCST->lpwPath;
							delete pFCST;
						}
					}
					if (m_hCBWnd)
						SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEDELETED), i);
					if (_wcsicmp(lpwFileName, m_lpwCurrentFile) == 0)
					{
						if (i > 0)
						{
							SetCurrentFile((m_pFileCollection[i])?
								m_pFileCollection[i]->lpwPath:m_pFileCollection[i - 1]->lpwPath);
						}
						else
						{
							SetCurrentFile((m_pFileCollection[i])?m_pFileCollection[i]->lpwPath:NULL);
						}

					}
					return 1;
				}
			}
			else if (dwFlag == FCF_RECENT)
			{
				if (m_lpwCurrentFile == NULL) return 0;
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
				{
					pFCST = m_pFileCollection[i];
					for (j = i; j < FC_MAX_FILES; j++)
					{
						if (j < (FC_MAX_FILES - 1))
						{
							if (m_pFileCollection[j + 1])
							{
								m_pFileCollection[j] = m_pFileCollection[j + 1];
							}
							else
							{
								m_pFileCollection[j] = NULL;
								delete[] pFCST->lpwPath;
								delete pFCST;
								break;
							}
						}
						else
						{
							m_pFileCollection[j] = NULL;
							delete[] pFCST->lpwPath;
							delete pFCST;
						}
					}
					if (m_hCBWnd)
						SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEDELETED), i);
					if (i > 0)
					{
						SetCurrentFile((m_pFileCollection[i])?
							m_pFileCollection[i]->lpwPath:m_pFileCollection[i - 1]->lpwPath);
					}
					else
					{
						SetCurrentFile((m_pFileCollection[i])?m_pFileCollection[i]->lpwPath:NULL);
					}
					return 1;
				}
			}
			else break;
		}
		else break;
	}
	return 0;
}

//Функция очищает коллекцию
void CFileCollection::Clear()
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			delete[] m_pFileCollection[i]->lpwPath;
			delete m_pFileCollection[i];
			m_pFileCollection[i] = NULL;
		}
	}
	if (m_hCBWnd)
		SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_COLCLEARED), 0);
	SetCurrentFile(NULL);
}

//Функция проверяет, существует ли в коллекции следующий (или предыдущий) файл
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::IsFileAvailable(DWORD dwFlag)
{
	ULONG i = 0;
	if (m_pFileCollection[0] == NULL) return 0;
	if (m_lpwCurrentFile == NULL) return 0;
	//Пытаемся найти текущий файл в коллекции
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
			{
				if (dwFlag == FCF_FORWARD)
				{
					if (i == (FC_MAX_FILES - 1)) return 0;
					if (m_pFileCollection[i + 1]) break; else return 0;
				}
				else if (dwFlag = FCF_BACKWARD)
				{
					if (i == 0) return 0;
					if (m_pFileCollection[i - 1]) break; else return 0;
				}
				else return 0;
			}
		}
		else return 0; //Найти не удалось... Странно.
	}
	return 1;
}

//Функция задает последний файл
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::SetRecentFile(LPCWSTR lpwFileName)
{
	if (_wcsicmp(m_lpwCurrentFile, lpwFileName) == 0) return 0;
	SetCurrentFile(lpwFileName);
	return 1;
}

//Функция сопоставляет пользовательские данные файлу (по индексу или по имени) в коллекции
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::SetUserData(LPCWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag, LONG_PTR lpUD)
{
	ULONG i;
	if (m_pFileCollection[0] == NULL) return 0;
	//Смотрим по индексу
	if (dwFlag == FCF_BYINDEX)
	{
		if (m_pFileCollection[dwIndex])
		{
			m_pFileCollection[dwIndex]->lpUserData = lpUD;
			if (m_hCBWnd)
				SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEUDCHANGED), dwIndex);
	
		} else return 0;
		return 1;
	}
	//Для получения индекса требуется найти файл...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, lpwFileName) == 0)
				{
					
					m_pFileCollection[i]->lpUserData = lpUD;
					if (m_hCBWnd)
						SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEUDCHANGED), i);
					return 1;
				}
			}
			else if (dwFlag == FCF_RECENT)
			{
				if (m_lpwCurrentFile == NULL) return 0;
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
				{
					m_pFileCollection[i]->lpUserData = lpUD;
					if (m_hCBWnd)
						SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEUDCHANGED), i);
					return 1;
				}
			}
			else break;
		}
		else break;
	}
	return 0;
}

//Функция возвращает пользовательские данные, которые сопоставленны с указанным файлом
//В случае успеха функция возвращает значение больше нуля, в случае ошибки - нуль
int CFileCollection::GetUserData(LPCWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag, LONG_PTR& lpUD)
{
	ULONG i;
	if (m_pFileCollection[0] == NULL) return 0;
	//Смотрим по индексу
	if (dwFlag == FCF_BYINDEX)
	{
		if (m_pFileCollection[dwIndex])
			lpUD = m_pFileCollection[dwIndex]->lpUserData;
		else return 0;
		return 1;
	}
	//Для получения индекса требуется найти файл...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, lpwFileName) == 0)
				{
					lpUD = m_pFileCollection[i]->lpUserData;
					return 1;
				}
			}
			else if (dwFlag == FCF_RECENT)
			{
				if (m_lpwCurrentFile == NULL) return 0;
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
				{
					lpUD = m_pFileCollection[i]->lpUserData;
					return 1;
				}
			}
			else break;
		}
		else break;
	}
	return 0;
}

//Функция задает окно обратного вызова
//Т.е. окно, которое будет получать уведомления о событиях
void CFileCollection::SetCallbackWnd(HWND hWnd)
{
	if (IsWindow(hWnd))
		m_hCBWnd = hWnd;
}

//Функция возвращает окно обратного вызова
HWND CFileCollection::GetCallbackWnd()
{
	return m_hCBWnd;
}

//Приватная функция для инициализации текущего файла
void CFileCollection::SetCurrentFile(LPCWSTR lpwPath)
{
	if (m_lpwCurrentFile)
		delete[] m_lpwCurrentFile;
	if (lpwPath)
	{
		m_lpwCurrentFile = new WCHAR[MAX_PATH];
		wcscpy(m_lpwCurrentFile, lpwPath);
	}
	else m_lpwCurrentFile = NULL;
	if (m_hCBWnd)
		SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_CURFILECHANGED), (LPARAM)lpwPath);
}
