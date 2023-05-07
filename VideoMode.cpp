//Работа с видеорежимами экрана

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

//Возвращает массив структур VIDEOMODE, содержащий перечень видеорежимов,
//поддерживаемых текущим видео драйвером
//Для того, что бы узнать требуемую размерность массива, достаточно вызвать
//функцию с аргументом 0
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

//Возвращает текущий видеорежим
void CVideoMode::GetCurrentVideoMode(LPVIDEOMODE pVM) const
{
	DEVMODE DM = {};
	EnumDisplaySettings(0, -1, &DM);
	pVM->dwPelsWidth = DM.dmPelsWidth;
	pVM->dwPelsHeight = DM.dmPelsHeight;
	pVM->dwBitsPerPel = DM.dmBitsPerPel;
	pVM->dwDisplayFrequency = DM.dmDisplayFrequency;
}

//Возвращает индекс указанного видеорежима в массиве видеорежимов (если там таковой имеется)
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

//По умолчанию меняет разрешение экрана на указанное
//Если задан флаг CVMF_TEST - функция проверяет поддержку видео драйвером
//указанного видеорежима
//Если задан флаг CVMF_UPDATEREGISTRY - настройка экрана сохраняется в
//системном реестре (для текущего пользователя)
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

//Восстанавливает исходный видеорежим (если при переходе в другой режим 
//не был задан флаг CVMF_UPDATEREGISTRY)
//Оригинальные настройки берутся из системного реестра
void CVideoMode::RestoreOriginalVideoMode()
{
	ChangeDisplaySettings(0, 0);
}