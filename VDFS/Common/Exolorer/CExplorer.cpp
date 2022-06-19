#include <UnionAfx.h>

namespace Common
{
	Explorer::Explorer (CString fname) {
		Init (fname);
	}

	void Explorer::Init (CString fname) {
		sFilter = fname;
		ZeroMemory (&mExplorer, sizeof(mExplorer));
		hFile = INVALID_HANDLE_VALUE;
	}

	bool Explorer::FindBegin () {
		if (hFile == INVALID_HANDLE_VALUE) {
			hFile = FindFirstFile (sFilter, &mExplorer);
			return hFile != INVALID_HANDLE_VALUE;
		}
		return true;
	}

	bool Explorer::FindNext () {
		if (hFile == INVALID_HANDLE_VALUE) {
			hFile = FindFirstFile (sFilter, &mExplorer);
			return hFile != INVALID_HANDLE_VALUE;
		}
		return FindNextFile (hFile, &mExplorer) != 0;
	}

	void Explorer::FindEnd () {
		if (hFile != INVALID_HANDLE_VALUE) {
			FindClose (hFile);
			ZeroMemory (&mExplorer, sizeof(mExplorer));
			hFile = INVALID_HANDLE_VALUE;
		}
	}

	text Explorer::FileName () {
		return mExplorer.cFileName;
	}

	bool Explorer::IsFolder (bool intoOnly) {
		bool isFolder = (mExplorer.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if (isFolder && intoOnly)
			if (CStringA(FileName()).Compare(".") || 
				CStringA(FileName()).Compare(".."))
				return false;
		return isFolder;
	}

	bool Explorer::IsFile () {
		return !IsFolder(false);
	}

	Explorer::~Explorer () {
		FindEnd ();
	}





	CExploreInfo::CExploreInfo(WIN32_FIND_DATA& findData, const CString& root)
	{
		bIsDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		sName = CString( findData.cFileName ).GetWord("\\", -1);
		sRoot = root;
		bIsEnd = False;
	}

	CString	CExploreInfo::GetName() const
	{
		return sName;
	}

	CString	CExploreInfo::GetRoot() const
	{
		return sRoot;
	}

	CString	CExploreInfo::GetFullName() const
	{
		if( sRoot.IsEmpty() )
			return sName;
		return CString::Combine( "%s\\%s", sRoot, sName );
	}

	bool32 CExploreInfo::IsFile() const
	{
		return !bIsDirectory;
	}

	bool32 CExploreInfo::IsDirectory() const
	{
		return bIsDirectory;
	}





	void ExplorerMasked::FindFiles(const CString& root, const CString& format)
	{
		CString sRoot		= root.GetPattern("", "\\", -1);
		CString sRootInside	= root.GetWord("\\", -1);
		CString sName		= format.GetWord(".", 1);
		CString sExtension	= format.GetWord(".", 2);
		CExploreInfo* info	= Null;

		Explorer explorer( sRoot.IsEmpty() ? "*.*" : CString::Combine( "%s\\*.*", sRoot ));
		while (explorer.FindNext())
		{
			if( explorer.IsFolder() && sRootInside == "*" )
			{
				info = new CExploreInfo( explorer.mExplorer, sRoot );
				arrFinded.Insert( info );

				FindFiles( sRoot.IsEmpty() ? 
					CString::Combine( "%t\\*", explorer.FileName()) : 
					CString::Combine( "%s\\%t\\*", sRoot, explorer.FileName()), format );
			}
			else if( explorer.IsFile() )
			{
				CString sFindName = CString( explorer.FileName() ).GetWord(".", 1);
				CString sFindExt  = CString( explorer.FileName() ).GetWord(".", 2);

				bool32 bName = sName	   == "*" ? True : sName	   == sFindName;
				bool32 bExt	 = sExtension  == "*" ? True : sExtension  == sFindExt;
				if( bName && bExt )
				{
					CExploreInfo* info = new CExploreInfo( explorer.mExplorer, sRoot );
					arrFinded.Insert( info );
				}
			}
		}
		if( info )
			info->SetBreakFlag();
	}

	ExplorerMasked::ExplorerMasked(const CString& root, const CString& format)
	{
		sRoot = root;
		sFormat = format;
	}

	void ExplorerMasked::Begin()
	{
		FindFiles( sRoot, sFormat );
	}

	uint32 ExplorerMasked::GetNum()
	{
		return arrFinded.GetNum();
	}

	const CExploreInfo* ExplorerMasked::GetFileInfo(const uint32& index)
	{
		return arrFinded[index];
	}

	ExplorerMasked::~ExplorerMasked()
	{
		arrFinded.DeleteData();
	}

}