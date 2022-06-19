#pragma once
#include "VolumeHeader.h"
#include "VolumeFile.h"
#include "File\File.h"
#include "File\Vorbis\Vorbis.h"
#include "FileTable\FileTable.h"


namespace VDFS {
  class Volume : public VDFSObject {
    static FileTable* GlobalFileTable;

    VolumeHeader Header;
    Directory* FileTree;
    FileCache* Cache;
    IFILE* FileHandle;
    string FullVolumeName;
    bool Zipped;

    Volume( const string& fullFileName );
    bool OpenVolume();
    bool ReadHeader();
    bool ReadFileList( Directory* directory, const long& index = 0 );
    bool ReadFileList();
    ~Volume();
  public:

    Directory* GetFileTree();
    const Directory* GetFileTree() const;
    const VolumeHeader& GetHeader() const;
    void CloseVolume();
    const string GetVolumeName() const;
    const string GetFullVolumeName() const;
    bool IsZipped();
    IFILE* GetFile();

    static Volume* OpenVolume( const string& fullFileName );
    static FileTable* SetGlobalFileTable( FileTable* table );
  };
}