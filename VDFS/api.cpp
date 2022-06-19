#include <UnionAfx.h>
#include "VDFS\VDFS.h"

extern void ProcessMonitor();


string NormalizeFileName( char*& name ) {
  if( name[0] == '\\' )
    name++;

  return string( name ).Upper();
}


uint GetProcessorsCount() {
  SYSTEM_INFO sysinfo;
  GetSystemInfo( &sysinfo );
  return sysinfo.dwNumberOfProcessors;
}


void NormalizeFlags( long& flags ) {
  if( flags == 0 )
    flags = FILE_FLAG_PHYSICAL | FILE_FLAG_VIRTUAL;
  else if( flags == FILE_FLAG_PHYSICALFIRST )
    flags = FILE_FLAG_PHYSICAL | FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICALFIRST;
}


string GetModName() {
  string commandLine = GetCommandLine();
  int gameId = commandLine.SearchI( "-game:", 0, True );
  if( gameId != Invalid ) {
    string optionName = commandLine.GetWordEx( "-", 1, gameId ).Shrink();
    return optionName.GetPattern( "", ".", -1 );
  }
  return "";
}


VDFS::FileLayered* GetErrorVisual( const char* fileNamePtr ) {
  if( VDFS::UseErrorVisuals ) {
    string extension = string( fileNamePtr ).GetExtension();
    VDFS::VDFS& vdfs = VDFS::VDFS::GetInstance();
    if( extension.Compare( "MRM" ) ) return vdfs.FindFile( "INVALID_SOURCE_FILE.MRM",   FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
    if( extension.Compare( "MDL" ) ) return vdfs.FindFile( "INVALID_SOURCE_FILE.MDL",   FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
    if( extension.Compare( "MDM" ) ) return vdfs.FindFile( "INVALID_SOURCE_FILE.MDM",   FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
    if( extension.Compare( "MMB" ) ) return vdfs.FindFile( "INVALID_SOURCE_FILE.MMB",   FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
    if( extension.Compare( "WAV" ) ) return vdfs.FindFile( "INVALID_SOURCE_FILE.WAV",   FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
    if( extension.Compare( "OGG" ) ) return vdfs.FindFile( "INVALID_SOURCE_FILE.WAV",   FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
    if( extension.Compare( "TEX" ) ) return vdfs.FindFile( "INVALID_SOURCE_FILE-C.TEX", FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
  }
  return Null;
}


VDFS::FileLayered* FindFileSafe( const string& fullName, long flags ) {
  VDFS::FileLayered* fileLayered = VDFS::VDFS::GetInstance().FindFile( fullName, flags );
  if( !fileLayered )
    fileLayered = GetErrorVisual( fullName );
  return fileLayered;
}


Array<VDFS::FileLayered*> FindFilesSafe( const string& fullName, long flags ) {
  Array<VDFS::FileLayered*> filesLayered = VDFS::VDFS::GetInstance().FindFiles( fullName, flags );
  if( filesLayered.GetNum() == 0 ) {
    VDFS::FileLayered* fileLayered = GetErrorVisual( fullName );
    if( fileLayered )
      filesLayered.Insert( fileLayered );
  }
  return filesLayered;
}


extern "C" {
  long vdf_fopen( char* _fullName, long flags ) {
    string fullName = NormalizeFileName( _fullName );
    NormalizeFlags( flags );
    VDFS::FileLayered* fileLayered = FindFileSafe( fullName, flags );
    if( !fileLayered )
      return Invalid;
    
    VDFS::File* file = fileLayered->GetTop();
    VDFS::FileStream* stream = file->Open();
    return (long)stream;
  }


  long vdf_fclose( long handle ) {
    if( handle == Invalid )
      return 0;

    VDFS::FileStream* stream = (VDFS::FileStream*)handle;
    stream->Close();
    return 0;
  }


  long vdf_fread( long handle, char* buffer, long size ) {
    if( handle == Invalid )
      return 0;
    
    VDFS::FileStream* stream = (VDFS::FileStream*)handle;
    long readed = stream->Read( buffer, size );
    return readed;
  }


  long vdf_fseek( long handle, long position ) {
    if( handle == Invalid )
      return 0;

    VDFS::FileStream* stream = (VDFS::FileStream*)handle;
    return stream->SetPosition( position );
  }


  long vdf_ftell( long handle ) {
    if( handle == Invalid )
      return 0;

    VDFS::FileStream* stream = (VDFS::FileStream*)handle;
    return stream->GetPosition();
  }


  long vdf_fexists( char* _fullName, long flags ) {
    string fullName = NormalizeFileName( _fullName );
    NormalizeFlags( flags );

    VDFS::FileLayered* fileLayered = FindFileSafe( fullName, flags );
    if( !fileLayered )
      return Failed;

    VDFS::File* file = fileLayered->GetTop();
    return file->IsVirtual() ? FILE_FLAG_VIRTUAL : FILE_FLAG_PHYSICAL;
  }


  long vdf_searchfile( char* _fileName, char* fileNamesList ) {
    string fileName = NormalizeFileName( _fileName );

    Array<VDFS::FileLayered*> filesLayered = FindFilesSafe( fileName, FILE_FLAG_VIRTUAL | FILE_FLAG_PHYSICAL );
    string filesListTmp;
    for( uint i = 0; i < filesLayered.GetNum(); i++ ) {
      if( i != 0 )
        filesListTmp += ( '|' );
      filesListTmp += filesLayered[i]->GetFullName();
    }
    
    memcpy( fileNamesList, filesListTmp, filesListTmp.Length() + 1 );
    return (long)filesLayered.GetNum();
  }


  long vdf_ffilesize( long handle ) {
    if( handle == Invalid )
      return 0;

    VDFS::FileStream* stream = (VDFS::FileStream*)handle;
    long size = (long)stream->GetSize();
    return size;
  }


  long vdf_initall( long, char*, long*, long* ) {
    static bool initialized = false;
    if( initialized )
      return 0;

    VDFS::DecompressedCacheSize  = VDFS::ReadOptionInt ( "DEBUG", "DecompressedCacheSize",  VDFS::DecompressedCacheSize );
    VDFS::UnzipThreadsCount      = VDFS::ReadOptionInt ( "DEBUG", "UnzipThreadsCount",      GetProcessorsCount() );
    VDFS::TerminateProcessHotkey = VDFS::ReadOptionBool( "DEBUG", "TerminateProcessHotkey", VDFS::TerminateProcessHotkey );
    VDFS::UseErrorVisuals        = VDFS::ReadOptionBool( "DEBUG", "UseErrorVisuals",        VDFS::UseErrorVisuals );

    string modName = GetModName();
    if( !modName.IsEmpty() ) {
      int modNameLength = min( modName.Length(), 63 );
      memcpy( VDFS::ModName, modName, modNameLength + 1 );
    }
    else
      ZeroMemory( VDFS::ModName, sizeof( VDFS::ModName ) );

    auto& vdfs = VDFS::VDFS::GetInstance();
    vdfs.LoadVirtualFiles();
    vdfs.LoadPhysicalFiles();
    initialized = true;
    
    if( VDFS::TerminateProcessHotkey )
      Thread( ProcessMonitor ).Detach();

    return 0;
  }


  long vdf_regvol( const char* volname ) {
    // PASS
    return None;
  }


  long vdf_import( const char* path ) {
    auto& vdfs = VDFS::VDFS::GetInstance();
    VDFS::Volume* volume = vdfs.LoadVolume( path );
    return volume ? True : False;
  }


  long vdf_exitall() {
    VDFS::VDFS::GetInstance().Exit();
    return None;
  }


  long vdf_filelist_physical( char**& list ) {
    return VDFS::VDFS::GetInstance().MakePhysicalFileList( list );
  }


  long vdf_filelist_virtual( char**& list ) {
    return VDFS::VDFS::GetInstance().MakeVirtualFileList( list );
  }


  long vdf_getvolumename( char* fullFileName, char*& volumeName ) {
    auto& vdfs = VDFS::VDFS::GetInstance();
    VDFS::FileLayered* fileLayered = vdfs.FindFile( fullFileName, FILE_FLAG_VIRTUAL );
    if( !fileLayered )
      return Invalid;

    VDFS::File* file = fileLayered->GetTop();
    const VDFS::Volume* volume = file->GetVolume();
    if( !volume )
      return Invalid;

    string name = volume->GetVolumeName();
    uint length = name.Length();
    volumeName = new char[length + 1]; // TODO операторы new delete
    memcpy( volumeName, name, length + 1 );
    return length - 1;
  }


  string GetLastErrorText() { // TODO
    return "";
  }

  long vdf_getlasterror( char* error ) { // TODO
    return 0;
  }



  long vdf_getfilehandle( long handle, long& offset ) {
    VDFS::FileStream* stream = (VDFS::FileStream*)handle;
    return (long)stream->GetHandle()->Stream;
  }




  long vdf_unpackogg( char* fullname, long flags ) { // TODO ??
    return False;
  }

  long vdf_packogg( char* fullname, long flags ) { // TODO ??
    return False;
  }

  long vdf_set_unzip_multithreading( long allow ) {
    return VDFS::UnzipMultithreading = allow ? true : false;
  }

  long vdf_updatephysicaltable() { // TODO
    return True;
  }



  long vdf_fdirexists( char* pathname, long flags ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }

  long vdf_changedir( char* pathname ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }

  long vdf_getdir( char* buffer ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }

  long vdf_findopen( char* path, long flags ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }

  long vdf_findnext( long findhandle, struct TVDFFINDDATA* finddata ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }

  long vdf_findclose( long findhandle ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }

  long vdf_setoption( long optid, long value ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }

  long vdf_getoption( long optid ) {
    Message::Warning( "G: Function not used!.", __FUNCTION__ );
    return -1;
  }
  
  int __stdcall DllMain( HMODULE hModule, DWORD fdwReason, LPVOID lpvReserved ) {
    if( fdwReason == DLL_PROCESS_ATTACH ) {
      char buffer[MAX_PATH];
      GetModuleFileName( GetModuleHandle( Null ), buffer, MAX_PATH );
      string moduleName = string( buffer ).GetWord( "\\", -1 );
      if( moduleName == "VDFS32G.EXE" || moduleName == "VDFS32E.EXE" )
        exit( 0 );
    }
    if( fdwReason == DLL_PROCESS_DETACH ) {

    }
    return True;
  }
}