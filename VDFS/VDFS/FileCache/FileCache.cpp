#include <UnionAfx.h>
#include "VDFS\VDFS.h"


namespace VDFS {
  FileCache::FileCache( IFILE* file, const uint& size ) {
    File = file;
    File->Open();
    CacheSizeMax = size;
    CacheSize = 0;
    ReadSizeLimit = size / 8;
    Cache = Null;
    FileEnd = 0xFFFFFFFF;
  }


  void FileCache::SetRange( const uint& start, const uint& length ) {
    Seek( start );
    FileEnd = start + length;
  }


  long FileCache::ReadFromFile( void* data, const uint& length ) {
    File->SetPosition( FilePosition );
    long readed = File->Read( data, length );
    FilePosition += length;
    return readed;
  }


  long FileCache::ReadFromCache( void* data, const uint& length ) {
    long maxToRead = GetCachedBytesCountToRead();
    long wantToRead = min( (long&)length, maxToRead );
    long cachePos = GetCachePosition();
    memcpy( data, Cache + cachePos, wantToRead );
    FilePosition += wantToRead;
    return wantToRead;
  }


  long FileCache::GetCachedBytesCountToRead() {
    return CacheSize - GetCachePosition();
  }


  long FileCache::GetCachePosition() {
    return FilePosition - CacheBase;
  }


  bool FileCache::IsOnRange( const uint& length ) {
    if( !Cache )
      return false;

    if( FilePosition < CacheBase )
      return false;

    if( FilePosition + length > CacheBase + CacheSize )
      return false;

    return true;
  }


  bool FileCache::InitRange( const uint& base ) {
    if( !Cache )
      Cache = new byte[CacheSizeMax];

    File->SetPosition( base );
    CacheSize = File->Read( Cache, CacheSizeMax );
    CacheBase = base;
    return true;
  }


  long FileCache::Read( void* data, const uint& length ) {
    if( FilePosition >= FileEnd )
      return 0;

    uint toRead = min2( length, FileEnd - FilePosition );
    if( toRead > ReadSizeLimit )
      return ReadFromFile( data, toRead );

    if( !IsOnRange( toRead ) )
      InitRange( FilePosition );

    return ReadFromCache( data, toRead );
  }


  int FileCache::Seek( const uint& position ) {
    int ok = File->SetPosition( position, SEEK_SET );
    FilePosition = File->GetPosition();
    return ok;
  }


  long FileCache::Tell() {
    return FilePosition;
  }


  void FileCache::Free() {
    if( Cache ) {
      delete[] Cache;
      Cache = Null;
    }
  }


  FileCache::~FileCache() {
    Free();
    if( File )
      File->Close();
  }
}