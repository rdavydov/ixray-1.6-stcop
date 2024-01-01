//----------------------------------------------------
// file: FileSystem.cpp
//----------------------------------------------------

#include "stdafx.h"
#pragma hdrstop

#include "FileSystem.h"

#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
//#include "commdlg.h"

#include <Shlobj.h>
//#include "OSDialog.h"
//#pragma comment(lib, "OSDialogB.lib")

int CALLBACK BrowseCallbackProc( HWND hWnd, UINT uMsg, LPARAM lParam,
  LPARAM lpData )
{
  if (uMsg == BFFM_INITIALIZED)
    SendMessage(hWnd, BFFM_SETSELECTION,TRUE, lpData);
  return 0;
}
//----------------------------------------------------

void EFS_Utils::MarkFile(LPCSTR fn, bool bDeleteSource)
{
	xr_string ext = strext(fn);
	ext.insert		(1,"~");
	xr_string backup_fn = EFS.ChangeFileExt(fn,ext.c_str());
	if (bDeleteSource){
		FS.file_rename(fn,backup_fn.c_str(),true);
	}else{
		FS.file_copy(fn,backup_fn.c_str());
	}
}

xr_string	EFS_Utils::AppendFolderToName(xr_string& tex_name, int depth, BOOL full_name)
{
	string1024 nm;
	xr_strcpy(nm,tex_name.c_str());
	tex_name = AppendFolderToName(nm,sizeof(nm),depth,full_name);
	return tex_name;
}
