#ifndef FILECOLLECTION_H
#define FILECOLLECTION_H

#ifndef _INC_TIME
#include <time.h>
#endif

//Максимальное количество файлов в программе
#define FC_MAX_FILES		1024

#define FCF_FIRST			0x00000010
#define FCF_FORWARD			0x00000020
#define FCF_BACKWARD		0x00000030
#define FCF_RANDOM			0x00000040
#define FCF_RECENT			0x00000050
#define FCF_BYINDEX			0x00000060
#define FCF_BYFILENAME		0x00000070

typedef struct _FCSTRUCT
{
	LPWSTR lpwPath;
	DWORD dwRndIndex;
	DWORD dwReserved;
} FCSTRUCT, *LPFCSTRUCT;

class CFileCollection
{
public:
	CFileCollection();
	~CFileCollection();
	int FileCount();
	int AppendFile(LPWSTR lpwFileName);
	int GetFile(LPWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag);
	int GetFileIndex(LPCWSTR lpwFileName, DWORD dwFlag);
	int NextFile(LPWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag);
	int DeleteFile(LPCWSTR lpwFileName, DWORD dwIndex, DWORD dwFlag);
	void Clear();
	int IsFileAvailable(DWORD dwFlag);
	int SetRecentFile(LPCWSTR lpwFileName);
protected:
	//
private:
	void SetCurrentFile(LPCWSTR lpwPath);
	LPFCSTRUCT m_pFileCollection[FC_MAX_FILES];
	LPWSTR m_lpwCurrentFile;
};

extern CFileCollection *pFileCollection;

__inline void Randomize(){
	srand((unsigned int)time(0));
}

__inline int Random(int val){
	return (val)?(rand()%(val)):0;
}

#endif