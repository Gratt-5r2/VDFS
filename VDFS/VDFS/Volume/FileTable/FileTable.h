#pragma once


namespace VDFS {
  struct FileNamePtr {
    const char* Full;
    const char* Short;
    FileNamePtr( const string& name );
  };


  class FileLayered {
    inline static int LayersCompare( File* const& file1, File* const& file2 ) {
      const uint64& version1 = file1->GetVersion().Binary;
      const uint64& version2 = file2->GetVersion().Binary;
      if( version1 > version2 ) return -1;
      if( version1 < version2 ) return  1;

      const uint& timestamp1 = file1->GetTimestamp();
      const uint& timestamp2 = file2->GetTimestamp();
      if( timestamp1 > timestamp2 ) return -1;
      if( timestamp1 < timestamp2 ) return  1;

      return  0;
    }

    string FullName;
    ArraySorted<File*, FileLayered::LayersCompare> Files;
  public:
    FileLayered( const string& fullName );
    const string& GetFullName() const;
    void Insert( File* file );
    void Remove( File* file );
    File* GetTop();
    bool IsEmpty();
    void Clear();
    ~FileLayered();
  };

  inline int Compare( FileLayered* file, const char* fileName ) {
    return strcmp( file->GetFullName(), fileName );
  }

  inline int Compare( const char* fileName, FileLayered* file ) {
    return strcmp( fileName, file->GetFullName() );
  }

  inline int Compare( FileLayered* file1, FileLayered* file2 ) {
    return strcmp( file1->GetFullName(), file2->GetFullName() );
  }


  class FileNode {
    string Name;
    Array<FileLayered*> FileLayers;
  public:
    FileNode( const string& name );
    const string& GetName() const;
    void Insert( File* file );
    void Remove( File* file );
    FileLayered* FindLayer( const FileNamePtr& namePtr );
    Array<FileLayered*> FindLayers( const FileNamePtr& namePtr );
    bool IsEmpty();
    uint GetLayersNum();
    uint FillFileList( char** list );
    ~FileNode();
  };

  inline int Compare( FileNode* fileNode, const char* fileName ) {
    return strcmp( fileNode->GetName(), fileName );
  }

  inline int Compare( const char* fileName, FileNode* fileNode ) {
    return strcmp( fileName, fileNode->GetName() );
  }

  inline int Compare( FileNode* fileNode1, FileNode* fileNode2 ) {
    return strcmp( fileNode1->GetName(), fileNode2->GetName() );
  }


  class FileTable {
    Array<FileNode*> FileNodes;
  public:
    void Insert( File* file );
    void Remove( File* file );
    FileLayered* FindLayer( const string& namePtr );
    Array<FileLayered*> FindLayers( const string& namePtr );
    uint MakeFileList( char**& list );
    uint GetLayersNum();
    ~FileTable();
  };
}