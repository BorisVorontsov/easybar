//������ � ������������� ������

#include <windows.h>
#include <stdio.h>

#include "videomode.h"

CVideoMode::CVideoMode()
{
	//
}

CVideoMode::~CVideoMode()
{
	//
}

//���������� ������ �������� VIDEOMODE, ���������� �������� ������������,
//�������������� ������� ����� ���������
//��� ����, ��� �� ������ ��������� ����������� �������, ���������� �������
//������� � ���������� 0
DWORD CVideoMode::EnumVideoModes(LPVIDEOMODE *pVMArr) const
{
	DEVMODE DM = {};
	ULONG i = 0;
	while(EnumDisplaySettings(0, i, &DM))
	{
		if (pVMArr)
		{
			pVMArr[i]->dwPelsWidth = DM.dmPelsWidth;
			pVMArr[i]->dwPelsHeight = DM.dmPelsHeight;
			pVMArr[i]->dwBitsPerPel = DM.dmBitsPerPel;
			pVMArr[i]->dwDisplayFrequency = DM.dmDisplayFrequency;
		}
		i++;
	}
	return i;
}

//���������� ������� ����������
void CVideoMode::GetCurrentVideoMode(LPVIDEOMODE pVM) const
{
	DEVMODE DM = {};
	EnumDisplaySettings(0, -1, &DM);
	pVM->dwPelsWidth = DM.dmPelsWidth;
	pVM->dwPelsHeight = DM.dmPelsHeight;
	pVM->dwBitsPerPel = DM.dmBitsPerPel;
	pVM->dwDisplayFrequency = DM.dmDisplayFrequency;
}

//���������� ������ ���������� ����������� � ������� ������������ (���� ��� ������� �������)
DWORD CVideoMode::GetVideoModeIndex(VIDEOMODE VM) const
{
	DEVMODE DM = {};
	ULONG i = 0;
	while(EnumDisplaySettings(0, i, &DM))
	{
		if ((DM.dmPelsWidth == VM.dwPelsWidth) && (DM.dmPelsHeight == VM.dwPelsHeight) &&
			(DM.dmBitsPerPel == VM.dwBitsPerPel) && (DM.dmDisplayFrequency == VM.dwDisplayFrequency))
		{
			return i;
		}
		i++;
	}
	return VM_ERROR;
}

//�� ��������� ������ ���������� ������ �� ���������
//���� ����� ���� CVMF_TEST - ������� ��������� ��������� ����� ���������
//���������� �����������
//���� ����� ���� CVMF_UPDATEREGISTRY - ��������� ������ ����������� �
//��������� ������� (��� �������� ������������)
BOOL CVideoMode::ChangeVideoMode(VIDEOMODE VM, DWORD dwFlag)
{
	DEVMODE DM = {};
	DM.dmSize = sizeof(DM);
	DM.dmPelsWidth = VM.dwPelsWidth;
	DM.dmPelsHeight = VM.dwPelsHeight;
	DM.dmBitsPerPel = VM.dwBitsPerPel;
	DM.dmDisplayFrequency = VM.dwDisplayFrequency;
	DM.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
	switch (dwFlag)
	{
		case CVMF_UPDATEREGISTRY:
			ChangeDisplaySettings(&DM, CDS_UPDATEREGISTRY);
			return TRUE;
		case CVMF_TEST:
			switch (ChangeDisplaySettings(&DM, CDS_TEST))
			{
				case DISP_CHANGE_SUCCESSFUL:
				case DISP_CHANGE_RESTART:
					return TRUE;
				default:
					break;
			}
			break;
		default:
			ChangeDisplaySettings(&DM, 0);
			return TRUE;
	}
	return FALSE;
}

//��������������� �������� ���������� (���� ��� �������� � ������ ����� 
//�� ��� ����� ���� CVMF_UPDATEREGISTRY)
//������������ ��������� ������� �� ���������� �������
void CVideoMode::RestoreOriginalVideoMode()
{
	ChangeDisplaySettings(0, 0);
}