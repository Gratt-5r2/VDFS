#pragma once

namespace Common {
	typedef class ASTAPI Explorer {
		friend class ExplorerMasked;
		CString sFilter;
		WIN32_FIND_DATA mExplorer;
		HANDLE hFile;
	public:
    Explorer( CString );
		void Init( CString );
		bool FindBegin();
		bool FindNext();
		void FindEnd();
		text FileName();
		bool IsFolder( bool intoOnly = true );
		bool IsFile();
		~Explorer();

	} WIN32FINDEXPLORER;

	class ASTAPI CExploreInfo {
		CString	sRoot;
		CString	sName;
		bool32	bIsDirectory;
		bool32	bIsEnd;
		uint32	ulFiles;
	public:
		CExploreInfo( WIN32_FIND_DATA& findData, const CString& root = NullStr );
		void	SetBreakFlag() { bIsEnd = True; };
		bool32	GetBreakFlag() const { return bIsEnd; };
		void	AddFile() { ulFiles++; };
		uint32	GetFilesCount() const { return ulFiles; };

		CString	GetName() const;
		CString	GetRoot() const;
		CString	GetFullName() const;
		bool32	IsFile() const;
		bool32	IsDirectory() const;
	};

	class ASTAPI ExplorerMasked {
		Array<CExploreInfo*> arrFinded;
		CString sRoot;
		CString sFormat;

		void FindFiles( const CString& root, const CString& format );
	public:

		ExplorerMasked( const CString& root, const CString& format );
		void Begin();
		uint32 GetNum();
		const CExploreInfo* GetFileInfo( const uint32& index );
		~ExplorerMasked();
	};
}