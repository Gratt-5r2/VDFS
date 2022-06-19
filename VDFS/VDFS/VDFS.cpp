#include <UnionAfx.h>
#include "VDFS\VDFS.h"

extern int filesInMemory; // DELETE ME

namespace VDFS {
  VDFS::VDFS() {
    VirtualFileTable = new FileTable();
    PhysicalFileTable = new FileTable();
    ZippedBlockReaderCache::GetInstance()->SetMemoryLimit( 1024 * 1024 * DecompressedCacheSize );
  }


  Volume* VDFS::LoadVolume( const string& volumeName ) {
    Volume* volume = FindVolume( volumeName );
    if( volume )
      return volume;

    FileTable* oldTable = Volume::SetGlobalFileTable( VirtualFileTable );
    volume = Volume::OpenVolume( volumeName );
    Volume::SetGlobalFileTable( oldTable );
    if( !volume )
      return Null;

    Volumes.Insert( volume );
    return volume;
  }


  Volume* VDFS::FindVolume( const string& valueName ) {
    string shortName = valueName.GetFileName();
    for( uint i = 0; i < Volumes.GetNum(); i++ ) {
      Volume* volume = Volumes[i];
      if( volume->GetVolumeName().GetFileName() == shortName )
        return volume;
    }

    return Null;
  }


  bool VDFS::LoadPhysicalFile( const string& fileName, const string& directory, const string& root ) {
    string fullFileName = string::Combine( "%s%s\\%s", root, directory, fileName );
    if( !IsFileExists( fullFileName ) )
      return false;

    File* file = File::Create( root.GetUpper(), directory.GetUpper(), fileName.GetUpper() );
    PhysicalFileTable->Insert( file );
    file->Release();
    return true;
  }


  void VDFS::LoadPhysicalFiles( const string& directory, const string& root ) {
    Explorer explorer( string::Combine( "%s%s\\*", root, directory ) );
    while( explorer.FindNext() ) {
      if( explorer.IsFile() ) {
        LoadPhysicalFile( explorer.FileName(), directory, root );
      }
      else if( explorer.IsFolder() )
        LoadPhysicalFiles( string::Combine( "%s\\%t", directory, explorer.FileName() ), root );
    }
  }


  inline string GetSavesDirectory() {
    if( ModName[0] == 0 )
      return "SAVES";

    return string::Combine( "SAVES_%t", ModName );
  }


  inline string GetFakeSystemName() {
    return GetSystemDirectoryName().GetWord( "\\" ).Upper();
  }

  
  void VDFS::LoadPhysicalFiles( const string& root ) {
    LoadPhysicalFiles( "_WORK", root );
    LoadPhysicalFiles( GetSavesDirectory(), root);
    LoadPhysicalFiles( GetFakeSystemName(), root );
  }


  void VDFS::LoadVirtualFiles( const string& directory ) {
    Explorer vdfs( string::Combine( "%s\\*.VDF", directory ) );
    while( vdfs.FindNext() )
      if( vdfs.IsFile() )
        LoadVolume( string::Combine( "%s\\%t", directory, vdfs.FileName() ) );

    Explorer mods( string::Combine( "%s\\*.MOD", directory ) );
    while( mods.FindNext() )
      if( mods.IsFile() )
        LoadVolume( string::Combine( "%s\\%t", directory, mods.FileName() ) );
  }


  void VDFS::LoadVirtualFiles() {
    LoadVirtualFiles( GetGameDirectory() + "DATA" );
    LoadVirtualFiles( GetGameDirectory() + "DATA\\PLUGINS" );
    LoadVirtualFiles( GetGameDirectory() + "DATA\\PATCHES" );
  }


  FileLayered* VDFS::CheckPhysicalFile( const string& fullName ) {
    if( IsFileExists( fullName ) )
      if( LoadPhysicalFile( fullName.GetFileName(), fullName.GetDirectory() ) )
        return PhysicalFileTable->FindLayer( fullName );

    return Null;
  }


  FileLayered* VDFS::FindFileLayered( const string& fileName, const uint& flags ) {
    if( IsFlagPhysicalFirst( flags ) ) {
      FileLayered* fileLayered = PhysicalFileTable->FindLayer( fileName );
      if( fileLayered )
        return fileLayered;

      fileLayered = CheckPhysicalFile( fileName );
      if( fileLayered )
        return fileLayered;
    }

    if( IsFlagVirtual( flags ) ) {
      FileLayered* fileLayered = VirtualFileTable->FindLayer( fileName );
      if( fileLayered )
        return fileLayered;
    }

    if( IsFlagPhysicalLast( flags ) ) {
      FileLayered* fileLayered = PhysicalFileTable->FindLayer( fileName );
      if( fileLayered )
        return fileLayered;

      fileLayered = CheckPhysicalFile( fileName );
      if( fileLayered )
        return fileLayered;
    }

    return Null;
  }


  Array<FileLayered*> VDFS::FindFilesLayered( const string& fileName, const uint& flags ) {
    Array<FileLayered*> Files;

    if( IsFlagPhysicalFirst( flags ) )
      Files += PhysicalFileTable->FindLayers( fileName );

    if( IsFlagVirtual( flags ) )
      Files += VirtualFileTable->FindLayers( fileName );

    if( IsFlagPhysicalLast( flags ) )
      Files += PhysicalFileTable->FindLayers( fileName );

    return Files;
  }


  FileLayered* VDFS::FindFile( const string& fileName, const uint& flags ) {
    return FindFileLayered( fileName, flags );
  }


  Array<FileLayered*> VDFS::FindFiles( const string& fileName, const uint& flags ) {
    return FindFilesLayered( fileName, flags );
  }


  uint VDFS::MakeVirtualFileList( char**& list ) const {
    return VirtualFileTable->MakeFileList( list );
  }


  uint VDFS::MakePhysicalFileList( char**& list ) const {
    return PhysicalFileTable->MakeFileList( list );
  }


  void VDFS::Exit() {
    Volumes.ReleaseData();
    Volumes.Clear();
    delete VirtualFileTable;
    delete PhysicalFileTable;
    VirtualFileTable = Null;
    PhysicalFileTable = Null;
  }


  VDFS::~VDFS() {

  }


  VDFS& VDFS::GetInstance() {
    static VDFS virtualDiskFileSystem;
    return virtualDiskFileSystem;
  }


  inline string GetExecutableFile() {
    HMODULE module = GetModuleHandle( Null );
    char moduleName[2048];
    GetModuleFileName( module, moduleName, sizeof( moduleName ) - 1 );
    return moduleName;
  }


  const string& GetGameDirectory() {
    static string directory = GetSystemDirectory().GetDirectory();
    return directory;
  }
  
  
  const string& GetSystemDirectory() {
    static string directory = GetExecutableFile().GetDirectory();
    return directory;
  }


  const string& GetSystemDirectoryName() {
    static string directory = string( GetSystemDirectory() ).Cut( 0, GetGameDirectory().Length() );
    return directory; 
  }
  
  
  const string& GetDataDirectory() {
    static string directory = GetGameDirectory() + "DATA\\";
    return directory;
  }


  const string& GetTemplatesDirectory() {
    static string directory = GetDataDirectory() + "$TEMPLATES$\\";
    static int directoryCreated = CreateDirectory( directory, 0 );
    return directory;
  }


  bool IsFileExists( const string& fileName ) {
    if( fileName.Length() <= 2 || fileName[1] != ':' )
      return _access( GetGameDirectory() + fileName, 0 ) == 0;

    return _access( fileName, 0 ) == 0;
  }
}