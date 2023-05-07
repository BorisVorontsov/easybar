//������ �� ������� ������

#include <windows.h>

#include "filecollection.h"

CFileCollection::CFileCollection()
{
	ULONG i = 0;
	m_lpCurrentFile = NULL;
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
			delete[] m_pFileCollection[i]->lpPath;
			delete m_pFileCollection[i];
		}
	}
}

//������� ���������� ���������� ������ � ��������� (���� � �������)
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

//������� ��������� ���� � ���������
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::AppendFile(LPWSTR lpFileName, BOOL bSendNfn)
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i] == NULL)
		{
			m_pFileCollection[i] = new FCSTRUCT;
			m_pFileCollection[i]->lpPath = new WCHAR[MAX_PATH];
			wcscpy(m_pFileCollection[i]->lpPath, lpFileName);
			m_pFileCollection[i]->dwRndIndex = 0;
			m_pFileCollection[i]->lpUserData = NULL;
			if (m_hCBWnd && bSendNfn)
				SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEADDED), (LPARAM)lpFileName);
			SetCurrentFile(lpFileName);
			break;
		}
		else
		{
			if (i == (FC_MAX_FILES - 1)) return 0;
			if (_wcsicmp(m_pFileCollection[i]->lpPath, lpFileName) == 0) return 0;
		}
	}
	return 1;
}

int CFileCollection::AppendFile(LPWSTR lpFileName, LONG_PTR lpUD, BOOL bSendNfn)
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i] == NULL)
		{
			m_pFileCollection[i] = new FCSTRUCT;
			m_pFileCollection[i]->lpPath = new WCHAR[MAX_PATH];
			wcscpy(m_pFileCollection[i]->lpPath, lpFileName);
			m_pFileCollection[i]->dwRndIndex = 0;
			m_pFileCollection[i]->lpUserData = lpUD;
			if (m_hCBWnd && bSendNfn)
			{
				SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEADDED), (LPARAM)lpFileName);
				SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEUDCHANGED), (LPARAM)i);
			}
			SetCurrentFile(lpFileName);
			break;
		}
		else
		{
			if (i == (FC_MAX_FILES - 1)) return 0;
			if (_wcsicmp(m_pFileCollection[i]->lpPath, lpFileName) == 0) return 0;
		}
	}
	return 1;
}

//������� ���������� � lpFileName ������� ��� ��������������� ����������
//������� (���� � ����) ���� (��� �������������)
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::GetFile(LPWSTR lpFileName, DWORD dwIndex, DWORD dwFlag)
{
	switch (dwFlag)
	{
		case FCF_RECENT:
			if (m_lpCurrentFile == NULL) return 0;
			wcscpy(lpFileName, m_lpCurrentFile);
			break;
		case FCF_BYINDEX:
			if (dwIndex >= FC_MAX_FILES) return 0;
			if (m_pFileCollection[dwIndex] == NULL) return 0;
			wcscpy(lpFileName, m_pFileCollection[dwIndex]->lpPath);
			break;
		default:
			return 0;
	}
	return 1;
}

//������� ���������� ������ ���������� ��� �������� ����� (���� � ����)
//� ������ ������ ������� ���������� �������� ������ ����
int CFileCollection::GetFileIndex(LPCWSTR lpFileName, DWORD dwFlag)
{
	ULONG i = 0;
	if (m_pFileCollection[0] == NULL) return -1;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_RECENT)
			{
				if (m_lpCurrentFile == NULL) break;
				if (_wcsicmp(m_pFileCollection[i]->lpPath, m_lpCurrentFile) == 0)
				{
					return i;
				}
			}
			else if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpPath, lpFileName) == 0)
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

//������� �������������� � ���������� � lpFileName ���������, �� ������� (���� � ����) ���
//���������� ���� � ���������
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::NextFile(LPWSTR lpFileName, DWORD dwIndex, DWORD dwFlag)
{
	ULONG i;
	if (m_pFileCollection[0] == NULL) return 0;
	//���� ��������� ������ - ���������� ������...
	if (dwFlag == FCF_FIRST)
	{
		wcscpy(lpFileName, m_pFileCollection[0]->lpPath);
		goto ExitFunction;
	}
	//�������� ������� ���� �� �������
	if (dwFlag == FCF_BYINDEX)
	{
		if (dwIndex >= FC_MAX_FILES) return 0;
		if (m_pFileCollection[dwIndex])
		{
			wcscpy(lpFileName, m_pFileCollection[dwIndex]->lpPath);
		}
		else return 0;
		goto ExitFunction;
	}
	//�������� ������� ��������� ���� �� ���������
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
			wcscpy(lpFileName, m_pFileCollection[lIndex]->lpPath);
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
	if (m_lpCurrentFile == NULL) return 0;
	//���� ������� ���� � ���������...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (_wcsicmp(m_pFileCollection[i]->lpPath, m_lpCurrentFile) == 0)
			{
				if (dwFlag == FCF_FORWARD)
				{
					if (i == (FC_MAX_FILES - 1)) return 0;
					//���� ���������� ���, ���������� �������
					if (!m_pFileCollection[i + 1])
					{
						wcscpy(lpFileName, m_pFileCollection[i]->lpPath);
					}
					else
					{
						wcscpy(lpFileName, m_pFileCollection[i + 1]->lpPath);
					}
					break;
				}
				else if (dwFlag == FCF_BACKWARD)
				{
					if (i == 0) return 0;
					//���� ����������� ���, ���������� �������
					if (!m_pFileCollection[i - 1])
					{
						wcscpy(lpFileName, m_pFileCollection[i]->lpPath);
					}
					else
					{
						wcscpy(lpFileName, m_pFileCollection[i - 1]->lpPath);
					}
					break;
				}
				else if (dwFlag == FCF_RECENT)
				{
					wcscpy(lpFileName, m_pFileCollection[i]->lpPath);
					break;
				}
				else return 0;
			}
		}
		else return 0; //�����-�� ����� ����� �� �������
	}
ExitFunction:
	SetCurrentFile(lpFileName);
	return 1;
}

//������� ���������� � ��������� ��������� ��� �� ������� ���� �� ��������� ������� (������)
//����� ���������, ��� ��������� �� ����� ���� � ���������, �.�. ����������� ���� �����, ���
//_������_����������_�����_ ������
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::MoveFile(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwNewIndex, DWORD dwFlag)
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
					if (_wcsicmp(m_pFileCollection[i]->lpPath, lpFileName) == 0)
					{
						if (i == dwNewIndex) return 0;
						uStartIndex = i;
						goto MoveFile_Begin;
					}
				}
				else if (dwFlag == FCF_RECENT)
				{
					if (m_lpCurrentFile == NULL) return 0;
					if (_wcsicmp(m_pFileCollection[i]->lpPath, m_lpCurrentFile) == 0)
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
	//� ��� ���� ������...
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

//������� ������� �� ��������� ��������� ��� �� ������� ����
//�������, ��� ����� ������� ������� ������...
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::DeleteFile(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwFlag, BOOL bSendNfn)
{
	ULONG i, j;
	LPFCSTRUCT pFCST;
	if (m_pFileCollection[0] == NULL) return 0;
	//�������� � ��������� �����, ������� � ���������� �������
	//��������� ����������� ������� �������...
	if (dwFlag == FCF_BYINDEX)
	{
		if (m_pFileCollection[dwIndex] == NULL) return 0;
		WCHAR lpFile[MAX_PATH] = {};
		wcscpy(lpFile, m_pFileCollection[dwIndex]->lpPath);
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
						delete[] pFCST->lpPath;
						delete pFCST;
						break;
					}
				}
				else
				{
					m_pFileCollection[i] = NULL;
					delete[] pFCST->lpPath;
					delete pFCST;
				}
			}
			else return 0;
		}
		if (m_hCBWnd && bSendNfn)
			SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEDELETED), dwIndex);
		if (_wcsicmp(lpFile, m_lpCurrentFile) == 0)
		{
			if (dwIndex > 0)
			{
				SetCurrentFile((m_pFileCollection[dwIndex])?
					m_pFileCollection[dwIndex]->lpPath:m_pFileCollection[dwIndex - 1]->lpPath);
			}
			else
			{
				SetCurrentFile((m_pFileCollection[dwIndex])?m_pFileCollection[dwIndex]->lpPath:NULL);
			}
		}
		return 1;
	}
	//��� ��������� ������� ��������� ����� ����...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpPath, lpFileName) == 0)
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
								delete[] pFCST->lpPath;
								delete pFCST;
								break;
							}
						}
						else
						{
							m_pFileCollection[j] = NULL;
							delete[] pFCST->lpPath;
							delete pFCST;
						}
					}
					if (m_hCBWnd && bSendNfn)
						SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEDELETED), i);
					if (_wcsicmp(lpFileName, m_lpCurrentFile) == 0)
					{
						if (i > 0)
						{
							SetCurrentFile((m_pFileCollection[i])?
								m_pFileCollection[i]->lpPath:m_pFileCollection[i - 1]->lpPath);
						}
						else
						{
							SetCurrentFile((m_pFileCollection[i])?m_pFileCollection[i]->lpPath:NULL);
						}

					}
					return 1;
				}
			}
			else if (dwFlag == FCF_RECENT)
			{
				if (m_lpCurrentFile == NULL) return 0;
				if (_wcsicmp(m_pFileCollection[i]->lpPath, m_lpCurrentFile) == 0)
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
								delete[] pFCST->lpPath;
								delete pFCST;
								break;
							}
						}
						else
						{
							m_pFileCollection[j] = NULL;
							delete[] pFCST->lpPath;
							delete pFCST;
						}
					}
					if (m_hCBWnd && bSendNfn)
						SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEDELETED), i);
					if (i > 0)
					{
						SetCurrentFile((m_pFileCollection[i])?
							m_pFileCollection[i]->lpPath:m_pFileCollection[i - 1]->lpPath);
					}
					else
					{
						SetCurrentFile((m_pFileCollection[i])?m_pFileCollection[i]->lpPath:NULL);
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

//������� ������� ���������
void CFileCollection::Clear(BOOL bSendNfn)
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			delete[] m_pFileCollection[i]->lpPath;
			delete m_pFileCollection[i];
			m_pFileCollection[i] = NULL;
		}
	}
	if (m_hCBWnd && bSendNfn)
		SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_COLCLEARED), 0);
	SetCurrentFile(NULL);
}

//������� ���������, ���������� �� � ��������� ��������� (��� ����������) ����
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::IsFileAvailable(DWORD dwFlag)
{
	ULONG i = 0;
	if (m_pFileCollection[0] == NULL) return 0;
	if (m_lpCurrentFile == NULL) return 0;
	//�������� ����� ������� ���� � ���������
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (_wcsicmp(m_pFileCollection[i]->lpPath, m_lpCurrentFile) == 0)
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
		else return 0; //����� �� �������... �������.
	}
	return 1;
}

//������� ������ ��������� ����
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::SetRecentFile(LPCWSTR lpFileName, BOOL bSendNfn)
{
	if (_wcsicmp(m_lpCurrentFile, lpFileName) == 0) return 0;
	SetCurrentFile(lpFileName, bSendNfn);
	return 1;
}

//������� ������������ ���������������� ������ ����� (�� ������� ��� �� �����) � ���������
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::SetUserData(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwFlag, LONG_PTR lpUD, BOOL bSendNfn)
{
	ULONG i;
	if (m_pFileCollection[0] == NULL) return 0;
	//������� �� �������
	if (dwFlag == FCF_BYINDEX)
	{
		if (m_pFileCollection[dwIndex])
		{
			m_pFileCollection[dwIndex]->lpUserData = lpUD;
			if (m_hCBWnd && bSendNfn)
				SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEUDCHANGED), dwIndex);
	
		} else return 0;
		return 1;
	}
	//��� ��������� ������� ��������� ����� ����...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpPath, lpFileName) == 0)
				{
					
					m_pFileCollection[i]->lpUserData = lpUD;
					if (m_hCBWnd && bSendNfn)
						SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_FILEUDCHANGED), i);
					return 1;
				}
			}
			else if (dwFlag == FCF_RECENT)
			{
				if (m_lpCurrentFile == NULL) return 0;
				if (_wcsicmp(m_pFileCollection[i]->lpPath, m_lpCurrentFile) == 0)
				{
					m_pFileCollection[i]->lpUserData = lpUD;
					if (m_hCBWnd && bSendNfn)
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

//������� ���������� ���������������� ������, ������� ������������� � ��������� ������
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::GetUserData(LPCWSTR lpFileName, DWORD dwIndex, DWORD dwFlag, LONG_PTR& lpUD)
{
	ULONG i;
	if (m_pFileCollection[0] == NULL) return 0;
	//������� �� �������
	if (dwFlag == FCF_BYINDEX)
	{
		if (m_pFileCollection[dwIndex])
			lpUD = m_pFileCollection[dwIndex]->lpUserData;
		else return 0;
		return 1;
	}
	//��� ��������� ������� ��������� ����� ����...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_BYFILENAME)
			{
				if (_wcsicmp(m_pFileCollection[i]->lpPath, lpFileName) == 0)
				{
					lpUD = m_pFileCollection[i]->lpUserData;
					return 1;
				}
			}
			else if (dwFlag == FCF_RECENT)
			{
				if (m_lpCurrentFile == NULL) return 0;
				if (_wcsicmp(m_pFileCollection[i]->lpPath, m_lpCurrentFile) == 0)
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

//������� ������ ���� ��������� ������
//�.�. ����, ������� ����� �������� ����������� � ��������
void CFileCollection::SetCallbackWnd(HWND hWnd)
{
	if (IsWindow(hWnd))
		m_hCBWnd = hWnd;
}

//������� ���������� ���� ��������� ������
HWND CFileCollection::GetCallbackWnd()
{
	return m_hCBWnd;
}

//��������� ������� ��� ������������� �������� �����
void CFileCollection::SetCurrentFile(LPCWSTR lpPath, BOOL bSendNfn)
{
	if (m_lpCurrentFile)
		delete[] m_lpCurrentFile;
	if (lpPath)
	{
		m_lpCurrentFile = new WCHAR[MAX_PATH];
		wcscpy(m_lpCurrentFile, lpPath);
	}
	else m_lpCurrentFile = NULL;
	if (m_hCBWnd && bSendNfn)
		SendMessage(m_hCBWnd, WM_FCNOTIFICATION, MAKEWPARAM(0, FCN_CURFILECHANGED), (LPARAM)lpPath);
}
