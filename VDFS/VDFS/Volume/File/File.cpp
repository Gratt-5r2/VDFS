#include <UnionAfx.h>
#include "VDFS\VDFS.h"
#pragma comment(lib, "version.lib")


int filesInMemory = 0; // DELETE ME


namespace VDFS {
  static FileVersion GetDllVersion( const char* fullFileName ) {
    FileVersion version;
    version.Binary = 0;

    ulong versionHandle = 0;
    ulong versionSize = GetFileVersionInfoSize( fullFileName, &versionHandle );
    byte* data = Null;
    uint dataSize = 0;

    if( versionSize != 0 ) {
      char* versionData = new char[versionSize];
      if( GetFileVersionInfo( fullFileName, versionHandle, versionSize, versionData ) ) {
        if( VerQueryValue( versionData, "\\", (VOID FAR* FAR*)&data, &dataSize ) ) {
          if( dataSize ) {
            VS_FIXEDFILEINFO* versionInfo = (VS_FIXEDFILEINFO*)data;
            if( versionInfo->dwSignature == 0xfeef04bd ) {
              version.Major = (uint16)(versionInfo->dwFileVersionMS >> 16);
              version.Minor = (uint16)(versionInfo->dwFileVersionMS >> 0);
              version.Micro = (uint16)(versionInfo->dwFileVersionLS >> 16);
              version.Build = (uint16)(versionInfo->dwFileVersionLS >> 0);
            }
          }
        }
      }
      delete[] versionData;
    }

    return version;
  }


  File::File() : VDFSObject() {
    ContainingVolume = Null;
    BaseFile = Null;
    BasePosition = 0;
    Size = 0;
    Timestamp = 0;
    Version.Binary = 0;
    filesInMemory++;
    WaveFile = false;
  }


  void File::SetBaseFile( IFILE* baseFile ) {
    if( BaseFile )
      BaseFile->Close();

    BaseFile = baseFile;
    BaseFile->Open();
  }


  void File::UpdateVersion() {
    if( !Name.EndWith( ".DLL" ) )
      return;

    FileStream* stream = Open();
    if( !stream )
      return;

    byte* data = new byte[Size];
    stream->Read( data, Size );
    stream->Close();

    string fullFileName = GetTemplatesDirectory() + string::Hex32( this );
    FILE* dll = fopen( fullFileName, "wb+" );
    if( !dll ) {
      delete[] data;
      return;
    }
    
    fwrite( data, 1, Size, dll );
    fclose( dll );
    delete[] data;

    Version = GetDllVersion( fullFileName );
    DeleteFile( fullFileName );
  }


  void File::UpdateSize() {
    FILE* file = fopen( GetRealName(), "rb" );
    if( !file ) {
      Size = 0;
      return;
    }

    fseek( file, 0, SEEK_END );
    Size = ftell( file );
    fclose( file );
  }


  void File::CheckAutorun() {
    string directoryName = SubDirectory.GetFileName();
    if( directoryName.Compare( "AUTORUN" ) || Name.EndWith( ".PATCH" ) ) {
      if( ContainingVolume && ContainingVolume->GetVolumeName() == "UNION.VDF" )
        if( Name.Compare( "ZPARSEREXTENDER.DLL" ) )
          return;

      LoadAutorunFile( GetFullName() );
    }
  }


  void File::CheckWaveFile() {
    WaveFile = Name.EndWith( ".WAV" ) || Name.EndWith( ".OGG" );
  }


  void File::CheckFakeSystem() {
    static string systemDirectoryName = GetSystemDirectoryName().GetUpper();
    static bool systemIsFake = systemDirectoryName != "SYSTEM\\";
    if( !systemIsFake )
      return;

    if( FullName.StartWith( systemDirectoryName ) )
      FullName = string::Combine( "SYSTEM\\%t", FullName.GetVector() + systemDirectoryName.Length());
  }


  FileStream* File::CreateFileStream( File* file, IFILE* handle ) {
    if( file->IsWaveFile() )
      return new FileStreamOgg( file, handle );

    return file->IsZipped() ?
      new FileStreamZipped( file, handle ) :
      new FileStream( file, handle );
  }


  FileStream* File::Open() {
    FileStream* stream = Null;

    if( !BaseFile ) {
      UpdateSize();
      IFILE* file = IFILE::Open( GetRealName() );
      stream = CreateFileStream( this, file );
      file->Release();
    }
    else
      stream = CreateFileStream( this, BaseFile );

    return stream;
  }


  void File::Close() {
    if( BaseFile )
      if( BaseFile->Close() == 0 )
        BaseFile = Null;
  }


  const string& File::GetName() const {
    return Name;
  }


  const string& File::GetFullName() const {
    return FullName;
  }


  const string& File::GetRealName() const {
    return RealName;
  }


  const FileVersion& File::GetVersion() const {
    return Version;
  }


  const uint& File::GetTimestamp() const {
    return Timestamp;
  }


  const Volume* File::GetVolume() const {
    return ContainingVolume;
  }


  const uint& File::GetBasePosition() const {
    return BasePosition;
  }


  bool File::IsVirtual() const {
    return BaseFile ? true : false;
  }


  bool File::IsZipped() const {
    if( ContainingVolume )
      return ContainingVolume->IsZipped();

    return false;
  }


  bool File::IsWaveFile() const {
    return WaveFile;
  }


  File* File::Create(
    Volume* volume,
    const string& subDirectory,
    const string& name,
    const uint& basePosition,
    const uint& size
    ) {
    File* file  = new File();
    file->Name         = name;
    file->SubDirectory = subDirectory.Completion( '\\' );
    file->FullName     = string::Combine( "%s%s", file->SubDirectory, name );
    file->RealName     = file->FullName;
    file->BasePosition = basePosition;
    file->Size         = size;
    file->SetBaseFile( volume->GetFile() );
    file->Timestamp = volume->GetHeader().Timestamp;
    file->ContainingVolume = volume;
    file->CheckFakeSystem();
    file->UpdateVersion();
    file->CheckAutorun();
    file->CheckWaveFile();
    return file;
  }


  File* File::Create(
    const string& baseDirectory,
    const string& subDirectory,
    const string& name
    ) {
    File* file = new File();
    file->Name          = name.GetUpper();
    file->BaseDirectory = baseDirectory.GetUpper().Completion( '\\' );
    file->SubDirectory  = subDirectory .GetUpper().Completion( '\\' );
    file->FullName      = string::Combine( "%s%s", file->SubDirectory, name );
    file->RealName      = string::Combine( "%s%s", file->BaseDirectory, file->FullName );
    file->CheckFakeSystem();
    file->CheckAutorun();
    file->CheckWaveFile();
    return file;
  }


  File::~File() {
    if( BaseFile )
      BaseFile->Close();
    filesInMemory--; // DELETE ME
  }








  Directory::Directory( Directory* parent ) : VDFSObject() {
    Parent = parent;
  }


  const Array<Directory*>& Directory::GetDirectories() const {
    return Directories;
  }


  const Array<File*>& Directory::GetFiles() const {
    return Files;
  }


  void Directory::Insert( Directory* directory ) {
    if( Directories.HasEqual( directory ) )
      return;

    Directories.Insert( directory );
    directory->Lock();
  }


  void Directory::Insert( File* file ) {
    if( Files.HasEqual( file ) )
      return;

    Files.Insert( file );
    file->Lock();
  }


  void Directory::Remove( Directory* directory ) {
    uint index = Directories.SearchEqual( directory );
    if( index == Invalid )
      return;

    Directories.RemoveAt( index );
    directory->Release();
  }


  void Directory::Remove( File* file ) {
    uint index = Files.SearchEqual( file );
    if( index == Invalid )
      return;

    Files.RemoveAt( index );
    file->Release();
  }


  void Directory::SetName( const string& name ) {
    Name = name;
    Name.Upper();
  }


  const string Directory::GetName() const {
    return Name;
  }


  const string Directory::GetFullName() const {
    string fullName;
    if( Parent ) {
      fullName += Parent->GetFullName();
      if( !fullName.IsEmpty() )
        fullName += "\\";
    }

    return fullName + Name;
  }


  Directory* Directory::GetDirectory( const string& path ) {
    uint index = path.Search( "\\" );
    string directoryName = index != Invalid ?
      string( path, index ) :
      string( path );

    for( uint i = 0; i < Directories.GetNum(); i++ ) {
      if( Directories[i]->Name == directoryName ) {
        Directory* directory = Directories[i];
        return index != Invalid ?
          directory->GetDirectory( path.GetVector() + index + 1 ) :
          directory;
      }
    }

    return Null;
  }


  File* Directory::GetFile( const string& path ) {
    uint index = path.Search( "\\" );
    string entryName = index != Invalid ?
      string( path, index ) :
      string( path );

    if( index != Invalid ) {
      for( uint i = 0; i < Directories.GetNum(); i++ ) {
        Directory* directory = Directories[i];
        if( directory->Name == entryName )
          return directory->GetFile( path.GetVector() + index + 1 );
      }
    }
    else for( uint i = 0; i < Files.GetNum(); i++ )
      if( Files[i]->GetName() == entryName )
        return Files[i];

    return Null;
  }


  Array<Directory*> Directory::FindDirectories( const string& namePtr ) {
    Array<Directory*> found;
    for( uint i = 0; i < Directories.GetNum(); i++ ) {
      Directory* directory = Directories[i];
      if( directory->GetName().Compare( namePtr ) )
        found.Insert( directory );

      found += directory->FindDirectories( namePtr );
    }

    return found;
  }


  Array<File*> Directory::FindFiles( const string& namePtr ) {
    Array<File*> found;
    for( uint i = 0; i < Files.GetNum(); i++ ) {
      File* file = Files[i];
      if( file->GetName().Compare( namePtr ) )
        found.Insert( file );
    }

    for( uint i = 0; i < Directories.GetNum(); i++ )
      found += Directories[i]->FindFiles( namePtr );

    return found;
  }


  const Directory* Directory::GetDirectory( const string& path ) const {
    return const_cast<Directory*>( this )->GetDirectory( path );
  }


  const File* Directory::GetFile( const string& path ) const {
    return const_cast<Directory*>(this)->GetFile( path );
  }


  Array<const Directory*> Directory::FindDirectories( const string& namePtr ) const {
    Array<Directory*> found = const_cast<Directory*>(this)->FindDirectories( namePtr );
    return reinterpret_cast<Array<const Directory*>&>( found );
  }


  Array<const File*> Directory::FindFiles( const string& namePtr ) const {
    Array<File*> found = const_cast<Directory*>(this)->FindFiles( namePtr );
    return reinterpret_cast<Array<const File*>&>(found);
  }


  Directory::~Directory() {
    Directories.ReleaseData();
    Directories.Clear();
    Files.ReleaseData();
    Files.Clear();
  }




  FileStream::FileStream( File* file, IFILE* handle ) : VDFSObject() {
    BaseFile = file;
    BaseFile->Lock();
    Cache = new FileCache( handle );
    Cache->SetRange( BaseFile->BasePosition, BaseFile->Size );
  }


  const string& FileStream::GetName() const {
    return BaseFile->GetName();
  }


  const string& FileStream::GetFullName() const {
    return BaseFile->GetFullName();
  }


  long FileStream::Read( void* data, const uint& length ) {
    return Cache->Read( data, length );
  }


  int FileStream::SetPosition( const long& position, const uint& origin ) {
    long correctedPosition = 0;
    switch( origin ) {
      case SEEK_SET: correctedPosition = position; break;
      case SEEK_CUR: correctedPosition = GetPosition() + position; break;
      case SEEK_END: correctedPosition = GetSize() - position; break;
    }

    return Cache->Seek( BaseFile->BasePosition + correctedPosition );
  }


  long FileStream::GetPosition() const {
    return Cache->Tell() - BaseFile->BasePosition;
  }


  long FileStream::GetSize() const {
    return BaseFile->Size;
  }


  void FileStream::Close() {
    if( Cache ) {
      delete Cache;
      Cache = Null;
    }
    if( BaseFile ) {
      BaseFile->Release();
      BaseFile = Null;
    }
  }

  IFILE* FileStream::GetHandle() {
    return Cache->File;
  }


  FileStream::~FileStream() {
    Close();
  }




  FileStreamZipped::FileStreamZipped( File* file, IFILE* handle ) : FileStream( file, handle ) {
    UpdateMultithreading();
    ZippedReader = new ZippedStreamReader( handle->Stream, file->GetBasePosition() );
  }


  void FileStreamZipped::UpdateMultithreading() {
    if( UnzipMultithreading && !GetName().EndWith( ".ZEN" ) )
      SetThreadsCount( UnzipThreadsCount );
    else
      SetThreadsCount( 1 );
  }


  long FileStreamZipped::Read( void* data, const uint& length ) {
    return ZippedReader->Read( (byte*)data, length );
  }


  int FileStreamZipped::SetPosition( const long& position, const uint& origin ) {
    return ZippedReader->Seek( position, origin );
  }


  long FileStreamZipped::GetPosition() const {
    return ZippedReader->Tell();
  }


  void FileStreamZipped::Close() {
    if( ZippedReader ) {
      ZippedReader->Close( false );
      ZippedReader = Null;
    }
    if( BaseFile ) {
      BaseFile->Release();
      BaseFile = Null;
    }
  }


  FileStreamZipped::~FileStreamZipped() {
    Close();
  }


  void FileStreamZipped::SetThreadsCount( const uint& count ) {
    ZIPPED_THREADS_COUNT = count;
  }
}