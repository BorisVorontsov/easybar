#ifndef FILEASSOCIATIONS_H
#define FILEASSOCIATIONS_H

#define FA_MAX_CUSTOMACTIONS		0x00000020

typedef struct _FACUSTOMACTION
{
	WCHAR lpRegCmdKey[64];
	WCHAR lpMenuName[128];
	WCHAR lpCmdLine[MAX_PATH];
	BOOL bDefault;
	DWORD dwReserved;
} FACUSTOMACTION, *LPFACUSTOMACTION;

class CFileAssociations
{
public:
	CFileAssociations();
	~CFileAssociations();
	BOOL AddCustomAction(LPCWSTR lpRegCmdKey, LPCWSTR lpMenuName,
		                 LPCWSTR lpCmdLine, BOOL bDefault = FALSE);
	void ClearCustomActions();
	void AddAssociation(LPCWSTR lpExtension, BOOL bCreateStdAction = TRUE);
	void RemoveAssociation(LPCWSTR lpExtension, BOOL bCompletely = FALSE);
	BOOL IsAssociated(LPCWSTR lpExtension);
	void UpdateShell();
	WCHAR m_lpAppPath[MAX_PATH];
	DWORD m_dwIconIndex;
	WCHAR m_lpRegAppKey[64];
	WCHAR m_lpFileDesc[128];
protected:
	//
private:
	LPFACUSTOMACTION m_pCustomActions[FA_MAX_CUSTOMACTIONS];
};

extern CFileAssociations *pFileAssociations;

#endif