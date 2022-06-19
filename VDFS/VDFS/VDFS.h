#pragma once
#include "VDFSObject.h"
#include "InputFileStream\InputFileStream.h"
#include "FileCache\FileCache.h"
#include "Volume\Volume.h"
#include "Options\Options.h"

#undef GetSystemDirectory
#define FILE_FLAG_VIRTUAL 1
#define FILE_FLAG_PHYSICAL 2
#define FILE_FLAG_PHYSICALFIRST 4
#define HasFlag( value, flag ) (( value & flag ) == flag)
#define IsFlagVirtual( value ) HasFlag( value, FILE_FLAG_VIRTUAL )
#define IsFlagPhysical( value ) HasFlag( value, FILE_FLAG_PHYSICAL )
#define IsFlagPhysicalFirst( value ) ( HasFlag( value, FILE_FLAG_PHYSICAL ) && HasFlag( value, FILE_FLAG_PHYSICALFIRST ) )
#define IsFlagPhysicalLast( value ) ( HasFlag( value, FILE_FLAG_PHYSICAL ) && !HasFlag( value, FILE_FLAG_PHYSICALFIRST ) )

namespace VDFS {
  const string& GetGameDirectory();
  const string& GetSystemDirectory();
  const string& GetSystemDirectoryName();
  const string& GetDataDirectory();
  const string& GetTemplatesDirectory();
  bool IsFileExists( const string& fileName );


  template<typename T>
  T min2( const T& a, const T& b ) {
    return a < b ? a : b;
  }


  class VDFS {
    Array<Volume*> Volumes;
    FileTable* VirtualFileTable;
    FileTable* PhysicalFileTable;

    FileLayered* FindFileLayered( const string& fileName, const uint& flags );
    Array<FileLayered*> FindFilesLayered( const string& fileName, const uint& flags );

    VDFS();
    ~VDFS();
  public:
    Volume* LoadVolume( const string& valueName );
    Volume* FindVolume( const string& valueName );
    bool LoadPhysicalFile( const string& fileName, const string& directory, const string& root = GetGameDirectory() );
    void LoadPhysicalFiles( const string& directory, const string& root = GetGameDirectory() );
    void LoadPhysicalFiles( const string& root = GetGameDirectory() );
    void LoadVirtualFiles( const string& directory );
    void LoadVirtualFiles();
    FileLayered* CheckPhysicalFile( const string& fullName );
    FileLayered* FindFile( const string& fileName, const uint& flags );
    Array<FileLayered*> FindFiles( const string& fileName, const uint& flags );
    uint MakeVirtualFileList( char**& list ) const;
    uint MakePhysicalFileList( char**& list ) const;
    void Exit();

    static VDFS& GetInstance();
  };
}