//Ассоциация [связывание] расширений (файлов) с программой

#include <windows.h>
#include <stdio.h>
#include <shlobj.h>

#include "fileassociations.h"

CFileAssociations::CFileAssociations()
{
	ZeroMemory(m_lpAppPath, sizeof(m_lpAppPath));
	m_dwIconIndex = 0;
	ZeroMemory(m_lpRegAppKey, sizeof(m_lpRegAppKey));
	ZeroMemory(m_lpFileDesc, sizeof(m_lpFileDesc));
	for (ULONG i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
		m_pCustomActions[i] = 0;
}

CFileAssociations::~CFileAssociations()
{
	for (ULONG i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
	{
		if (m_pCustomActions[i]) delete m_pCustomActions[i];
	}
}

//Функция добавляет дополнительное действие в список дополнительных действий
BOOL CFileAssociations::AddCustomAction(LPCWSTR lpRegCmdKey, LPCWSTR lpMenuName,
										LPCWSTR lpCmdLine, BOOL bDefault)
{
	for (ULONG i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
	{
		if (m_pCustomActions[i] == 0)
		{
			m_pCustomActions[i] = new FACUSTOMACTION;
			wcscpy(m_pCustomActions[i]->lpRegCmdKey, lpRegCmdKey);
			wcscpy(m_pCustomActions[i]->lpMenuName, lpMenuName);
			wcscpy(m_pCustomActions[i]->lpCmdLine, lpCmdLine);
			m_pCustomActions[i]->bDefault = bDefault;
			break;
		}
		else
		{
			if (i == (FA_MAX_CUSTOMACTIONS - 1)) return FALSE;
		}
	}
	return TRUE;
}

//Функция очищает список дополнительных действий
void CFileAssociations::ClearCustomActions()
{
	for (ULONG i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
	{
		if (m_pCustomActions[i]) delete m_pCustomActions[i];
		m_pCustomActions[i] = 0;
	}
}

//Функция связывает указанное расширение с программой
void CFileAssociations::AddAssociation(LPCWSTR lpExtension, BOOL bCreateStdAction)
{
	if ((!wcslen(m_lpAppPath)) || (!wcslen(m_lpRegAppKey)) || (!wcslen(m_lpFileDesc))) return;
	ULONG i;
	LPWSTR lpExtTmp = new WCHAR[wcslen(lpExtension) + 1];
    WCHAR lpKeyName[64] = {};
    WCHAR lpData[MAX_PATH] = {};
    HKEY hKey, hSubKey;
	DWORD dwDisposition, dwSZSIZE;
	wcscpy(lpExtTmp, lpExtension);
	_wcslwr(lpExtTmp);
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, m_lpRegAppKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, m_lpRegAppKey, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, 0, &hKey, &dwDisposition) == ERROR_SUCCESS)
		{
			RegSetValueEx(hKey, 0, 0, REG_SZ, (CONST LPBYTE)m_lpFileDesc, (DWORD)(wcslen(m_lpFileDesc) + 1) * sizeof(WCHAR));
			if (bCreateStdAction)
			{
				if (RegCreateKeyEx(hKey, L"shell\\open", 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
				{
					wcscpy(lpData, L"&Open");
					RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpData, (DWORD)(wcslen(lpData) + 1) * sizeof(WCHAR));
					RegCloseKey(hSubKey);
				}
				if (RegCreateKeyEx(hKey, L"shell\\open\\command", 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
				{
					swprintf(lpData, L"\"%s\" \"%%1\"", m_lpAppPath);
					RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpData, (DWORD)(wcslen(lpData) + 1) * sizeof(WCHAR));
					RegCloseKey(hSubKey);
				}
				if (RegOpenKeyEx(hKey, L"shell", 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
				{
					wcscpy(lpData, L"open");
					RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpData, (DWORD)(wcslen(lpData) + 1) * sizeof(WCHAR));
					RegCloseKey(hSubKey);
				}
			}
			for (i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
			{
				if (m_pCustomActions[i])
				{
					swprintf(lpKeyName, L"shell\\%s", m_pCustomActions[i]->lpRegCmdKey);
					if (RegCreateKeyEx(hKey, lpKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
					{
						RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)m_pCustomActions[i]->lpMenuName,
							(DWORD)(wcslen(m_pCustomActions[i]->lpMenuName) + 1) * sizeof(WCHAR));
						RegCloseKey(hSubKey);
					}
					swprintf(lpKeyName, L"shell\\%s\\command", m_pCustomActions[i]->lpRegCmdKey);
					if (RegCreateKeyEx(hKey, lpKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
					{
						RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)m_pCustomActions[i]->lpCmdLine,
							(DWORD)(wcslen(m_pCustomActions[i]->lpCmdLine) + 1) * sizeof(WCHAR));
						RegCloseKey(hSubKey);
					}
					if (m_pCustomActions[i]->bDefault)
					{
						if (RegOpenKeyEx(hKey, L"shell", 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
						{
							RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)m_pCustomActions[i]->lpRegCmdKey,
								(DWORD)(wcslen(m_pCustomActions[i]->lpRegCmdKey) + 1) * sizeof(WCHAR));
							RegCloseKey(hSubKey);
						}
					}
				}
			}
			if (RegCreateKeyEx(hKey, L"DefaultIcon", 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
			{
				swprintf(lpData, L"%s,%i", m_lpAppPath, m_dwIconIndex);
				RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpData, (DWORD)(wcslen(lpData) + 1) * sizeof(WCHAR));
				RegCloseKey(hSubKey);
			}
			RegCloseKey(hKey);
		}
	}
	else
	{
		RegCloseKey(hKey);
	}
    swprintf(lpKeyName, L".%s", lpExtTmp);
    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, lpKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hKey, &dwDisposition) == ERROR_SUCCESS)
	{
		ZeroMemory(lpData, sizeof(lpData));
		dwSZSIZE = sizeof(lpData);
		RegQueryValueEx(hKey, 0, 0, 0, (LPBYTE)lpData, &dwSZSIZE);
		if (wcslen(lpData))
		{
			RegSetValueEx(hKey, L"PreviousAssociation", 0, REG_SZ,
				(CONST LPBYTE)lpData, (DWORD)(wcslen(lpData) + 1) * sizeof(WCHAR));
		}
		RegSetValueEx(hKey, 0, 0, REG_SZ, (CONST LPBYTE)m_lpRegAppKey, (DWORD)(wcslen(m_lpRegAppKey) + 1) * sizeof(WCHAR));
		RegCloseKey(hKey);
	}
	delete[] lpExtTmp;
}

//Функция удаляет связь расширения с программой
//Если bCompletely будет равен TRUE - произойдет полное удаление, включая и 'm_lpRegAppKey'
void CFileAssociations::RemoveAssociation(LPCWSTR lpExtension, BOOL bCompletely)
{
	if (!wcslen(m_lpRegAppKey)) return;
	ULONG i;
	LPWSTR lpExtTmp = new WCHAR[wcslen(lpExtension) + 1];
    WCHAR lpKeyName[64] = {};
    WCHAR lpData[MAX_PATH] = {};
	HKEY hKey;
	DWORD dwSZSIZE;
	wcscpy(lpExtTmp, lpExtension);
	_wcslwr(lpExtTmp);
    if (bCompletely)
	{
        swprintf(lpKeyName, L"%s\\DefaultIcon", m_lpRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpKeyName);
        swprintf(lpKeyName, L"%s\\shell\\open\\command", m_lpRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpKeyName);
        swprintf(lpKeyName, L"%s\\shell\\open", m_lpRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpKeyName);
		for (i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
		{
			if (m_pCustomActions[i])
			{
				swprintf(lpKeyName, L"%s\\shell\\%s\\command", m_lpRegAppKey, m_pCustomActions[i]->lpRegCmdKey);
				RegDeleteKey(HKEY_CLASSES_ROOT, lpKeyName);
				swprintf(lpKeyName, L"%s\\shell\\%s", m_lpRegAppKey, m_pCustomActions[i]->lpRegCmdKey);
				RegDeleteKey(HKEY_CLASSES_ROOT, lpKeyName);
			}
		}
        swprintf(lpKeyName, L"%s\\shell", m_lpRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpKeyName);
        RegDeleteKey(HKEY_CLASSES_ROOT, m_lpRegAppKey);
    }
	swprintf(lpKeyName, L".%s", lpExtTmp);
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpKeyName, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		ZeroMemory(lpData, sizeof(lpData));
		dwSZSIZE = sizeof(lpData);
		RegQueryValueEx(hKey, L"PreviousAssociation", 0, 0, (LPBYTE)lpData, &dwSZSIZE);
		if (wcslen(lpData))
		{
			RegSetValueEx(hKey, 0, 0, REG_SZ, (CONST LPBYTE)lpData, (DWORD)(wcslen(lpData) + 1) * sizeof(WCHAR));
			RegDeleteValue(hKey, L"PreviousAssociation");
		}
		else
		{
			if (RegDeleteKey(hKey, 0) != ERROR_SUCCESS)
			{
				RegSetValueEx(hKey, 0, 0, REG_SZ, 0, 0);
			}
		}
		RegCloseKey(hKey);
	}
	delete[] lpExtTmp;
}

//Функция проверяет, связано ли указанное расширение с программой
BOOL CFileAssociations::IsAssociated(LPCWSTR lpExtension)
{
	if (!wcslen(m_lpRegAppKey)) return FALSE;
	LPWSTR lpExtTmp = new WCHAR[wcslen(lpExtension) + 1];
	WCHAR lpKeyName[64] = {};
	WCHAR lpData[MAX_PATH] = {};
	DWORD dwSZSIZE;
	HKEY hKey;
	BOOL bResult = FALSE;
	wcscpy(lpExtTmp, lpExtension);
	_wcslwr(lpExtTmp);
	swprintf(lpKeyName, L".%s", lpExtTmp);
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpKeyName, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		dwSZSIZE = sizeof(lpData);
		RegQueryValueEx(hKey, 0, 0, 0, (LPBYTE)lpData, &dwSZSIZE);
		if (_wcsicmp(lpData, m_lpRegAppKey) == 0) bResult = TRUE;
		RegCloseKey(hKey);
	}
	delete[] lpExtTmp;
	return bResult;
}

//Функция обновляет оболочку Windows
//Эту функцию следует вызывать после AddAssociation()/RemoveAssociation()
void CFileAssociations::UpdateShell()
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSHNOWAIT, 0, 0);
}