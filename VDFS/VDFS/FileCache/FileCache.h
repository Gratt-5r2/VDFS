#pragma once


namespace VDFS {
  class FileCache {
    friend class FileStream;

    IFILE* File;
    uint CacheSizeMax;
    uint CacheSize;
    uint CacheBase;
    uint FilePosition;
    uint FileEnd;
    byte* Cache;
    uint ReadSizeLimit;

    bool IsOnRange( const uint& length );
    bool InitRange( const uint& base );
    long ReadFromFile( void* data, const uint& length );
    long ReadFromCache( void* data, const uint& length );
    long GetCachedBytesCountToRead();
    long GetCachePosition();
  public:

    FileCache( IFILE* file, const uint& size = 0x2000 );
    void SetRange( const uint& start, const uint& length );
    long Read( void* data, const uint& length );
    int Seek( const uint& position );
    long Tell();
    void Free();
    ~FileCache();
  };
}