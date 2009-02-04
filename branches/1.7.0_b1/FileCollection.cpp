//������ �� ������� ������

#include <windows.h>

#include "filecollection.h"

CFileCollection::CFileCollection()
{
	ULONG i = 0;
	m_lpwCurrentFile = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		m_pFileCollection[i] = 0;
	}
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

//������� ���������� ���������� ������ � ��������� (���� � �������)
int CFileCollection::FileCount()
{
	ULONG i = 0;
	if (m_pFileCollection[0] == 0) return 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i] == 0)
			return i;
	}
	return FC_MAX_FILES;
}

//������� ��������� ���� � ���������
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::AppendFile(LPWSTR lpwFileName)
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i] == 0)
		{
			m_pFileCollection[i] = new FCSTRUCT;
			m_pFileCollection[i]->lpwPath = new WCHAR[MAX_PATH];
			wcscpy(m_pFileCollection[i]->lpwPath, lpwFileName);
			m_pFileCollection[i]->dwRndIndex = 0;
			m_pFileCollection[i]->dwReserved = 0;
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

//������� ���������� � lpwFileName ������� ��� ��������������� ����������
//������� (���� � ����) ���� (��� �������������)
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::GetFile(LPWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag)
{
	switch (dwFlag)
	{
		case FCF_RECENT:
			if (m_lpwCurrentFile == 0) return 0;
			wcscpy(lpwFileName, m_lpwCurrentFile);
			break;
		case FCF_BYINDEX:
			if (dwIndex >= FC_MAX_FILES) return 0;
			if (m_pFileCollection[dwIndex] == 0) return 0;
			wcscpy(lpwFileName, m_pFileCollection[dwIndex]->lpwPath);
			break;
		default:
			return 0;
	}
	return 1;
}

//������� ���������� ������ ���������� ��� �������� ����� (���� � ����)
//� ������ ������ ������� ���������� �������� ������ ����
int CFileCollection::GetFileIndex(LPCWSTR lpwFileName, DWORD dwFlag)
{
	ULONG i = 0;
	if (m_pFileCollection[0] == 0) return -1;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (dwFlag == FCF_RECENT)
			{
				if (m_lpwCurrentFile == 0) break;
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

//������� �������������� � ���������� � lpwFileName ���������, �� ������� (���� � ����) ���
//���������� ���� � ���������
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::NextFile(LPWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag)
{
	ULONG i;
	if (m_pFileCollection[0] == 0) return 0;
	//���� ��������� ������ - ���������� ������...
	if (dwFlag == FCF_FIRST)
	{
		wcscpy(lpwFileName, m_pFileCollection[0]->lpwPath);
		goto ExitFunction;
	}
	//�������� ������� ���� �� �������
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
	//�������� ������� ��������� ���� �� ���������
	if (dwFlag == FCF_RANDOM)
	{
		ULONG lFileCnt = 0, lIndex = 0;
		for (i = 0; i < FC_MAX_FILES; i++)
		{
			if (m_pFileCollection[i] == 0)
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
	if (m_lpwCurrentFile == 0) return 0;
	//���� ������� ���� � ���������...
	for (i = 0; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
			{
				if (dwFlag == FCF_FORWARD)
				{
					if (i == (FC_MAX_FILES - 1)) return 0;
					//���� ���������� ���, ���������� �������
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
					//���� ����������� ���, ���������� �������
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
		else return 0; //�����-�� ����� ����� �� �������
	}
ExitFunction:
	SetCurrentFile(lpwFileName);
	return 1;
}

//������� ������� �� ��������� ��������� ��� �� ������� ����
//�������, ��� ����� ������� ������� ������...
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::DeleteFile(LPCWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag)
{
	ULONG i, j;
	if (m_pFileCollection[0] == 0) return 0;
	//�������� � ��������� �����, ������� � ���������� �������
	//��������� ����������� ������� �������...
	if (dwFlag == FCF_BYINDEX)
	{
		WCHAR lpwFile[MAX_PATH] = { 0 };
		for (i = dwIndex; i < FC_MAX_FILES; i++)
		{
			if (m_pFileCollection[i])
			{
				wcscpy(lpwFile, m_pFileCollection[i]->lpwPath);
				if (i < (FC_MAX_FILES - 1))
				{
					if (m_pFileCollection[i + 1])
					{
						wcscpy(m_pFileCollection[i]->lpwPath, m_pFileCollection[i + 1]->lpwPath);
					}
					else
					{
						delete[] m_pFileCollection[i]->lpwPath;
						m_pFileCollection[i] = 0;
						break;
					}
				}
				else
				{
					delete[] m_pFileCollection[i]->lpwPath;
					delete m_pFileCollection[i];
					m_pFileCollection[i] = 0;
				}
			}
			else return 0;
		}
		if (_wcsicmp(lpwFile, m_lpwCurrentFile) == 0)
		{
			if (i > 0)
			{
				SetCurrentFile((m_pFileCollection[i])?
					m_pFileCollection[i]->lpwPath:m_pFileCollection[i - 1]->lpwPath);
			}
			else
			{
				SetCurrentFile((m_pFileCollection[i])?m_pFileCollection[i]->lpwPath:0);
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
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, lpwFileName) == 0)
				{
					for (j = i; j < FC_MAX_FILES; j++)
					{
						if (j < (FC_MAX_FILES - 1))
						{
							if (m_pFileCollection[j + 1])
							{
								wcscpy(m_pFileCollection[j]->lpwPath, m_pFileCollection[j + 1]->lpwPath);
							}
							else
							{
								delete[] m_pFileCollection[j]->lpwPath;
								delete m_pFileCollection[j];
								m_pFileCollection[j] = 0;
								break;
							}
						}
						else
						{
							delete[] m_pFileCollection[j]->lpwPath;
							delete m_pFileCollection[j];
							m_pFileCollection[j] = 0;
						}
					}
					if (_wcsicmp(lpwFileName, m_lpwCurrentFile) == 0)
					{
						if (i > 0)
						{
							SetCurrentFile((m_pFileCollection[i])?
								m_pFileCollection[i]->lpwPath:m_pFileCollection[i - 1]->lpwPath);
						}
						else
						{
							SetCurrentFile((m_pFileCollection[i])?m_pFileCollection[i]->lpwPath:0);
						}

					}
					return 1;
				}
			}
			else if (dwFlag == FCF_RECENT)
			{
				if (m_lpwCurrentFile == 0) return 0;
				if (_wcsicmp(m_pFileCollection[i]->lpwPath, m_lpwCurrentFile) == 0)
				{
					for (j = i; j < FC_MAX_FILES; j++)
					{
						if (j < (FC_MAX_FILES - 1))
						{
							if (m_pFileCollection[j + 1])
							{
								wcscpy(m_pFileCollection[j]->lpwPath, m_pFileCollection[j + 1]->lpwPath);
							}
							else
							{
								delete[] m_pFileCollection[j]->lpwPath;
								delete m_pFileCollection[j];
								m_pFileCollection[j] = 0;
								break;
							}
						}
						else
						{
							delete[] m_pFileCollection[j]->lpwPath;
							delete m_pFileCollection[j];
							m_pFileCollection[j] = 0;
						}
					}
					if (i > 0)
					{
						SetCurrentFile((m_pFileCollection[i])?
							m_pFileCollection[i]->lpwPath:m_pFileCollection[i - 1]->lpwPath);
					}
					else
					{
						SetCurrentFile((m_pFileCollection[i])?m_pFileCollection[i]->lpwPath:0);
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
void CFileCollection::Clear()
{
	ULONG i = 0;
	for (; i < FC_MAX_FILES; i++)
	{
		if (m_pFileCollection[i])
		{
			delete[] m_pFileCollection[i]->lpwPath;
			delete m_pFileCollection[i];
			m_pFileCollection[i] = 0;
		}
	}
	SetCurrentFile(0);
}

//������� ���������, ���������� �� � ��������� ��������� (��� ����������) ����
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::IsFileAvailable(DWORD dwFlag)
{
	if (m_pFileCollection[0] == 0) return 0;
	if (m_lpwCurrentFile == 0) return 0;
	ULONG i = 0;
	//�������� ����� ������� ���� � ���������
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
		else return 0; //����� �� �������... �������.
	}
	return 1;
}

//������� ������ ��������� ����
//� ������ ������ ������� ���������� �������� ������ ����, � ������ ������ - ����
int CFileCollection::SetRecentFile(LPCWSTR lpwFileName)
{
	if (_wcsicmp(m_lpwCurrentFile, lpwFileName) == 0) return 0;
	SetCurrentFile(lpwFileName);
	return 1;
}

//��������� ������� ��� ������������� �������� �����
void CFileCollection::SetCurrentFile(LPCWSTR lpwPath)
{
	if (m_lpwCurrentFile)
		delete[] m_lpwCurrentFile;
	if (lpwPath)
	{
		m_lpwCurrentFile = new WCHAR[MAX_PATH];
		wcscpy(m_lpwCurrentFile, lpwPath);
	}
	else m_lpwCurrentFile = 0;
}
