#pragma once
#include <zlib\ZippedStream\ZippedAfx.h>

namespace VDFS {
  enum FileFlags {
    File_Normal = 0,
    File_Zipped = 1,
    File_Ogg    = 2
  };


  struct FileVersion {
    union {
      struct {
        uint16 Major;
        uint16 Minor;
        uint16 Micro;
        uint16 Build;
      };
      uint64 Binary;
    };
  };


  class Volume;


  class File : public VDFSObject {
    friend class FileStream;

    Volume* ContainingVolume;
    IFILE* BaseFile;
    string BaseDirectory;
    string SubDirectory;
    string RealName;
    string FullName;
    string Name;
    uint BasePosition;
    uint Size;
    FileFlags Flags;
    FileVersion Version;
    uint Timestamp;
    bool WaveFile;

    File();
    ~File();
    void SetBaseFile( IFILE* baseFile );
    void UpdateVersion();
    void UpdateSize();
    void CheckAutorun();
    void CheckWaveFile();
    void CheckFakeSystem();
    static FileStream* CreateFileStream( File* file, IFILE* handle );

  public:
    FileStream* Open();
    void Close();
    const string& GetName() const;
    const string& GetFullName() const;
    const string& GetRealName() const;
    const FileVersion& GetVersion() const;
    const uint& GetTimestamp() const;
    const Volume* GetVolume() const;
    const uint& GetBasePosition() const;
    bool IsVirtual() const;
    bool IsZipped() const;
    bool IsWaveFile() const;

    static File* Create(
      Volume* volume,
      const string& subDirectory,
      const string& name,
      const uint& basePosition,
      const uint& size
      );

    static File* Create(
      const string& baseDirectory,
      const string& subDirectory,
      const string& name
      );
  };


  class Directory : public VDFSObject {
    Directory* Parent;
    Array<Directory*> Directories;
    Array<File*> Files;
    string Name;

  public:
    Directory( Directory* parent = Null );
    const Array<Directory*>& GetDirectories() const;
    const Array<File*>& GetFiles() const;
    void Insert( Directory* directory );
    void Insert( File* file );
    void Remove( Directory* directory );
    void Remove( File* file );
    void SetName( const string& name );
    const string GetName() const;
    const string GetFullName() const;
    Directory* GetDirectory( const string& path );
    File* GetFile( const string& path );
    Array<Directory*> FindDirectories( const string& namePtr );
    Array<File*> FindFiles( const string& namePtr );
    const Directory* GetDirectory( const string& path ) const;
    const File* GetFile( const string& path ) const;
    Array<const Directory*> FindDirectories( const string& namePtr ) const;
    Array<const File*> FindFiles( const string& namePtr ) const;
    ~Directory();
  };


  class FileStream : public VDFSObject {
    friend class File;
    friend class FileStreamOgg;

  protected:
    File* BaseFile;
    FileCache* Cache;
    FileStream( File* file, IFILE* handle );

  public:
    virtual const string& GetName() const;
    virtual const string& GetFullName() const;
    virtual long Read( void* data, const uint& length );
    virtual int SetPosition( const long& position, const uint& origin = SEEK_SET );
    virtual long GetPosition() const;
    virtual long GetSize() const;
    virtual void Close();
    virtual IFILE* GetHandle();
    virtual ~FileStream();
  };


  class FileStreamZipped : public FileStream {
    friend class File;

  protected:
    ZippedStreamReader* ZippedReader;
    void UpdateMultithreading();
    FileStreamZipped( File* file, IFILE* handle );

  public:
    virtual long Read( void* data, const uint& length );
    virtual int SetPosition( const long& position, const uint& origin = SEEK_SET );
    virtual long GetPosition() const;
    virtual void Close();
    virtual ~FileStreamZipped();

    static void SetThreadsCount( const uint& count );
  };


  inline string ToString( File* file ) {
    return file->GetName();
  }

  inline int Compare( File* file, const char* fileName ) {
    return strcmp( file->GetName(), fileName );
  }

  inline int Compare( const char* fileName, File* file ) {
    return strcmp( fileName, file->GetName() );
  }

  inline int Compare( File* file1, File* file2 ) {
    return strcmp( file1->GetName(), file2->GetName() );
  }
}