//----------------------------------------------------
// file: Library.h
//----------------------------------------------------

#ifndef LibraryH
#define LibraryH

#include "../../xrEngine/pure.h"     
//----------------------------------------------------
class CEditableObject;

using EditObjMap = xr_map<AnsiString, CEditableObject*, astr_pred>;
using EditObjPairIt = EditObjMap::iterator;
//----------------------------------------------------
class ECORE_API ELibrary//:	public pureDeviceCreate, public pureDeviceDestroy
{
	bool				m_bReady;
	friend class TfrmChoseObject;
	EditObjMap			m_EditObjects;

    CEditableObject*	LoadEditObject		(LPCSTR full_name);
    void				UnloadEditObject	(LPCSTR full_name);
public:
						ELibrary			();
	virtual 			~ELibrary			();
                    
    void  		RemoveObject		(LPCSTR fname, EItemType type, bool& res);
    void  		RenameObject		(LPCSTR fn0, LPCSTR fn1, EItemType type);

	void 				OnCreate			();
	void 				OnDestroy			();
	void 				Save				(FS_FileSet* modif_map=0);

    void 				ReloadObjects		();
    void 				CleanLibrary		();
    void 				ReloadObject		(LPCSTR name);

    CEditableObject*	CreateEditObject	(LPCSTR name);
    void				RemoveEditObject	(CEditableObject*& object);

    int					GetObjects			(FS_FileSet& files);
    int					ObjectCount			(){return m_EditObjects.size();}

    void				EvictObjects		();

	virtual		void	OnDeviceCreate		();
	virtual		void	OnDeviceDestroy		();
};

extern ECORE_API ELibrary Lib;
//----------------------------------------------------
#endif /*_INCDEF_Library_H_*/

