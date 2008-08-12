#ifndef FILEASSOCIATIONS_H
#define FILEASSOCIATIONS_H

#define FA_MAX_CUSTOMACTIONS		0x00000020

typedef struct _FACUSTOMACTION
{
	WCHAR lpwRegCmdKey[64];
	WCHAR lpwMenuName[128];
	WCHAR lpwCmdLine[MAX_PATH];
	BOOL bDefault;
	DWORD dwReserved;
} FACUSTOMACTION, *LPFACUSTOMACTION;

class CFileAssociations
{
public:
	CFileAssociations();
	~CFileAssociations();
	BOOL AddCustomAction(LPCWSTR lpwRegCmdKey, LPCWSTR lpwMenuName,
		                 LPCWSTR lpwCmdLine, BOOL bDefault = FALSE);
	void ClearCustomActions();
	void AddAssociation(LPCWSTR lpwExtension, BOOL bCreateStdAction = TRUE);
	void RemoveAssociation(LPCWSTR lpwExtension, BOOL bCompletely = FALSE);
	BOOL IsAssociated(LPCWSTR lpwExtension);
	void UpdateShell();
	WCHAR m_lpwAppPath[MAX_PATH];
	DWORD m_dwIconIndex;
	WCHAR m_lpwRegAppKey[64];
	WCHAR m_lpwFileDesc[128];
protected:
	//
private:
	LPFACUSTOMACTION m_pCustomActions[FA_MAX_CUSTOMACTIONS];
};

extern CFileAssociations *pFileAssociations;

#endif