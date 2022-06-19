#include <UnionAfx.h>
#include "VDFS\VDFS.h"


namespace VDFS {
  FileNamePtr::FileNamePtr( const string& name ) : Full( name ) {
    Short = Full + name.Length() - 1;
    while( Short > Full ) {
      const char* next = Short - 1;
      if( *next == '\\' )
        break;

      Short = next;
    }
  }


  FileLayered::FileLayered( const string& fullName ) : FullName( fullName ) {

  }

  const string& FileLayered::GetFullName() const {
    return FullName;
  }

  void FileLayered::Insert( File* file ) {
    file->Lock();
    Files.Insert( file );
  }

  void FileLayered::Remove( File* file ) {
    if( Files.Remove( file ) != Invalid )
      file->Release();
  }

  File* FileLayered::GetTop() {
    return Files.GetFirst();
  }

  bool FileLayered::IsEmpty() {
    return Files.GetNum() == 0;
  }

  void FileLayered::Clear() {
    Files.ReleaseData();
    Files.Clear();
  }

  FileLayered::~FileLayered() {
    Clear();
  }






  FileNode::FileNode( const string& name ) : Name( name ) {
    
  }

  const string& FileNode::GetName() const {
    return Name;
  }

  void FileNode::Insert( File* file ) {
    const string& fullName = file->GetFullName();
    uint index = FileLayers.SearchEqualSorted( fullName );
    if( index != Invalid ) {
      FileLayered* layered = FileLayers[index];
      layered->Insert( file );
      return;
    }

    FileLayered* layered = new FileLayered( fullName );
    layered->Insert( file );
    FileLayers.InsertSorted( layered );
  }

  void FileNode::Remove( File* file ) {
    const string& fullName = file->GetFullName();
    uint index = FileLayers.SearchEqualSorted( fullName );
    if( index != Invalid ) {
      FileLayered* layered = FileLayers[index];
      layered->Remove( file );
      if( layered->IsEmpty() )
        FileLayers.DeleteAt( index );
    }
  }

  FileLayered* FileNode::FindLayer( const FileNamePtr& namePtr ) {
    if( namePtr.Full != namePtr.Short ) {
      uint index = FileLayers.SearchEqualSorted( namePtr.Full );
      if( index == Invalid )
        return Null;

      return FileLayers[index];
    }
    
    return FileLayers.GetLast();
  }

  Array<FileLayered*> FileNode::FindLayers( const FileNamePtr& namePtr ) {
    return FileLayers;
  }

  bool FileNode::IsEmpty() {
    return FileLayers.GetNum() == 0;
  }

  uint FileNode::GetLayersNum() {
    return FileLayers.GetNum();
  }

  uint FileNode::FillFileList( char** list ) {
    uint num = 0;
    for each( auto& layer in FileLayers ) {
      string srcName = layer->GetTop()->GetFullName();
      char* dstName = new char[srcName.Length() + 1];
      memcpy( dstName, srcName, srcName.Length() + 1 );
      list[num++] = dstName;
    }
    return num;
  }

  FileNode::~FileNode() {
    FileLayers.DeleteData();
  }





  void FileTable::Insert( File* file ) {
    const string& name = file->GetName();
    uint index = FileNodes.SearchEqualSorted( name );
    if( index != Invalid ) {
      FileNode* node = FileNodes[index];
      node->Insert( file );
      return;
    }

    FileNode* node = new FileNode( name );
    node->Insert( file );
    FileNodes.InsertSorted( node );
  }

  void FileTable::Remove( File* file ) {
    const string& name = file->GetName();
    uint index = FileNodes.SearchEqualSorted( name );
    if( index != Invalid ) {
      FileNode* node = FileNodes[index];
      node->Remove( file );
      if( node->IsEmpty() )
        FileNodes.DeleteAt( index );
    }
  }

  FileLayered* FileTable::FindLayer( const string& name ) {
    FileNamePtr namePtr = name;
    uint index = FileNodes.SearchEqualSorted( namePtr.Short );
    if( index == Invalid )
      return Null;

    return FileNodes[index]->FindLayer( namePtr );
  }

  Array<FileLayered*> FileTable::FindLayers( const string& name ) {
    FileNamePtr namePtr = name;
    uint index = FileNodes.SearchEqualSorted( namePtr.Short );
    if( index == Invalid )
      return Array<FileLayered*>();

    return FileNodes[index]->FindLayers( namePtr );
  }

  uint FileTable::MakeFileList( char**& list ) {
    uint fillCount = 0;
    list = new char*[GetLayersNum()];
    for each( auto & node in FileNodes )
      fillCount += node->FillFileList( &list[fillCount] );
    return fillCount;
  }

  uint FileTable::GetLayersNum() {
    uint num = 0;
    for each( auto& node in FileNodes )
      num += node->GetLayersNum();
    return num;
  }

  FileTable::~FileTable() {
    FileNodes.DeleteData();
  }
}