//Ассоциация [связывание] расширений (файлов) с программой

#include <windows.h>
#include <stdio.h>
#include <shlobj.h>

#include "fileassociations.h"

CFileAssociations::CFileAssociations()
{
	ZeroMemory(m_lpwAppPath, sizeof(m_lpwAppPath));
	m_dwIconIndex = 0;
	ZeroMemory(m_lpwRegAppKey, sizeof(m_lpwRegAppKey));
	ZeroMemory(m_lpwFileDesc, sizeof(m_lpwFileDesc));
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
BOOL CFileAssociations::AddCustomAction(LPCWSTR lpwRegCmdKey, LPCWSTR lpwMenuName,
										LPCWSTR lpwCmdLine, BOOL bDefault)
{
	for (ULONG i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
	{
		if (m_pCustomActions[i] == 0)
		{
			m_pCustomActions[i] = new FACUSTOMACTION;
			wcscpy(m_pCustomActions[i]->lpwRegCmdKey, lpwRegCmdKey);
			wcscpy(m_pCustomActions[i]->lpwMenuName, lpwMenuName);
			wcscpy(m_pCustomActions[i]->lpwCmdLine, lpwCmdLine);
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
void CFileAssociations::AddAssociation(LPCWSTR lpwExtension, BOOL bCreateStdAction)
{
	if ((!wcslen(m_lpwAppPath)) || (!wcslen(m_lpwRegAppKey)) || (!wcslen(m_lpwFileDesc))) return;
	ULONG i;
	LPWSTR lpwExtTmp = new WCHAR[wcslen(lpwExtension) + 1];
    WCHAR lpwKeyName[64] = { 0 };
    WCHAR lpwData[MAX_PATH] = { 0 };
    HKEY hKey, hSubKey;
	DWORD dwDisposition, dwSZSIZE;
	wcscpy(lpwExtTmp, lpwExtension);
	_wcslwr(lpwExtTmp);
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, m_lpwRegAppKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, m_lpwRegAppKey, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, 0, &hKey, &dwDisposition) == ERROR_SUCCESS)
		{
			RegSetValueEx(hKey, 0, 0, REG_SZ, (CONST LPBYTE)m_lpwFileDesc, (wcslen(m_lpwFileDesc) + 1) * sizeof(WCHAR));
			if (bCreateStdAction)
			{
				if (RegCreateKeyEx(hKey, L"shell\\open", 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
				{
					wcscpy(lpwData, L"&Open");
					RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpwData, (wcslen(lpwData) + 1) * sizeof(WCHAR));
					RegCloseKey(hSubKey);
				}
				if (RegCreateKeyEx(hKey, L"shell\\open\\command", 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
				{
					swprintf(lpwData, L"\"%s\" \"%%1\"", m_lpwAppPath);
					RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpwData, (wcslen(lpwData) + 1) * sizeof(WCHAR));
					RegCloseKey(hSubKey);
				}
				if (RegOpenKeyEx(hKey, L"shell", 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
				{
					wcscpy(lpwData, L"open");
					RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpwData, (wcslen(lpwData) + 1) * sizeof(WCHAR));
					RegCloseKey(hSubKey);
				}
			}
			for (i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
			{
				if (m_pCustomActions[i])
				{
					swprintf(lpwKeyName, L"shell\\%s", m_pCustomActions[i]->lpwRegCmdKey);
					if (RegCreateKeyEx(hKey, lpwKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
					{
						RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)m_pCustomActions[i]->lpwMenuName,
							(wcslen(m_pCustomActions[i]->lpwMenuName) + 1) * sizeof(WCHAR));
						RegCloseKey(hSubKey);
					}
					swprintf(lpwKeyName, L"shell\\%s\\command", m_pCustomActions[i]->lpwRegCmdKey);
					if (RegCreateKeyEx(hKey, lpwKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
					{
						RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)m_pCustomActions[i]->lpwCmdLine,
							(wcslen(m_pCustomActions[i]->lpwCmdLine) + 1) * sizeof(WCHAR));
						RegCloseKey(hSubKey);
					}
					if (m_pCustomActions[i]->bDefault)
					{
						if (RegOpenKeyEx(hKey, L"shell", 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
						{
							RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)m_pCustomActions[i]->lpwRegCmdKey,
								(wcslen(m_pCustomActions[i]->lpwRegCmdKey) + 1) * sizeof(WCHAR));
							RegCloseKey(hSubKey);
						}
					}
				}
			}
			if (RegCreateKeyEx(hKey, L"DefaultIcon", 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
			{
				swprintf(lpwData, L"%s,%i", m_lpwAppPath, m_dwIconIndex);
				RegSetValueEx(hSubKey, 0, 0, REG_SZ, (CONST LPBYTE)lpwData, (wcslen(lpwData) + 1) * sizeof(WCHAR));
				RegCloseKey(hSubKey);
			}
			RegCloseKey(hKey);
		}
	}
	else
	{
		RegCloseKey(hKey);
	}
    swprintf(lpwKeyName, L".%s", lpwExtTmp);
    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, lpwKeyName, 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, 0, &hKey, &dwDisposition) == ERROR_SUCCESS)
	{
		ZeroMemory(lpwData, sizeof(lpwData));
		dwSZSIZE = sizeof(lpwData);
		RegQueryValueEx(hKey, 0, 0, 0, (LPBYTE)lpwData, &dwSZSIZE);
		if (wcslen(lpwData))
		{
			RegSetValueEx(hKey, L"PreviousAssociation", 0, REG_SZ,
				(CONST LPBYTE)lpwData, (wcslen(lpwData) + 1) * sizeof(WCHAR));
		}
		RegSetValueEx(hKey, 0, 0, REG_SZ, (CONST LPBYTE)m_lpwRegAppKey, (wcslen(m_lpwRegAppKey) + 1) * sizeof(WCHAR));
		RegCloseKey(hKey);
	}
	delete[] lpwExtTmp;
}

//Функция удаляет связь расширения с программой
//Если bCompletely будет равен TRUE - произойдет полное удаление, включая и 'm_lpwRegAppKey'
void CFileAssociations::RemoveAssociation(LPCWSTR lpwExtension, BOOL bCompletely)
{
	if (!wcslen(m_lpwRegAppKey)) return;
	ULONG i;
	LPWSTR lpwExtTmp = new WCHAR[wcslen(lpwExtension) + 1];
    WCHAR lpwKeyName[64] = { 0 };
    WCHAR lpwData[MAX_PATH] = { 0 };
	HKEY hKey;
	DWORD dwSZSIZE;
	wcscpy(lpwExtTmp, lpwExtension);
	_wcslwr(lpwExtTmp);
    if (bCompletely)
	{
        swprintf(lpwKeyName, L"%s\\DefaultIcon", m_lpwRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpwKeyName);
        swprintf(lpwKeyName, L"%s\\shell\\open\\command", m_lpwRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpwKeyName);
        swprintf(lpwKeyName, L"%s\\shell\\open", m_lpwRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpwKeyName);
		for (i = 0; i < FA_MAX_CUSTOMACTIONS; i++)
		{
			if (m_pCustomActions[i])
			{
				swprintf(lpwKeyName, L"%s\\shell\\%s\\command", m_lpwRegAppKey, m_pCustomActions[i]->lpwRegCmdKey);
				RegDeleteKey(HKEY_CLASSES_ROOT, lpwKeyName);
				swprintf(lpwKeyName, L"%s\\shell\\%s", m_lpwRegAppKey, m_pCustomActions[i]->lpwRegCmdKey);
				RegDeleteKey(HKEY_CLASSES_ROOT, lpwKeyName);
			}
		}
        swprintf(lpwKeyName, L"%s\\shell", m_lpwRegAppKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, lpwKeyName);
        RegDeleteKey(HKEY_CLASSES_ROOT, m_lpwRegAppKey);
    }
	swprintf(lpwKeyName, L".%s", lpwExtTmp);
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpwKeyName, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		ZeroMemory(lpwData, sizeof(lpwData));
		dwSZSIZE = sizeof(lpwData);
		RegQueryValueEx(hKey, L"PreviousAssociation", 0, 0, (LPBYTE)lpwData, &dwSZSIZE);
		if (wcslen(lpwData))
		{
			RegSetValueEx(hKey, 0, 0, REG_SZ, (CONST LPBYTE)lpwData, (wcslen(lpwData) + 1) * sizeof(WCHAR));
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
	delete[] lpwExtTmp;
}

//Функция проверяет, связано ли указанное расширение с программой
BOOL CFileAssociations::IsAssociated(LPCWSTR lpwExtension)
{
	if (!wcslen(m_lpwRegAppKey)) return FALSE;
	LPWSTR lpwExtTmp = new WCHAR[wcslen(lpwExtension) + 1];
	WCHAR lpwKeyName[64] = { 0 };
	WCHAR lpwData[MAX_PATH] = { 0 };
	DWORD dwSZSIZE;
	HKEY hKey;
	BOOL bResult = FALSE;
	wcscpy(lpwExtTmp, lpwExtension);
	_wcslwr(lpwExtTmp);
	swprintf(lpwKeyName, L".%s", lpwExtTmp);
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpwKeyName, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		dwSZSIZE = sizeof(lpwData);
		RegQueryValueEx(hKey, 0, 0, 0, (LPBYTE)lpwData, &dwSZSIZE);
		if (_wcsicmp(lpwData, m_lpwRegAppKey) == 0) bResult = TRUE;
		RegCloseKey(hKey);
	}
	delete[] lpwExtTmp;
	return bResult;
}

//Функция обновляет оболочку Windows
//Эту функцию следует вызывать после AddAssociation()/RemoveAssociation()
void CFileAssociations::UpdateShell()
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSHNOWAIT, 0, 0);
}