#include <UnionAfx.h>
#include "VDFS\VDFS.h"


namespace VDFS {
  FileTable* Volume::GlobalFileTable = Null;


  Volume::Volume( const string& fullFileName ) : VDFSObject() {
    Cache = Null;
    FullVolumeName = fullFileName.GetFullPath();
    FileTree = Null;
    FileHandle = Null;
  }


  bool Volume::OpenVolume() {
    CloseVolume();
    FileHandle = IFILE::Open( FullVolumeName );
    if( FileHandle != Null ) {
      Cache = new FileCache( FileHandle );
      FileTree = new Directory();
      return true;
    }

    return false;
  }


  bool Volume::ReadHeader() {
    FileHandle->SetPosition( 0 );
    long readed = FileHandle->Read( &Header, sizeof( Header ) );
    Zipped = Header.Flags == 160;
    return readed == sizeof( VolumeHeader );
  }


  bool Volume::ReadFileList( Directory* baseDirectory, const long& index ) {
    const uint volumeHeaderSize = sizeof( VolumeHeader );
    const uint fileEntrySize = sizeof( FileEntry );
    FileEntry entry;

    long position = volumeHeaderSize + fileEntrySize * index;
    while( true ) {
      Cache->Seek( position );
      long readed = Cache->Read( &entry, fileEntrySize );
      if( !readed )
        break;

      position += fileEntrySize;
      if( entry.IsDirectory( Header ) ) {
        Directory* directory = new Directory( baseDirectory );
        directory->SetName( entry.GetName() );
        baseDirectory->Insert( directory );
        ReadFileList( directory, entry.Index );
        directory->Release();
      }
      else {
        File* file = File::Create(
          this,
          baseDirectory->GetFullName(),
          entry.GetName(),
          entry.Position,
          entry.Size
          );

        baseDirectory->Insert( file );
        file->Release();

        if( GlobalFileTable )
          GlobalFileTable->Insert( file );
      }

      if( entry.IsLastFile() )
        break;
    }

    return true;
  }


  bool Volume::ReadFileList() {
    Cache->Seek( sizeof( Header ) );
    ReadFileList( FileTree );
    return true;
  }


  Directory* Volume::GetFileTree() {
    return FileTree;
  }


  const Directory* Volume::GetFileTree() const {
    return FileTree;
  }


  const VolumeHeader& Volume::GetHeader() const {
    return Header;
  }


  Volume* Volume::OpenVolume( const string& fullFileName ) {
    Volume* volume = new Volume( fullFileName );
    if( !volume->OpenVolume() || !volume->ReadHeader() ) {
      delete volume;
      return Null;
    }

    volume->ReadFileList();

    string prefix = string(volume->Zipped ? "Zipped " : " ").PadLeft( 12 );
    string name = volume->GetVolumeName();
    cmd << CMD_YELLOW_INT << prefix
        << CMD_BLACK_INT  << L"● "
        << CMD_GREEN_INT  << name
        << CMD_RESET      << endl;

    return volume;
  }


  void Volume::CloseVolume() {
    if( FileHandle ) {
      FileHandle->Close();
      FileHandle = Null;
    }

    if( FileTree ) {
      FileTree->Release();
      FileTree = Null;
    }
  }


  const string Volume::GetVolumeName() const {
    return FullVolumeName.GetFileName();
  }


  const string Volume::GetFullVolumeName() const {
    return FullVolumeName;
  }


  bool Volume::IsZipped() {
    return Zipped;
  }


  IFILE* Volume::GetFile() {
    return FileHandle;
  }


  FileTable* Volume::SetGlobalFileTable( FileTable* fileTable ) {
    FileTable* oldHashList = GlobalFileTable;
    GlobalFileTable = fileTable;
    return oldHashList;
  }


  Volume::~Volume() {
    CloseVolume();
  }
}